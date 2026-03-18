#pragma once

#include <QLibrary>
#include "IPlainElGraph.h"

// Вызывательное соглашение InitPlainDLL — __cdecl только на Windows.
// На других платформах DLL скорее всего недоступна,
// но код должен компилироваться без ошибок.
#if defined(Q_OS_WIN)
using InitPlainDLL_t = IPlainElGraph* (__cdecl*)();
#else
using InitPlainDLL_t = IPlainElGraph* (*)();
#endif

// Инкапсулирует загрузку ElGraphCtrl.dll и владение IPlainElGraph*.
// Не синглтон — каждый SDLChild владеет своим экземпляром.
// Время жизни: ровно такое же, как у владеющего SDLChild.
class ElGraphService
{
public:
    ElGraphService() = default;

    // Запрет копирования: QLibrary и IPlainElGraph* — уникальные ресурсы
    ElGraphService(const ElGraphService&)            = delete;
    ElGraphService& operator=(const ElGraphService&) = delete;

    // Перемещение не нужно — объект живёт как поле SDLChild
    ElGraphService(ElGraphService&&)            = delete;
    ElGraphService& operator=(ElGraphService&&) = delete;

    ~ElGraphService();

    // Загружает DLL, вызывает InitPlainDLL(), затем CreateChildWindow(parentHwnd).
    // parentHwnd — нативный HWND окна SDL на Windows, nullptr на других платформах.
    // При любой ошибке логирует через spdlog и возвращает false.
    // После успешного вызова isLoaded() == true.
    bool init(void* parentHwnd);

    // Уничтожает grc и выгружает библиотеку.
    // Безопасно вызывать повторно.
    void shutdown();

    // Доступ к интерфейсу библиотеки (nullptr если не загружена)
    IPlainElGraph* graph() const { return m_grc; }
    bool isLoaded()        const { return m_grc != nullptr; }

private:
    // QLibrary("ElGraphCtrl") — Qt сам добавит .dll / .so / .dylib
    QLibrary       m_lib{"ElGraphCtrl"};
    IPlainElGraph* m_grc = nullptr;
};
