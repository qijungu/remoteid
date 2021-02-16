#include <generic/config.h>
#include <generic/param.h>
#include <generic/types.h>
#include "ec/ec.h"
#include "bigint/bi.h"
#include "hash/hashing.h"
#include "pbc/pbc.h"
#include "fp/fp12.h"

#include "auth/keysys.h"

// P, generator of G1, is ECFP_GENERATOR
// Q, generator of G2, is ECFP2_GENERATOR

#if (BNCURVE == BN256 || BNCURVE == BN254)
	// master key (secret)
	const fp_t msk = {0x3e036826, 0xff686421, 0xa031d027, 0x6f6a59a2, 0x8f8f8151, 0x9ed62d29, 0x6356fbe5, 0x0ce8416f};

#elif (BNCURVE == BN158)
	// master key (secret)
	const fp_t msk = {0xcf12c778, 0x49e62142, 0xf452aea5, 0x5f84c5ef, 0x238c216e};

#endif

// private to the key server
ecpoint_fp Kp;   // Kp = msk * P
ecpoint_fp2 Kq;  // Kq = msk * Q

// published by the key server
auth_param_t authparam;

/**
 * setup authentication parameters system-wide
 */
void authparams_setup() {
    // keep secret
    ecfp_mul(&Kp, &ECFP_GENERATOR, msk);     // Kp = s*P
    ecfp2_mul(&Kq, &ECFP2_GENERATOR, msk);   // Kq = s*Q

    // publish
    bi_copy(authparam.p, PRIME_P);
    bi_copy(authparam.n, EC_PARAM_N);
    ecfp_copy(&authparam.P, &ECFP_GENERATOR);
    ecfp2_copy(&authparam.Q, &ECFP2_GENERATOR);
    ecfp2_copy(&authparam.Kq, &Kq);

    // publish
    pbc_map_opt_ate(authparam.ePK, &authparam.P, &authparam.Kq);
    pbc_map_opt_ate(authparam.ePQ, &authparam.P, &authparam.Q);

    // validate
    fp12_t r;
    fp12_exp(r, authparam.ePQ, msk);
    return;
}

/**
 * Assign private key on signer's id, method 1
 * @param  id,  signer's id in bytes
 * @param  len, length of id in bytes
 * @param  res, the private key
 */
void extract_key1(ecpoint_fp* res, byte* id, int len) {
    bigint_t d;
    H1(d, id, len);            // d = H1(id)
    ecfp_mul(res, &Kp, d);     // Us = d*s*P
}

/**
 * Assign private key on signer's id, method 2
 * @param  id,  signer's id in bytes
 * @param  len, length of id in bytes
 * @param  res, the private key
 */
void extract_key2(ecpoint_fp* res, byte* id, int len) {
    bigint_t c, d;
    H1(d, id, len);                     // d = H1(id)
    fp_add_std(c, d, msk, EC_PARAM_N);  // c = (d+s) % n
    fp_inv_n(d, c);                     // d = 1/(d+s) % n
    ecfp_mul(res, &authparam.P, d);     // Us = (1/(d+s) %n) * P
}
