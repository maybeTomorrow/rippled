#include "sm2.h"
#include "sm3.h"
#include <iostream>

typedef std::vector<unsigned char> bytes;
typedef unsigned short int uint16;


bytes ZA(point &pA, bytes uid)
{
	Curve cu;
	if (uid.size() == 0)
	{
		uid = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};
	}

	bytes za;
	uint16 Entla = uint16(8 * uid.size());
	za.push_back(((Entla >> 8) & 0xFF));
	za.push_back((Entla & 0xFF));

	if (uid.size() > 0)
	{
		byteAppend(za, uid);
	}

	byteAppend(za, gmpToByte(cu.a_coeffi));

	byteAppend(za, gmpToByte(cu.b_coeffi));
	byteAppend(za, gmpToByte(cu.G.x));
	byteAppend(za, gmpToByte(cu.G.y));

	bytes xBuf = gmpToByte(pA.x);
	bytes yBuf = gmpToByte(pA.y);
	int n = xBuf.size();
	if (n < 32)
	{
		xBuf.insert(xBuf.begin(), 32 - n, 0);
	}

	n = yBuf.size();
	if (n < 32)
	{
		yBuf.insert(yBuf.begin(), 32 - n, 0);
	}
	byteAppend(za, xBuf);
	byteAppend(za, yBuf);

	unsigned int *rs = sm3(za);

	mpz_t hash;
	mpz_init(hash);
	intToGmp(hash, rs, 256);
	return gmpToByte(hash);
}


// 生成公钥私钥
void key_from_hex(User &auser, Curve &acurve, const char *str)
{
	mpz_set_str(auser.dA, str, 16);
	acurve.mul(auser.pA, acurve.G, auser.dA);
}
// 生成公钥私钥
void generate_key(User &auser, Curve &acurve)
{
	gmp_randstate_t state;
	gmp_randinit_default(state);
	unsigned int seed;
	syscall(SYS_getrandom, &seed, sizeof(unsigned long int), 1);
	gmp_randseed_ui(state, seed);
	do
	{
		mpz_urandomm(auser.dA, state, acurve.n_ranks);
	} while (!mpz_cmp_ui(auser.dA, 0));
	gmp_randclear(state);

	acurve.mul(auser.pA, acurve.G, auser.dA);
}

// 数字签名生成
void sign_generate(Signature &result, bytes message, User &user, Curve &acurve)
{
	gmp_randstate_t state;
	gmp_randinit_default(state);
	unsigned int seed;
	syscall(SYS_getrandom, &seed, sizeof(unsigned long int), 1);
	gmp_randseed_ui(state, seed);

	bytes uid;
	bytes z_a = ZA(user.pA, uid);
	bytes final_message;
	mpz_t k, temp;
	point apoint;
	unsigned int sz;
	unsigned int *mess, *hashResult;
	mpz_init(k);
	mpz_init(temp);
	while (true)
	{
		do
		{
			mpz_urandomm(k, state, acurve.n_ranks);
		} while (!mpz_cmp_ui(k, 0)); // k属于[1,n-1]
		acurve.mul(apoint, acurve.G, k);
		// sz = gmpToInt(mess, message, mpz_sizeinbase(message, 16) * 4);
		// hashResult = sm3(message);
		final_message = byteMarge(z_a, message);
		hashResult = sm3(final_message);
		// mpz_set_str(result.r,"f4d62937b58f1e21c5a073186ff5bd7d5b34e59b0011eaa506fd9be0a6ce3c8d",16);
		intToGmp(result.r, hashResult, 256);
		std::cout << "a hash:" << mpz_get_str(NULL, 16, result.r);
		mpz_add(result.r, result.r, apoint.x);
		mpz_mod(result.r, result.r, acurve.n_ranks);

		// r==0 或 r+k==n 则返回重新生成k
		mpz_add(temp, result.r, k);
		if (!mpz_cmp_ui(result.r, 0) || !mpz_cmp(acurve.n_ranks, temp))
			continue;

		mpz_mul(temp, result.r, user.dA);
		mpz_sub(temp, k, temp);
		mpz_add_ui(result.s, user.dA, 1);
		mpz_invert(result.s, result.s, acurve.n_ranks);
		mpz_mul(result.s, result.s, temp);
		mpz_mod(result.s, result.s, acurve.n_ranks);

		if (!mpz_cmp_ui(result.s, 0))
			continue; // s==0 重新生成k
		break;
	}
	// delete mess;
	delete hashResult;
	gmp_randclear(state);
	mpz_clear(temp);
	mpz_clear(k);
}



// 签名验证
bool sign_verify(Signature &result, bytes message, point &pA, Curve &acurve)
{
 
 
	bytes uid;
	bytes z_a = ZA(pA, uid);
	bytes final_message = byteMarge(z_a, message);
	unsigned int *hashResult = sm3(final_message);

	mpz_t e, t;
	mpz_init(e);
	mpz_init(t);
	intToGmp(e, hashResult, 256);

	mpz_add(t, result.r, result.s);
	mpz_mod(t, t, acurve.n_ranks);
	point pointa, pointb;
	acurve.mul(pointa, acurve.G, result.s);
	acurve.mul(pointb, pA, t);
	acurve.add(pointa, pointa, pointb);
	mpz_add(e, e, pointa.x);
	mpz_mod(e, e, acurve.n_ranks);
	bool flag;
	if (!mpz_cmp_ui(t, 0) || mpz_cmp(result.r, e))
		flag = false; // t==0 或 r！=e 验证不通过
	else
		flag = true;
	delete hashResult;
	mpz_clear(t);
	mpz_clear(e);
	return flag;
}
