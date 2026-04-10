#include "IOReadWrite.hpp"

namespace io {
    namespace __impl {
        class buffer {
        public:
            buffer() :
                data(nullptr),
                size(0) {}
            
            ~buffer() {
                if (this->data)
                    free(this->data);
            }

            std::span<char>
            get_span(size_t uSize) {
                if (this->size < uSize) {
                    this->data  = (char*)realloc((void*)this->data, uSize);
                    this->size  = uSize;
                }

                return { this->data, this->size };
            }
            
        private:
            char*   data;
            size_t  size;
        } static g_buffer;

        extern std::span<char>
        get_io_buffer(size_t uSize) {
            return g_buffer.get_span(uSize);
        }

        std::string
        TextInputBase::getword_impl(io::SerialIStream& stream) {
            std::string
                strWord;
            std::optional<std::byte>
                optc;
            while ((bool)(optc = stream.Read())) {
                if (!isspace((int)*optc)) {
                    stream.PutBack(*optc);
                    break;
                }
            }
            while ((bool)(optc = stream.Read())) {
                if (isspace((int)*optc)) {
                    stream.PutBack(*optc);
                    break;
                }
        
                strWord.push_back((char)*optc);
            }
        
            return strWord;
        }

        std::string
        TextInputBase::getline_impl(io::SerialIStream& stream) {
            std::string
                strLine;
            std::optional<std::byte>
                optc;
            while ((bool)(optc = stream.Read())) {
                if ((char)*optc == '\n') {
                    break;
                }
                
                strLine += (char)*optc;
            }

            return strLine;
        }

        std::string
        TextInputBase::getall_impl(io::SerialIStream& stream) {
            std::string
                strAll;
            std::optional<std::byte>
                optc;
            while ((bool)(optc = stream.Read())) {
                strAll += (char)*optc;
            }

            return strAll;
        }

        void
        TextOutputBase::importword_impl(io::SerialIStream& from, io::SerialOStream& to) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = from.Read())) {
                if (!isspace((int)*optc)) {
                    from.PutBack(*optc);
                    break;
                }
            }
            while ((bool)(optc = from.Read())) {
                if (isspace((int)*optc)) {
                    from.PutBack(*optc);
                    break;
                }

                to.Write(*optc);
            }
        }

        void
        TextOutputBase::importline_impl(io::SerialIStream& from, io::SerialOStream& to) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = from.Read())) {
                if ((char)*optc == '\n') {
                    break;
                }

                to.Write(*optc);
            }
        }

        void
        TextOutputBase::importall_impl(io::SerialIStream& from, io::SerialOStream& to) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = from.Read())) {
                to.Write(*optc);
            }
        }

        void
        BinaryOutputBase::importdata_impl(io::SerialIStream& from, io::SerialOStream& to, size_t uByteCount) {
            while (uByteCount != 0) {
                std::optional<std::byte>
                    optc = from.Read();
                if (!optc)
                    break;
                to.Write(*optc);
                uByteCount -= 1;
            }
        }

        void
        BinaryInputBase::exportdata_impl(io::SerialIStream& from, io::SerialOStream& to, size_t uByteCount) {
            while (uByteCount != 0) {
                std::optional<std::byte>
                    optc = from.Read();
                if (!optc)
                    break;
                to.Write(*optc);
                uByteCount -= 1;
            }
        }

        void
        TextInputBase::exportword_impl(io::SerialIStream& from, io::SerialIOStream& to) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = from.Read())) {
                if (!isspace((int)*optc)) {
                    from.PutBack(*optc);
                    break;
                }
            }
            while ((bool)(optc = from.Read())) {
                if (isspace((int)*optc)) {
                    from.PutBack(*optc);
                    break;
                }

                to.Write(*optc);
            }
        }

        void
        TextInputBase::exportline_impl(io::SerialIStream& from, io::SerialIOStream& to) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = from.Read())) {
                if ((char)*optc == '\n') {
                    break;
                }

                to.Write(*optc);
            }
        }

        void
        TextInputBase::exportall_impl(io::SerialIStream& from, io::SerialIOStream& to) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = from.Read())) {
                to.Write(*optc);
            }
        }

        intptr_t
        TextInputBase::getint_impl(io::SerialIStream& stream, int base, bool(*fnIsDigit)(char)) {
            size_t
                uSize       = 0;
            std::optional<std::byte>
                optc        = std::nullopt;
            std::span<char>
                spnBuffer   = g_buffer.get_span(32);
            
        ParseSpacing:
            if ((bool)(optc = stream.Read())) {
                if (!isspace((int)*optc)) {
                    stream.PutBack(*optc);
                    goto ParseFirstChar;
                }
                else
                    goto ParseSpacing;
            }
            else
                return 0;

        ParseFirstChar:
            if ((bool)(optc = stream.Read())) {
                char c = (char)*optc;
                
                if (c == '-' || c == '+' || isdigit(c)) {
                    spnBuffer[uSize] = c;
                    uSize += 1;
                    goto ParseDigits;
                }
                else {
                    stream.PutBack(*optc);
                    return 0;
                }
            }
            else
                return 0;

        ParseDigits:
            if (uSize == spnBuffer.size())
                goto GenerateValue;
            if ((bool)(optc = stream.Read())) {
                char c = (char)*optc;
                if (fnIsDigit(c)) {
                    spnBuffer[uSize] = c;
                    uSize += 1;
                    goto ParseDigits;
                }
                else {
                    stream.PutBack(*optc);
                    goto GenerateValue;
                }
            }
            else
                goto GenerateValue;

        GenerateValue:
            intptr_t
                iResult;
            std::from_chars(
                spnBuffer.data(),
                spnBuffer.data() + uSize,
                iResult, base);
            return iResult;
        }

        double
        TextInputBase::getfloat_impl(io::SerialIStream& stream) {
            size_t
                uSize       = 0;
            std::optional<std::byte>
                optc        = std::nullopt;
            std::span<char>
                spnBuffer   = g_buffer.get_span(32);

        ParseSpacing:
            if ((bool)(optc = stream.Read())) {
                if (!isspace((int)*optc)) {
                    stream.PutBack(*optc);
                    goto ParseFirstChar;
                }
                else
                    goto ParseSpacing;
            }
            else
                return 0;

        ParseFirstChar:
            if ((bool)(optc = stream.Read())) {
                char c = (char)*optc;
                
                if (c == '-' || c == '+' || isdigit(c)) {
                    spnBuffer[uSize] = c;
                    uSize += 1;
                    goto ParseNaturalPart;
                }
                else if (c == '.' || c == ',') {
                    spnBuffer[uSize] = '.';
                    uSize += 1;
                    goto ParseFractionalPart;
                }
                else {
                    stream.PutBack(*optc);
                    return 0;
                }
            }
            else
                return 0;

        ParseNaturalPart:
            if (uSize == spnBuffer.size())
                goto GenerateValue;
            if ((bool)(optc = stream.Read())) {
                char c = (char)*optc;
                if (isdigit(c)) {
                    spnBuffer[uSize] = c;
                    uSize += 1;
                    goto ParseNaturalPart;
                }
                else if (c == '.' || c == ',') {
                    spnBuffer[uSize] = '.';
                    uSize += 1;
                    goto ParseFractionalPart;
                }
                else {
                    stream.PutBack(*optc);
                    goto GenerateValue;
                }
            }
            else
                goto GenerateValue;

        ParseFractionalPart:
            if (uSize == spnBuffer.size())
                goto GenerateValue;
            if ((bool)(optc = stream.Read())) {
                char c = (char)*optc;
                if (isdigit(c)) {
                    spnBuffer[uSize] = c;
                    uSize += 1;
                    goto ParseFractionalPart;
                }
                else {
                    stream.PutBack(*optc);
                    goto GenerateValue;
                }
            }
            else
                goto GenerateValue;

        GenerateValue:
            double
                fResult;
            std::from_chars(
                spnBuffer.data(),
                spnBuffer.data() + uSize,
                fResult, std::chars_format::fixed);
            return fResult;
        }
    }
}