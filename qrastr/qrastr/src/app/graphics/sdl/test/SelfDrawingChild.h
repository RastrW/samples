#pragma once

// Скрываем платформенные типы за void*
// чтобы не тащить windows.h / Xlib.h в заголовок
class SelfDrawingChild
{
public:
    SelfDrawingChild()  = default;
    ~SelfDrawingChild() { destroy(); }

    // parentHandle:
    //   Windows → HWND  (reinterpret_cast<void*>(winId()))
    //   Linux   → Window XID (reinterpret_cast<void*>(winId()))
    bool create(void* parentHandle, int x, int y, int w, int h);
    void resize(int x, int y, int w, int h);
    void destroy();

private:
    void* m_hwnd    = nullptr;   // реально HWND
    int   m_frame   = 0;

    static long __stdcall WndProc(void* hwnd, unsigned msg,
                                  unsigned long long wp, long long lp);
    static bool registerClass();
};