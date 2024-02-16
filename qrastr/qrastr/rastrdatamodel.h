#ifndef RASTRDATAMODEL_H
#define RASTRDATAMODEL_H

#include <QicsDataModel.h>

class RastrDataModel
        : public QObject
        //: public QicsDataModel
{
    Q_OBJECT
public:
    RastrDataModel() ;
    virtual ~RastrDataModel() = default;
    const QicsDataItem *item(int row, int col) const;
    QicsDataModelRow rowItems(int row, int first_col=0, int last_col=-1) const {
        QicsDataModelRow rv;
    /*
        if ((last_col < 0) || (last_col > lastColumn()))
            last_col = lastColumn();

        // Go through each column in the row and add the data item
        // to the row vector.

        for (int i = first_col; i <= last_col; ++i) {
            const QicsDataItem *itm = item(row, i);
            rv.push_back(itm->clone());
        }
    */
        return rv;
    };
     QicsDataModelColumn columnItems(int col, int first_row=0, int last_row=-1) const {
         QicsDataModelColumn cv;
     /*
         if ((last_row < 0) || (last_row > lastRow()))
             last_row = lastRow();

         // Go through each row in the column and add the data item
         // to the column vector.

         for (int i = first_row; i <= last_row; ++i) {
             const QicsDataItem *itm = item(i, col);
             cv.push_back(itm->clone());
         }
     */
         return cv;

     };

     void setRowItems(int row, const QicsDataModelRow &v){
         /*
         QicsDataModelRow::const_iterator iter;
         int col = 0;

         // Temporarily turn signal emitting off, so setItem() doesn't emit
         // a signal for each cell that is changed.

         bool old_emit = emitSignals();
         setEmitSignals(false);

         // Iterate through each value in the row vector and use setItem()
         // to do the actual "setting".

         for (iter = v.begin(); iter != v.end(); ++iter) {
             if (col > (SDM_NumDataItems - 1))
                 break;
             setItem(row, col++, **iter);
         }

         // Restore previous signal emitting setting.

         setEmitSignals(old_emit);

         if (emitSignals())
             emit modelChanged(QicsRegion(row, 0, row, col-1));
         */
     };

     void setColumnItems(int col, const QicsDataModelColumn &v){
         /*
         QicsDataModelColumn::const_iterator iter;
         int row = 0;

         // Temporarily turn signal emitting off, so setItem() doesn't emit
         // a signal for each cell that is changed.

         bool old_emit = emitSignals();
         setEmitSignals(false);

         // Iterate through each value in the column vector and use setItem()
         // to do the actual "setting".

         for (iter = v.begin(); iter != v.end(); ++iter) {
             if (row > lastRow())
                 break;

             setItem(row++, col, **iter);
         }

         // Restore previous signal emitting setting.

         setEmitSignals(old_emit);

         if (emitSignals())
             emit modelChanged(QicsRegion(0, col, row-1, col));
         */


     };
/*
     // reimplemented methods of the StockDataSet
     void setSymbol(unsigned int idx, QString sym);
     void setHigh(unsigned int idx, double val);
     void setLow(unsigned int idx, double val);
     void setClose(unsigned int idx, double val);
     void setVolume(unsigned int idx, unsigned int val);
     void insertStock(int position = -1);
     void removeStock(unsigned int idx);

 public slots:
     void setItem(int row, int col, const QicsDataItem &item);
     void clearItem(int row, int col);
     void clearModel(void);
     void addRows(int number_of_rows);
     void addColumns(int number_of_columns);
     void insertRows(int number_of_rows, int starting_position);
     void insertColumns(int number_of_columns, int starting_position);
     void deleteRow(int row);
     void deleteRows(int num_rows, int start_row);
     void deleteColumn(int col);
     void deleteColumns(int num_cols, int start_col);

 protected:
     enum { SDM_Symbol = 0, SDM_Close = 1, SDM_High = 2,
         SDM_Low = 3, SDM_Volume = 4, SDM_NumDataItems = 5 } StockDataIndex;

     QicsDataItem *m_item;
*/

};// class RastrDataModel

// This is an example of a custom data model.

////////////////////////////////////////////////////////////////////////

// First, here are some existing classes that define stock data.  These are
// supposed to simulate the kinds of classes that a typical programmer might
// already have when s/he decides to use the QicsTable.

// We will be using these classes when we make the custom data model.

//class StockPrivate;
// Represents a single stock
class StockPrivate
{
public:
    StockPrivate() {}
    virtual ~StockPrivate() {}

    inline QString symbol() const { return m_symbol; }
    inline void setSymbol(const QString &sym) { m_symbol = sym; }

    inline double high() const { return m_high; }
    inline void setHigh(double val) { m_high = val; }

    inline double low() const { return m_low; }
    inline void setLow(double val) { m_low = val; }

    inline double close() const { return m_close; }
    inline void setClose(double val) { m_close = val; }

    inline unsigned int volume() const { return m_volume; }
    inline void setVolume(int val) { m_volume = val; }

protected:
    QString m_symbol;
    double m_high;
    double m_low;
    double m_close;
    unsigned int m_volume;
};

// Represents a set of stocks
class StockDataSet
{
public:

   /*
    StockDataSet();
    virtual ~StockDataSet();

    inline unsigned int numStocks(void) const
    { return m_stocks.size(); }

    virtual void insertStock(int position = -1);
    virtual void removeStock(int idx);

    QString symbol(int idx) const;
    virtual void setSymbol(int idx, QString sym);

    double high(int idx) const;
    virtual void setHigh(int idx, double val);

    double low(int idx) const;
    virtual void setLow(int idx, double val);

    double close(int idx) const;
    virtual void setClose(int idx, double val);

    unsigned int volume(int idx) const;
    virtual void setVolume(int idx, unsigned int val);
    */

    inline unsigned int numStocks(void) const {
        return m_stocks.size();
    }
    StockDataSet(){
    }
    ~StockDataSet() {
    }
    void insertStock(int position) {
        if (position == -1)
            position = m_stocks.size();
        StockPrivate *sd = new StockPrivate();
        int count = 0;
        StockList::iterator iter;
        for (iter = m_stocks.begin(); iter != m_stocks.end(); ++iter) {
            if (count == position)
                break;
            ++count;
        }
        m_stocks.insert(iter, sd);
    }
    void removeStock(int idx) {
        if (idx >= m_stocks.size())
            return;
        int count = 0;
        StockList::iterator iter;
        for (iter = m_stocks.begin(); iter != m_stocks.end(); ++iter) {
            if (count == idx) {
                delete (*iter);
                break;
            }
            ++count;
        }
        m_stocks.erase(iter);
    }
    QString symbol(int idx) const {
        if (idx < m_stocks.size())
            return m_stocks.at(idx)->symbol();
        return QString();
    }
    void setSymbol(int idx, QString sym) {
        if (idx < m_stocks.size())
            m_stocks.at(idx)->setSymbol(sym);
    }
    double high(int idx) const  {
        if (idx < m_stocks.size())
            return m_stocks.at(idx)->high();
        return -1.0;
    }
    void setHigh(int idx, double val) {
        if (idx < m_stocks.size())
            m_stocks.at(idx)->setHigh(val);
    }
    double low(int idx) const {
        if (idx < m_stocks.size())
            return m_stocks.at(idx)->low();
        return -1.0;
    }
    void setLow(int idx, double val) {
        if (idx < m_stocks.size())
            m_stocks.at(idx)->setLow(val);
    }
    double close(int idx) const {
        if (idx < m_stocks.size())
            return m_stocks.at(idx)->close();
        return -1.0;
    }
    void setClose(int idx, double val) {
        if (idx < m_stocks.size())
            m_stocks.at(idx)->setClose(val);
    }
    unsigned int volume(int idx) const  {
        if (idx < m_stocks.size())
            return m_stocks.at(idx)->volume();
        return 0;
    }
    void setVolume(int idx, unsigned int val)  {
        if (idx < m_stocks.size())
            m_stocks.at(idx)->setVolume(val);
    }
protected:
    typedef QVector<StockPrivate *> StockList;
    StockList m_stocks;
};

////////////////////////////////////////////////////////////////////////

class StockDataModel : public QicsDataModel, public StockDataSet
{
    Q_OBJECT
public:
   /*
    StockDataModel();
    virtual ~StockDataModel();

    const QicsDataItem *item(int row, int col) const;

    QicsDataModelRow rowItems(int row, int first_col=0, int last_col=-1) const;
    QicsDataModelColumn columnItems(int col, int first_row=0, int last_row=-1) const;

    void setRowItems(int row, const QicsDataModelRow &v);
    void setColumnItems(int col, const QicsDataModelColumn &v);

    // reimplemented methods of the StockDataSet
    void setSymbol(unsigned int idx, QString sym);
    void setHigh(unsigned int idx, double val);
    void setLow(unsigned int idx, double val);
    void setClose(unsigned int idx, double val);
    void setVolume(unsigned int idx, unsigned int val);
    void insertStock(int position = -1);
    void removeStock(unsigned int idx);

public slots:
    void setItem(int row, int col, const QicsDataItem &item);
    void clearItem(int row, int col);
    void clearModel(void);
    void addRows(int number_of_rows);
    void addColumns(int number_of_columns);
    void insertRows(int number_of_rows, int starting_position);
    void insertColumns(int number_of_columns, int starting_position);
    void deleteRow(int row);
    void deleteRows(int num_rows, int start_row);
    void deleteColumn(int col);
    void deleteColumns(int num_cols, int start_col);
*/
protected:
    enum {
        SDM_Symbol = 0,
        SDM_Close = 1,
        SDM_High = 2,
        SDM_Low = 3,
        SDM_Volume = 4,
        SDM_NumDataItems = 5
    } StockDataIndex;

    QicsDataItem *m_item;

public:

    StockDataModel()
        : QicsDataModel(),
          StockDataSet() {
        m_item = nullptr;
        // our data model always has SDM_NumDataItems columns
        setNumColumns(SDM_NumDataItems);
    }
    virtual ~StockDataModel()
    {
        delete m_item;
    }
    const QicsDataItem* item(int row, int col) const
    {
        // we need to modify the internal _item data member inside
        // this const method.  This modification is not externally
        // visible, so this const_cast is ok.
        StockDataModel *self = const_cast<StockDataModel *>(this);
        if (m_item)
            delete self->m_item;
        // We create a new QicsDataItem of the appropriate type
        // and return a const pointer to it.
        if (row < m_stocks.size()) {
            switch (col)
            {
                case SDM_Symbol:
                    self->m_item = new QicsDataString(symbol(row));
                break;
                case SDM_Close:
                    self->m_item = new QicsDataDouble(close(row));
                break;
                case SDM_High:
                    self->m_item = new QicsDataDouble(high(row));
                break;
                case SDM_Low:
                    self->m_item = new QicsDataDouble(low(row));
                break;
                case SDM_Volume:
                    self->m_item = new QicsDataInt(volume(row));
                break;
            default:
                self->m_item = 0;
            }
        }
        else
            self->m_item = 0;
        return m_item;
    }
    QicsDataModelRow rowItems(int row, int first_col, int last_col) const  {
        QicsDataModelRow rv;
        if ((last_col < 0) || (last_col > lastColumn()))
            last_col = lastColumn();
        // Go through each column in the row and add the data item
        // to the row vector.
        for (int i = first_col; i <= last_col; ++i) {
            const QicsDataItem *itm = item(row, i);
            rv.push_back(itm->clone());
        }
        return rv;
    }
    QicsDataModelColumn columnItems(int col, int first_row, int last_row) const  {
        QicsDataModelColumn cv;
        if ((last_row < 0) || (last_row > lastRow()))
            last_row = lastRow();
        // Go through each row in the column and add the data item
        // to the column vector.
        for (int i = first_row; i <= last_row; ++i) {
            const QicsDataItem *itm = item(i, col);
            cv.push_back(itm->clone());
        }
        return cv;
    }
    void setRowItems(int row, const QicsDataModelRow &v)  {
        QicsDataModelRow::const_iterator iter;
        int col = 0;
        // Temporarily turn signal emitting off, so setItem() doesn't emit
        // a signal for each cell that is changed.
        bool old_emit = emitSignals();
        setEmitSignals(false);
        // Iterate through each value in the row vector and use setItem()
        // to do the actual "setting".
        for (iter = v.begin(); iter != v.end(); ++iter) {
            if (col > (SDM_NumDataItems - 1))
                break;
            setItem(row, col++, **iter);
        }
        // Restore previous signal emitting setting.
        setEmitSignals(old_emit);
        if (emitSignals())
            emit modelChanged(QicsRegion(row, 0, row, col-1));
    }
    void setColumnItems(int col, const QicsDataModelColumn &v){
        QicsDataModelColumn::const_iterator iter;
        int row = 0;
        // Temporarily turn signal emitting off, so setItem() doesn't emit
        // a signal for each cell that is changed.
        bool old_emit = emitSignals();
        setEmitSignals(false);
        // Iterate through each value in the column vector and use setItem()
        // to do the actual "setting".
        for (iter = v.begin(); iter != v.end(); ++iter) {
            if (row > lastRow())
                break;
            setItem(row++, col, **iter);
        }
        // Restore previous signal emitting setting.
        setEmitSignals(old_emit);
        if (emitSignals())
            emit modelChanged(QicsRegion(0, col, row-1, col));
    }
    void setSymbol(unsigned int idx, QString sym){
        // We call appropriate StockDataSet method here to actually store
        // the value, and then we emit the required signal.
        StockDataSet::setSymbol(idx, sym);
        if (emitSignals())
            emit modelChanged(QicsRegion(idx, SDM_Symbol, idx, SDM_Symbol));
    }
    void setHigh(unsigned int idx, double val) {
        // We call appropriate StockDataSet method here to actually store
        // the value, and then we emit the required signal.
        StockDataSet::setHigh(idx, val);
        if (emitSignals())
            emit modelChanged(QicsRegion(idx, SDM_High, idx, SDM_High));
    }
    void setLow(unsigned int idx, double val) {
        // We call appropriate StockDataSet method here to actually store
        // the value, and then we emit the required signal.
        StockDataSet::setLow(idx, val);
        if (emitSignals())
            emit modelChanged(QicsRegion(idx, SDM_Low, idx, SDM_Low));
    }
    void setClose(unsigned int idx, double val) {
        // We call appropriate StockDataSet method here to actually store
        // the value, and then we emit the required signal.
        StockDataSet::setClose(idx, val);
        if (emitSignals())
            emit modelChanged(QicsRegion(idx, SDM_Close, idx, SDM_Close));
    }
    void setVolume(unsigned int idx, unsigned int val) {
        // We call appropriate StockDataSet method here to actually store
        // the value, and then we emit the required signal.
        StockDataSet::setVolume(idx, val);
        if (emitSignals())
            emit modelChanged(QicsRegion(idx, SDM_Volume, idx, SDM_Volume));
    }
    void insertStock(int position) {
        // We call appropriate StockDataSet method here to actually insert
        // the row, and then we emit the required signal.
        StockDataSet::insertStock(position);
        setNumRows(numRows() + 1);
        if (emitSignals()) {
            if (position == -1)
                emit rowsAdded(1);
            else
                emit rowsInserted(1, position);

            emit modelSizeChanged(numRows(), numColumns());
        }
    }
    void removeStock(unsigned int idx) {
        // We call appropriate StockDataSet method here to actually remove
        // the row, and then we emit the required signal.
        StockDataSet::removeStock(idx);
        setNumRows(numRows() - 1);
        if (emitSignals()) {
            emit rowsDeleted(1, idx);
            emit modelSizeChanged(numRows(), numColumns());
        }
    }

    public slots:

    void setItem(int row, int col, const QicsDataItem &item) {
        if (row < static_cast<int> (numStocks())) {
            // Here, we determine which stock attribute we should be setting
            // by looking at the column attribute.
            // The modelChanged() signal will be emitted by the individual
            // set methods (setSymbol, setClose, etc).
            switch (col)
            {
                case SDM_Symbol:
                    if (item.type() == QicsDataItem_String) {
                        const QicsDataString *ds = static_cast<const QicsDataString *> (&item);
                        setSymbol(row, ds->data());
                    }
                break;
                case SDM_Close:
                    if (item.type() == QicsDataItem_Double) {
                        const QicsDataDouble *df = static_cast<const QicsDataDouble *> (&item);
                        setClose(row, df->data());
                    }
                break;
                case SDM_High:
                    if (item.type() == QicsDataItem_Double) {
                        const QicsDataDouble *df = static_cast<const QicsDataDouble *> (&item);
                        setHigh(row, df->data());
                    }
                break;
                case SDM_Low:
                    if (item.type() == QicsDataItem_Double) {
                        const QicsDataDouble *df = static_cast<const QicsDataDouble *> (&item);
                        setLow(row, df->data());
                    }
                break;
                case SDM_Volume:
                    if (item.type() == QicsDataItem_Int) {
                        const QicsDataInt *di =	static_cast<const QicsDataInt *> (&item);
                        setVolume(row, di->data());
                    }
                break;
                default:
                break;
            }
        }
    }
    void clearItem(int row, int col) {
        if (row < static_cast<int> (numStocks())) {
            // We determine which stock attribute we should be clearing
            // by looking at the column attribute.
            // The modelChanged() signal will be emitted by the individual
            // set methods (setSymbol, setClose, etc).
            switch (col)
            {
                case SDM_Symbol:
                    setSymbol(row, QString());
                break;
                case SDM_Close:
                    setClose(row, 0.0);
                break;
                case SDM_High:
                    setHigh(row, 0.0);
                break;
                case SDM_Low:
                    setLow(row, 0.0);
                break;
                case SDM_Volume:
                    setClose(row, 0);
                break;
                default:
                break;
            }
        }
    }
    void clearModel(void) {
        int nrows = numStocks();
        // Temporarily turn signal emitting off, so removeStock() doesn't emit
        // a signal for each row that is removed.
        bool old_emit = emitSignals();
        setEmitSignals(false);
        for (int i = 0; i < nrows; ++i)
            removeStock(0);
        // Restore previous signal emitting setting.
        setEmitSignals(old_emit);
        if (emitSignals())
            emit modelSizeChanged(numRows(), numColumns());
    }
    void addRows(int number_of_rows) {
        // We use the existing insertStock() call to do the add, but we
        // have to emit the required signal so all views will update
        // Temporarily turn signal emitting off, so insertStock() doesn't emit
        // a signal for each row that is inserted.
        bool old_emit = emitSignals();
        setEmitSignals(false);
        for (int i = 0; i < number_of_rows; ++i)
            insertStock(-1);
        // Restore previous signal emitting setting.
        setEmitSignals(old_emit);
        if (emitSignals()) {
            emit rowsAdded(number_of_rows);
            emit modelSizeChanged(numRows(), numColumns());
        }
    }
    void addColumns(int) {
        // We don't allow adding columns (each stock has a fixed number
        // of data points).  So we just return without emitting any signals
    }
    void insertRows(int number_of_rows, int starting_position) {
        // We use the existing insertStock() call to do the insert, but we
        // have to emit the required signal so all views will update
        // Temporarily turn signal emitting off, so setItem() doesn't emit
        // a signal for each cell that is changed.
        bool old_emit = emitSignals();
        setEmitSignals(false);
        for (int i = 0; i < number_of_rows; ++i)
            insertStock(starting_position);
        // Restore previous signal emitting setting.
        setEmitSignals(old_emit);
        if (emitSignals()) {
            emit rowsInserted(number_of_rows, starting_position);
            emit modelSizeChanged(numRows(), numColumns());
        }
    }
    void insertColumns(int, int) {
        // We don't allow inserting columns (each stock has a fixed number
        // of data points).  So we just return without emitting any signals
    }
    void deleteRow(int row) {
        // We use the existing removeStock() call to do the delete, which will
        // also emit the required signal.
        if (row < static_cast<int> (numStocks()))
            removeStock(row);
    }
    void deleteRows(int num_rows, int start_row){
        // We use the existing removeStock() call to do the delete, but we
        // have to emit the required signal so all views will update
        // Temporarily turn signal emitting off, so setItem() doesn't emit
        // a signal for each cell that is changed.
        bool old_emit = emitSignals();
        setEmitSignals(false);
        int num_deleted = 0;
        for (int i = 0; i < num_rows; ++i) {
            if (start_row < static_cast<int> (numStocks())) {
                removeStock(start_row);
                ++num_deleted;
            }
        }
        // Restore previous signal emitting setting.
        setEmitSignals(old_emit);
        if (num_deleted > 0 && emitSignals()) {
            emit rowsDeleted(1, num_deleted);
            emit modelSizeChanged(numRows(), numColumns());
        }
    }
    void deleteColumn(int) {
        // We don't allow deleting columns (each stock has a fixed number
        // of data points).  So we just return without emitting any signals
    }
    void deleteColumns(int, int){
        // We don't allow deleting columns (each stock has a fixed number
        // of data points).  So we just return without emitting any signals
    }
};


#endif // RASTRDATAMODEL_H
