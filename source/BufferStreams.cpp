#include "BufferStreams.hpp"

namespace io {
    IOBufferStream::IOBufferStream(std::span<const std::byte> buffer) :
        deqBuffer(std::from_range, buffer) {}

    bool
    IOBufferStream::EndOfStream() const noexcept {
        return this->flags_eof;
    }

    bool
    IOBufferStream::Good() const noexcept {
        return true;
    }

    intptr_t
    IOBufferStream::GetPosition() const noexcept {
        return this->iCurPos;
    }

    void
    IOBufferStream::ClearFlags() noexcept {
        this->flags_eof = false;
    }

    void
    IOBufferStream::ClearBuffer() noexcept {
        this->deqBuffer.clear();
        this->iCurPos   = 0;
        this->ClearFlags();
    }

    bool
    IOBufferStream::Flush() noexcept {
        return true;
    }

    bool
    IOBufferStream::SetPosition(intptr_t offset, StreamOffsetOrigin from) {
        switch(from) {
        case StreamOffsetOrigin::CurrentPos:
            offset  += this->iCurPos;
            break;

        case StreamOffsetOrigin::StreamStart:
            offset  += 0;
            break;

        case StreamOffsetOrigin::StreamEnd:
            offset  += (intptr_t)this->deqBuffer.size();
            break;
        }

        auto
            itNewPos    = this->deqBuffer.begin() + offset;
        if (itNewPos < this->deqBuffer.begin() || itNewPos >= this->deqBuffer.end())
            return false;

        this->iCurPos       = offset;
        this->flags_eof     = false;
        this->retbuf_size   = 0;
        return true;
    }

    intptr_t
    IOBufferStream::Erase(intptr_t iFirst, intptr_t iLast) {
        return this->deqBuffer.erase(
            this->deqBuffer.begin() + iFirst,
            this->deqBuffer.begin() + iLast) - this->deqBuffer.begin();
        this->iCurPos   = std::min<intptr_t>(
                            this->iCurPos,
                            (intptr_t)this->deqBuffer.size());
    }

    intptr_t
    IOBufferStream::Insert(intptr_t iWhere, std::span<const std::byte> bytes) {
        auto
            itWhere = this->deqBuffer.begin() + iWhere;
        this->deqBuffer.insert(
            itWhere,
            bytes.begin(),
            bytes.end());
        return iWhere;
    }

    intptr_t
    IOBufferStream::Insert(intptr_t iWhere, io::SerialIStream& from, size_t uCount) {
        auto
            itWhere = this->deqBuffer.begin() + iWhere;
        for (size_t i = 0; i != uCount; ++i) {
            std::optional<std::byte>
                optc    = from.Read();
            if (!optc)
                break;

            itWhere     = ++this->deqBuffer.insert(itWhere, *optc);
        }

        return iWhere;
    }

    intptr_t
    IOBufferStream::Replace(intptr_t iFirst, intptr_t iLast, std::span<const std::byte> bytes) {
        return this->Insert(
            this->Erase(iFirst, iLast),
            bytes);
    }

    intptr_t
    IOBufferStream::Replace(intptr_t iFirst, intptr_t iLast, io::SerialIStream& from, size_t uCount) {
        return this->Insert(
            this->Erase(iFirst, iLast),
            from, uCount);
    }

    bool
    IOBufferStream::Write(std::byte c) {
        auto
            itCurPos    = this->deqBuffer.begin() + this->iCurPos;
        this->deqBuffer.insert(
            itCurPos, c);
        this->iCurPos   += 1;

        this->retbuf_size = 0;
        this->ClearFlags();
        return true;
    }

    size_t
    IOBufferStream::WriteSome(std::span<const std::byte> buffer) {
        auto
            itCurPos    = this->deqBuffer.begin() + this->iCurPos;
        this->deqBuffer.insert_range(
            itCurPos, buffer);
        this->iCurPos   += buffer.size();

        this->retbuf_size = 0;
        this->ClearFlags();
        return buffer.size();
    }

    std::optional<std::byte>
    IOBufferStream::Read() {
        if (this->retbuf_size != 0) {
            this->retbuf_size -= 1;
            return this->retbuf[this->retbuf_size];
        }

        auto
            itCurr  = this->deqBuffer.begin() + this->iCurPos,
            itEnd   = this->deqBuffer.end();
        if (itCurr != itEnd) {
            this->iCurPos   += 1;
            return *itCurr;
        }

        this->flags_eof = true;
        return std::nullopt;
    }

    size_t
    IOBufferStream::ReadSome(std::span<std::byte> buffer) {
        for (size_t i = 0; i != buffer.size(); ++i) {
            std::optional<std::byte>
                c   = this->Read();
            if (!c.has_value()) {
                return i;
            }

            buffer[i] = *c;
        }

        return buffer.size();
    }

    bool
    IOBufferStream::PutBack(std::byte c) {
        if (this->retbuf_size < sizeof(this->retbuf)) {
            this->retbuf[this->retbuf_size] = c;
            this->retbuf_size += 1;
            this->ClearFlags();
            return true;
        }
        else
            return false;
    }
}