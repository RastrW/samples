#pragma once

#pragma once
#include <QObject>
#include <optional>
#include <array>
#include "graph_layout.h"
#include "table/ITableRepository.h"

class NativeGraphHandler : public QObject {
    Q_OBJECT
public:
    explicit NativeGraphHandler(ITableRepository* pRepo,
                                QObject* parent = nullptr);

    /// Основной метод построения GraphLayout из таблиц rastr.
    GraphLayout getLayout(
        int layer                                    = 0,
        std::optional<std::array<float,4>> bbox      = std::nullopt,
        bool metadataOnly                            = false,
        bool autoCenter                              = false
        );

    /// Записывает новую SVG-позицию узла в graph_node.
    void moveNode(int nodeId, float x, float y);

    /// Переключает состояние узла (sta / sel).
    void toggleNode(int nodeId, const std::string& action);

    /// Переключает состояние ветви (sta).
    void toggleBranch(int branchId, const std::string& action);

private:
    static constexpr int SCALE = 10;

    ITableRepository* m_pRepo;

    // ── Вспомогательные статические методы ──────────
    static std::string voltageColor(int uhom);
    /// Точка подключения ветви к узлу
    static std::pair<float,float> connPoint(
        float sx, float sy, int npri, float nprb, int nodType);
    /// Позиция подписи ветви из graph_text
    static std::tuple<float,float,float> labelPos(
        const QDataBlock& gt, int gtSize, int gtIdx,
        int colWx, int colWy, int colUgol,
        float cx, float cy);
    /// SVG transform-строка для подписи
    static std::string buildLabelTransform(
        const QDataBlock& gt, int gtSize, int gtIdx,
        int colWx, int colWy, int colUgol,
        float cx, float cy);

    static GraphLayout emptyLayout();

    // ── Типобезопасные читалки из QDataBlock ─────────────────────────────────
    static long   getLong  (const QDataBlock& b, int row, long col);
    static double getDouble(const QDataBlock& b, int row, long col);
    /// getString + trim пробелов по краям (как .strip() в Python)
    static std::string getString(const QDataBlock& b, int row, long col);
};