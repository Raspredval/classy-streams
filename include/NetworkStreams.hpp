#pragma once
#include "IOStreams.hpp"

#include <string_view>
#include <stdexcept>
#include <optional>

#include <netinet/in.h>                                                                                          
#include <sys/unistd.h>                                                                                          
#include <sys/socket.h>                                                                                          
#include <arpa/inet.h>                                                                                           
#include <sys/un.h>
#include <netdb.h>


namespace io {
    namespace __impl {
        class BufferedNetworkStream {
        public:
            BufferedNetworkStream() = delete;

            BufferedNetworkStream(
                const BufferedNetworkStream&) = delete;
            
            BufferedNetworkStream(
                BufferedNetworkStream&&) noexcept = delete;
            
            BufferedNetworkStream&
            operator=(const BufferedNetworkStream&) = delete;
            
            BufferedNetworkStream&
            operator=(BufferedNetworkStream&&) noexcept = delete;

            BufferedNetworkStream(int fdSocket) {
                if (fdSocket < 0)
                    throw std::runtime_error("failed to create a socket");

                this->s.fdSocket    = fdSocket;
            }

            ~BufferedNetworkStream() noexcept {
                shutdown(this->s.fdSocket, SHUT_RD);
                this->Flush();
                shutdown(this->s.fdSocket, SHUT_RDWR);
                close(this->s.fdSocket);
            }

            std::optional<std::byte>
            Read() noexcept {
                if (this->s.uRetLen != 0)
                    return this->s.lpRetBuf[--this->s.uRetLen];
                
                if (this->i.uBegin == this->i.uEnd) {
                    if (!this->GetInput())
                        return std::nullopt;
                }

                return this->i.lpData[this->i.uBegin++];
            }

            bool
            Write(std::byte c) noexcept {
                if (this->o.uSize == sizeof(o.lpData)) {
                    if (!this->Flush())
                        return false;
                }

                this->o.lpData[this->o.uSize++] = c;
                return true;
            }

            size_t
            ReadSome(std::span<std::byte> buffer) noexcept {
                for (size_t i = 0; i != buffer.size(); ++i) {
                    std::optional<std::byte>
                        optc    = this->Read();
                    if (!optc)
                        return i;
                }

                return buffer.size();
            }

            size_t
            WriteSome(std::span<const std::byte> buffer) noexcept {
                for (size_t i = 0; i != buffer.size(); ++i) {
                    if (!this->Write(buffer[i]))
                        return i;
                }

                return buffer.size();
            }

            bool
            PutBack(std::byte c) noexcept {
                if (this->s.uRetLen == sizeof(this->s.lpRetBuf))
                    return false;

                this->s.lpRetBuf[this->s.uRetLen++] = c;
                return true;
            }

            bool
            Flush() noexcept {
                if (this->o.uSize == 0)
                    return false;

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
            ClearFlags() noexcept {
                this->s.bEOF    = false;
                this->s.bErr    = false;
            }

            bool
            EndOfStream() const noexcept {
                return (bool)this->s.bEOF;
            }

            bool
            Error() const noexcept {
                return (bool)this->s.bEOF;
            }

            int
            Descriptor() const noexcept {
                return this->s.fdSocket;
            }

        private:
            bool
            GetInput() {
                if (this->i.uBegin != this->i.uEnd)
                    return false;

                ssize_t
                    iInputSize  = recv(
                                    this->s.fdSocket,
                                    this->i.lpData,
                                    sizeof(this->i.lpData),
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

            struct InputBuffer {
                std::byte
                    lpData[sizeof(size_t) * 32];
                size_t
                    uBegin      = 0,
                    uEnd        = 0;
            } i;

            struct OutputBuffer {
                std::byte
                    lpData[sizeof(size_t) * 32];
                size_t
                    uSize       = 0;
            } o;
            
            struct State {
                int
                    fdSocket    = -1;
                uint8_t
                    bEOF    : 1 = false,
                    bErr    : 1 = false,
                    uRetLen : 6 = 0;
                std::byte
                    lpRetBuf[std::max(sizeof(int), 2uz) - 1];
            } s;
        };

        class NetworkStreamViewBase :
            virtual public StreamState {
        public:
            NetworkStreamViewBase() = default;
            NetworkStreamViewBase(BufferedNetworkStream* hSocket) :
                hStream(hSocket) {}

            [[nodiscard]] bool
            EndOfStream() const noexcept override {
                return this->hStream->EndOfStream();
            }

            [[nodiscard]] bool
            Good() const noexcept override {
                return !this->hStream->Error();
            }

            bool
            Flush() noexcept override {
                return this->hStream->Flush();
            }

            void
            ClearFlags() noexcept override {
                return this->hStream->ClearFlags();
            }
        
            BufferedNetworkStream*
            Handle() const noexcept {
                return this->hStream;
            }

        protected:
            BufferedNetworkStream*
                hStream = nullptr;
        };

        class NetworkStreamBase :
            public NetworkStreamViewBase {
        public:
            NetworkStreamBase(int fdSocket) :
                NetworkStreamViewBase(new BufferedNetworkStream(fdSocket)) {}

            NetworkStreamBase(const NetworkStreamBase&) = delete;

            NetworkStreamBase(NetworkStreamBase&& obj) noexcept {
                this->hStream   = obj.hStream;
                obj.hStream     = nullptr;
            }

            NetworkStreamBase&
            operator=(const NetworkStreamBase&) = delete;

            NetworkStreamBase&
            operator=(NetworkStreamBase&& obj) noexcept {
                NetworkStreamBase
                    temp    = std::move(obj);
                std::swap(this->hStream, temp.hStream);
                return *this;
            }

            ~NetworkStreamBase() noexcept {
                if (this->hStream != nullptr) {
                    delete this->hStream;
                    this->hStream   = nullptr;
                }
            }
        };

        template<typename AddressT, typename StreamViewT> requires
            std::derived_from<StreamViewT, NetworkStreamViewBase>
        class BasicClient {
        public:
            using AddressType       =
                AddressT;
            using StreamViewType    =
                StreamViewT;
            using ConnectionType    =
                std::optional<StreamViewT>;

            BasicClient() :
                stream(socket(AddressT::AddressFamily, SOCK_STREAM, 0)) {}

            ConnectionType
            Connect(const AddressT& addr) {
                ConnectionType
                    connection = std::nullopt;
                
                int
                    fdClient    = this->stream.Handle()->Descriptor();
                if (connect(fdClient, (const struct sockaddr*)&addr, sizeof(AddressT)) == 0) {
                    connection.emplace(
                        StreamViewT(this->stream.Handle()));
                }

                return connection;
            }

        private:
            NetworkStreamBase
                stream;
        };

        template<typename AddressT, typename StreamT> requires
            std::derived_from<StreamT, NetworkStreamBase>
        class BasicServer {
        public:
            using AddressType       =
                AddressT;
            using StreamType        =
                StreamT;
            using ConnectionType    =
                std::optional<std::pair<StreamT, AddressT>>;

            BasicServer(const AddressT& addr, int iPendingConnections = 32) :
                fdServer(socket(AddressT::AddressFamily, SOCK_STREAM, 0))
            {
                if (fdServer < 0) {
                    throw std::runtime_error("failed to create server socket");
                }

                if (bind(this->fdServer, (const struct sockaddr*)&addr, sizeof(AddressT)) != 0) {
                    throw std::runtime_error("failed to bind the server socket to an address");
                }

                if (listen(this->fdServer, iPendingConnections) != 0) {
                    throw std::runtime_error("failed to set the server socket into listening mode");
                }
            }

            BasicServer(const BasicServer&) = delete;
            BasicServer(BasicServer&& obj) noexcept {
                this->fdServer  = obj.fdServer;
                obj.fdServer    = -1;
            }

            BasicServer&
            operator=(const BasicServer&) = delete;
            BasicServer&
            operator=(BasicServer&& obj) noexcept {
                BasicServer
                    temp    = std::move(obj);
                std::swap(this->fdServer, temp.fdServer);
                return *this;
            }

            ConnectionType
            Accept() {
                ConnectionType
                    connection  = std::nullopt;

                AddressT
                    addrAccept;
                socklen_t
                    uSockAddrlen;
                int
                    fdAccept    = accept(this->fdServer, (struct sockaddr*)&addrAccept, &uSockAddrlen);
                if (fdAccept >= 0) {
                    connection.emplace(
                        StreamT(fdAccept),
                        addrAccept);
                }
                
                return connection;
            }

            ~BasicServer() {
                if (this->fdServer >= 0) {
                    shutdown(this->fdServer, SHUT_RDWR);
                    close(this->fdServer);
                }
            }

        private:
            int
                fdServer = -1;
        };
    }

    class INetworkStreamView :
        public SerialIStream,
        public __impl::NetworkStreamViewBase {
    public:
        INetworkStreamView(__impl::BufferedNetworkStream* hStream) :
            NetworkStreamViewBase(hStream) {
                shutdown(this->hStream->Descriptor(), SHUT_WR);
            }

        std::optional<std::byte>
        Read() override {
            return this->hStream->Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->hStream->ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->hStream->PutBack(c);
        }
    };

    class ONetworkStreamView :
        public SerialOStream,
        public __impl::NetworkStreamViewBase {
    public:
        ONetworkStreamView(__impl::BufferedNetworkStream* hStream) :
            NetworkStreamViewBase(hStream) {
                shutdown(this->hStream->Descriptor(), SHUT_RD);
            }

        bool
        Write(std::byte c) override {
            return this->hStream->Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->hStream->WriteSome(buffer);
        }
    };

    class IONetworkStreamView :
        public SerialIOStream,
        public __impl::NetworkStreamViewBase {
    public:
        IONetworkStreamView(__impl::BufferedNetworkStream* hStream) :
            NetworkStreamViewBase(hStream) {}
        
        std::optional<std::byte>
        Read() override {
            return this->hStream->Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->hStream->ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->hStream->PutBack(c);
        }

        bool
        Write(std::byte c) override {
            return this->hStream->Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->hStream->WriteSome(buffer);
        }
    };

    class INetworkStream :
        public SerialIStream,
        public __impl::NetworkStreamBase {
    public:
        INetworkStream(int fdSocket) :
            NetworkStreamBase(fdSocket) {
                shutdown(this->hStream->Descriptor(), SHUT_WR);
            }

        std::optional<std::byte>
        Read() override {
            return this->hStream->Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->hStream->ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->hStream->PutBack(c);
        }
    };

    class ONetworkStream :
        public SerialOStream,
        public __impl::NetworkStreamBase {
    public:
        ONetworkStream(int fdSocket) :
            NetworkStreamBase(fdSocket) {
                shutdown(this->hStream->Descriptor(), SHUT_RD);
            }

        bool
        Write(std::byte c) override {
            return this->hStream->Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->hStream->WriteSome(buffer);
        }
    };

    class IONetworkStream :
        public SerialIOStream,
        public __impl::NetworkStreamBase {
    public:
        IONetworkStream(int fdSocket) :
            NetworkStreamBase(fdSocket) {}
        
        std::optional<std::byte>
        Read() override {
            return this->hStream->Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->hStream->ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->hStream->PutBack(c);
        }

        bool
        Write(std::byte c) override {
            return this->hStream->Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->hStream->WriteSome(buffer);
        }
    };

    namespace IPv4 {
        struct Address :
        public sockaddr_in
        {
            static constexpr sa_family_t
                AddressFamily   = AF_INET;

            Address() = default;

            Address(in_addr_t uAddress, in_port_t uPort) :
                sockaddr_in {
                    .sin_family = AF_INET,
                    .sin_port   = htons(uPort),
                    .sin_addr   = {uAddress},
                    .sin_zero   = {}
                } {}

            Address(in_port_t uPort) :
                Address(INADDR_ANY, uPort) {}

            Address(std::string_view strvAddress, std::string_view strvService = {}) {
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

            std::string_view
            ToString() {
                return inet_ntoa(this->sin_addr);
            }
        };

        using INetworkServer    =
            __impl::BasicServer<IPv4::Address, INetworkStream>;
        using ONetworkServer    =
            __impl::BasicServer<IPv4::Address, ONetworkStream>;
        using IONetworkServer   =
            __impl::BasicServer<IPv4::Address, IONetworkStream>;

        using INetworkClient    =
            __impl::BasicClient<IPv4::Address, INetworkStreamView>;
        using ONetworkClient    =
            __impl::BasicClient<IPv4::Address, ONetworkStreamView>;
        using IONetworkClient   =
            __impl::BasicClient<IPv4::Address, IONetworkStreamView>;
    }

    namespace Local {
        struct Address :
            public sockaddr_un
        {
            static constexpr sa_family_t
                AddressFamily   = AF_LOCAL;
            
            Address() = default;

            Address(std::string_view strvFilepath) :
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
        };

        using INetworkServer    =
            __impl::BasicServer<Local::Address, INetworkStream>;
        using ONetworkServer    =
            __impl::BasicServer<Local::Address, ONetworkStream>;
        using IONetworkServer   =
            __impl::BasicServer<Local::Address, IONetworkStream>;

        using INetworkClient    =
            __impl::BasicClient<Local::Address, INetworkStreamView>;
        using ONetworkClient    =
            __impl::BasicClient<Local::Address, ONetworkStreamView>;
        using IONetworkClient   =
            __impl::BasicClient<Local::Address, IONetworkStreamView>;
    }
}