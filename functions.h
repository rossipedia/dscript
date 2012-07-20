////////////////////////////////////////////////////////////////////////////////
// DScript Scripting Language
// Copyright (C) 2003 Bryan "daerid" Ross
//
// Permission to copy, use, modify, sell and distribute this software is
// granted provided this copyright notice appears in all copies. This
// software is provided "as is" without express or implied warranty, and
// with no claim as to its suitability for any purpose.
////////////////////////////////////////////////////////////////////////////////

#ifndef __DSCRIPT_FUNCTIONS_H__
#define __DSCRIPT_FUNCTIONS_H__

////////////////////////////////////////////////////////////////////////////////
// Standard Library Includes
#include <vector>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DScript Includes
#include "value.h"
////////////////////////////////////////////////////////////////////////////////

namespace dscript
{
    /// Defines the type of argument list
    /// to be passed to host functions
    typedef std::vector<value> args_t;

    /// Defines a pointer to a function that may
    /// be called from script.
    typedef void (*host_function_t)(
        const args_t& args,
        class context& ctx
        );

    /// Maintains a List of all currently defined functions
    class func_table
    {
    public:
        struct entry
        {
            string_table::entry name;
            bool is_host;
            instr_iter begin;
            instr_iter start;
            instr_iter end;
            host_function_t host_func;
            int min_args;
            int max_args;
            std::string usage_string;
        };
    private:
        typedef std::map<
            string_table::entry,
            func_table::entry,
            cmp_ste
        > func_map;
    public:

        entry* find(string_table::entry name);        
 
        void add_script_func(
            string_table::entry name,
            instr_iter begin,
            instr_iter start,
            instr_iter end
            );

        void add_host_func(
            string_table::entry name,
            host_function_t callback
            );

        void add_host_func(
            string_table::entry name,
            host_function_t callback,
            int minargs,
            int maxargs,
            const char* usage
            );

        void remove_host_func(
            string_table::entry name
            );
    private:
        func_map functions;
    };
}

#endif//__DSCRIPT_FUNCTIONS_H__