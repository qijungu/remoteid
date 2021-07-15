#include "hash/hashing.h"
#include "ec/ec.h"
#include "bigint/bi.h"
#include "pbc/pbc.h"
#include "fp/fp.h"
#include "fp/fp12.h"

#include "auth/keysys.h"
#include "auth/verifier.h"

/**
 * Verify a message, method 1
 * @param  sig, the signature to verify
 * @param  id, the remote id in bytes
 * @param  len_id, the length of id in bytes
 * @param  msg, the message to verify in bytes
 * @param  len_msg, the length of message in bytes
 */
int verify1(fp_t sig, const byte* id, int len_id, const byte* msg, int len_msg) {

    ecpoint_fp s;
    int v = ecfp_get_point(&s, sig);
    // cannot recover a point from sig (x coordinate), so not a valid signature
    if (v != 0) return v;

    // hash id and message
    bigint_t d, h;
    H1(d, id, len_id);     // d = H1(id)
    H2(h, msg, len_msg);   // h = H2(m)

    // right side
    fp12_t r;
    //ecpoint_fp t0, t1;
    //ecfp_mul(&t0, &authparam.P, d);          // t = d*P
    //ecfp_mul(&t1, &t0, h);                   // t = d*h*P
    //pbc_map_opt_ate(r, &t1, &authparam.Kq);  // r = E(d*h*P, K) = E(d*h*P, s*Q)
    word_t t0[2*BI_WORDS];
    bi_multiply(t0, d, h);
    bigint_t t1;
    fp_rdc_2l_std(t0, EC_PARAM_N, EC_PARAM_MU_N);
    bi_copy(t1, t0);
    fp12_exp_cyclotomic(r, authparam.ePK, t1); // r = E(P,K)^(d*h)
    //fp12_exp(r, authparam.ePK, t1);

    // left side
    fp12_t l;
    pbc_map_opt_ate(l, &s, &authparam.Q);    // l = E(s, Q) = E(d*h*s*P, Q)

    // compare
    v = fp12_compare(l, r);

    // valid
    if (v == 0) return 0;

    // not valid, then try the negative signature point
    // negate signature
    //ecfp_neg_affine(&s);
    // left side
    //pbc_map_opt_ate(l, &s, &authparam.Q);

    fp12_t l1;
    fp12_inv(l1, l);

    // compare
    v = fp12_compare(l1, r);

    return v;

    // proof: E(P,Q)^(dhs)
    //fp12_t r1, r2, r3, r4;
    //pbc_map_opt_ate(r1, &authparam.P, &authparam.Q);
    //fp12_exp_cyclotomic(r2, r1, d);
    //fp12_exp_cyclotomic(r3, r2, h);
    //fp12_exp_cyclotomic(r4, r3, msk);
}

/**
 * Verify a message, method 2
 * @param  sig, the signature to verify
 * @param  id, the remote id in bytes
 * @param  len_id, the length of id in bytes
 * @param  msg, the message to verify in bytes
 * @param  len_msg, the length of message in bytes
 */
int verify2(fp_t sig, byte* id, int len_id, byte* msg, int len_msg) {

    ecpoint_fp s;
    int v = ecfp_get_point(&s, sig);
    // cannot recover a point from sig (x coordinate), so not a valid signature
    if (v != 0) return v;

    // hash id and message
    bigint_t d, h;
    H1(d, id, len_id);     // d = H1(id)
    H2(h, msg, len_msg);   // h = H2(m)

    // right side
    fp12_t r;
    //ecpoint_fp t0;
    //ecfp_mul(&t0, &authparam.P, h);             // t = h*P
    //pbc_map_opt_ate(r, &t0, &authparam.Q);      // r = E(h*P, Q)
    fp12_exp_cyclotomic(r, authparam.ePQ, h);     // r = E(P,Q)^h
    //fp12_exp(r, authparam.ePQ, h);

    // left side
    fp12_t l;
    ecpoint_fp2 t1, t2;
    ecfp2_mul(&t1, &authparam.Q, d);            // t = d*Q
    ecfp2_add_affine(&t2, &t1, &authparam.Kq);  // t = d*Q + K = d*Q + s*Q
    pbc_map_opt_ate(l, &s, &t2);                // l = E(s, (d+s)*Q) = E(h/(s+d)*P, (d+s)*Q)

    // compare
    v = fp12_compare(l, r);

    // valid
    if (v == 0) return 0;

    // not valid, then try the negative signature point
    // negate signature
    //ecfp_neg_affine(&s);
    // left side
    //pbc_map_opt_ate(l, &s, &t2);

    fp12_t l1;
    fp12_inv(l1, l);

    // compare
    v = fp12_compare(l1, r);

    return v;

    // proof: E(P,Q)^h
    //fp12_t r1, r2;
    //pbc_map_opt_ate(r1, &authparam.P, &authparam.Q);
    //fp12_exp_cyclotomic(r2, r1, h);
}

