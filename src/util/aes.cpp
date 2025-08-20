#include "aes.h"

#include <openssl/err.h>
#include <openssl/evp.h>

namespace di {

void handle_openssl_errors() {
    std::string   str;
    unsigned long code;
    while ((code = ERR_get_error()) != 0) {
        char buffer[256];
        ERR_error_string_n(code, buffer, sizeof(buffer));
        str += std::string(buffer) + "\n";
    }
    if (str.empty()) str = "Unknown error.";
    throw OpenSSLException("{}", str);
}

AES::AES(const std::span<uint8_t>& key) : m_ctx(nullptr) {
    m_ctx = EVP_CIPHER_CTX_new();
    if (!m_ctx) {
        handle_openssl_errors();
    }

    const EVP_CIPHER* cipher_type = nullptr;
    switch (key.size()) {
    case 16: // AES-128
        cipher_type = EVP_aes_128_ecb();
        break;
    case 24: // AES-192
        cipher_type = EVP_aes_192_ecb();
        break;
    case 32: // AES-256
        cipher_type = EVP_aes_256_ecb();
        break;
    default:
        EVP_CIPHER_CTX_free(m_ctx);
        throw OpenSSLException("Invalid key size. Must be 16/24/32 bytes.");
    }

    if (EVP_DecryptInit_ex(m_ctx, cipher_type, nullptr, key.data(), nullptr)
        != 1) {
        EVP_CIPHER_CTX_free(m_ctx);
        handle_openssl_errors();
    }
}

AES::~AES() {
    if (m_ctx) {
        EVP_CIPHER_CTX_free(m_ctx);
    }
}

std::vector<uint8_t> AES::decrypt(const std::span<uint8_t>& ciphertext) {
    if (ciphertext.empty()) {
        return {};
    }

    std::vector<uint8_t> plaintext(ciphertext.size());
    int                  len = 0;

    if (EVP_DecryptUpdate(
            m_ctx,
            plaintext.data(),
            &len,
            ciphertext.data(),
            ciphertext.size()
        )
        != 1) {
        handle_openssl_errors();
    }
    int plaintext_len = len;

    if (EVP_DecryptFinal_ex(m_ctx, plaintext.data() + len, &len) != 1) {
        handle_openssl_errors();
    }
    plaintext_len += len;

    plaintext.resize(plaintext_len);
    return plaintext;
}

} // namespace di
