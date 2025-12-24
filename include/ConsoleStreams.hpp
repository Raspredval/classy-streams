#pragma once
#include "FileStreams.hpp"
#include "TextIO.hpp"

namespace io {
    namespace __impl {
        static IFileStreamView
            std_input(stdin);
        static OFileStreamView
            std_output(stdout),
            std_error(stderr);
    }

    static SerialIStream&
        std_input   = __impl::std_input;
    static SerialOStream&
        std_output  = __impl::std_output;
    static SerialOStream&
        std_error   = __impl::std_error;

    static SerialTextInput
        cin(std_input);
    static SerialTextOutput
        cout(std_output),
        cerr(std_error);
}
