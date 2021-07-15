#ifndef _AUTH_VERIFIER_H_
#define _AUTH_VERIFIER_H_

#include "generic/types.h"

int verify1(fp_t sig, const byte* id, int len_id, const byte* msg, int len_msg);
int verify2(fp_t sig, const byte* id, int len_id, const byte* msg, int len_msg);

#endif /* _AUTH_VERIFIER_H_ */
