#include "protocolLogWidget.h"
#include <QVBoxLayout>
#include <cassert>
#include "sciLogViewer.h"

ProtocolLogWidget::ProtocolLogWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_viewer = new SciLogViewer(this);
    layout->addWidget(m_viewer);
}

void ProtocolLogWidget::clear()
{
    m_viewer->setContent("");
    n_stage_max_id_ = 0;
}

void ProtocolLogWidget::onQStringAppendProtocol(const QString& qstr)
{
    std::string str{""};
    for(int i = 0; i < n_stage_max_id_ ; i++){
        str += "\t";
    }
    str += qstr.toStdString();
    encodeHtml(str);
    str += "\n";
    m_viewer->appendTextCustom(str);
}

void ProtocolLogWidget::onRastrLog(const _log_data& log_data)
{
    std::string str;
    str.reserve(128);
    for (int i = 0; i < log_data.n_stage_id - 1; ++i)
        str += '\t';

    if (LogMessageTypes::OpenStage == log_data.lmt)
    {
        str += "<STAGE";
        str += std::to_string(log_data.n_stage_id);
        str += ">\t";
        str += log_data.str_msg;
        str += '\n';
        m_viewer->appendTextCustom(str);
        n_stage_max_id_ = log_data.n_stage_id;
        return;
    }

    if (LogMessageTypes::CloseStage == log_data.lmt)
    {
        str += "</STAGE";
        str += std::to_string(log_data.n_stage_id);
        str += ">\n";
        m_viewer->appendTextCustom(str);
        assert(n_stage_max_id_ == log_data.n_stage_id);
        if (n_stage_max_id_ == log_data.n_stage_id)
            --n_stage_max_id_;
        return;
    }

    // Обычное сообщение — выводим как есть (без XML-тегов)
    str += log_data.str_msg;
    str += '\n';
    m_viewer->appendTextCustom(str);
}

void ProtocolLogWidget::onRastrPrint(const std::string& str_msg)
{
    m_viewer->appendTextCustom(str_msg);
}

void ProtocolLogWidget::encodeHtml(std::string& data)
{
    std::string buf;
    buf.reserve(data.size() + 30);
    for (char c : data) {
        switch (c) {
            case '&':  buf.append("&amp;");  break;
            case '"':  buf.append("&quot;"); break;
            case '\'': buf.append("&apos;"); break;
            case '<':  buf.append("&lt;");   break;
            case '>':  buf.append("&gt;");   break;
            default:   buf.push_back(c);     break;
        }
    }
    data.swap(buf);
}

void ProtocolLogWidget::encodeHtml(std::string& data_out, const QString& qstr_in)
{
    data_out.reserve(static_cast<size_t>(qstr_in.size()) + 50);
    for (int pos = 0; pos < qstr_in.size(); ++pos) {
        const QLatin1Char ch{ qstr_in[pos].toLatin1() };
        if      (ch == QLatin1Char('&'))  data_out.append("&amp;");
        else if (ch == QLatin1Char('"'))  data_out.append("&quot;");
        else if (ch == QLatin1Char('\'')) data_out.append("&apos;");
        else if (ch == QLatin1Char('<'))  data_out.append("&lt;");
        else if (ch == QLatin1Char('>'))  data_out.append("&gt;");
        else                              data_out.push_back(qstr_in[pos].toLatin1());
    }
    data_out += '\n';
}
