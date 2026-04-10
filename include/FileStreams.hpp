static_assert(__cplusplus >= 202302, "requires C++23 minimum version");

#pragma once
#include <cstdio>
#include <string_view>

#include "IOStreams.hpp"


namespace io {
    namespace __impl {
        class SerialFileStreamViewBase :
            virtual public  StreamState {
        public:
            SerialFileStreamViewBase(FILE* handle);

            [[nodiscard]] bool
            EndOfStream() const noexcept override;

            [[nodiscard]] bool
            Good() const noexcept override;

            void
            ClearFlags() noexcept override;

            [[nodiscard]] FILE*
            Handle() const noexcept;

            bool
            Flush() noexcept override;

        protected:
            std::optional<std::byte>
            Read();

            size_t
            ReadSome(std::span<std::byte> buffer);

            bool
            Write(std::byte c);

            size_t
            WriteSome(std::span<const std::byte> buffer);

            bool
            PutBack(std::byte c);

            FILE*
                handle = nullptr;
        };

        class FileStreamViewBase :
            virtual public StreamPosition,
            public SerialFileStreamViewBase {
        public:
            FileStreamViewBase(FILE* handle);

            [[nodiscard]] intptr_t
            GetPosition() const noexcept override;

            bool
            SetPosition(intptr_t offset, StreamOffsetOrigin from = StreamOffsetOrigin::StreamStart) override;
        };

        class SerialFileStreamBase :
            public SerialFileStreamViewBase {
        public:
            SerialFileStreamBase(const SerialFileStreamBase&) = delete;
            SerialFileStreamBase(SerialFileStreamBase&& obj) noexcept;

            SerialFileStreamBase&
            operator=(const SerialFileStreamBase&) = delete;
            SerialFileStreamBase&
            operator=(SerialFileStreamBase&& obj) noexcept;

            SerialFileStreamBase(std::string_view strvFilename, std::string_view strvMode);

            ~SerialFileStreamBase() noexcept;
        };


        class FileStreamBase :
            public FileStreamViewBase {
        public:
            FileStreamBase(const FileStreamBase&) = delete;
            FileStreamBase(FileStreamBase&& obj) noexcept;

            FileStreamBase&
            operator=(const FileStreamBase&) = delete;
            FileStreamBase&
            operator=(FileStreamBase&& obj) noexcept;

            FileStreamBase(std::string_view strvFilename, std::string_view strvMode);

            ~FileStreamBase() noexcept;
        };
    }

    class IFileStreamView :
        public  IStream,
        public  __impl::FileStreamViewBase {
    public:
        IFileStreamView(FILE* handle);
        
        std::optional<std::byte>
        Read() override;

        size_t
        ReadSome(std::span<std::byte> buffer) override;

        bool
        PutBack(std::byte c) override;
    };

    class OFileStreamView :
        public  OStream,
        public  __impl::FileStreamViewBase {
    public:
        OFileStreamView(FILE* handle);
        
        bool
        Write(std::byte c) override;

        size_t
        WriteSome(std::span<const std::byte> buffer) override;
    };

    class IOFileStreamView :
        public  IOStream,
        public  __impl::FileStreamViewBase {
    public:
        IOFileStreamView(FILE* handle);

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

    class IFileStream :
        public  IStream,
        public  __impl::FileStreamBase {
    public:
        IFileStream(std::string_view strvFilename, bool bIsBinaryFile = false);

        std::optional<std::byte>
        Read() override;

        size_t
        ReadSome(std::span<std::byte> buffer) override;

        bool
        PutBack(std::byte c) override;
    };

    class OFileStream :
        public  OStream,
        public  __impl::FileStreamBase {
    public:
        OFileStream(std::string_view strvFilename, bool bIsBinaryFile = false);

        bool
        Write(std::byte c) override;

        size_t
        WriteSome(std::span<const std::byte> buffer) override;
    };

    class IOFileStream :
        public  IOStream,
        public  __impl::FileStreamBase {
    public:
        IOFileStream(std::string_view strvFilename, bool bIsBinaryFile = false);

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
    };

    class SerialIFileStreamView :
        public  SerialIStream,
        public  __impl::SerialFileStreamViewBase {
    public:
        SerialIFileStreamView(FILE* handle);
        
        std::optional<std::byte>
        Read() override;

        size_t
        ReadSome(std::span<std::byte> buffer) override;

        bool
        PutBack(std::byte c) override;
    };

    class SerialOFileStreamView :
        public  SerialOStream,
        public  __impl::SerialFileStreamViewBase {
    public:
        SerialOFileStreamView(FILE* handle);
        
        bool
        Write(std::byte c) override;

        size_t
        WriteSome(std::span<const std::byte> buffer) override;
    };

    class SerialIOFileStreamView :
        public  SerialIOStream,
        public  __impl::SerialFileStreamViewBase {
    public:
        SerialIOFileStreamView(FILE* handle);

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

    class SerialIFileStream :
        public  SerialIStream,
        public  __impl::SerialFileStreamBase {
    public:
        SerialIFileStream(std::string_view strvFilename, bool bIsBinaryFile = false);

        std::optional<std::byte>
        Read() override;

        size_t
        ReadSome(std::span<std::byte> buffer) override;

        bool
        PutBack(std::byte c) override;
    };

    class SerialOFileStream :
        public  SerialOStream,
        public  __impl::SerialFileStreamBase {
    public:
        SerialOFileStream(std::string_view strvFilename, bool bIsBinaryFile = false);

        bool
        Write(std::byte c) override;

        size_t
        WriteSome(std::span<const std::byte> buffer) override;
    };

    class SerialIOFileStream :
        public  SerialIOStream,
        public  __impl::SerialFileStreamBase {
    public:
        SerialIOFileStream(std::string_view strvFilename, bool bIsBinaryFile = false);

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
    };
}

