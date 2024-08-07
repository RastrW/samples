#ifndef SCIHLP_H
#define SCIHLP_H

#include "ScintillaEdit.h"

class SciHlp : public ScintillaEdit {
    Q_OBJECT
public:
    SciHlp(QWidget *parent = nullptr);
    virtual ~SciHlp() = default;

};//class SciHlp{

#endif // SCIHLP_H
