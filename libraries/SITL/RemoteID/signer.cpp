#include "hash/hashing.h"
#include "ec/ec.h"
#include "bigint/bi.h"

#include "auth/keysys.h"
#include "auth/signer.h"

// private signer's key
ecpoint_fp Us;

/**
 * Sign a message, method 1
 * @param  msg, the message to sign in bytes
 * @param  len, the length of message in bytes
 * @param  res, the result signature
 */
void sign1(fp_t res, byte* msg, int len) {
    bigint_t h;
    H2(h, msg, len);        // h = H2(m)
    ecpoint_fp s;
    ecfp_mul(&s, &Us, h);   // s = h*Us = h*s*P
    bi_copy(res, s.x);
}

/**
 * Sign a message, method 2
 * @param  msg, the message to sign in bytes
 * @param  len, the length of message in bytes
 * @param  res, the result signature
 */
void sign2(fp_t res, byte* msg, int len) {
    bigint_t h;
    H2(h, msg, len);        // h = H2(m)
    ecpoint_fp s;
    ecfp_mul(&s, &Us, h);   // s = h*Us = h/(d+s) * P
    bi_copy(res, s.x);
}
