static_assert(__cplusplus >= 202302, "requires C++23 minimum version");

#pragma once
#include <format>
#include <concepts>
#include <string_view>

#include "IOStreams.hpp"


namespace io {
    namespace __impl {
        extern std::span<char>
        get_io_buffer(size_t uRequestedSize);

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
                std::span<char>
                    spnBuffer   = get_io_buffer(std::formatted_size("{:d}", val));
                auto result = std::format_to_n(
                                spnBuffer.data(), (ptrdiff_t)spnBuffer.size(),
                                "{:d}", val);
                return self.puts({(const char*)spnBuffer.data(), (size_t)result.size});
            }

            template<std::integral I>
            const auto&
            putx(this const auto& self, I val) {
                std::span<char>
                    spnBuffer   = get_io_buffer(std::formatted_size("{:x}", val));
                auto result = std::format_to_n(
                                spnBuffer.data(), (ptrdiff_t)spnBuffer.size(),
                                "{:x}", val);
                return self.puts({(const char*)spnBuffer.data(), (size_t)result.size});
            }

            template<std::integral I>
            const auto&
            puto(this const auto& self, I val) {
                std::span<char>
                    spnBuffer   = get_io_buffer(std::formatted_size("{:o}", val));
                auto result = std::format_to_n(
                                spnBuffer.data(), (ptrdiff_t)spnBuffer.size(),
                                "{:o}", val);
                return self.puts({(const char*)spnBuffer.data(), (size_t)result.size});
            }

            template<std::integral I>
            const auto&
            putb(this const auto& self, I val) {
                std::span<char>
                    spnBuffer   = get_io_buffer(std::formatted_size("{:b}", val));
                auto result = std::format_to_n(
                                spnBuffer.data(), (ptrdiff_t)spnBuffer.size(),
                                "{:b}", val);
                return self.puts({(const char*)spnBuffer.data(), (size_t)result.size});
            }

            template<std::floating_point F>
            const auto&
            putf(this const auto& self, F val) {
                std::span<char>
                    spnBuffer   = get_io_buffer(std::formatted_size("{:f}", val));
                auto result = std::format_to_n(
                                spnBuffer.data(), (ptrdiff_t)spnBuffer.size(),
                                "{:f}", val);
                return self.puts({(const char*)spnBuffer.data(), (size_t)result.size});
            }

            template<typename... Args>
            const auto&
            fmt(this const auto& self, const std::format_string<const Args&...>& strfmt, const Args&... args) {
                std::span<char>
                    spnBuffer   = get_io_buffer(std::formatted_size(strfmt, args...));
                auto result = std::format_to_n(
                                spnBuffer.data(), (ptrdiff_t)spnBuffer.size(),
                                strfmt, args...);
                return self.puts({(const char*)spnBuffer.data(), (size_t)result.size});
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
            importf(this const auto& self, io::SerialIStream& from);

            const auto&
            importwr(this const auto& self, io::SerialIStream& from) {
                TextOutputBase::importword_impl(from, self.stream());
                return self;
            }

            const auto&
            importln(this const auto& self, io::SerialIStream& from) {
                TextOutputBase::importline_impl(from, self.stream());
                return self;
            }

            const auto&
            importall(this const auto& self, io::SerialIStream& from) {
                TextOutputBase::importall_impl(from, self.stream());
                return self;
            }

            const auto&
            importd(this const auto& self, io::SerialIStream& from);

            const auto&
            importx(this const auto& self, io::SerialIStream& from);

            const auto&
            importo(this const auto& self, io::SerialIStream& from);

            const auto&
            importb(this const auto& self, io::SerialIStream& from);

            const auto&
            importi(this const auto& self, io::SerialIStream& from) {
                return self.importd(from);
            }

        protected:
            static void
            importword_impl(io::SerialIStream& from, io::SerialOStream& to);

            static void
            importline_impl(io::SerialIStream& from, io::SerialOStream& to);

            static void
            importall_impl(io::SerialIStream& from, io::SerialOStream& to);
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
                out = TextInputBase::getword_impl(self.stream());
                return self;
            }

            const auto&
            getln(this const auto& self, std::string& out) {
                out = TextInputBase::getline_impl(self.stream());
                return self;
            }

            const auto&
            getall(this const auto& self, std::string& out) {
                out = TextInputBase::getall_impl(self.stream());
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
                out = self.getint_impl(
                    self.stream(), 10);
                return self;
            }

            const auto&
            getx(this const auto& self, std::integral auto& out) {
                out = self.getint_impl(
                    self.stream(), 16);
                return self;
            }

            const auto&
            geto(this const auto& self, std::integral auto& out) {
                out = self.getint_impl(
                    self.stream(), 8);
                return self;
            }

            const auto&
            getb(this const auto& self, std::integral auto& out) {
                out = self.getint_impl(
                    self.stream(), 2);
                return self;
            }

            const auto&
            geti(this const auto& self, std::integral auto& out) {
                return self.getd(out);
            }

            const auto&
            getf(this const auto& self, std::floating_point auto& out) {
                out = self.getfloat_impl(
                    self.stream());
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
            exportwr(this const auto& self, io::SerialOStream& to) {
                TextInputBase::exportword_impl(self.stream(), to);
                return self;
            }

            const auto&
            exportln(this const auto& self, io::SerialOStream& to) {
                TextInputBase::exportline_impl(self.stream(), to);
                return self;
            }

            const auto&
            exportall(this const auto& self, io::SerialOStream& to) {
                TextInputBase::exportall_impl(self.stream(), to);
            }

            const auto&
            exportd(this const auto& self, io::SerialOStream& to);

            const auto&
            exportx(this const auto& self, io::SerialOStream& to);

            const auto&
            exporto(this const auto& self, io::SerialOStream& to);

            const auto&
            exportb(this const auto& self, io::SerialOStream& to);

        protected:
            static std::string
            getword_impl(io::SerialIStream& stream);

            static std::string
            getline_impl(io::SerialIStream& stream);

            static std::string
            getall_impl(io::SerialIStream& stream);

            static intptr_t
            getint_impl(io::SerialIStream& stream, int base);

            static double
            getfloat_impl(io::SerialIStream& stream);

            static void
            exportword_impl(io::SerialIStream& from, io::SerialOStream& to);

            static void
            exportline_impl(io::SerialIStream& from, io::SerialOStream& to);

            static void
            exportall_impl(io::SerialIStream& from, io::SerialOStream& to);
        };

        class BinaryOutputBase {
        public:
            const auto&
            putdata(this const auto& self, std::span<const std::byte> buffer) {
                self.stream().WriteSome(buffer);
                return self;
            }

            const auto&
            puti(this const auto& self, std::integral auto value) {
                return self.putdata({
                    (const std::byte*)&value, sizeof(value)
                });
            }

            const auto&
            putf(this const auto& self, std::floating_point auto value) {
                return self.putdata({
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
                    return self.putdata(val);
                else if constexpr (std::integral<V>)
                    return self.puti(val);
                else if constexpr (std::floating_point<V>)
                    return self.putf(val);
                else
                    return self;
            }

            const auto&
            importdata(this const auto& self, io::SerialIStream& from, size_t uByteCount = SIZE_MAX) {
                BinaryOutputBase::importdata_impl(from, self.stream(), uByteCount);
                return self;
            }

            template<std::integral V>
            const auto&
            importi(this const auto& self, io::SerialIStream& from);

            template<std::floating_point V>
            const auto&
            importf(this const auto& self, io::SerialIStream& from);

        protected:
            static void
            importdata_impl(io::SerialIStream& from, io::SerialOStream& to, size_t uByteCount);
        };

        class BinaryInputBase {
        public:
            const auto&
            getdata(this const auto& self, std::span<std::byte> buffer) {
                self.stream().ReadSome(buffer);
                return self;
            }

            const auto&
            geti(this const auto& self, std::integral auto& value) {
                return self.getdata({ (std::byte*)&value, sizeof(value) });
            }

            const auto&
            getf(this const auto& self, std::floating_point auto& value) {
                return self.getdata({ (std::byte*)&value, sizeof(value) });
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
                    return self.getdata(value);
                else if constexpr (std::integral<std::decay_t<V>>)
                    return self.geti(value);
                else if constexpr (std::floating_point<std::decay_t<V>>)
                    return self.getf(value);
                else
                    return self;
            }

            const auto&
            exportdata(this const auto& self, io::SerialOStream& to, size_t uByteCount = SIZE_MAX) {
                BinaryInputBase::exportdata_impl(self.stream(), to, uByteCount);
                return self;
            }

            template<std::integral V>
            const auto&
            exporti(this const auto& self, io::SerialOStream& to);

            template<std::floating_point V>
            const auto&
            exportf(this const auto& self, io::SerialOStream& to);

        protected:
            static void
            exportdata_impl(io::SerialIStream& from, io::SerialOStream& to, size_t uByteCount);
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
            self.getf(fValue);
            io::SerialTextOutput(to).putf(fValue);
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

        template<std::integral V>
        const auto&
        BinaryInputBase::exporti(this const auto& self, io::SerialOStream& to) {
            V iValue;
            self.geti(iValue);
            SerialBinaryOutput(to).puti(iValue);
            return self;
        }

        template<std::floating_point V>
        const auto&
        BinaryInputBase::exportf(this const auto& self, io::SerialOStream& to) {
            V fValue;
            self.getf(fValue);
            SerialBinaryOutput(to).putf(fValue);
            return self;
        }
    }
}