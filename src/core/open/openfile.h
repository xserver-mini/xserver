/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#ifndef HEADER_OPEN_FILE_H
#define HEADER_OPEN_FILE_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string>

namespace open
{

struct OpenFile
{
    static int ReadFile(const std::string& filePath, std::string& buffer, const char* m = "rb");
    static int WriteFile(const std::string& filePath, const std::string& buffer, const char* m = "wb");
    static std::string JoinPath(const std::string& dirPath, const std::string& fileName);
    static std::string GetWriteDirPath();

    static bool IsExist(const std::string& filePath);
    static bool IsFile(const std::string& filePath);
    static bool IsDir(const std::string& filePath);
    static bool MakeDir(const std::string& filePath);
    static bool Commit(FILE* file);
};

};

#endif //HEADER_OPEN_FILE_H