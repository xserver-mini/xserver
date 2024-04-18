/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#pragma once

#include <cassert>
#include <string>
#include <vector>

class XCache
{
    std::vector<std::string> vectCells_;
    XCache():cellSize_(0){ assert(false); }
public:
    XCache(int cellSize);
    virtual ~XCache();
    bool load(const std::string& fileName);
    bool save(const std::string& fileName);

    inline int cellSize() { return cellSize_; }
    inline size_t size() { return vectCells_.size(); }

    template <class T>
    void addCell(const T& cell)
    {
        if (sizeof(T) != cellSize_)
        {
            assert(false);
            return;
        }
        vectCells_.resize(vectCells_.size() + 1);
        vectCells_.back().append((const char*)&cell, sizeof(T));
    }

    template <class T>
    T* getCell(size_t idx)
    {
        if (sizeof(T) != cellSize_)
        {
            assert(false);
            return 0;
        }
        if (idx >= vectCells_.size())
        {
            assert(false);
            return 0;
        }
        return (T*)vectCells_[idx].data();
    }
protected:
    const int cellSize_;
};

