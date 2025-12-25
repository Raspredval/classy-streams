#pragma once
#include "FileStreams.hpp"
#include "IOReadWrite.hpp"

namespace io {
    static SerialIFileStreamView
        std_input   = ::stdin;
    static SerialOFileStreamView
        std_output  = ::stdout,
        std_error   = ::stderr;

    static SerialTextInput
        cin(std_input);
    static SerialTextOutput
        cout(std_output),
        cerr(std_error);
}
