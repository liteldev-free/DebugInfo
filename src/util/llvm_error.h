#pragma once

#include <llvm/Support/Error.h>

namespace di {

class LLVMException : public RuntimeException<LLVMException> {
public:
    explicit LLVMException(
        std::string_view error_message_di   = "",
        std::string_view error_message_llvm = ""
    )
    : RuntimeException("There were some problems when calling LLVM.") {
        if (!error_message_di.empty())
            add_context("error_message_di", error_message_di);
        if (!error_message_llvm.empty())
            add_context("error_message_llvm", error_message_llvm);
    }

    constexpr std::string category() const { return "exception.llvm"; }
};

DI_INLINE void check_llvm_result(Error err, std::string_view msg = "") {
    if (err) {
        std::string        err_detail;
        raw_string_ostream os(err_detail);
        os << err;
        throw LLVMException(msg, err_detail);
    }
}

template <typename T>
DI_INLINE T
check_llvm_result(Expected<T> val_or_err, std::string_view msg = "") {
    if (val_or_err) return std::move(*val_or_err);
    else {
        std::string        err_detail;
        raw_string_ostream os(err_detail);
        auto               err = val_or_err.takeError();
        os << err;
        throw LLVMException(msg, err_detail);
    }
}

template <typename T>
DI_INLINE T&
check_llvm_result(Expected<T&> val_or_err, std::string_view msg = "") {
    if (val_or_err) return *val_or_err;
    else {
        std::string        err_detail;
        raw_string_ostream os(err_detail);
        auto               err = val_or_err.takeError();
        os << err;
        throw LLVMException(msg, err_detail);
    }
}

} // namespace di