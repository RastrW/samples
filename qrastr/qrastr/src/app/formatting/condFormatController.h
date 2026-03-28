#pragma once
#include <QObject>
#include "condFormat.h"

class RModel;
namespace Qtitan { class GridTableView; }
class CondFormat;

///@class оркестрирует всё:
/// load  → читает JSON → пишет в RModel
/// edit  → читает из RModel → диалог → пишет в RModel → JSON
/// save  → читает из RModel → пишет JSON
/// RModel — единственный владелец форматов во время работы программы.
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
    void saveToJson(); ///< Сериализует текущее состояние RModel → JSON

    RModel*                m_model;
    Qtitan::GridTableView* m_view;
    QWidget*               m_parentWidget;
};
