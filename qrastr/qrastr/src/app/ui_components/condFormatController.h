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
                                  QWidget*                parentWidget);

    /// Загружает форматы из JSON и применяет к модели.
    void loadFromJson();

    /// Открывает диалог редактирования форматов для колонки column.
    void editCondFormats(std::size_t column); 

private:
    void onFormatsChanged(); ///< Сохраняет в JSON

    RModel*                                m_model;
    Qtitan::GridTableView*                 m_view;
    QWidget*                               m_parentWidget;
    std::map<int, std::vector<CondFormat>> m_formats;
};
