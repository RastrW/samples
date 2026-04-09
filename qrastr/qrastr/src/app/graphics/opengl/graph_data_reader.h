#pragma once
#include "QDataBlocks.h"

// вспомогательный класс для удобного доступа к QDataBlock
class BlockView {
public:
    BlockView(QDataBlock& block, const std::vector<std::string>& colNames) {
        for (int i = 0; i < (int)colNames.size(); i++)
            m_idx[colNames[i]] = i;
        m_block = &block;
    }

    long   getLong  (int row, const std::string& col) const {
        return std::visit(ToLong{}, m_block->Get(row, m_idx.at(col)));
    }
    double getDouble(int row, const std::string& col) const {
        return std::visit(ToDouble{}, m_block->Get(row, m_idx.at(col)));
    }
    std::string getString(int row, const std::string& col) const {
        return std::visit(ToString{}, m_block->Get(row, m_idx.at(col)));
    }
    int size() const { return m_block->RowsCount(); }

private:
    QDataBlock* m_block;
    std::unordered_map<std::string, int> m_idx;
};