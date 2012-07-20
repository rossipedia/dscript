////////////////////////////////////////////////////////////////////////////////
// DScript Scripting Language
// Copyright (C) 2003 Bryan "daerid" Ross
//
// Permission to copy, use, modify, sell and distribute this software is
// granted provided this copyright notice appears in all copies. This
// software is provided "as is" without express or implied warranty, and
// with no claim as to its suitability for any purpose.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Standard Library Includes
#include <sstream>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DScript Includes
#include "value.h"
////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace dscript;

value::value() 
    : strval(""), intval(0), fltval(0), type(type_str)
{
}

value::value(int i) 
    : intval(i), type(type_int) 
{
}

value::value(double d)
 : fltval(d), type(type_flt)
{
}

value::value(const string& s)
 : strval(s), type(type_str)
{
}

value::value(string_table::entry s)
 : strval(s), type(type_str)
{
}

value::value(const value& other)
     : type(other.type), 
     strval(other.strval), 
     intval(other.intval), 
     fltval(other.fltval)
{
}

value& value::operator = (const string& s)
{
    type = type_str;
    strval = s;
    return *this;
}

value& value::operator = (string_table::entry s)
{
    type = type_str;
    strval = s;
    return *this;
}

value& value::operator = (int i)
{
    type = type_int;
    intval = i;
    return *this;
}

value& value::operator = (double d)
{
    type = type_flt;
    fltval = d;
    return *this;
}

value& value::operator =(const value& other)
{
    type = other.type;
    intval = other.intval;
    strval = other.strval;
    fltval = other.fltval;
    return *this;
}

string value::to_str() const 
{
    switch(type)
    {
    case type_int:
        {
            stringstream s;
            s << intval;
            return s.str();
        }
        break;
    case type_flt:
        {
            stringstream s;
            s << fltval;
            return s.str();
        }
        break;
    default:
        return strval;
    }
}

int value::to_int() const
{
    switch(type)
    {
    case type_str:
        {
            int i = 0;
            stringstream s;
            s << strval;
            s >> i;
            return i;
        }
        break;
    case type_flt:
        return (int)fltval;
        break;
    default:
        return intval;
    }
}

double value::to_flt() const
{
    switch(type)
    {
    case type_str:
        {
            double d = 0.0;
            stringstream s;
            s << strval;
            s >> d;
            return d;
        }
        break;
    case type_int:
        return (double)intval;
        break;
    default:
        return fltval;
    }
}

void value::set_type(value::ty new_type)
{
    switch(new_type)
    {
    case type_str:
        strval = to_str();
        break;
    case type_int:
        intval = to_int();
        break;
    case type_flt:
        fltval = to_flt();
        break;
    }
    type = new_type;
}
