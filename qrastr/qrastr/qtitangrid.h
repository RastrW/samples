#ifndef QTITANGRID_H
#define QTITANGRID_H
#pragma once

#include <DevMachines/QtitanGrid>

/* Вопросы по QTitan
 * 1. Как вывести номера строк в гриде?
 * 2. Как спрятать панель группировки по колонкам
 * 3. (+)Как поймать событие щелкчка по ячейке -> onItemPressed(const QModelIndex &index){   ->  SOLVED
 * 4. Как спрятать выбранные не используюя фильтр ?
 * 5. Как сделать тип  multicells Selection мышкой без использования клавиши Shift
 * 6. В продолжении 5 -> выборкой скроллить таблицу ?
 * 7. Как сделть разные контекстные меню по ячейкам и щаголовкам ? (похоже никак)
 * 8. Как добавить строку автофильтра ?
 * 9. В Qtitane не работают шорткаты команд контекстного меню , хотя del встроенный работает.
 * 10. Как сделать комбобокс из картинок (RTabWidget :: column_qt->setEditorType(GridEditor::ComboBoxPicture);)
 * 11. Загрузка файла при открытой форме (узлы) приводит к некорректному отображению таблицы
 * ...

*/

class CustomFilterCondition: public GridFilterCondition {
public:
    CustomFilterCondition(GridFilter* filter = Q_NULL);
    bool isTrue(const QModelIndex& index) override;
    GridFilterCondition* clone() const override;
    QString createPresentation() const override;
    int conditionCount() const override;
#ifndef QTN_NOUSE_XML_MODULE
    bool saveToXML(IXmlStreamWriter *) override;
    bool loadFromXML(IXmlStreamReader *) override;
#endif
    static QString xml_name;
public:
    void addRow(int modelRowIndex);
    void removeRow(int modelRowIndex);
private:
    QSet<int> m_modelRows;
};

#endif // QTITANGRID_H
