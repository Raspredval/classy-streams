#include "IOReadWrite.hpp"
#include <cmath>

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
        TextInputBase::exportword_impl(io::SerialIStream& from, io::SerialOStream& to) {
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
        TextInputBase::exportline_impl(io::SerialIStream& from, io::SerialOStream& to) {
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
        TextInputBase::exportall_impl(io::SerialIStream& from, io::SerialOStream& to) {
            std::optional<std::byte>
                optc;
            while ((bool)(optc = from.Read())) {
                to.Write(*optc);
            }
        }

        static int
        dec2int(char c) {
            switch (c) {
            case '0'...'9':
                return c - '0';

            default:
                return -1;
            }
        }

        static int
        oct2int(char c) {
            switch (c) {
            case '0'...'7':
                return c - '0';

            default:
                return -1;
            }
        }

        static int
        hex2int(char c) {
            switch (c) {
            case '0'...'7':
                return c - '0';

            case 'A'...'F':
                return c - 'A';

            case 'a'...'f':
                return c - 'f';

            default:
                return -1;
            }
        }

        static int
        bin2int(char c) {
            switch (c) {
            case '0'...'1':
                return c - '0';

            default:
                return -1;
            }
        }

        intptr_t
        TextInputBase::getint_impl(io::SerialIStream& stream, int base) {
            intptr_t
                iValue  = 0,
                iSign   = 1;
            std::optional<std::byte>
                optc    = std::nullopt;

            using
                c2int_t = int(*)(char);
            c2int_t
                fnC2Int = nullptr;
            switch (base) {
            case 2:
                fnC2Int  = bin2int; break;
            case 8:
                fnC2Int  = oct2int; break;
            case 10:
                fnC2Int  = dec2int; break;
            case 16:
                fnC2Int  = hex2int; break;
            default:
                return 0;
            }

        ParseSpacing:
            if ((bool)(optc = stream.Read())) {
                if (!std::isspace((int)*optc)) {
                    stream.PutBack(*optc);
                    goto ParseSign;
                }
                else
                    goto ParseSpacing;
            }
            else
                return 0;

        ParseSign:
            if ((bool)(optc = stream.Read())) {
                switch ((char)*optc) {
                case '-':
                    iSign   = -1; break;
                case '+':
                    iSign   = +1; break;
                default:
                    stream.PutBack(*optc);
                } goto ParseFirstDigit;
            }
            else
                return 0;

        ParseFirstDigit:
            if ((bool)(optc = stream.Read())) {
                int
                    iDigit  = fnC2Int((char)*optc);
                if (iDigit >= 0) {
                    iValue  = iDigit;
                    goto ParseDigit;
                }
                else
                    stream.PutBack(*optc);
            } return 0;

        ParseDigit:
            if ((bool)(optc = stream.Read())) {
                int
                    iDigit  = fnC2Int((char)*optc);
                if (iDigit >= 0) {
                    iValue  = iValue * base + iDigit;
                    goto ParseDigit;
                }
                else
                    stream.PutBack(*optc);
            } return iValue * iSign;
        }

        double
        TextInputBase::getfloat_impl(io::SerialIStream& stream) {
            double
                fValue  = 0.,
                fSgnFrc = 1.,
                fExp    = 0.,
                fExpSgn = 1.;
            std::optional<std::byte>
                optc    = std::nullopt;

        ParseSpacing:
            if ((bool)(optc = stream.Read())) {
                if (!std::isspace((int)*optc)) {
                    stream.PutBack(*optc);
                    goto ParseSign;
                }
                else
                    goto ParseSpacing;
            }
            else
                return NAN;

        ParseSign:
            if ((bool)(optc = stream.Read())) {
                switch ((char)*optc) {
                case '-':
                    fSgnFrc = -1.; break;
                case '+':
                    fSgnFrc = +1.; break;
                default:
                    stream.PutBack(*optc);
                }

                goto ParseFirstNatural;
            }
            else
                return NAN;

        ParseFirstNatural:
            if ((bool)(optc = stream.Read())) {
                int
                    iDigit  = dec2int((char)*optc);
                if (iDigit >= 0) {
                    fValue  = iDigit;
                    goto ParseNatural;
                }
                else
                    stream.PutBack(*optc);
            } return NAN;

        ParseNatural:
            if ((bool)(optc = stream.Read())) {
                int
                    iDigit  = dec2int((char)*optc);
                if (iDigit < 0) {
                    stream.PutBack(*optc);
                    goto ParseDot;
                }
                else {
                    fValue  = fValue * 10. + iDigit;
                    goto ParseNatural;
                }
            }
            else
                return fValue * fSgnFrc;

        ParseDot:
            if ((bool)(optc = stream.Read())) {
                if ((char)*optc == '.')
                    goto ParseFract;
                else
                    stream.PutBack(*optc);
            } return fValue * fSgnFrc;

        ParseFract:
            if ((bool)(optc = stream.Read())) {
                int
                    iDigit  = dec2int((char)*optc);
                if (iDigit < 0) {
                    stream.PutBack(*optc);
                    goto ParseE;
                }
                else {
                    fValue  = fValue * 10. + iDigit;
                    fSgnFrc *= 0.1;
                    goto ParseFract;
                }
            }
            else
                return fValue * fSgnFrc;

        ParseE:
            if ((bool)(optc = stream.Read())) {
                switch ((char)*optc) {
                case 'E':
                case 'e':
                    goto ParseExpSign;
                default:
                    stream.PutBack(*optc);
                }
            } return fValue * fSgnFrc;

        ParseExpSign:
            if ((bool)(optc = stream.Read())) {
                switch ((char)*optc) {
                case '-':
                    fExpSgn = -1.f; break;
                case '+':
                    fExpSgn = +1.f; break;
                default:
                    stream.PutBack(*optc);
                    return NAN;
                } goto ParseFirstExp;
            }

        ParseFirstExp:
            if ((bool)(optc = stream.Read())) {
                int
                    iDigit  = dec2int((char)*optc);
                if (iDigit >= 0) {
                    fExp    = iDigit;
                    goto ParseExp;
                }
                else
                    stream.PutBack(*optc);
            } return NAN;

        ParseExp:
            if ((bool)(optc = stream.Read())) {
                int
                    iDigit  = dec2int((char)*optc);
                if (iDigit >= 0) {
                    fExp    = fExp * 10. + iDigit;
                    goto ParseExp;
                }
                else
                    stream.PutBack(*optc);
            } return fValue * fSgnFrc * std::pow(10, fExp * fExpSgn);
        }
    }
}