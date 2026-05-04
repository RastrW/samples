#pragma once

#pragma once
#include <QObject>
#include <optional>
#include <array>
#include "graph_layout.h"
#include "table/ITableRepository.h"

/**
 * @class Читает таблицы rastr напрямую через ITableRepository.
 *
 * Алгоритм и схема таблиц взяты из макроса files/Экспорт в SVG.rbs.
 *
 * Система координат:
 *   SCALE = 10
 *   wx = min(graph_node.k_x),  wy = min(graph_node.k_y) — смещение видового экрана
 *   svg_x = (k_x - wx) * SCALE
 *   svg_y = (k_y - wy) * SCALE
 *
 *   Координаты в rastr (k_x, k_y) — целочисленные индексы ячеек сетки, не метры.
 *   SCALE=10 переводит их в SVG-пиксели с достаточным разрешением для рендера.
 *
 * Возвращаемые данные — структурированный GraphLayout.
 * Рендеринг выполняется во внешнем виджете
 */
class NativeGraphHandler : public QObject
{
    Q_OBJECT
public:
    explicit NativeGraphHandler(ITableRepository* pRepo,
                                QObject* parent = nullptr);

    /**
     * Основной метод построения GraphLayout из таблиц rastr.
     *
     * @param layer        Зарезервировано (в rastr слои не хранятся стандартно).
     * @param bbox         Фильтр видимой области: {x_min,y_min,x_max,y_max} в SVG-координатах.
     *                     nullopt = вернуть всю схему.
     * @param metadataOnly Быстрый путь: только viewBox + voltageStyles без узлов/ветвей.
     * @param autoCenter   Сервер сам вычисляет bbox центра 50% схемы.
     */
    GraphLayout getLayout(
        int                              layer        = 0,
        std::optional<std::array<float,4>> bbox       = std::nullopt,
        bool                             metadataOnly = false,
        bool                             autoCenter   = false
        );

    /**
     * Записывает новую SVG-позицию узла в graph_node.k_x / k_y.
     * Нет оптимизатора — позиция сохраняется точно как указана.
     * После вызова необходимо перезапросить getLayout().
     */
    void moveNode(int nodeId, float x, float y);

    /// Переключает состояние узла:
    ///   "onoffnode"   → sta  0↔1
    ///   "rechecknode" → sel  0↔1
    void toggleNode(int nodeId, const std::string& action);

    /// Переключает состояние ветви:
    ///   "onoffvetv" → sta 0↔1
    void toggleBranch(int branchId, const std::string& action);

private:
    // SCALE: единицы сетки rastr → SVG-пиксели
    static constexpr int SCALE = 10;

    ITableRepository* m_pRepo;

    // ── Вспомогательные методы ────────────────────────────────────────────────

    /// Возвращает HEX-цвет уровня напряжения
    /// При отсутствии точного совпадения выбирает ближайший ключ.
    static std::string voltageColor(int uhom);

    /**
     * Точка подключения ветви к узлу.
     * Алгоритм из макроса: AddLine, начало ветви.
     *
     * @param sx, sy  SVG-координаты узла
     * @param npri    направление шины (>0 горизонталь, <0 вертикаль)
     * @param nprb    смещение точки подключения вдоль шины
     * @param nodType тип узла (-1 = шина, 0-5 = трансформатор)
     */
    static std::pair<float,float> connPoint(
        float sx, float sy, int npri, float nprb, int nodType);

    /**
     * Позиция подписи ветви из таблицы graph_text.
     * Возвращает (x, y, angle_deg).
     * Смещения w_x/w_y хранятся в условных единицах:
     *   32 субпикселя = 1 пиксель → сдвиг на 1 ед.
     */
    static std::tuple<float,float,float> labelPos(
        const QDataBlock& gt, int gtSize, int gtIdx,
        long colWx, long colWy, long colUgol,
        float cx, float cy);

    /// SVG transform-строка для подписи: "translate(x,y)" или "translate(x,y) rotate(a)".
    static std::string buildLabelTransform(
        const QDataBlock& gt, int gtSize, int gtIdx,
        long colWx, long colWy, long colUgol,
        float cx, float cy);

    /// Возвращает пустой layout (1000×1000) — при ошибке или пустой схеме.
    static GraphLayout emptyLayout();

    // ── Типобезопасные читалки из QDataBlock ─────────────────────────────────
    static long        getLong  (const QDataBlock& b, int row, long col);
    static double      getDouble(const QDataBlock& b, int row, long col);
    /// getString + trim пробелов по краям
    static std::string getString(const QDataBlock& b, int row, long col);
};