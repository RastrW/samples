#pragma once
#include <unordered_map>
#include "table/tableIndexTypes.h"
#include "table/tableIndexHash.h"

class QVariant;

///@class ── Кеш фона (condFormat) ────────────────────────────────────────────────
/// Заполняется в data(BackgroundRole), инвалидируется в slot_DataChanged.
/// QVariant() (invalid) = «формат не нашёлся» — тоже кешируется,
/// чтобы не запускать STRING_BOOL повторно.
struct BackgroundCache {
	// row → col → результат (valid или invalid QVariant)
    std::unordered_map<int, std::unordered_map<ModelColumn, QVariant>> data;

	void invalidateRows(int from, int to);
	void clear();

    void invalidateColumn(ModelColumn col);
    const QVariant* get(int row, ModelColumn col) const;
    void put(int row, ModelColumn col, QVariant v);
	// Освободить место для вставки строк [first..last]:
	// строки >= first сдвигаются вниз на count позиций.
	void shiftRowsDown(int first, int count);

	// Удалить строки [first..first+count-1] и сдвинуть остальные вверх.
	void shiftRowsUp(int first, int count);
};