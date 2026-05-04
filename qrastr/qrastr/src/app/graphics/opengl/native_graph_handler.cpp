#include "native_graph_handler.h"

#include <spdlog/spdlog.h>
#include <cmath>
#include <algorithm>
#include <climits>
#include <cfloat>
#include <map>
#include <set>

// ─────────────────────────────────────────────────────────────────────────────
//  Цвета уровней напряжения — ГОСТ Р 59919-2021, Таблица 5.1
//  RGB → HEX: (R,G,B) → #RRGGBB
// ─────────────────────────────────────────────────────────────────────────────
// Особые состояния — требуют проверки доп. полей rastr:
//   Заземлено:  #FF9900  (255:153:0)  — sta==2 или признак заземления
//   Перегрузка: #FF0000  (255:0:0)    — по результатам расчёта (i_zag > 100)
//   Неизвестно: #8C8C8C  (140:140:140)— нет данных расчёта
static const std::map<int,std::string> kVoltageColors = {
    {1150, "#CD8AFF"},  // 205:138:255
    { 750, "#4141F0"},  // 65:65:240   (800 кВ ППТ)
    { 500, "#B80000"},  // 184:0:0
    { 400, "#87FDC2"},  // 135:253:194 (ЛЭП, цепи ППТ)
    { 330, "#00CC00"},  // 0:204:0
    { 220, "#CCCC00"},  // 204:204:0
    { 150, "#AA9600"},  // 170:150:0
    { 110, "#4699CC"},  // 70:153:204
    {  60, "#C25A5A"},  // 194:90:90   (27–60 кВ)
    {  35, "#C25A5A"},  // 194:90:90
    {  27, "#C25A5A"},  // 194:90:90
    {  20, "#A464A4"},  // 164:100:164 (6–24 кВ)
    {  15, "#A464A4"},
    {  10, "#A464A4"},
    {   6, "#A464A4"},
    {   0, "#CCCCCC"},  // 204:204:204 — Без напряжения
};

// Символ трансформатора по типу (из макроса AddLineVH)
// Используется при расширении: tip=1 всегда даёт "2tr", остальные — при tip узла.
static const std::map<int,std::string> kTransSymbol = {
    {0, "atr"  },
    {1, "3tr"  },
    {2, "2tr"  },
    {3, "3tr"  },
    {4, "atr-r"},
    {5, "atr"  },
    };

// Значения slb/sle, которые не нужно показывать (нулевые)
static const std::set<std::string> kZeroVals = {"0", "0.0", "0,0"};

NativeGraphHandler::NativeGraphHandler(ITableRepository* pRepo, QObject* parent)
    : QObject(parent), m_pRepo(pRepo)
{}

long NativeGraphHandler::getLong(const QDataBlock& b, int row, long col)
{
    return std::visit(ToLong{}, b.Get(row, col));
}

double NativeGraphHandler::getDouble(const QDataBlock& b, int row, long col)
{
    return std::visit(ToDouble{}, b.Get(row, col));
}

std::string NativeGraphHandler::getString(const QDataBlock& b, int row, long col)
{
    auto s = std::visit(ToString{}, b.Get(row, col));
    // убираем пробелы по краям (как .strip() в Python)
    auto beg = s.find_first_not_of(" \t\r\n");
    auto end = s.find_last_not_of(" \t\r\n");
    return (beg == std::string::npos) ? "" : s.substr(beg, end - beg + 1);
}

GraphLayout NativeGraphHandler::emptyLayout()
{
    GraphLayout l;
    l.view_box = {0.f, 0.f, 1000.f, 1000.f};
    return l;
}

std::string NativeGraphHandler::voltageColor(int uhom)
{
    auto it = kVoltageColors.find(uhom);
    if (it != kVoltageColors.end())
        return it->second;

    // ближайший ключ
    int best    = 0;
    int bestDist = INT_MAX;
    for (auto& [k, v] : kVoltageColors) {
        int d = std::abs(k - uhom);
        if (d < bestDist) { bestDist = d; best = k; }
    }
    return kVoltageColors.at(best);
}

std::pair<float,float> NativeGraphHandler::connPoint(
    float sx, float sy, int npri, float nprb, int nodType)
{
    // Точка подключения ветви к узлу.
    // Алгоритм из макроса: AddLine, начало ветви.
    if (nodType == -1) {
        // Шина
        if (npri > 0)
            return {sx + nprb * SCALE, sy};
        else
            return {sx, sy - nprb * SCALE};
    }
    // Трансформатор: GetTransPos — сложная логика ориентации.
    // MVP: подключаем в центр узла.
    return {sx, sy};
}

std::tuple<float,float,float> NativeGraphHandler::labelPos(
    const QDataBlock& gt, int gtSize, int gtIdx,
    long colWx, long colWy, long colUgol,
    float cx, float cy)
{
    // Позиция подписи ветви из таблицы graph_text.
    // Возвращает (x, y, angle_deg).
    // Смещения w_x/w_y хранятся в условных единицах:
    //   32 субпикселя = 1 пиксель → сдвиг на 1 ед.
    if (gtIdx < 0 || gtIdx >= gtSize)
        return {cx, cy, 0.f};

    float rawX = static_cast<float>(getDouble(gt, gtIdx, colWx)) * SCALE;
    float rawY = static_cast<float>(getDouble(gt, gtIdx, colWy)) * SCALE;
    int   ox   = (static_cast<int>(rawX) / 32 + 1) / 2;
    int   oy   = (static_cast<int>(rawY) / 32 + 1) / 2;
    float ang  = static_cast<float>(getDouble(gt, gtIdx, colUgol));
    return {cx + static_cast<float>(ox), cy + static_cast<float>(oy), ang};
}

std::string NativeGraphHandler::buildLabelTransform(
    const QDataBlock& gt, int gtSize, int gtIdx,
    long colWx, long colWy, long colUgol,
    float cx, float cy)
{
    // SVG transform строка для подписи:
    //   "translate(x,y)" или "translate(x,y) rotate(a)"
    auto [lx, ly, ang] = labelPos(gt, gtSize, gtIdx,
                                  colWx, colWy, colUgol, cx, cy);
    char buf[128];
    if (ang != 0.f)
        std::snprintf(buf, sizeof(buf),
                      "translate(%.2f,%.2f) rotate(%.3f)", lx, ly, ang);
    else
        std::snprintf(buf, sizeof(buf),
                      "translate(%.2f,%.2f)", lx, ly);
    return buf;
}

GraphLayout NativeGraphHandler::getLayout(
    int /*layer*/,
    std::optional<std::array<float,4>> bbox,
    bool metadataOnly,
    bool autoCenter)
{
    // Этапы построения схемы:
    //   1. graph_node → вычислить начало координат (wx, wy), полный viewBox
    //   2. Ранний выход при metadataOnly=true (только viewBox + voltageStyles)
    //   3. Читать электрические таблицы: node, ATtrans, graph_figur, graph_text, graph_settext
    //   4. Построить список узлов: SVG-позиция, тип, фигуры оборудования, сдвиг подписи
    //   5. Читать таблицы маршрутов: graph_vetv, vetv
    //   6. Построить список ветвей: полилинии, символы трансформаторов, подписи токов
    //   7. Собрать voltage_styles и вернуть GraphLayout

    // ── 1. graph_node: координаты и параметры отображения ────────────────────
    auto gnBlock = m_pRepo->getBlock("graph_node", "ny,k_x,k_y,npri,ind_fig,ind_text");
    const QDataBlock& gn = *gnBlock;
    const int gnSize = static_cast<int>(gn.RowsCount());

    const long gnNy      = m_pRepo->columnIndex("graph_node", "ny");
    const long gnKx      = m_pRepo->columnIndex("graph_node", "k_x");
    const long gnKy      = m_pRepo->columnIndex("graph_node", "k_y");
    const long gnNpri    = m_pRepo->columnIndex("graph_node", "npri");
    const long gnIndFig  = m_pRepo->columnIndex("graph_node", "ind_fig");
    const long gnIndText = m_pRepo->columnIndex("graph_node", "ind_text");

    if (gnSize == 0) {
        spdlog::warn("NativeGraphHandler: graph_node пуста — схема не загружена");
        return emptyLayout();
    }

    // wx, wy — начало координат (минимумы k_x / k_y по всем узлам)
    int wx = INT_MAX, wy = INT_MAX;
    for (int i = 0; i < gnSize; i++) {
        wx = std::min(wx, static_cast<int>(getLong(gn, i, gnKx)));
        wy = std::min(wy, static_cast<int>(getLong(gn, i, gnKy)));
    }

    // Полный viewBox — вычисляем по ВСЕМ узлам до применения bbox-фильтра.
    // Нужен для autoCenter и для корректного viewBox при bbox-запросах.
    float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
    for (int i = 0; i < gnSize; i++) {
        float sx = static_cast<float>((getLong(gn, i, gnKx) - wx) * SCALE);
        float sy = static_cast<float>((getLong(gn, i, gnKy) - wy) * SCALE);
        minX = std::min(minX, sx); maxX = std::max(maxX, sx);
        minY = std::min(minY, sy); maxY = std::max(maxY, sy);
    }
    ViewBox fullViewBox{
        minX - 100.f, minY - 100.f,
        maxX - minX + 200.f, maxY - minY + 200.f
    };

    // autoCenter: вычислить центральный bbox на сервере (центр 50% схемы)
    if (autoCenter && !bbox.has_value()) {
        float cx = fullViewBox.x + fullViewBox.width  * 0.5f;
        float cy = fullViewBox.y + fullViewBox.height * 0.5f;
        float bw = fullViewBox.width  * 0.5f;
        float bh = fullViewBox.height * 0.5f;
        bbox = {cx - bw/2.f, cy - bh/2.f, cx + bw/2.f, cy + bh/2.f};
    }

    // ── 2. metadataOnly — быстрый путь: только viewBox + voltageStyles ────────
    // Читаем только 2 таблицы вместо 8+.
    if (metadataOnly) {
        auto ndBlock2 = m_pRepo->getBlock("node", "uhom");
        const QDataBlock& nd2 = *ndBlock2;
        const long ndUhom2 = m_pRepo->columnIndex("node", "uhom");
        const int  ndSize2 = static_cast<int>(nd2.RowsCount());

        // sorted(unique_uhom, reverse=True) → std::set + реверсный обход
        std::set<int> uhoms;
        for (int i = 0; i < ndSize2; i++)
            uhoms.insert(static_cast<int>(getLong(nd2, i, ndUhom2)));

        GraphLayout layout;
        layout.view_box = fullViewBox;
        for (auto it = uhoms.rbegin(); it != uhoms.rend(); ++it)
            layout.voltage_styles.push_back({"u" + std::to_string(*it),
                                             voltageColor(*it), 1.f});
        layout.voltage_styles.push_back({"u-off",      "#CCCCCC", 1.f});
        layout.voltage_styles.push_back({"u-unknown",  "#8C8C8C", 1.f});
        layout.voltage_styles.push_back({"u-overload", "#FF0000", 2.f});
        layout.layers.push_back({0, "Основная схема"});
        return layout;
    }

    // ── 3. Читаем электрические таблицы ──────────────────────────────────────

    // node: электрические данные
    auto ndBlock = m_pRepo->getBlock("node", "ny,sta,uhom,name,vras,pg,bsh");
    const QDataBlock& nd = *ndBlock;
    const int ndSize = static_cast<int>(nd.RowsCount());

    const long ndNy   = m_pRepo->columnIndex("node", "ny");
    const long ndSta  = m_pRepo->columnIndex("node", "sta");
    const long ndUhom = m_pRepo->columnIndex("node", "uhom");
    const long ndName = m_pRepo->columnIndex("node", "name");
    const long ndPg   = m_pRepo->columnIndex("node", "pg");
    const long ndBsh  = m_pRepo->columnIndex("node", "bsh");

    // node_by_ny: ny → индекс строки в таблице node
    std::unordered_map<int,int> nodeByNy;
    nodeByNy.reserve(ndSize);
    for (int i = 0; i < ndSize; i++)
        nodeByNy[static_cast<int>(getLong(nd, i, ndNy))] = i;

    // ATtrans: узлы-трансформаторы {ny → type}
    std::unordered_map<int,int> atByNy;
    if (m_pRepo->tableExists("ATtrans")) {
        try {
            auto atBlock = m_pRepo->getBlock("ATtrans", "n_zero,Type");
            const QDataBlock& at = *atBlock;
            const int atSize = static_cast<int>(at.RowsCount());
            const long atNZero = m_pRepo->columnIndex("ATtrans", "n_zero");
            const long atType  = m_pRepo->columnIndex("ATtrans", "Type");
            for (int i = 0; i < atSize; i++) {
                int t = static_cast<int>(getLong(at, i, atType));
                if (t != 2)  // 2 = вторичная обмотка, не рисуем отдельно
                    atByNy[static_cast<int>(getLong(at, i, atNZero))] = t;
            }
        } catch (const std::exception& e) {
            spdlog::warn("NativeGraphHandler: ATtrans read error: {}", e.what());
        }
    }

    // graph_figur: фигуры (генераторы, реакторы, шунты)
    std::shared_ptr<QDataBlock> gfBlock;
    int gfSize = 0;
    long gfSizeCol = 0, gfZapr = 0, gfNpr = 0;
    try {
        gfBlock  = m_pRepo->getBlock("graph_figur", "size,zapret,npr");
        gfSize   = static_cast<int>(gfBlock->RowsCount());
        gfSizeCol = m_pRepo->columnIndex("graph_figur", "size");
        gfZapr   = m_pRepo->columnIndex("graph_figur", "zapret");
        gfNpr    = m_pRepo->columnIndex("graph_figur", "npr");
    } catch (const std::exception& e) {
        spdlog::warn("NativeGraphHandler: graph_figur недоступна: {}", e.what());
    }

    // graph_text: позиции меток узлов и ветвей
    auto gtBlock = m_pRepo->getBlock("graph_text", "w_x,w_y,ugol");
    const QDataBlock& gt = *gtBlock;
    const int gtSize = static_cast<int>(gt.RowsCount());
    const long gtWx   = m_pRepo->columnIndex("graph_text", "w_x");
    const long gtWy   = m_pRepo->columnIndex("graph_text", "w_y");
    const long gtUgol = m_pRepo->columnIndex("graph_text", "ugol");

    // graph_settext: какой слот соответствует колонке "name"
    int nameGtOffset = 0;
    if (m_pRepo->tableExists("graph_settext")) {
        try {
            auto gsBlock = m_pRepo->getBlock("graph_settext", "tip,zapret");
            const QDataBlock& gs = *gsBlock;
            const int gsSize = static_cast<int>(gs.RowsCount());
            const long gsTip  = m_pRepo->columnIndex("graph_settext", "tip");
            const long gsZapr = m_pRepo->columnIndex("graph_settext", "zapret");
            for (int i = 0; i < gsSize; i++) {
                if (getString(gs, i, gsTip) == "name" &&
                    getLong(gs, i, gsZapr) == 0)
                {
                    nameGtOffset = i;
                    break;
                }
            }
        } catch (const std::exception& e) {
            spdlog::warn("NativeGraphHandler: graph_settext недоступна: {}", e.what());
        }
    }

    // ── Bbox-фильтр: конвертировать SVG coords → rastr coords ────────────────
    // svg_x = (k_x - wx) * SCALE  →  k_x = svg_x / SCALE + wx
    int bkxMin = INT_MIN, bkxMax = INT_MAX;
    int bkyMin = INT_MIN, bkyMax = INT_MAX;
    if (bbox.has_value()) {
        float x0 = (*bbox)[0], y0 = (*bbox)[1];
        float x1 = (*bbox)[2], y1 = (*bbox)[3];
        bkxMin = static_cast<int>(x0 / SCALE) + wx - 1;
        bkxMax = static_cast<int>(x1 / SCALE) + wx + 1;
        bkyMin = static_cast<int>(y0 / SCALE) + wy - 1;
        bkyMax = static_cast<int>(y1 / SCALE) + wy + 1;
    }

    // ── 4. Построить список узлов ─────────────────────────────────────────────
    GraphLayout layout;
    layout.view_box = fullViewBox;

    std::unordered_map<int, std::pair<float,float>> nodePos;
    std::unordered_map<int,int> npriByNy;

    for (int i = 0; i < gnSize; i++) {
        int ny   = static_cast<int>(getLong(gn, i, gnNy));
        int kx   = static_cast<int>(getLong(gn, i, gnKx));
        int ky   = static_cast<int>(getLong(gn, i, gnKy));
        int npri = static_cast<int>(getLong(gn, i, gnNpri));
        npriByNy[ny] = npri;

        if (!nodeByNy.count(ny)) continue;

        // Применить bbox-фильтр
        if (kx < bkxMin || kx > bkxMax || ky < bkyMin || ky > bkyMax)
            continue;

        float svgX = static_cast<float>((kx - wx) * SCALE);
        float svgY = static_cast<float>((ky - wy) * SCALE);
        nodePos[ny] = {svgX, svgY};

        int ndIdx   = nodeByNy[ny];
        int sta     = static_cast<int>(getLong(nd, ndIdx, ndSta));
        int uhom    = static_cast<int>(getLong(nd, ndIdx, ndUhom));
        int nodType = atByNy.count(ny) ? atByNy.at(ny) : -1;

        // Имя узла и смещение подписи
        std::string nameStr = getString(nd, ndIdx, ndName);
        float nameDx = 2.f, nameDy = -3.f, nameAngle = 0.f;
        if (!nameStr.empty()) {
            int indTxt = static_cast<int>(getLong(gn, i, gnIndText)) + nameGtOffset;
            if (indTxt >= 0 && indTxt < gtSize) {
                float rawX = static_cast<float>(getDouble(gt, indTxt, gtWx)) * SCALE;
                float rawY = static_cast<float>(getDouble(gt, indTxt, gtWy)) * SCALE;
                nameDx    = static_cast<float>((static_cast<int>(rawX) / 32 + 1) / 2);
                nameDy    = static_cast<float>((static_cast<int>(rawY) / 32 + 1) / 2);
                nameAngle = static_cast<float>(getDouble(gt, indTxt, gtUgol));
            }
        }

        // Фигуры оборудования на шине (генераторы, реакторы, шунты)
        // nf=0 — генератор (кружок с кривой): не отображать если pg не задано
        // nf=1 — реактор/БСК: bsh>0 → реактор (symbol 4, nf=2), bsh<0 → БСК (symbol 5, nf=3)
        std::vector<NodeFigure> figures;
        if (sta == 0 && gfBlock) {
            const QDataBlock& gf = *gfBlock;
            double pg  = getDouble(nd, ndIdx, ndPg);
            double bsh = getDouble(nd, ndIdx, ndBsh);
            int indFig = static_cast<int>(getLong(gn, i, gnIndFig));

            for (int nf = 0; nf < 3; nf++) {
                int idx = indFig + nf;
                if (idx >= gfSize) break;
                if (getLong(gf, idx, gfZapr) == 1) continue;
                // nf==1 — генератор: не показывать если pg==0
                if (nf == 1 && pg == 0.0) continue;
                // nf==2 — реактор/БСК: bsh>=0 → реактор (actualNf=2), bsh<0 → БСК (actualNf=3)
                int actualNf = nf;
                if (nf == 2) actualNf = (bsh >= 0.0) ? 2 : 3;
                figures.push_back({
                    actualNf,
                    static_cast<int>(getLong(gf, idx, gfNpr)),
                    static_cast<int>(getLong(gf, idx, gfSizeCol))
                });
            }
        }

        NodeData node;
        node.id         = ny;
        node.x          = svgX;
        node.y          = svgY;
        node.uhom       = uhom;
        node.nod_type   = nodType;
        node.npri       = npri;
        node.sta        = sta;
        node.name       = std::move(nameStr);
        node.name_dx    = nameDx;
        node.name_dy    = nameDy;
        node.name_angle = nameAngle;
        node.figures    = std::move(figures);
        layout.nodes.push_back(std::move(node));
    }

    if (nodePos.empty()) {
        spdlog::warn("NativeGraphHandler: ни один узел graph_node не найден в node — файл не загружен?");
        return emptyLayout();
    }

    // ── 5. Читаем таблицы маршрутов ───────────────────────────────────────────

    // graph_vetv: маршрут ветвей
    auto gvBlock = m_pRepo->getBlock("graph_vetv",
                                     "ip,iq,nizg,nprb,npre,tip,ind_text,"
                                     "izg1_x,izg1_y,izg2_x,izg2_y,"
                                     "izg3_x,izg3_y,izg4_x,izg4_y");
    const QDataBlock& gv = *gvBlock;
    const int gvSize = static_cast<int>(gv.RowsCount());

    const long gvIp     = m_pRepo->columnIndex("graph_vetv", "ip");
    const long gvIq     = m_pRepo->columnIndex("graph_vetv", "iq");
    const long gvNizg   = m_pRepo->columnIndex("graph_vetv", "nizg");
    const long gvNprb   = m_pRepo->columnIndex("graph_vetv", "nprb");
    const long gvNpre   = m_pRepo->columnIndex("graph_vetv", "npre");
    const long gvTip    = m_pRepo->columnIndex("graph_vetv", "tip");
    const long gvIndTxt = m_pRepo->columnIndex("graph_vetv", "ind_text");
    const long izg_x[]  = {
        m_pRepo->columnIndex("graph_vetv", "izg1_x"),
        m_pRepo->columnIndex("graph_vetv", "izg2_x"),
        m_pRepo->columnIndex("graph_vetv", "izg3_x"),
        m_pRepo->columnIndex("graph_vetv", "izg4_x"),
    };
    const long izg_y[] = {
        m_pRepo->columnIndex("graph_vetv", "izg1_y"),
        m_pRepo->columnIndex("graph_vetv", "izg2_y"),
        m_pRepo->columnIndex("graph_vetv", "izg3_y"),
        m_pRepo->columnIndex("graph_vetv", "izg4_y"),
    };

    // vetv: электрические данные ветвей
    auto vtBlock = m_pRepo->getBlock("vetv", "ip,iq,slb,sle,sta,i_zag,signP");
    const QDataBlock& vt = *vtBlock;
    const int vtSize = static_cast<int>(vt.RowsCount());

    const long vtIp    = m_pRepo->columnIndex("vetv", "ip");
    const long vtIq    = m_pRepo->columnIndex("vetv", "iq");
    const long vtSlb   = m_pRepo->columnIndex("vetv", "slb");
    const long vtSle   = m_pRepo->columnIndex("vetv", "sle");
    const long vtSta   = m_pRepo->columnIndex("vetv", "sta");
    const long vtIZag  = m_pRepo->columnIndex("vetv", "i_zag");
    const long vtSignP = m_pRepo->columnIndex("vetv", "signP");

    // vetv_idx: (ip,iq) → строка в vetv
    std::map<std::pair<int,int>,int> vetvIdx;
    for (int i = 0; i < vtSize; i++) {
        auto key = std::make_pair(
            static_cast<int>(getLong(vt, i, vtIp)),
            static_cast<int>(getLong(vt, i, vtIq)));
        if (!vetvIdx.count(key)) vetvIdx[key] = i;
    }

    // ── 6. Построить список ветвей ────────────────────────────────────────────
    for (int i = 0; i < gvSize; i++) {
        int ip = static_cast<int>(getLong(gv, i, gvIp));
        int iq = static_cast<int>(getLong(gv, i, gvIq));

        if (!nodePos.count(ip) || !nodePos.count(iq)) continue;

        // Ищем запись в vetv (возможно в обратном порядке)
        bool reversedVetv = false;
        int  vi = -1;
        auto it = vetvIdx.find({ip, iq});
        if (it != vetvIdx.end()) {
            vi = it->second;
        } else {
            auto itr = vetvIdx.find({iq, ip});
            if (itr != vetvIdx.end()) { vi = itr->second; reversedVetv = true; }
        }
        if (vi < 0) continue;

        float nprb = static_cast<float>(getDouble(gv, i, gvNprb));
        float npre = static_cast<float>(getDouble(gv, i, gvNpre));
        int   nizg = static_cast<int>(getLong(gv, i, gvNizg));

        auto [bx, by] = nodePos[ip];
        auto [ex, ey] = nodePos[iq];

        auto [connBx, connBy] = connPoint(bx, by,
                                          npriByNy.count(ip) ? npriByNy[ip] : 0,
                                          nprb,
                                          atByNy.count(ip) ? atByNy[ip] : -1);
        auto [connEx, connEy] = connPoint(ex, ey,
                                          npriByNy.count(iq) ? npriByNy[iq] : 0,
                                          npre,
                                          atByNy.count(iq) ? atByNy[iq] : -1);

        // ── Строим полилинию ──────────────────────────────────────────────────
        // По макросу (AddLine): первые nizg-1 точек — смещения от conn_start,
        // последняя точка — смещение от conn_end.
        std::vector<float> pts = {connBx, connBy};
        int capped = std::min(nizg, 4);

        for (int n = 0; n < capped - 1; n++) {
            float dx = static_cast<float>(getDouble(gv, i, izg_x[n]));
            float dy = static_cast<float>(getDouble(gv, i, izg_y[n]));
            pts.push_back(connBx + dx * SCALE);
            pts.push_back(connBy + dy * SCALE);
        }
        if (capped >= 1) {
            float dx = static_cast<float>(getDouble(gv, i, izg_x[capped-1]));
            float dy = static_cast<float>(getDouble(gv, i, izg_y[capped-1]));
            pts.push_back(connEx + dx * SCALE);
            pts.push_back(connEy + dy * SCALE);
        }
        pts.push_back(connEx);
        pts.push_back(connEy);

        // ── Метки (slb/sle из vetv), позиции из graph_text ───────────────────
        std::string slbRaw = getString(vt, vi, vtSlb);
        std::string sleRaw = getString(vt, vi, vtSle);
        // Если vetv записана в обратном порядке — меняем местами
        if (reversedVetv) std::swap(slbRaw, sleRaw);

        int indTxt = static_cast<int>(getLong(gv, i, gvIndTxt));

        auto [x1txt, y1txt, bang] = labelPos(gt, gtSize, indTxt,
                                             gtWx, gtWy, gtUgol, connBx, connBy);
        auto [x2txt, y2txt, eang] = labelPos(gt, gtSize, indTxt + 1,
                                             gtWx, gtWy, gtUgol, connEx, connEy);

        // Направление стрелок — алгоритм VBA-макроса "Экспорт в SVG.rbs".
        // signP=0 → ip-источник ("text->"), signP=1 → iq-источник ("<-text").
        // При reversedVetv signP инвертируется (в vetv ip/iq переставлены).
        // Геометрика:
        //   ветвь вправо (x2>=x1) или вверх (y2<=y1) → инвертировать plb
        //   ветвь влево (x2<x1) или вниз (y2>y1) → поменять anchor (не инвертировать)
        int signPRaw = static_cast<int>(getLong(vt, vi, vtSignP));
        // Корректируем signP для перевёрнутых записей в vetv
        int plbBase = signPRaw ^ (reversedVetv ? 1 : 0);

        std::vector<BranchLabel> labels;

        // slb (slot 0): исходный anchor=start
        if (!slbRaw.empty() && !kZeroVals.count(slbRaw)) {
            int         plb0  = plbBase;
            std::string cvizb = "start";
            if (bang >= 0) {                     // горизонтальная подпись
                if (x2txt < x1txt) cvizb = "end";  // влево → меняем anchor, plb без изменений
                else               plb0 = 1 - plb0; // вправо → инвертируем plb
            } else {                             // вертикальная подпись
                if (y2txt > y1txt) cvizb = "end";   // вниз → меняем anchor, plb без изменений
                else               plb0 = 1 - plb0;  // вверх → инвертируем plb
            }
            std::string tf   = buildLabelTransform(gt, gtSize, indTxt,
                                                 gtWx, gtWy, gtUgol, connBx, connBy);
            std::string text = plb0 ? ("<-" + slbRaw) : (slbRaw + "->");
            labels.push_back({0, cvizb, std::move(tf), std::move(text)});
        }

        // sle (slot 1): исходный anchor=end, plb сбрасывается
        if (!sleRaw.empty() && !kZeroVals.count(sleRaw)) {
            int         plb1  = plbBase;
            std::string cvize = "end";
            if (eang >= 0) {                      // горизонтальная подпись
                if (x2txt < x1txt) cvize = "start"; // влево → меняем anchor, plb без изменений
                else               plb1 = 1 - plb1;  // вправо → инвертируем plb
            } else {                              // вертикальная подпись
                if (y2txt > y1txt) cvize = "start";  // вниз → меняем anchor, plb без изменений
                else               plb1 = 1 - plb1;   // вверх → инвертируем plb
            }
            std::string tf   = buildLabelTransform(gt, gtSize, indTxt + 1,
                                                 gtWx, gtWy, gtUgol, connEx, connEy);
            std::string text = plb1 ? ("<-" + sleRaw) : (sleRaw + "->");
            labels.push_back({1, cvize, std::move(tf), std::move(text)});
        }

        // ── Класс напряжения / особое состояние ──────────────────────────────
        int    staV  = static_cast<int>(getLong(vt, vi, vtSta));
        double iZagV = getDouble(vt, vi, vtIZag);

        std::string vcls, vclsEnd;
        if (staV != 0) {
            vcls = vclsEnd = "u-off";
        } else if (iZagV > 100.0) {
            vcls = vclsEnd = "u-overload";
        } else {
            int uhomIp = 0, uhomIq = 0;
            auto nip = nodeByNy.find(ip);
            auto niq = nodeByNy.find(iq);
            if (nip != nodeByNy.end())
                uhomIp = static_cast<int>(getLong(nd, nip->second, ndUhom));
            if (niq != nodeByNy.end())
                uhomIq = static_cast<int>(getLong(nd, niq->second, ndUhom));
            vcls    = "u" + std::to_string(uhomIp);
            vclsEnd = "u" + std::to_string(uhomIq);
        }

        // ── Фигура трансформатора (tip==1) ────────────────────────────────────
        // Трансформатор размещается на «среднем» сегменте ветви.
        //   nizg=0,1 → сегмент pts[0:2]→pts[2:4]
        //   nizg>=2  → сегмент pts[2:4]→pts[4:6]
        int tip = static_cast<int>(getLong(gv, i, gvTip));
        std::optional<BranchFigure>      figure;
        std::vector<std::vector<float>>  segments = {pts};

        if (tip == 1) {
            int   splitIdx;
            float sx2, sy2, ex2, ey2;

            if (nizg >= 2 && static_cast<int>(pts.size()) >= 6) {
                splitIdx = 4;
                sx2 = pts[2]; sy2 = pts[3];
                ex2 = pts[4]; ey2 = pts[5];
            } else {
                splitIdx = 2;
                sx2 = pts[0]; sy2 = pts[1];
                ex2 = pts[2]; ey2 = pts[3];
            }

            float midX     = (sx2 + ex2) * 0.5f;
            float midY     = (sy2 + ey2) * 0.5f;
            float bdx      = ex2 - sx2;
            float bdy      = ey2 - sy2;
            float angleDeg = std::atan2(bdy, bdx) * 180.f / static_cast<float>(M_PI);

            // Единичный вектор вдоль ветви
            float segLen = std::sqrt(bdx*bdx + bdy*bdy);
            float cosA = 1.f, sinA = 0.f;
            if (segLen > 0.f) { cosA = bdx/segLen; sinA = bdy/segLen; }

            // Линии заканчиваются у краёв кружков (r=3.5, центры -2.5 и +2.6 от mid)
            float pIpX = midX - 6.f * cosA;  // ближний край кружка ip-стороны
            float pIpY = midY - 6.f * sinA;
            float pIqX = midX + 6.1f * cosA; // ближний край кружка iq-стороны
            float pIqY = midY + 6.1f * sinA;

            char tfBuf[128];
            std::snprintf(tfBuf, sizeof(tfBuf),
                          "translate(%.2f,%.2f) rotate(%.3f)",
                          midX, midY, angleDeg);

            figure = BranchFigure{"2tr", tfBuf, vclsEnd};

            // Разбить pts на два сегмента: до и после трансформатора
            std::vector<float> seg1(pts.begin(), pts.begin() + splitIdx);
            seg1.push_back(pIpX); seg1.push_back(pIpY);

            std::vector<float> seg2 = {pIqX, pIqY};
            seg2.insert(seg2.end(), pts.begin() + splitIdx, pts.end());

            segments = {std::move(seg1), std::move(seg2)};
        }

        BranchData branch;
        branch.id                = i;
        branch.vbeg              = ip;
        branch.vend              = iq;
        branch.voltage_class     = std::move(vcls);
        branch.voltage_class_end = (tip == 1) ? vclsEnd : "";
        branch.segments          = std::move(segments);
        branch.figure            = std::move(figure);
        branch.labels            = std::move(labels);
        branch.state             = (staV == 0) ? 0 : 1;
        layout.branches.push_back(std::move(branch));
    }

    // ── 7. Стили напряжений из уникальных uhom + особые состояния ────────────
    // sorted(unique_uhom, reverse=True) → std::set + реверсный обход
    std::set<int> uhomSet;
    for (int i = 0; i < ndSize; i++)
        uhomSet.insert(static_cast<int>(getLong(nd, i, ndUhom)));
    for (auto it = uhomSet.rbegin(); it != uhomSet.rend(); ++it)
        layout.voltage_styles.push_back({"u" + std::to_string(*it),
                                         voltageColor(*it), 1.f});
    layout.voltage_styles.push_back({"u-off",      "#CCCCCC", 1.f}); // Без напряжения
    layout.voltage_styles.push_back({"u-unknown",  "#8C8C8C", 1.f}); // Неизвестно
    layout.voltage_styles.push_back({"u-overload", "#FF0000", 2.f}); // Перегрузка

    // Слои в rastr не хранятся в стандартной таблице — возвращаем один дефолтный
    layout.layers.push_back({0, "Основная схема"});

    if (bbox.has_value()) {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "%.2f,%.2f,%.2f,%.2f",
                      (*bbox)[0], (*bbox)[1], (*bbox)[2], (*bbox)[3]);
        layout.loaded_bbox = buf;
    }

    spdlog::info("NativeGraphHandler::getLayout → {} узлов, {} ветвей",
                 layout.nodes.size(), layout.branches.size());
    return layout;
}

void NativeGraphHandler::moveNode(int nodeId, float x, float y)
{
    // Записывает новую позицию в graph_node.k_x / k_y.
    // Нет оптимизатора — позиция сохраняется точно как указана.
    auto gnBlock = m_pRepo->getBlock("graph_node", "ny,k_x,k_y");
    const QDataBlock& gn = *gnBlock;
    const int gnSize = static_cast<int>(gn.RowsCount());

    const long gnNy = m_pRepo->columnIndex("graph_node", "ny");
    const long gnKx = m_pRepo->columnIndex("graph_node", "k_x");
    const long gnKy = m_pRepo->columnIndex("graph_node", "k_y");

    int wx = INT_MAX, wy = INT_MAX;
    for (int i = 0; i < gnSize; i++) {
        wx = std::min(wx, static_cast<int>(getLong(gn, i, gnKx)));
        wy = std::min(wy, static_cast<int>(getLong(gn, i, gnKy)));
    }

    for (int i = 0; i < gnSize; i++) {
        if (getLong(gn, i, gnNy) != nodeId) continue;

        long newKx = static_cast<long>(std::round(x / SCALE)) + wx;
        long newKy = static_cast<long>(std::round(y / SCALE)) + wy;

        m_pRepo->setValue("graph_node", "k_x", i, FieldVariantData{newKx});
        m_pRepo->setValue("graph_node", "k_y", i, FieldVariantData{newKy});

        spdlog::info("moveNode: ny={} → k_x={}, k_y={}", nodeId, newKx, newKy);
        return;
    }
    spdlog::warn("moveNode: узел {} не найден в graph_node", nodeId);
}

void NativeGraphHandler::toggleNode(int nodeId, const std::string& action)
{
    auto ndBlock = m_pRepo->getBlock("node", "ny,sta,sel");
    const QDataBlock& nd = *ndBlock;
    const int ndSize = static_cast<int>(nd.RowsCount());

    const long ndNy  = m_pRepo->columnIndex("node", "ny");
    const long ndSta = m_pRepo->columnIndex("node", "sta");
    const long ndSel = m_pRepo->columnIndex("node", "sel");

    for (int i = 0; i < ndSize; i++) {
        if (getLong(nd, i, ndNy) != nodeId) continue;

        if (action == "onoffnode") {
            long cur = getLong(nd, i, ndSta);
            m_pRepo->setValue("node", "sta", i,
                              FieldVariantData{cur != 0 ? 0L : 1L});
        } else if (action == "rechecknode") {
            long cur = getLong(nd, i, ndSel);
            m_pRepo->setValue("node", "sel", i,
                              FieldVariantData{cur != 0 ? 0L : 1L});
        }
        return;
    }
    spdlog::warn("toggleNode: узел {} не найден", nodeId);
}

void NativeGraphHandler::toggleBranch(int branchId, const std::string& action)
{
    auto gvBlock = m_pRepo->getBlock("graph_vetv", "ip,iq");
    const QDataBlock& gv = *gvBlock;
    const int gvSize = static_cast<int>(gv.RowsCount());

    if (branchId < 0 || branchId >= gvSize) {
        spdlog::warn("toggleBranch: ветвь {} вне диапазона", branchId);
        return;
    }

    const long gvIp = m_pRepo->columnIndex("graph_vetv", "ip");
    const long gvIq = m_pRepo->columnIndex("graph_vetv", "iq");

    int ip = static_cast<int>(getLong(gv, branchId, gvIp));
    int iq = static_cast<int>(getLong(gv, branchId, gvIq));

    auto vtBlock = m_pRepo->getBlock("vetv", "ip,iq,sta");
    const QDataBlock& vt = *vtBlock;
    const int vtSize = static_cast<int>(vt.RowsCount());

    const long vtIp  = m_pRepo->columnIndex("vetv", "ip");
    const long vtIq  = m_pRepo->columnIndex("vetv", "iq");
    const long vtSta = m_pRepo->columnIndex("vetv", "sta");

    for (int i = 0; i < vtSize; i++) {
        if (getLong(vt, i, vtIp) == ip && getLong(vt, i, vtIq) == iq) {
            if (action == "onoffvetv") {
                long cur = getLong(vt, i, vtSta);
                m_pRepo->setValue("vetv", "sta", i,
                                  FieldVariantData{cur != 0 ? 0L : 1L});
            }
            return;
        }
    }
    spdlog::warn("toggleBranch: ветвь ip={} iq={} не найдена в vetv", ip, iq);
}
