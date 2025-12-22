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
            using AddressType       =
                AddressT;
            using StreamType        =
                StreamT;
            using ConnectionType    =
                std::optional<StreamType>;

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

            ConnectionType
            Connect(const AddressT& addr) {
                if (connect(this->fdClient, (const struct sockaddr*)&addr, sizeof(AddressT)) != 0)
                    return std::nullopt;

                return StreamT(this->fdClient);
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
        
        class BufferedNetworkStream {
        public:
            // TODO the rest of the buffered stream, lmao
        };

        // TODO: NetworkStreamViewBase
        // TODO: NetworkStreamBase
    }

    class INonOwningNetworkStream :
        public  SerialIStream,
        public  __impl::NonOwningNetworkStreamBase {
    public:
        INonOwningNetworkStream(int fdSocket) :
            NonOwningNetworkStreamBase(fdSocket)
        {
            shutdown(this->Handle(), SHUT_WR);
        }
        
        std::optional<std::byte>
        Read() override {
            return this->NonOwningNetworkStreamBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->NonOwningNetworkStreamBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->NonOwningNetworkStreamBase::PutBack(c);
        }
    };

    class ONonOwningNetworkStream :
        public  SerialOStream,
        public  __impl::NonOwningNetworkStreamBase {
    public:
        ONonOwningNetworkStream(int fdSocket) :
            NonOwningNetworkStreamBase(fdSocket)
        {
            shutdown(this->Handle(), SHUT_RD);
        }

        bool
        Write(std::byte c) override {
            return this->NonOwningNetworkStreamBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->NonOwningNetworkStreamBase::WriteSome(buffer);
        }
    };

    class IONonOwningNetworkStream :
        public  SerialIOStream,
        public  __impl::NonOwningNetworkStreamBase {
    public:
        IONonOwningNetworkStream(int fdSocket) :
            NonOwningNetworkStreamBase(fdSocket) {}

        std::optional<std::byte>
        Read() override {
            return this->NonOwningNetworkStreamBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->NonOwningNetworkStreamBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->NonOwningNetworkStreamBase::PutBack(c);
        }

        bool
        Write(std::byte c) override {
            return this->NonOwningNetworkStreamBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->NonOwningNetworkStreamBase::WriteSome(buffer);
        }
    };

    class IOwningNetworkStream :
        public  SerialIStream,
        public  __impl::OwningNetworkStreamBase {
    public:
        IOwningNetworkStream(int fdSocket) :
            OwningNetworkStreamBase(fdSocket)
        {
            shutdown(this->Handle(), SHUT_WR);
        }
        
        std::optional<std::byte>
        Read() override {
            return this->OwningNetworkStreamBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->OwningNetworkStreamBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->OwningNetworkStreamBase::PutBack(c);
        }
    };

    class OOwningNetworkStream :
        public  SerialOStream,
        public  __impl::OwningNetworkStreamBase {
    public:
        OOwningNetworkStream(int fdSocket) :
            OwningNetworkStreamBase(fdSocket)
        {
            shutdown(this->Handle(), SHUT_RD);
        }

        bool
        Write(std::byte c) override {
            return this->OwningNetworkStreamBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->OwningNetworkStreamBase::WriteSome(buffer);
        }
    };

    class IOOwningNetworkStream :
        public  SerialIOStream,
        public  __impl::OwningNetworkStreamBase {
    public:
        IOOwningNetworkStream(int fdSocket) :
            OwningNetworkStreamBase(fdSocket) {}

        std::optional<std::byte>
        Read() override {
            return this->OwningNetworkStreamBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->OwningNetworkStreamBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->OwningNetworkStreamBase::PutBack(c);
        }

        bool
        Write(std::byte c) override {
            return this->OwningNetworkStreamBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->OwningNetworkStreamBase::WriteSome(buffer);
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
            __impl::BasicServer<IPv4::Address, IOwningNetworkStream>;
        using ONetworkServer    =
            __impl::BasicServer<IPv4::Address, OOwningNetworkStream>;
        using IONetworkServer   =
            __impl::BasicServer<IPv4::Address, IOOwningNetworkStream>;

        using INetworkClient    =
            __impl::BasicClient<IPv4::Address, INonOwningNetworkStream>;
        using ONetworkClient    =
            __impl::BasicClient<IPv4::Address, ONonOwningNetworkStream>;
        using IONetworkClient   =
            __impl::BasicClient<IPv4::Address, IONonOwningNetworkStream>;
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
            __impl::BasicServer<Local::Address, IOwningNetworkStream>;
        using ONetworkServer    =
            __impl::BasicServer<Local::Address, OOwningNetworkStream>;
        using IONetworkServer   =
            __impl::BasicServer<Local::Address, IOOwningNetworkStream>;

        using INetworkClient    =
            __impl::BasicClient<Local::Address, INonOwningNetworkStream>;
        using ONetworkClient    =
            __impl::BasicClient<Local::Address, ONonOwningNetworkStream>;
        using IONetworkClient   =
            __impl::BasicClient<Local::Address, IONonOwningNetworkStream>;
    }
}