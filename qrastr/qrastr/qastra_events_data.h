#ifndef QASTRA_EVENTS_DATA_H
#define QASTRA_EVENTS_DATA_H
#pragma once

#include "IPlainRastr.h"
///////////////////////////////////////////////////////////////////////
//RastrEngine.cs
///////////////////////////////////////////////////////////////////////
///
// Событие, возникающее при изменении данных в таблице
//public event EventHandler<ChangeDataEventArgs> OnChangeData; // IRastrEventHint
//public class ChangeDataEventArgs : EventArgs
//    public ChangeDataHint Hint
//    public string         Table
//    public string         Column
//    public int            RowIndex
struct _hint_data{
    EventHints  hint;
    std::string str_table;
    std::string str_column;
    long        n_indx;
};

// Событие журналирования
//public event EventHandler<LogEventArgs> OnLog; //IRastrEventLog
//public class LogEventArgs : EventArgs
//  public LogMessageCode MessageCode
//  public int Level
//  public int StageId
//  public string TableName
//  public int TableIndex
//  public string Description
//  public string FormName
struct _log_data{
    LogMessageTypes lmt;
    long            n_stage_id;
    std::string     str_msg;
    std::string     str_table;
    std::string     str_col;
    IndexT          n_indx;
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

#endif // QASTRA_EVENTS_DATA_H
