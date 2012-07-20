////////////////////////////////////////////////////////////////////////////////
// DScript Scripting Language
// Copyright (C) 2003 Bryan "daerid" Ross
//
// Permission to copy, use, modify, sell and distribute this software is
// granted provided this copyright notice appears in all copies. This
// software is provided "as is" without express or implied warranty, and
// with no claim as to its suitability for any purpose.
////////////////////////////////////////////////////////////////////////////////

#ifndef __DSCRIPT_COMPILER_H__
#define __DSCRIPT_COMPILER_H__

////////////////////////////////////////////////////////////////////////////////
// Standard Library Include Files
#include <string>
#include <stdexcept>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DScript Include Files
#include "instruction.h"
#include "stringtable.h"
#include "floattable.h"
////////////////////////////////////////////////////////////////////////////////

namespace dscript
{
    /// Compile a string of dscript code into a codeblock
    codeblock_t compile(
        const std::string& code,
        string_table& strings,
        float_table& floats
        );

    /// Saves a compiled codeblock to a binary file, for faster loading times
    void save_codeblock(
        const std::string& filename,
        const codeblock_t& code
        );

    /// Loads a compiled file saved with save_codeblock() into a codeblock,
    /// which can be executed by the vmachine
    codeblock_t load_compiled_file(
        const std::string& filename,
        string_table& strings,
        float_table& floats
        );

    /// This is a support structure for tracking parse and compile
    /// errors, using exceptions.
    struct code_position
    {
        code_position(int l,int c) : line(l), col(c) {}
        code_position(const code_position& other) 
            : line(other.line), col(other.col)
        {}
        code_position& operator = (const code_position& other)
        {
            line = other.line;
            col = other.col;
        }
        int line;
        int col;
    };

    /// This is thrown when the compiler has encountered a semantic error
    /// This is not touched by the actual parser
    struct compiler_error : public std::runtime_error
    {
        code_position pos;
        compiler_error(const std::string& msg,const code_position& p) 
            : runtime_error(msg), pos(p)
        {}
    };
}

#endif//__DSCRIPT_COMPILER_H__