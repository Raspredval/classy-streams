#include "ConsoleStreams.hpp"

namespace io {
    SerialIFileStreamView
        std_input   = { ::stdin };
    SerialOFileStreamView
        std_output  = { ::stdout },
        std_error   = { ::stderr };

    SerialTextInput
        cin(std_input);
    SerialTextOutput
        cout(std_output),
        cerr(std_error);
}