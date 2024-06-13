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
#include <vector>

#if defined(_WIN32)
struct dirent {
    char d_name[256];
};
typedef void* DIR;
#else
#include <dirent.h>
#endif
#include <stdint.h>

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

    static DIR* OpenDir(const std::string& filePath);
    static struct dirent* ReadDir(DIR* dir);
    static int32_t CloseDir(DIR* dir);
    static bool ListDir(const std::string& filePath, std::vector<std::string>& fileNames);

};

};

#endif //HEADER_OPEN_FILE_H