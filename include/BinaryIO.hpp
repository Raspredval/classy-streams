#pragma once
#include <concepts>
#include "IOStreams.hpp"

namespace io {
    namespace __impl {
        class SerialBinaryOutput {
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
                std::same_as<V, std::span<const std::byte>> ||
                std::integral<V> ||
                std::floating_point<V>
            const auto&
            put(this const auto& self, V val) {
                if constexpr (std::same_as<V, std::span<const std::byte>>)
                    return self.put_data(val);
                else if constexpr (std::integral<V>)
                    return self.put_int(val);
                else if constexpr (std::floating_point<V>)
                    return self.put_float(val);
                else
                    return self;
            }

            const auto&
            forward_from(this const auto& self, io::SerialIStream& from, size_t uCount = SIZE_MAX) {
                auto&
                    to      = self.stream();

                while (uCount > 0) {
                    auto
                        optc    = from.Read();
                    if (!optc)
                        break;
                    to.Write(*optc);
                    uCount      -= 1;
                }

                return self;
            }
        };

        class SerialBinaryInput {
        public:
            const auto&
            get_data(this const auto& self, std::span<std::byte> buffer) {
                self.stream().ReadSome(buffer);
                return self;
            }

            const auto&
            get_int(this const auto& self, std::integral auto& value) {
                return self.put_data({ (std::byte*)&value, sizeof(value) });
            }

            const auto&
            get_float(this const auto& self, std::floating_point auto& value) {
                return self.put_data({ (std::byte*)&value, sizeof(value) });
            }

            template<typename V> requires
                std::same_as<V, std::span<std::byte>> || 
                (
                    (!std::is_const_v<V> && std::is_lvalue_reference_v<V>) && (
                        std::integral<std::decay_t<V>> ||
                        std::floating_point<std::decay_t<V>>
                    )
                )
            const auto&
            get(this const auto& self, V value) {
                if constexpr (std::same_as<V, std::span<std::byte>>)
                    return self.get_data(value);
                else if constexpr (std::integral<std::decay_t<V>>)
                    return self.get_int(value);
                else if constexpr (std::floating_point<std::decay_t<V>>)
                    return self.get_float(value);
                else
                    return self;
            }

            const auto&
            forward_to(this const auto& self, io::SerialOStream& to, size_t uCount = SIZE_MAX) {
                auto&
                    from    = self.stream();

                while (uCount > 0) {
                    auto
                        optc    = from.Read();
                    if (!optc)
                        break;
                    to.Write(*optc);
                    uCount      -= 1;
                }

                return self;
            }
        };

        class SerialAccessBinaryIO {
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

        class RandomAccessBinaryIO :
            public SerialAccessBinaryIO {
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

    class SerialBinaryInput :
        public __impl::SerialAccessBinaryIO,
        public __impl::SerialBinaryInput {
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
        public __impl::SerialAccessBinaryIO,
        public __impl::SerialBinaryOutput {
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
        public __impl::SerialAccessBinaryIO,
        public __impl::SerialBinaryInput,
        public __impl::SerialBinaryOutput {
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
        public __impl::RandomAccessBinaryIO,
        public __impl::SerialBinaryInput {
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
        public __impl::RandomAccessBinaryIO,
        public __impl::SerialBinaryOutput {
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
        public __impl::RandomAccessBinaryIO,
        public __impl::SerialBinaryInput,
        public __impl::SerialBinaryOutput {
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
}