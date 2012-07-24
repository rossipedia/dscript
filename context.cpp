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
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <sstream>
#include <fstream>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DScript Includes
#include "context.h"
#include "compiler.h"
////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace dscript;

context::context() : log_out(0)
{
}

void context::enable_logging(ostream* out)
{
    log_out = out;
}

void context::disable_logging()
{
    log_out = 0;
}

void context::log_msg(const string& message)
{
    if(log_out != 0)
        *log_out << message << endl;
}

void context::set_return(const value& val)
{
    runtime.m_return_val = val;
}

void context::link_function(const char* name,host_function_t callback)
{
    link_function(name,callback,-1,-1,0);
}

void context::link_function(
    const char* name,
    host_function_t callback,
    int minargs,
    int maxargs,
    const char* usage
)
{
    string_table::entry ste = runtime.strings.insert(name);
    if(callback == 0)
        // remove it
        runtime.functions.remove_host_func(ste);
    else
        runtime.functions.add_host_func(
            ste,
            callback,
            minargs,
            maxargs,
            usage
            );
}

value context::get_global(const string& name)
{
    if(name.length() > 0 && name[0] == '$')
        return runtime.globals[runtime.strings.insert(name)];
    else
        return value();
}

void context::set_global(const string& name,const value& val)
{
    if(name.length() > 0 && name[0] == '$')
        runtime.globals[runtime.strings.insert(name)] = val;
}

value context::get_local(const string& name)
{
    if(
        runtime.m_callstack.size() > 0 &&
        name.length() > 0 &&
        name[0] == '%'
        )
    {
        return
            runtime.m_callstack.top()[
                runtime.strings.insert(
                    name
                    )
            ];
    }
    else
        return value();

}

void context::set_local(const string& name, const value& val)
{
        if(
        runtime.m_callstack.size() > 0 &&
        name.length() > 0 &&
        name[0] == '%'
        )
    {
            runtime.m_callstack.top()[
                runtime.strings.insert(
                    name
                    )
            ] = val;
    }
}

value context::call(const string& func)
{
    args_t args;
    return call(func,args);
}

value context::call(const string& func,const args_t& args)
{
    runtime.m_return_val.clear();
    runtime.m_param_stack.clear();

    func_table::entry* e =
        runtime.functions.find(
            runtime.strings.insert(func)
            );

    if(e == 0)
        log_msg(func + ": function not found");
    else if(e->is_host)
        (*e->host_func)(args,*this);
    else
    {
        // copy the args to the param stack (backwards)!
        runtime.m_param_stack.assign(
            args.begin(),
            args.end()
            );
        reverse(
            runtime.m_param_stack.begin(),
            runtime.m_param_stack.end()
            );

        // run the script function
        runtime.execute(
            e->begin,
            e->end,
            e->start,
            *this
            );
    }
    return runtime.m_return_val;
}

void dump_asm(const codeblock_t& codeblock,ostream& out)
{
    // dump the op_name
    for(size_t off = 0; off < codeblock.size(); ++off)
    {
        // output the offset
        out << setw(5) << setfill('0') << off << ':';
        // output the name
        op_code op = codeblock[off].get_op_code();
        // output the integer val
        unsigned int v = codeblock[off].get_int();
        out << '('<< v << ") " << get_op_name(op) << endl;
        switch(codeblock[off].get_op_code())
        {
        case op_call_func:
        case op_push_var:
        case op_inc_var:
        case op_dec_var:
        case op_pop_param:
        case op_assign_var:
        case op_mul_asn_var:
        case op_div_asn_var:
        case op_mod_asn_var:
        case op_add_asn_var:
        case op_sub_asn_var:
        case op_cat_asn_var:
        case op_band_asn_var:
        case op_bor_asn_var:
        case op_bxor_asn_var:
        case op_shl_asn_var:
        case op_shr_asn_var:
            {
                ++off;
                // output the offset
                out << setw(5) << setfill('0') << off << ':';
                // out the string
                out << codeblock[off].get_str() << endl;
            }
            break;
        case op_push_str:
            {
                ++off;
                // output the offset
                out << setw(5) << setfill('0') << off << ':';
                // output the escaped string
                out << '"' << escape(codeblock[off].get_str()) << '"' << endl;
            }
            break;
        case op_push_float:
            {
                ++off;
                // output the offset
                out << setw(5) << setfill('0') << off << ':';
                // output the float value
                out << *(codeblock[off].get_flt()) << endl;
            }
            break;
        case op_push_int:
            {
                ++off;
                // output the offset
                out << setw(5) << setfill('0') << off << ':';
                // out the int value
                out << codeblock[off].get_int() << endl;
            }
            break;
        case op_jmp_false:
        case op_jmp:
            {
                ++off;
                // output the offset
                out << setw(5) << setfill('0') << off << ':';
                // out the jump to offset
                out << codeblock[off].get_offset() << endl;
            }
            break;
        case op_decl_func:
            {
                ++off;
                // output the offset
                out << setw(5) << setfill('0') << off << ':';
                // out the function name
                out << codeblock[off].get_str() << endl;

                ++off;
                // output the offset
                out << setw(5) << setfill('0') << off << ':';
                // out the function end offset
                out << codeblock[off].get_offset() << endl;
            }
            break;
        }
     }
}

void context::dump_code(std::ostream& out,const std::string& code)
{
    try
    {
        string_table strings;
        float_table floats;
        codeblock_t codeblock = dscript::compile(code,strings,floats);
        dump_asm(codeblock,out);
    }
    catch(compiler_error& ce)
    {
        if(log_out != 0)
        {
            stringstream msg;
            msg << "Compiler Error: " << ce.what() << endl;
            msg << "At: " << ce.pos.line << ":" << ce.pos.col << endl;
            log_msg(msg.str());
        }
    }
}

void context::dump_file(std::ostream& out,const std::string& file)
{
    ifstream infile(file.c_str());
    if(!infile)
    {
        log_msg(file + " could not be opened.");
        return;
    }
    infile >> noskipws;
    string code_str;
    code_str.assign(
        istream_iterator<char>(infile),
        istream_iterator<char>()
        );
    dump_code(out,code_str);
}

bool context::eval(const std::string& code)
{
    try
    {
        codeblock_t& codeblock = codeblocks[code];
        codeblock = dscript::compile(code,runtime.strings,runtime.floats);
        runtime.execute(
            codeblock.begin(),
            codeblock.end(),
            codeblock.begin(),
            *this
            );
        return true;
    }
    catch(compiler_error& ce)
    {
        if(log_out != 0)
        {
            stringstream msg;
            msg << "Compiler Error: " << ce.what() << endl;
            msg << "At: " << ce.pos.line << ":" << ce.pos.col << endl;
            log_msg(msg.str());
        }
        return false;
    }
}

bool context::exec(const std::string& file)
{
    ifstream infile(file.c_str());
    if(!infile)
    {
        log_msg(file + " could not be opened.");
        return false;
    }
    infile >> noskipws;
    string code_str;
    code_str.assign(
        istream_iterator<char>(infile),
        istream_iterator<char>()
        );
    try
    {
        codeblock_t& codeblock = codeblocks[file];
        codeblock = dscript::compile(code_str,runtime.strings,runtime.floats);
        runtime.execute(
            codeblock.begin(),
            codeblock.end(),
            codeblock.begin(),
            *this
            );
        return true;
    }
    catch(compiler_error& ce)
    {
        if(log_out != 0)
        {
            stringstream msg;
            msg << "Compiler Error: " << ce.what() << endl;
            msg << "At: " << ce.pos.line << ":" << ce.pos.col << endl;
            log_msg(msg.str());
        }
        return false;
    }
}

bool context::exec_compiled(const std::string& file)
{
    string comp_file = file + ".dsc";
    ifstream infile(comp_file.c_str());
    if(!infile)
    {
        log_msg(file + " could not be opened.");
        return false;
    }
    try
    {
        codeblock_t& code = codeblocks[file];
        code = load_compiled_file(comp_file,runtime.strings,runtime.floats);
        runtime.execute(
            code.begin(),
            code.end(),
            code.begin(),
            *this
            );
        return true;
    }
    catch(std::runtime_error& e)
    {
        if(log_out != 0)
            log_msg(e.what());
        return false;
    }
}

bool context::compile(const std::string& file)
{
    ifstream infile(file.c_str());
    if(!infile)
    {
        log_msg(file + " could not be opened.");
        return false;
    }
    infile >> noskipws;
    string code_str;
    code_str.assign(
        istream_iterator<char>(infile),
        istream_iterator<char>()
        );
    try
    {
        save_codeblock(
            file + ".dsc",
            dscript::compile(code_str,runtime.strings,runtime.floats)
            );
        return true;
    }
    catch(compiler_error& ce)
    {
        if(log_out != 0)
        {
            stringstream msg;
            msg << "Compiler Error: " << ce.what() << endl;
            msg << "At: " << ce.pos.line << ":" << ce.pos.col;
            log_msg(msg.str());
        }
        return false;
    }
    catch(runtime_error& e)
    {
        log_msg(e.what());
        return false;
    }
}
