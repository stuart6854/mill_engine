#pragma once

#include "core/engine.hpp"
#include "core/application.hpp"

#include <iostream>

#ifdef MILL_WINDOWS
// #include <windows.h>
// int wWinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPreInst*/, LPWSTR /*nCmdLine*/, int /*nCmdShow*/)
int main(int /*argc*/, char** /*argv*/)
#elif MILL_LINUX
int main(int /*argc*/, char** /*argv*/)
#endif
{
    std::cout << "Hello World!" << std::endl;

    auto* engine = new mill::Engine;
    auto* app = mill::create_application();

    engine->run(app);

    delete app;
    delete engine;

    return 0;
}