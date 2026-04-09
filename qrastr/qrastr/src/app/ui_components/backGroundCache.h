#pragma once
#include <unordered_map>

class QVariant;

///@class ── Кеш фона (condFormat) ────────────────────────────────────────────────
/// Заполняется в data(BackgroundRole), инвалидируется в slot_DataChanged.
/// QVariant() (invalid) = «формат не нашёлся» — тоже кешируется,
/// чтобы не запускать STRING_BOOL повторно.
struct BackGroundCache {
	// row → col → результат (valid или invalid QVariant)
	std::unordered_map<int, std::unordered_map<int, QVariant>> data;

	void invalidateRows(int from, int to);
	void clear();

	void invalidateColumn(int col);
	const QVariant* get(int row, int col) const;
	void put(int row, int col, QVariant v);
	// Освободить место для вставки строк [first..last]:
	// строки >= first сдвигаются вниз на count позиций.
	void shiftRowsDown(int first, int count);

	// Удалить строки [first..first+count-1] и сдвинуть остальные вверх.
	void shiftRowsUp(int first, int count);
};