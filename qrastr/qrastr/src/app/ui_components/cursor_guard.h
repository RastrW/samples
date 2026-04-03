#pragma once

#include <QApplication>

/**
 * @brief RAII-обёртка для курсора ожидания.
 *
 * Устанавливает Qt::WaitCursor в конструкторе и гарантированно
 * восстанавливает исходный курсор в деструкторе — даже при исключении.
 *
 * Использование:
 * @code
 *   {
 *       CursorGuard guard;          // курсор → WaitCursor
 *       riskyOperation();           // исключение? — деструктор всё равно сработает
 *   }                               // курсор восстановлен автоматически
 * @endcode
 */
class CursorGuard {
public:
    CursorGuard()                          { QApplication::setOverrideCursor(Qt::WaitCursor); }
    ~CursorGuard()                         { QApplication::restoreOverrideCursor(); }

    // Запрещаем копирование и перемещение — объект владеет ресурсом
    CursorGuard(const CursorGuard&)            = delete;
    CursorGuard& operator=(const CursorGuard&) = delete;
    CursorGuard(CursorGuard&&)                 = delete;
    CursorGuard& operator=(CursorGuard&&)      = delete;
};