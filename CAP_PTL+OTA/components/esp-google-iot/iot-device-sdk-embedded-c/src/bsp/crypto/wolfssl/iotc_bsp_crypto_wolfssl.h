/* Copyright 2019-2020 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C.
 * It is licensed under the BSD 3-Clause license; you may not use this file
 * except in compliance with the License.
 *
 * You may obtain a copy of the License at:
 *  https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __IOTC_BSP_CRYPTO_WOLFSSL_H__
#define __IOTC_BSP_CRYPTO_WOLFSSL_H__

/**
 * @file iotc_bsp_crypto_wolfssl.h
 * @brief Used for sharing the same WC_RNG instance with all modules
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <cyassl/ctaocrypt/random.h>
extern WC_RNG wolfcrypt_rng;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_CRYPTO_WOLFSSL_H__ */