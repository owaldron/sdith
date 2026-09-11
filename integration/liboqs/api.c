#include <stdlib.h>
#include <string.h>

#include <oqs/rand.h>

#include "sdith_signature.h"
#include "api.h"

// owaldron TODO fix this - it is temporary data specific to SDitH3-L1-gf2-short

//  Set these three values apropriately for your algorithm
#define CRYPTO_SECRETKEYBYTES 147
#define CRYPTO_PUBLICKEYBYTES 70
#define CRYPTO_BYTES 3721

// Change the algorithm name
#define CRYPTO_ALGNAME "SDiTH_CAT1_SHORT"
#define SIGNATURE_PARAMS CAT1_SHORT_PARAMETERS

// Zeroize secret scratch before free. The write goes through a volatile pointer
// so the compiler cannot drop it as a dead store ahead of free().
static void secure_zeroize(void* ptr, size_t len) {
  volatile unsigned char* p = (volatile unsigned char*)ptr;
  while (len--) *p++ = 0;
}

OQS_STATUS crypto_sign_keypair(uint8_t* public_key, uint8_t* secret_key) {
  // TODO:  is this necessary? It might get compiled-out 
  // safeguard
  if (CRYPTO_BYTES != sdith_signature_bytes(&SIGNATURE_PARAMS) ||
      CRYPTO_PUBLICKEYBYTES != sdith_public_key_bytes(&SIGNATURE_PARAMS) ||
      CRYPTO_SECRETKEYBYTES != sdith_secret_key_bytes(&SIGNATURE_PARAMS))
    return OQS_ERROR;

  // owaldron TODO: pre-compute this, add a runtime check, and move the space to the stack
  uint64_t entropy_bytes = sdith_keygen_entropy_bytes(&SIGNATURE_PARAMS);
  uint64_t tmp_bytes = sdith_keygen_tmp_bytes(&SIGNATURE_PARAMS);
  uint8_t* entropy = malloc(entropy_bytes);
  if (entropy == NULL) return OQS_ERROR;
  uint8_t* tmp_space = malloc(tmp_bytes);
  if (tmp_space == NULL) {
    free(entropy);
    return OQS_ERROR;
  }
  OQS_randombytes(entropy, entropy_bytes);
  sdith_keygen(&SIGNATURE_PARAMS, secret_key, public_key, entropy, tmp_space);
  // tmp_space holds the raw solution, entropy holds the sk_seed: wipe both.
  secure_zeroize(tmp_space, tmp_bytes);
  secure_zeroize(entropy, entropy_bytes);
  free(tmp_space);
  free(entropy);
  return OQS_SUCCESS;
}

OQS_STATUS crypto_sign_sign(
	uint8_t *signature,
	size_t *signature_len,
	const uint8_t *message,
	size_t message_len,
	const uint8_t *secret_key)
{
  uint64_t entropy_bytes = sdith_signature_entropy_bytes(&SIGNATURE_PARAMS);
  uint64_t tmp_bytes = sdith_signature_tmp_bytes(&SIGNATURE_PARAMS);
  uint8_t* entropy = malloc(entropy_bytes);
  if (entropy == NULL) return OQS_ERROR;
  uint8_t* tmp_space = malloc(tmp_bytes);
  if (tmp_space == NULL) {
    free(entropy);
    return OQS_ERROR;
  }
  OQS_randombytes(entropy, entropy_bytes);
  sdith_sign(&SIGNATURE_PARAMS, signature, message, message_len, secret_key, entropy, tmp_space);
  *signature_len = CRYPTO_BYTES;
  // tmp_space and entropy hold secret signing state: wipe both.
  secure_zeroize(tmp_space, tmp_bytes);
  secure_zeroize(entropy, entropy_bytes);
  free(tmp_space);
  free(entropy);
  return OQS_SUCCESS;
}

OQS_STATUS crypto_sign_verify(
	const uint8_t *signature,
	size_t signature_len,
	const uint8_t *message,
	size_t message_len,
	const uint8_t *public_key
) {
  if (signature_len != CRYPTO_BYTES) return OQS_ERROR;
  uint64_t tmp_bytes = sdith_verify_tmp_bytes(&SIGNATURE_PARAMS);
  uint8_t* tmp_space = malloc(tmp_bytes);
  if (tmp_space == NULL) return OQS_ERROR;
  uint8_t res = sdith_verify(
    &SIGNATURE_PARAMS,
    signature,
    message,
    message_len,
    public_key,
    tmp_space
  );
  free(tmp_space); // Touches only public data
  return res ? OQS_SUCCESS : OQS_ERROR;
}
