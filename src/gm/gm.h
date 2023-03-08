#ifndef _GM_H
#define _GM_H
#include "sm3.h"
#include <stddef.h>


void sm2_sign_generate(const unsigned char *,const unsigned char *, uint64 ,uint8 *,size_t *);
bool sm2_sign_verify(const unsigned char *, size_t , const uint8[33], const uint8[72]);
void sm2_pubkey_create_compress(uint8 *, const uint8*);
void sm3_hash(const unsigned char *m, size_t mlen,unsigned char rs[32]);
size_t hex2bytes(const char *,uint8 *);
void bytes2hex(char *&, const uint8 *, size_t);
#endif
   
