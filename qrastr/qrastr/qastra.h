#ifndef QASTRA_H
#define QASTRA_H

#include <QObject>
//#include "IPlainRastr.h"
#include "common_qrastr.h"
#include "qastra_events_data.h"

class CUIFormsCollection;
class QAstra
    : public QObject
    , public IRastrEventsSinkBase{
    Q_OBJECT
public:
    typedef std::shared_ptr<IPlainRastr> _sp_rastr;
    static spdlog::level::level_enum getSpdLevel(const LogMessageTypes lmt){
        spdlog::level::level_enum level_out = spdlog::level::level_enum::off;
        switch(lmt){
            case LogMessageTypes::SystemError:
                level_out = spdlog::level::level_enum::critical;
            break;
            case LogMessageTypes::Failed:
                level_out = spdlog::level::level_enum::err;
            break;
            case LogMessageTypes::Error:
                level_out = spdlog::level::level_enum::err;
            break;
            case LogMessageTypes::Warning:
                level_out = spdlog::level::level_enum::warn;
            break;
            case LogMessageTypes::Message:
                level_out = spdlog::level::level_enum::info;
            break;
            case LogMessageTypes::Info:
                level_out = spdlog::level::level_enum::info;
            break;
            case LogMessageTypes::OpenStage:
                //assert(!"OpenStage");
            break;
            case LogMessageTypes::CloseStage:
                //assert(!"CloseStage");
            break;
            case LogMessageTypes::EnterDefault:
                assert(!"EnterDefault");
            break;
            case LogMessageTypes::Reset:
                assert(!"Reset");
            break;
            case LogMessageTypes::None:
                assert(!"Node");
            break;
            default:
                assert(!"unknown log message type!");
            break;
        };
        return level_out;
    }
    IPlainRastrRetCode OnEvent(const IRastrEventLog& Event) noexcept override {
        //spdlog::info( "OnEvent.Log Status: {:3}  StageId: {:2} EventMsg: {:40} Table: {:10}  Column: {:5} Index: {:5} UIForm: {:10}"
        spdlog::log( getSpdLevel(Event.Status()), "OnEvent.Log Status: {:3}  StageId: {:2} EventMsg: {:40} Table: {:10}  Column: {:5} Index: {:5} UIForm: {:10}"
            , static_cast<std::underlying_type<LogMessageTypes>::type>(Event.Status())
            , Event.StageId()
            , Event.Message()
            , Event.DBLocation().Table()
            , Event.DBLocation().Column()
            , Event.DBLocation().Index()
            , Event.UIForm()
        );
        _log_data log_data;
        log_data.lmt        = Event.Status();
        log_data.n_stage_id = Event.StageId();
        log_data.str_msg    = Event.Message();
        log_data.str_table  = Event.DBLocation().Table();
        log_data.str_col    = Event.DBLocation().Column();
        log_data.n_indx     = Event.DBLocation().Index();
        log_data.str_uiform = Event.UIForm();
        emit onRastrLog(log_data);
        return IPlainRastrRetCode::Ok;
    }
    IPlainRastrRetCode OnEvent(const IRastrEventHint& Event) noexcept override {
        _hint_data dh;
        dh.hint       = Event.Hint();
        dh.str_table  = Event.DBLocation().Table();
        dh.str_column = Event.DBLocation().Column();
        dh.n_indx     = Event.DBLocation().Index();
        spdlog::info( "OnEvent.Hint: [{:10}] [{:1}] Table: {:10} Column: {:5} Index: {}"
            , getHintName(Event.Hint())
            , static_cast<std::underlying_type<EventHints>::type>(Event.Hint())
            , Event.DBLocation().Table()
            , Event.DBLocation().Column()
            , Event.DBLocation().Index()
        );
        emit onRastrHint( dh );
        return IPlainRastrRetCode::Ok;
    }
    IPlainRastrRetCode OnEvent(const IRastrEventBase& Event) noexcept override {
        spdlog::info( "OnEvent.Base: [{}]", static_cast<std::underlying_type<EventTypes>::type>(Event.Type()) );
        //emit onEvent( static_cast<std::underlying_type<EventTypes>::type>(Event.Type()) );
        if(Event.Type() == EventTypes::Print)
            spdlog::info( "OnEvent.Print: {}", static_cast<const IRastrEventPrint&>(Event).Message() );
        /*
    Log,		// протокол
    Print,		// вывод скрипта
    Hint,		// хинт изменения
    Command		// хинт команды UI
*/
        return IPlainRastrRetCode::Ok;
    }
    IPlainRastrRetCode OnUICommand(const IRastrEventBase& Event, IPlainRastrVariant* Result) noexcept override {
        EventTypes et = Event.Type();
        std::string str;
        if(Event.Type() == EventTypes::Hint){
            str = fmt::format("[{}][{}] {} {} {} "
                , getHintName(static_cast<const IRastrEventHint&>(Event).Hint())
                , static_cast<std::underlying_type<EventHints>::type>( static_cast<const IRastrEventHint&>(Event).Hint() )
                , static_cast<const IRastrEventHint&>(Event).DBLocation().Table()
                , static_cast<const IRastrEventHint&>(Event).DBLocation().Column()
                , static_cast<const IRastrEventHint&>(Event).DBLocation().Index()
            );
        }else if(Event.Type() == EventTypes::Command){
            str = fmt::format("{} {} {}"
                , static_cast<const IRastrEventCommand&>(Event).Arg1() // stringutils::acp_encode
                , static_cast<const IRastrEventCommand&>(Event).Arg2()
                , static_cast<const IRastrEventCommand&>(Event).Arg3()
            );
        }else if(Event.Type() == EventTypes::Log){
            const auto& w = static_cast<const IRastrEventLog&>(Event);
            str = fmt::format( "EventTypes::Log" );
        }else if(Event.Type() == EventTypes::Print){
            const auto& w = static_cast<const IRastrEventPrint&>(Event);
            str = fmt::format( "EventTypes::Print" );
        }
        spdlog::info( "OnUICommand ({}): {}", static_cast<std::underlying_type<EventTypes>::type>( Event.Type()), str );
        Result->String("Done");
        return IPlainRastrRetCode::Ok;
    }
    static const char* const getHintName(EventHints eh){
        switch(eh){
            case EventHints::None:             return "hint_None";
            case EventHints::ChangeAll:        return "hint_ChangeAll";
            case EventHints::ChangeColumn:     return "hint_ChangeColumn";
            case EventHints::ChangeRow:        return "hint_ChangeRow";
            case EventHints::ChangeData:       return "hint_ChangeData";
            case EventHints::AddRow:           return "hint_AddRow";
            case EventHints::DeleteRow:        return "hint_DeleteRow";
            case EventHints::InsertRow :       return "hint_InsertRow";
            case EventHints::ChangeTable:      return "hint_ChangeTable";
            case EventHints::BeforeRowDelete:  return "hint_BeforeRowDelete";
            case EventHints::DeleteTable :     return "hint_DeleteTable";
            case EventHints::ChangeColor:      return "hint_ChangeColor";
            case EventHints::AddTable :        return "hint_AddTable";
        }
        assert(!"getHintName()");
        return "hint_xz";
    }
    explicit  QAstra(QObject *parent = nullptr);
    virtual   ~QAstra() = default;
    void      setRastr(const _sp_rastr& sp_rastr_in);
    _sp_rastr getRastr() const;
    IPlainRastrRetCode      Load( eLoadCode LoadCode, const std::string_view& FilePath, const std::string_view& TemplatePath );
    void      Save( const std::string_view& FilePath, const std::string_view& TemplatePath ) ;
    eASTCode  Rgm(const std::string_view& parameters = {});
    //IPlainRastrResultObject<T>  GetVal( const std::string_view& Table, const std::string_view& Col , const long index );
    //IRastrVariantPtr
    std::string GetVal( const std::string_view& Table, const std::string_view& Col , const long row );
signals:
    void onRastrHint( const _hint_data& );
    void onRastrLog ( const _log_data&  );
private:
    _sp_rastr sp_rastr_;
     std::unique_ptr<CUIFormsCollection> upCUIFormsCollection_;

};

#endif // QASTRA_H
