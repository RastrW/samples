#ifndef RASTRDATAMODEL_H
#define RASTRDATAMODEL_H

#include <QicsDataModel.h>
#include "astra_exp.h"
#include "License2/json.hpp"
#include "astra_shared.h"

template <typename... Args>
void loggg( int eCod, std::string_view sv_format, Args&&... args ) {
    //const std::string str_log{fmt::format(sv_format, args...)};
}

typedef std::vector
        < std::variant< int, double, std::string > > _col_data ;

class RCol
    : public _col_data
{
public:
    enum _en_data{ // in _col_data
        DATA_ERR =  -1,
        DATA_INT =   0,
        DATA_DBL =   1,
        DATA_STR =   2
    };
    template <typename... Args>
    RCol(Args&&... args)
        : _col_data{args...} {
    }
    virtual ~RCol() = default;
    void setMeta(const nlohmann::json& j_meta_in){
        j_meta_ = j_meta_in;
        en_data_ = _en_data::DATA_ERR;
        const std::string str_Type = j_meta_["Type"];
        int n_type = std::stoi(str_Type);
        enComPropTT com_prop_tt = static_cast<enComPropTT>(n_type);
        switch(com_prop_tt){
            case enComPropTT::COM_PR_INT	   : //= 0,
            case enComPropTT::COM_PR_BOOL	   : //= 3,
            case enComPropTT::COM_PR_ENUM	   : //= 4,
            case enComPropTT::COM_PR_ENPIC	   : //= 5,
            case enComPropTT::COM_PR_COLOR	   : //= 6,
            case enComPropTT::COM_PR_SUPERENUM : //= 7,
            case enComPropTT::COM_PR_TIME	   : //= 8,
            case enComPropTT::COM_PR_HEX	   : //= 9
                en_data_ = _en_data::DATA_INT;
            break;
            case enComPropTT::COM_PR_REAL	   : //= 1,
                en_data_ = _en_data::DATA_DBL;
            break;
            case enComPropTT::COM_PR_STRING	   : //= 2,
                en_data_ = _en_data::DATA_STR;
            break;
        }
    }
    std::string    str_name_;
    _en_data       en_data_;
private:
    nlohmann::json j_meta_;
};// class RCol

class RData
    : public std::vector<RCol> {
public:
    void SetNumRows(long n_new_num_rows){
        n_num_rows_ = n_new_num_rows;
    }
    int AddCol(const RCol& rcol){
  //      if(rcol.size()!=n_num_rows_)
  //          return -1;
        emplace_back(rcol);
        return size();
    }
    std::string getCommaSeparatedFieldNames(){
        std::string str_tmp;
        for( const RCol& col_data : *this ) {
            str_tmp += col_data.str_name_;
            str_tmp += ",";
        }
        if(str_tmp.length()>0){
            str_tmp.erase(str_tmp.length()-1);
        }
        return str_tmp;
    }
    void Trace() const {
        for(const RCol& col : *this){
            qDebug() << " col: " << col.str_name_.c_str();
            for(const _col_data::value_type& cdata : col ){
                switch(col.en_data_){
                    case RCol::_en_data::DATA_INT :
                        qDebug()<<"cdata : "<< std::get<int>(cdata);
                    break;
                    case RCol::_en_data::DATA_DBL :
                        qDebug()<<"cdata : "<< std::get<double>(cdata);
                    break;
                    case RCol::_en_data::DATA_STR :
                        qDebug()<<"cdata : "<< std::get<std::string>(cdata).c_str();
                    break;
                    default:
                        qDebug()<<"cdata : unknown!! ";
                    break;
                }
                //qDebug()<<"cdata : "<< std::to_string(cdata).c_str();
            }
        }
    }
private:
    long n_num_rows_ = 0;
};// class RData





class RastrDataModel
        //: public QObject
        : public QicsDataModel
{
    Q_OBJECT
    const RData& rdata_;
    QicsDataItem* pditem_ = nullptr;
public:
    RastrDataModel(const RData& rdata_in) :
        rdata_(rdata_in){
        /*
       RCol rc_int  {1,3,5,45,6};
       RCol rc_str  {"good","very good", "the best ","excelent","привет медвед"};
       RCol rc_dbl  {12.13, 13.12, 55.05, 66.123, 12.1232323};
       int nRes = 0;
       rdata_.SetNumRows(rc_int.size());
       nRes = rdata_.AddCol(rc_int); Q_ASSERT(nRes>=0);
       nRes = rdata_.AddCol(rc_str); Q_ASSERT(nRes>0);
       nRes = rdata_.AddCol(rc_dbl); Q_ASSERT(nRes>0);
       nRes = rdata_.AddCol(rc_int); Q_ASSERT(nRes>0);
*/
       setNumColumns(rdata_.size());
       //setNumRows(rc_int.size());
       setNumRows(rdata_.begin()->size());
    }
    virtual ~RastrDataModel(){
    }
    const QicsDataItem* item(int row, int col) const
    {
         RastrDataModel* self = const_cast<RastrDataModel *>(this);
         if (self->pditem_){
            delete self->pditem_;
            self->pditem_ = nullptr;
         }
         RData::const_iterator iter_col = rdata_.begin() + col;
         _col_data::const_iterator iter_data = (*iter_col).begin() + row;
         switch((*iter_data).index()){
            case RCol::_en_data::DATA_INT: self->pditem_ = new QicsDataInt    ( std::get<int>(*iter_data) );                 break;
            case RCol::_en_data::DATA_STR: self->pditem_ = new QicsDataString ( std::get<std::string>(*iter_data).c_str() ); break;
            case RCol::_en_data::DATA_DBL: self->pditem_ = new QicsDataDouble ( std::get<double>(*iter_data) );              break;
            default :                      self->pditem_ = new QicsDataString ( "type_unknown" );                            break;
         }
         //  self->pditem_ = new QicsDataString( "dfffd df" );
         return self->pditem_;

        // we need to modify the internal _item data member inside
        // this const method.  This modification is not externally
        // visible, so this const_cast is ok.
        // auto* self = const_cast<RastrDataModel*>(this);
/*
        StockDataModel *self = const_cast<StockDataModel *>(this);
        if (m_item)
            delete self->m_item;
        // We create a new QicsDataItem of the appropriate type
        // and return a const pointer to it.
        if (row < m_stocks.size()) {
            switch (col) {
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
        }else{
            self->m_item = 0;
        }

        return m_item;
*/
        return nullptr;
    }
    QicsDataModelRow rowItems(int row, int first_col, int last_col) const  {
        QicsDataModelRow rv;
       /*
        if ((last_col < 0) || (last_col > lastColumn()))
            last_col = lastColumn();
        // Go through each column in the row and add the data item
        // to the row vector.
        for (int i = first_col; i <= last_col; ++i) {
            const QicsDataItem *itm = item(row, i);
            rv.push_back(itm->clone());
        }*/
        return rv;
    }
    QicsDataModelColumn columnItems(int col, int first_row, int last_row) const  {
        QicsDataModelColumn cv;
        /*
        if ((last_row < 0) || (last_row > lastRow()))
            last_row = lastRow();
        // Go through each row in the column and add the data item
        // to the column vector.
        for (int i = first_row; i <= last_row; ++i) {
            const QicsDataItem *itm = item(i, col);
            cv.push_back(itm->clone());
        }*/
        return cv;
    }
    void setRowItems(int row, const QicsDataModelRow &v)  {
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
    }
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
    }
    void setSymbol(unsigned int idx, QString sym){
        /*
        // We call appropriate StockDataSet method here to actually store
        // the value, and then we emit the required signal.
        StockDataSet::setSymbol(idx, sym);
        if (emitSignals())
            emit modelChanged(QicsRegion(idx, SDM_Symbol, idx, SDM_Symbol));
         */
    }
    void setHigh(unsigned int idx, double val) { /*
        // We call appropriate StockDataSet method here to actually store
        // the value, and then we emit the required signal.
        StockDataSet::setHigh(idx, val);
        if (emitSignals())
            emit modelChanged(QicsRegion(idx, SDM_High, idx, SDM_High));*/
    }
    void setLow(unsigned int idx, double val) { /*
        // We call appropriate StockDataSet method here to actually store
        // the value, and then we emit the required signal.
        StockDataSet::setLow(idx, val);
        if (emitSignals())
            emit modelChanged(QicsRegion(idx, SDM_Low, idx, SDM_Low));*/
    }
    void setClose(unsigned int idx, double val) {/*
        // We call appropriate StockDataSet method here to actually store
        // the value, and then we emit the required signal.
        StockDataSet::setClose(idx, val);
        if (emitSignals())
            emit modelChanged(QicsRegion(idx, SDM_Close, idx, SDM_Close));*/
    }
    void setVolume(unsigned int idx, unsigned int val) {/*
        // We call appropriate StockDataSet method here to actually store
        // the value, and then we emit the required signal.
        StockDataSet::setVolume(idx, val);
        if (emitSignals())
            emit modelChanged(QicsRegion(idx, SDM_Volume, idx, SDM_Volume));*/
    }
    void insertStock(int position) { /*
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
        }*/
    }
    void removeStock(unsigned int idx) {/*
        // We call appropriate StockDataSet method here to actually remove
        // the row, and then we emit the required signal.
        StockDataSet::removeStock(idx);
        setNumRows(numRows() - 1);
        if (emitSignals()) {
            emit rowsDeleted(1, idx);
            emit modelSizeChanged(numRows(), numColumns());
        }*/
    }

    public slots:

    void setItem(int row, int col, const QicsDataItem &item) { /*
        if (row < static_cast<int> (numStocks())) {
            // Here, we determine which stock attribute we should be setting
            // by looking at the column attribute.
            // The modelChanged() signal will be emitted by the individual
            // set methods (setSymbol, setClose, etc).
            switch (col) {
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
        }*/
    }
    void clearItem(int row, int col) { /*
        if (row < static_cast<int> (numStocks())) {
            // We determine which stock attribute we should be clearing
            // by looking at the column attribute.
            // The modelChanged() signal will be emitted by the individual
            // set methods (setSymbol, setClose, etc).
            switch (col) {
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
        }*/
    }
    void clearModel(void) { /*
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
            emit modelSizeChanged(numRows(), numColumns()); */
    }
    void addRows(int number_of_rows) { /*
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
        }*/
    }
    void addColumns(int) {
        // We don't allow adding columns (each stock has a fixed number
        // of data points).  So we just return without emitting any signals
    }
    void insertRows(int number_of_rows, int starting_position) { /*
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
        }  */
    }
    void insertColumns(int, int) {
        // We don't allow inserting columns (each stock has a fixed number
        // of data points).  So we just return without emitting any signals
    }
    void deleteRow(int row) { /*
        // We use the existing removeStock() call to do the delete, which will
        // also emit the required signal.
        if (row < static_cast<int> (numStocks()))
            removeStock(row); */
    }
    void deleteRows(int num_rows, int start_row){ /*
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
        }*/
    }
    void deleteColumn(int) {
        // We don't allow deleting columns (each stock has a fixed number
        // of data points).  So we just return without emitting any signals
    }
    void deleteColumns(int, int) {
        // We don't allow deleting columns (each stock has a fixed number
        // of data points).  So we just return without emitting any signals
    }
};// class RastrDataModel

// This is an example of a custom data model.

///////////////////////// FROM EXAMPLE ///////////////////////////////////////////////

// First, here are some existing classes that define stock data.  These are
// supposed to simulate the kinds of classes that a typical programmer might
// already have when s/he decides to use the QicsTable.

// We will be using these classes when we make the custom data model.

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

class StockDataModel
        : public QicsDataModel
        , public StockDataSet
{
    Q_OBJECT
public:
protected:
    enum {
        SDM_Symbol = 0, // col - 0
        SDM_Close = 1,  // col - 1
        SDM_High = 2,   // col ...
        SDM_Low = 3,
        SDM_Volume = 4,
        SDM_NumDataItems = 5 // num cols
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
            switch (col) {
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
        }else{
            self->m_item = 0;
        }
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
            switch (col) {
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
            switch (col) {
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
    void deleteColumns(int, int) {
        // We don't allow deleting columns (each stock has a fixed number
        // of data points).  So we just return without emitting any signals
    }
};

#endif // RASTRDATAMODEL_H
