#pragma once

#include <llvm/Support/Error.h>

namespace di {

class BaseException {
public:
    virtual std::string what() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const BaseException& e) {
        os << e.what();
        return os;
    }

protected:
    BaseException() = default;
};

template <typename Derived>
class RuntimeException : public BaseException {
public:
    template <typename... Args>
    explicit RuntimeException(
        const std::format_string<Args...> fmt,
        Args&&... args
    )
    : m_reason(std::format(fmt, std::forward<Args>(args)...)),
      m_stacktrace(std::stacktrace::current()) {}

    constexpr std::string category() const {
        return static_cast<const Derived*>(this)->category();
    }

    std::string what() const final {
        // [exception.base] Reason...
        //
        // Context Information:
        //   val = 1
        //
        // StackTrace:
        //   #1 example_func(int) at /app/example.cpp:12
        //   #2 ...
        // ...

        std::string context_information;
        if (!m_context_information.empty()) {
            context_information = "\n\nContext Information: \n";
            for (const auto& [key, value] : m_context_information) {
                context_information += std::format("  {} = {}", key, value);
            }
        }

        std::string stacktrace = "\n\nStackTrace: \n";
        int         stack_idx  = -1; // ignore first entry.
        for (const auto& entry : m_stacktrace) {
            stack_idx++;
            if (stack_idx == 0) continue;
            auto func_name   = entry.description();
            auto source_file = entry.source_file();
            if (func_name.empty()) func_name = "<unknown>";
            if (source_file.empty()) source_file = "<\?\?>";
            stacktrace += std::format(
                "  #{} {} at {}:{}",
                stack_idx,
                func_name,
                source_file,
                entry.source_line()
            );
        }

        return std::format(
            "[{}] {}{}{}\n",
            category(),
            m_reason,
            context_information,
            stacktrace
        );
    }

protected:
    template <typename T>
    constexpr void add_context(const std::string& key, T&& value) {
        m_context_information[key] = std::format("{}", std::forward<T>(value));
    }

    template <std::integral T>
    constexpr void add_context_v_hex(const std::string& key, T value) {
        m_context_information[key] =
            std::format("{:#x}", std::forward<T>(value));
    }

private:
    std::string m_reason;

    std::unordered_map<std::string, std::string> m_context_information;
    std::stacktrace                              m_stacktrace;
};

class UnixException : public RuntimeException<UnixException> {
public:
    explicit UnixException()
    : RuntimeException<UnixException>("{}", std::strerror(errno)) {
        add_context("errno", errno);
    }

    constexpr std::string category() const { return "exception.unix"; }
};

class ConvertEnumException : public RuntimeException<ConvertEnumException> {
public:
    // TODO: compile-time reflection.
    // TODO: remove helper.

    template <Enumerate T>
    explicit ConvertEnumException(T enum_val)
    : RuntimeException("Unable to convert string to enumeration value because "
                       "input value is bad.") {
        add_context("enum_type", typeid(T).name());
        add_context("value", underlying_value(enum_val));
    }

    template <typename T>
    explicit ConvertEnumException(std::string_view enum_str, TypeOnly<T>)
    : RuntimeException("Unable to convert enumeration value to string because "
                       "input value is bad.") {
        add_context("enum_type", typeid(T).name());
        add_context("string", enum_str);
    }

    constexpr std::string category() const { return "exception.enumconvert"; }
};

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

constexpr void check_llvm_result(Error err, std::string_view msg = "") {
    if (err) {
        std::string        err_detail;
        raw_string_ostream os(err_detail);
        os << err;
        throw LLVMException(msg, err_detail);
    }
}

template <typename T>
constexpr T
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
constexpr T&
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
