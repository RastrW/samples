#ifndef QASTRA_H
#define QASTRA_H
#pragma once

#include <QObject>
#include "common_qrastr.h"
#include "qastra_events_data.h"

class CUIFormsCollection;
class QAstra
    : public QObject
    , public IRastrEventsSinkBase{
    Q_OBJECT
public:
    typedef std::shared_ptr<IPlainRastr> _sp_rastr;
    explicit  QAstra(QObject *parent = nullptr);
    virtual   ~QAstra() = default;
    static spdlog::level::level_enum getSpdLevel(const LogMessageTypes lmt);
    IPlainRastrRetCode OnEvent(const IRastrEventLog& Event) noexcept override;
    IPlainRastrRetCode OnEvent(const IRastrEventHint& Event) noexcept override;
    IPlainRastrRetCode OnEvent(const IRastrEventBase& Event) noexcept override;
    IPlainRastrRetCode OnUICommand(const IRastrEventBase& Event, IPlainRastrVariant* Result) noexcept override;
    static const char* const getHintName(EventHints eh);
    void      setRastr(const _sp_rastr& sp_rastr_in);
    _sp_rastr getRastr() const;
    IPlainRastrRetCode Load( eLoadCode LoadCode, const std::string_view& FilePath, const std::string_view& TemplatePath );
    void      Save( const std::string_view& FilePath, const std::string_view& TemplatePath ) ;
    eASTCode  Kdd(const std::string_view& parameters = {});
    eASTCode  Rgm(const std::string_view& parameters = {});
    eASTCode  Opf(const std::string_view& parameters = {});
    eASTCode  SMZU(const std::string_view& parameters = {});
    void  CalcIdop(double Temp, double Pcab , const std::string_view& selection = {} , bool IgnoreTTables = false);
    eASTCode  Kz(const std::string_view& parameters, eNonsym Nonsym, long p1, long p2, long p3, double LengthFromP1InProc, double rd, double z_re, double z_im);
    FieldVariantData GetVal( const std::string_view& Table, const std::string_view& Col , const long row );
    std::string GetStringVal( const std::string_view& Table, const std::string_view& Col , const long row );
signals:
    void onRastrHint ( const _hint_data& );
    void onRastrLog  ( const _log_data&  );
    void onRastrPrint( const std::string& );
private:
    _sp_rastr sp_rastr_;
     std::unique_ptr<CUIFormsCollection> upCUIFormsCollection_;
};

#endif // QASTRA_H
