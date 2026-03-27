#include "SelfDrawingChild.h"

#if defined(_WIN32)
#include <windows.h>
#include <cwchar>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

static const COLORREF kColors[] = {
    RGB(30,  80, 160), RGB(20, 130, 80),
    RGB(160, 60,  30), RGB(90,  40, 140),
};

bool SelfDrawingChild::registerClass()
{
    static bool done = false;
    if (done) return true;
    // registerClass() вызывается один раз (флаг done).
    // Регистрирует класс Win32 "SelfDrawingChild" с собственным WndProc.
    // CS_HREDRAW | CS_VREDRAW — полная перерисовка при смене размера.
    // hbrBackground = nullptr — фон не заполняется системой автоматически,
    // чтобы не было мерцания поверх GDI-рисования в WM_PAINT.
    WNDCLASSEXW wc  = {};
    wc.cbSize       = sizeof(wc);
    wc.style        = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc  = reinterpret_cast<WNDPROC>(SelfDrawingChild::WndProc);
    wc.hInstance    = ::GetModuleHandleW(nullptr);
    wc.hbrBackground= nullptr;
    wc.lpszClassName= L"SelfDrawingChild";
    wc.hCursor      = ::LoadCursor(nullptr, IDC_ARROW);

    done = (::RegisterClassExW(&wc) != 0);
    return done;
}

bool SelfDrawingChild::create(void* parentHandle, int x, int y, int w, int h)
{
    if (!registerClass()) return false;

    HWND parent = reinterpret_cast<HWND>(parentHandle);
    // WS_CHILD  — окно встроено в родителя (не имеет рамки, заголовка).
    // WS_VISIBLE — сразу видимо, не нужен ShowWindow.
    // WS_CLIPSIBLINGS — не рисует поверх соседних дочерних окон (SDL-поверхности).
    // lpCreateParams = this — передаём указатель на SelfDrawingChild,
    // чтобы WndProc мог сохранить его в GWLP_USERDATA при WM_CREATE.
    HWND hwnd = ::CreateWindowExW(
        0, L"SelfDrawingChild", L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        x, y, w, h,
        parent, nullptr, ::GetModuleHandleW(nullptr),
        this
        );
    m_hwnd = hwnd;
    return hwnd != nullptr;
}

void SelfDrawingChild::resize(int x, int y, int w, int h)
{
    if (!m_hwnd) return;
    ::SetWindowPos(reinterpret_cast<HWND>(m_hwnd),
                   nullptr, x, y, w, h,
                   SWP_NOZORDER | SWP_NOACTIVATE);
}

void SelfDrawingChild::destroy()
{
    if (m_hwnd) {
        ::DestroyWindow(reinterpret_cast<HWND>(m_hwnd));
        m_hwnd = nullptr;
    }
}

long __stdcall SelfDrawingChild::WndProc(void* hwnd_, unsigned msg,
                                         unsigned long long wp, long long lp)
{
    HWND hwnd = reinterpret_cast<HWND>(hwnd_);
    SelfDrawingChild* self = nullptr;

    if (msg == WM_CREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = reinterpret_cast<SelfDrawingChild*>(cs->lpCreateParams);
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        ::SetTimer(hwnd, 1, 500, nullptr);
        return 0;
    }

    self = reinterpret_cast<SelfDrawingChild*>(
        ::GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
    case WM_TIMER:
        self->m_frame++;
        ::InvalidateRect(hwnd, nullptr, FALSE);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = ::BeginPaint(hwnd, &ps);
        RECT rc; ::GetClientRect(hwnd, &rc);

        HBRUSH br = ::CreateSolidBrush(kColors[self->m_frame % 4]);
        ::FillRect(hdc, &rc, br);
        ::DeleteObject(br);

        ::SetBkMode(hdc, TRANSPARENT);
        ::SetTextColor(hdc, RGB(255, 255, 255));
        wchar_t buf[64];
        swprintf(buf, 64, L"Windows GDI | frame %d", self->m_frame);
        ::DrawTextW(hdc, buf, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        ::EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND: return 1;
    case WM_DESTROY:
        ::KillTimer(hwnd, 1);
        return 0;
    }
    return ::DefWindowProcW(hwnd, msg, wp, lp);
}

#endif // _WIN32
