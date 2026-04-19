#include "backgroundCache.h"
#include <QVariant>

void BackgroundCache::invalidateRows(int from, int to) {
	for (int r = from; r <= to; ++r) data.erase(r);
}

void BackgroundCache::clear() { data.clear(); }

void BackgroundCache::invalidateColumn(int col) {
	for (auto& [row, cols] : data)
		cols.erase(col);
}

const QVariant* 
BackgroundCache::get(int row, int col) const {
	auto it = data.find(row);
	if (it == data.end()) return nullptr;
	auto jt = it->second.find(col);
	return (jt != it->second.end()) ? &jt->second : nullptr;
}
void 
BackgroundCache::put(int row, int col, QVariant v) {
	data[row][col] = std::move(v);
}

void BackgroundCache::shiftRowsDown(int first, int count) {
	std::unordered_map<int, std::unordered_map<int, QVariant>> shifted;
	for (auto& [row, cols] : data) {
		const int newRow = (row >= first) ? row + count : row;
		shifted[newRow] = std::move(cols);
	}
	data = std::move(shifted);
}

void BackgroundCache::shiftRowsUp(int first, int count) {
	std::unordered_map<int, std::unordered_map<int, QVariant>> shifted;
	for (auto& [row, cols] : data) {
		if (row >= first && row < first + count) continue; // удалённые
		const int newRow = (row >= first + count) ? row - count : row;
		shifted[newRow] = std::move(cols);
	}
	data = std::move(shifted);
}