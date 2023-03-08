#include <cstddef>
#include <vector>
#include "sm3.h"



#define shiftLeft(x, j) ((x << j) | (x >> (32 - j)))
#define P0(x) (x ^ shiftLeft(x, 9) ^ shiftLeft(x, 17))
#define P1(x) (x ^ shiftLeft(x, 15) ^ shiftLeft(x, 23))
#define FF(x, y, z, j) ((j < 16) ? (x ^ y ^ z) : ((x & y) | (x & z) | (y & z)))
#define GG(x, y, z, j) ((j < 16) ? (x ^ y ^ z) : ((x & y) | ((~x) & z)))

const uint T[64] = {
	0x79cc4519, 0xf3988a32, 0xe7311465, 0xce6228cb, 0x9cc45197, 0x3988a32f, 0x7311465e, 0xe6228cbc,
	0xcc451979, 0x988a32f3, 0x311465e7, 0x6228cbce, 0xc451979c, 0x88a32f39, 0x11465e73, 0x228cbce6,
	0x9d8a7a87, 0x3b14f50f, 0x7629ea1e, 0xec53d43c, 0xd8a7a879, 0xb14f50f3, 0x629ea1e7, 0xc53d43ce,
	0x8a7a879d, 0x14f50f3b, 0x29ea1e76, 0x53d43cec, 0xa7a879d8, 0x4f50f3b1, 0x9ea1e762, 0x3d43cec5,
	0x7a879d8a, 0xf50f3b14, 0xea1e7629, 0xd43cec53, 0xa879d8a7, 0x50f3b14f, 0xa1e7629e, 0x43cec53d,
	0x879d8a7a, 0xf3b14f5, 0x1e7629ea, 0x3cec53d4, 0x79d8a7a8, 0xf3b14f50, 0xe7629ea1, 0xcec53d43,
	0x9d8a7a87, 0x3b14f50f, 0x7629ea1e, 0xec53d43c, 0xd8a7a879, 0xb14f50f3, 0x629ea1e7, 0xc53d43ce,
	0x8a7a879d, 0x14f50f3b, 0x29ea1e76, 0x53d43cec, 0xa7a879d8, 0x4f50f3b1, 0x9ea1e762, 0x3d43cec5};

struct Message
{
	bytes arr;		 // 存储消息
	uint64 countBits; // 比特数
	uint W[68];		 // 消息扩展以及杂凑结果存于W【0-7】
	Message(bytes anarr)
	{
		arr = anarr;
		countBits = arr.size() * 8;
	}
	~Message()
	{
	}
};

void CF(uint word[8], Message &mess, uint64 begin)
{
	// 消息扩展
	for (int i = 0; i < 16; i++)
	{
		uint a = 0;
		for (int k = 0; k < 4; k++)
		{
			a |= mess.arr[(begin*16+i) * 4 + k];
			if (k != 3){
				a <<= 8;
			}
				
		}
		mess.W[i] = a;
	}

	for (int i = 16; i < 68; i++)
	{
		uint a = mess.W[i - 16] ^ mess.W[i - 9] ^ shiftLeft(mess.W[i - 3], 15);
		mess.W[i] = P1(a) ^ shiftLeft(mess.W[i - 13], 7) ^ mess.W[i - 6];
	}

	// 压缩过程
	uint wordtemp[8];
	for (int i = 0; i < 8; i++)
		wordtemp[i] = word[i];
	for (int j = 0; j < 64; j++)
	{
		uint ss1, ss2, tt1, tt2;
		ss2 = shiftLeft(word[0], 12);
		ss1 = ss2 + T[j] + word[4];
		ss1 = shiftLeft(ss1, 7);
		ss2 ^= ss1;
		tt1 = FF(word[0], word[1], word[2], j) + word[3] + ss2 + (mess.W[j] ^ mess.W[j + 4]);
		tt2 = GG(word[4], word[5], word[6], j) + word[7] + ss1 + mess.W[j];
		word[3] = word[2];
		word[2] = shiftLeft(word[1], 9);
		word[1] = word[0];
		word[0] = tt1;
		word[7] = word[6];
		word[6] = shiftLeft(word[5], 19);
		word[5] = word[4];
		word[4] = P0(tt2);
	}
	for (int i = 0; i < 8; i++)
		word[i] ^= wordtemp[i];
}
void iteration(Message &mess)
{
	// 消息填充
	mess.arr.push_back(0x80);

	int blockSize = 64;
	while (mess.arr.size() % blockSize != 56)
	{
		mess.arr.push_back(0x00);
	}
	mess.arr.push_back(mess.countBits >> 56 & 0xff);
	mess.arr.push_back(mess.countBits >> 48 & 0xff);
	mess.arr.push_back(mess.countBits >> 40 & 0xff);
	mess.arr.push_back(mess.countBits >> 32 & 0xff);
	mess.arr.push_back(mess.countBits >> 24 & 0xff);
	mess.arr.push_back(mess.countBits >> 16 & 0xff);
	mess.arr.push_back(mess.countBits >> 8 & 0xff);
	mess.arr.push_back(mess.countBits >> 0 & 0xff);

	// 迭代过程
	uint word[8] = {0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600, 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e};
	uint64 sz = mess.arr.size() / 64;
	int i = 0;
	while (sz--){
		CF(word, mess, i++);
	}
		
	for (int i = 0; i < 8; i++)
		mess.W[i] = word[i];
}

unsigned int *sm3(std::vector<unsigned char> buffer)
{
	Message *mess = new Message(buffer);
	iteration(*mess);
	uint *k = new uint[8];
	for (int i = 0; i < 8; i++)
		k[i] = mess->W[i];
	delete mess;
	return k;
}