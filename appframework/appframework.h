#pragma once

#include "appframework/iapplication.h"
#include "common_stl.h"

class AppFramework
{
public:
    void Execute(const Span<String> &args);

private:

    String CurrentApp = "";

    IApplication *TheApp = nullptr;
};