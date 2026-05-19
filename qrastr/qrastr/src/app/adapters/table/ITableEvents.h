#pragma once
#include <QObject>
#include <string>

// Сигнальный интерфейс — только для подписки.
class ITableEvents : public QObject {
    Q_OBJECT
public:
    explicit ITableEvents(QObject* parent = nullptr)
        : QObject(parent) {}
    virtual ~ITableEvents() = default;

signals:
    /**
     * Данные изменились в прямоугольнике [rowFrom..rowTo] × [colFrom..colTo].
     * @param colFrom  имя колонки начала диапазона; пустая строка = с первой
     * @param colTo    имя колонки конца диапазона;  пустая строка = до последней
     */
    void sig_dataChanged(const std::string& tname,
                         int rowFrom, const std::string& colFrom,
                         int rowTo,   const std::string& colTo);
    ///< перестроение модели
    void sig_beginResetModel  (const std::string& tname);
    void sig_endResetModel    (const std::string& tname);
    ///< вставка строки
    void sig_beginInsertRow   (const std::string& tname, int first, int last);
    void sig_endInsertRow     (const std::string& tname);
    ///< удаление строк
    void sig_beginRemoveRows  (const std::string& tname, int first, int last);
    void sig_endRemoveRows    (const std::string& tname);
    ///< обновление представлений
    void sig_updateModel      (const std::string& tname);
    void sig_updateView       (const std::string& tname);
    void sig_resetModel       (const std::string& tname);
    /// Строки таблицы tname были добавлены/удалены —
    /// все NAMEREF/SUPERENUM, ссылающиеся на неё, должны обновить кеш.
    void sig_referenceChanged (const std::string& tname);
};