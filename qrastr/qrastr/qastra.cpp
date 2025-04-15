#include "qastra.h"
#include "common_qrastr.h"
#include "UIForms.h"
#include "utils.h"

using WrapperExceptionType = std::runtime_error;
#include "IPlainRastrWrappers.h"

class EventSink
        : public IRastrEventsSinkBase{
public:

    EventSink(const QAstra* pqa){
    }

    virtual ~EventSink() = default;

    IPlainRastrRetCode OnEvent(const IRastrEventLog& Event) noexcept override {
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

    IPlainRastrRetCode OnEvent(const IRastrEventHint& Event) noexcept override {
        spdlog::info( "Hint: [{:10}] [{:1}] Table: {:10} Column: {:5} Index: {}"
            , getHintName(Event.Hint())
            , static_cast<std::underlying_type<EventHints>::type>(Event.Hint())
            , Event.DBLocation().Table()
            , Event.DBLocation().Column()
            , Event.DBLocation().Index()
        );
        return IPlainRastrRetCode::Ok;
    }

    IPlainRastrRetCode OnEvent(const IRastrEventBase& Event) noexcept override {
        if(Event.Type() == EventTypes::Print)
            //std::cout << "Print: " << static_cast<const IRastrEventPrint&>(Event).Message() << std::endl;
            spdlog::info( "Print: {}", static_cast<const IRastrEventPrint&>(Event).Message() );
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
};

QAstra::QAstra(QObject *parent)
    : QObject{parent}{
}

void QAstra::setRastr(const _sp_rastr& sp_rastr_in){
    sp_rastr_     = sp_rastr_in;
    IRastrResultVerify{sp_rastr_->SetOutEnumAsInt(true)};
    IRastrResultVerify(sp_rastr_->SubscribeEvents(this));
}

QAstra::_sp_rastr QAstra::getRastr() const {
    return sp_rastr_;
}

//void QAstra::Load( eLoadCode LoadCode, const std::string_view& FilePath, const std::string_view& TemplatePath ){
IPlainRastrRetCode QAstra::Load( eLoadCode LoadCode, const std::string_view& FilePath, const std::string_view& TemplatePath ){
    IRastrResultVerify loadresult{ sp_rastr_->Load( LoadCode, FilePath, TemplatePath ) };
    return loadresult->Code();
}

void QAstra::Save( const std::string_view& FilePath, const std::string_view& TemplatePath ){
    IRastrResultVerify saveresult{ sp_rastr_->Save( FilePath, TemplatePath ) };
}

eASTCode QAstra::Kdd(const std::string_view& parameters){
    IRastrPayload  kddresult{ sp_rastr_->Kdd(parameters) };
    return kddresult.Value();
}

eASTCode QAstra::Rgm(const std::string_view& parameters){
    IRastrPayload  rgmresult{ sp_rastr_->Rgm(parameters) };
    return rgmresult.Value();
}

eASTCode QAstra::Opf(const std::string_view& parameters){
    IRastrPayload  opfresult{ sp_rastr_->OPF(parameters) };
    return opfresult.Value();
}

eASTCode QAstra::Smzu_tst(long parameter){

    long par = static_cast<long>(std::get<double>(GetVal("com_optim","koef_kt",0)));
    IRastrPayload  smzu_tst_result{ sp_rastr_->Smzu_tst(par) };
    return smzu_tst_result.Value();
}

eASTCode QAstra::Kz(const std::string_view& parameters, eNonsym Nonsym, long p1, long p2, long p3, double LengthFromP1InProc, double rd, double z_re, double z_im){
    IRastrPayload  kz_result{ sp_rastr_->Kz( parameters, Nonsym, p1, p2, p3, LengthFromP1InProc, rd, z_re, z_im) };
    return kz_result.Value();
}

std::string QAstra::GetStringVal( const std::string_view& Table, const std::string_view& Col , const long row ){
    IRastrTablesPtr tablesx{ sp_rastr_->Tables() };
    IRastrPayload tablecount{ tablesx->Count() };
    IRastrTablePtr table{ tablesx->Item(Table) };
    IRastrObjectPtr<IPlainRastrColumns> columns{ table->Columns() };
    IRastrColumnPtr col {columns->Item(Col)};
    IRastrVariantPtr v_ptr{ col->Value(row) };
    IRastrPayload payload{ v_ptr->String() };
    std::string str_val = stringutils::acp_decode(payload.Value());
    return str_val;
}

FieldVariantData QAstra::GetVal( const std::string_view& Table, const std::string_view& Col , const long row ){
    IRastrTablesPtr tablesx{ sp_rastr_->Tables() };
    IRastrPayload tablecount{ tablesx->Count() };
    IRastrTablePtr table{ tablesx->Item(Table) };
    IRastrObjectPtr<IPlainRastrColumns> columns{ table->Columns() };
    IRastrColumnPtr col {columns->Item(Col)};
    IRastrVariantPtr v_ptr{ col->Value(row) };

    if (IRastrPayload(v_ptr->Type()).Value() == eFieldVariantType::Monostate )
        return std::monostate();

    ePropType col_type = IRastrPayload(col->Type()).Value();

    switch (col_type)
    {
    case ePropType::Bool:
        return IRastrPayload(v_ptr->Bool()).Value();
    case ePropType::Double:
        return IRastrPayload(v_ptr->Double()).Value();
        break;
    case ePropType::Int:
    case ePropType::Enpic:
    case ePropType::Color:
    //case ePropType::Enum:
    //case ePropType::Superenum:
        return IRastrPayload(v_ptr->Long()).Value();
        break;
    case ePropType::String:
        return IRastrPayload(v_ptr->String()).Value();
        break;
    case ePropType::Enum:
    case ePropType::Superenum:
       // return IRastrPayload(v_ptr->String()).Value();
        return IRastrPayload(v_ptr->Long()).Value();
        break;

    default:
        break;
    }
    return std::monostate();
}






