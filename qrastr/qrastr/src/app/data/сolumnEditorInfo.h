#pragma once
#include <QStringList>
#include <QPixmap>

struct ColumnEditorInfo {
    enum class Type {
        None, Numeric, CheckBox, ComboBox,
        ComboBoxPicture, DateTime, Color, NameRef
    };

    Type        editorType = Type::None;
    QStringList comboItems;
    int         decimals   = 2;
    double      minVal     = -1e6;
    double      maxVal     =  1e6;

    struct NameRefData {
        struct ColDesc {
            QString header;
        };
        struct Row {
            size_t key;
            std::vector<std::string> values; // по одному на каждый ColDesc
        };
        std::vector<ColDesc> columns; // отображаемые колонки (не считая ключа)
        std::vector<Row>     rows;    // строки (ключи могут повторяться)

        // Совместимость: найти первое имя по ключу (для отображения в ячейке)
        QString nameByKey(int key) const {
            for (const auto& r : rows)
                if (static_cast<int>(r.key) == key && !r.values.empty())
                    return QString::fromStdString(r.values[0]);
            return {};
        }
    };
    NameRefData nameRefData;

    struct PicItem { QPixmap image; QString label; };
    QList<PicItem> picItems;
};
