#include <format>
#include <stdexcept>
#include "FileStreams.hpp"

namespace io {
    namespace __impl {
        SerialFileStreamViewBase::SerialFileStreamViewBase(FILE* handle) :
            handle(handle) {}

        bool
        SerialFileStreamViewBase::EndOfStream() const noexcept {
            return feof(this->handle);
        }

        bool
        SerialFileStreamViewBase::Good() const noexcept {
            return !ferror(this->handle);
        }

        void
        SerialFileStreamViewBase::ClearFlags() noexcept {
            return clearerr(this->handle);
        }

        FILE*
        SerialFileStreamViewBase::Handle() const noexcept {
            return this->handle;
        }

        bool
        SerialFileStreamViewBase::Flush() noexcept {
            return fflush(this->handle) == 0;
        }

        std::optional<std::byte>
        SerialFileStreamViewBase::Read() {
            auto c =
                fgetc(this->handle);
            return (c != EOF)
                ? std::optional{ (std::byte)c }
                : std::nullopt;
        }

        size_t
        SerialFileStreamViewBase::ReadSome(std::span<std::byte> buffer) {
            return fread(
                buffer.data(),
                1, buffer.size(),
                this->handle);
        }

        bool
        SerialFileStreamViewBase::Write(std::byte c) {
            auto result =
                fputc((int)c, this->handle);
            return result != EOF;
        }

        size_t
        SerialFileStreamViewBase::WriteSome(std::span<const std::byte> buffer) {
            return fwrite(
                buffer.data(),
                1, buffer.size(),
                this->handle);
        }

        bool
        SerialFileStreamViewBase::PutBack(std::byte c) {
            return ungetc((int)c, this->handle) != EOF;
        }

        FileStreamViewBase::FileStreamViewBase(FILE* handle) :
            SerialFileStreamViewBase(handle) {}

        intptr_t
        FileStreamViewBase::GetPosition() const noexcept {
            return ftell(this->handle);
        }

        bool
        FileStreamViewBase::SetPosition(intptr_t offset, StreamOffsetOrigin from) {
            return !fseek(
                this->handle,
                offset,
                (int)from);
        }

        SerialFileStreamBase::SerialFileStreamBase(SerialFileStreamBase&& obj) noexcept :
            SerialFileStreamViewBase(obj.handle)
        {
            obj.handle  = nullptr;
        }

        SerialFileStreamBase&
        SerialFileStreamBase::operator=(SerialFileStreamBase&& obj) noexcept {
            SerialFileStreamBase
                temp    = std::move(obj);
            std::swap(
                this->handle, temp.handle);
            return *this;
        }

        SerialFileStreamBase::SerialFileStreamBase(std::string_view strvFilename, std::string_view strvMode) :
            SerialFileStreamViewBase(fopen(strvFilename.data(), strvMode.data()))
        {
            if (this->handle == nullptr) {
                throw std::runtime_error(std::format(
                    "failed to open file {} with mode {}",
                    strvFilename, strvMode));
            }
        }

        SerialFileStreamBase::~SerialFileStreamBase() noexcept {
            if (this->handle != nullptr)
                fclose(this->handle);
        }

        FileStreamBase::FileStreamBase(FileStreamBase&& obj) noexcept :
            FileStreamViewBase(obj.handle)
        {
            obj.handle      = nullptr;
        }

        FileStreamBase&
        FileStreamBase::operator=(FileStreamBase&& obj) noexcept {
            FileStreamBase
                temp    = std::move(obj);
            std::swap(
                this->handle, temp.handle);
            return *this;
        }

        FileStreamBase::FileStreamBase(std::string_view strvFilename, std::string_view strvMode) :
            FileStreamViewBase(fopen(strvFilename.data(), strvMode.data()))
        {
            if (this->handle == nullptr) {
                throw std::runtime_error(std::format(
                    "failed to open file {} with mode {}",
                    strvFilename, strvMode));
            }
        }

        FileStreamBase::~FileStreamBase() noexcept {
            if (this->handle != nullptr)
                fclose(this->handle);
        }
    }

    IFileStreamView::IFileStreamView(FILE* handle) :
        FileStreamViewBase(handle) {}

    std::optional<std::byte>
    IFileStreamView::Read() {
        return this->FileStreamViewBase::Read();
    }

    size_t
    IFileStreamView::ReadSome(std::span<std::byte> buffer) {
        return this->FileStreamViewBase::ReadSome(buffer);
    }

    bool
    IFileStreamView::PutBack(std::byte c) {
        return this->FileStreamViewBase::PutBack(c);
    }

    OFileStreamView::OFileStreamView(FILE* handle) :
        FileStreamViewBase(handle) {}

    bool
    OFileStreamView::Write(std::byte c) {
        return this->FileStreamViewBase::Write(c);
    }

    size_t
    OFileStreamView::WriteSome(std::span<const std::byte> buffer) {
        return this->FileStreamViewBase::WriteSome(buffer);
    }

    IOFileStreamView::IOFileStreamView(FILE* handle) :
        FileStreamViewBase(handle) {}

    std::optional<std::byte>
    IOFileStreamView::Read() {
        return this->FileStreamViewBase::Read();
    }

    size_t
    IOFileStreamView::ReadSome(std::span<std::byte> buffer) {
        return this->FileStreamViewBase::ReadSome(buffer);
    }

    bool
    IOFileStreamView::PutBack(std::byte c) {
        return this->FileStreamViewBase::PutBack(c);
    }

    bool
    IOFileStreamView::Write(std::byte c) {
        return this->FileStreamViewBase::Write(c);
    }

    size_t
    IOFileStreamView::WriteSome(std::span<const std::byte> buffer) {
        return this->FileStreamViewBase::WriteSome(buffer);
    }

    IFileStream::IFileStream(std::string_view strvFilename) :
        FileStreamBase(strvFilename, "rb") {}

    std::optional<std::byte>
    IFileStream::Read() {
        return this->FileStreamBase::Read();
    }

    size_t
    IFileStream::ReadSome(std::span<std::byte> buffer) {
        return this->FileStreamBase::ReadSome(buffer);
    }

    bool
    IFileStream::PutBack(std::byte c) {
        return this->FileStreamBase::PutBack(c);
    }

    OFileStream::OFileStream(std::string_view strvFilename) :
        FileStreamBase(strvFilename, "wb") {}

    bool
    OFileStream::Write(std::byte c) {
        return this->FileStreamBase::Write(c);
    }

    size_t
    OFileStream::WriteSome(std::span<const std::byte> buffer) {
        return this->FileStreamBase::WriteSome(buffer);
    }

    IOFileStream::IOFileStream(std::string_view strvFilename) :
        FileStreamBase(strvFilename, "rb+") {}

    bool
    IOFileStream::Write(std::byte c) {
        return this->FileStreamBase::Write(c);
    }

    size_t
    IOFileStream::WriteSome(std::span<const std::byte> buffer) {
        return this->FileStreamBase::WriteSome(buffer);
    }

    std::optional<std::byte>
    IOFileStream::Read() {
        return this->FileStreamBase::Read();
    }

    size_t
    IOFileStream::ReadSome(std::span<std::byte> buffer) {
        return this->FileStreamBase::ReadSome(buffer);
    }

    bool
    IOFileStream::PutBack(std::byte c) {
        return this->FileStreamBase::PutBack(c);
    }

    SerialIFileStreamView::SerialIFileStreamView(FILE* handle) :
        SerialFileStreamViewBase(handle) {}

    std::optional<std::byte>
    SerialIFileStreamView::Read() {
        return this->SerialFileStreamViewBase::Read();
    }

    size_t
    SerialIFileStreamView::ReadSome(std::span<std::byte> buffer) {
        return this->SerialFileStreamViewBase::ReadSome(buffer);
    }

    bool
    SerialIFileStreamView::PutBack(std::byte c) {
        return this->SerialFileStreamViewBase::PutBack(c);
    }

    SerialOFileStreamView::SerialOFileStreamView(FILE* handle) :
        SerialFileStreamViewBase(handle) {}

    bool
    SerialOFileStreamView::Write(std::byte c) {
        return this->SerialFileStreamViewBase::Write(c);
    }

    size_t
    SerialOFileStreamView::WriteSome(std::span<const std::byte> buffer) {
        return this->SerialFileStreamViewBase::WriteSome(buffer);
    }

    SerialIOFileStreamView::SerialIOFileStreamView(FILE* handle) :
        SerialFileStreamViewBase(handle) {}

    std::optional<std::byte>
    SerialIOFileStreamView::Read() {
        return this->SerialFileStreamViewBase::Read();
    }

    size_t
    SerialIOFileStreamView::ReadSome(std::span<std::byte> buffer) {
        return this->SerialFileStreamViewBase::ReadSome(buffer);
    }

    bool
    SerialIOFileStreamView::PutBack(std::byte c) {
        return this->SerialFileStreamViewBase::PutBack(c);
    }

    bool
    SerialIOFileStreamView::Write(std::byte c) {
        return this->SerialFileStreamViewBase::Write(c);
    }

    size_t
    SerialIOFileStreamView::WriteSome(std::span<const std::byte> buffer) {
        return this->SerialFileStreamViewBase::WriteSome(buffer);
    }

    SerialIFileStream::SerialIFileStream(std::string_view strvFilename) :
        SerialFileStreamBase(strvFilename, "rb") {}

    std::optional<std::byte>
    SerialIFileStream::Read() {
        return this->SerialFileStreamBase::Read();
    }

    size_t
    SerialIFileStream::ReadSome(std::span<std::byte> buffer) {
        return this->SerialFileStreamBase::ReadSome(buffer);
    }

    bool
    SerialIFileStream::PutBack(std::byte c) {
        return this->SerialFileStreamBase::PutBack(c);
    }

    SerialOFileStream::SerialOFileStream(std::string_view strvFilename) :
        SerialFileStreamBase(strvFilename, "wb") {}

    bool
    SerialOFileStream::Write(std::byte c) {
        return this->SerialFileStreamBase::Write(c);
    }

    size_t
    SerialOFileStream::WriteSome(std::span<const std::byte> buffer) {
        return this->SerialFileStreamBase::WriteSome(buffer);
    }

    SerialIOFileStream::SerialIOFileStream(std::string_view strvFilename) :
        SerialFileStreamBase(strvFilename, "rb+") {}

    bool
    SerialIOFileStream::Write(std::byte c) {
        return this->SerialFileStreamBase::Write(c);
    }

    size_t
    SerialIOFileStream::WriteSome(std::span<const std::byte> buffer) {
        return this->SerialFileStreamBase::WriteSome(buffer);
    }

    std::optional<std::byte>
    SerialIOFileStream::Read() {
        return this->SerialFileStreamBase::Read();
    }

    size_t
    SerialIOFileStream::ReadSome(std::span<std::byte> buffer) {
        return this->SerialFileStreamBase::ReadSome(buffer);
    }

    bool
    SerialIOFileStream::PutBack(std::byte c) {
        return this->SerialFileStreamBase::PutBack(c);
    }
}