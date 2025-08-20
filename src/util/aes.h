#pragma once

#include <openssl/types.h>

namespace di {

class AES {
public:
    explicit AES(const std::span<uint8_t>& key);

    ~AES();

    AES(const AES&)            = delete;
    AES(AES&&)                 = delete;
    AES& operator=(const AES&) = delete;
    AES& operator=(AES&&)      = delete;

    std::vector<uint8_t> decrypt(const std::span<uint8_t>& ciphertext);

private:
    EVP_CIPHER_CTX* m_ctx;
};

class OpenSSLException : public RuntimeException<OpenSSLException> {
public:
    using RuntimeException::RuntimeException;

    constexpr std::string category() const { return "exception.ossl"; }
};

} // namespace di
