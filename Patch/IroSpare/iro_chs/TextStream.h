#pragma once

#include "tp_stub.h"
#include "my.h"


class iXmoeTJSTextReadStream
{
public:
	virtual tjs_uint TJS_INTF_METHOD Read(LPWSTR& Buffer, tjs_uint& size) = 0;
	virtual void     TJS_INTF_METHOD Destruct() = 0; // must delete itself
};

iXmoeTJSTextReadStream * XmoeCreateTextStreamForRead(const ttstr & name);
