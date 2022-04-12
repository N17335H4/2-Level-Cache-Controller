#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <sstream>
#include <stdlib.h>
#include <iomanip>
#include "set.hpp"
#include "constants.hpp"

using namespace std;

unsigned long s_l1_block;    //L1 block size   
unsigned long s_l1;          //L1 size  
unsigned long w_l1;          //#ways in L1 
unsigned long s_l2_block;    //L2 block size
unsigned long s_l2;          //L2 size  
unsigned long w_l2;          //#ways in L2
char *input;                 //input trace filename

int main(int argc, char *argv[])
{
    int option;
    cout << "Enter 0 for usim traces or 1 for paired benchmarks" << endl;
    cin >> option;
    switch (option)
    {
    case 0: // cl_packet::traceReader()
        break;
    
    default:
        break;
    }

    ifstream fin;
	fin.open("configurations.txt");
    //gathering cache configuration information
	while (fin) {
        fin >> s_l1_block;
        fin >> s_l1;
        fin >> w_l1;
        fin >> s_l2_block;
        fin >> s_l2;
        fin >> w_l2;
	}
    
	fin.close();
    input = argv[1];

    //displaying cache configuration information
    cout << "\n  size of the block:\t\t" << s_l1_block;
    cout << "\n  size of L1 cache:\t\t" << s_l1;
    cout << "\n  L1 set associativity:\t\t" << w_l1;
    cout << "\n  size of L2 cache:\t\t" << s_l2;
    cout << "\n  L1 set associativity:\t\t" << w_l2;
    cout << "\n  input file path:\t\t" << input << "\n\n";

    CACHE L2;
    CACHE L1;
    //Cache initialization
    if (s_l2 != 0)
    {
        L2 = CACHE(s_l2_block, s_l2, w_l2, NULL);

        L1 = CACHE(s_l1_block, s_l1, w_l1, &L2);
    }
    else
    {
        L1 = CACHE(s_l1_block, s_l1, w_l1, NULL);
    }
    
    FILE *fp;
    fp = fopen(input, "r");
    if (fp == NULL)
    {
        exit(0);
    }

    char operation;                   //stores operation is read/write
    char addressString[s_address];    //stores address in string format
    unsigned long address;            //stores address in unsigned long format
    char temp[1];

    //Cache performance variables
    unsigned long l1_Rhit = 0;
    unsigned long l1_Rmiss = 0;
    unsigned long l1_Whit = 0;
    unsigned long l1_Wmiss = 0;
    unsigned long l2_Rhit = 0;
    unsigned long l2_Rmiss = 0;
    unsigned long l2_Whit = 0;
    unsigned long l2_Wmiss = 0;

    stringstream ss;
    operation = fgetc(fp);
    while (operation != EOF)
    {
        fgets(temp, sizeof(char), (FILE *)fp);          
        fgets(addressString, s_address, (FILE *)fp); 
        ss << hex << addressString;
        ss >> address;

        if (operation == 'r' || operation == 'R')
        {
            L1.readOp(address);
            //when L1 hit
            if(l1_Rmiss==L1.read_miss) {
                l1_Rhit++ ;
                cout << "read hit no-action" << endl;
            }
            //when L1 miss and L2 hit
            else if(l2_Rmiss==L2.read_miss) {
                l2_Rhit++;
                l1_Rmiss=L1.read_miss;
                cout << "read miss hit" << endl;
            }
            //when both L1 and L2 miss
            else {
                l1_Rmiss=L1.read_miss;
                l2_Rmiss=L2.read_miss;
                cout << "read miss miss" << endl;
            }
        }
        else if (operation == 'w' || operation == 'W')
        {
            char data = 0;
            L1.writeOp(address);
            //when L1 hit
            if(l1_Wmiss==L1.write_miss) {
                l1_Whit++;
                cout << "write hit no-action" << endl;
            }
            //when L1 miss and L2 hit
            else if(l2_Rmiss==L2.read_miss) {
                l1_Wmiss=L1.write_miss;
                cout << "write miss hit" << endl;
            }
            //when both L1 and L2 miss
            else {
                l1_Wmiss=L1.write_miss;
                l2_Rmiss=L2.read_miss;
                l2_Wmiss=L2.write_miss;
                cout << "write miss miss" << endl;
            }
        }

        operation = fgetc(fp);
    }
    fclose(fp);

    //gets cache performance results
    L1.cachePerformance();
    if (s_l2 != 0)
    {
        L2.cachePerformance();
    }

    //displaying results
    cout << "\nnumber of L1 reads:" << dec << L1.reads;
    cout << "\nnumber of L1 read misses:" << dec << L1.read_miss;
    cout << "\nnumber of L1 writes:" << dec << L1.writes;
    cout << "\nnumber of L1 write misses:" << dec << L1.write_miss;
    cout << "\nL1 hit rate:" << fixed << setprecision(4) << 1 - L1.miss_rate;
    cout << "\nnumber of writebacks from L1 memory:" << dec << L1.write_backs;
    if (s_l2 != 0)
    {
        cout << "\nnumber of L2 trace accesses:" << dec << L2.reads;
        cout << "\nnumber of L2 misses:" << dec << L2.read_miss;
        cout << "\nL2 hit rate:" << fixed << setprecision(4) << 1-((float)L2.read_miss / (L2.reads));
        cout << "\nnumber of writebacks from L2 memory:" << dec << L2.write_backs;
    }

    return 0;
}