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
// Standard Library Include Files
#include <iostream>
#include <string>
#include <fstream>
#include <map>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Boost Includes (http://www.boost.org)
#include <boost/scoped_array.hpp>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DScript Include Files
#include "compiler.h"
////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace 
{
    /// Read an element from a stream
    template<typename TypeT>
    void read_elem(istream& in,TypeT* data, size_t size = sizeof(TypeT))
    {
        if(in.eof())
            throw std::runtime_error("Premature end of file.");
        if(in.read(reinterpret_cast<char*>(data),size).fail())
            throw std::runtime_error("Error reading data from file.");
    }
}

namespace dscript {

codeblock_t load_compiled_file(const string& filename,string_table& strings,float_table& floats)
{
    // open up an istream
    ifstream file(filename.c_str(),ios::binary);

    if(!file)
        throw std::runtime_error(filename + " could not be found.");

    // read in the dsc tag
    char dsc_tag[4];
    read_elem(file,dsc_tag,4);
    if(dsc_tag[3] != '\0')
        throw std::runtime_error(filename + " is not a valid DSC file");
    if(strcmp(dsc_tag,"DSC") != 0)
        throw std::runtime_error(filename + " is not a valid DSC file");

    // read the instruction count
    size_t instr_count;
    read_elem(file,&instr_count);
    // allocate a buffer for the instructions
    boost::scoped_array<unsigned int> buffer(new unsigned int[instr_count]);

    // read all the instructions
    read_elem(file,buffer.get(),instr_count * sizeof(unsigned int));

    // copy to a codeblock
    codeblock_t code(buffer.get(),buffer.get() + instr_count);

    // load the string table

    // read how many
    size_t count = 0;
    read_elem(file,&count);
    for(size_t i = 0; i < count; ++i)
    {
        // read the length of the string
        size_t len;
        read_elem(file,&len);

        // read the string
        boost::scoped_array<char> buf(new char[len + 1]);
        read_elem(file,buf.get(),len);
        buf[len] = '\0';

        // add it to the string table, and get the entry
        string_table::entry ste = strings.insert(buf.get());

        // now read how many offsets this string has
        size_t offset_count;
        read_elem(file,&offset_count);
        // read each offset and set it to the ste
        for(size_t off = 0; off < offset_count; ++off)
        {
            // read the offset
            size_t offset = 0;
            read_elem(file,&offset);
            // set the code at that offset to the ste
            code[offset] = ste;
        }
    }

    // load the float table
    read_elem(file,&count);
    for(size_t i = 0; i < count; ++i)
    {
        // read the float
        double d = 0.0;
        read_elem(file,&d);

        // add it to the float table and get the entry
        float_table::entry fte = floats.insert(d);

        // now read how many offsets this float has
        size_t offset_count;
        read_elem(file,&offset_count);
        // read each offset and set it to the fte
        for(size_t off = 0; off < offset_count; ++off)
        {
            // read the offset
            size_t offset = 0;
            read_elem(file,&offset);
            // set the code at that offset to the fte
            code[offset] = fte;
        }
    }
    return code;
}

template<typename TypeT>
void write_elem(ostream& out,const TypeT* data,size_t size = sizeof(TypeT))
{
    out.write(
        reinterpret_cast<const char*>(data),
        size
        );
    if(out.fail() | out.bad())
        throw std::runtime_error("Could not write to file.");
}

void save_codeblock(const string& filename,const codeblock_t& code)
{
    // oh, noes!
    ofstream file(filename.c_str(),ios::binary);
    if(!file)
        throw std::runtime_error( filename + " could not be opened.");

    // write out the dsc tag
    write_elem(file,"DSC",4);
    // write out the instruction count
    size_t s = code.size();
    write_elem(file,&s);

    // this will hold the stringtable values
    typedef map<string_table::entry,vector<size_t>,cmp_ste> file_s_table;
    file_s_table s_table;

    // this will hold the float table values
    typedef map<float_table::entry,vector<size_t> > file_f_table;
    file_f_table f_table;


    // now, write out the instruction stream
    for(size_t i = 0; i < code.size(); ++i)
    {
        // write the op code
        int c = code[i].get_int();
        write_elem(file,&c);

        // op codes and such
        switch(code[i].get_op_code())
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
        case op_push_str:
            {
                ++i;
                // out a placeholder (4 0's)
                size_t s = 0;
                write_elem(file,&s);
                // add it to the list of offsets for this entry
                s_table[code[i].get_str()].push_back(i);
            }
            break;
        case op_push_float:
            {
                ++i;
                // out a placeholder (4 0's)
                size_t s = 0;
                write_elem(file,&s);
                // add this offset to the list for this float
                f_table[code[i].get_flt()].push_back(i);
            }
            break;
        case op_push_int:
        case op_jmp_false:
        case op_jmp:
            {
                ++i;
                // output the offset
                int _i = code[i].get_int();
                write_elem(file,&_i);
            }
            break;
        case op_decl_func:
            {
                ++i;
                // out a placeholder (4 0's)
                size_t s = 0;
                write_elem(file,&s);
                // add it to the list of offsets for this entry
                s_table[code[i].get_str()].push_back(i);

                ++i;
                // output the offset
                int _i = code[i].get_int();
                write_elem(file,&_i);
            }
            break;
        }
    }
    // codestream is written
    // flush it
    file.flush();

    // now write the string table

    // write out how many strings there are
    size_t count = s_table.size();
    write_elem(file,&count);

    // now write out each string and it's offset
    file_s_table::iterator str = s_table.begin();
    file_s_table::iterator s_end = s_table.end();
    for(; str != s_end; ++str)
    {
        string s = str->first;
        vector<size_t>& offsets = str->second;
        // write out the size of the string
        size_t len = s.length();
        write_elem(file,&len);
        // write out the string itself
        write_elem(
            file,
            s.c_str(),
            len
            );

        // now write out how many offsets this string has
        size_t count = offsets.size();
        write_elem(file,&count);

        // now write out each offset
        for(size_t i = 0; i < count; ++i)
        {
            size_t off = offsets[i];
            write_elem(
                file,
                &off
                );
        }
    }
    // flush it
    file.flush();

    // now write the float table

    // write out how many floats there are
    count = f_table.size();
    write_elem(file,&count);

    // now write out each float and it's offset
    file_f_table::iterator f = f_table.begin();
    file_f_table::iterator f_end = f_table.end();
    for(; f != f_end; ++f)
    {
        float_table::entry flt = f->first;
        vector<size_t>& offsets = f->second;

        // write out the float itself
        write_elem(
                file,
                flt
            );

        // now write out how many offsets this float has
        size_t count = offsets.size();
        write_elem(file,&count);

        // now write out each offset
        for(size_t i = 0; i < count; ++i)
        {
            size_t off = offsets[i];
            write_elem(
                file,
                &off
                );
        }
    }

    file.flush();
}

}