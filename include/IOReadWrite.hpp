#pragma once
#include <format>
#include <concepts>
#include <string_view>

#include "IOStreams.hpp"


namespace io {
    namespace __impl {
        class TextOutputBase {
        public:
            const auto&
            put_char(this const auto& self, char c) {
                self.stream().Write((std::byte)c);
                return self;
            }

            const auto&
            put_str(this const auto& self, std::string_view strv) {
                self.stream().WriteSome({
                    (const std::byte*)strv.data(),
                    strv.size()
                });
                return self;
            }

            const auto&
            put_endl(this const auto& self) {
                return self.put_char('\n');
            }

            template<std::integral I>
            const auto&
            put_int(this const auto& self, I val, int base = 10) {
                char
                    lpcTemp[32];

                std::to_chars_result
                    result  = std::to_chars(
                                std::begin(lpcTemp),
                                std::end(lpcTemp),
                                val, base);
                *result.ptr = '\0'; 
                
                std::string_view
                    strvInt     = ((bool)result.ec) ? lpcTemp : "errint";
                return self.put_str(strvInt);
            }

            template<std::integral I>
            const auto&
            put_dec(this const auto& self, I val) {
                return self.put_int(val, 10);
            }

            template<std::integral I>
            const auto&
            put_hex(this auto& self, I val) {
                return self.put_int(val, 16);
            }

            template<std::integral I>
            const auto&
            put_oct(this auto& self, I val) {
                return self.put_int(val, 8);
            }

            template<std::integral I>
            const auto&
            put_bin(this auto& self, I val) {
                return self.put_int(val, 2);
            }

            template<std::floating_point F>
            const auto&
            put_float(this auto& self, F val) {
                char
                    lpcTemp[32];
                
                std::to_chars_result
                    result  = std::to_chars(
                                std::begin(lpcTemp),
                                std::end(lpcTemp),
                                val,
                                std::chars_format::fixed);
                *result.ptr = '\0';

                std::string_view
                    strvFloat   = ((bool)result.ec) ? lpcTemp : "errfloat";
                return self.put_str(strvFloat);
            }

            template<std::floating_point F>
            const auto&
            put_float_p(this auto& self, F val, int precision) {
                char
                    lpcTemp[32];
                
                std::to_chars_result
                    result  = std::to_chars(
                                std::begin(lpcTemp),
                                std::end(lpcTemp),
                                val,
                                std::chars_format::fixed,
                                precision);
                *result.ptr = '\0';

                std::string_view
                    strvFloat   = ((bool)result.ec) ? lpcTemp : "errfloat";
                return self.put_str(strvFloat);
            }

            template<typename... Args>
            const auto&
            fmt(this const auto& self, const std::format_string<Args...>& strfmt, Args&&... args) {
                static char
                    lpcBuffer[256];
                auto
                    result  = std::format_to_n(
                                lpcBuffer, sizeof(lpcBuffer),
                                strfmt, std::forward<Args>(args)...);

                return self.put_str({lpcBuffer, (size_t)result.size});
            }

            template<typename V> requires
                std::same_as<char, V> ||
                std::constructible_from<std::string_view, V> ||
                std::integral<V> ||
                std::floating_point<V>
            const auto&
            put(this const auto& self, const V& val) {
                if constexpr (std::same_as<V, char>)
                    return self.put_char(val);
                else if constexpr (std::constructible_from<std::string_view, V>)
                    return self.put_str(val);
                else if constexpr (std::integral<V>)
                    return self.put_int(val);
                else if constexpr (std::floating_point<V>)
                    return self.put_float(val);
                else
                    return self;
            }

            const auto&
            forward_char_from(this const auto& self, io::SerialIStream& from);

            const auto&
            forward_int_from(this const auto& self, io::SerialIStream& from, int base = 10);

            const auto&
            forward_float_from(this const auto& self, io::SerialIStream& from);

            const auto&
            forward_float_from(this const auto& self, io::SerialIStream& from, int precision);

            const auto&
            forward_word_from(this const auto& self, io::SerialIStream& from);

            const auto&
            forward_line_from(this const auto& self, io::SerialIStream& from);

            const auto&
            forward_all_from(this const auto& self, io::SerialIStream& from);

            const auto&
            forward_dec_from(this const auto& self, io::SerialIStream& from) {
                return self.export_int(from, 10);
            }

            const auto&
            forward_hex_from(this const auto& self, io::SerialIStream& from) {
                return self.export_int(from, 16);
            }

            const auto&
            forward_oct_from(this const auto& self, io::SerialIStream& from) {
                return self.export_int(from, 8);
            }

            const auto&
            forward_bin_from(this const auto& self, io::SerialIStream& from) {
                return self.export_int(from, 2);
            }
        };

        class TextInputBase {
        public:
            const auto&
            get_char(this const auto& self, char& out) {
                std::optional<std::byte>
                    optc = self.stream().Read();
                if (optc)
                    out = (char)*optc;
                return self;
            }

            const auto&
            get_word(this const auto& self, std::string& out) {
                std::string
                    strWord;
                std::optional<std::byte>
                    optc;
                while ((bool)(optc = self.stream().Read())) {
                    if (!isspace((int)*optc)) {
                        self.stream().PutBack(*optc);
                        break;
                    }
                }
                while ((bool)(optc = self.stream().Read())) {
                    if (isspace((int)*optc)) {
                        self.stream().PutBack(*optc);
                        break;
                    }

                    strWord.push_back((char)*optc);
                }

                out = std::move(strWord);
                return self;
            }

            const auto&
            get_line(this const auto& self, std::string& out) {
                std::string
                    strLine;
                std::optional<std::byte>
                    optc;
                while ((bool)(optc = self.stream().Read())) {
                    if ((char)*optc == '\n') {
                        break;
                    }
                    
                    strLine += (char)*optc;
                }

                out = std::move(strLine);
                return self;
            }

            const auto&
            get_all(this const auto& self, std::string& out) {
                std::string
                    strAll;
                std::optional<std::byte>
                    optc;
                while ((bool)(optc = self.stream().Read())) {
                    strAll += (char)*optc;
                }

                out = std::move(strAll);
                return self;
            }

            const auto&
            get_dec(this const auto& self, std::integral auto& out) {
                return self.get_int_impl(
                    out, 10,
                    [](char c) -> bool {
                        return c >= '0' && c <= '9';
                    });
            }

            const auto&
            get_hex(this const auto& self, std::integral auto& out) {
                return self.get_int_impl(
                    out, 16,
                    [](char c) -> bool {
                        return
                            (c >= '0' && c <= '9') ||
                            (c >= 'a' && c <= 'f') ||
                            (c >= 'A' && c <= 'F');
                    });
            }

            const auto&
            get_oct(this const auto& self, std::integral auto& out) {
                return self.get_int_impl(
                    out, 8,
                    [](char c) -> bool {
                        return c >= '0' && c <= '7';
                    });
            }

            const auto&
            get_bin(this const auto& self, std::integral auto& out) {
                return self.get_int_impl(
                    out, 2,
                    [](char c) -> bool {
                        return c >= '0' && c <= '1';
                    });
            }

            const auto&
            get_int(this const auto& self, std::integral auto& out, int base = 10) {
                switch (base) {
                case 2:
                    return self.get_bin(out);
                
                case 8:
                    return self.get_oct(out);

                case 10:
                    return self.get_dec(out);

                case 16:
                    return self.get_hex(out);

                default:
                    return self;
                }
            }

            const auto&
            get_float(this const auto& self, std::floating_point auto& out) {
                char
                    lpcBuffer[32];
                size_t
                    uSize = 0;
                std::optional<std::byte>
                    optc;
            
            ParseSpacing:
                if ((bool)(optc = self.stream().Read())) {
                    if (!isspace((int)*optc)) {
                        self.stream().PutBack(*optc);
                        goto ParseFirstChar;
                    }
                    else
                        goto ParseSpacing;
                }
                else
                    return self;

            ParseFirstChar:
                if ((bool)(optc = self.stream().Read())) {
                    char c = (char)*optc;
                    
                    if (c == '-' || c == '+' || isdigit(c)) {
                        lpcBuffer[uSize] = c;
                        uSize += 1;
                        goto ParseNaturalPart;
                    }
                    else if (c == '.' || c == ',') {
                        lpcBuffer[uSize] = '.';
                        uSize += 1;
                        goto ParseFractionalPart;
                    }
                    else {
                        self.stream().PutBack(*optc);
                        return self;
                    }
                }
                else
                    return self;

            ParseNaturalPart:
                if (uSize == sizeof(lpcBuffer))
                    goto GenerateValue;
                if ((bool)(optc = self.stream().Read())) {
                    char c = (char)*optc;
                    if (isdigit(c)) {
                        lpcBuffer[uSize] = c;
                        uSize += 1;
                        goto ParseNaturalPart;
                    }
                    else if (c == '.' || c == ',') {
                        lpcBuffer[uSize] = '.';
                        uSize += 1;
                        goto ParseFractionalPart;
                    }
                    else {
                        self.stream().PutBack(*optc);
                        goto GenerateValue;
                    }
                }
                else
                    goto GenerateValue;

            ParseFractionalPart:
                if (uSize == sizeof(lpcBuffer))
                    goto GenerateValue;
                if ((bool)(optc = self.stream().Read())) {
                    char c = (char)*optc;
                    if (isdigit(c)) {
                        lpcBuffer[uSize] = c;
                        uSize += 1;
                        goto ParseFractionalPart;
                    }
                    else {
                        self.stream().PutBack(*optc);
                        goto GenerateValue;
                    }
                }
                else
                    goto GenerateValue;

            GenerateValue:
                std::from_chars(
                    lpcBuffer, lpcBuffer + uSize,
                    out, std::chars_format::fixed);
                return self;
            }

            template<typename V> requires
                std::same_as<char, V> ||
                std::same_as<std::string, V> ||
                std::integral<V> ||
                std::floating_point<V>
            const auto&
            get(this const auto& self, V& val) {
                if constexpr (std::same_as<char, V>)
                    return self.get_char(val);
                else if constexpr (std::constructible_from<std::string_view, V>)
                    return self.get_word(val);
                else if constexpr (std::integral<V>)
                    return self.get_int(val);
                else if constexpr (std::floating_point<V>)
                    return self.get_float(val);
                else
                    return self;
            }

            const auto&
            forward_char_to(this const auto& self, io::SerialOStream& to);

            const auto&
            forward_int_to(this const auto& self, io::SerialOStream& to, int base = 10);

            const auto&
            forward_float_to(this const auto& self, io::SerialOStream& to);

            const auto&
            forward_float_to(this const auto& self, io::SerialOStream& to, int precision);

            const auto&
            forward_word_to(this const auto& self, io::SerialOStream& to);

            const auto&
            forward_line_to(this const auto& self, io::SerialOStream& to);

            const auto&
            forward_all_to(this const auto& self, io::SerialOStream& to);

            const auto&
            forward_dec_to(this const auto& self, io::SerialOStream& to) {
                return self.import_int(to, 10);
            }

            const auto&
            forward_hex_to(this const auto& self, io::SerialOStream& to) {
                return self.import_int(to, 16);
            }

            const auto&
            forward_oct_to(this const auto& self, io::SerialOStream& to) {
                return self.import_int(to, 8);
            }

            const auto&
            forward_bin_to(this const auto& self, io::SerialOStream& to) {
                return self.import_int(to, 2);
            }

        protected:
            const auto&
            get_int_impl(this const auto& self, std::integral auto& out, int base, bool(*fnIsDigit)(char)) {
                char
                    lpcBuffer[32];
                size_t
                    uSize = 0;
                std::optional<std::byte>
                    optc;
                
            ParseSpacing:
                if ((bool)(optc = self.stream().Read())) {
                    if (!isspace((int)*optc)) {
                        self.stream().PutBack(*optc);
                        goto ParseFirstChar;
                    }
                    else
                        goto ParseSpacing;
                }
                else
                    return self;

            ParseFirstChar:
                if ((bool)(optc = self.stream().Read())) {
                    char c = (char)*optc;
                    
                    if (c == '-' || c == '+' || isdigit(c)) {
                        lpcBuffer[uSize] = c;
                        uSize += 1;
                        goto ParseDigits;
                    }
                    else {
                        self.stream().PutBack(*optc);
                        return self;
                    }
                }
                else
                    return self;

            ParseDigits:
                if (uSize == sizeof(lpcBuffer))
                    goto GenerateValue;
                if ((bool)(optc = self.stream().Read())) {
                    char c = (char)*optc;
                    if (fnIsDigit(c)) {
                        lpcBuffer[uSize] = c;
                        uSize += 1;
                        goto ParseDigits;
                    }
                    else {
                        self.stream().PutBack(*optc);
                        goto GenerateValue;
                    }
                }
                else
                    goto GenerateValue;

            GenerateValue:
                std::from_chars(
                    lpcBuffer, lpcBuffer + uSize,
                    out, base);
                return self;
            }
        };

        class BinaryOutputBase {
        public:
            const auto&
            put_data(this const auto& self, std::span<const std::byte> buffer) {
                self.stream().WriteSome(buffer);
                return self;
            }
            
            const auto&
            put_int(this const auto& self, std::integral auto value) {
                return self.put_data({
                    (const std::byte*)&value, sizeof(value)
                });
            }

            const auto&
            put_float(this const auto& self, std::floating_point auto value) {
                return self.put_data({
                    (const std::byte*)&value, sizeof(value)
                });
            }

            template<typename V> requires
                std::constructible_from<std::span<const std::byte>, V> ||
                std::integral<V> ||
                std::floating_point<V>
            const auto&
            put(this const auto& self, V val) {
                if constexpr (std::constructible_from<std::span<const std::byte>, V>)
                    return self.put_data(val);
                else if constexpr (std::integral<V>)
                    return self.put_int(val);
                else if constexpr (std::floating_point<V>)
                    return self.put_float(val);
                else
                    return self;
            }

            const auto&
            forward_data_from(this const auto& self, io::SerialIStream& from, size_t uByteCount = SIZE_MAX);

            template<std::integral V>
            const auto&
            forward_int_from(this const auto& self, io::SerialIStream& from);

            template<std::floating_point V>
            const auto&
            forward_float_from(this const auto& self, io::SerialIStream& from);
        };

        class BinaryInputBase {
        public:
            const auto&
            get_data(this const auto& self, std::span<std::byte> buffer) {
                self.stream().ReadSome(buffer);
                return self;
            }

            const auto&
            get_int(this const auto& self, std::integral auto& value) {
                return self.get_data({ (std::byte*)&value, sizeof(value) });
            }

            const auto&
            get_float(this const auto& self, std::floating_point auto& value) {
                return self.get_data({ (std::byte*)&value, sizeof(value) });
            }

            template<typename V> requires
                std::constructible_from<std::span<std::byte>, V> ||
                (std::is_lvalue_reference_v<V> && !std::is_const_v<V> && (
                    std::integral<std::decay_t<V>> ||
                    std::floating_point<std::decay_t<V>>
                ))
            const auto&
            get(this const auto& self, V&& value) {
                if constexpr (std::constructible_from<std::span<std::byte>, V>)
                    return self.get_data(value);
                else if constexpr (std::integral<std::decay_t<V>>)
                    return self.get_int(value);
                else if constexpr (std::floating_point<std::decay_t<V>>)
                    return self.get_float(value);
                else
                    return self;
            }

            const auto&
            forward_data_to(this const auto& self, io::SerialOStream& to, size_t uByteCount = SIZE_MAX);

            template<std::integral V>
            const auto&
            forward_int_to(this const auto& self, io::SerialOStream& to);

            template<std::floating_point V>
            const auto&
            forward_float_to(this const auto& self, io::SerialOStream& to);
        };
        
        class SerialIOBase {
        public:
            const auto&
            good(this const auto& self, bool& out) {
                out = self.stream().Good();
                return self;
            }

            const auto&
            ended(this const auto& self, bool& out) {
                out = self.stream().EndOfStream();
                return self;
            }

            const auto&
            flush(this const auto& self) {
                self.stream().Flush();
                return self;
            }
        };

        class RandomAccessIOBase :
            public SerialIOBase {
        public:
            const auto&
            go(this const auto& self, intptr_t offset, StreamOffsetOrigin origin = StreamOffsetOrigin::CurrentPos) {
                self.stream().SetPosition(offset, origin);
                return self;
            }

            const auto&
            go_start(this const auto& self) {
                return self.go(0, StreamOffsetOrigin::StreamStart);
            }

            const auto&
            go_end(this const auto& self) {
                return self.go(0, StreamOffsetOrigin::StreamEnd);
            }
        };
    }

    class SerialTextInput :
        public __impl::SerialIOBase,
        public __impl::TextInputBase {
    public:
        SerialTextInput(const SerialTextInput&) = delete;
        
        SerialTextInput(io::SerialIStream& is) :
            refStream(is) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }

    private:
        io::SerialIStream&
            refStream;
    };

    class SerialTextOutput :
        public __impl::SerialIOBase,
        public __impl::TextOutputBase {
    public:
        SerialTextOutput(const SerialTextOutput&) = delete;
        
        SerialTextOutput(io::SerialOStream& os) :
            refStream(os) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }

    private:
        io::SerialOStream&
            refStream;
    };

    class SerialTextIO :
        public __impl::SerialIOBase,
        public __impl::TextInputBase,
        public __impl::TextOutputBase {
    public:
        SerialTextIO(const SerialTextIO&) = delete;

        SerialTextIO(io::SerialIOStream& ios) :
            refStream(ios) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }
    
    private:
        io::SerialIOStream&
            refStream;
    };

    class TextInput :
        public __impl::RandomAccessIOBase,
        public __impl::TextInputBase {
    public:
        TextInput(const TextInput&) = delete;
        
        TextInput(io::IStream& is) :
            refStream(is) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }

    private:
        io::IStream&
            refStream;
    };

    class TextOutput :
        public __impl::RandomAccessIOBase,
        public __impl::TextOutputBase {
    public:
        TextOutput(const TextOutput&) = delete;
        
        TextOutput(io::OStream& os) :
            refStream(os) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }

    private:
        io::OStream&
            refStream;
    };

    class TextIO :
        public __impl::RandomAccessIOBase,
        public __impl::TextInputBase,
        public __impl::TextOutputBase {
    public:
        TextIO(const TextIO&) = delete;

        TextIO(io::IOStream& ios) :
            refStream(ios) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }
    
    private:
        io::IOStream&
            refStream;
    };

    class SerialBinaryInput :
        public __impl::SerialIOBase,
        public __impl::BinaryInputBase {
    public:
        SerialBinaryInput(const SerialBinaryInput&) = delete;
        
        SerialBinaryInput(io::SerialIStream& is) :
            refStream(is) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }

    private:
        io::SerialIStream&
            refStream;
    };

    class SerialBinaryOutput :
        public __impl::SerialIOBase,
        public __impl::BinaryOutputBase {
    public:
        SerialBinaryOutput(const SerialBinaryOutput&) = delete;
        
        SerialBinaryOutput(io::SerialOStream& os) :
            refStream(os) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }

    private:
        io::SerialOStream&
            refStream;
    };

    class SerialBinaryIO :
        public __impl::SerialIOBase,
        public __impl::BinaryInputBase,
        public __impl::BinaryOutputBase {
    public:
        SerialBinaryIO(const SerialBinaryIO&) = delete;

        SerialBinaryIO(io::SerialIOStream& ios) :
            refStream(ios) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }
    
    private:
        io::SerialIOStream&
            refStream;
    };

    class BinaryInput :
        public __impl::RandomAccessIOBase,
        public __impl::BinaryInputBase {
    public:
        BinaryInput(const BinaryInput&) = delete;
        
        BinaryInput(io::IStream& is) :
            refStream(is) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }

    private:
        io::IStream&
            refStream;
    };

    class BinaryOutput :
        public __impl::RandomAccessIOBase,
        public __impl::BinaryOutputBase {
    public:
        BinaryOutput(const BinaryOutput&) = delete;
        
        BinaryOutput(io::OStream& os) :
            refStream(os) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }

    private:
        io::OStream&
            refStream;
    };

    class BinaryIO :
        public __impl::RandomAccessIOBase,
        public __impl::BinaryInputBase,
        public __impl::BinaryOutputBase {
    public:
        BinaryIO(const BinaryIO&) = delete;

        BinaryIO(io::IOStream& ios) :
            refStream(ios) {}

        auto&
        stream() const noexcept {
            return this->refStream;
        }
    
    private:
        io::IOStream&
            refStream;
    };

    namespace __impl {
        const auto&
        TextOutputBase::forward_char_from(this const auto& self, io::SerialIStream& from) {
            std::optional<std::byte>
                optc = from.Read();
            if (optc)
                self.Write(*optc);
            return self;
        }

        const auto&
        TextOutputBase::forward_int_from(this const auto& self, io::SerialIStream& from, int base) {
            intptr_t i;
            io::SerialTextInput(from)
                .get_int(i, base);
            return self.put_int(i);
        }

        const auto&
        TextOutputBase::forward_float_from(this const auto& self, io::SerialIStream& from) {
            double f;
            io::SerialTextInput(from)
                .get_float(f);
            return self.put_float(f);
        }

        const auto&
        TextOutputBase::forward_float_from(this const auto& self, io::SerialIStream& from, int precision) {
            double f;
            io::SerialTextInput(from)
                .get_float(f);
            return self.put_float_p(f, precision);
        }

        const auto&
        TextOutputBase::forward_word_from(this const auto& self, io::SerialIStream& from) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = from.Read())) {
                if (!isspace((int)*optc)) {
                    from.PutBack(*optc);
                    break;
                }
            }
            while ((bool)(optc = from.Read())) {
                if (isspace((int)*optc)) {
                    from.PutBack(*optc);
                    break;
                }

                self.stream().Write(*optc);
            }

            return self;
        }

        const auto&
        TextOutputBase::forward_line_from(this const auto& self, io::SerialIStream& from) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = from.Read())) {
                if ((char)*optc == '\n') {
                    break;
                }

                self.stream().Write(*optc);
            }

            return self;
        }

        const auto&
        TextOutputBase::forward_all_from(this const auto& self, io::SerialIStream& from) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = from.Read())) {
                self.stream().Write(*optc);
            }

            return self;
        }

        const auto&
        TextInputBase::forward_char_to(this const auto& self, io::SerialOStream& to) {
            std::optional<std::byte>
                optc = self.stream().Read();
            if (optc)
                to.Write(*optc);
            return self;
        }

        const auto&
        TextInputBase::forward_word_to(this const auto& self, io::SerialOStream& to) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = self.stream().Read())) {
                if (!isspace((int)*optc)) {
                    self.stream().PutBack(*optc);
                    break;
                }
            }
            while ((bool)(optc = self.stream().Read())) {
                if (isspace((int)*optc)) {
                    self.stream().PutBack(*optc);
                    break;
                }

                to.Write(*optc);
            }

            return self;
        }

        const auto&
        TextInputBase::forward_line_to(this const auto& self, io::SerialOStream& to) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = self.stream().Read())) {
                if ((char)*optc == '\n') {
                    break;
                }

                to.Write(*optc);
            }

            return self;
        }

        const auto&
        TextInputBase::forward_all_to(this const auto& self, io::SerialOStream& to) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = self.stream().Read())) {
                to.Write(*optc);
            }

            return self;
        }

        const auto&
        TextInputBase::forward_int_to(this const auto& self, io::SerialOStream& to, int base) {
            intptr_t iValue;
            self.get_int(iValue, base);
            io::SerialTextOutput
                out(to);
            out.put_int(iValue);
            return self;
        }

        const auto&
        TextInputBase::forward_float_to(this const auto& self, io::SerialOStream& to) {
            double fValue;
            self.get_float(fValue);
            io::SerialTextOutput
                out(to);
            out.put_float(fValue);
            return self;
        }

        const auto&
        TextInputBase::forward_float_to(this const auto& self, io::SerialOStream& to, int precision) {
            double fValue;
            self.get_float(fValue);
            io::SerialTextOutput
                out(to);
            out.put_float_p(fValue, precision);
            return self;
        }

        const auto&
        BinaryOutputBase::forward_data_from(this const auto& self, io::SerialIStream& from, size_t uByteCount) {
            while (uByteCount != 0) {
                std::optional<std::byte>
                    optc = from.Read();
                if (!optc)
                    break;
                self.stream().Write(*optc);
                uByteCount -= 1;
            }

            return self;
        }

        template<std::integral V>
        const auto&
        BinaryOutputBase::forward_int_from(this const auto& self, io::SerialIStream& from) {
            V iValue;
            SerialBinaryInput(from)
                .get_int(iValue);
            self.put_int(iValue);
            return self;
        }

        template<std::floating_point V>
        const auto&
        BinaryOutputBase::forward_float_from(this const auto& self, io::SerialIStream& from) {
            V fValue;
            SerialBinaryInput(from)
                .get_float(fValue);
            self.put_float(fValue);
            return self;
        }

        const auto&
        BinaryInputBase::forward_data_to(this const auto& self, io::SerialOStream& to, size_t uByteCount) {
            while (uByteCount != 0) {
                std::optional<std::byte>
                    optc = self.stream().Read();
                if (!optc)
                    break;
                to.Write(*optc);
                uByteCount -= 1;
            }

            return self;
        }

        template<std::integral V>
        const auto&
        BinaryInputBase::forward_int_to(this const auto& self, io::SerialOStream& to) {
            V iValue;
            self.get_int(iValue);
            SerialBinaryOutput(to)
                .put_int(iValue);
            return self;
        }

        template<std::floating_point V>
        const auto&
        BinaryInputBase::forward_float_to(this const auto& self, io::SerialOStream& to) {
            V fValue;
            self.get_float(fValue);
            SerialBinaryOutput(to)
                .put_float(fValue);
            return self;
        }
    }
}