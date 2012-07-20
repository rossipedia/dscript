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
#include "opcodes.h"
////////////////////////////////////////////////////////////////////////////////

using namespace dscript;

// name array
const char* g_op_names[] = 
{
    "op_push_param",
    "op_call_func",
    "op_push_str",
    "op_push_int",
    "op_push_float",
    "op_cat_aidx_expr",
    "op_push_var",
    "op_push_var_value",
    "op_load_ret",
    "op_inc_var",
    "op_dec_var",
    "op_neg",
    "op_log_not",
    "op_bit_not",
    "op_mul",
    "op_div",
    "op_mod",
    "op_add",
    "op_sub",
    "op_cat",
    "op_shl",
    "op_shr",
    "op_cmp_less_eq",
    "op_cmp_less",
    "op_cmp_grtr_eq",
    "op_cmp_grtr",
    "op_eq",
    "op_neq",
    "op_bit_and",
    "op_bit_or",
    "op_bit_xor",
    "op_log_and",
    "op_log_or",
    "op_decl_func",
    "op_pop_param",
    "op_return",
    "op_store_ret",
    "op_assign",
    "op_assign_var",
    "op_mul_asn",
    "op_mul_asn_var",
    "op_div_asn",
    "op_div_asn_var",
    "op_mod_asn",
    "op_mod_asn_var",
    "op_add_asn",
    "op_add_asn_var",
    "op_sub_asn",
    "op_sub_asn_var",
    "op_cat_asn",
    "op_cat_asn_var",
    "op_band_asn",
    "op_band_asn_var",
    "op_bor_asn",
    "op_bor_asn_var",
    "op_bxor_asn",
    "op_bxor_asn_var",
    "op_shl_asn",
    "op_shl_asn_var",
    "op_shr_asn",
    "op_shr_asn_var",
    "op_jmp_false",
    "op_jmp"
};

const char* dscript::get_op_name(op_code op)
{
    return g_op_names[op];
}