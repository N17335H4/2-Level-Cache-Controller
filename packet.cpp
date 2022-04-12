/*
two attributes: (address, read/write)
*/
#include <iostream>
#include <fstream>
#include "packet.hpp"
#include "types.hpp"
using namespace std;

void cl_packet::traceReader() {
    string line;
	ifstream fin;
	fin.open("usim_traces.txt");
	while (fin) {
		getline(fin, line);
        cl_packet::parse(line);
	}
	fin.close();
}

void cl_packet::parse(string line) {
    string l_transactionStr = line;
    string l_delimiter = " ";
    size_t l_pos = 0;
    string l_token;
    unsigned int l_count = 0;
    t_VirtualAddress l_address;
    char l_reqType;
    t_Size l_nonLsCount;
    t_data l_data;

    /*
    in the loop, l_count = 0 at first, in the first word, integer part is stored in l_nonLsCount [datatype t_size], and remaining part in l_end
    in the second iteration, l_count = 1, second word is either R/W [exhibiting the type of request]
    in the third iteration, l_count = 2, third word's integer part is stored in l_address and and remaining part in l_end
    in the fourth iteration, l_count = 3, final word having the address is stored in l_data
    */

    //l_pos holds the position of a SPACE occurred in the l_transactionStr, excluding if it is at the end of the string
    while ((l_pos = l_transactionStr.find(l_delimiter)) != string::npos) {
            //l_token has the first word
            l_token = l_transactionStr.substr(0, l_pos);
        const char* l_value = l_token.c_str();
        char* l_end;
        if (l_count == 0) {
            l_nonLsCount = strtol(l_value, &l_end, 0);
        }
        else if (l_count == 1) {
            if (l_value[0] == 'R') {
                    l_reqType = 'R';
                }
                else if (l_value[0] == 'W') {
                    l_reqType = 'W';
                }
                // else {
                //     assert(false);
                // }
        }
        else if (l_count == 2) {
            l_address = strtol(l_value, &l_end, 0);
        }
        else if (l_count == 3) {
            // l_data = new unsigned char[64];
            // memcpy(l_data, l_value, 64);
        }
        //removing leftmost word from l_transactionStr
            l_transactionStr.erase(0, l_pos + l_delimiter.length());
        l_count++;
}
}