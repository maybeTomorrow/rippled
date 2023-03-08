#include "sm2.h"
#include "sm3.h"
#include <cstdlib>
#include <iostream>
#include <vector>
const unsigned int base[32] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80,
							   0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000,
							   0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000,
							   0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000};

const unsigned long baseLong[64] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000, 0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000, 0x100000000, 0x200000000, 0x400000000, 0x800000000, 0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000, 0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000, 0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000, 0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000, 0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000, 0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000, 0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000};

const unsigned char cb[8] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};

char *byteToHex(std::vector<unsigned char> bf)
{
	mpz_t a;
	byteToGmp(a, bf);
	return mpz_get_str(NULL, 16, a);
}

// mpz_t 类型数 转换为 int数组
// len为mpz_t数的比特长度
std::vector<unsigned char> gmpToByte(mpz_t mess)
{

	std::vector<unsigned char> buffer;

	unsigned long length = mpz_sizeinbase(mess, 2);
	int bit;
	int index = 0;
	unsigned char a = 0;
	while (length--)
	{
		bit = length % 8;
		if (mpz_tstbit(mess, length))
		{
			a |= cb[bit];
		}
		if (bit == 0)
		{
			buffer.push_back(a);
			index++;
			a = 0;
		}
	}
	return buffer;
}

void byteAppend(std::vector<unsigned char> &buffer1, std::vector<unsigned char> buffer2)
{
	buffer1.insert(buffer1.end(), buffer2.begin(), buffer2.end());
}

std::vector<unsigned char> byteMarge(std::vector<unsigned char> buffer1, std::vector<unsigned char> buffer2)
{
	std::vector<unsigned char> n;
	n.insert(n.end(), buffer1.begin(), buffer1.end());
	n.insert(n.end(), buffer2.begin(), buffer2.end());
	return n;
}

int hexchar2int(char c)
{
	if ('0' <= c && c <= '9')
		return c - '0';
	else if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	else if ('A' <= c && c <= 'F')
		return c - 'A' + 10;
	else
		return -1;
}

void bytes2hex(char * &out, const uint8 *buf, size_t len)
{ 
	out=new char[len*2+1];
	size_t i, j;
	for (i = j = 0; i < len; i++)
	{
		char c;
		c = (buf[i] >> 4) & 0xf;
		c = (c > 9) ? c + 'a' - 10 : c + '0';
		out[j++] = c;
		c = (buf[i] & 0xf);
		c = (c > 9) ? c + 'a' - 10 : c + '0';
		out[j++] = c;
	}
	out[j]='\0';
}

size_t hex2bin(const char *in, size_t inlen, uint8 *out)
{
	size_t blen = inlen / 2;
	int c = 0;
	if (inlen % 2)
		return -1;

	while (inlen)
	{
		if ((c = hexchar2int(*in++)) < 0)
			return -1;
		*out = (uint8_t)c << 4;
		if ((c = hexchar2int(*in++)) < 0)
			return -1;
		*out |= (uint8)c;
		inlen -= 2;
		out++;
	}
	return blen;
}

// mpz_t 类型数 转换为 int数组
// len为mpz_t数的比特长度
void byteToGmp(mpz_t mess, std::vector<unsigned char> buffer)
{

	int size = buffer.size();
	mpz_init2(mess, size * 8);

	for (int i = size - 1; i > -1; i--) // size()容器中实际数据个数
	{
		for (int j = 0; j < 8; j++)
		{
			if (buffer[i] >> j & 1)
			{
				mpz_setbit(mess, (size - i - 1) * 8 + j);
			}
		}
	}
}

// mpz_t 类型数 转换为 int数组
// len为mpz_t数的比特长度
unsigned int gmpToByte(unsigned int *&result, mpz_t mess, unsigned int len)
{
	unsigned long length = mpz_sizeinbase(mess, 2);
	if (len > length)
		length = len;
	unsigned long temp = length;
	if (length % 16 == 0)
	{
		result = new unsigned int[length % 16];
	}
	else
	{
		result = new unsigned int[length % 16 + 1];
	}

	int bit = 31;
	int index = 0;
	result[0] = 0;
	while (length--)
	{
		if (bit == -1)
		{
			index++;
			result[index] = 0;
			bit = 31;
		}
		if (mpz_tstbit(mess, length))
		{
			result[index] |= base[bit];
		}
		bit--;
	}
	return temp;
}
// mpz_t 类型数 转换为 int数组
// len为mpz_t数的比特长度
unsigned int gmpToInt(unsigned int *&result, mpz_t mess, unsigned int len)
{
	unsigned long length = mpz_sizeinbase(mess, 2);
	if (len > length)
		length = len;
	unsigned long temp = length;
	result = new unsigned int[(31 + length) >> 5];
	int bit = 31;
	int index = 0;
	result[0] = 0;
	while (length--)
	{
		if (bit == -1)
		{
			index++;
			result[index] = 0;
			bit = 31;
		}
		if (mpz_tstbit(mess, length))
		{
			result[index] |= base[bit];
		}
		bit--;
	}
	return temp;
}
// mpz_t 类型数 转换为 int数组
// len为mpz_t数的比特长度
unsigned int gmpToLong(unsigned long *&result, mpz_t mess, unsigned int len)
{
	unsigned long length = mpz_sizeinbase(mess, 2);
	if (len > length)
		length = len;
	unsigned long temp = 0;
	result = new unsigned long[(63 + length) >> 6];
	int bit = 0;
	int index = 0;
	result[0] = 0;
	while (temp <= length)
	{
		if (bit == 64)
		{
			index++;
			result[index] = 0;
			bit = 0;
		}
		if (mpz_tstbit(mess, temp))
		{
			result[index] |= baseLong[bit];
		}
		bit++;
		temp++;
	}
	return length;
}

// int数组 转为 mpz_t 数，bits为数组的比特长度
void intToGmp(mpz_t result, unsigned int *arr, unsigned long bits)
{
	unsigned long index = bits / 32 - 1;
	if (bits % 32)
		index++;
	unsigned int k = bits % 32;
	unsigned long i;
	if (k != 0)
	{
		i = k;
		k = arr[index] >> (32 - k);
		mpz_set_ui(result, k);
	}
	else
	{
		i = 0;
		index++;
		mpz_set_ui(result, 0);
	}
	while (index--)
	{
		for (int j = 0; j < 32; j++)
			if (arr[index] >> j & 1)
				mpz_setbit(result, i + j);
		i += 32;
	}
}

// 椭圆曲线上 点到比特串 的转换
void pointToGmp(mpz_t result, point &p, mpz_t Fq)
{
	mpz_t X1, Y1;
	mpz_init_set(X1, p.x);
	mpz_init_set(Y1, p.y);
	unsigned long t = mpz_sizeinbase(Fq, 2);
	unsigned long l = (t + 7) >> 3;
	// int y = mpz_tstbit(p.y, 0); // 点的y坐标最右一比特
	mpz_mul_2exp(result, X1, 8 * l);
	mpz_xor(result, result, Y1);
	mpz_setbit(result, 16 * l + 2);
	mpz_clear(X1);
	mpz_clear(Y1);
}

// 比特串到椭圆曲线上点 的转换
void gmpToPoint(point &result, mpz_t bitstring, Curve &acurve)
{
	unsigned long t = mpz_sizeinbase(acurve.p_prime, 2);
	unsigned long l = (t + 7) >> 3;
	unsigned length = mpz_sizeinbase(bitstring, 2);
	mpz_tdiv_r_2exp(result.x, bitstring, 2 * l * 8);
	mpz_tdiv_r_2exp(result.y, bitstring, l * 8);
	mpz_tdiv_q_2exp(result.x, result.x, l * 8);
	unsigned int begin = 2 * l * 8, end = length - 1, pc = 0;
	while (end >= begin)
	{
		pc <<= 1;
		if (mpz_tstbit(bitstring, end--))
			pc++;
	}
	// bool y;
	if (pc != 4)
		exit(1);
	mpz_t yy;
	mpz_init(yy);
	bool f = acurve.compute(yy, result.x);
	if (!f)
		exit(1);
	mpz_powm_ui(yy, yy, 2, acurve.p_prime);
	if (!mpz_cmp(yy, result.y))
		exit(1);
	mpz_clear(yy);
}
