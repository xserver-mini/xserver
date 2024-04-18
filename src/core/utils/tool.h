/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#pragma once

#if '\x01\x02\x03\x04' == 0x01020304
#define LEInt32(__V__) ((((__V__) >> 24) & 0xff) | (((__V__) >> 8) & 0xff00) | (((__V__) << 8) & 0xff0000) | (((__V__) << 24) & 0xff000000))
#elif '\x01\x02\x03\x04' == 0x04030201
#define LEInt32(__N__) (__N__)
#else
#error "WTF? What endian do I meet?"
#endif


//#pragma pack(push, 1)
//
//#pragma pack(pop)