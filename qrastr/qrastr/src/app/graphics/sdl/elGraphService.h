#pragma once

#include <QLibrary>
#include "IPlainElGraph.h"

// Инкапсулирует загрузку ElGraphCtrl.dll и владение IPlainElGraph*.
class ElGraphService
{
public:
    ElGraphService();
    ElGraphService(const ElGraphService&)            = delete;
    ElGraphService& operator=(const ElGraphService&) = delete;
    ElGraphService(ElGraphService&&)            = delete;
    ElGraphService& operator=(ElGraphService&&) = delete;

    ~ElGraphService();

    // Загружает DLL, вызывает InitPlainDLL(), затем CreateChildWindow(parentHwnd).
    // parentHwnd — нативный HWND окна SDL на Windows, nullptr на других платформах.
    bool init(void* parentHwnd);
    // Уничтожает grc и выгружает библиотеку.
    // Безопасно вызывать повторно.
    void shutdown();
    // Доступ к интерфейсу библиотеки (nullptr если не загружена)
    IPlainElGraph* graph() const { return m_grc; }
    bool isLoaded()        const { return m_grc != nullptr; }
private:
    QLibrary       m_lib;
    IPlainElGraph* m_grc = nullptr;
};
