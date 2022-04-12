#include "set.hpp"
#include <math.h>
#include "constants.hpp"
#include <iostream>

using namespace std;

CACHE::CACHE()
{
}

CACHE::CACHE(unsigned long s_block, unsigned long s_cache, unsigned long w_cache, CACHE *n_level)
{
    this->s_block = s_block;
    this->s_cache = s_cache;
    this->w_cache = w_cache;
    this->n_level = n_level;

    //number of sets per block
    c_sets = s_cache / (s_block * w_cache);

    //#BITS
    b_blockoffset = ceil(log2(s_block));
    b_index = ceil(log2(c_sets));
    b_tag = s_address - b_index - b_blockoffset;

    //MASKS
    for (unsigned long i = 0; i < b_blockoffset; i++)
    {
        m_blockoffset = (m_blockoffset << 1) + 1;
    }

    for (unsigned long i = 0; i < b_index; i++)
    {
        m_index = (m_index << 1) + 1;
    }
    m_index = m_index << (b_blockoffset);

    for (unsigned long i = 0; i < b_tag; i++)
    {
        m_tag = (m_tag << 1) + 1;
    }
    m_tag = m_tag << (b_index + b_blockoffset);

    //TagArray initialization
    tagStore = new TagArray *[c_sets];
    for (unsigned long i = 0; i < c_sets; i++)
    {
        tagStore[i] = new TagArray[w_cache];
    }

    for (unsigned long i = 0; i < c_sets; i++)
    {
        for (unsigned long j = 0; j < w_cache; j++)
        {
            tagStore[i][j].t_tag = vacant;
            tagStore[i][j].lastAccessTimes = j;
            tagStore[i][j].t_data.isDirty = false;
            tagStore[i][j].t_data.isValid = false;
        }
    }
}

void CACHE::cachePerformance()
{
    //cache performance calculation - miss rate
    miss_rate = ((float)(read_miss + write_miss)) / (reads + writes);
}

void CACHE::readOp(unsigned long address)
{
    reads++;
    //#BITS
    unsigned long t_tag = (address & m_tag) >> (s_address - b_tag);
    unsigned long index = (address & m_index) >> (b_blockoffset);

    //Traversing through all ways (sets) and returns a hit if a tag with valid data is matched
    for (unsigned long j = 0; j < w_cache; j++)
    {
        if (tagStore[index][j].t_tag == t_tag && tagStore[index][j].t_data.isValid == true)
        {
            read_hit++;
            resetlastAccessTimes(index, tagStore[index][j].lastAccessTimes);
            return;
        }
    }

    read_miss++;
    //if no tag match, traverse through all ways and checks for a vacant one
    {
        for (unsigned long j = 0; j < w_cache; j++)
        {
            if (tagStore[index][j].t_tag == vacant && tagStore[index][j].t_data.isValid == false)
            {
                requestL2Read(address);
                tagStore[index][j].t_tag = t_tag;
                tagStore[index][j].t_data.isValid = true;
                resetlastAccessTimes(index, tagStore[index][j].lastAccessTimes);
                return;
            }
        }
    }
    unsigned long i_replace = replace(address);
    requestL2Read(address);
    tagStore[index][i_replace].t_tag = t_tag;
    tagStore[index][i_replace].t_data.isValid = true;
    tagStore[index][i_replace].t_data.isDirty = false;
    resetlastAccessTimes(index, tagStore[index][i_replace].lastAccessTimes);
}

void CACHE::writeOp(unsigned long address)
{
    writes++;
    //#BITS
    unsigned long t_tag = (address & m_tag) >> (s_address - b_tag);
    unsigned long index = (address & m_index) >> (b_blockoffset);

    //Traversing through all ways (sets) and returns a hit if a tag with valid data is matched
    for (unsigned long j = 0; j < w_cache; j++)
    {
        if (tagStore[index][j].t_tag == t_tag && tagStore[index][j].t_data.isValid == true)
        {
            write_hit++;
            tagStore[index][j].t_data.isDirty = true;
            resetlastAccessTimes(index, tagStore[index][j].lastAccessTimes);
            return;
        }
    }

    write_miss++;
    //if no tag match, traverse through all ways and checks for a vacant one
    {
        for (unsigned long j = 0; j < w_cache; j++)
        {
            if (tagStore[index][j].t_tag == vacant && tagStore[index][j].t_data.isValid == false)
            {
                requestL2Read(address);
                tagStore[index][j].t_tag = t_tag;
                tagStore[index][j].t_data.isValid = true;
                tagStore[index][j].t_data.isDirty = true;
                resetlastAccessTimes(index, tagStore[index][j].lastAccessTimes);
                return;
            }
        }
    }
    unsigned long i_replace = replace(address);
    requestL2Read(address);
    tagStore[index][i_replace].t_tag = t_tag;
    tagStore[index][i_replace].t_data.isValid = true;
    tagStore[index][i_replace].t_data.isDirty = true;
    resetlastAccessTimes(index, tagStore[index][i_replace].lastAccessTimes);
}

unsigned long CACHE::replace(unsigned long address)
{
    //#BITS
    unsigned long t_tag = (address & m_tag) >> (s_address - b_tag);
    unsigned long index = (address & m_index) >> (b_blockoffset);

    //getting the datablock that can be replaced - LRU policy
    unsigned long i_replace; 
    for (unsigned long j = 0; j < w_cache; j++)
    {
        if (tagStore[index][j].lastAccessTimes == (w_cache - 1))
        {
            i_replace = j;
            break;
        }
    }
    //write to nextlevel
    if (tagStore[index][i_replace].t_data.isValid && tagStore[index][i_replace].t_data.isDirty)
    {
        unsigned long oldAddress = (tagStore[index][i_replace].t_tag << (b_index + b_blockoffset)) + (index << (b_blockoffset));
        requestL2Write(oldAddress);
        tagStore[index][i_replace].t_data.isValid = false;
        tagStore[index][i_replace].t_data.isDirty = false;
    }
    return i_replace;
}

void CACHE::resetlastAccessTimes(unsigned long index, unsigned long lastAccessTimes)
{
    for (unsigned long j = 0; j < w_cache; j++)
    {
        if (tagStore[index][j].lastAccessTimes < lastAccessTimes)
        {
            tagStore[index][j].lastAccessTimes++;
        }
        else if (tagStore[index][j].lastAccessTimes == lastAccessTimes)
        {
            tagStore[index][j].lastAccessTimes = 0;
        }
    }
}

//used when a miss is observed in L1
void CACHE::requestL2Read(unsigned long address)
{
    if (n_level != NULL)
    {
        n_level->readOp(address);
    }
}

//used when some block is to be evicted from L1
void CACHE::requestL2Write(unsigned long address)
{
    write_backs++;
    if (n_level != NULL)
    {
        n_level->writeOp(address);
    }
}