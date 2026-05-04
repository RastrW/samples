#include "native_graph_handler.h"

#include <spdlog/spdlog.h>
#include <cmath>
#include <algorithm>
#include <climits>
#include <cfloat>
#include <map>
#include <set>

//  Таблица цветов уровней напряжения
// RGB → HEX: (R,G,B) → #RRGGBB
static const std::map<int,std::string> kVoltageColors = {
    {1150, "#CD8AFF"},
    { 750, "#4141F0"},
    { 500, "#B80000"},
    { 400, "#87FDC2"},
    { 330, "#00CC00"},
    { 220, "#CCCC00"},
    { 150, "#AA9600"},
    { 110, "#4699CC"},
    {  60, "#C25A5A"},
    {  35, "#C25A5A"},
    {  27, "#C25A5A"},
    {  20, "#A464A4"},
    {  15, "#A464A4"},
    {  10, "#A464A4"},
    {   6, "#A464A4"},
    {   0, "#CCCCCC"},
    };

NativeGraphHandler::NativeGraphHandler(ITableRepository* pRepo,
                                       QObject* parent)
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
    if (it != kVoltageColors.end()) return it->second;

    // ближайший ключ
    int best = 0;
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
    // Трансформатор: MVP — подключаем в центр узла
    return {sx, sy};
}

std::tuple<float,float,float> NativeGraphHandler::labelPos(
    const QDataBlock& gt, int gtSize, int gtIdx,
    int colWx, int colWy, int colUgol,
    float cx, float cy)
{
    //Позиция подписи ветви из таблицы graph_text.
    //Возвращает (x, y, angle_deg).
    //Смещения w_x/w_y хранятся в условных единицах: 32 субпикселя = 1 пиксель → сдвиг на 1 ед.
    if (gtIdx < 0 || gtIdx >= gtSize)
        return {cx, cy, 0.f};

    float rawX = getDouble(gt, gtIdx, colWx) * SCALE;
    float rawY = getDouble(gt, gtIdx, colWy) * SCALE;
    int   ox   = (static_cast<int>(rawX) / 32 + 1) / 2;
    int   oy   = (static_cast<int>(rawY) / 32 + 1) / 2;
    float ang  = static_cast<float>(getDouble(gt, gtIdx, colUgol));
    return {cx + ox, cy + oy, ang};
}

std::string NativeGraphHandler::buildLabelTransform(
    const QDataBlock& gt, int gtSize, int gtIdx,
    int colWx, int colWy, int colUgol,
    float cx, float cy)
{
    //SVG transform строка для подписи: 'translate(x,y)' или 'translate(x,y) rotate(a)'
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
    //Этапы построения схемы:
    //1. graph_node → вычислить начало координат (wx, wy), полный viewBox
    //2. Ранний выход при metadata_only=True (только viewBox + voltage_styles)
    //3. Читать электрические таблицы: node, ATtrans, graph_figur, graph_text, graph_settext
    //4. Построить список узлов: SVG-позиция, тип, фигуры оборудования, сдвиг подписи
    //5. Читать таблицы маршрутов: graph_vetv, vetv
    //6. Построить список ветвей: полилинии, символы трансформаторов, подписи токов
    //7. Собрать voltage_styles и вернуть GraphLayout

    // ── 1. graph_node: координаты узлов ─────────────────────────────────────
    auto gnBlock = m_pRepo->getBlock("graph_node", "ny,k_x,k_y,npri,ind_fig,ind_text");
    const QDataBlock& gn = *gnBlock;
    const int gnSize = static_cast<int>(gn.RowsCount());

    // Индексы колонок graph_node
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
    // Нужен для auto_center и для корректного viewBox при bbox-запросах.
    float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
    for (int i = 0; i < gnSize; i++) {
        float sx = static_cast<float>((getLong(gn, i, gnKx) - wx) * SCALE);
        float sy = static_cast<float>((getLong(gn, i, gnKy) - wy) * SCALE);
        minX = std::min(minX, sx); maxX = std::max(maxX, sx);
        minY = std::min(minY, sy); maxY = std::max(maxY, sy);
    }
    ViewBox fullViewBox {
        minX - 100.f, minY - 100.f,
        maxX - minX + 200.f, maxY - minY + 200.f
    };

    // auto_center: сервер сам выбирает центральный bbox (50% схемы)
    if (autoCenter && !bbox.has_value()) {
        float cx = fullViewBox.x + fullViewBox.width  * 0.5f;
        float cy = fullViewBox.y + fullViewBox.height * 0.5f;
        float bw = fullViewBox.width  * 0.5f;
        float bh = fullViewBox.height * 0.5f;
        bbox = {cx - bw/2, cy - bh/2, cx + bw/2, cy + bh/2};
    }

    // ── 2. metadata_only — быстрый путь (только viewBox + voltageStyles) ────
    if (metadataOnly) {
        auto ndBlock = m_pRepo->getBlock("node", "uhom");
        const QDataBlock& nd = *ndBlock;
        const long ndUhom = m_pRepo->columnIndex("node", "uhom");
        const int  ndSize = static_cast<int>(nd.RowsCount());

        std::set<int> uhoms;
        for (int i = 0; i < ndSize; i++)
            uhoms.insert(static_cast<int>(getLong(nd, i, ndUhom)));

        GraphLayout layout;
        layout.view_box = fullViewBox;
        for (int u : uhoms){
            layout.voltage_styles.push_back({"u" + std::to_string(u),
                                             voltageColor(u), 1.f});
        }
        layout.voltage_styles.push_back({"u-off",      "#CCCCCC", 1.f});
        layout.voltage_styles.push_back({"u-unknown",  "#8C8C8C", 1.f});
        layout.voltage_styles.push_back({"u-overload", "#FF0000", 2.f});
        return layout;
    }

    // ── 3. Читаем электрические таблицы ─────────────────────────────────────
    auto ndBlock = m_pRepo->getBlock("node",        "ny,sta,uhom,name,vras,pg,bsh");
    auto gfBlock = m_pRepo->getBlock("graph_figur", "size,zapret,npr");
    auto gtBlock = m_pRepo->getBlock("graph_text",  "w_x,w_y,ugol");

    const QDataBlock& nd = *ndBlock;
    const QDataBlock& gf = *gfBlock;
    const QDataBlock& gt = *gtBlock;

    const int ndSize = static_cast<int>(nd.RowsCount());
    const int gfSize = static_cast<int>(gf.RowsCount());
    const int gtSize = static_cast<int>(gt.RowsCount());

    // Индексы колонок node
    const long ndNy   = m_pRepo->columnIndex("node", "ny");
    const long ndSta  = m_pRepo->columnIndex("node", "sta");
    const long ndUhom = m_pRepo->columnIndex("node", "uhom");
    const long ndName = m_pRepo->columnIndex("node", "name");
    const long ndPg   = m_pRepo->columnIndex("node", "pg");
    const long ndBsh  = m_pRepo->columnIndex("node", "bsh");

    // Индексы колонок graph_figur
    const long gfSize_ = m_pRepo->columnIndex("graph_figur", "size");
    const long gfZapr  = m_pRepo->columnIndex("graph_figur", "zapret");
    const long gfNpr   = m_pRepo->columnIndex("graph_figur", "npr");

    // Индексы колонок graph_text
    const long gtWx   = m_pRepo->columnIndex("graph_text", "w_x");
    const long gtWy   = m_pRepo->columnIndex("graph_text", "w_y");
    const long gtUgol = m_pRepo->columnIndex("graph_text", "ugol");

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
                if (t != 2)   // 2 = вторичная обмотка — не рисуем отдельно
                    atByNy[static_cast<int>(getLong(at, i, atNZero))] = t;
            }
        } catch (const std::exception& e) {
            spdlog::warn("NativeGraphHandler: ATtrans read error: {}", e.what());
        }
    }

    //graph_fiqur
    //graph_text
    // graph_settext: смещение слота подписи "name"
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
            spdlog::warn("NativeGraphHandler: graph_settext error: {}", e.what());
        }
    }

    // ── bbox-фильтр → rastr-координаты ──────────────────────────────────────
    int bkxMin = INT_MIN, bkxMax = INT_MAX;
    int bkyMin = INT_MIN, bkyMax = INT_MAX;
    if (bbox.has_value()) {
        float x0 = (*bbox)[0], y0 = (*bbox)[1],
            x1 = (*bbox)[2], y1 = (*bbox)[3];
        bkxMin = static_cast<int>(x0 / SCALE) + wx - 1;
        bkxMax = static_cast<int>(x1 / SCALE) + wx + 1;
        bkyMin = static_cast<int>(y0 / SCALE) + wy - 1;
        bkyMax = static_cast<int>(y1 / SCALE) + wy + 1;
    }

    // ── 4. Строим список узлов ───────────────────────────────────────────────
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

        // bbox-фильтр
        if (kx < bkxMin || kx > bkxMax || ky < bkyMin || ky > bkyMax)
            continue;

        float sx = static_cast<float>((kx - wx) * SCALE);
        float sy = static_cast<float>((ky - wy) * SCALE);
        nodePos[ny] = {sx, sy};

        int ndIdx   = nodeByNy[ny];
        int sta     = static_cast<int>(getLong(nd, ndIdx, ndSta));
        int uhom    = static_cast<int>(getLong(nd, ndIdx, ndUhom));
        int nodType = atByNy.count(ny) ? atByNy[ny] : -1;

        // Смещение подписи
        std::string nameStr = getString(nd, ndIdx, ndName);
        float nameDx = 2.f, nameDy = -3.f;
        if (!nameStr.empty()) {
            int indTxt = static_cast<int>(getLong(gn, i, gnIndText)) + nameGtOffset;
            if (indTxt >= 0 && indTxt < gtSize) {
                float rawX = getDouble(gt, indTxt, gtWx) * SCALE;
                float rawY = getDouble(gt, indTxt, gtWy) * SCALE;
                nameDx = static_cast<float>((static_cast<int>(rawX) / 32 + 1) / 2);
                nameDy = static_cast<float>((static_cast<int>(rawY) / 32 + 1) / 2);
            }
        }

        // Фигуры оборудования (генераторы, реакторы, шунты) — только если узел включён
        std::vector<NodeFigure> figures;
        if (sta == 0) {
            double pg  = getDouble(nd, ndIdx, ndPg);
            double bsh = getDouble(nd, ndIdx, ndBsh);
            int indFig = static_cast<int>(getLong(gn, i, gnIndFig));

            for (int nf = 0; nf < 3; nf++) {
                int idx = indFig + nf;
                if (idx >= gfSize) break;
                if (getLong(gf, idx, gfZapr) == 1) continue;
                // nf==1 — генератор: не показывать если pg==0
                if (nf == 1 && pg == 0.0) continue;
                // nf==2 — реактор/БСК: bsh>=0 → реактор, bsh<0 → БСК
                int actualNf = nf;
                if (nf == 2) actualNf = (bsh >= 0.0) ? 2 : 3;
                figures.push_back({
                    actualNf,
                    static_cast<int>(getLong(gf, idx, gfNpr)),
                    static_cast<int>(getLong(gf, idx, gfSize_))
                });
            }
        }

        NodeData node;
        node.id       = ny;
        node.x        = sx;
        node.y        = sy;
        node.uhom     = uhom;
        node.nod_type = nodType;
        node.npri     = npri;
        node.sta      = sta;
        node.name     = std::move(nameStr);
        node.name_dx  = nameDx;
        node.name_dy  = nameDy;
        node.figures  = std::move(figures);
        layout.nodes.push_back(std::move(node));
    }

    if (nodePos.empty()) {
        spdlog::warn("NativeGraphHandler: ни один узел не найден — файл не загружен?");
        return emptyLayout();
    }

    // ── 5. Читаем таблицы ветвей ─────────────────────────────────────────────
    //graph_vetv: маршрут ветвей
    auto gvBlock = m_pRepo->getBlock("graph_vetv",
                             "ip,iq,nizg,nprb,npre,tip,ind_text,"
                             "izg1_x,izg1_y,izg2_x,izg2_y,"
                             "izg3_x,izg3_y,izg4_x,izg4_y");
    auto vtBlock = m_pRepo->getBlock("vetv", "ip,iq,slb,sle,sta,i_zag,signP");

    const QDataBlock& gv = *gvBlock;
    const QDataBlock& vt = *vtBlock;

    const int gvSize = static_cast<int>(gv.RowsCount());
    const int vtSize = static_cast<int>(vt.RowsCount());

    // Индексы колонок graph_vetv
    const long gvIp     = m_pRepo->columnIndex("graph_vetv", "ip");
    const long gvIq     = m_pRepo->columnIndex("graph_vetv", "iq");
    const long gvNizg   = m_pRepo->columnIndex("graph_vetv", "nizg");
    const long gvNprb   = m_pRepo->columnIndex("graph_vetv", "nprb");
    const long gvNpre   = m_pRepo->columnIndex("graph_vetv", "npre");
    const long gvTip    = m_pRepo->columnIndex("graph_vetv", "tip");
    const long gvIndTxt = m_pRepo->columnIndex("graph_vetv", "ind_text");
    const long gvIzg1x  = m_pRepo->columnIndex("graph_vetv", "izg1_x");
    const long gvIzg1y  = m_pRepo->columnIndex("graph_vetv", "izg1_y");
    const long gvIzg2x  = m_pRepo->columnIndex("graph_vetv", "izg2_x");
    const long gvIzg2y  = m_pRepo->columnIndex("graph_vetv", "izg2_y");
    const long gvIzg3x  = m_pRepo->columnIndex("graph_vetv", "izg3_x");
    const long gvIzg3y  = m_pRepo->columnIndex("graph_vetv", "izg3_y");
    const long gvIzg4x  = m_pRepo->columnIndex("graph_vetv", "izg4_x");
    const long gvIzg4y  = m_pRepo->columnIndex("graph_vetv", "izg4_y");

    //vetv: электрические данные ветвей
    const long vtIp    = m_pRepo->columnIndex("vetv", "ip");
    const long vtIq    = m_pRepo->columnIndex("vetv", "iq");
    const long vtSlb   = m_pRepo->columnIndex("vetv", "slb");
    const long vtSle   = m_pRepo->columnIndex("vetv", "sle");
    const long vtSta   = m_pRepo->columnIndex("vetv", "sta");
    const long vtIZag  = m_pRepo->columnIndex("vetv", "i_zag");
    const long vtSignP = m_pRepo->columnIndex("vetv", "signP");

    // Имена колонок изгибов (порядок соответствует индексам gvIzg*x/y)
    const long izg_x[] = {gvIzg1x, gvIzg2x, gvIzg3x, gvIzg4x};
    const long izg_y[] = {gvIzg1y, gvIzg2y, gvIzg3y, gvIzg4y};

    // vetv_idx: (ip,iq) → строка
    std::map<std::pair<int,int>,int> vetvIdx;
    for (int i = 0; i < vtSize; i++) {
        auto key = std::make_pair(
            static_cast<int>(getLong(vt, i, vtIp)),
            static_cast<int>(getLong(vt, i, vtIq)));
        if (!vetvIdx.count(key)) vetvIdx[key] = i;
    }

    // Значения slb/sle, которые не надо показывать
    static const std::set<std::string> kZeroVals = {"0","0.0","0,0"};

    // ── 6. Строим список ветвей ───────────────────────────────────────────────
    for (int i = 0; i < gvSize; i++) {
        int ip = static_cast<int>(getLong(gv, i, gvIp));
        int iq = static_cast<int>(getLong(gv, i, gvIq));

        if (!nodePos.count(ip) || !nodePos.count(iq)) continue;

        // Ищем запись в vetv (возможно в обратном порядке)
        bool reversedVetv = false;
        int vi = -1;
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
                                          nprb, atByNy.count(ip) ? atByNy[ip] : -1);
        auto [connEx, connEy] = connPoint(ex, ey,
                                          npriByNy.count(iq) ? npriByNy[iq] : 0,
                                          npre, atByNy.count(iq) ? atByNy[iq] : -1);

        // ── Строим полилинию ─────────────────────────────────────────────────
        // По макросу: первые nizg-1 точек — смещения от conn_start,
        // последняя — смещение от conn_end.
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

        // ── Метки тока ───────────────────────────────────────────────────────
        std::string slbRaw = getString(vt, vi, vtSlb);
        std::string sleRaw = getString(vt, vi, vtSle);
        if (reversedVetv) std::swap(slbRaw, sleRaw);

        int indTxt = static_cast<int>(getLong(gv, i, gvIndTxt));

        auto [x1txt, y1txt, bang] = labelPos(gt, gtSize, indTxt,
                                             gtWx, gtWy, gtUgol, connBx, connBy);
        auto [x2txt, y2txt, eang] = labelPos(gt, gtSize, indTxt + 1,
                                             gtWx, gtWy, gtUgol, connEx, connEy);

        int signPRaw  = static_cast<int>(getLong(vt, vi, vtSignP));
        int plbBase   = signPRaw ^ (reversedVetv ? 1 : 0);

        std::vector<BranchLabel> labels;

        // slb (slot 0)
        if (!slbRaw.empty() && !kZeroVals.count(slbRaw)) {
            int plb0 = plbBase;
            std::string cvizb = "start";
            if (bang >= 0) {
                if (x2txt < x1txt) cvizb = "end";
                else                plb0 = 1 - plb0;
            } else {
                if (y2txt > y1txt) cvizb = "end";
                else                plb0 = 1 - plb0;
            }
            std::string tf   = buildLabelTransform(gt, gtSize, indTxt,
                                                 gtWx, gtWy, gtUgol, connBx, connBy);
            std::string text = plb0 ? ("<-" + slbRaw) : (slbRaw + "->");
            labels.push_back({0, cvizb, tf, text});
        }

        // sle (slot 1)
        if (!sleRaw.empty() && !kZeroVals.count(sleRaw)) {
            int plb1 = plbBase;
            std::string cvize = "end";
            if (eang >= 0) {
                if (x2txt < x1txt) cvize = "start";
                else                plb1 = 1 - plb1;
            } else {
                if (y2txt > y1txt) cvize = "start";
                else                plb1 = 1 - plb1;
            }
            std::string tf   = buildLabelTransform(gt, gtSize, indTxt + 1,
                                                 gtWx, gtWy, gtUgol, connEx, connEy);
            std::string text = plb1 ? ("<-" + sleRaw) : (sleRaw + "->");
            labels.push_back({1, cvize, tf, text});
        }

        // ── Класс напряжения / особое состояние ─────────────────────────────
        int    staV   = static_cast<int>(getLong(vt, vi, vtSta));
        double iZagV  = getDouble(vt, vi, vtIZag);

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

        // ── Фигура трансформатора (tip == 1) ─────────────────────────────────
        int tip = static_cast<int>(getLong(gv, i, gvTip));
        std::optional<BranchFigure> figure;
        std::vector<std::vector<float>> segments = {pts};

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

            float midX = (sx2 + ex2) * 0.5f;
            float midY = (sy2 + ey2) * 0.5f;
            float bdx  = ex2 - sx2;
            float bdy  = ey2 - sy2;
            float angleDeg = std::atan2(bdy, bdx) * 180.f / M_PI;

            float segLen = std::sqrt(bdx*bdx + bdy*bdy);
            float cosA = 1.f, sinA = 0.f;
            if (segLen > 0.f) { cosA = bdx/segLen; sinA = bdy/segLen; }

            // Края кружков (r=3.5, центры -2.5 и +2.6 от mid)
            float pIpX = midX - 6.f * cosA;
            float pIpY = midY - 6.f * sinA;
            float pIqX = midX + 6.1f * cosA;
            float pIqY = midY + 6.1f * sinA;

            char tfBuf[128];
            std::snprintf(tfBuf, sizeof(tfBuf),
                          "translate(%.2f,%.2f) rotate(%.3f)",
                          midX, midY, angleDeg);

            figure = BranchFigure{"2tr", tfBuf, vclsEnd};

            // Разбить на два сегмента: до и после трансформатора
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
        branch.voltage_class     = vcls;
        branch.voltage_class_end = (tip == 1) ? vclsEnd : "";
        branch.segments          = std::move(segments);
        branch.figure            = std::move(figure);
        branch.labels            = std::move(labels);
        branch.state             = (staV == 0) ? 0 : 1;
        layout.branches.push_back(std::move(branch));
    }

    // ── 7. Стили напряжений ──────────────────────────────────────────────────
    std::set<int> uhomSet;
    for (int i = 0; i < ndSize; i++)
        uhomSet.insert(static_cast<int>(getLong(nd, i, ndUhom)));
    for (int u : uhomSet)
        layout.voltage_styles.push_back({"u" + std::to_string(u),
                                         voltageColor(u), 1.f});
    layout.voltage_styles.push_back({"u-off",      "#CCCCCC", 1.f});
    layout.voltage_styles.push_back({"u-unknown",  "#8C8C8C", 1.f});
    layout.voltage_styles.push_back({"u-overload", "#FF0000", 2.f});

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