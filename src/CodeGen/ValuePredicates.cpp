#include "stdafx.h"

#include "NativeJIT/ValuePredicates.h"


namespace NativeJIT
{
    unsigned Size(unsigned __int8 value)
    {
        return (value == 0) ? 0 : 1;
    }


    unsigned Size(unsigned __int16 value)
    {
        return (value == 0) ? 0 : ((value <= 0xff) ? 1 : 2);
    }


    unsigned Size(unsigned __int32 value)
    {
        return (value == 0) ? 0 : ((value <= 0xff) ? 1 : ((value <= 0xffff) ? 2 : 4));
    }


    unsigned Size(unsigned __int64 value)
    {
        return (value == 0) ? 0 : ((value <= 0xff)? 1 : ((value <= 0xffff) ? 2 : ((value <= 0xffffffff) ? 4 : 8)));
    }


    unsigned Size(__int8 value)
    {
        return (value == 0) ? 0 : 1;
    }


    unsigned Size(__int16 value)
    {
        return (value == 0) ? 0 : ((value >= -0x80 && value <= 0x7f)? 1 : 2);
    }


    unsigned Size(__int32 value)
    {
        return (value == 0)? 0 : ((value >= -0x80 && value <= 0x7f)? 1 : ((value >= -0x8000 && value <= 0x7fff) ? 2 : 4));
    }


    unsigned Size(__int64 value)
    {
        return (value == 0) ? 0 : ((value >= -0x80LL && value <= 0x7fLL)? 1 : ((value >= -0x8000LL && value <= 0x7fffLL) ? 2 : ((value >= -0x80000000LL && value <= 0x7fffffffLL) ? 4 : 8)));
    }
}