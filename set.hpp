struct DataArray
{
    bool isValid;
    bool isDirty;
};

struct TagArray
{
    long long t_tag;         
    unsigned long lastAccessTimes;
    DataArray t_data;
};

class CACHE{

    private:
        unsigned long s_block;   
        unsigned long s_cache; 
        unsigned long w_cache; 
        struct TagArray **tagStore;    
        CACHE *n_level;              

        unsigned long c_sets;
        unsigned long b_blockoffset;
        unsigned long b_index;
        unsigned long b_tag;

        unsigned long m_tag = 0;
        unsigned long m_index = 0;
        unsigned long m_blockoffset = 0;

        unsigned long replace(unsigned long);
        void resetlastAccessTimes(unsigned long, unsigned long);
        void requestL2Write(unsigned long); 
        void requestL2Read(unsigned long);

    public:
        unsigned long reads = 0;
        unsigned long read_miss = 0;
        unsigned long read_hit = 0;
        unsigned long writes = 0;
        unsigned long write_miss = 0;
        unsigned long write_hit = 0;
        unsigned long write_backs = 0;
        unsigned long block_miss = 0;
        float miss_rate = 0;

        CACHE();
        CACHE(unsigned long, unsigned long, unsigned long, CACHE*);
        void cachePerformance();
        void readOp(unsigned long);
        void writeOp(unsigned long);
};