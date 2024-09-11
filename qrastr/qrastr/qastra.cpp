#include "qastra.h"
#include "common_qrastr.h"
using WrapperExceptionType = std::runtime_error;
#include "IPlainRastrWrappers.h"

class EventSink
        : public IRastrEventsSinkBase{
public:
    IPlainRastrRetCode OnEvent(const IRastrEventLog& Event) noexcept override
    {
        spdlog::info( "Log Status: {:3}  StageId: {:2} EventMsg: {:40} Table: {:10}  Column: {:5} Index: {:5} UIForm: {:10}"
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
    IPlainRastrRetCode OnEvent(const IRastrEventHint& Event) noexcept override
    {
        spdlog::info( "Hint: {:1} Table: {:10} Column: {:5} Index: {} "
            , static_cast<std::underlying_type<EventHints>::type>(Event.Hint())
            , Event.DBLocation().Table()
            , Event.DBLocation().Column()
            , Event.DBLocation().Index()
        );
        return IPlainRastrRetCode::Ok;
    }

    IPlainRastrRetCode OnEvent(const IRastrEventBase& Event) noexcept override
    {
        if(Event.Type() == EventTypes::Print)
            //std::cout << "Print: " << static_cast<const IRastrEventPrint&>(Event).Message() << std::endl;
            spdlog::info( "Print: {}", static_cast<const IRastrEventPrint&>(Event).Message() );
        return IPlainRastrRetCode::Ok;
    }

    IPlainRastrRetCode OnUICommand(const IRastrEventBase& Event, IPlainRastrVariant* Result) noexcept override
    {
        EventTypes et = Event.Type();
        std::string str;
        if(Event.Type() == EventTypes::Hint){
            str = fmt::format(" {} {} {} "
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
};

QAstra::QAstra(QObject *parent)
    : QObject{parent}{
}
void QAstra::setRastr(_sp_rastr sp_rastr_in){
    sp_rastr_     = sp_rastr_in;
    up_EventSink_ = std::make_unique<EventSink>();
    IRastrResultVerify(sp_rastr_->SubscribeEvents(up_EventSink_.get()));
}
void QAstra::LoadFile( eLoadCode LoadCode, const std::string_view& FilePath, const std::string_view& TemplatePath ){
    IRastrResultVerify loadresult{ sp_rastr_->Load(LoadCode,FilePath,TemplatePath) };
}
eASTCode QAstra::Rgm(const std::string_view& parameters){
    IRastrPayload  rgmresult{ sp_rastr_->Rgm("") };
    return rgmresult.Value();
}


