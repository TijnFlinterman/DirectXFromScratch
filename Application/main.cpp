#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>
#include "../Application.h"
#include "TriangleDemo.h"

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    try
    {
        Application& app = Application::Get();
        app.Initialize(hInstance);

        app.RegisterDemo(std::make_shared<TriangleDemo>());

        int result = app.Run();
        app.Shutdown();
        return result;
    }
    catch (std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}