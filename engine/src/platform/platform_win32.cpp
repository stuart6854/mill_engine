#if 0
    #include "mill/platform/platform_interface.hpp"

    #if MILL_WINDOWS

        #include "mill/core/debug.hpp"
        #include "mill/core/engine.hpp"
        #include "mill/events/events.hpp"
        #include "mill/input/input_codes.hpp"

        #include <windows.h>
        #include <windowsx.h>  // Param input extraction

namespace mill::platform
{
        #define U32_CAST(_x) static_cast<u32>(_x)

    constexpr LPCSTR g_WindowClass = "mill_window_class";

    struct InternalStateWin32
    {
        HINSTANCE hInstance{ nullptr };

        struct Window
        {
            HWND hwnd{ nullptr };
            i32 width{};
            i32 height{};
        };
        std::unordered_map<void*, Window> windows{};
    };
    static Owned<InternalStateWin32> s_Win32State{};

    // Clock
    static f64 s_ClockFrequency{};
    static LARGE_INTEGER s_ClockStartTime{};

    auto CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) -> LRESULT;

    bool platform_initialise()
    {
        LOG_INFO("Platform - Win32 - Initialising...");

        s_Win32State = CreateOwned<InternalStateWin32>();

        s_Win32State->hInstance = GetModuleHandle(nullptr);

        // Setup and register window class
        HICON icon = LoadIcon(s_Win32State->hInstance, IDI_APPLICATION);
        WNDCLASSA window_class{};
        window_class.style = CS_DBLCLKS;  // Get double-clicks
        window_class.lpfnWndProc = win32_process_message;
        window_class.cbClsExtra = 0;
        window_class.cbWndExtra = 0;
        window_class.hInstance = s_Win32State->hInstance;
        window_class.hIcon = icon;
        window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);  // nullptr = Manage cursor manually
        window_class.hbrBackground = nullptr;                   // Transparent
        window_class.lpszClassName = g_WindowClass;
        if (!RegisterClassA(&window_class))
        {
            LOG_CRITICAL("Platform - Win32 - Window registration failed!");
            MessageBoxA(nullptr, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
            return false;
        }

        #if 0
		// Create window
        u32 client_x = init.windowPos.x;
        u32 client_y = init.windowPos.y;
        u32 client_w = init.windowSize.x;
        u32 client_h = init.windowSize.y;

        u32 window_x = client_x == -1 ? (GetSystemMetrics(SM_CXSCREEN) - client_w) / 2 : client_x;
        u32 window_y = client_y == -1 ? (GetSystemMetrics(SM_CYSCREEN) - client_h) / 2 : client_y;
        u32 window_w = client_w;
        u32 window_h = client_h;

        u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
        u32 window_ex_style = WS_EX_APPWINDOW;

        window_style |= WS_MAXIMIZEBOX;
        window_style |= WS_MINIMIZEBOX;
        window_style |= WS_THICKFRAME;

        // Obtain the size of the border
        RECT border_rect{};
        AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

        // In this case, the border rectangle is negative
        window_x += border_rect.left;
        window_y += border_rect.top;

        // Grow by the size of the OS border
        window_w += border_rect.right - border_rect.left;
        window_h += border_rect.bottom - border_rect.top;

        HWND handle = CreateWindowExA(window_ex_style,
                                      "mill_window_class",
                                      init.appName.c_str(),
                                      window_style,
                                      window_x,
                                      window_y,
                                      window_w,
                                      window_h,
                                      0,
                                      0,
                                      s_Win32State->hInstance,
                                      0);
        if (handle == nullptr)
        {
            LOG_CRITICAL("Platform - Win32 - Window creation failed!");
            MessageBoxA(nullptr, "Window creation failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
            return false;
        }
        s_Win32State->hwnd = handle;

        // Show the window
        bool should_active = true;  // #TODO: If the window should not accept input, this should be false
        i32 show_window_command_flags = should_active ? SW_SHOW : SW_SHOWNOACTIVATE;
        // If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE
        // If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE
        ShowWindow(s_Win32State->hwnd, show_window_command_flags);
        #endif  // 0

        // Clock setup
        LARGE_INTEGER frequency{};
        QueryPerformanceFrequency(&frequency);
        s_ClockFrequency = 1.0 / static_cast<f64>(frequency.QuadPart);
        QueryPerformanceCounter(&s_ClockStartTime);

        return true;
    }

    void platform_shutdown()
    {
        LOG_INFO("Platform - Win32 - Shutting down...");

        for (auto& [handle, window] : s_Win32State->windows)
        {
            LOG_WARN("Platform - Win32 - Window was not destroyed.");
            DestroyWindow(window.hwnd);
        }
        s_Win32State->windows.clear();

        s_Win32State = nullptr;
    }

    bool platform_pump_messages()
    {
        MSG message{};
        while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        return true;
    }

    auto create_window(const WindowInfo& info) -> HandleWindow
    {
        ASSERT(s_Win32State);
        ASSERT(info.width && info.height);

        InternalStateWin32::Window window{};

        // Create window
        u32 client_x = info.pos_x;
        u32 client_y = info.pos_y;
        u32 client_w = info.width;
        u32 client_h = info.height;

        u32 window_x = client_x == -1 ? (GetSystemMetrics(SM_CXSCREEN) - client_w) / 2 : client_x;
        u32 window_y = client_y == -1 ? (GetSystemMetrics(SM_CYSCREEN) - client_h) / 2 : client_y;
        u32 window_w = client_w;
        u32 window_h = client_h;

        u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
        u32 window_ex_style = WS_EX_APPWINDOW;

        window_style |= WS_MAXIMIZEBOX;
        window_style |= WS_MINIMIZEBOX;
        window_style |= WS_THICKFRAME;

        // Obtain the size of the border
        RECT border_rect{};
        AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

        // In this case, the border rectangle is negative
        window_x += border_rect.left;
        window_y += border_rect.top;

        // Grow by the size of the OS border
        window_w += border_rect.right - border_rect.left;
        window_h += border_rect.bottom - border_rect.top;

        HWND hwnd = CreateWindowExA(window_ex_style,
                                    g_WindowClass,
                                    info.title.c_str(),
                                    window_style,
                                    window_x,
                                    window_y,
                                    window_w,
                                    window_h,
                                    0,
                                    0,
                                    s_Win32State->hInstance,
                                    0);
        if (hwnd == nullptr)
        {
            LOG_CRITICAL("Platform - Win32 - Window creation failed!");
            MessageBoxA(nullptr, "Window creation failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
            return {};
        }
        window.hwnd = hwnd;

        // Show the window
        bool should_active = true;  // #TODO: If the window should not accept input, this should be false
        i32 show_window_command_flags = should_active ? SW_SHOW : SW_SHOWNOACTIVATE;
        // If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE
        // If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE
        ShowWindow(window.hwnd, show_window_command_flags);

        HandleWindow handle{ hwnd };
        s_Win32State->windows[handle.ptr] = window;

        LOG_INFO("Platform - Win32 - Window created <{}>.", fmt::ptr(handle.ptr));

        return handle;
    }

    void destroy_window(HandleWindow window_handle)
    {
        ASSERT(s_Win32State);
        ASSERT(window_handle.ptr);
        ASSERT(s_Win32State->windows.contains(window_handle.ptr));

        LOG_INFO("Platform - Win32 - Destroying window <{}>.", fmt::ptr(window_handle.ptr));

        auto& window = s_Win32State->windows.at(window_handle.ptr);

        DestroyWindow(window.hwnd);
        s_Win32State->windows.erase(window_handle.ptr);
    }

        #if 0
    auto platform_allocate(u64 size, bool /*aligned*/) -> void*
    {
        return malloc(size);
    }

    void platform_free(void* block, bool /*aligned*/)
    {
        free(block);
    }
        #endif

    auto platform_get_absolute_time() -> f64
    {
        LARGE_INTEGER now_time{};
        QueryPerformanceCounter(&now_time);
        return static_cast<f64>(now_time.QuadPart) * s_ClockFrequency;
    }

    auto CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) -> LRESULT
    {
        #define APP_HAS_HANDLED 0
        #define APP_WILL_HANDLE 1

        auto& events = Engine::get()->get_events();
        Event event{};

        switch (msg)
        {
            case WM_ERASEBKGND:
                // Notify the OS that erasing will be handled by the application to prevent flicker.
                return APP_WILL_HANDLE;
            case WM_CLOSE:
                event.type = EventType::eWindowClose;
                event.context = hwnd;
                events.post_immediate(event);

                return APP_HAS_HANDLED;
            case WM_DESTROY: PostQuitMessage(0); return APP_HAS_HANDLED;
            case WM_SIZE:
            {
                // Get the updated size
                RECT rect{};
                GetClientRect(hwnd, &rect);
                u32 width = rect.right - rect.left;
                u32 height = rect.bottom - rect.top;

                event.type = EventType::eWindowSize;
                event.context = hwnd;
                event.data.i32[0] = width;
                event.data.i32[1] = height;
                events.post_immediate(event);
            }
            break;
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                // Key pressed or released?
                bool is_down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
                auto key = static_cast<u32>(w_param);  // #TODO: Possibly use scan codes

                event.type = EventType::eInputKey;
                event.context = hwnd;
                event.data.u32[0] = key;
                event.data.u32[1] = is_down;
                events.post_immediate(event);
            }
            break;
            case WM_MOUSEMOVE:
            {
                // Mouse move
                i32 x_position = GET_X_LPARAM(l_param);
                i32 y_position = GET_Y_LPARAM(l_param);

                event.type = EventType::eInputMouseMove;
                event.context = hwnd;
                event.data.i32[0] = x_position;
                event.data.i32[1] = y_position;
                events.post_immediate(event);
            }
            break;
            case WM_MOUSEWHEEL:
            {
                i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
                if (z_delta != 0)
                {
                    // Flatten the input to an OS-independent (-1, 1)
                    z_delta = (z_delta < 0) ? -1 : 1;

                    event.type = EventType::eInputMouseWheel;
                    event.context = hwnd;
                    event.data.i32[0] = z_delta;
                    events.post_immediate(event);
                }
            }
            break;
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
            {
                // Button pressed or released?
                bool is_down = (msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN);
                auto btn = static_cast<u32>(w_param);

                event.type = EventType::eInputMouseBtn;
                event.context = hwnd;
                event.data.i32[0] = btn;
                event.data.i32[1] = is_down;
                events.post_immediate(event);
            }
            break;
            default: break;
        }

        return DefWindowProcA(hwnd, msg, w_param, l_param);
    }
}

    #endif
#endif  // 0
