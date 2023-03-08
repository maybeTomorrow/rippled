#include "sm3.h"
#include "sm2.h"
#include "gm.h"
#include <cstring>
#include <stddef.h>
#include <iostream>

// create public key from private


size_t hex2bytes(const char *str,uint8 *bf){
    return hex2bin(str,strlen(str),bf);
}

void sm2_user_gen( User &auser,const unsigned char *seckey){
    
    Curve cur;
    bytes prv(seckey, seckey + 32);
    byteToGmp(auser.dA, prv);
    cur.mul(auser.pA, cur.G, auser.dA);
}

void sm2_pubkey_create(uint8 *pubkey, const unsigned char *seckey)
{
    User auser;
    sm2_user_gen(auser,seckey);
}

// create public key from private

void sm2_pubkey_create_compress(uint8 *pubkey, const uint8 *seckey)
{
    User auser;
    sm2_user_gen(auser,seckey);
    bytes rs=compress(auser.pA);
    if (!rs.empty()){
        memcpy(pubkey,&rs[0],rs.size()*sizeof(uint8));
    }
}

//sign a message

void sm2_sign_generate(const unsigned char *seckey,const unsigned char *m, uint64 mlen,uint8 *b,size_t *len){

    char *msg,*pub,*rs;
    bytes2hex(msg,m,mlen);
    bytes2hex(pub,seckey,32);


    Signature sign;
    Curve cur;
    User auser;
    bytes message(m,m+mlen);
    sm2_user_gen(auser,seckey);
    sign_generate(sign,message,auser,cur); 
    bytes r=gmpToByte(sign.r);
    bytes s=gmpToByte(sign.s);
    bytes bf;
    uint8 rl=r.size() & 0xff;
    uint8 sl=s.size() & 0xff;

      //like neg
    if((r[0]&0x80)==0x80){
        r.insert(r.begin(),0x00);
    }
    if((s[0]&0x80)==0x80){
        s.insert(s.begin(),0x00);
    }
   
    byteAppend(bf,bytes{0x30, uint8(sl+rl+4),0x02,rl});
    byteAppend(bf,r);
    byteAppend(bf,bytes{0x02,sl});
    byteAppend(bf,s);
    *len=bf.size();
    memcpy(b,&bf[0],bf.size()*sizeof(uint8));

    bytes2hex(rs,b,72);
    std::cout << "sign generate,msg:" << msg <<",pk:"<<pub<<",rs:"<<rs;
}
 


 bool sm2_sign_verify(const unsigned char *m, size_t mlen, const uint8 pk[33], const uint8 RS[72]){

    char *message,*pub,*rs;
    bytes2hex(message,m,mlen);
    bytes2hex(pub,pk,33);
    bytes2hex(rs,RS,72);
    std::cout << "sign verify,msg:" << message <<",pub:"<<pub<<",rs:"<<rs;

    Signature sign;
    //asn1 decode
    int i=0;
    if (RS[i]!=0x30){
        return 0;
    }
    i+=2;
    if (RS[i++]!=0x02){
        return 0;
    }
    //decode r len
    uint8 rlen=RS[i++];
    bytes r,s;
    for(uint8 j=0;j<rlen;j++){
        r.push_back(RS[i++]);
    }
    if (RS[i++]!=0x02){
        return 0;
    }
    rlen=RS[i++];
    for(uint8 j=0;j<rlen;j++){
        s.push_back(RS[i++]);
    }
    byteToGmp(sign.r,r);
    byteToGmp(sign.s,s);
    Curve cur;
    bytes bf(m,m+mlen);
    bytes pub_c(pk,pk+33);
    point p=decompress(pub_c);
    return  sign_verify(sign,bf,p,cur);
 }


// return a sm3 hash
void sm3_hash(const unsigned char *m, size_t mlen,unsigned char rs[32])
{
    bytes buffer(m,m+mlen);;
    uint *hashResult;
    hashResult = sm3(buffer);
    mpz_t a;
    intToGmp(a,hashResult,256);
    bytes ha=gmpToByte(a);
    memcpy(rs,&ha[0],ha.size()*sizeof(uint8));
}
