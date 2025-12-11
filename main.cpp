#include <windows.h>
#include <vector>
#include <string>
#include "Emulator.h"

int WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nShowCmd)
{
    // convert lpCmdLine into normal argc/argv
    int argc = 0;
    LPWSTR* wideArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<std::string> argv(argc);

    for (int i = 0; i < argc; i++) {
        char buffer[512];
        wcstombs(buffer, wideArgv[i], sizeof(buffer));
        argv[i] = buffer;
    }

    LocalFree(wideArgv);

    if (argc < 2) {
        MessageBoxA(NULL, "Usage: SimpleNES <rom.nes>", "Error", MB_OK);
        return 0;
    }

    Emulator emu;
    emu.loadROM(argv[1]);
    emu.run();
    return 0;
}
