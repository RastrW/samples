#ifndef QASTRA_H
#define QASTRA_H

#include <QObject>
#include "IPlainRastr.h"
#include "common_qrastr.h"

struct _data_hint{
    EventHints  hint;
    std::string str_table;
    std::string str_column;
    long        n_indx;
};

class QAstra
    : public QObject
    , public IRastrEventsSinkBase{
    Q_OBJECT
public:
    typedef std::shared_ptr<IPlainRastr> _sp_rastr;

    IPlainRastrRetCode OnEvent(const IRastrEventLog& Event) noexcept override {
        spdlog::info( "OnEvent.Log Status: {:3}  StageId: {:2} EventMsg: {:40} Table: {:10}  Column: {:5} Index: {:5} UIForm: {:10}"
            , static_cast<std::underlying_type<LogMessageTypes>::type>(Event.Status())
            , Event.StageId()
            , Event.Message()
            , Event.DBLocation().Table()
            , Event.DBLocation().Column()
            , Event.DBLocation().Index()
            , Event.UIForm()
        );
        return IPlainRastrRetCode::Ok;
    }
    IPlainRastrRetCode OnEvent(const IRastrEventHint& Event) noexcept override {
        _data_hint dh;
        dh.hint       = Event.Hint();
        dh.str_table  = Event.DBLocation().Table();
        dh.str_column = Event.DBLocation().Column();
        dh.n_indx     = Event.DBLocation().Index();
        emit onRastrHint( dh );
        spdlog::info( "OnEvent.Hint: [{:10}] [{:1}] Table: {:10} Column: {:5} Index: {}"
            , getHintName(Event.Hint())
            , static_cast<std::underlying_type<EventHints>::type>(Event.Hint())
            , Event.DBLocation().Table()
            , Event.DBLocation().Column()
            , Event.DBLocation().Index()
        );
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

    explicit QAstra(QObject *parent = nullptr);
    virtual ~QAstra() = default;
    void setRastr(_sp_rastr sp_rastr_in);
    void LoadFile( eLoadCode LoadCode, const std::string_view& FilePath, const std::string_view& TemplatePath );
    eASTCode Rgm(const std::string_view& parameters = {});
signals:
    void onRastrHint(const _data_hint& );
private:
    _sp_rastr sp_rastr_;
};

#endif // QASTRA_H
