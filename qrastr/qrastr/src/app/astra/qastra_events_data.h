#pragma once

#include <astra/IPlainRastr.h>

///////////////////////////////////////////////////////////////////////
//RastrEngine.cs
///////////////////////////////////////////////////////////////////////

// Событие, возникающее при изменении данных в таблице
struct _hint_data{
    EventHints  hint;           // Тип события (изменение, вставка, удаление)
    std::string str_table;      // Имя таблицы
    std::string str_column;     // Имя колонки
    long        n_indx;         // Индекс строки
};

// Событие журналирования
struct _log_data{
    LogMessageTypes lmt;            // Тип сообщения (Info, Warning, Error)
    long            n_stage_id;     // ID стадии расчёта
    std::string     str_msg;        // Текст сообщения
    std::string     str_table;
    std::string     str_col;
    IndexT          n_indx;         // Индекс (если применимо)
    std::string     str_uiform;
};

// Событие протоколирования
//public event EventHandler<ProtEventArgs> OnProtocol; //IRastrEventBase
//public class ProtEventArgs : EventArgs
//  public string Message { get; private set; }

// Событие CommandMain
//public event EventHandler<CommandMainEventArgs> OnCommandMain; // OnUICommand(const IRastrEventBase& ... )
//public class CommandMainEventArgs:EventArgs
//  public CommandMainId CommandId
//  public string        P1
//  public string        P2
//  public int           Pp
//  public object        Result

