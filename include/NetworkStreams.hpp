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
        template<typename AddressT, typename StreamT> requires
            std::derived_from<StreamT, io::__impl::StreamState>
        class BasicClient {
        public:
            BasicClient() :
                fdClient(socket(AddressT::AddressFamily, SOCK_STREAM, 0))
            {
                if (this->fdClient < 0) {
                    throw std::runtime_error("failed to create a stream socket");
                }
            }

            BasicClient(const BasicClient&) = delete;
            BasicClient(BasicClient&& obj) noexcept {
                this->fdClient  = obj.fdClient;
                obj.fdClient    = -1;
            }

            BasicClient&
            operator=(const BasicClient&) = delete;
            BasicClient&
            operator=(BasicClient&& obj) noexcept {
                BasicClient
                    temp    = std::move(obj);
                std::swap(this->fdClient, temp.fdClient);
                return *this;
            }

            std::optional<StreamT>
            Connect(const AddressT& addr) {
                if (connect(this->fdClient, (const struct sockaddr*)&addr, sizeof(AddressT)) != 0)
                    return std::nullopt;

                return StreamT(dup(this->fdClient));
            }

            ~BasicClient() {
                if (this->fdClient >= 0) {
                    shutdown(this->fdClient, SHUT_RDWR);
                    close(this->fdClient);
                }
            }

        private:
            int
                fdClient = -1;
        };

        template<typename AddressT, typename StreamT> requires
            std::derived_from<StreamT, io::__impl::StreamState>
        class BasicServer {
        public:
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

            std::optional<std::pair<StreamT, AddressT>>
            Accept() {
                AddressT
                    addrAccept;
                socklen_t
                    uSockAddrlen;
                int
                    fdAccept    = accept(this->fdServer, (struct sockaddr*)&addrAccept, &uSockAddrlen);
                if (fdAccept < 0)
                    return std::nullopt;

                return std::pair<StreamT, AddressT>{ StreamT(fdAccept), addrAccept };
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
        
        class NetworkStreamBase :
            virtual public  StreamState
        {
        public:
            NetworkStreamBase(const NetworkStreamBase&) = delete;
            NetworkStreamBase(NetworkStreamBase&& obj) noexcept {
                this->fdSocket  = obj.fdSocket;
                obj.fdSocket    = -1;
            }

            NetworkStreamBase&
            operator=(const NetworkStreamBase&) = delete;
            NetworkStreamBase&
            operator=(NetworkStreamBase&& obj) noexcept {
                NetworkStreamBase
                    temp    = std::move(obj);
                std::swap(
                    this->fdSocket, temp.fdSocket);
                return *this;
            }

            NetworkStreamBase(int fdSocket) :
                fdSocket(fdSocket),
                bEOF(false),
                bErr(false)
            {
                if (fdSocket < 0)
                    throw std::runtime_error("invalid socket handle");
            }

            [[nodiscard]] bool
            EndOfStream() const noexcept override {
                return (bool)this->bEOF;
            }

            [[nodiscard]] bool
            Good() const noexcept override {
                return !(bool)this->bErr;
            }

            void
            ClearFlags() noexcept override {
                this->bEOF  = 0;
                this->bErr  = 0;
            }

            int
            Handle() const noexcept {
                return this->fdSocket;
            }

            ~NetworkStreamBase() {
                if (this->fdSocket >= 0) {
                    shutdown(this->fdSocket, SHUT_RDWR);
                    close(this->fdSocket);
                }
            }

        protected:
            size_t
            ReadSome(std::span<std::byte> buffer) {
                this->ClearFlags();
                size_t
                    i = 0;

                if (uRSize != 0) {
                    size_t
                        uReadCount  = std::min<size_t>(this->uRSize, buffer.size());
                    while (i != uReadCount) {
                        buffer[i]       = this->lpRBuf[this->uRSize - 1];
                        this->uRSize    -= 1;
                        i               += 1;
                    }
                }

                if (i == buffer.size())
                    return i;

                ssize_t
                    iReadSize   = recv(
                                    this->fdSocket,
                                    buffer.data() + i, buffer.size() - i,
                                    0);
                if (iReadSize < 0) {
                    this->bErr  = true;
                    return i;
                }

                size_t
                    uReadSize   = (size_t)iReadSize;

                if (uReadSize < buffer.size()) {
                    this->bEOF  = true;
                }

                return (size_t)iReadSize + i;
            }

            size_t
            WriteSome(std::span<const std::byte> buffer) {
                this->ClearFlags();
                this->uRSize    = 0;
                
                ssize_t
                    iWriteSize  = send(
                                    this->fdSocket,
                                    buffer.data(), buffer.size(),
                                    0);
                if (iWriteSize < 0) {
                    this->bErr  = true;
                    return 0;
                }

                size_t
                    uWriteSize  = (size_t)iWriteSize;
                if (uWriteSize < buffer.size()) {
                    this->bEOF  = true;
                }

                return uWriteSize;
            }

            std::optional<std::byte>
            Read() {
                std::byte c;
                if (this->ReadSome({&c, 1}) == 0)
                    return std::nullopt;

                return c;
            }

            bool
            Write(std::byte c) {
                return this->WriteSome({&c, 1}) != 0;
            }

            bool
            PutBack(std::byte c) {
                if (this->uRSize == sizeof(this->lpRBuf))
                    return false;
                this->lpRBuf[this->uRSize] = c;
                this->uRSize += 1;
                return true;
            }

        private:
            int
                fdSocket = -1;
            uint8_t
                bEOF    : 1,
                bErr    : 1,
                uRSize  : 6;
            std::byte
                lpRBuf[std::max(alignof(int), 2uz) - 1];
        };
    }

    class INetworkStream :
        public  SerialIStream,
        public  __impl::NetworkStreamBase {
    public:
        INetworkStream(int fdSocket) :
            NetworkStreamBase(fdSocket)
        {
            shutdown(this->Handle(), SHUT_WR);
        }
        
        std::optional<std::byte>
        Read() override {
            return this->NetworkStreamBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->NetworkStreamBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->NetworkStreamBase::PutBack(c);
        }
    };

    class ONetworkStream :
        public  SerialOStream,
        public  __impl::NetworkStreamBase {
    public:
        ONetworkStream(int fdSocket) :
            NetworkStreamBase(fdSocket)
        {
            shutdown(this->Handle(), SHUT_RD);
        }

        bool
        Write(std::byte c) override {
            return this->NetworkStreamBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->NetworkStreamBase::WriteSome(buffer);
        }
    };

    class IONetworkStream :
        public  SerialIOStream,
        public  __impl::NetworkStreamBase {
    public:
        IONetworkStream(int fdSocket) :
            NetworkStreamBase(fdSocket) {}

        std::optional<std::byte>
        Read() override {
            return this->NetworkStreamBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->NetworkStreamBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->NetworkStreamBase::PutBack(c);
        }

        bool
        Write(std::byte c) override {
            return this->NetworkStreamBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->NetworkStreamBase::WriteSome(buffer);
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
            __impl::BasicClient<IPv4::Address, INetworkStream>;
        using ONetworkClient    =
            __impl::BasicClient<IPv4::Address, ONetworkStream>;
        using IONetworkClient   =
            __impl::BasicClient<IPv4::Address, IONetworkStream>;
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
            __impl::BasicClient<Local::Address, INetworkStream>;
        using ONetworkClient    =
            __impl::BasicClient<Local::Address, ONetworkStream>;
        using IONetworkClient   =
            __impl::BasicClient<Local::Address, IONetworkStream>;
    }
}