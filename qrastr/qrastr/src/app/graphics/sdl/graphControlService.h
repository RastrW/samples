#pragma once
#include <QLibrary>
#include "IPlainElGraph.h"

class IPlainRastr;

// Сигнатуры экспортов GrCtrlClient.dll
#if defined(Q_OS_WIN)
using GCC_InitControl_t  = void (__cdecl*)(IPlainElGraph*, void*);
using GCC_CloseControl_t = void (__cdecl*)(IPlainElGraph*);
using GCC_InitPlainDLL_t = void(__cdecl*)(IPlainRastr*, const char*);
#else
///@todo
#endif

/**
 * @class Инкапсулирует загрузку GrCtrlClient.dll.
 *
 * Время жизни = времени жизни программы.
 * На каждое открытое окно вызывает InitControl(pcontrol),
 * перед закрытием — CloseControl(pcontrol).
 *
 */
class GraphControlService
{
public:
    GraphControlService(IPlainRastr* rastr);

    GraphControlService(const GraphControlService&)            = delete;
    GraphControlService& operator=(const GraphControlService&) = delete;
    GraphControlService(GraphControlService&&)            = delete;
    GraphControlService& operator=(GraphControlService&&) = delete;

    ~GraphControlService();

    /// Загружает GraphClient.dll. Вызывать один раз при старте.
    bool load();
    /// Выгружает DLL. Вызывать при завершении приложения.
    void unload();
    bool isLoaded() const { return m_loaded; }
    /**
     * @brief Вызвать после успешного CreateChildWindow для нового окна.
     * Устанавливает подписку GCC на хинты pcontrol.
     */
    void initControl(IPlainElGraph* pcontrol);
    /**
     * @brief Вызвать ДО уничтожения окна/ElGraphService.
     * Отписывает GCC от событий pcontrol.
     */
    void closeControl(IPlainElGraph* pcontrol);
private:
    QLibrary            m_lib;
    bool                m_loaded       = false;
    IPlainRastr*        m_rastr        = nullptr;
    GCC_InitControl_t   m_initControl  = nullptr;
    GCC_InitPlainDLL_t  m_initPlainDll = nullptr;
    GCC_CloseControl_t  m_closeControl = nullptr;
};
