#ifndef CONDFORMAT_H
#define CONDFORMAT_H

#include <QString>
#include <QColor>
#include <QFont>
#include <QModelIndex>
#include <regex>


template<typename C, typename E>
bool contains(const C& container, E element)
{
    return std::find(container.begin(), container.end(), element) != container.end();
}

template<typename T1, typename T2, typename E>
bool contains(const std::map<T1, T2>& container, E element)
{
    return container.find(element) != container.end();
}

template<typename C, typename E>
int find_index(const C& container, E element)
{
    int index = 0;
    for (auto it = container.begin(); it != container.end(); it++)
    {
        if ((*it) == element)
            return index;
        index++;
    }
    return -1;
}

class STRING_PARSE_BASE
{
public:
    enum class op { GREAT_OR_EQUAL, LESS_OR_EQUAL, EQUAL, GREATER, LESS };
    std::vector<std::pair<std::string, op>> operators = { {">=",op::GREAT_OR_EQUAL},
                                                         {"<=",op::LESS_OR_EQUAL} ,
                                                         {"=", op::EQUAL} ,
                                                         {">", op::GREATER} ,
                                                         {"<", op::LESS} ,
                                                         };
    std::vector<std::string> split(const std::string str,
                                   const std::string regex_str) {
        std::regex regexz(regex_str);
        return { std::sregex_token_iterator(str.begin(), str.end(), regexz, -1),
                std::sregex_token_iterator() };
    }
    void replaceAll(std::string& str, const std::string& from, const std::string& to) {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }
    inline std::string trim(std::string& str)
    {
        str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
        str.erase(str.find_last_not_of(' ') + 1);         //surfixing spaces
        return str;
    }
    //C++ 20
    /*inline bool to_double(std::string& str,double& dval)
    {
        //double result{};
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), dval);
        if (ec == std::errc())
            return true;
        else if (ec == std::errc::invalid_argument)
            return false;
        else if (ec == std::errc::result_out_of_range)
            return false;
        else return false;
    }*/
    inline bool to_double(std::string& str,double& dval)
    {
        char* pEnd = NULL;
        dval = strtod(str.c_str(), &pEnd);
        if (*pEnd) // error was detected
            return false;
        else return true;
    }
};

//Format Example:
// value comes first then expression. Expression contains single operation
// Use as row filter for highlighting cells.
// Use exapmle: STRING_BOOL("4<30").res()
class STRING_BOOL : private STRING_PARSE_BASE
{
public :
    STRING_BOOL(std::string _str)
    {
        str = _str;
    }
    std::vector<std::string> Check()
    {
        std::vector<std::string> v_check;
        for (auto& op : operators)
        {
            auto operands = split(str.data(), op.first);
            if (operands.size() == 2)
            {
                operands[0] = trim(operands[0]);
                operands[1] = trim(operands[1]);
                double d;
                if (!to_double(operands[0],d))
                    v_check.push_back(operands[0]);

                if (!to_double(operands[1], d))
                    v_check.push_back(operands[1]);

                break;
            }
        }
        return v_check;
    }

    bool res() {
        for (auto& op : operators)
        {
            auto operands = split(str.data(), op.first);
            if (operands.size() == 2)
            {
                double v1 = std::stod(operands[0].c_str());
                double v2 = 0.0;
                try
                {
                    v2 = std::stod(operands[1].c_str());
                }
                catch (const std::invalid_argument& ex_arg)
                {

                }
                switch (op.second)
                {
                case op::GREAT_OR_EQUAL:
                    return (v1 >= v2);
                    break;
                case op::LESS_OR_EQUAL:
                    return (v1 <= v2);
                    break;
                case op::EQUAL:
                    return (v1 == v2);
                    break;
                case op::GREATER:
                    return (v1 > v2);
                    break;
                case op::LESS:
                    return (v1 < v2);
                    break;
                default :
                    return false;
                }
            }
        }
        return false;
    }
    void replace(std::string from, std::string to)
    {
        replaceAll(str, from, to);
    }
private:

    std::string str;
};

class QAbstractTableModel;

// Conditional formatting for given format to table cells based on a specified condition.
class CondFormat
{
public:

    enum Alignment {
        AlignLeft = 0,
        AlignRight,
        AlignCenter,
        AlignJustify
    };

    // List of alignment texts. Order must be as Alignment definition above.
    static QStringList alignmentTexts() {
        return {QObject::tr("Left"), QObject::tr("Right"), QObject::tr("Center"), QObject::tr("Justify")};
    }

    // Get alignment from combined Qt alignment (note that this will lose any combination of our Alignment enum
    // with other values present in the flag (e.g. vertical alignment).
    static Alignment fromCombinedAlignment(Qt::Alignment align);

    CondFormat() {}
    CondFormat(const QString& filter,
               const QColor& foreground,
               const QColor& background,
               const QFont& font,
               const Alignment alignment = AlignLeft,
               const QString& encoding = QString());

    // Create a new CondFormat from values obtained from the model
    CondFormat(const QString& filter,
               const QAbstractTableModel* model,
               const QModelIndex index,
               const QString& encoding = QString());

    static std::string filterToSqlCondition(const QString& value, const QString& encoding = QString());

private:
    std::string m_sqlCondition;
    QString m_filter;
    QColor m_bgColor;
    QColor m_fgColor;
    QFont m_font;
    Alignment m_align;

public:
    std::string sqlCondition() const { return m_sqlCondition; }
    QString filter() const { return m_filter; }

    QColor backgroundColor() const { return m_bgColor; }
    QColor foregroundColor() const { return m_fgColor; }
    void setBackgroundColor(QColor color) { m_bgColor = color; }
    void setForegroundColor(QColor color) { m_fgColor = color; }

    bool isBold() const { return m_font.bold(); }
    bool isItalic() const { return m_font.italic(); }
    bool isUnderline() const { return m_font.underline(); }
    void setBold(bool value) { m_font.setBold(value); }
    void setItalic(bool value) { m_font.setItalic(value); }
    void setUnderline(bool value) { m_font.setUnderline(value); }

    QFont font() const { return m_font; }
    void setFontFamily(const QString &family) { m_font.setFamily(family); }
    void setFontPointSize(int pointSize) { m_font.setPointSize(pointSize); }

    Alignment alignment() const { return m_align; }
    void setAlignment(Alignment value) { m_align = value; }
    Qt::AlignmentFlag alignmentFlag() const;
};

#endif // CONDFORMAT_H
