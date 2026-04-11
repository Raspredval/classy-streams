static_assert(__cplusplus >= 202302, "requires C++23 minimum version");

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

            BufferedNetworkStream(int fdSocket);

            ~BufferedNetworkStream() noexcept;

            std::optional<std::byte>
            Read() noexcept;

            bool
            Write(std::byte c) noexcept;

            size_t
            ReadSome(std::span<std::byte> buffer) noexcept;

            size_t
            WriteSome(std::span<const std::byte> buffer) noexcept;

            bool
            PutBack(std::byte c) noexcept;

            bool
            Flush() noexcept;

            void
            ClearFlags() noexcept;

            bool
            EndOfStream() const noexcept;

            bool
            Error() const noexcept;

            int
            Descriptor() const noexcept;

        private:
            bool
            GetInput();

            struct InputBuffer {
                static constexpr size_t
                    uBufCap     = sizeof(size_t) * 1024;
                std::byte*
                    lpData      = nullptr;
                size_t
                    uBegin      = 0,
                    uEnd        = 0;
            } i;

            struct OutputBuffer {
                static constexpr size_t
                    uBufCap     = sizeof(size_t) * 1024;
                std::byte*
                    lpData      = nullptr;
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
                    lpRetBuf[sizeof(int) - 1];
            } s;
        };

        class NetworkStreamViewBase :
            virtual public StreamState {
        public:
            NetworkStreamViewBase() = default;
            NetworkStreamViewBase(BufferedNetworkStream* hStream);

            [[nodiscard]] bool
            EndOfStream() const noexcept override;

            [[nodiscard]] bool
            Good() const noexcept override;

            bool
            Flush() noexcept override;

            void
            ClearFlags() noexcept override;

            BufferedNetworkStream*
            Handle() const noexcept;

        protected:
            BufferedNetworkStream*
                hStream = nullptr;
        };

        class NetworkStreamBase :
            public NetworkStreamViewBase {
        public:
            NetworkStreamBase(int fdSocket);

            NetworkStreamBase(const NetworkStreamBase&) = delete;

            NetworkStreamBase(NetworkStreamBase&& obj) noexcept;

            NetworkStreamBase&
            operator=(const NetworkStreamBase&) = delete;

            NetworkStreamBase&
            operator=(NetworkStreamBase&& obj) noexcept;

            ~NetworkStreamBase() noexcept;
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
                if (addr.Connect(fdClient)) {
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

                if (!addr.Bind(this->fdServer)) {
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
        INetworkStreamView(__impl::BufferedNetworkStream* hStream);

        std::optional<std::byte>
        Read() override;

        size_t
        ReadSome(std::span<std::byte> buffer) override;

        bool
        PutBack(std::byte c) override;
    };

    class ONetworkStreamView :
        public SerialOStream,
        public __impl::NetworkStreamViewBase {
    public:
        ONetworkStreamView(__impl::BufferedNetworkStream* hStream);

        bool
        Write(std::byte c) override;

        size_t
        WriteSome(std::span<const std::byte> buffer) override;
    };

    class IONetworkStreamView :
        public SerialIOStream,
        public __impl::NetworkStreamViewBase {
    public:
        IONetworkStreamView(__impl::BufferedNetworkStream* hStream);

        std::optional<std::byte>
        Read() override;

        size_t
        ReadSome(std::span<std::byte> buffer) override;

        bool
        PutBack(std::byte c) override;

        bool
        Write(std::byte c) override;

        size_t
        WriteSome(std::span<const std::byte> buffer) override;
    };

    class INetworkStream :
        public SerialIStream,
        public __impl::NetworkStreamBase {
    public:
        INetworkStream(int fdSocket);

        std::optional<std::byte>
        Read() override;

        size_t
        ReadSome(std::span<std::byte> buffer) override;

        bool
        PutBack(std::byte c) override;
    };

    class ONetworkStream :
        public SerialOStream,
        public __impl::NetworkStreamBase {
    public:
        ONetworkStream(int fdSocket);

        bool
        Write(std::byte c) override;

        size_t
        WriteSome(std::span<const std::byte> buffer) override;
    };

    class IONetworkStream :
        public SerialIOStream,
        public __impl::NetworkStreamBase {
    public:
        IONetworkStream(int fdSocket);

        std::optional<std::byte>
        Read() override;

        size_t
        ReadSome(std::span<std::byte> buffer) override;

        bool
        Write(std::byte c) override;

        size_t
        WriteSome(std::span<const std::byte> buffer) override;

        bool
        PutBack(std::byte c) override;
    };

    namespace IPv4 {
        struct Addr :
            public sockaddr_in
        {
            static constexpr sa_family_t
                AddressFamily   = AF_INET;

            Addr() = default;

            Addr(in_addr_t uAddress, in_port_t uPort);

            Addr(in_port_t uPort) :
                Addr(INADDR_ANY, uPort) {}

            Addr(std::string_view strvAddress, std::string_view strvService = {});

            [[nodiscard]] bool
            Connect(int fd) const noexcept;

            [[nodiscard]] bool
            Bind(int fd) const noexcept;

            std::string_view
            ToString();
        };

        using INetworkServer    =
            __impl::BasicServer<IPv4::Addr, INetworkStream>;
        using ONetworkServer    =
            __impl::BasicServer<IPv4::Addr, ONetworkStream>;
        using IONetworkServer   =
            __impl::BasicServer<IPv4::Addr, IONetworkStream>;

        using INetworkClient    =
            __impl::BasicClient<IPv4::Addr, INetworkStreamView>;
        using ONetworkClient    =
            __impl::BasicClient<IPv4::Addr, ONetworkStreamView>;
        using IONetworkClient   =
            __impl::BasicClient<IPv4::Addr, IONetworkStreamView>;
    }

    namespace Local {
        struct Addr :
            public sockaddr_un
        {
            static constexpr sa_family_t
                AddressFamily   = AF_LOCAL;

            Addr() = default;

            Addr(std::string_view strvFilepath);

            [[nodiscard]] bool
            Connect(int fd) const noexcept;

            [[nodiscard]] bool
            Bind(int fd) const noexcept;
        };

        using INetworkServer    =
            __impl::BasicServer<Local::Addr, INetworkStream>;
        using ONetworkServer    =
            __impl::BasicServer<Local::Addr, ONetworkStream>;
        using IONetworkServer   =
            __impl::BasicServer<Local::Addr, IONetworkStream>;

        using INetworkClient    =
            __impl::BasicClient<Local::Addr, INetworkStreamView>;
        using ONetworkClient    =
            __impl::BasicClient<Local::Addr, ONetworkStreamView>;
        using IONetworkClient   =
            __impl::BasicClient<Local::Addr, IONetworkStreamView>;
    }
}