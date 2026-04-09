#pragma once
#include <string>
#include <vector>
#include <optional>

struct ViewBox { float x, y, width, height; };

struct VoltageStyle {
    std::string css_class;   // "u110", "u-off", ...
    std::string stroke;      // "#4699CC"
    float stroke_width;
};

struct NodeFigure {
    int nf;     // 0=генератор, 1=реактор, 2=шунт
    int fnpr;   // позиция вдоль шины
    int size;   // размер/направление
};

struct NodeData {
    int id;
    float x, y;
    int uhom;
    int nod_type;   // -1=шина, 0-5=тип трансформатора
    int npri;		// длина/направление шины (>0 по горизонтали, <0 по вертикали)
    int sta;		// state (0 = on, non-zero = off)
    std::string name;
    float name_dx, name_dy; // смещение текстовой метки x, y
    std::vector<NodeFigure> figures;
};

struct BranchLabel {
    int slot;
    std::string anchor;
    std::string transform;   // "translate(x,y) rotate(a)"
    std::string text;        // "120->", "<-85"
};

struct BranchFigure {
    std::string symbol_id;   // "2tr", "atr", ...
    std::string transform;
    std::string voltage_class_end; // для tip=1: класс напряжения стороны iq
};

struct BranchData {
    int id;
    int vbeg, vend;
    std::string voltage_class;
    std::string voltage_class_end;
    std::vector<std::vector<float>> segments;  // полилинии
    std::optional<BranchFigure> figure;
    std::vector<BranchLabel> labels;
    int state;
};

struct GraphLayout {
    ViewBox view_box;
    std::vector<VoltageStyle> voltage_styles;
    std::vector<NodeData> nodes;
    std::vector<BranchData> branches;
    std::optional<std::string> loaded_bbox; //"x_min,y_min,x_max,y_max" SVG coords, None = full schema
};