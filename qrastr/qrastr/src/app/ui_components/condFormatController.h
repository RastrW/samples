#pragma once
#include <QObject>


class RModel;
namespace Qtitan { class GridTableView; }
class CondFormat;

class CondFormatController : public QObject
{
    Q_OBJECT
public:
    explicit CondFormatController(RModel*                 model,
                                  Qtitan::GridTableView*  view,
                                  QWidget*                parentWidget,
                                  QObject*                parent = nullptr);

    /// Загружает форматы из JSON и применяет к модели.
    void loadFromJson();

    /// Открывает диалог редактирования форматов для колонки column.
    void editCondFormats(std::size_t column);

    /// Только для чтения — используется в ContextMenuBuilder.
    const std::map<int, std::vector<CondFormat>>& formats() const {
        return m_formats;
    }

signals:
    void formatsChanged();   ///< После применения изменений

private slots:
    void onFormatsChanged(); ///< Сохраняет в JSON

private:
    RModel*                                m_model;
    Qtitan::GridTableView*                 m_view;
    QWidget*                               m_parentWidget;
    std::map<int, std::vector<CondFormat>> m_formats;
};
