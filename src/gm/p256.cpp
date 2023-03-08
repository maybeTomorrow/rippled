#include <gmp.h>
#include "sm2.h"
#include "sm3.h" 

 

const uint bottom29Bits = 0x1FFFFFFF;
const uint bottom28Bits = 0xFFFFFFF;


std::vector<unsigned char> compress(point &pt)
{
    std::vector<unsigned char> a = std::vector<unsigned char>(33);

    a[0] = mpz_tstbit(pt.y, 0);

    bytes ab = gmpToByte(pt.x);
    int bz = ab.size();
    int c = bz - 32;
    for (int i = 1; i < 33; i++)
    {
        if (c < 0)
        {
            a[i] = 0;
        }
        else
        {
            a[i] = ab[c];
        }
        c++;
    }
    return a;
}

const uint sm2P256Carry[] = {
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x2,
    0x0,
    0x1FFFFF00,
    0x7FF,
    0x0,
    0x0,
    0x0,
    0x2000000,
    0x0,
    0x4,
    0x0,
    0x1FFFFE00,
    0xFFF,
    0x0,
    0x0,
    0x0,
    0x4000000,
    0x0,
    0x6,
    0x0,
    0x1FFFFD00,
    0x17FF,
    0x0,
    0x0,
    0x0,
    0x6000000,
    0x0,
    0x8,
    0x0,
    0x1FFFFC00,
    0x1FFF,
    0x0,
    0x0,
    0x0,
    0x8000000,
    0x0,
    0xA,
    0x0,
    0x1FFFFB00,
    0x27FF,
    0x0,
    0x0,
    0x0,
    0xA000000,
    0x0,
    0xC,
    0x0,
    0x1FFFFA00,
    0x2FFF,
    0x0,
    0x0,
    0x0,
    0xC000000,
    0x0,
    0xE,
    0x0,
    0x1FFFF900,
    0x37FF,
    0x0,
    0x0,
    0x0,
    0xE000000,
    0x0,
};

// carry < 2 ^ 3
void p256ReduceCarry(uint *a, uint carry)
{
    a[0] += sm2P256Carry[carry * 9 + 0];
    a[2] += sm2P256Carry[carry * 9 + 2];
    a[3] += sm2P256Carry[carry * 9 + 3];
    a[7] += sm2P256Carry[carry * 9 + 7];
}

uint nonZeroToAllOnes(uint x)
{
    return ((x - 1) >> 31) - 1;
}

void p256ReduceDegree(uint *&a, uint64 *&b)
{

    uint *tmp = new uint[18];
    uint carry, x, xMask;

    // tmp
    // 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  | 8  |  9 | 10 ...
    // 29 | 28 | 29 | 28 | 29 | 28 | 29 | 28 | 29 | 28 | 29 ...
    tmp[0] = uint(b[0]) & bottom29Bits;
    tmp[1] = uint(b[0]) >> 29;
    tmp[1] |= (uint(b[0] >> 32) << 3) & bottom28Bits;
    tmp[1] += uint(b[1]) & bottom28Bits;
    carry = tmp[1] >> 28;
    tmp[1] &= bottom28Bits;
    for (int i = 2; i < 17; i++)
    {
        tmp[i] = (uint(b[i - 2] >> 32)) >> 25;
        tmp[i] += (uint(b[i - 1])) >> 28;
        tmp[i] += (uint(b[i - 1] >> 32) << 4) & bottom29Bits;
        tmp[i] += uint(b[i]) & bottom29Bits;
        tmp[i] += carry;
        carry = tmp[i] >> 29;
        tmp[i] &= bottom29Bits;

        i++;
        if (i == 17)
        {
            break;
        }
        tmp[i] = uint(b[i - 2] >> 32) >> 25;
        tmp[i] += uint(b[i - 1]) >> 29;
        tmp[i] += ((uint(b[i - 1] >> 32)) << 3) & bottom28Bits;
        tmp[i] += uint(b[i]) & bottom28Bits;
        tmp[i] += carry;
        carry = tmp[i] >> 28;
        tmp[i] &= bottom28Bits;
    }
    tmp[17] = uint(b[15] >> 32) >> 25;
    tmp[17] += uint(b[16]) >> 29;
    tmp[17] += uint(b[16] >> 32) << 3;
    tmp[17] += carry;

    for (int i = 0;; i += 2)
    {

        tmp[i + 1] += tmp[i] >> 29;
        x = tmp[i] & bottom29Bits;
        tmp[i] = 0;
        if (x > 0)
        {
            uint set4 = 0;
            uint set7 = 0;
            xMask = nonZeroToAllOnes(x);
            tmp[i + 2] += (x << 7) & bottom29Bits;
            tmp[i + 3] += x >> 22;
            if (tmp[i + 3] < 0x10000000)
            {
                set4 = 1;
                tmp[i + 3] += 0x10000000 & xMask;
                tmp[i + 3] -= (x << 10) & bottom28Bits;
            }
            else
            {
                tmp[i + 3] -= (x << 10) & bottom28Bits;
            }
            if (tmp[i + 4] < 0x20000000)
            {
                tmp[i + 4] += 0x20000000 & xMask;
                tmp[i + 4] -= set4; // 借位
                tmp[i + 4] -= x >> 18;
                if (tmp[i + 5] < 0x10000000)
                {
                    tmp[i + 5] += 0x10000000 & xMask;
                    tmp[i + 5] -= 1; // 借位
                    if (tmp[i + 6] < 0x20000000)
                    {
                        set7 = 1;
                        tmp[i + 6] += 0x20000000 & xMask;
                        tmp[i + 6] -= 1; // 借位
                    }
                    else
                    {
                        tmp[i + 6] -= 1; // 借位
                    }
                }
                else
                {
                    tmp[i + 5] -= 1;
                }
            }
            else
            {
                tmp[i + 4] -= set4; // 借位
                tmp[i + 4] -= x >> 18;
            }
            if (tmp[i + 7] < 0x10000000)
            {
                tmp[i + 7] += 0x10000000 & xMask;
                tmp[i + 7] -= set7;
                tmp[i + 7] -= (x << 24) & bottom28Bits;
                tmp[i + 8] += (x << 28) & bottom29Bits;
                if (tmp[i + 8] < 0x20000000)
                {
                    tmp[i + 8] += 0x20000000 & xMask;
                    tmp[i + 8] -= 1;
                    tmp[i + 8] -= x >> 4;
                    tmp[i + 9] += ((x >> 1) - 1) & xMask;
                }
                else
                {
                    tmp[i + 8] -= 1;
                    tmp[i + 8] -= x >> 4;
                    tmp[i + 9] += (x >> 1) & xMask;
                }
            }
            else
            {
                tmp[i + 7] -= set7; // 借位
                tmp[i + 7] -= (x << 24) & bottom28Bits;
                tmp[i + 8] += (x << 28) & bottom29Bits;
                if (tmp[i + 8] < 0x20000000)
                {
                    tmp[i + 8] += 0x20000000 & xMask;
                    tmp[i + 8] -= x >> 4;
                    tmp[i + 9] += ((x >> 1) - 1) & xMask;
                }
                else
                {
                    tmp[i + 8] -= x >> 4;
                    tmp[i + 9] += (x >> 1) & xMask;
                }
            }
        }

        if (i + 1 == 9)
        {
            break;
        }

        tmp[i + 2] += tmp[i + 1] >> 28;
        x = tmp[i + 1] & bottom28Bits;
        tmp[i + 1] = 0;
        if (x > 0)
        {
            uint set5 = 0;
            uint set8 = 0;
            uint set9 = 0;
            xMask = nonZeroToAllOnes(x);
            tmp[i + 3] += (x << 7) & bottom28Bits;
            tmp[i + 4] += x >> 21;
            if (tmp[i + 4] < 0x20000000)
            {
                set5 = 1;
                tmp[i + 4] += 0x20000000 & xMask;
                tmp[i + 4] -= (x << 11) & bottom29Bits;
            }
            else
            {
                tmp[i + 4] -= (x << 11) & bottom29Bits;
            }
            if (tmp[i + 5] < 0x10000000)
            {
                tmp[i + 5] += 0x10000000 & xMask;
                tmp[i + 5] -= set5; // 借位
                tmp[i + 5] -= x >> 18;
                if (tmp[i + 6] < 0x20000000)
                {
                    tmp[i + 6] += 0x20000000 & xMask;
                    tmp[i + 6] -= 1; // 借位
                    if (tmp[i + 7] < 0x10000000)
                    {
                        set8 = 1;
                        tmp[i + 7] += 0x10000000 & xMask;
                        tmp[i + 7] -= 1; // 借位
                    }
                    else
                    {
                        tmp[i + 7] -= 1; // 借位
                    }
                }
                else
                {
                    tmp[i + 6] -= 1; // 借位
                }
            }
            else
            {
                tmp[i + 5] -= set5; // 借位
                tmp[i + 5] -= x >> 18;
            }
            if (tmp[i + 8] < 0x20000000)
            {
                set9 = 1;
                tmp[i + 8] += 0x20000000 & xMask;
                tmp[i + 8] -= set8;
                tmp[i + 8] -= (x << 25) & bottom29Bits;
            }
            else
            {
                tmp[i + 8] -= set8;
                tmp[i + 8] -= (x << 25) & bottom29Bits;
            }
            if (tmp[i + 9] < 0x10000000)
            {
                tmp[i + 9] += 0x10000000 & xMask;
                tmp[i + 9] -= set9; // 借位
                tmp[i + 9] -= x >> 4;
                tmp[i + 10] += (x - 1) & xMask;
            }
            else
            {
                tmp[i + 9] -= set9; // 借位
                tmp[i + 9] -= x >> 4;
                tmp[i + 10] += x & xMask;
            }
        }
    }

    carry = uint(0);
    for (int i = 0; i < 8; i++)
    {
        a[i] = tmp[i + 9];
        a[i] += carry;
        a[i] += (tmp[i + 10] << 28) & bottom29Bits;
        carry = a[i] >> 29;
        a[i] &= bottom29Bits;

        i++;
        a[i] = tmp[i + 9] >> 1;
        a[i] += carry;
        carry = a[i] >> 28;
        a[i] &= bottom28Bits;
    }
    a[8] = tmp[17];
    a[8] += carry;
    carry = a[8] >> 29;
    a[8] &= bottom29Bits;
    p256ReduceCarry(a, carry);
}

// c = a * b
void p256Mul(uint *c, uint *a, uint *b)
{

    uint64 *tmp = new uint64[17];

    tmp[0] = uint64(a[0]) * uint64(b[0]);
    tmp[1] = uint64(a[0]) * (uint64(b[1]) << 0) +
             uint64(a[1]) * (uint64(b[0]) << 0);
    tmp[2] = uint64(a[0]) * (uint64(b[2]) << 0) +
             uint64(a[1]) * (uint64(b[1]) << 1) +
             uint64(a[2]) * (uint64(b[0]) << 0);
    tmp[3] = uint64(a[0]) * (uint64(b[3]) << 0) +
             uint64(a[1]) * (uint64(b[2]) << 0) +
             uint64(a[2]) * (uint64(b[1]) << 0) +
             uint64(a[3]) * (uint64(b[0]) << 0);
    tmp[4] = uint64(a[0]) * (uint64(b[4]) << 0) +
             uint64(a[1]) * (uint64(b[3]) << 1) +
             uint64(a[2]) * (uint64(b[2]) << 0) +
             uint64(a[3]) * (uint64(b[1]) << 1) +
             uint64(a[4]) * (uint64(b[0]) << 0);
    tmp[5] = uint64(a[0]) * (uint64(b[5]) << 0) +
             uint64(a[1]) * (uint64(b[4]) << 0) +
             uint64(a[2]) * (uint64(b[3]) << 0) +
             uint64(a[3]) * (uint64(b[2]) << 0) +
             uint64(a[4]) * (uint64(b[1]) << 0) +
             uint64(a[5]) * (uint64(b[0]) << 0);
    tmp[6] = uint64(a[0]) * (uint64(b[6]) << 0) +
             uint64(a[1]) * (uint64(b[5]) << 1) +
             uint64(a[2]) * (uint64(b[4]) << 0) +
             uint64(a[3]) * (uint64(b[3]) << 1) +
             uint64(a[4]) * (uint64(b[2]) << 0) +
             uint64(a[5]) * (uint64(b[1]) << 1) +
             uint64(a[6]) * (uint64(b[0]) << 0);
    tmp[7] = uint64(a[0]) * (uint64(b[7]) << 0) +
             uint64(a[1]) * (uint64(b[6]) << 0) +
             uint64(a[2]) * (uint64(b[5]) << 0) +
             uint64(a[3]) * (uint64(b[4]) << 0) +
             uint64(a[4]) * (uint64(b[3]) << 0) +
             uint64(a[5]) * (uint64(b[2]) << 0) +
             uint64(a[6]) * (uint64(b[1]) << 0) +
             uint64(a[7]) * (uint64(b[0]) << 0);
    // tmp[8] has the greatest value but doesn't overflow. See logic in
    // p256Square.
    tmp[8] = uint64(a[0]) * (uint64(b[8]) << 0) +
             uint64(a[1]) * (uint64(b[7]) << 1) +
             uint64(a[2]) * (uint64(b[6]) << 0) +
             uint64(a[3]) * (uint64(b[5]) << 1) +
             uint64(a[4]) * (uint64(b[4]) << 0) +
             uint64(a[5]) * (uint64(b[3]) << 1) +
             uint64(a[6]) * (uint64(b[2]) << 0) +
             uint64(a[7]) * (uint64(b[1]) << 1) +
             uint64(a[8]) * (uint64(b[0]) << 0);
    tmp[9] = uint64(a[1]) * (uint64(b[8]) << 0) +
             uint64(a[2]) * (uint64(b[7]) << 0) +
             uint64(a[3]) * (uint64(b[6]) << 0) +
             uint64(a[4]) * (uint64(b[5]) << 0) +
             uint64(a[5]) * (uint64(b[4]) << 0) +
             uint64(a[6]) * (uint64(b[3]) << 0) +
             uint64(a[7]) * (uint64(b[2]) << 0) +
             uint64(a[8]) * (uint64(b[1]) << 0);
    tmp[10] = uint64(a[2]) * (uint64(b[8]) << 0) +
              uint64(a[3]) * (uint64(b[7]) << 1) +
              uint64(a[4]) * (uint64(b[6]) << 0) +
              uint64(a[5]) * (uint64(b[5]) << 1) +
              uint64(a[6]) * (uint64(b[4]) << 0) +
              uint64(a[7]) * (uint64(b[3]) << 1) +
              uint64(a[8]) * (uint64(b[2]) << 0);
    tmp[11] = uint64(a[3]) * (uint64(b[8]) << 0) +
              uint64(a[4]) * (uint64(b[7]) << 0) +
              uint64(a[5]) * (uint64(b[6]) << 0) +
              uint64(a[6]) * (uint64(b[5]) << 0) +
              uint64(a[7]) * (uint64(b[4]) << 0) +
              uint64(a[8]) * (uint64(b[3]) << 0);
    tmp[12] = uint64(a[4]) * (uint64(b[8]) << 0) +
              uint64(a[5]) * (uint64(b[7]) << 1) +
              uint64(a[6]) * (uint64(b[6]) << 0) +
              uint64(a[7]) * (uint64(b[5]) << 1) +
              uint64(a[8]) * (uint64(b[4]) << 0);
    tmp[13] = uint64(a[5]) * (uint64(b[8]) << 0) +
              uint64(a[6]) * (uint64(b[7]) << 0) +
              uint64(a[7]) * (uint64(b[6]) << 0) +
              uint64(a[8]) * (uint64(b[5]) << 0);
    tmp[14] = uint64(a[6]) * (uint64(b[8]) << 0) +
              uint64(a[7]) * (uint64(b[7]) << 1) +
              uint64(a[8]) * (uint64(b[6]) << 0);
    tmp[15] = uint64(a[7]) * (uint64(b[8]) << 0) +
              uint64(a[8]) * (uint64(b[7]) << 0);
    tmp[16] = uint64(a[8]) * (uint64(b[8]) << 0);
    p256ReduceDegree(c, tmp);
}

// c = a + b
void p256Add(uint *c, uint *a, uint *b)
{
    uint carry = 0;
    for (int i = 0;; i++)
    {
        c[i] = a[i] + b[i];
        c[i] += carry;
        carry = c[i] >> 29;
        c[i] &= bottom29Bits;
        i++;
        if (i == 9)
        {
            break;
        }
        c[i] = a[i] + b[i];
        c[i] += carry;
        carry = c[i] >> 28;
        c[i] &= bottom28Bits;
    }
    p256ReduceCarry(c, carry);
}

void p256Square(uint *b, uint *a)
{
    unsigned long *tmp = new uint64[17];
    tmp[0] = uint64(a[0]) * uint64(a[0]);
    tmp[1] = uint64(a[0]) * (uint64(a[1]) << 1);
    tmp[2] = uint64(a[0]) * (uint64(a[2]) << 1) +
             uint64(a[1]) * (uint64(a[1]) << 1);
    tmp[3] = uint64(a[0]) * (uint64(a[3]) << 1) +
             uint64(a[1]) * (uint64(a[2]) << 1);
    tmp[4] = uint64(a[0]) * (uint64(a[4]) << 1) +
             uint64(a[1]) * (uint64(a[3]) << 2) +
             uint64(a[2]) * uint64(a[2]);
    tmp[5] = uint64(a[0]) * (uint64(a[5]) << 1) +
             uint64(a[1]) * (uint64(a[4]) << 1) +
             uint64(a[2]) * (uint64(a[3]) << 1);
    tmp[6] = uint64(a[0]) * (uint64(a[6]) << 1) +
             uint64(a[1]) * (uint64(a[5]) << 2) +
             uint64(a[2]) * (uint64(a[4]) << 1) +
             uint64(a[3]) * (uint64(a[3]) << 1);
    tmp[7] = uint64(a[0]) * (uint64(a[7]) << 1) +
             uint64(a[1]) * (uint64(a[6]) << 1) +
             uint64(a[2]) * (uint64(a[5]) << 1) +
             uint64(a[3]) * (uint64(a[4]) << 1);
    // tmp[8] has the greatest value of 2**61 + 2**60 + 2**61 + 2**60 + 2**60,
    // which is < 2**64 as required.
    tmp[8] = uint64(a[0]) * (uint64(a[8]) << 1) +
             uint64(a[1]) * (uint64(a[7]) << 2) +
             uint64(a[2]) * (uint64(a[6]) << 1) +
             uint64(a[3]) * (uint64(a[5]) << 2) +
             uint64(a[4]) * uint64(a[4]);
    tmp[9] = uint64(a[1]) * (uint64(a[8]) << 1) +
             uint64(a[2]) * (uint64(a[7]) << 1) +
             uint64(a[3]) * (uint64(a[6]) << 1) +
             uint64(a[4]) * (uint64(a[5]) << 1);
    tmp[10] = uint64(a[2]) * (uint64(a[8]) << 1) +
              uint64(a[3]) * (uint64(a[7]) << 2) +
              uint64(a[4]) * (uint64(a[6]) << 1) +
              uint64(a[5]) * (uint64(a[5]) << 1);
    tmp[11] = uint64(a[3]) * (uint64(a[8]) << 1) +
              uint64(a[4]) * (uint64(a[7]) << 1) +
              uint64(a[5]) * (uint64(a[6]) << 1);
    tmp[12] = uint64(a[4]) * (uint64(a[8]) << 1) +
              uint64(a[5]) * (uint64(a[7]) << 2) +
              uint64(a[6]) * uint64(a[6]);
    tmp[13] = uint64(a[5]) * (uint64(a[8]) << 1) +
              uint64(a[6]) * (uint64(a[7]) << 1);
    tmp[14] = uint64(a[6]) * (uint64(a[8]) << 1) +
              uint64(a[7]) * (uint64(a[7]) << 1);
    tmp[15] = uint64(a[7]) * (uint64(a[8]) << 1);
    tmp[16] = uint64(a[8]) * uint64(a[8]);

    p256ReduceDegree(b, tmp);
}

void p256FromBig(uint *result, mpz_t a, Curve &cur)
{

    mpz_t x;
    mpz_init(x);
    mpz_mul_2exp(x, a, 257); // lsh
    mpz_mod(x, x, cur.p_prime);

    for (int i = 0; i < 9; i++)
    {
        unsigned long *a;
        int al = gmpToLong(a, x, 0);
        if (al > 0)
        {
            result[i] = a[0] & bottom29Bits;
        }
        else
        {
            result[i] = 0;
        }

        mpz_tdiv_q_2exp(x, x, 29); // rsh
        i++;
        if (i == 9)
        {
            break;
        }
        al = gmpToLong(a, x, 0);

        if (al > 0)
        {
            result[i] = a[0] & bottom28Bits;
        }
        else
        {
            result[i] = 0;
        }
        mpz_tdiv_q_2exp(x, x, 28);
    }
}

void p256FieldElement(uint *a)
{
    for (int i = 0; i < 9; i++)
    {
        a[i] = 0;
    }
}

// X = r * R mod P
// r = X * R' mod P
void p256ToBig(uint *x, mpz_t r, Curve &cur)
{

    mpz_t tm;
    mpz_init(r);
    mpz_init(tm);
    mpz_set_d(r, x[8]);
    for (int i = 7; i >= 0; i--)
    {
        if ((i & 1) == 0)
        {
            mpz_mul_2exp(r, r, 29);
        }
        else
        {
            mpz_mul_2exp(r, r, 28);
        }
        mpz_set_d(tm, x[i]);
        mpz_add(r, r, tm);
    }
    mpz_mul(r, r, cur.u_sqrt);
    mpz_mod(r, r, cur.p_prime);
}

bool isEven(mpz_t a)
{
    return (mpz_odd_p(a) == 0); /* true iff a is even (works for -ve a as well) */
}

void pow2bi(mpz_t rs, long long exp)
{
    mpz_init_set_ui(rs, 1);
    mpz_mul_2exp(rs, rs, exp);
}

void modSqrt(mpz_t rs, mpz_t x, mpz_t p)
{

    if (mpz_cmp(x, p) == 1)
    {
        mpz_mod(x, x, p);
    }

    mpz_t tmp;
    mpz_init(tmp);

    if (mpz_jacobi(x, p) != 1)
    {

        return; // empty list; no solutions
    }

    unsigned long hi = mpz_getlimbn(p, 0);

    if (hi % 4 == 3)
    {
        // Check whether p is 3 mod 4, and if so, use the faster algorithm.
        /* code */

        mpz_add_ui(tmp, p, uint64(1));
        mpz_div_ui(tmp, tmp, uint64(4));
        mpz_powm(rs, x, tmp, p);

        mpz_sub(tmp, p, rs);
 
        // resultis r,tmp;
        return;
    }

    mpz_t q, s, z;
    mpz_init(q);
    mpz_init_set_ui(s, 0);

    // Tonelli-Shanks step 1: Factor prime - 1 of the form q * 2 ^ s(with Q odd)
    mpz_sub_ui(q, p, uint64(1));

    while (isEven(q))
    {
        mpz_add_ui(s, s, 1);
        mpz_div_ui(q, q, 2);
    }

    // step 2: Select a z which is a quadratic non resudue modulo prime
    mpz_init_set_ui(z, 1);

    while (mpz_jacobi(z, p) != -1)
    {
        mpz_add_ui(z, z, 1);
    }

    /* step 3 */
    mpz_t m, c, t ,e, b;
    
    mpz_t q12; // q+1 /2
    mpz_init(q12);
    mpz_init(c);
    mpz_init(t);
    mpz_init(e);
    mpz_init(b);
    mpz_init_set(m, s); // m = s;

    mpz_powm(c, z, q, p);
    mpz_powm(t, x, q, p);
    mpz_add_ui(q12, q, 1);
    mpz_div_ui(q12, q12, 2);
    mpz_powm(rs, x, q12, p);
    // c = modPower(z, q, prime);
    // t = modPower(a, q, prime);
    // R = modPower(a, (q + 1) / 2, prime);

    // Step 4: Search for a solution
    while (true)
    {
        uint64 ui_t = mpz_get_ui(t);
        if (ui_t == 1)
        {
            break;
        }

        if (ui_t == 0)
        {
            mpz_set_ui(rs, 0);
            break;
        }
        // Find the lowest i such that t ^ (2 ^ i) = 1 (mod prime)
        uint64 i = 0;
        uint64 ui_m = mpz_get_ui(m);
        mpz_t tmp_mr;
        mpz_init_set_ui(e, 1);
        mpz_init(tmp_mr);
        for (i = 0; i < ui_m; i++)
        {
            mpz_powm(tmp_mr, t, e, p);
            if (mpz_get_ui(tmp_mr) == 1)
                break;
            mpz_mul_ui(e, e, 2); /* e = 2^i */
        }

        ui_m = mpz_get_ui(m);
        // Update next value to iterate
        long long temp = ui_m - i - 1;

        pow2bi(tmp_mr, temp);

        mpz_powm(b, c, tmp_mr, p); /* b = c^(2^(m-i-1)) mod prime */
                                   // R = (R * b) % prime;
        mpz_mul(rs, rs, b);
        mpz_mod(rs, rs, p);
        //	t = (t * b * b) % prime;

        mpz_mul(t, t, b);
        mpz_mul(t, t, b);
        mpz_mod(t, t, p);
        // c = (b * b) % prime;
        mpz_mul(c, b, b);
        mpz_mod(c, c, p);
        // m = i;           /*  i < m, so m decreases each time round */
        mpz_set_ui(m, i);
    }
}

//
point decompress(std::vector<unsigned char> buf)
{
    mpz_t pub;
    unsigned char lastb = buf[0];
    std::vector<unsigned char> c;
    for (int i = 1; i < 33; i++)
    {
        c.push_back(buf[i]);
    }
    byteToGmp(pub, c);
    Curve cu;
    uint aa[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint xx[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint xx3[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    p256FromBig(xx, pub, cu);
    p256Square(xx3, xx);   // x3 = x ^ 2
    p256Mul(xx3, xx3, xx); // x3 = x ^ 2 * x

    uint curev_a[9], curve_b[9];
    p256FieldElement(curev_a);
    p256FieldElement(curve_b);
    p256FromBig(curev_a, cu.a_coeffi, cu);
    p256FromBig(curve_b, cu.b_coeffi, cu);

    p256Mul(aa, curev_a, xx); // a = a * x
    p256Add(xx3, xx3, aa);
    p256Add(xx3, xx3, curve_b);

    mpz_t y2, y;

    mpz_init(y);
    p256ToBig(xx3, y2, cu);
    modSqrt(y, y2, cu.p_prime); // y := new(big.Int).ModSqrt(y2, sm2P256.P)

    uint yp = mpz_tstbit(y, 0); // if getLastBit(y)!= uint(a[0]) {
    if (yp != uint(buf[0]))
    {
        mpz_sub(y, cu.p_prime, y); // y.Sub(sm2P256.P, y)
    }
 

    point result;
    mpz_init_set(result.x,pub);
    mpz_init_set(result.y,y);
    return result;
}
