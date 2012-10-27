////////////////////////////////////////////////////////////////////////////////
// DScript Scripting Language
// Copyright (C) 2003 Bryan "daerid" Ross
//
// Permission to copy, use, modify, sell and distribute this software is
// granted provided this copyright notice appears in all copies. This
// software is provided "as is" without express or implied warranty, and
// with no claim as to its suitability for any purpose.
////////////////////////////////////////////////////////////////////////////////

#ifndef __DSCRIPT_INSTRUCTION_H__
#define __DSCRIPT_INSTRUCTION_H__

////////////////////////////////////////////////////////////////////////////////
// Standard Library Includes
#include <vector>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DScript Includes
#include "opcodes.h"
#include "stringtable.h"
#include "floattable.h"
////////////////////////////////////////////////////////////////////////////////

namespace dscript
{
    class instruction
    {
    public:
        instruction() { data.opcode = op_invalid; }
        instruction(const instruction& other) { data = other.data; }
        instruction(op_code op) { data.opcode = op; }
        instruction(string_table::entry ste) { data.strval = ste; }
        instruction(float_table::entry fte) { data.fltval = fte; }
        instruction(int i) { data.intval = i; }
        instruction(size_t off) { data.offset = off; }

        op_code get_op_code() const { return data.opcode; }
        string_table::entry get_str() const { return data.strval; }
        float_table::entry get_flt() const { return data.fltval; }
        int get_int() const { return data.intval; }
        size_t get_offset() const { return data.offset; }

    private:
        union {
            op_code opcode;
            string_table::entry strval;
            float_table::entry fltval;
            int intval;
            size_t offset;
        } data;
    };

    typedef std::vector<instruction> codeblock_t;
    typedef codeblock_t::const_iterator instr_iter;
}

#endif//__DSCRIPT_INSTRUCTION_H__
