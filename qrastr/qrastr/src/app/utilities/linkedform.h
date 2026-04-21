#ifndef LINKEDFORM_H
#define LINKEDFORM_H
#pragma once

#include <QObject>
#include "utils.h"


class RtabController;

class LinkedForm
{
public:
    std::string linkedform;
    std::string linkedname;
    std::string selection;
    std::string bind;
    long row;
    std::vector<int> vbindvals;
    RtabController* pbaseform;
    std::vector<QMetaObject::Connection> vconn;

    std::string get_selection_result()
    {
        selection_result = selection;
        replaceAll(selection_result,"%d",vbindvals);
        return selection_result;
    }
     void fillBindVals();
private:
    std::string selection_result;

};

class LinkedMacro
{
public:
    std::string col;
    std::string macrofile;
    std::string macrodesc;
    std::string addstr;
    std::string templatetags;
    long row;
    std::vector<int> vbindvals;
    RtabController* pbaseform;
    std::vector<QMetaObject::Connection> vconn;

    void FillBindVals();
private:
    std::string selection_result;

};

#endif // LINKEDFORM_H
