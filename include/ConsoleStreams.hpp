#pragma once
#include "FileStreams.hpp"
#include "TextIO.hpp"

namespace io {
    namespace __impl {
        static IFileStreamView
            __std_input(stdin);
        static OFileStreamView
            __std_output(stdout),
            __std_error(stderr);
    }

    static SerialIStream&
        std_input   = __impl::__std_input;
    static SerialOStream&
        std_output  = __impl::__std_output;
    static SerialOStream&
        std_error   = __impl::__std_error;

    static SerialTextInput
        cin(std_input);
    static SerialTextOutput
        cout(std_output),
        cerr(std_error);
}
