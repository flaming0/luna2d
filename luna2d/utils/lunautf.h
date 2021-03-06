//-----------------------------------------------------------------------------
// luna2d engine
// Copyright 2014-2016 Stepan Prokofjev
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#pragma once

#include "lunaengine.h"

//-------------------------------------
// Helpers for work with UTF-8 encoding
//-------------------------------------
namespace luna2d{ namespace utf{

std::u32string ToUtf32(const std::string& string); // Convert UTF-8 string to UTF-32 string
std::string FromUtf32(const std::u32string& string); // Convert UTF-32 string to UTF-8 string

}}


//--------------------------------------------------------------------------------
// WP8.1 toolset don't support unicode literals
// So, for UTF32 strings, "LUNA_UTF32" macro should be used instead of "U" literal
//--------------------------------------------------------------------------------
#if LUNA_PLATFORM == LUNA_PLATFORM_WP
	#define LUNA_UTF32(str) luna2d::utf::ToUtf32(str)
#else
	#define LUNA_UTF32(str) U##str
#endif
