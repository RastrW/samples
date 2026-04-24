#include "BarsMDPAdapter.h"
#include "QBarsMDP.h"

BarsMDPAdapter::BarsMDPAdapter(std::shared_ptr<QBarsMDP> qbarsmdp)
    : m_qbarsmdp(std::move(qbarsmdp))
{
    assert(m_qbarsmdp != nullptr);
}

void BarsMDPAdapter::Init(std::string_view sections) {
    m_qbarsmdp->Init(sections.data());
}
void BarsMDPAdapter::UpdateMDPFields()  { m_qbarsmdp->UpdateMDPFields();  }
void BarsMDPAdapter::UpdateAUTOFields() { m_qbarsmdp->UpdateAUTOFields(); }