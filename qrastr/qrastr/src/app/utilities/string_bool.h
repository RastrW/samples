#ifndef STRING_BOOL_H
#define STRING_BOOL_H
#pragma once

#include <utils.h>

class STRING_PARSE_BASE
{
public:
    enum class op { GREAT_OR_EQUAL, LESS_OR_EQUAL, NOT_EQUAL,EQUAL, GREATER, LESS, SUM, MINUS,MULTIPLY,DIVIDE , AND, OR	};
    std::vector<std::pair<std::string, op>> operators = {
        {"&", op::AND} ,
        {"|", op::OR} ,
        {">=",op::GREAT_OR_EQUAL},
        {"<=",op::LESS_OR_EQUAL} ,
        {"!=",op::NOT_EQUAL} ,
        {"<>",op::NOT_EQUAL} ,
        {"=", op::EQUAL} ,
        {">", op::GREATER} ,
        {"<", op::LESS} ,
        {"+", op::SUM} ,
        {"-", op::MINUS} ,
        {"*", op::MULTIPLY} ,
        {"/", op::DIVIDE} ,

        };

    inline bool to_double(std::string& str, double& dval)
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

        std::string tmp_str = str;
        for (auto& oper : operators)
            replaceAll(tmp_str,oper.first, " ");

        auto operands = split2(tmp_str," ");
        for (auto& operand : operands)
        {
            double d;
            trim(operand);
            if (!to_double(operand, d))
                v_check.push_back(operand);
        }
        return v_check;
    }

    double res() {
        return res(str);
    }
    double res(std::string _str) {
        bool bfindop = false;
        std::cout << "expr::" << _str << std::endl;
        for (auto& op : operators)
        {
            auto operands = split2(_str, op.first);
            if (operands.size() == 2)
            {
                bfindop = true;
                double v1 = res(trim(operands[0]));
                double v2 = res(trim(operands[1]));

                double op_res = perform_operation(v1, v2, op.second);
                std::cout << op.first << ":\t"<<trim(operands[0])<<op.first << trim(operands[1])<<" -> "<<v1<< op.first<<v2<<"="<< op_res << std::endl;
                return op_res;
            }
        }
        if (!bfindop)
        {
            std::cout << "val: " << _str << std::endl;
            return _str.empty()?0.0: std::stod(trim(_str).c_str());
        }
        return 0.0;

    }
    double perform_operation(double v1, double v2, op operation)
    {
        switch (operation)
        {
        case op::AND:
            return ((v1 != 0) && (v2 != 0));
            break;
        case op::OR:
            return ((v1 != 0) || (v2 != 0));
            break;
        case op::GREAT_OR_EQUAL:
            return (v1 >= v2);
            break;
        case op::LESS_OR_EQUAL:
            return (v1 <= v2);
            break;
        case op::NOT_EQUAL:
            return (v1 != v2);
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
        case op::MULTIPLY:
            return (v1 * v2);
            break;
        case op::DIVIDE:
            return (v1 / v2);
            break;
        case op::SUM:
            return (v1 + v2);
            break;
        case op::MINUS:
            return (v1 - v2);
            break;
        default:
            return false;
        }
    }
    void replace(std::string from, std::string to)
    {
        replaceAll(str, from, to);
    }
private:

    std::string str;
};

#endif // STRING_BOOL_H
