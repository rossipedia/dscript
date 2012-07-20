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
// DScript Headers
#include "functions.h"
////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace dscript;

func_table::entry* func_table::find(string_table::entry name)
{
    func_map::iterator found = functions.find(name);
    if(found == functions.end())
        return 0;
    else
        return &(found->second);
}

void func_table::add_script_func
(
    string_table::entry name,
    instr_iter begin,
    instr_iter start,
    instr_iter end
)
{
    entry& e = functions[name];
    e.begin = begin;
    e.start = start;
    e.end = end;
    e.is_host = false;
    e.name = name;
}

void func_table::add_host_func
(
    string_table::entry name,
    host_function_t callback
)
{
    add_host_func(name,callback,-1,-1,"");
}

void func_table::add_host_func
(
    string_table::entry name,
    host_function_t callback,
    int minargs,
    int maxargs,
    const char* usage
)
{
    entry& e = functions[name];
    e.is_host = true;
    e.name = name;
    e.host_func = callback;
    e.min_args = minargs;
    e.max_args = maxargs;
    if(usage != 0)
        e.usage_string = usage;
}

void func_table::remove_host_func(
    string_table::entry name
    )
{
    // ok, find it
    func_map::iterator f = functions.find(name);
    if(f != functions.end())
    {
        if(f->second.is_host)
            functions.erase(f);
    }
}