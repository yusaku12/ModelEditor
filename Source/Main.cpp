#include "System.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR pScmdline, int iCmdShow)
{
    bool hr = System::Instance()->Initialize();
    if (!hr)
        return 0;
    System::Instance()->Run();
    return 0;
}