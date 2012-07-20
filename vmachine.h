////////////////////////////////////////////////////////////////////////////////
// DScript Scripting Language
// Copyright (C) 2003 Bryan "daerid" Ross
//
// Permission to copy, use, modify, sell and distribute this software is
// granted provided this copyright notice appears in all copies. This
// software is provided "as is" without express or implied warranty, and
// with no claim as to its suitability for any purpose.
////////////////////////////////////////////////////////////////////////////////

#ifndef __DSCRIPT_VMACHINE_H__
#define __DSCRIPT_VMACHINE_H__

////////////////////////////////////////////////////////////////////////////////
// Standard Library Includes
#include <stack>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DScript Includes
#include "instruction.h"
#include "value.h"
#include "functions.h"
////////////////////////////////////////////////////////////////////////////////

namespace dscript
{
    /// The virtual machine. This object takes care of the actual execution
    /// of the bytecode contained in a codeblock
    class vmachine
    {
    public:
        void execute(
            instr_iter begin,
            instr_iter end,
            instr_iter instr,
            class context& ctx
            );

        friend class context;
    private:
        
        // the stack frames
        std::stack<dictionary_t> m_callstack;
        
        // the runtime stack
        std::stack<value> m_runtime_stack;
        dictionary_t globals;
        
        // the return register
        value m_return_val;
        
        // the function call param stack
        std::vector<value> m_param_stack;
        
        // string table
        string_table strings;
        
        // double table
        float_table floats;
        
        // the function table
        func_table functions;
    };
}

#endif//__DSCRIPT_VMACHINE_H__