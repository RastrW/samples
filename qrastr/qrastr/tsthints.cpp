#include "qastra.h"
#include "tsthints.h"
#include "common_qrastr.h"
using WrapperExceptionType = std::runtime_error;
//#include <astra/IPlainRastrWrappers.h>
#include "IPlainRastrWrappers.h"

TstHints::TstHints(QWidget *parent)
    //: QWidget{parent} {
    :QTableWidget{parent}{
}
void TstHints::setQAstra(std::weak_ptr<QAstra> wp_qastra_in){
    wp_qastra_ = wp_qastra_in;
}
void TstHints::setTableName(std::string_view sv_tname_in){
    str_tname_ = sv_tname_in;
}
void TstHints::setColNames(const _vstrs& vstrs_col_names_in){
    vstrs_col_names_ = vstrs_col_names_in;
    setColumnCount(0);
    setColumnCount(vstrs_col_names_.size());
    for (int col = 0; col < vstrs_col_names_.size(); ++col){
      setHorizontalHeaderItem(col, new QTableWidgetItem(QString("%1").arg(vstrs_col_names_[col].c_str())));
    }
}
void TstHints::onRastrHint(const _hint_data& hint_data){
    try{
        spdlog::info("i alive");
        std::shared_ptr<QAstra> sp_qastra = wp_qastra_.lock(); assert(sp_qastra);
        if(nullptr!=sp_qastra){
            QAstra::_sp_rastr sp_rastr = sp_qastra->getRastr();
            if(EventHints::ChangeAll == hint_data.hint){
                IRastrTablesPtr tablesx{ sp_rastr->Tables() };
                IRastrPayload tablecount{ tablesx->Count() };
                spdlog::info("tablecount: {}", tablecount.Value() );
                IRastrTablePtr table{ tablesx->Item(str_tname_) };
                IRastrPayload tablesize{ table->Size() };
                spdlog::info("table[{}].tablesize: {}", str_tname_, tablesize.Value());
                IRastrPayload tablename{ table->Name() };
                spdlog::info("{}.tablename: {}", str_tname_, tablename.Value() );
                IRastrObjectPtr<IPlainRastrColumns> columns{ table->Columns() };
                setWindowTitle(tablename.Value().c_str());
                typedef std::vector<IRastrColumnPtr> _v_rastr_col_ptrs;
                _v_rastr_col_ptrs v_rastr_col_ptrs;
                v_rastr_col_ptrs.reserve(vstrs_col_names_.size());
                for (int col = 0; col < vstrs_col_names_.size(); ++col){
                    v_rastr_col_ptrs.emplace_back( columns->Item(vstrs_col_names_[col]) ) ;
                }
                const long n_num_rows = tablesize.Value();
                setRowCount(n_num_rows);
                for (int col = 0; col < vstrs_col_names_.size(); ++col){
                    for (int row = 0; row < n_num_rows; ++row){
                        //setItem(row, col, new QTableWidgetItem(QString("T %1-%2").arg(row + 1).arg(col+1)));
                        IRastrVariantPtr v_ptr{ v_rastr_col_ptrs[col]->Value(row) };
                        IRastrPayload payload{ v_ptr->String() };
                        std::string str = stringutils::acp_decode(payload.Value());
                        setItem(row, col, new QTableWidgetItem(QString("%1").arg(str.c_str()) ));
                    }
                }
            }
            /*
                            IRastrTablesPtr tablesx{ rastr->Tables() };
                            IRastrPayload tablecount{ tablesx->Count() };
                            spdlog::info("tablecount: {}", tablecount.Value() );
                            IRastrTablePtr nodes{ tablesx->Item("node") };

                            IRastrPayload tablesize{ nodes->Size() };
                            spdlog::info("node.tablesize: {}",tablesize.Value());
                            IRastrPayload tablename{ nodes->Name() };
                            spdlog::info("node.tablename: {}",tablename.Value() );
                            IRastrObjectPtr<IPlainRastrColumns> nodecolumns{ nodes->Columns() };
                            IRastrColumnPtr ny{ nodecolumns->Item("ny") };
                            IRastrColumnPtr name{ nodecolumns->Item("name") };
                            IRastrColumnPtr v{ nodecolumns->Item("vras") };
                            IRastrColumnPtr delta{ nodecolumns->Item("delta") };
                            for (long index{ 0 }; index < tablesize.Value(); index++) {
                                IRastrVariantPtr Varny{ ny->Value(index) };
                                IRastrVariantPtr Varname{ name->Value(index) };
                                IRastrVariantPtr Varv{ v->Value(index) };
                                IRastrVariantPtr Vardelta{ delta->Value(index) };

                                IRastrPayload nyvalue{ Varny->Long() };
                                IRastrPayload namevalue{ Varname->String() };
                                IRastrPayload vvalue{ Varv->Double() };
                                IRastrPayload deltavalue{ Vardelta->Double() };
                                spdlog::info( "ny: {:10} name: {:15} vras: {:3.2f} delta: {:2.3f}"
                                    , nyvalue.Value()
                                    , stringutils::acp_decode(namevalue.Value())
                                    , vvalue.Value()
                                    , deltavalue.Value()
                                );
                            }
            */
        }
    }catch(const std::exception& ex){
        spdlog::error("std::exception: {}", ex.what());
    }catch(...){
        spdlog::error("Exception in TstHints::onRastrHint(...)!");
    }
}
