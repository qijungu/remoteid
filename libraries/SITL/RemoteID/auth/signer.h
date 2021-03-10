#ifndef _AUTH_SIGNER_H_
#define _AUTH_SIGNER_H_

#include "generic/types.h"

extern ecpoint_fp Us;  // signer's private key
void sign1(fp_t res, byte* msg, int len);
void sign2(fp_t res, byte* msg, int len);

#endif /* _AUTH_SIGNER_H_ */
