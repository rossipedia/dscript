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
#include <algorithm>
#include <sstream>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DScript Includes
#include "vmachine.h"
#include "context.h"
////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace dscript;

void vmachine::execute(
                       instr_iter begin,
                       instr_iter end,
                       instr_iter instr,
                       context& ctx
                       )
{
    // push a new stack frame
    m_callstack.push(dictionary_t());

    // the current stack frame
    dictionary_t& stack_frame = m_callstack.top();

    while(instr != end)
    {
        op_code o = instr->get_op_code();
        bool returned = false;
        switch(o)
        {
        case op_push_param:
            {
                // push the top of the stack as a parameter
                // then pop the top of the runtime stack
                m_param_stack.push_back(m_runtime_stack.top());
                m_runtime_stack.pop();
                ++instr;
            }
	        break;

        case op_call_func:
            // get a reference to the func_table entry
            {
                // get the name of the function
                ++instr;
                string_table::entry name = instr->get_str();
                // Clear the return value (in case of error)
                m_return_val.clear();
                // get a reference to the function
                func_table::entry* e = functions.find(name);
                if(e == 0)
                {
                    m_return_val.set_type(value::type_int);
                    m_return_val.intval = 0;
                    ctx.log_msg("function \"" + string(name) + "\" not found");
                    m_param_stack.clear();
                }
                else if(e->is_host)
                {
                    // validate min/max args
                    if(e->min_args != -1)
                    {
                        if(m_param_stack.size() < size_t(e->min_args))
                            ctx.log_msg("Usage: " + (e->name + e->usage_string));
                    }
                    if(e->max_args != -1)
                    {
                        if(m_param_stack.size() > size_t(e->max_args))
                            ctx.log_msg("Usage: " + (e->name + e->usage_string));
                    }

                    // host function
                    // gotta reverse the args
                    reverse(m_param_stack.begin(),m_param_stack.end());
                    // call it!
                    (*(e->host_func))(m_param_stack,ctx);
                    // clear the param frame
                    m_param_stack.clear();
                }
                else
                {
                    // script function
                    execute(
                        e->begin,
                        e->end,
                        e->start,
                        ctx
                        );
                    // clear the param frame
                    m_param_stack.clear();
                }
                ++instr;
            }
	        break;

        case op_push_str:
            // push the named string
            ++instr;
            m_runtime_stack.push(instr->get_str());
            ++instr;
	        break;

        case op_push_int:
            // push the int
            ++instr;
            m_runtime_stack.push(instr->get_int());
            ++instr;
	        break;

        case op_push_float:
            // push a float
            ++instr;
            m_runtime_stack.push(*(instr->get_flt()));
            ++instr;
	        break;

        case op_cat_aidx_expr:
            // cat the two values on top with a '_'
            {
                value tocat = m_runtime_stack.top();
                m_runtime_stack.pop();
                tocat.set_type(value::type_str);
                m_runtime_stack.top() =
                    m_runtime_stack.top().to_str() + '_' + tocat.to_str();
                ++instr;
            }
	        break;

        case op_push_var:
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                dictionary_t& dict = (ste[0]=='$') ? globals : stack_frame;
                m_runtime_stack.push(dict[instr->get_str()]);
                ++instr;
            }
	        break;

        case op_push_var_value:
            // replace the top of the stack with the value of 
            // the variable named by the top of the stack
            {
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                dictionary_t& dict = (ste[0]=='$') ? globals : stack_frame;
                m_runtime_stack.top() = dict[ste];
                ++instr;
            }
            break;

        case op_load_ret:
            {
                // push the value in the return 
                // register onto the top of the stack
                m_runtime_stack.push(m_return_val);
                ++instr;
            }
	        break;

        case op_inc_var:
            // increment the variable named by one
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                dictionary_t& dict = (ste[0]=='$') ? globals : stack_frame;
                dict[ste].set_type(value::type_int);
                ++(dict[ste].intval);
                ++instr;
            }
            break;

        case op_dec_var:
            // decrement the variable named by one
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                dictionary_t& dict = (ste[0]=='$') ? globals : stack_frame;
                dict[ste].set_type(value::type_int);
                --(dict[ste].intval);
                ++instr;
            }
	        break;

        case op_neg:
            // negate the top of the stack
            // promote to int
            m_runtime_stack.top().set_type(value::type_int);
            m_runtime_stack.top().intval =  -(m_runtime_stack.top().intval);
            ++instr;
	        break;

        case op_log_not:
            // logical not the top of the stack
            // promote to int
            m_runtime_stack.top().set_type(value::type_int);
            m_runtime_stack.top().intval =  !(m_runtime_stack.top().intval);
            ++instr;
	        break;

        case op_bit_not:
            // binary not the top of the stack
            // promote to int
            m_runtime_stack.top().set_type(value::type_int);
            m_runtime_stack.top().intval =  ~(m_runtime_stack.top().intval);
            ++instr;
	        break;

        case op_mul:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // multiply the new top by the popped top
                // check types (keep int if both are int, otherwise go to flt)
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                    newtop.intval *= top.intval;
                else
                {
                    newtop.set_type(value::type_flt);
                    newtop.fltval *= top.to_flt();
                }                
                ++instr;
            }
	        break;

        case op_div:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // divide the new top by the popped top
                // check types (keep int if both are int, otherwise go to flt)
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                {
                    // check divide by zero error
                    if(top.intval == 0)
                        throw runtime_error("Divide by zero encountered.");
                    newtop.intval /= top.intval;
                }
                else
                {
                    newtop.set_type(value::type_flt);
                    newtop.fltval /= top.to_flt();
                }                
                ++instr;
            }
	        break;

        case op_mod:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // mod the new top by the popped top
                // mod may only be done on integral types
                newtop.set_type(value::type_int);
                newtop.intval %= top.to_int();
                ++instr;
            }
	        break;

        case op_add:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // add the new top to the popped top
                // check types (keep int if both are int, otherwise go to flt)
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                    newtop.intval += top.intval;
                else
                {
                    newtop.set_type(value::type_flt);
                    newtop.fltval += top.to_flt();
                }                
                ++instr;
            }
	        break;

        case op_sub:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // subtract the popped top from the new top
                // check types (keep int if both are int, otherwise go to flt)
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                    newtop.intval -= top.intval;
                else
                {
                    newtop.set_type(value::type_flt);
                    newtop.fltval -= top.to_flt();
                }                
                ++instr;
            }
	        break;

        case op_cat:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // concat the two top values together
                // check types (keep int if both are int, otherwise go to flt)
                newtop.set_type(value::type_str);
                newtop.strval.append(top.to_str());
                ++instr;
            }
	        break;

        case op_shl:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // shift left the new top by the popped top
                // shift left may only be done on integral types
                newtop.set_type(value::type_int);
                newtop.intval <<= top.to_int();
                ++instr;
            }
	        break;

        case op_shr:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // shift right the new top by the popped top
                // shift right may only be done on integral types
                newtop.set_type(value::type_int);
                newtop.intval >>= top.to_int();
                ++instr;
            }
	        break;

        case op_cmp_less_eq:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // check types (keep int if both are int, otherwise go to flt)
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                    newtop.intval = newtop.intval <= top.intval;
                else
                {
                    // bypass the conversion process
                    newtop.intval = newtop.to_flt() <= top.to_flt();
                    newtop.type = value::type_int;
                }
                ++instr;
            }
	        break;

        case op_cmp_less:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // check types (keep int if both are int, otherwise go to flt)
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                    newtop.intval = newtop.intval < top.intval;
                else
                {
                    // bypass the conversion process
                    newtop.intval = newtop.to_flt() < top.to_flt();
                    newtop.type = value::type_int;
                }
                ++instr;
            }
	        break;

        case op_cmp_grtr_eq:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // check types (keep int if both are int, otherwise go to flt)
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                    newtop.intval = newtop.intval >= top.intval;
                else
                {
                    // bypass the conversion process
                    newtop.intval = newtop.to_flt() >= top.to_flt();
                    newtop.type = value::type_int;
                }
                ++instr;
            }
	        break;

        case op_cmp_grtr:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // check types (keep int if both are int, otherwise go to flt)
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                    newtop.intval = newtop.intval > top.intval;
                else
                {
                    // bypass the conversion process
                    newtop.intval = newtop.to_flt() > top.to_flt();
                    newtop.type = value::type_int;
                }
                ++instr;
            }
	        break;

        case op_eq:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // check types (keep int if both are int, otherwise go to flt)
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                    newtop.intval = newtop.intval == top.intval;
                else
                {
                    // bypass the conversion process
                    newtop.intval = newtop.to_flt() == top.to_flt();
                    newtop.type = value::type_int;
                }
                ++instr;
            }
	        break;

        case op_neq:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // check types (keep int if both are int, otherwise go to flt)
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                    newtop.intval = newtop.intval != top.intval;
                else
                {
                    // bypass the conversion process
                    newtop.intval = newtop.to_flt() != top.to_flt();
                    newtop.type = value::type_int;
                }
                ++instr;
            }
	        break;

        case op_bit_and:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // only can be done on ints
                newtop.set_type(value::type_int);
                newtop.intval &= top.to_int();
                ++instr;
            }
	        break;

        case op_bit_or:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // only can be done on ints
                newtop.set_type(value::type_int);
                newtop.intval |= top.to_int();
                ++instr;
            }
	        break;

        case op_bit_xor:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // only can be done on ints
                newtop.set_type(value::type_int);
                newtop.intval ^= top.to_int();
                ++instr;
            }
	        break;

        case op_log_and:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // check types
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                    newtop.intval = newtop.intval && top.intval;
                else
                {
                    // bypass the conversion process
                    newtop.intval = newtop.to_flt() && top.to_flt();
                    newtop.type = value::type_int;
                }
                ++instr;
            }
	        break;

        case op_log_or:
            {
                // grab the top one
                value top = m_runtime_stack.top();
                m_runtime_stack.pop();
                value& newtop = m_runtime_stack.top();
                // check types
                if(
                    top.type == value::type_int && 
                    newtop.type == value::type_int
                    )
                    newtop.intval = newtop.intval || top.intval;
                else
                {
                    // bypass the conversion process
                    newtop.intval = newtop.to_flt() || top.to_flt();
                    newtop.type = value::type_int;
                }
                ++instr;
            }
	        break;

        case op_decl_func:
            {
                // first thing will be the function name, 
                // second will be offset at which function ends
                ++instr;
                string_table::entry func_name = instr->get_str();
                ++instr;
                int offset = instr->get_int();
                ++instr; // now points to first instruction of the function
                instr_iter func_end = begin + offset;
                
                functions.add_script_func(
                    func_name,
                    begin,
                    instr,
                    func_end
                    );

                instr = func_end;
            }
	        break;

        case op_pop_param:
            {
                // grab the name
                ++instr;
                string_table::entry ste = instr->get_str();
                // pop the top of the param stack into the named var
                if(m_param_stack.size() > 0)
                {
                    stack_frame[ste] = m_param_stack.back();
                    m_param_stack.pop_back();
                }
                else
                    stack_frame[ste].clear();
                ++instr;
            }
	        break;

        case op_return:
            returned = true;
	        break;

        case op_store_ret:
            // store the top of the runtime stack in the return value register
            m_return_val = m_runtime_stack.top();
            m_runtime_stack.pop();
            ++instr;
	        break;

        case op_assign:
            // the top of the stack is a value
            // the next underneath is the name of a variable
            // store the value in the variable
            {
                value v = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                dictionary_t& dict = (ste[0] == '$') ? globals : stack_frame;
                dict[ste] = v;
                ++instr;
            }
	        break;

        case op_assign_var:
            // assign the variable designated
            // the value on the top of the stack
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                dictionary_t& dict = (ste[0] == '$') ? globals : stack_frame;
                dict[ste] = m_runtime_stack.top();
                m_runtime_stack.pop();
                ++instr;
            }
	        break;

        case op_mul_asn:
            // the top of the stack is a value
            // the next underneath is the name of a variable
            // multiply the variable by the value
            {
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                // set the var's value
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                if(var.type == value::type_int && val.type == value::type_int)
                    var.intval *= val.intval;
                else
                {
                    var.set_type(value::type_flt);
                    var.fltval *= val.to_flt();
                }
                ++instr;
            }
	        break;

        case op_mul_asn_var:
            // multiply a specified variable by a value
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                if(var.type == value::type_int && val.type == value::type_int)
                    var.intval *= val.intval;
                else
                {
                    var.set_type(value::type_flt);
                    var.fltval *= val.to_flt();
                }
                ++instr;
            }
	        break;

        case op_div_asn:
	        // the top of the stack is a value
            // the next underneath is the name of a variable
            // divide the variable by the value
            {
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                // set the var's value
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                if(var.type == value::type_int && val.type == value::type_int)
                    var.intval /= val.intval;
                else
                {
                    var.set_type(value::type_flt);
                    var.fltval /= val.to_flt();
                }
                ++instr;
            }
	        break;

        case op_div_asn_var:
            // divide a specified variable by a value
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                value& val = m_runtime_stack.top();
                m_runtime_stack.pop();
                if(var.type == value::type_int && val.type == value::type_int)
                    var.intval /= val.intval;
                else
                {
                    var.set_type(value::type_flt);
                    var.fltval /= val.to_flt();
                }
                ++instr;
            }
	        break;

        case op_mod_asn:
            // the top of the stack is a value
            // the next underneath is the name of a variable
            // multiply the variable by the value
            {
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                // set the var's value
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                var.set_type(value::type_int);
                var.intval %= val.to_int();
                ++instr;
            }
	        break;
        
        case op_mod_asn_var:
            // mod a specified variable by a value
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                value& val = m_runtime_stack.top();
                m_runtime_stack.pop();
                // must be an int type
                var.set_type(value::type_int);
                var.intval %= val.to_int();
                ++instr;
            }
	        break;

        case op_add_asn:
            // the top of the stack is a value
            // the next underneath is the name of a variable
            // add the value to the variable
            {
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                // set the var's value
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                if(var.type == value::type_int && val.type == value::type_int)
                    var.intval += val.intval;
                else
                {
                    var.set_type(value::type_flt);
                    var.fltval += val.to_flt();
                }
                ++instr;
            }
	        break;

        case op_add_asn_var:
            // add a specified variable to a value
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                value& val = m_runtime_stack.top();
                m_runtime_stack.pop();
                if(var.type == value::type_int && val.type == value::type_int)
                    var.intval += val.intval;
                else
                {
                    var.set_type(value::type_flt);
                    var.fltval += val.to_flt();
                }
                ++instr;
            }
	        break;

        case op_sub_asn:
            // the top of the stack is a value
            // the next underneath is the name of a variable
            // add the value to the variable
            {
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                // set the var's value
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                if(var.type == value::type_int && val.type == value::type_int)
                    var.intval -= val.intval;
                else
                {
                    var.set_type(value::type_flt);
                    var.fltval -= val.to_flt();
                }
                ++instr;
            }
	        break;

        case op_sub_asn_var:
            // subtract a value from a specified variable 
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                value& val = m_runtime_stack.top();
                m_runtime_stack.pop();
                if(var.type == value::type_int && val.type == value::type_int)
                    var.intval -= val.intval;
                else
                {
                    var.set_type(value::type_flt);
                    var.fltval -= val.to_flt();
                }
                ++instr;
            }
	        break;

        case op_cat_asn:
            // the top of the stack is a value
            // the next underneath is the name of a variable
            // add the value to the variable
            {
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                // set the var's value
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                var.set_type(value::type_str);
                var.strval.append(val.to_str());
                ++instr;
            }
	        break;

        case op_cat_asn_var:
            // add a specified variable to a value
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                var.set_type(value::type_str);
                var.strval.append(val.to_str());
                ++instr;
            }
	        break;

        case op_band_asn:
            // binary and a variable's value
            {
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                // set the var's value
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                var.set_type(value::type_int);
                var.intval &= val.to_int();
                ++instr;
            }
	        break;

        case op_band_asn_var:
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                value& val = m_runtime_stack.top();
                m_runtime_stack.pop();
                // must be an int type
                var.set_type(value::type_int);
                var.intval &= val.to_int();
                ++instr;
            }
	        break;

        case op_bor_asn:
	        // binary or a variable's value
            {
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                // set the var's value
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                var.set_type(value::type_int);
                var.intval |= val.to_int();
                ++instr;
            }
	        break;

        case op_bor_asn_var:
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                value& val = m_runtime_stack.top();
                m_runtime_stack.pop();
                // must be an int type
                var.set_type(value::type_int);
                var.intval |= val.to_int();
                ++instr;
            }
	        break;

        case op_bxor_asn:
            // binary xor a variable's value
            {
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                // set the var's value
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                var.set_type(value::type_int);
                var.intval ^= val.to_int();
                ++instr;
            }
	        break;

        case op_bxor_asn_var:
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                value& val = m_runtime_stack.top();
                m_runtime_stack.pop();
                // must be an int type
                var.set_type(value::type_int);
                var.intval ^= val.to_int();
                ++instr;
            }
	        break;

        case op_shl_asn:
            // shift left a variable's value
            {
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                // set the var's value
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                var.set_type(value::type_int);
                var.intval <<= val.to_int();
                ++instr;
            }
	        break;

        case op_shl_asn_var:
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                value& val = m_runtime_stack.top();
                m_runtime_stack.pop();
                // must be an int type
                var.set_type(value::type_int);
                var.intval <<= val.to_int();
                ++instr;
            }
	        break;

        case op_shr_asn:
            // shift right a variable's value
            {
                value val = m_runtime_stack.top();
                m_runtime_stack.pop();
                string_table::entry ste = 
                    strings.insert(m_runtime_stack.top().to_str());
                m_runtime_stack.pop();
                // set the var's value
                value& var = 
                    (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                var.set_type(value::type_int);
                var.intval >>= val.to_int();
                ++instr;
            }
	        break;

        case op_shr_asn_var:
            {
                ++instr;
                string_table::entry ste = instr->get_str();
                value& var = (ste[0] == '$') ? globals[ste] : stack_frame[ste];
                value& val = m_runtime_stack.top();
                m_runtime_stack.pop();
                // must be an int type
                var.set_type(value::type_int);
                var.intval >>= val.to_int();
                ++instr;
            }
	        break;

        case op_jmp_false:
            {
                ++instr;
                // offset
                int offset = instr->get_int();
                // check the top of the stack
                if(!m_runtime_stack.top().to_int())
                    instr = begin + offset;
                else
                    ++instr;
                m_runtime_stack.pop();
            }
	        break;

        case op_jmp:
            {
                ++instr;
                // offset
                instr = begin + instr->get_int();
            }
	        break;

        default:
            {
                stringstream msg;
                msg << "Unknown op_code encountered: " << 
                    instr->get_int() << flush;
                throw runtime_error(msg.str());
            }
        }
        if(returned)
            break; // exit the loop
    }
    // pop the stack frame
    m_callstack.pop();
}