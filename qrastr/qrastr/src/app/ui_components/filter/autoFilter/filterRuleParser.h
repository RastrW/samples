#pragma once
#include "filterRule.h"

/// @brief Парсит строку вида ">400", "==uhom", "<=110.5", "!= qmin".
/// Пустая строка → FilterRule с op==None (нет условия).
struct FilterRuleParser {
    static FilterRule parse(const QString& text) {
        FilterRule rule;
        if (text.trimmed().isEmpty()) return rule;

        struct OpEntry { QString tok; FilterRule::Op op; };
        static const OpEntry ops[] = {
            { ">=", FilterRule::Op::Ge  },
            { "<=", FilterRule::Op::Le  },
            { "!=", FilterRule::Op::Neq },
            { "==", FilterRule::Op::Eq  },
            { ">",  FilterRule::Op::Gt  },
            { "<",  FilterRule::Op::Lt  },
            { "=",  FilterRule::Op::Eq  },   // синоним ==
        };

        QString t = text.trimmed();
        for (const auto& e : ops) {
            if (t.startsWith(e.tok)) {
                rule.op    = e.op;
                rule.value = t.mid(e.tok.length()).trimmed();
                bool ok = false;
                double d = rule.value.toDouble(&ok);
                if (ok) { rule.isNum = true; rule.numValue = d; }
                return rule;
            }
        }
        // Нет оператора → трактуем как ==
        rule.op    = FilterRule::Op::Eq;
        rule.value = t;
        bool ok = false;
        double d = t.toDouble(&ok);
        if (ok) { rule.isNum = true; rule.numValue = d; }
        return rule;
    }
};