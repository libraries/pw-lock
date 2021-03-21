/* The file perform EOS wallet signature verification.
 *
 * The EOS scatter wallet signature is generated by
 * scatter.getArbitrarySignature() API, refer to
 * https://github.com/GetScatter/scatter-js/blob/55511b68d53efc9cb1aeed69336a078b5d941aa6/packages/core/src/models/WalletInterface.js#L16
 *
 */
#include "libsig.h"
#include "pw_k1_helper.h"

#define SHA256_CTX sha256_context
#define EOS_SIGNATURE_SIZE 65

/**
 * EOS scatter wallet requires each word of message to be signed no more
 * than 12 characters, so we need to split hex transaction message digest,
 * inerset blank character every 12 character, refer
 * https://get-scatter.com/developers/api/requestarbitrarysignature
 *
 * @param source transaction message digest, its size is 32 bytes.
 * @param dest message digest hex after blank added, its length is 64 + 5 =
 * 69 char
 *
 */
int split_hex_hash(unsigned char* source, unsigned char* dest) {
  int i;
  for (i = 0; i < HASH_SIZE; i++) {
    if (i > 0 && i % 6 == 0) {
      *dest = ' ';
      dest++;
    }
    dest += sprintf((char*)dest, "%02x", source[i]);
  }
  return 0;
}

/**
 * @param message transaction message digest for signature verification, size is
 * 32 bytes
 * @param eth_address last 20 bytes keccak256 hash of pubkey, used to shield the
 * real pubkey. size is 20 bytes
 * @param lock_bytes signature signed by eos wallet, size is 65 bytes.
 *
 */
int validate_eos(unsigned char* message, unsigned char* eth_address,
                 unsigned char* lock_bytes, uint64_t lock_bytes_size) {
  if (lock_bytes_size != EOS_SIGNATURE_SIZE) {
    return ERROR_WITNESS_SIZE;
  }

  int split_message_len = HASH_SIZE * 2 + 5;
  unsigned char splited_message[split_message_len];
  /* split message to words length <= 12 */
  split_hex_hash(message, splited_message);

  SHA256_CTX sha256_ctx;
  sha256_init(&sha256_ctx);
  sha256_update(&sha256_ctx, splited_message, split_message_len);
  sha256_final(&sha256_ctx, message);

  return verify_signature(message, lock_bytes, eth_address);
}