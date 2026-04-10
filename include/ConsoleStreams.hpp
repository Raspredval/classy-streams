static_assert(__cplusplus >= 202302, "requires C++23 minimum version");

#pragma once
#include "FileStreams.hpp"
#include "IOReadWrite.hpp"

namespace io {
    extern SerialIFileStreamView
        std_input;
    extern SerialOFileStreamView
        std_output, std_error;

    extern SerialTextInput
        cin;
    extern SerialTextOutput
        cout, cerr;
}
