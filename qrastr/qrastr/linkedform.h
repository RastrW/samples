#ifndef LINKEDFORM_H
#define LINKEDFORM_H

#include <QObject>
//#include "rtabwidget.h"
#include "utils.h"


class RtabWidget;
//class LinkedForm: public QObject
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

   /* {
        for (const auto key : split( bind ,','))
        {
           // int col = pbaseform->prm->getRdata()->mCols_.at(key);
           // long val = std::visit(ToLong(),pbaseform->prm->getRdata()->pnparray_->Get(row,col));
           // vbindvals.push_back(val);
        }
    }*/
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
