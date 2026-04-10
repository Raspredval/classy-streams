static_assert(__cplusplus >= 202302, "requires C++23 minimum version");

#pragma once
#include "FileStreams.hpp"
#include "IOReadWrite.hpp"

namespace io {
    inline SerialIFileStreamView
        std_input   = {::stdin};
    inline SerialOFileStreamView
        std_output  = {::stdout},
        std_error   = {::stderr};

    inline SerialTextInput
        cin(std_input);
    inline SerialTextOutput
        cout(std_output),
        cerr(std_error);
}
