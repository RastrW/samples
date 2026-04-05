#pragma once

#include <QLibrary>

class IPlainElGraph;

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

    /// Загружает DLL и вызывает CreateChildWindow(parentHwnd).
    bool init(void* parentHwnd);
    // Уничтожает grc и выгружает библиотеку.
    // Безопасно вызывать повторно.
    void shutdown();
    // Доступ к интерфейсу библиотеки (nullptr если не загружена)
    IPlainElGraph* graph() const { return m_grc; }
    bool isLoaded()        const { return m_grc != nullptr; }
    void destroyWindow();
private:

    QLibrary       m_lib;
    IPlainElGraph* m_grc = nullptr;
};
