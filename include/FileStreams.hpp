#pragma once
#include <cstdio>
#include <format>
#include <stdexcept>
#include <string_view>

#include "IOStreams.hpp"


namespace io {
    namespace __impl {
        class SerialFileStreamViewBase :
            virtual public  StreamState {
        public:
            SerialFileStreamViewBase(FILE* handle) :
                handle(handle) {}

            [[nodiscard]] bool
            EndOfStream() const noexcept override {
                return feof(this->handle);
            }

            [[nodiscard]] bool
            Good() const noexcept override {
                return !ferror(this->handle);
            }

            void
            ClearFlags() noexcept override {
                return clearerr(this->handle);
            }

            [[nodiscard]] FILE*
            Handle() const noexcept {
                return this->handle;
            }

            bool
            Flush() noexcept override {
                return fflush(this->handle) == 0;
            }

        protected:
            std::optional<std::byte>
            Read() {
                auto c =
                    fgetc(this->handle);
                return (c != EOF)
                    ? std::optional{ (std::byte)c }
                    : std::nullopt;
            }

            size_t
            ReadSome(std::span<std::byte> buffer) {
                return fread(
                    buffer.data(),
                    1, buffer.size(),
                    this->handle);
            }

            bool
            Write(std::byte c) {
                auto result =
                    fputc((int)c, this->handle);
                return result != EOF;
            }

            size_t
            WriteSome(std::span<const std::byte> buffer) {
                return fwrite(
                    buffer.data(),
                    1, buffer.size(),
                    this->handle);
            }

            bool
            PutBack(std::byte c) {
                return ungetc((int)c, this->handle) != EOF;
            }

            FILE*
                handle = nullptr;
        };

        class FileStreamViewBase :
            virtual public StreamPosition,
            public SerialFileStreamViewBase {
        public:
            FileStreamViewBase(FILE* hFile) :
                SerialFileStreamViewBase(hFile) {}

            [[nodiscard]] intptr_t
            GetPosition() const noexcept override {
                return ftell(this->handle);
            }

            bool
            SetPosition(
                intptr_t             offset,
                StreamOffsetOrigin  from = StreamOffsetOrigin::StreamStart) override
            {
                return !fseek(
                    this->handle,
                    offset,
                    (int)from);
            }
        };

        class SerialFileStreamBase :
            public SerialFileStreamViewBase {
        public:
            SerialFileStreamBase(const SerialFileStreamBase&) = delete;
            SerialFileStreamBase(SerialFileStreamBase&& obj) noexcept :
                SerialFileStreamViewBase(obj.handle)
            {
                obj.handle      = nullptr;
            }

            SerialFileStreamBase&
            operator=(const SerialFileStreamBase&) = delete;
            SerialFileStreamBase&
            operator=(SerialFileStreamBase&& obj) noexcept {
                SerialFileStreamBase
                    temp    = std::move(obj);
                std::swap(
                    this->handle, temp.handle);
                return *this;
            }

            SerialFileStreamBase(std::string_view strvFilename, std::string_view strvMode) :
                SerialFileStreamViewBase(fopen(strvFilename.data(), strvMode.data()))
            {
                if (this->handle == nullptr) {
                    throw std::runtime_error(std::format(
                        "failed to open file {} with mode {}",
                        strvFilename, strvMode));
                }
            }

            ~SerialFileStreamBase() noexcept {
                if (this->handle != nullptr)
                    fclose(this->handle);
            }
        };


        class FileStreamBase :
            public FileStreamViewBase {
        public:
            FileStreamBase(const FileStreamBase&) = delete;
            FileStreamBase(FileStreamBase&& obj) noexcept :
                FileStreamViewBase(obj.handle)
            {
                obj.handle      = nullptr;
            }

            FileStreamBase&
            operator=(const FileStreamBase&) = delete;
            FileStreamBase&
            operator=(FileStreamBase&& obj) noexcept {
                FileStreamBase
                    temp    = std::move(obj);
                std::swap(
                    this->handle, temp.handle);
                return *this;
            }

            FileStreamBase(std::string_view strvFilename, std::string_view strvMode) :
                FileStreamViewBase(fopen(strvFilename.data(), strvMode.data()))
            {
                if (this->handle == nullptr) {
                    throw std::runtime_error(std::format(
                        "failed to open file {} with mode {}",
                        strvFilename, strvMode));
                }
            }

            ~FileStreamBase() noexcept {
                if (this->handle != nullptr)
                    fclose(this->handle);
            }
        };
    }

    class IFileStreamView :
        public  IStream,
        public  __impl::FileStreamViewBase {
    public:
        IFileStreamView(FILE* handle) :
            FileStreamViewBase(handle) {}
        
        std::optional<std::byte>
        Read() override {
            return this->FileStreamViewBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->FileStreamViewBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->FileStreamViewBase::PutBack(c);
        }
    };

    class OFileStreamView :
        public  OStream,
        public  __impl::FileStreamViewBase {
    public:
        OFileStreamView(FILE* handle) :
            FileStreamViewBase(handle) {}
        
        bool
        Write(std::byte c) override {
            return this->FileStreamViewBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->FileStreamViewBase::WriteSome(buffer);
        }
    };

    class IOFileStreamView :
        public  IOStream,
        public  __impl::FileStreamViewBase {
    public:
        IOFileStreamView(FILE* handle) :
            FileStreamViewBase(handle) {}

        std::optional<std::byte>
        Read() override {
            return this->FileStreamViewBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->FileStreamViewBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->FileStreamViewBase::PutBack(c);
        }

        bool
        Write(std::byte c) override {
            return this->FileStreamViewBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->FileStreamViewBase::WriteSome(buffer);
        }
    };

    class IFileStream :
        public  IStream,
        public  __impl::FileStreamBase {
    public:
        IFileStream(std::string_view strvFilename) :
            FileStreamBase(strvFilename, "r") {}

        std::optional<std::byte>
        Read() override {
            return this->FileStreamBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->FileStreamBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->FileStreamBase::PutBack(c);
        }
    };

    class OFileStream :
        public  OStream,
        public  __impl::FileStreamBase {
    public:
        OFileStream(std::string_view strvFilename) :
            FileStreamBase(strvFilename, "w") {}

        bool
        Write(std::byte c) override {
            return this->FileStreamBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->FileStreamBase::WriteSome(buffer);
        }
    };

    class IOFileStream :
        public  IOStream,
        public  __impl::FileStreamBase {
    public:
        IOFileStream(std::string_view strvFilename) :
            FileStreamBase(strvFilename, "r+") {}

        bool
        Write(std::byte c) override {
            return this->FileStreamBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->FileStreamBase::WriteSome(buffer);
        }

        std::optional<std::byte>
        Read() override {
            return this->FileStreamBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->FileStreamBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->FileStreamBase::PutBack(c);
        }
    };

    class SerialIFileStreamView :
        public  SerialIStream,
        public  __impl::SerialFileStreamViewBase {
    public:
        SerialIFileStreamView(FILE* handle) :
            SerialFileStreamViewBase(handle) {}
        
        std::optional<std::byte>
        Read() override {
            return this->SerialFileStreamViewBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->SerialFileStreamViewBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->SerialFileStreamViewBase::PutBack(c);
        }
    };

    class SerialOFileStreamView :
        public  SerialOStream,
        public  __impl::SerialFileStreamViewBase {
    public:
        SerialOFileStreamView(FILE* handle) :
            SerialFileStreamViewBase(handle) {}
        
        bool
        Write(std::byte c) override {
            return this->SerialFileStreamViewBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->SerialFileStreamViewBase::WriteSome(buffer);
        }
    };

    class SerialIOFileStreamView :
        public  SerialIOStream,
        public  __impl::SerialFileStreamViewBase {
    public:
        SerialIOFileStreamView(FILE* handle) :
            SerialFileStreamViewBase(handle) {}

        std::optional<std::byte>
        Read() override {
            return this->SerialFileStreamViewBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->SerialFileStreamViewBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->SerialFileStreamViewBase::PutBack(c);
        }

        bool
        Write(std::byte c) override {
            return this->SerialFileStreamViewBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->SerialFileStreamViewBase::WriteSome(buffer);
        }
    };

    class SerialIFileStream :
        public  SerialIStream,
        public  __impl::SerialFileStreamBase {
    public:
        SerialIFileStream(std::string_view strvFilename) :
            SerialFileStreamBase(strvFilename, "r") {}

        std::optional<std::byte>
        Read() override {
            return this->SerialFileStreamBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->SerialFileStreamBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->SerialFileStreamBase::PutBack(c);
        }
    };

    class SerialOFileStream :
        public  SerialOStream,
        public  __impl::SerialFileStreamBase {
    public:
        SerialOFileStream(std::string_view strvFilename) :
            SerialFileStreamBase(strvFilename, "w") {}

        bool
        Write(std::byte c) override {
            return this->SerialFileStreamBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->SerialFileStreamBase::WriteSome(buffer);
        }
    };

    class SerialIOFileStream :
        public  SerialIOStream,
        public  __impl::SerialFileStreamBase {
    public:
        SerialIOFileStream(std::string_view strvFilename) :
            SerialFileStreamBase(strvFilename, "r+") {}

        bool
        Write(std::byte c) override {
            return this->SerialFileStreamBase::Write(c);
        }

        size_t
        WriteSome(std::span<const std::byte> buffer) override {
            return this->SerialFileStreamBase::WriteSome(buffer);
        }

        std::optional<std::byte>
        Read() override {
            return this->SerialFileStreamBase::Read();
        }

        size_t
        ReadSome(std::span<std::byte> buffer) override {
            return this->SerialFileStreamBase::ReadSome(buffer);
        }

        bool
        PutBack(std::byte c) override {
            return this->SerialFileStreamBase::PutBack(c);
        }
    };
}

