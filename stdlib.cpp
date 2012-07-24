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
//
// This implementation of a standard library is far from complete.
// Feel free to contribute any functions you believe should be in here.
// Email contributions to: me@daerid.com
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Standard Library Includes
#include <cstring>
#include <cstdio>
#include <cmath>
#include <iostream>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Boost Includes
#include <boost/scoped_array.hpp>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DScript Includes
#include "stdlib.h"
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Handle some macros
#ifdef feof
#undef feof
#endif//feof
////////////////////////////////////////////////////////////////////////////////

#define ARGS const dscript::args_t& args,dscript::context& ctx

using namespace std;

namespace dscript
{

namespace stdlib
{
    // String Functions
    void strcmp(ARGS)
    {
        ctx.set_return(
            ::strcmp(
                args[0].to_str().c_str(),
                args[1].to_str().c_str()
                )
            );
    }
    void stricmp(ARGS)
    {
        ctx.set_return(
            ::strcmp(
                args[0].to_str().c_str(),
                args[1].to_str().c_str()
                )
            );
    }
    void strncmp(ARGS)
    {
        ctx.set_return(
            ::strncmp(
                args[0].to_str().c_str(),
                args[1].to_str().c_str(),
                args[2].to_int()
                )
            );
    }

    void strnicmp(ARGS)
    {
        ctx.set_return(
            ::_strnicmp(
                args[0].to_str().c_str(),
                args[1].to_str().c_str(),
                args[2].to_int()
                )
            );
    }

    void substr(ARGS)
    {
        size_t en = (args.size() == 2) ? string::npos : args[2].to_int();
        ctx.set_return(
            args[0].to_str().substr(
                args[1].to_int(),
                en
                )
            );
    }

    // Math functions
    void sqrt(ARGS)
    {
        ctx.set_return(
            ::sqrt(
                args[0].to_flt()
                )
            );
    }

    void sin(ARGS)
    {
        ctx.set_return(
            ::sin(
                args[0].to_flt()
                )
            );
    }

    void cos(ARGS)
    {
        ctx.set_return(
            ::cos(
                args[0].to_flt()
                )
            );
    }

    void tan(ARGS)
    {
        ctx.set_return(
            ::tan(
                args[0].to_flt()
                )
            );
    }

    void asin(ARGS)
    {
        ctx.set_return(
            ::atan(
                args[0].to_flt()
                )
            );
    }

    void acos(ARGS)
    {
        ctx.set_return(
            ::acos(
                args[0].to_flt()
                )
            );
    }

    void atan(ARGS)
    {
        ctx.set_return(
            ::atan(
                args[0].to_flt()
                )
            );
    }

    void pow(ARGS)
    {
        ctx.set_return(
            ::pow(
                args[0].to_flt(),
                args[1].to_flt()
                )
            );
    }

    


    // IO functions
    void print(ARGS)
    {
        args_t::const_iterator a = args.begin();
        args_t::const_iterator e = args.end();
        for(; a != e; ++a)
            cout << *a << flush;
    }

    void readln(ARGS)
    {
        // print any prompts
        print(args,ctx);
        string ln;
        getline(cin,ln);
        ctx.set_return(
            ln
            );
    }

    void fopen(ARGS)
    {
        // open the file
        FILE* fp = ::fopen(args[0].to_str().c_str(),args[1].to_str().c_str());
        ctx.set_return(*(reinterpret_cast<int*>(&fp)));
    }

    void fclose(ARGS)
    {
        // close the file
        int a = args[0].to_int();
        FILE* fp = *(reinterpret_cast<FILE**>(&a));
        ctx.set_return(
            ::fclose(fp)
            );
    }

    void fgets(ARGS)
    {
        int a = args[0].to_int();
        FILE* fp = *(reinterpret_cast<FILE**>(&a));
        int len = args[1].to_int();
        boost::scoped_array<char> buffer(new char[len + 1]);
        char* read = ::fgets(buffer.get(),len,fp);
        if(read == NULL)
        {
            ctx.log_msg("Error reading from file.");
            ctx.set_return(0);
        }
        else
            ctx.set_return(read);
    }

    void fputs(ARGS)
    {
        int a = args[0].to_int();
        FILE* fp = *(reinterpret_cast<FILE**>(&a));
        if(::fputs(args[1].to_str().c_str(),fp) == EOF)
        {
            ctx.log_msg("Error writing to file.");
            ctx.set_return(0);
        }
        else
            ctx.set_return(1);

    }

    void feof(ARGS)
    {
        int a = args[0].to_int();
        FILE* fp = *(reinterpret_cast<FILE**>(&a));
        ctx.set_return(
            ::feof(fp) != 0
            );
    }

    // General functions
    void gettype(ARGS)
    {
        dscript::value v = args[0];
        switch(v.type)
        {
        case dscript::value::type_int:
            ctx.set_return("integer");
            break;
        case dscript::value::type_flt:
            ctx.set_return("float");
            break;
        case dscript::value::type_str:
            ctx.set_return("string");
            break;
        default:
            ctx.set_return("unknown");
            break;
        }
    }
}

namespace 
{
    void link_string_functions(dscript::context& ctx)
    {
        ctx.link_function(
            "strcmp",
            &dscript::stdlib::strcmp,
            2,2,"(%str1,%str2)"
            );

        ctx.link_function(
            "stricmp",
            &dscript::stdlib::stricmp,
            2,2,"(%str1,%str2)"
            );
        
        ctx.link_function(
            "strncmp",
            &dscript::stdlib::strncmp,
            3,3,"(%str1,%str2,%count)"
            );

        ctx.link_function(
            "strnicmp",
            &dscript::stdlib::strnicmp,
            3,3,"(%str1,%str2,%count)"
            );

        ctx.link_function(
            "substr",
            &dscript::stdlib::substr,
            2,3,"(%str,%start[,%length])"
            );
    }

    void link_math_functions(dscript::context& ctx)
    {
        ctx.link_function(
            "sqrt",
            &dscript::stdlib::sqrt,
            1,1,"(%num)"
            );

        ctx.link_function(
            "sin",
            &dscript::stdlib::sin,
            1,1,"(%num)"
            );

        ctx.link_function(
            "cos",
            &dscript::stdlib::cos,
            1,1,"(%num)"
            );

        ctx.link_function(
            "tan",
            &dscript::stdlib::tan,
            1,1,"(%num)"
            );

        ctx.link_function(
            "asin",
            &dscript::stdlib::asin,
            1,1,"(%num)"
            );

        ctx.link_function(
            "acos",
            &dscript::stdlib::acos,
            1,1,"(%num)"
            );

        ctx.link_function(
            "atan",
            &dscript::stdlib::atan,
            1,1,"(%num)"
            );
        
        ctx.link_function(
            "pow",
            &dscript::stdlib::pow,
            2,2,"(%num,%exp)"
            );



    }

    void link_io_functions(dscript::context& ctx)
    {
        ctx.link_function(
            "print",
            &dscript::stdlib::print
            );

        ctx.link_function(
            "readln",
            &dscript::stdlib::readln
            );

        ctx.link_function(
            "fopen",
            &dscript::stdlib::fopen,
            2,2,"(%filename,%mode)"
            );

        ctx.link_function(
            "fclose",
            &dscript::stdlib::fclose,
            1,1,"(%filehandle)"
            );

        ctx.link_function(
            "fgets",
            &dscript::stdlib::fgets,
            2,2,"(%filehandle,%maxlength)"
            );

        ctx.link_function(
            "fputs",
            &dscript::stdlib::fgets,
            2,2,"(%filehandle,%value)"
            );

        ctx.link_function(
            "feof",
            &dscript::stdlib::feof,
            1,1,"(%filehandle)"
            );

    }

    void link_gen_functions(dscript::context& ctx)
    {
        ctx.link_function(
            "gettype",
            &dscript::stdlib::gettype,
            1,1,"(%val)"
            );
    }
}



void link_stdlib(context& ctx)
{
    // String Functions
    link_string_functions(ctx);

    // Math functions
    link_math_functions(ctx);

    // IO functions
    link_io_functions(ctx);

    // General functions
    link_gen_functions(ctx);
}

}// end namespace