#include "customFilterCondition.h"


/* CustomFilterCondition */
CustomFilterCondition::CustomFilterCondition(GridFilter* filter)
    : GridFilterCondition(filter) { }

bool CustomFilterCondition::isTrue(const QModelIndex& index)
{
    return m_modelRows.contains(index.row());
}

GridFilterCondition* CustomFilterCondition::clone() const
{
    CustomFilterCondition* retval = new CustomFilterCondition(m_filter);
    retval->m_modelRows = m_modelRows;
    return retval;
}

QString CustomFilterCondition::createPresentation() const
{
    QString retval = QObject::tr("custom filter: row in (");
    QList<int> list = m_modelRows.values();
    std::sort(list.begin(), list.end());
    for (QList<int>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        if (it != list.constBegin())
            retval += QStringLiteral(", ");
        retval += QString::number(*it);
    }
    retval += QStringLiteral(")");
    return retval;
}

int CustomFilterCondition::conditionCount() const
{
    return m_modelRows.size();
}

QString CustomFilterCondition::xml_name = QStringLiteral("CustomCondition");

#ifndef QTN_NOUSE_XML_MODULE
bool CustomFilterCondition::saveToXML(IXmlStreamWriter* xmlwriter)
{
    xmlwriter->writeStartElement(QStringLiteral("Qtitan:%1").arg(xml_name));
    for (QSet<int>::const_iterator it = m_modelRows.constBegin(); it != m_modelRows.constEnd(); ++it)
    {
        xmlwriter->writeStartElement(QStringLiteral("Qtitan:Row"));
        xmlwriter->writeAttribute(QStringLiteral("index"), QStringLiteral("%1").arg(*it));
        xmlwriter->writeEndElement();
    }
    xmlwriter->writeEndElement();
    return true;
}

bool CustomFilterCondition::loadFromXML(IXmlStreamReader* xmlreader)
{
    m_modelRows.clear();
    if (xmlreader->tokenType() != QXmlStreamReader::StartElement ||
        xmlreader->qualifiedName() != QStringLiteral("Qtitan:%1").arg(xml_name))
        return false;
    while (xmlreader->readNextStartElement())
    {
        if (xmlreader->qualifiedName() != QStringLiteral("Qtitan:Row"))
            return false;
        QXmlStreamAttributes attrs = xmlreader->attributes();
        if (attrs.hasAttribute(QStringLiteral("index")))
            m_modelRows.insert(attrs.value(QStringLiteral("index")).toInt());
        xmlreader->skipCurrentElement();
    }
    return true;
}
#endif

void CustomFilterCondition::addRow(int modelRowIndex)
{
    m_modelRows.insert(modelRowIndex);
}

void CustomFilterCondition::removeRow(int modelRowIndex)
{
    m_modelRows.remove(modelRowIndex);
}
