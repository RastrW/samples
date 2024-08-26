#include <iostream>
#include <filesystem>
#include <QLibrary>
#include "qastra.h"

using WrapperExceptionType = std::runtime_error;
#include "IPlainRastrWrappers.h"

qAstra::qAstra(QObject *parent)
    : QObject{parent}{
}
class EventSink : public IRastrEventsSinkBase
{
public:
    void OnEvent(const IRastrEventLog& Event) override
    {
        std::cout << "Log Status: " << static_cast<std::underlying_type<LogMessageTypes>::type>(Event.Status()) << " ";
        std::cout << "StageId: " << Event.StageId() << " ";
        std::cout << Event.Message() << " ";
        std::cout << "Table: " << Event.DBLocation().Table() << " ";
        std::cout << "Column: " << Event.DBLocation().Column() << " ";
        std::cout << "Index: " << Event.DBLocation().Index() << " ";
        std::cout << "UIForm: " << Event.UIForm() << " ";
        std::cout << std::endl;
    }
    void OnEvent(const IRastrEventHint& Event) override
    {
        std::cout << "Hint: " << static_cast<std::underlying_type<EventHints>::type>(Event.Hint()) << " ";
        std::cout << "Table: " << Event.DBLocation().Table() << " ";
        std::cout << "Column: " << Event.DBLocation().Column() << " ";
        std::cout << "Index: " << Event.DBLocation().Index() << " ";
        std::cout << std::endl;
    }

    void OnEvent(const IRastrEventBase& Event) override
    {
        if(Event.Type() == EventTypes::Print)
            std::cout << "Print: " << static_cast<const IRastrEventPrint&>(Event).Message() << std::endl;
    }

    bool OnUICommand(const IRastrEventBase& Event, IPlainRastrVariant* Result) override
    {
        Result->String("Done");
        return true;
    }
};

int qAstra::tst_iplainrastr() const {
#ifdef _DEBUG
    //std::filesystem::current_path("/source/repos/rastr/RastrWin/Debug64/");
    std::filesystem::current_path(R"(C:\projects\rastr\RastrWin\Debug64)");
#else
    std::filesystem::current_path("/source/repos/rastr/RastrWin/Release64/");
#endif
    SetConsoleOutputCP(CP_UTF8);
    try{
        QLibrary qlRastr{"astra"};
        if(qlRastr.load()){
            const QFunctionPointer pfn{ qlRastr.resolve("PlainRastrFactory") };
            if(pfn!=nullptr){
                _prf fnFactory = reinterpret_cast<_prf>(pfn);
                Destroyable rastr{ (fnFactory)() };
                EventSink sink;
                IRastrResultVerify(rastr->SubscribeEvents(&sink));
                //IRastrResultVerify loadresult{ rastr->Load(eLoadCode::RG_REPL, "/Documents/RastrWin3/test-rastr/cx195.rg2", "/Documents/RastrWin3/SHABLON/режим.rg2") };
                IRastrResultVerify loadresult{
                    rastr->Load(
                        eLoadCode::RG_REPL,
                        stringutils::acp_encode(R"(C:\Users\ustas\Documents\RastrWin3\test-rastr\cx195.rg2)"),
                        //stringutils::acp_encode(R"(C:\Users\ustas\Documents\RastrWin3\SHABLON\режим.rg2)")
                        std::filesystem::u8path(R"(C:\Users\ustas\Documents\RastrWin3\SHABLON\режим.rg2)").generic_string()

                    )
                };
                IRastrPayload  rgmresult{ rastr->Rgm("") };
                std::cout << static_cast<std::underlying_type<eASTCode>::type>(rgmresult.Value()) << std::endl;
                IRastrTablesPtr tablesx{ rastr->Tables() };
                IRastrPayload tablecount{ tablesx->Count() };
                std::cout << tablecount.Value() << std::endl;
                IRastrTablePtr nodes{ tablesx->Item("node") };

                IRastrPayload tablesize{ nodes->Size() };
                std::cout << tablesize.Value() << std::endl;
                IRastrPayload tablename{ nodes->Name() };
                std::cout << tablename.Value() << std::endl;
                IRastrObjectPtr<IPlainRastrColumns> nodecolumns{ nodes->Columns() };
                IRastrColumnPtr ny{ nodecolumns->Item("ny") };
                IRastrColumnPtr name{ nodecolumns->Item("name") };
                IRastrColumnPtr v{ nodecolumns->Item("vras") };
                IRastrColumnPtr delta{ nodecolumns->Item("delta") };

                for (long index{ 0 }; index < tablesize.Value(); index++)
                {
                    IRastrVariantPtr Varny{ ny->Value(index) };
                    IRastrVariantPtr Varname{ name->Value(index) };
                    IRastrVariantPtr Varv{ v->Value(index) };
                    IRastrVariantPtr Vardelta{ delta->Value(index) };

                    IRastrPayload nyvalue{ Varny->Long() };
                    IRastrPayload namevalue{ Varname->String() };
                    IRastrPayload vvalue{ Varv->Double() };
                    IRastrPayload deltavalue{ Vardelta->Double() };

                    std::cout << nyvalue.Value() << " "
                        << stringutils::acp_decode(namevalue.Value()) << " "
                        << vvalue.Value() << " "
                        << deltavalue.Value() << " "
                        << std::endl;
                }
                IRastrResultVerify selectionresult{ nodes->SetSelection("uhom>330") };
                IRastrObjectPtr<IPlainRastrDataset> dataset{ nodes->Dataset("ny, name, uhom, 62,")};
                const long datasize{ IRastrPayload(dataset->Count()).Value() };

                for (long colindex = 0; colindex < datasize; colindex++)
                {
                    IRastrObjectPtr<IPlainRastrDatasetColumn> datacolumn{ dataset->Item(colindex) };
                    std::cout << IRastrPayload(datacolumn->Name()).Value() << " "
                              << static_cast<std::underlying_type<ePropType>::type>(IRastrPayload(datacolumn->Type()).Value())
                              << std::endl;
                    const long columnsize{ IRastrPayload(datacolumn->Count()).Value() };
                    for (long rowindex = 0; rowindex < columnsize; rowindex++)
                    {
                        IRastrObjectPtr<IPlainRastrDatasetValue> indexedvalue{ datacolumn->Item(rowindex) };
                        IRastrVariantPtr value{ indexedvalue->Value() };

                        std::cout << IRastrPayload(indexedvalue->Index()).Value() << " "
                            << stringutils::acp_decode(IRastrPayload(value->String()).Value())
                            << std::endl;
                    }
                }
                IRastrObjectPtr dcol{ dataset->Item("ny") };
                // на выходе из скопа врапперы делают Destroy в хипе астры
                IRastrResultVerify(rastr->UnsubscribeEvents(&sink));
                //FreeLibrary(hRastr);
            }else{
                std::cout << "Failed get pointer on function 'PlainRastrFactory()' : " << GetLastError() << std::endl;
            }
        }else{
            std::cout << "Failed to load ASTRA.dll : " << GetLastError() << std::endl;
        }
    }catch(const std::exception& ex){
        std::cout << stringutils::acp_decode(ex.what());
    }
    return 1;
}