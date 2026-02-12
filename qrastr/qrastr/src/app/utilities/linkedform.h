#ifndef LINKEDFORM_H
#define LINKEDFORM_H
#pragma once

#include <QObject>
#include "utils.h"


class RtabWidget;

class LinkedForm
{
    //Q_OBJECT
public:
    //LinkedForm();
    std::string linkedform;
    std::string linkedname;
    std::string selection;
    std::string bind;
    long row;
    std::vector<int> vbindvals;
    RtabWidget* pbaseform;
    std::vector<QMetaObject::Connection> vconn;

    std::string get_selection_result()
    {
        selection_result = selection;
        replaceAll(selection_result,"%d",vbindvals);
        return selection_result;
    }
//public slots:
     void FillBindVals();
private:
    std::string selection_result;

};

#endif // LINKEDFORM_H
