static_assert(__cplusplus >= 202302, "requires C++23 minimum version");

#pragma once
#include <format>
#include <concepts>
#include <string_view>

#include "IOStreams.hpp"


namespace io {
    namespace __impl {
        class buffer {
            friend class TextInputBase;
            friend class TextOutputBase;
        public:
            buffer() :
                data(nullptr),
                size(0) {}
            
            ~buffer() {
                if (this->data)
                    free(this->data);
            }

        private:
            void realloc(size_t uSize) {
                if (this->size < uSize) {
                    this->data  = (char*)::realloc((void*)this->data, uSize);
                    this->size  = uSize;
                }
            }
    
            char*   data;
            size_t  size;
        } inline g_buffer;

        class TextOutputBase {
        public:
            const auto&
            putc(this const auto& self, char c) {
                self.stream().Write((std::byte)c);
                return self;
            }

            const auto&
            puts(this const auto& self, std::string_view strv) {
                self.stream().WriteSome({
                    (const std::byte*)strv.data(),
                    strv.size()
                });
                return self;
            }

            const auto&
            putbool(this const auto& self, bool val) {
                return self.puts(val ? "true" : "false");
            }

            const auto&
            putnl(this const auto& self) {
                return self.putc('\n');
            }

            template<std::integral I>
            const auto&
            puti(this const auto& self, I val) {
                return self.putd(val);
            }

            template<std::integral I>
            const auto&
            putd(this const auto& self, I val) {
                g_buffer.realloc(std::formatted_size("{:d}", val));
                auto result = std::format_to_n(
                                g_buffer.data, (ptrdiff_t)g_buffer.size,
                                "{:d}", val);
                return self.puts({(const char*)g_buffer.data, (size_t)result.size});
            }

            template<std::integral I>
            const auto&
            putx(this const auto& self, I val) {
                g_buffer.realloc(std::formatted_size("{:x}", val));
                auto result = std::format_to_n(
                                g_buffer.data, (ptrdiff_t)g_buffer.size,
                                "{:x}", val);
                return self.puts({(const char*)g_buffer.data, (size_t)result.size});
            }

            template<std::integral I>
            const auto&
            puto(this const auto& self, I val) {
                g_buffer.realloc(std::formatted_size("{:o}", val));
                auto result = std::format_to_n(
                                g_buffer.data, (ptrdiff_t)g_buffer.size,
                                "{:o}", val);
                return self.puts({(const char*)g_buffer.data, (size_t)result.size});
            }

            template<std::integral I>
            const auto&
            putb(this const auto& self, I val) {
                g_buffer.realloc(std::formatted_size("{:d}", val));
                auto result = std::format_to_n(
                                g_buffer.data, (ptrdiff_t)g_buffer.size,
                                "{:b}", val);
                return self.puts({(const char*)g_buffer.data, (size_t)result.size});
            }

            template<std::floating_point F>
            const auto&
            putf(this const auto& self, F val) {
                g_buffer.realloc(std::formatted_size("{:f}", val));
                auto result = std::format_to_n(
                                g_buffer.data, (ptrdiff_t)g_buffer.size,
                                "{:f}", val);
                return self.puts({(const char*)g_buffer.data, (size_t)result.size});
            }

            template<typename... Args>
            const auto&
            fmt(this const auto& self, const std::format_string<Args...>& strfmt, Args&&... args) {
                g_buffer.realloc(std::formatted_size(strfmt, std::forward<Args>(args)...));
                auto result = std::format_to_n(
                                g_buffer.data, (ptrdiff_t)g_buffer.size,
                                strfmt, std::forward<Args>(args)...);

                return self.puts({(const char*)g_buffer.data, (size_t)result.size});
            }

            template<typename V> requires
                std::same_as<char, V> ||
                std::constructible_from<std::string_view, V> ||
                std::integral<V> ||
                std::floating_point<V>
            const auto&
            put(this const auto& self, const V& val) {
                if constexpr (std::same_as<V, char>)
                    return self.putc(val);
                else if constexpr (std::integral<V>)
                    return self.puti(val);
                else if constexpr (std::floating_point<V>)
                    return self.putf(val);
                else if constexpr (std::constructible_from<std::string_view, V>)
                    return self.puts(val);
                else
                    return self;
            }

            const auto&
            importc(this const auto& self, io::SerialIStream& from);

            const auto&
            inporti(this const auto& self, io::SerialIStream& from) {
                return self.importd(from);
            }

            const auto&
            importf(this const auto& self, io::SerialIStream& from);

            const auto&
            importwr(this const auto& self, io::SerialIStream& from);

            const auto&
            importln(this const auto& self, io::SerialIStream& from);

            const auto&
            import_all(this const auto& self, io::SerialIStream& from);

            const auto&
            importd(this const auto& self, io::SerialIStream& from);

            const auto&
            importx(this const auto& self, io::SerialIStream& from);

            const auto&
            importo(this const auto& self, io::SerialIStream& from);

            const auto&
            importb(this const auto& self, io::SerialIStream& from);
        };

        class TextInputBase {
        public:
            const auto&
            getc(this const auto& self, char& out) {
                std::optional<std::byte>
                    optc = self.stream().Read();
                if (optc)
                    out = (char)*optc;
                return self;
            }

            const auto&
            getwr(this const auto& self, std::string& out) {
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
            getln(this const auto& self, std::string& out) {
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
            getbool(this const auto& self, bool& out) {
                std::string s;
                self.getwr(s);
                out = (s == "true");
                return self;
            }

            const auto&
            getd(this const auto& self, std::integral auto& out) {
                return self.get_int_impl(
                    out, 10,
                    [](char c) -> bool {
                        return c >= '0' && c <= '9';
                    });
            }

            const auto&
            getx(this const auto& self, std::integral auto& out) {
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
            geto(this const auto& self, std::integral auto& out) {
                return self.get_int_impl(
                    out, 8,
                    [](char c) -> bool {
                        return c >= '0' && c <= '7';
                    });
            }

            const auto&
            getb(this const auto& self, std::integral auto& out) {
                return self.get_int_impl(
                    out, 2,
                    [](char c) -> bool {
                        return c >= '0' && c <= '1';
                    });
            }

            const auto&
            geti(this const auto& self, std::integral auto& out) {
                return self.get_dec(out);
            }

            const auto&
            getf(this const auto& self, std::floating_point auto& out) {
                size_t
                    uSize = 0;
                std::optional<std::byte>
                    optc;
                g_buffer.realloc(32);

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
                        g_buffer.data[uSize] = c;
                        uSize += 1;
                        goto ParseNaturalPart;
                    }
                    else if (c == '.' || c == ',') {
                        g_buffer.data[uSize] = '.';
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
                if (uSize == g_buffer.size)
                    goto GenerateValue;
                if ((bool)(optc = self.stream().Read())) {
                    char c = (char)*optc;
                    if (isdigit(c)) {
                        g_buffer.data[uSize] = c;
                        uSize += 1;
                        goto ParseNaturalPart;
                    }
                    else if (c == '.' || c == ',') {
                        g_buffer.data[uSize] = '.';
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
                if (uSize == g_buffer.size)
                    goto GenerateValue;
                if ((bool)(optc = self.stream().Read())) {
                    char c = (char)*optc;
                    if (isdigit(c)) {
                        g_buffer.data[uSize] = c;
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
                    g_buffer.data, g_buffer.data + uSize,
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
                    return self.getc(val);
                else if constexpr (std::integral<V>)
                    return self.geti(val);
                else if constexpr (std::floating_point<V>)
                    return self.getf(val);
                else if constexpr (std::constructible_from<std::string_view, V>)
                    return self.getwr(val);
                else
                    return self;
            }

            const auto&
            exportc(this const auto& self, io::SerialOStream& to);

            const auto&
            exporti(this const auto& self, io::SerialOStream& to) {
                return self.exportd(to);
            }

            const auto&
            exportf(this const auto& self, io::SerialOStream& to);

            const auto&
            exportwr(this const auto& self, io::SerialOStream& to);

            const auto&
            exportln(this const auto& self, io::SerialOStream& to);

            const auto&
            export_all(this const auto& self, io::SerialOStream& to);

            const auto&
            exportd(this const auto& self, io::SerialOStream& to);

            const auto&
            exportx(this const auto& self, io::SerialOStream& to);

            const auto&
            exporto(this const auto& self, io::SerialOStream& to);

            const auto&
            exportb(this const auto& self, io::SerialOStream& to);

        protected:
            const auto&
            get_int_impl(this const auto& self, std::integral auto& out, int base, bool(*fnIsDigit)(char)) {
                size_t
                    uSize = 0;
                std::optional<std::byte>
                    optc;
                g_buffer.realloc(32);
                
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
                        g_buffer.data[uSize] = c;
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
                if (uSize == g_buffer.size)
                    goto GenerateValue;
                if ((bool)(optc = self.stream().Read())) {
                    char c = (char)*optc;
                    if (fnIsDigit(c)) {
                        g_buffer.data[uSize] = c;
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
                    g_buffer.data, g_buffer.data + uSize,
                    out, base);
                return self;
            }
        };

        class BinaryOutputBase {
        public:
            const auto&
            putdt(this const auto& self, std::span<const std::byte> buffer) {
                self.stream().WriteSome(buffer);
                return self;
            }
            
            const auto&
            puti(this const auto& self, std::integral auto value) {
                return self.putdt({
                    (const std::byte*)&value, sizeof(value)
                });
            }

            const auto&
            putf(this const auto& self, std::floating_point auto value) {
                return self.putdt({
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
                    return self.putdt(val);
                else if constexpr (std::integral<V>)
                    return self.puti(val);
                else if constexpr (std::floating_point<V>)
                    return self.putf(val);
                else
                    return self;
            }

            const auto&
            importdt(this const auto& self, io::SerialIStream& from, size_t uByteCount = SIZE_MAX);

            template<std::integral V>
            const auto&
            importi(this const auto& self, io::SerialIStream& from);

            template<std::floating_point V>
            const auto&
            importf(this const auto& self, io::SerialIStream& from);
        };

        class BinaryInputBase {
        public:
            const auto&
            getdt(this const auto& self, std::span<std::byte> buffer) {
                self.stream().ReadSome(buffer);
                return self;
            }

            const auto&
            geti(this const auto& self, std::integral auto& value) {
                return self.getdt({ (std::byte*)&value, sizeof(value) });
            }

            const auto&
            getf(this const auto& self, std::floating_point auto& value) {
                return self.getdt({ (std::byte*)&value, sizeof(value) });
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
                    return self.getdt(value);
                else if constexpr (std::integral<std::decay_t<V>>)
                    return self.geti(value);
                else if constexpr (std::floating_point<std::decay_t<V>>)
                    return self.getf(value);
                else
                    return self;
            }

            const auto&
            exportdt(this const auto& self, io::SerialOStream& to, size_t uByteCount = SIZE_MAX);

            template<std::integral V>
            const auto&
            exporti(this const auto& self, io::SerialOStream& to);

            template<std::floating_point V>
            const auto&
            exportf(this const auto& self, io::SerialOStream& to);
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
        TextOutputBase::importc(this const auto& self, io::SerialIStream& from) {
            char c;
            io::SerialTextInput(from).getc(c);
            return self.putc(c);
        }

        const auto&
        TextOutputBase::importd(this const auto& self, io::SerialIStream& from) {
            intptr_t i;
            io::SerialTextInput(from).getd(i);
            return self.putd(i);
        }

        const auto&
        TextOutputBase::importx(this const auto& self, io::SerialIStream& from) {
            intptr_t i;
            io::SerialTextInput(from).getx(i);
            return self.putx(i);
        }

        const auto&
        TextOutputBase::importo(this const auto& self, io::SerialIStream& from) {
            intptr_t i;
            io::SerialTextInput(from).geto(i);
            return self.puto(i);
        }

        const auto&
        TextOutputBase::importb(this const auto& self, io::SerialIStream& from) {
            intptr_t i;
            io::SerialTextInput(from).getb(i);
            return self.putb(i);
        }

        const auto&
        TextOutputBase::importf(this const auto& self, io::SerialIStream& from) {
            double f;
            io::SerialTextInput(from).getf(f);
            return self.putf(f);
        }

        const auto&
        TextOutputBase::importwr(this const auto& self, io::SerialIStream& from) {
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
        TextOutputBase::importln(this const auto& self, io::SerialIStream& from) {
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
        TextOutputBase::import_all(this const auto& self, io::SerialIStream& from) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = from.Read())) {
                self.stream().Write(*optc);
            }

            return self;
        }

        const auto&
        TextInputBase::exportc(this const auto& self, io::SerialOStream& to) {
            char c;
            self.getc(c);
            io::SerialTextOutput(to).putc(c);
            return self;
        }

        const auto&
        TextInputBase::exportd(this const auto& self, io::SerialOStream& to) {
            intptr_t i;
            self.getd(i);
            io::SerialTextOutput(to).putd(i);
            return self;
        }

        const auto&
        TextInputBase::exportx(this const auto& self, io::SerialOStream& to) {
            intptr_t i;
            self.getx(i);
            io::SerialTextOutput(to).putx(i);
            return self;
        }

        const auto&
        TextInputBase::exporto(this const auto& self, io::SerialOStream& to) {
            intptr_t i;
            self.geto(i);
            io::SerialTextOutput(to).puto(i);
            return self;
        }

        const auto&
        TextInputBase::exportb(this const auto& self, io::SerialOStream& to) {
            intptr_t i;
            self.getb(i);
            io::SerialTextOutput(to).putb(i);
            return self;
        }

        const auto&
        TextInputBase::exportf(this const auto& self, io::SerialOStream& to) {
            double fValue;
            self.get_float(fValue);
            io::SerialTextOutput(to).putf(fValue);
            return self;
        }

        const auto&
        TextInputBase::exportwr(this const auto& self, io::SerialOStream& to) {
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
        TextInputBase::exportln(this const auto& self, io::SerialOStream& to) {
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
        TextInputBase::export_all(this const auto& self, io::SerialOStream& to) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = self.stream().Read())) {
                to.Write(*optc);
            }

            return self;
        }

        const auto&
        BinaryOutputBase::importdt(this const auto& self, io::SerialIStream& from, size_t uByteCount) {
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
        BinaryOutputBase::importi(this const auto& self, io::SerialIStream& from) {
            V iValue;
            SerialBinaryInput(from).geti(iValue);
            return self.puti(iValue);
        }

        template<std::floating_point V>
        const auto&
        BinaryOutputBase::importf(this const auto& self, io::SerialIStream& from) {
            V fValue;
            SerialBinaryInput(from).getf(fValue);
            return self.putf(fValue);
        }

        const auto&
        BinaryInputBase::exportdt(this const auto& self, io::SerialOStream& to, size_t uByteCount) {
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
        BinaryInputBase::exporti(this const auto& self, io::SerialOStream& to) {
            V iValue;
            self.get_int(iValue);
            SerialBinaryOutput(to).puti(iValue);
            return self;
        }

        template<std::floating_point V>
        const auto&
        BinaryInputBase::exportf(this const auto& self, io::SerialOStream& to) {
            V fValue;
            self.get_float(fValue);
            SerialBinaryOutput(to).putf(fValue);
            return self;
        }
    }
}