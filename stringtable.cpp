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
#include "stringtable.h"
////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace dscript;

string_table::entry string_table::insert(const string& val)
{
    typedef set<string>::const_iterator iter_t;
    typedef pair<iter_t,bool> ret_t;
    ret_t r = m_strings.insert(val);
    return r.first->c_str();
}

string_table::entry string_table::find(const string& val)
{
    typedef set<string>::const_iterator c_iter;
    c_iter found = m_strings.find(val);
    if(found == m_strings.end())
        return 0;
    else
        return found->c_str();
}

string dscript::escape(const string& str)
{
    typedef string::const_iterator iter_t;
    iter_t st = str.begin(); // skip beginning "
    iter_t en = str.end(); // skip ending "
    // not the fastest way of doing it, but it works
    string res;
    for(; st != en; ++st)
    {
        switch(*st)
        {
        case '\n':
            res.append("\\n");
            break;
        case '\t':
            res.append("\\t");
            break;
        case '\r':
            res.append("\\r");
            break;
        case '\f':
            res.append("\\f");
            break;
        case '"':
            res.append("\\\"");
            break;
        default:
            res.push_back(*st);
        }
    }
    return res;
}

string dscript::unescape(const string& str)
{
    typedef string::const_iterator iter_t;
    iter_t st = str.begin() + 1; // skip beginning "
    iter_t en = str.end() - 1; // skip ending "
    // not the fastest way of doing it, but it works
    string res;
    for(; st != en; ++st)
    {
        if(*st == '\\')
        {
            // check next
            ++st;
            switch(*st)
            {
            case 'n':
                res.push_back('\n');
                break;
            case 't':
                res.push_back('\t');
                break;
            case 'r':
                res.push_back('\r');
                break;
            case 'f':
                res.push_back('\f');
                break;
            case '"':
                res.push_back('"');
                break;
            default:
                res.push_back(*st);
            }
        }
        else
            res.push_back(*st);
    }
    return res;
}