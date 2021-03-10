#ifndef _AUTH_KEYSYS_H_
#define _AUTH_KEYSYS_H_

#include "generic/types.h"

// public system parameters
extern auth_param_t authparam;

void authparams_setup();
void extract_key1(ecpoint_fp* res, byte* id, int len);
void extract_key2(ecpoint_fp* res, byte* id, int len);

#endif /* _AUTH_KEYSYS_H_ */
