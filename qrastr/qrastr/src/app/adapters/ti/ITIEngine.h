#pragma once

class ITIEngine {
public:
    virtual ~ITIEngine() = default;

    virtual long RecalcDor()    = 0;
    virtual long UpdateTables() = 0;
    virtual long CalcPTI()      = 0;
    virtual long DobavPTI()     = 0;
    virtual long FiltrTI()      = 0;
};