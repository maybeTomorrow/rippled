#ifndef _SM3_H
#define _SM3_H

#include <vector>

typedef unsigned char uint8;
typedef unsigned int uint;
typedef unsigned long uint64;
typedef std::vector<unsigned char> bytes;

unsigned int *sm3(std::vector<unsigned char>); 
 
#endif
