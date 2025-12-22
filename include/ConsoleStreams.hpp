#pragma once
#include "FileStreams.hpp"
#include "TextIO.hpp"

namespace io {
    static IFileStreamView
        std_input(stdin);
    static OFileStreamView
        std_output(stdout),
        std_error(stderr);

    static TextInput
        cin(std_input);
    static TextOutput
        cout(std_output),
        cerr(std_error);
}
