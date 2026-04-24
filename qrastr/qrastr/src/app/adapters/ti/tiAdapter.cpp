#include "TIAdapter.h"
#include "QTI.h"

TIAdapter::TIAdapter(std::shared_ptr<QTI> qti)
    : m_qti(std::move(qti))
{
    assert(m_qti != nullptr);
}

long TIAdapter::RecalcDor()    { return m_qti->RecalcDor();    }
long TIAdapter::UpdateTables() { return m_qti->UpdateTables(); }
long TIAdapter::CalcPTI()      { return m_qti->CalcPTI();      }
long TIAdapter::DobavPTI()     { return m_qti->DobavPTI();     }
long TIAdapter::FiltrTI()      { return m_qti->FiltrTI();      }