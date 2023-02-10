#include "DROPMANAGER.h"
#include <assert.h>

void DROPMANAGER::Load(TCHAR* t)
{
    wpath = t;
    std::string s_temp(wpath.begin(), wpath.end());
    path = s_temp;
    loaded = true;
}

std::string DROPMANAGER::Path()
{
    loaded = false;
    return path;
}

bool DROPMANAGER::Loaded()
{
    return loaded;
}

std::wstring DROPMANAGER::WPath()
{
    loaded = false;
    return wpath;
}