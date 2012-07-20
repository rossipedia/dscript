////////////////////////////////////////////////////////////////////////////////
// DScript Scripting Language
// Copyright (C) 2003 Bryan "daerid" Ross
//
// Permission to copy, use, modify, sell and distribute this software is
// granted provided this copyright notice appears in all copies. This
// software is provided "as is" without express or implied warranty, and
// with no claim as to its suitability for any purpose.
////////////////////////////////////////////////////////////////////////////////

#ifndef __DSCRIPT_FLOATTABLE_H__
#define __DSCRIPT_FLOATTABLE_H__

////////////////////////////////////////////////////////////////////////////////
// Standard Library Includes
#include <set>
////////////////////////////////////////////////////////////////////////////////

namespace dscript
{
    /// Maintains a set of doubles in use by the runtime
    class float_table
    {
    public:
        typedef const double* entry;
        entry insert(const double& val);
        entry find(const double& val);
    private:
        std::set<double> m_floats;
    };
}

#endif//__DSCRIPT_FLOATTABLE_H__