#ifndef FILTERLINEEDIT_H
#define FILTERLINEEDIT_H
#pragma once

#include <QLineEdit>

class FilterLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit FilterLineEdit(QWidget* parent, std::vector<FilterLineEdit*>* filters = nullptr, size_t columnnum = 0);
private:
    std::vector<FilterLineEdit*>* filterList;
    size_t columnNumber;
};

#endif // FILTERLINEEDIT_H
