#pragma once
#include "Singleton.h"
#include <Windows.h>
#include <string>

class DROPMANAGER : public Singleton<DROPMANAGER>
{
    std::string path{};
    std::wstring wpath{};
    bool loaded{};

public:

    void Load(TCHAR* t);
    bool Loaded();

    std::string Path();
    std::wstring WPath();
};