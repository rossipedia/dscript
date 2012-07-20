////////////////////////////////////////////////////////////////////////////////
// DScript Scripting Language
// Copyright (C) 2003 Bryan "daerid" Ross
//
// Permission to copy, use, modify, sell and distribute this software is
// granted provided this copyright notice appears in all copies. This
// software is provided "as is" without express or implied warranty, and
// with no claim as to its suitability for any purpose.
////////////////////////////////////////////////////////////////////////////////

#ifndef __DSCRIPT_STRINGTABLE_H__
#define __DSCRIPT_STRINGTABLE_H__

////////////////////////////////////////////////////////////////////////////////
// Standard Library Includes
#include <string>
#include <set>
////////////////////////////////////////////////////////////////////////////////

namespace dscript
{
    /// Maintains a list of strings in use by the runtime
    class string_table
    {
    public:
        typedef const char* entry;
        entry insert(const std::string& val);
        entry find(const std::string& val);
    private:
        std::set<std::string> m_strings;
    };

    /// Used to compare two string_table::entries
    struct cmp_ste
    {
        bool operator()(string_table::entry left,string_table::entry right) const
        {
            return stricmp(left,right) < 0;
        }
    };

    std::string escape(const std::string& str);
    std::string unescape(const std::string& str);
}

#endif//__DSCRIPT_STRINGTABLE_H__