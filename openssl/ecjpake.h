/*
 *
 *    Copyright (c) 2017 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef HEADER_ECJPAKE_H
# define HEADER_ECJPAKE_H

# include <openssl/opensslconf.h>

# ifdef  __cplusplus
extern "C" {
# endif

# include <openssl/ec.h>
# include <openssl/bn.h>
# include <openssl/sha.h>

#define ECJPAKE_API
#if defined(ECJPAKE_SHARED)
    #if defined(ECJPAKE_SHARED_EXPORT)
        #if defined(_MSC_VER)
            #define ECJPAKE_API __declspec(dllexport)
        #else
            #define ECJPAKE_API __attribute__((visibility("default")))
        #endif
    #else
        #if defined(_MSC_VER)
            #define ECJPAKE_API __declspec(dllimport)
        #else
            #define ECJPAKE_API
        #endif
    #endif
#endif

typedef struct ECJPAKE_CTX ECJPAKE_CTX;

typedef struct {
    EC_POINT *Gr;            /* G * r (r random) */
    BIGNUM *b;               /* b = r - x * h,
				h = hash(G, G * r, G * x, name) */
} ECJPAKE_ZKP;

typedef struct {
    EC_POINT *Gx;            /* G * x in step 1,
				G * ((xa + xc + xd) * xb * s) in step 2 */
    ECJPAKE_ZKP zkpx;        /* ZKP(x) or ZKP(xb * s) */
} ECJPAKE_STEP_PART;

typedef struct {
    ECJPAKE_STEP_PART p1;    /* {G * x3, ZKP(x3)} or {G * x1, ZKP(x1)} */
    ECJPAKE_STEP_PART p2;    /* {G * x4, ZKP(x4)} or {G * x2, ZKP(x2)} */
} ECJPAKE_STEP1;

typedef ECJPAKE_STEP_PART ECJPAKE_STEP2;

typedef struct {
    unsigned char hhk[SHA256_DIGEST_LENGTH];
} ECJPAKE_STEP3A;

typedef struct {
    unsigned char hk[SHA256_DIGEST_LENGTH];
} ECJPAKE_STEP3B;

/*
 * Defines pointer to the function that calculates SHA256 hash of elliptic curve
 * point. ECJPAKE implements it's own point hash function.
 * Use ECJPAKE_Set_HashECPoint() to provide alternative implementation of the
 * point hash.
 */
typedef int(*ECJPAKE_HASHPOINT_FUNC_PTR)(ECJPAKE_CTX *, SHA256_CTX *,
                                          const EC_POINT *);

/*
 * Sets the function that will be used to hash elliptic curve point.
 * If this function is not called the ecjpake uses it's own (default)
 * implementation of the point hash function.
 */
ECJPAKE_API void ECJPAKE_Set_HashECPoint(ECJPAKE_HASHPOINT_FUNC_PTR hashpoint_custom);

/* Initializes ECJPAKE_CTX with protocol parameters */
ECJPAKE_API ECJPAKE_CTX *ECJPAKE_CTX_new(const EC_GROUP *group, const BIGNUM *secret,
                             const unsigned char *local_id_num,
                             const size_t local_id_len,
                             const unsigned char *peer_id_num,
                             const size_t peer_id_len);

/* Releases ECJPAKE_CTX */
ECJPAKE_API void ECJPAKE_CTX_free(ECJPAKE_CTX *ctx);

/*
 * Functions to initialize, generate, process, and release ECJPAKE_STEP1 data.
 * Note that ECJPAKE_STEP1 can be used multiple times before release
 * without another init.
 */
ECJPAKE_API int ECJPAKE_STEP1_init(ECJPAKE_STEP1 *s1, const ECJPAKE_CTX *ctx);
ECJPAKE_API int ECJPAKE_STEP1_generate(ECJPAKE_STEP1 *send, ECJPAKE_CTX *ctx);
ECJPAKE_API int ECJPAKE_STEP1_process(ECJPAKE_CTX *ctx, const ECJPAKE_STEP1 *received);
ECJPAKE_API void ECJPAKE_STEP1_release(ECJPAKE_STEP1 *s1);

/*
 * Functions to initialize, generate, process, and release ECJPAKE_STEP2 data.
 * Note that ECJPAKE_STEP2 can be used multiple times before release
 * without another init.
 */
ECJPAKE_API int ECJPAKE_STEP2_init(ECJPAKE_STEP2 *s2, const ECJPAKE_CTX *ctx);
ECJPAKE_API int ECJPAKE_STEP2_generate(ECJPAKE_STEP2 *send, ECJPAKE_CTX *ctx);
ECJPAKE_API int ECJPAKE_STEP2_process(ECJPAKE_CTX *ctx, const ECJPAKE_STEP2 *received);
ECJPAKE_API void ECJPAKE_STEP2_release(ECJPAKE_STEP2 *s2);

/*
 * Optionally verify the shared key. If the shared secrets do not
 * match, the two ends will disagree about the shared key, but
 * otherwise the protocol will succeed.
 */
ECJPAKE_API void ECJPAKE_STEP3A_init(ECJPAKE_STEP3A *s3a);
ECJPAKE_API int ECJPAKE_STEP3A_generate(ECJPAKE_STEP3A *send, ECJPAKE_CTX *ctx);
ECJPAKE_API int ECJPAKE_STEP3A_process(ECJPAKE_CTX *ctx, const ECJPAKE_STEP3A *received);
ECJPAKE_API void ECJPAKE_STEP3A_release(ECJPAKE_STEP3A *s3a);

ECJPAKE_API void ECJPAKE_STEP3B_init(ECJPAKE_STEP3B *s3b);
ECJPAKE_API int ECJPAKE_STEP3B_generate(ECJPAKE_STEP3B *send, ECJPAKE_CTX *ctx);
ECJPAKE_API int ECJPAKE_STEP3B_process(ECJPAKE_CTX *ctx, const ECJPAKE_STEP3B *received);
ECJPAKE_API void ECJPAKE_STEP3B_release(ECJPAKE_STEP3B *s3b);

/*
 * Returns shared secret value. The value belongs to the library and will be
 * released when ctx is released, and will change when a new handshake is
 * performed.
 */
ECJPAKE_API const unsigned char *ECJPAKE_get_shared_key(const ECJPAKE_CTX *ctx);

/* Returns elliptic curve group used in the current ECJPAKE handshake. */
ECJPAKE_API const EC_GROUP *ECJPAKE_get_ecGroup(const ECJPAKE_CTX *ctx);

/* BEGIN ERROR CODES */
/*
 * The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */
ECJPAKE_API void ERR_load_ECJPAKE_strings(void);

/* Error codes for the ECJPAKE functions. */

/* Function codes. */
# define ECJPAKE_F_COMPUTE_KEY                            100
# define ECJPAKE_F_EC_POINT_IS_LEGAL                      101
# define ECJPAKE_F_ECJPAKE_CTX_NEW                        102
# define ECJPAKE_F_ECJPAKE_STEP1_GENERATE                 103
# define ECJPAKE_F_ECJPAKE_STEP1_PROCESS                  104
# define ECJPAKE_F_ECJPAKE_STEP2_GENERATE                 105
# define ECJPAKE_F_ECJPAKE_STEP2_PROCESS                  106
# define ECJPAKE_F_ECJPAKE_STEP3A_PROCESS                 107
# define ECJPAKE_F_ECJPAKE_STEP3B_PROCESS                 108
# define ECJPAKE_F_GENERATE_ZKP                           109
# define ECJPAKE_F_GENRAND                                110
# define ECJPAKE_F_STEP_PART_GENERATE                     111
# define ECJPAKE_F_STEP_PART_INIT                         112
# define ECJPAKE_F_VERIFY_ZKP                             113
# define ECJPAKE_F_ZKP_HASH                               114

/* Reason codes. */
# define ECJPAKE_R_G_IS_NOT_LEGAL                         100
# define ECJPAKE_R_G_TO_THE_X3_IS_NOT_LEGAL               101
# define ECJPAKE_R_G_TO_THE_X4_IS_NOT_LEGAL               102
# define ECJPAKE_R_HASH_OF_HASH_OF_KEY_MISMATCH           103
# define ECJPAKE_R_HASH_OF_KEY_MISMATCH                   104
# define ECJPAKE_R_VERIFY_X3_FAILED                       105
# define ECJPAKE_R_VERIFY_X4_FAILED                       106
# define ECJPAKE_R_VERIFY_X4S_FAILED                      107
# define ECJPAKE_R_ZKP_VERIFY_FAILED                      108

#ifndef ERR_LIB_ECJPAKE
# define ERR_LIB_ECJPAKE         50
#endif

#ifndef ECJPAKEerr
# ifndef OPENSSL_IS_BORINGSSL
#  define ECJPAKEerr(f,r) ERR_PUT_error(ERR_LIB_ECJPAKE,(f),(r),__FILE__,__LINE__)
# else
#  define ECJPAKEerr(f,r) ERR_put_error(ERR_LIB_ECJPAKE,(f),(r),__FILE__,__LINE__)
# endif
#endif

# ifdef  __cplusplus
}
# endif
#endif
