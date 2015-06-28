//-----------------------------------------------------------------------------
// luna2d engine
// Copyright 2014-2015 Stepan Prokofjev
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

#include "sqobject.h"

namespace luna2d{

class SqTable : public SqObject
{
	friend struct SqStack<SqTable>;

public:
	SqTable(SqVm* vm);
	SqTable(HSQUIRRELVM vm);
	SqTable(const SqTable& fn);

private:
	SqTable(const std::shared_ptr<SqRef>& ref);

public:
	SqTable& operator=(const SqTable& fn);
};


template<>
struct SqStack<SqTable>
{
	inline static void Push(HSQUIRRELVM vm, const SqTable& fn)
	{
		SqStack<std::shared_ptr<SqRef>>::Push(vm, fn.GetRef());
	}

	inline static SqTable Get(HSQUIRRELVM vm, int index = -1)
	{
		if(sq_gettype(vm, index) != OT_TABLE) return SqTable(vm);
		return SqTable(SqStack<std::shared_ptr<SqRef>>::Get(vm, index));
	}
};

}