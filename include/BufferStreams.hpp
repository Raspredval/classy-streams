static_assert(__cplusplus >= 202302, "requires C++23 minimum version");

#pragma once
#include <deque>
#include <cstdint>

#include "IOStreams.hpp"


namespace io {
    class IOBufferStream :
        virtual public  __impl::StreamState,
        virtual public  __impl::StreamPosition,
        public io::IOStream {
    public:
        IOBufferStream() = default;
        IOBufferStream(std::span<const std::byte> buffer);
        
        bool
        EndOfStream() const noexcept override;

        bool
        Good() const noexcept override;

        intptr_t
        GetPosition() const noexcept override;

        void
        ClearFlags() noexcept override;

        void
        ClearBuffer() noexcept;

        bool
        Flush() noexcept override;

        bool
        SetPosition(intptr_t offset, StreamOffsetOrigin from = StreamOffsetOrigin::StreamStart) override;

        intptr_t
        Erase(intptr_t iFirst, intptr_t iLast);

        intptr_t
        Insert(intptr_t iWhere, std::span<const std::byte> bytes);

        intptr_t
        Insert(intptr_t iWhere, io::SerialIStream& from, size_t uCount = SIZE_MAX);

        intptr_t
        Replace(intptr_t iFirst, intptr_t iLast, std::span<const std::byte> bytes);

        intptr_t
        Replace(intptr_t iFirst, intptr_t iLast, io::SerialIStream& is, size_t uCount = SIZE_MAX);

        bool
        Write(std::byte c) override;

        size_t
        WriteSome(std::span<const std::byte> buffer) override;

        std::optional<std::byte>
        Read() override;

        size_t
        ReadSome(std::span<std::byte> buffer) override;

        bool
        PutBack(std::byte c) override;

    private:
        std::deque<std::byte>
            deqBuffer;
        intptr_t
            iCurPos = 0;
        
        struct {
            std::byte
                retbuf[alignof(intptr_t) - 1];
            uint8_t
                retbuf_size : 7 = 0,
                flags_eof   : 1 = false;
        };
    };
}
