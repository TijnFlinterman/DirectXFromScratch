#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl.h>
#include <memory>
#include <vector>
#include <string>

#include "Device.h"
#include "CommandQueue.h"
#include "SwapChain.h"

class IDemo;

class Application
{
public:
    static Application& Get();

    void Initialize(HINSTANCE hInstance, bool useWarp = false);
    void Shutdown();
    int  Run();

    void RegisterDemo(std::shared_ptr<IDemo> demo);

    Device& GetDevice() { return m_Device; }
    CommandQueue& GetCommandQueue() { return m_CommandQueue; }
    SwapChain& GetSwapChain() { return m_SwapChain; }

    uint32_t GetClientWidth()  const { return m_ClientWidth; }
    uint32_t GetClientHeight() const { return m_ClientHeight; }

private:
    Application() = default;

    void InitWindow(HINSTANCE hInstance);
    void RunDemo(std::shared_ptr<IDemo> demo);

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    Device       m_Device;
    CommandQueue m_CommandQueue;
    SwapChain    m_SwapChain;

    HINSTANCE m_hInstance = nullptr;
    HWND      m_hWnd = nullptr;
    RECT      m_WindowRect = {};

    uint32_t  m_ClientWidth = 1280;
    uint32_t  m_ClientHeight = 720;

    bool m_IsInitialized = false;
    bool m_VSync = true;
    bool m_Fullscreen = false;

    std::vector<std::shared_ptr<IDemo>> m_Demos;
    std::shared_ptr<IDemo>              m_ActiveDemo;
};