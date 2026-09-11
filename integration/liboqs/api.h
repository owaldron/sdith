#ifndef API_H
#define API_H

// This header wraps the SDiTH signature API for use in liboqs

#include "sdith_signature.h"
#include <stddef.h>

#include <oqs/common.h>

#ifndef OQS_INTERGRATION_NAMESPACE_PREFIX
#warning "OQS_INTERGRATION_NAMESPACE_PREFIX is not defined, default to no namespace"
#define OQS_INTERGRATION_NAMESPACE_PREFIX
#endif
#define OQS_INTERGRATION_CONCAT_(lhs, rhs) lhs##rhs
#define OQS_INTERGRATION_CONCAT(lhs, rhs) OQS_INTERGRATION_CONCAT_(lhs, rhs)
#define OQS_INTERGRATION_NAMESPACE(name) OQS_INTERGRATION_CONCAT(OQS_INTERGRATION_NAMESPACE_PREFIX, name)

#define crypto_sign_keypair OQS_INTERGRATION_NAMESPACE(crypto_sign_keypair)
#define crypto_sign_sign OQS_INTERGRATION_NAMESPACE(crypto_sign_sign)
#define crypto_sign_verify OQS_INTERGRATION_NAMESPACE(crypto_sign_verify)

/**
 * Generates the public and private key
 * @param public_key    [out] public key
 * @param secret_key    [out] secret key
 * @return              OQS_SUCCESS on success, OQS_FAILURE on failure
 */
EXPORT OQS_STATUS crypto_sign_keypair(uint8_t* public_key,
	                            uint8_t* secret_key);

/**
 * Signs a message with the private key
 * @param signature     [out] signature
 * @param signature_len [out] length of the signature
 * @param message       [in] message to sign
 * @param message_len   [in] length of the message
 * @param secret_key    [in] secret key
 */
EXPORT OQS_STATUS crypto_sign_sign(uint8_t *signature,
                             size_t *signature_len,
                             const uint8_t *message,
                             size_t message_len,
                             const uint8_t *secret_key);

/**
 * Verifies a signature with the public key
 * @param message       [in] message to verify
 * @param message_len   [in] length of the message
 * @param signature     [in] signature to verify
 * @param signature_len [in] length of the signature
 * @param public_key    [in] public key
 * @return              OQS_SUCCESS on successful verification, OQS_FAILURE on failure
 */
EXPORT OQS_STATUS crypto_sign_verify(const uint8_t *signature,
                                     size_t signature_len,
                                     const uint8_t *message,
                                     size_t message_len,
                                     const uint8_t *public_key);

#endif  // API_H
