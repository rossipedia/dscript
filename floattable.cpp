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
// DScript Includes
#include "floattable.h"
////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace dscript;

float_table::entry float_table::insert(const double& val)
{
    typedef set<double>::const_iterator iter_t;
    typedef pair<iter_t,bool> ret_t;
    ret_t r = m_floats.insert(val);
    return &(*(r.first));
}

float_table::entry float_table::find(const double& val)
{
    typedef set<double>::const_iterator c_iter;
    c_iter found = m_floats.find(val);
    if(found == m_floats.end())
        return 0;
    else
        return &(*(found));
}