#include "NetworkStreams.hpp"

namespace io {
    namespace __impl {
        BufferedNetworkStream::BufferedNetworkStream(int fdSocket) {
            if (fdSocket < 0)
                throw std::runtime_error("initializing network stream with invalid socket descriptor");

            this->i.lpData      = new std::byte[this->i.uBufCap];
            this->o.lpData      = new std::byte[this->o.uBufCap];
            this->s.fdSocket    = fdSocket;
        }

        BufferedNetworkStream::~BufferedNetworkStream() noexcept {
            this->Flush();
            shutdown(this->s.fdSocket, SHUT_RD);
            while (this->GetInput()) {}
            shutdown(this->s.fdSocket, SHUT_RDWR);
            close(this->s.fdSocket);
            delete[] this->i.lpData;
            delete[] this->o.lpData;
        }

        std::optional<std::byte>
        BufferedNetworkStream::Read() noexcept {
            if (this->s.uRetLen != 0)
                return this->s.lpRetBuf[--this->s.uRetLen];

            if (this->i.uBegin == this->i.uEnd) {
                if (!this->GetInput())
                    return std::nullopt;
            }

            return this->i.lpData[this->i.uBegin++];
        }

        bool
        BufferedNetworkStream::Write(std::byte c) noexcept {
            if (this->o.uSize == this->o.uBufCap) {
                if (!this->Flush())
                    return false;
            }

            this->o.lpData[this->o.uSize++] = c;
            return true;
        }

        size_t
        BufferedNetworkStream::ReadSome(std::span<std::byte> buffer) noexcept {
            for (size_t i = 0; i != buffer.size(); ++i) {
                std::optional<std::byte>
                    optc    = this->Read();
                if (!optc)
                    return i;
            }

            return buffer.size();
        }

        size_t
        BufferedNetworkStream::WriteSome(std::span<const std::byte> buffer) noexcept {
            for (size_t i = 0; i != buffer.size(); ++i) {
                if (!this->Write(buffer[i]))
                    return i;
            }

            return buffer.size();
        }

        bool
        BufferedNetworkStream::PutBack(std::byte c) noexcept {
            if (this->s.uRetLen == sizeof(this->s.lpRetBuf))
                return false;

            this->s.lpRetBuf[this->s.uRetLen++] = c;
            return true;
        }

        bool
        BufferedNetworkStream::Flush() noexcept {
            if (this->o.uSize == 0)
                return true;

            ssize_t
                iOutputSize = send(
                                this->s.fdSocket,
                                this->o.lpData,
                                this->o.uSize,
                                0);
            if (iOutputSize < 0) {
                this->s.bErr = true;
                return false;
            }

            this->o.uSize   = 0;
            return true;
        }

        void
        BufferedNetworkStream::ClearFlags() noexcept {
            this->s.bEOF    = false;
            this->s.bErr    = false;
        }

        bool
        BufferedNetworkStream::Error() const noexcept {
            return (bool)this->s.bEOF;
        }

        bool
        BufferedNetworkStream::EndOfStream() const noexcept {
            return (bool)this->s.bEOF;
        }

        int
        BufferedNetworkStream::Descriptor() const noexcept {
            return this->s.fdSocket;
        }

        bool
        BufferedNetworkStream::GetInput() {
            ssize_t
                iInputSize  = recv(
                                this->s.fdSocket,
                                this->i.lpData,
                                this->i.uBufCap,
                                0);
            if (iInputSize < 0) {
                this->s.bErr = true;
                return false;
            }

            if (iInputSize == 0) {
                this->s.bEOF = true;
                return false;
            }

            this->i.uBegin  = 0;
            this->i.uEnd    = (size_t)iInputSize;
            return true;
        }

        NetworkStreamViewBase::NetworkStreamViewBase(BufferedNetworkStream* hStream) :
            hStream(hStream) {}

        bool
        NetworkStreamViewBase::EndOfStream() const noexcept {
            return this->hStream->EndOfStream();
        }

        bool
        NetworkStreamViewBase::Good() const noexcept {
            return !this->hStream->Error();
        }

        bool
        NetworkStreamViewBase::Flush() noexcept {
            return this->hStream->Flush();
        }

        void
        NetworkStreamViewBase::ClearFlags() noexcept {
            return this->hStream->ClearFlags();
        }

        BufferedNetworkStream*
        NetworkStreamViewBase::Handle() const noexcept {
            return this->hStream;
        }

        NetworkStreamBase::NetworkStreamBase(int fdSocket) :
            NetworkStreamViewBase(new BufferedNetworkStream(fdSocket)) {}

        NetworkStreamBase::NetworkStreamBase(NetworkStreamBase&& obj) noexcept {
            this->hStream   = obj.hStream;
            obj.hStream     = nullptr;
        }

        NetworkStreamBase&
        NetworkStreamBase::operator=(NetworkStreamBase&& obj) noexcept {
            NetworkStreamBase
                temp    = std::move(obj);
            std::swap(this->hStream, temp.hStream);
            return *this;
        }

        NetworkStreamBase::~NetworkStreamBase() noexcept {
            if (this->hStream != nullptr) {
                delete this->hStream;
                this->hStream   = nullptr;
            }
        }
    }

    INetworkStreamView::INetworkStreamView(__impl::BufferedNetworkStream* hStream) :
        NetworkStreamViewBase(hStream) {}

    std::optional<std::byte>
    INetworkStreamView::Read() {
        return this->hStream->Read();
    }

    size_t
    INetworkStreamView::ReadSome(std::span<std::byte> buffer) {
        return this->hStream->ReadSome(buffer);
    }

    bool
    INetworkStreamView::PutBack(std::byte c) {
        return this->hStream->PutBack(c);
    }

    ONetworkStreamView::ONetworkStreamView(__impl::BufferedNetworkStream* hStream) :
        NetworkStreamViewBase(hStream) {}

    bool
    ONetworkStreamView::Write(std::byte c) {
        return this->hStream->Write(c);
    }

    size_t
    ONetworkStreamView::WriteSome(std::span<const std::byte> buffer) {
        return this->hStream->WriteSome(buffer);
    }

    IONetworkStreamView::IONetworkStreamView(__impl::BufferedNetworkStream* hStream) :
        NetworkStreamViewBase(hStream) {}

    std::optional<std::byte>
    IONetworkStreamView::Read() {
        return this->hStream->Read();
    }

    size_t
    IONetworkStreamView::ReadSome(std::span<std::byte> buffer) {
        return this->hStream->ReadSome(buffer);
    }

    bool
    IONetworkStreamView::PutBack(std::byte c) {
        return this->hStream->PutBack(c);
    }

    bool
    IONetworkStreamView::Write(std::byte c) {
        return this->hStream->Write(c);
    }

    size_t
    IONetworkStreamView::WriteSome(std::span<const std::byte> buffer) {
        return this->hStream->WriteSome(buffer);
    }

    INetworkStream::INetworkStream(int fdSocket) :
        NetworkStreamBase(fdSocket)
    {
        shutdown(this->hStream->Descriptor(), SHUT_WR);
    }

    std::optional<std::byte>
    INetworkStream::Read() {
        return this->hStream->Read();
    }

    size_t
    INetworkStream::ReadSome(std::span<std::byte> buffer) {
        return this->hStream->ReadSome(buffer);
    }

    bool
    INetworkStream::PutBack(std::byte c) {
        return this->hStream->PutBack(c);
    }

    ONetworkStream::ONetworkStream(int fdSocket) :
        NetworkStreamBase(fdSocket)
    {
        shutdown(this->hStream->Descriptor(), SHUT_RD);
    }

    bool
    ONetworkStream::Write(std::byte c) {
        return this->hStream->Write(c);
    }

    size_t
    ONetworkStream::WriteSome(std::span<const std::byte> buffer) {
        return this->hStream->WriteSome(buffer);
    }

    IONetworkStream::IONetworkStream(int fdSocket) :
        NetworkStreamBase(fdSocket) {}

    std::optional<std::byte>
    IONetworkStream::Read() {
        return this->hStream->Read();
    }

    size_t
    IONetworkStream::ReadSome(std::span<std::byte> buffer) {
        return this->hStream->ReadSome(buffer);
    }

    bool
    IONetworkStream::Write(std::byte c) {
        return this->hStream->Write(c);
    }

    size_t
    IONetworkStream::WriteSome(std::span<const std::byte> buffer) {
        return this->hStream->WriteSome(buffer);
    }

    bool
    IONetworkStream::PutBack(std::byte c) {
        return this->hStream->PutBack(c);
    }

    IPv4::Addr::Addr(in_addr_t uAddress, in_port_t uPort) :
        sockaddr_in {
            .sin_family = AF_INET,
            .sin_port   = htons(uPort),
            .sin_addr   = {uAddress},
            .sin_zero   = {}
        } {}

    IPv4::Addr::Addr(std::string_view strvAddress, std::string_view strvService) {
        struct addrinfo
            hints   = {
                .ai_flags       = AI_PASSIVE,
                .ai_family      = AF_INET,
                .ai_socktype    = SOCK_STREAM,
                .ai_protocol    = 0,
                .ai_addrlen     = 0,
                .ai_addr        = nullptr,
                .ai_canonname   = nullptr,
                .ai_next        = nullptr
            },
            *lpResult;

        int
            errcode = getaddrinfo(
                        strvAddress.data(),
                        strvService.data(),
                        &hints,
                        &lpResult);
        switch (errcode) { // maybe expand later
        case 0:
            break;

        default:
            throw std::runtime_error("failed to get the IPv4 address from string");
        }

        if (lpResult->ai_family != AF_INET || lpResult->ai_addrlen != sizeof(struct sockaddr_in))
            throw std::runtime_error("result address family isn't IPv4");

        const struct sockaddr_in*
            addr    = (const struct sockaddr_in*)lpResult->ai_addr;
        this->sin_family    = AF_INET;
        this->sin_addr      = addr->sin_addr;
        this->sin_port      = addr->sin_port;

        freeaddrinfo(lpResult);
    }

    bool
    IPv4::Addr::Connect(int fd) const noexcept {
        return
            connect(fd, (const struct sockaddr*)this, sizeof(Addr)) == 0;
    }

    bool
    IPv4::Addr::Bind(int fd) const noexcept {
        return
            bind(fd, (const struct sockaddr*)this, sizeof(Addr)) == 0;
    }

    std::string_view
    IPv4::Addr::ToString() {
        return inet_ntoa(this->sin_addr);
    }

    Local::Addr::Addr(std::string_view strvFilepath) :
        sockaddr_un{ .sun_family = AF_LOCAL, .sun_path = {} }
    {
        size_t
            uStrLen = std::min(
                        strvFilepath.size(),
                        sizeof(this->sun_path) - 1);
        for (size_t i = 0; i != uStrLen; ++i) {
            this->sun_path[i] = strvFilepath[i];
        }
        this->sun_path[uStrLen] = '\0';
    }

    bool
    Local::Addr::Connect(int fd) const noexcept {
        return
            access(this->sun_path, F_OK) == 0 &&
            connect(fd, (const struct sockaddr*)this, sizeof(Addr)) == 0;
    }

    bool
    Local::Addr::Bind(int fd) const noexcept {
        return
            unlink(this->sun_path) == 0 &&
            bind(fd, (const struct sockaddr*)this, sizeof(Addr)) == 0;
    }


}