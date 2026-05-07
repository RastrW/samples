#pragma once
#include <string>
#include <vector>
#include <optional>

/// Текстовая метка тока на ветви (slb / sle).
struct BranchLabel {
    int         slot;       // 0 = slb (начало), 1 = sle (конец)
    std::string anchor;     // "start" или "end" — выравнивание текста
    std::string transform;  // "translate(x,y)" или "translate(x,y) rotate(a)"
    std::string text;       // "120->", "<-85"
};

/// Символ трансформатора, размещаемый посередине ветви (только при tip==1).
struct BranchFigure {
    std::string symbol_id;          // "2tr", "atr", "3tr", ...
    std::string transform;          // "translate(cx,cy) rotate(angle)"
    std::string voltage_class_end;  // цвет второго кружка (сторона iq)
};

/// Ветвь схемы (линия, трансформатор и т.п.).
struct BranchData {
    int id;
    int vbeg, vend;                          // номера узлов ip / iq
    std::string voltage_class;               // "u110", "u-off", "u-overload", ...
    std::string voltage_class_end;           // для tip==1: класс напряжения стороны iq
    std::vector<std::vector<float>> segments;// полилинии: обычно 1, при tip==1 — 2
    std::optional<BranchFigure> figure;      // символ трансформатора (если есть)
    std::vector<BranchLabel> labels;         // метки токов
    int state;                               // 0 = включена, 1 = выключена
};

/// Фигура оборудования на шине (генератор, реактор, шунт/БСК).
struct NodeFigure {
    int nf;     // slot index: 0=генератор, 1=реактор, 2=шунт (bsh>=0), 3=БСК (bsh<0)
    int fnpr;   // позиция вдоль шины (в единицах сетки)
    int size;   // размер/направление (>0 = выше шины, <0 = ниже шины)
};

/// Узел схемы (шина или трансформатор).
struct NodeData {
    int         id;
    float       x, y;          // SVG-координаты: (k_x - wx)*SCALE, (k_y - wy)*SCALE
    int         uhom;           // класс напряжения, кВ
    int         nod_type;       // -1 = шина, 0-5 = тип трансформатора
    int         npri;           // длина/направление шины (>0 горизонталь, <0 вертикаль)
    int         sta;            // состояние (0 = включён, иначе = выключен)
    std::string name;           // название узла
    float       name_dx;        // смещение текстовой метки по X
    float       name_dy;        // смещение текстовой метки по Y
    float       name_angle;     // угол поворота текстовой метки (градусы)
    std::vector<NodeFigure> figures;
};

/// Стиль отображения уровня напряжения
struct VoltageStyle {
    std::string css_class;   // "u110", "u-off", "u-unknown", "u-overload"
    std::string stroke;      // "#4699CC"
    float       stroke_width;
};

/// Видовой прямоугольник схемы в SVG-координатах.
struct ViewBox {
    float x, y, width, height;
};

/// Слой схемы (rastr не хранит слои в стандартной таблице; возвращается один дефолтный).
struct LayerInfo {
    int         id;
    std::string name;
};

/// Полный layout схемы
struct GraphLayout {
    ViewBox                    view_box;
    std::vector<VoltageStyle>  voltage_styles;
    std::vector<BranchData>    branches;
    std::vector<NodeData>      nodes;
    std::vector<LayerInfo>     layers;
    std::optional<std::string> loaded_bbox; // "x_min,y_min,x_max,y_max" SVG coords; nullopt = полная схема
};