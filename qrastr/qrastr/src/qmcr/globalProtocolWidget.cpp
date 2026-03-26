#include "globalProtocolWidget.h"
#include <QVBoxLayout>
#include <cassert>
#include "sciLogViewer.h"

GlobalProtocolWidget::GlobalProtocolWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_viewer = new SciLogViewer(this);
    layout->addWidget(m_viewer);
}

void GlobalProtocolWidget::clear()
{
    m_viewer->setContent({});
    m_stageMaxId = 0;
}

std::string GlobalProtocolWidget::makeIndent(int n)
{
    return std::string(static_cast<size_t>(std::max(0, n)), '\t');
}

void GlobalProtocolWidget::onAppendText(const QString& text)
{
    std::string line = makeIndent(static_cast<int>(m_stageMaxId));
    line += text.toStdString();   // UTF-8 через toStdString()
    if (line.empty() || line.back() != '\n')
        line += '\n';
    m_viewer->appendTextCustom(line);
}

void GlobalProtocolWidget::onRastrLog(const _log_data& log_data)
{
    std::string str;
    str.reserve(128);
    str += makeIndent(log_data.n_stage_id - 1);

    if (log_data.lmt == LogMessageTypes::OpenStage) {
        str += "<STAGE";
        str += std::to_string(log_data.n_stage_id);
        str += ">\t";
        str += log_data.str_msg;
        str += '\n';
        m_viewer->appendTextCustom(str);
        m_stageMaxId = log_data.n_stage_id;
        return;
    }

    if (log_data.lmt == LogMessageTypes::CloseStage) {
        str += "</STAGE";
        str += std::to_string(log_data.n_stage_id);
        str += ">\n";
        m_viewer->appendTextCustom(str);
        if (m_stageMaxId == log_data.n_stage_id)
            --m_stageMaxId;
        return;
    }
}

void GlobalProtocolWidget::onRastrPrint(const std::string& msg)
{
    m_viewer->appendTextCustom(msg);
}
