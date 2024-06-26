/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/
#define _CRT_SECURE_NO_WARNINGS

#include "openfile.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <vector>
#include <map>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#include <windows.h>
#include <wchar.h>

#else
#include <sys/stat.h>
#include <unistd.h>

#endif

namespace open
{

int OpenFile::ReadFile(const std::string& filePath, std::string& buffer, const char* m)
{
    FILE* f = fopen(filePath.c_str(), m);
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer.resize(len);
    int ret = (int)fread((void*)buffer.data(), 1, len, f);
    fclose(f);
    return ret == 0 ? -1 : len;
}

int OpenFile::WriteFile(const std::string& filePath, const std::string& buffer, const char* m)
{
    FILE* f = fopen(filePath.c_str(), m);
    if (!f) return -1;

    int ret = (int)fwrite((void*)buffer.data(), 1, buffer.size(), f);
    fclose(f);
    return ret == 0 ? -1 : (int)buffer.size();
}


std::string OpenFile::JoinPath(const std::string& dirPath, const std::string& fileName)
{
    std::string output;
    if (dirPath.empty()) 
    {
        output.append("./");
    }
    else
    {
        output.append(dirPath);
    }
    if (fileName.empty())
    {
        return output;
    }

    auto c = output.back();
    if (c == '/' || c == '\\')
    {
        if ((*fileName.data() == '/') || (*fileName.data() == '\\')) 
        {
            output.append(fileName.data() + 1);
        }
        else 
        {
            output.append(fileName);
        }
    }
    else
    {
        if (*fileName.data() == '/' || *fileName.data() == '\\') 
        {
            output.append(fileName);
        }
        else 
        {
            output.append("/");
            output.append(fileName);
        }
    }
    return output;
}

std::string OpenFile::GetWriteDirPath()
{
    return "./";
}

bool OpenFile::ListDir(const std::string& filePath, std::vector<std::string>& fileNames)
{
    DIR* pDir = OpenDir(filePath);
    if (!pDir) return false;

    struct dirent* pDirent;
    while ((pDirent = ReadDir(pDir)) != NULL)
    {
        if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0)
            continue;

        fileNames.push_back(pDirent->d_name);
    }
    CloseDir(pDir);
    return true;
}



#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

std::wstring StringMultiByteToWideChar(const std::string& strA)
{
    std::wstring ret;
    if (!strA.empty())
    {
        int nNum = MultiByteToWideChar(CP_ACP, 0, strA.c_str(), -1, NULL, 0);
        if (nNum)
        {
            WCHAR* wideCharString = new WCHAR[nNum + 1];
            wideCharString[0] = 0;
            nNum = MultiByteToWideChar(CP_ACP, 0, strA.c_str(), -1, wideCharString, nNum + 1);
            ret = wideCharString;
            delete[] wideCharString;
        }
        else
        {
            printf("Wrong convert to WideChar code:0x%x \n", GetLastError());
        }
    }
    return ret;
}

std::string StringWideCharToUtf8(const std::wstring& strWideChar)
{
    std::string ret;
    if (!strWideChar.empty())
    {
        int nNum = WideCharToMultiByte(CP_UTF8, 0, strWideChar.c_str(), -1, NULL, 0, NULL, FALSE);
        if (nNum)
        {
            char* utf8String = new char[nNum + 1];
            utf8String[0] = 0;

            nNum = WideCharToMultiByte(CP_UTF8, 0, strWideChar.c_str(), -1, utf8String, nNum + 1, NULL, FALSE);

            ret = utf8String;
            delete[] utf8String;
        }
        else
        {
            printf("Wrong convert to Utf8 code:0x%x \n", GetLastError());
        }
    }
    return ret;
}

std::string StringMultiByteToUtf8(const std::string& strA)
{
    std::wstring wret = StringMultiByteToWideChar(strA);
    return StringWideCharToUtf8(wret);
}

std::wstring StringUtf8ToWideChar(const std::string& strUtf8)
{
    std::wstring ret;
    if (!strUtf8.empty())
    {
        int nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
        if (nNum)
        {
            WCHAR* wideCharString = new WCHAR[nNum + 1];
            wideCharString[0] = 0;
            nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wideCharString, nNum + 1);
            ret = wideCharString;
            delete[] wideCharString;
        }
        else
        {
            printf("Wrong convert to WideChar code:0x%x\n", GetLastError());
        }
    }
    return ret;
}

//static wchar_t* mz_os_unicode_string_create(const char* string, int32_t encoding)
//{
//    wchar_t* string_wide = NULL;
//    uint32_t string_wide_size = 0;
//
//    string_wide_size = MultiByteToWideChar(encoding, 0, string, -1, NULL, 0);
//    if (string_wide_size == 0)
//        return NULL;
//    string_wide = (wchar_t*)malloc((string_wide_size + 1) * sizeof(wchar_t));
//    if (string_wide == NULL)
//        return NULL;
//
//    memset(string_wide, 0, sizeof(wchar_t) * (string_wide_size + 1));
//    MultiByteToWideChar(encoding, 0, string, -1, string_wide, string_wide_size);
//
//    return string_wide;
//}

//static void mz_os_unicode_string_delete(wchar_t** string)
//{
//    if (string != NULL) {
//        free(*string);
//        *string = NULL;
//    }
//}

#define MZ_ENCODING_CODEPAGE_437        (437)
#define MZ_ENCODING_CODEPAGE_932        (932)
#define MZ_ENCODING_CODEPAGE_936        (936)
#define MZ_ENCODING_CODEPAGE_950        (950)
#define MZ_ENCODING_UTF8                (65001)

bool OpenFile::IsExist(const std::string& filePath)
{
    if (filePath.empty())
    {
        return false;
    }
    std::wstring wStrPath = StringUtf8ToWideChar(filePath);
    DWORD attr = GetFileAttributesW(wStrPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    return true;
}

bool OpenFile::IsFile(const std::string& filePath)
{
    if (filePath.empty())
    {
        return false;
    }
    std::wstring wStrPath = StringUtf8ToWideChar(filePath);
    DWORD attr = GetFileAttributesW(wStrPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    if (attr & FILE_ATTRIBUTE_DIRECTORY)
    {
        return false;
    }
    return true;
}

bool OpenFile::IsDir(const std::string& filePath)
{
    if (filePath.empty())
    {
        return false;
    }
    std::wstring wStrPath = StringUtf8ToWideChar(filePath);
    uint32_t attr = GetFileAttributesW(wStrPath.data());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    if (attr & FILE_ATTRIBUTE_DIRECTORY)
    {
        return true;
    }
    return false;
}

bool OpenFile::MakeDir(const std::string& filePath)
{
    if (filePath.empty()) return false;
    if ((filePath[0] != 0) && (filePath.size() <= 3) && (filePath[1] == ':'))
    {
        return IsDir(filePath);
    }
    if (IsDir(filePath))
    {
        return true;
    }
    std::wstring wStrPath = StringUtf8ToWideChar(filePath);
    CreateDirectoryW(wStrPath.data(), NULL);
    if (IsDir(filePath))
    {
        return true;
    }
    return false;
}

typedef struct DIR_int_s {
    void* find_handle;
    WIN32_FIND_DATAW find_data;
    struct dirent    entry;
    uint8_t          end;
} DIR_int;

DIR* OpenFile::OpenDir(const std::string& filePath)
{
    if (filePath.empty())
        return NULL;

    std::string fixedPath = JoinPath(filePath, "*");
    std::wstring wFixedPathh = StringUtf8ToWideChar(fixedPath);

    WIN32_FIND_DATAW find_data;
    void* handle = FindFirstFileW(wFixedPathh.data(), &find_data);
    if (handle == INVALID_HANDLE_VALUE)
        return NULL;

    DIR_int* dir_int = (DIR_int*)malloc(sizeof(DIR_int));
    if (dir_int == NULL)
        return NULL;

    dir_int->find_handle = handle;
    dir_int->end = 0;
    memcpy(&dir_int->find_data, &find_data, sizeof(dir_int->find_data));
    return (DIR*)dir_int;
}

struct dirent* OpenFile::ReadDir(DIR* dir)
{
    if (dir == NULL)
        return NULL;

    DIR_int* dir_int = (DIR_int*)dir;
    if (dir_int->end)
        return NULL;

    WideCharToMultiByte(CP_UTF8, 0, dir_int->find_data.cFileName, -1, dir_int->entry.d_name, sizeof(dir_int->entry.d_name), NULL, NULL);

    if (FindNextFileW(dir_int->find_handle, &dir_int->find_data) == 0) {
        if (GetLastError() != ERROR_NO_MORE_FILES)
            return NULL;

        dir_int->end = 1;
    }
    return &dir_int->entry;
}

int32_t OpenFile::CloseDir(DIR* dir)
{
    if (dir == NULL)
        return -1;

    DIR_int* dir_int = (DIR_int*)dir;
    if (dir_int->find_handle != INVALID_HANDLE_VALUE)
        FindClose(dir_int->find_handle);

    free(dir_int);
    return 0;
}

#include <io.h>
bool OpenFile::Commit(FILE* file)
{
    auto fd = _fileno(file);
    if (_commit(fd) != 0)
    {
        return false;
    }
    return true;
}

#else
#include <sys/types.h>
#include <sys/stat.h>

#if defined(__APPLE__) || defined(__unix__) || defined(__riscos__)
#  include <utime.h>
#  include <unistd.h>
#endif
#if defined(__APPLE__)
#  include <mach/clock.h>
#  include <mach/mach.h>
#endif

bool OpenFile::IsExist(const std::string& filePath)
{
    struct stat st;
    if (stat(filePath.data(), &st) == 0)
    {
        return true;
    }
    return false;
 }

bool OpenFile::IsFile(const std::string& filePath)
{
    struct stat st = { 0 };
    if (stat(filePath.data(), &st) == 0)
    {
        return !S_ISDIR(st.st_mode);
    }
    return false;
}

bool OpenFile::IsDir(const std::string& filePath)
{
    struct stat st = { 0 };
    if (stat(filePath.data(), &st) == 0)
    {
        return S_ISDIR(st.st_mode);
    }
    return false;
}

bool OpenFile::MakeDir(const std::string& filePath)
{
    if (!filePath.empty()) 
    {
        return false;
    }
    if (IsDir(filePath))
    {
        return true;
    }
    int ret = mkdir(filePath.data(), S_IRWXU | S_IRWXG | S_IRWXO);
    if (ret != 0 && (errno != EEXIST))
    {
        return false;
    }
    return IsDir(filePath);
}

DIR* OpenFile::OpenDir(const std::string& filePath)
{
    return opendir(path);
}

struct dirent* OpenFile::ReadDir(DIR* dir)
{
    if (dir == NULL)
        return NULL;

    return readdir(dir);
}

int32_t OpenFile::CloseDir(DIR* dir)
{
    if (dir == NULL)
        return -1;

    if (closedir(dir) == -1)
        return MZ_INTERNAL_ERROR;

    return 0;
}

bool OpenFile::Commit(FILE* file)
{
    auto fd = fileno(file);
    if (fsync(fd) != 0) 
    {
        return false;
    }
    return true;
}

#endif


};