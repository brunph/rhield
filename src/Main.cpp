#include "CommonIncludes.hpp"
#include "decoder/Decoder.hpp"
#include "util/MapParser.hpp"
#include "core/Instance.hpp"

int main() {
    spdlog::set_level(spdlog::level::trace);

    constexpr auto RHIELD_ARCH = 64;
    constexpr auto RHIELD_FILE_NAME = "Demo.exe";

    rhield::Instance<RHIELD_ARCH> instance(RHIELD_FILE_NAME);

    if (!instance.init()) {
        CRITICAL("Failed to initialize Rhield instance!");
        return 0;
    }

    instance.setRandomizeSections(true);

    // funcs
    //instance.addFunctionByName("?add@@YAHHH@Z");
    instance.addFunctionByName("main");

    if (!instance.run()) {
        CRITICAL("Failed to run Rhield instance!");
        return 0;
    }

    if (!instance.save("Demo.rhield.exe")) {
        CRITICAL("Failed to save Rhield executable");
        return 0;
    }
    return 1;
}