#include "Application.h"
#include "Helpers.h"
#include <shellapi.h>

// Forward declare IDemo here to avoid circular includes
#include "IDemo.h"
#include <cassert>

static Application* g_AppInstance = nullptr;

Application& Application::Get()
{
    static Application instance;
    return instance;
}

void Application::Initialize(HINSTANCE hInstance, bool useWarp)
{
    m_hInstance = hInstance;

    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

#if defined(_DEBUG)
    // Enable debug layer
    Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif

    InitWindow(hInstance);

    m_Device.Initialize(useWarp);
    m_CommandQueue.Initialize(m_Device.GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_SwapChain.Initialize(m_Device.GetDevice(), m_CommandQueue.GetQueue(),
        m_hWnd, m_ClientWidth, m_ClientHeight);

    m_IsInitialized = true;
    ::ShowWindow(m_hWnd, SW_SHOW);
}

void Application::Shutdown()
{
    m_CommandQueue.Shutdown();
}

void Application::RegisterDemo(std::shared_ptr<IDemo> demo)
{
    m_Demos.push_back(demo);
}

int Application::Run()
{
    for (auto& demo : m_Demos)
        RunDemo(demo);

    return 0;
}

void Application::RunDemo(std::shared_ptr<IDemo> demo)
{
    m_ActiveDemo = demo;
    m_ActiveDemo->LoadContent();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else
        {
            static auto t0 = std::chrono::high_resolution_clock().now();
            auto t1 = std::chrono::high_resolution_clock().now();
            float deltaTime = std::chrono::duration<float>(t1 - t0).count();
            t0 = t1;

            m_ActiveDemo->OnUpdate(deltaTime);
            m_ActiveDemo->OnRender();
        }
    }

    m_CommandQueue.Flush();
    m_ActiveDemo->UnloadContent();
    m_ActiveDemo = nullptr;
}

void Application::InitWindow(HINSTANCE hInstance)
{
    const wchar_t* windowClassName = L"DX12WindowClass";

    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = ::LoadIcon(hInstance, NULL);
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = windowClassName;
    windowClass.hIconSm = ::LoadIcon(hInstance, NULL);
    static ATOM atom = ::RegisterClassExW(&windowClass);
    assert(atom > 0);

    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    RECT windowRect = { 0, 0, static_cast<LONG>(m_ClientWidth), static_cast<LONG>(m_ClientHeight) };
    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;
    int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
    int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

    m_hWnd = ::CreateWindowExW(NULL, windowClassName, L"DirectX12 From Scratch",
        WS_OVERLAPPEDWINDOW, windowX, windowY, windowWidth, windowHeight,
        NULL, NULL, hInstance, nullptr);

    assert(m_hWnd && "Failed to create window");
    ::GetWindowRect(m_hWnd, &m_WindowRect);
}

LRESULT CALLBACK Application::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Application& app = Application::Get();

    if (app.m_IsInitialized)
    {
        switch (message)
        {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
            case VK_ESCAPE:
                ::PostQuitMessage(0);
                break;
            case VK_F11:
                // fullscreen toggle can be added later
                break;
            }
            break;
        }
        case WM_SIZE:
        {
            RECT clientRect = {};
            ::GetClientRect(hwnd, &clientRect);
            uint32_t width = clientRect.right - clientRect.left;
            uint32_t height = clientRect.bottom - clientRect.top;

            app.m_CommandQueue.Flush();
            app.m_SwapChain.Resize(width, height);
            app.m_ClientWidth = width;
            app.m_ClientHeight = height;

            if (app.m_ActiveDemo)
                app.m_ActiveDemo->OnResize(width, height);
            break;
        }
        case WM_DESTROY:
            ::PostQuitMessage(0);
            break;
        default:
            return ::DefWindowProcW(hwnd, message, wParam, lParam);
        }
    }
    else
    {
        return ::DefWindowProcW(hwnd, message, wParam, lParam);
    }

    return 0;
}