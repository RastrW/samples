#ifndef FILTERTABLEHEADER_H
#define FILTERTABLEHEADER_H

#include <QHeaderView>
class FilterLineEdit;

class FilterTableHeader : public QHeaderView
{
    Q_OBJECT
public:
    explicit FilterTableHeader(QTableView* parent = nullptr);
    QSize sizeHint() const override;
private:
    std::vector<FilterLineEdit*> filterWidgets;
    size_t columnNumber;
public slots:
    void generateFilters(size_t number, size_t number_of_hidden_filters = 1);
    void adjustPositions();
protected:
    void updateGeometries() override;
};

#endif // FILTERTABLEHEADER_H
