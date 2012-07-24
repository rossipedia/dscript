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
// For debugging the parser. Undefine to track the parser's progress
// See documentation at http://spirit.sourceforge.net
// #define BOOST_SPIRIT_DEBUG
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Spirit Parser Framework Includes
#include <boost/spirit/home/classic.hpp>
#include <boost/spirit/home/classic/tree/parse_tree.hpp>
#include <boost/spirit/home/classic/tree/parse_tree_utils.hpp>
#include <boost/spirit/home/classic/tree/tree_to_xml.hpp>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Standard Library Includes
#include <stack>
#include <fstream>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// DScript Includes
#include "compiler.h"
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Use some namespaces to avoid wrist injury :)
using namespace std;
using namespace dscript;
using namespace boost::spirit::classic;
////////////////////////////////////////////////////////////////////////////////

namespace dscript
{

/// This struct contains all the information that the compiler
/// needs to convert a parse tree into a codeblock
struct compile_context
{
    compile_context(
        string_table& _strings,
        float_table& _floats
        ) : strings(_strings), floats(_floats)
    {
    }
    string_table& strings;
    float_table& floats;

    size_t func_depth;
    size_t loop_count;

    stack< stack<size_t> > break_indices;
    stack< stack<size_t> > continue_indices;

    codeblock_t code; // get the count by calling size()
};

////////////////////////////////////////////////////////////////////////////////
// THE SPIRIT PARSER FOR DSCRIPT
////////////////////////////////////////////////////////////////////////////////

/// This is the skip grammar. This grammar defines the language that
/// the parser will use to skip white space and others
struct skip_grammar : public boost::spirit::classic::grammar<skip_grammar>
{
    template<typename ScanT>
    struct definition
    {
        definition(const skip_grammar& self)
        {
            skip
                =   space_p
                |   comment_p("//")
                ;

            debug();
        }

		rule<ScanT> skip;

        void debug()
        {
#ifdef BOOST_SPIRIT_DEBUG
            BOOST_SPIRIT_DEBUG_RULE(skip);
#endif
        }

		const rule<ScanT>& start()
        {
            return skip;
        }
    };
};


/// This enum specifies all the node ids that the parse tree will generate
/// if the code is parsed correctly.
enum parser_ids
{
    stmt_list_id,
    stmt_block_id,
    stmt_id,
    func_call_id,
    ident_id,
    expr_id,
    func_decl_id,
    var_id,
    lvar_id,
    gvar_id,
    constant_id,
    flt_const_id,
    int_const_id,
    str_const_id,
    return_stmt_id,
    assign_stmt_id,
    assign_op_id,
    logical_op_id,
    bitwise_expr_id,
    expr_atom_id,
    bitwise_op_id,
    equality_expr_id,
    equality_op_id,
    compare_expr_id,
    compare_op_id,
    shift_expr_id,
    shift_op_id,
    add_expr_id,
    add_op_id,
    mul_expr_id,
    mul_op_id,
    unary_expr_id,
    unary_op_id,
    if_stmt_id,
    while_stmt_id,
    for_stmt_id,
    break_stmt_id,
    continue_stmt_id,
    inc_dec_expr_id,
    inc_dec_stmt_id,
    inc_dec_op_id,
    aidx_id,
    num_rule_ids
};

/// This is the actual language specification for DScript
struct grammar : public boost::spirit::classic::grammar<grammar>
{
    template<typename ScanT>
    struct definition
    {
        rule<ScanT, parser_tag<stmt_list_id> > stmt_list;
        rule<ScanT, parser_tag<stmt_block_id> > stmt_block;
        rule<ScanT, parser_tag<stmt_id> > stmt;
        rule<ScanT, parser_tag<func_call_id> > func_call;
        rule<ScanT, parser_tag<ident_id> > ident;
        rule<ScanT, parser_tag<expr_id> > expr;
        rule<ScanT, parser_tag<func_decl_id> > func_decl;
        rule<ScanT, parser_tag<var_id> > var;
        rule<ScanT, parser_tag<lvar_id> > lvar;
        rule<ScanT, parser_tag<gvar_id> > gvar;
        rule<ScanT, parser_tag<constant_id> > constant;
        rule<ScanT, parser_tag<flt_const_id> > flt_const;
        rule<ScanT, parser_tag<int_const_id> > int_const;
        rule<ScanT, parser_tag<str_const_id> > str_const;
        rule<ScanT, parser_tag<return_stmt_id> > return_stmt;
        rule<ScanT, parser_tag<assign_stmt_id> > assign_stmt;
        rule<ScanT, parser_tag<assign_op_id> > assign_op;
        rule<ScanT, parser_tag<bitwise_expr_id> > bitwise_expr;
        rule<ScanT, parser_tag<expr_atom_id> > expr_atom;
        rule<ScanT, parser_tag<bitwise_op_id> > bitwise_op;
        rule<ScanT, parser_tag<equality_expr_id> > equality_expr;
        rule<ScanT, parser_tag<equality_op_id> > equality_op;
        rule<ScanT, parser_tag<compare_expr_id> > compare_expr;
        rule<ScanT, parser_tag<compare_op_id> > compare_op;
        rule<ScanT, parser_tag<logical_op_id> > logical_op;
        rule<ScanT, parser_tag<shift_expr_id> > shift_expr;
        rule<ScanT, parser_tag<shift_op_id> > shift_op;
        rule<ScanT, parser_tag<add_expr_id> > add_expr;
        rule<ScanT, parser_tag<add_op_id> > add_op;
        rule<ScanT, parser_tag<mul_expr_id> > mul_expr;
        rule<ScanT, parser_tag<mul_op_id> > mul_op;
        rule<ScanT, parser_tag<unary_expr_id> > unary_expr;
        rule<ScanT, parser_tag<unary_op_id> > unary_op;
        rule<ScanT, parser_tag<if_stmt_id> > if_stmt;
        rule<ScanT, parser_tag<while_stmt_id> > while_stmt;
        rule<ScanT, parser_tag<for_stmt_id> > for_stmt;
        rule<ScanT, parser_tag<break_stmt_id> > break_stmt;
        rule<ScanT, parser_tag<continue_stmt_id> > continue_stmt;
        rule<ScanT, parser_tag<inc_dec_op_id> > inc_dec_op;
        rule<ScanT, parser_tag<inc_dec_expr_id> > inc_dec_expr;
        rule<ScanT, parser_tag<inc_dec_stmt_id> > inc_dec_stmt;
        rule<ScanT, parser_tag<aidx_id> > aidx;

        void debug()
        {
#ifdef BOOST_SPIRIT_DEBUG
            //// *E DBG*
            BOOST_SPIRIT_DEBUG_RULE(stmt_list);
            BOOST_SPIRIT_DEBUG_RULE(stmt_block);
            BOOST_SPIRIT_DEBUG_RULE(stmt);
            BOOST_SPIRIT_DEBUG_RULE(func_call);
            BOOST_SPIRIT_DEBUG_RULE(ident);
            BOOST_SPIRIT_DEBUG_RULE(expr);
            BOOST_SPIRIT_DEBUG_RULE(func_decl);
            BOOST_SPIRIT_DEBUG_RULE(var);
            BOOST_SPIRIT_DEBUG_RULE(lvar);
            BOOST_SPIRIT_DEBUG_RULE(gvar);
            BOOST_SPIRIT_DEBUG_RULE(constant);
            BOOST_SPIRIT_DEBUG_RULE(flt_const);
            BOOST_SPIRIT_DEBUG_RULE(int_const);
            BOOST_SPIRIT_DEBUG_RULE(str_const);
            BOOST_SPIRIT_DEBUG_RULE(return_stmt);
            BOOST_SPIRIT_DEBUG_RULE(assign_stmt);
            BOOST_SPIRIT_DEBUG_RULE(assign_op);
            BOOST_SPIRIT_DEBUG_RULE(bitwise_expr);
            BOOST_SPIRIT_DEBUG_RULE(expr_atom);
            BOOST_SPIRIT_DEBUG_RULE(bitwise_op);
            BOOST_SPIRIT_DEBUG_RULE(equality_expr);
            BOOST_SPIRIT_DEBUG_RULE(equality_op);
            BOOST_SPIRIT_DEBUG_RULE(compare_expr);
            BOOST_SPIRIT_DEBUG_RULE(compare_op);
            BOOST_SPIRIT_DEBUG_RULE(logical_op);
            BOOST_SPIRIT_DEBUG_RULE(shift_expr);
            BOOST_SPIRIT_DEBUG_RULE(shift_op);
            BOOST_SPIRIT_DEBUG_RULE(add_expr);
            BOOST_SPIRIT_DEBUG_RULE(add_op);
            BOOST_SPIRIT_DEBUG_RULE(mul_expr);
            BOOST_SPIRIT_DEBUG_RULE(mul_op);
            BOOST_SPIRIT_DEBUG_RULE(unary_expr);
            BOOST_SPIRIT_DEBUG_RULE(unary_op);
            BOOST_SPIRIT_DEBUG_RULE(if_stmt);
            BOOST_SPIRIT_DEBUG_RULE(while_stmt);
            BOOST_SPIRIT_DEBUG_RULE(for_stmt);
            BOOST_SPIRIT_DEBUG_RULE(inc_dec_op);
            BOOST_SPIRIT_DEBUG_RULE(inc_dec_expr);
            BOOST_SPIRIT_DEBUG_RULE(inc_dec_stmt);
            BOOST_SPIRIT_DEBUG_RULE(aidx);
            //// *E DBG*
#endif
        }

        definition(const dscript::grammar& self)
        {
            stmt_list
                =   *stmt
                ;

            stmt_block
                =   inner_node_d[
                        '{' >> *stmt >> '}'
                    ]
                ;

            stmt
                =   func_call >> discard_node_d[ ch_p(';') ]
                |   func_decl
                |   return_stmt >> discard_node_d[ ch_p(';') ]
                |   assign_stmt >> discard_node_d[ ch_p(';') ]
                |   stmt_block
                |   if_stmt
                |   while_stmt
                |   break_stmt >> discard_node_d[ ch_p(';') ]
                |   continue_stmt >> discard_node_d[ ch_p(';') ]
                |   for_stmt
                |   ';' // null statement
                ;

            return_stmt
                =   str_p("return") >> !expr
                ;

            break_stmt
                =   str_p("break")
                ;

            continue_stmt
                =   str_p("continue")
                ;

            assign_stmt
                =   var >> assign_op >> expr
                |   inc_dec_stmt
                ;

            inc_dec_stmt
                =   inc_dec_op >> var
                |   var >> inc_dec_op
                ;

            inc_dec_op
                =   str_p("++")
                |   "--"
                ;

            if_stmt
                =   discard_node_d[ str_p("if") ] >> inner_node_d[ '(' >> expr >> ')' ]
                    >> stmt
                        >> !(
                            discard_node_d[ str_p("else") ] >> stmt
                        )
                ;

            while_stmt
                =   discard_node_d[ str_p("while") ] >> inner_node_d[ '(' >> expr >> ')' ]
                    >> stmt
                ;

            for_stmt
                =   discard_node_d[ str_p("for") >> '(' ]
                    >>  !assign_stmt >> ch_p(';') >>
                        !expr >> ch_p(';') >>
                        !assign_stmt
                        >> discard_node_d[ ch_p(')') ]
                    >> stmt
                ;

            assign_op
                =   longest_d[
                        ch_p('=')
                        |   "*="
                        |   "/="
                        |   "%="
                        |   "+="
                        |   "-="
                        |   "&="
                        |   "|="
                        |   "^="
                        |   "@="
                        |   "<<="
                        |   ">>="
                    ]
                ;

            func_call
                =   ident >> discard_node_d[ ch_p('(') ]
                        >> infix_node_d[ !list_p(expr,',') ]
                        >> discard_node_d[ ch_p(')') ]
                ;

            ident
                =   lexeme_d[
                        token_node_d[
                            alpha_p >> *(alnum_p | '_' | ':')
                        ]
                    ]
                ;

            expr
                =   bitwise_expr >> *(logical_op >> bitwise_expr)
                ;

            logical_op
                =   str_p("||")
                |   "&&"
                ;

            bitwise_expr
                =   equality_expr >> *(bitwise_op >> equality_expr)
                ;

            bitwise_op
                =   ch_p('&')
                |   '|'
                |   '^'
                ;

            equality_expr
                =   compare_expr >> *(equality_op >> compare_expr)
                ;

            equality_op
                =   str_p("==")
                |   "!="
                ;

            compare_expr
                =   shift_expr >> *(compare_op >> shift_expr)
                ;

            compare_op
                =   longest_d[
                        ch_p('<')
                        |   '>'
                        |   "<="
                        |   ">="
                    ]
                ;

            shift_expr
                =   add_expr >> *(shift_op >> add_expr)
                ;

            shift_op
                =   str_p("<<")
                |   ">>"
                ;

            add_expr
                =   mul_expr >> *(add_op >> mul_expr)
                ;

            add_op
                =   ch_p('+')
                |   '-'
                |   '@'
                ;

            mul_expr
                =   unary_expr >> *(mul_op >> unary_expr)
                ;

            mul_op
                =   ch_p('*')
                |   '/'
                |   '%'
                ;

            unary_expr
                =   inc_dec_expr
                |   !unary_op >> expr_atom
                ;

            inc_dec_expr
                =   inc_dec_op >> var
                |   var >> inc_dec_op
                ;

            unary_op
                =   ch_p('!')
                |   '+'
                |   '-'
                |   '~'
                ;

            expr_atom
                =   constant
                |   var
                |   inner_node_d[ '(' >> expr >> ')' ]
                |   func_call
                ;

            constant
                =   flt_const
                |   int_const
                |   str_const
                ;

            flt_const
                =   strict_real_p
                ;

            int_const
                =   lexeme_d[
                        token_node_d[
                            "0x" >> uint_parser<unsigned, 16, 1, 8>()
                        ]
                    ]
                |   int_p
                ;

            str_const
                =   lexeme_d[
                        token_node_d[
                            '"' >> *(c_escape_ch_p - '"') >> '"'
                        ]
                    ]
                ;

            gvar
                =   lexeme_d[
                        token_node_d[
                            ch_p('$') >> alpha_p >> *(alnum_p | '_' | ':')
                        ]
                    ]
                ;

            lvar
                =   lexeme_d[
                        token_node_d[
                            ch_p('%') >> alpha_p >> *(alnum_p | '_' | ':')
                        ]
                    ]
                ;

            var
                =   (gvar | lvar)
                    >> !inner_node_d[ '[' >> aidx >> ']' ]
                ;

            aidx
                =   infix_node_d[ list_p(expr,',') ]
                ;

            func_decl
                =   discard_node_d[ str_p("function") ] >> ident
                    >> discard_node_d[ ch_p('(') ]
                        >> infix_node_d[ !list_p(lvar,',') ]
                    >> discard_node_d[ ch_p(')') ]
                    >> stmt_block
                ;

            debug();
        }

        const rule<ScanT,parser_tag<stmt_list_id> >& start()
        {
            return stmt_list;
        }
    };
};

////////////////////////////////////////////////////////////////////////////////
// The DScript compiler framework.
//
// These functions take the parse tree generated by spirit's pt_parse function
// and use the information in it to create the codeblock, which is akin to
// assembly language.
////////////////////////////////////////////////////////////////////////////////

template<typename TreeIterT>
struct compile_error : public std::runtime_error
{
    compile_error(const std::string& msg,TreeIterT _node) : runtime_error(msg),node(_node) {}
    TreeIterT node;
};

template<typename TreeIterT>
void compile_func_call(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == func_call_id);

    // Are there params to be pushed?
    if(iter->children.size() > 1)
    {
        TreeIterT expr = iter->children.end();
        --expr; // last first (push args from right to left
        assert(expr->value.id() == expr_id);
        TreeIterT end = iter->children.begin();
        for(; expr != end; --expr)
        {
            assert(expr->value.id() == expr_id);
            // compile the expression
            compile_expr(expr,ctx);
            // and push it onto the param stop
            // (popping it from the runtime stack)
            ctx.code.push_back(op_push_param);
        }
    }
    // first node is the identifier
    const TreeIterT::value_type& ident = get_first_leaf(*iter);
    assert(ident.value.id() == ident_id);
    // Call the function
    ctx.code.push_back(op_call_func);
    string name(ident.value.begin(),ident.value.end());
    ctx.code.push_back(ctx.strings.insert(name));
}

template<typename TreeIterT>
void compile_constant(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == constant_id);
    // the child holds the actual type of constant
    const TreeIterT::value_type& lit = get_first_leaf(*iter);
    string val(lit.value.begin(),lit.value.end());
    switch(lit.value.id().to_long())
    {
    case str_const_id:
        // push a string constant
        val = unescape(val);
        ctx.code.push_back(op_push_str);
        ctx.code.push_back(ctx.strings.insert(val));
        break;
    case int_const_id:
        // push an integer constant
        {
            stringstream i;
            int ival;
            i << val;
            if(val.length() > 2 && val.substr(0,2) == "0x")
                i >> hex >> ival;
            else
                i >> ival;
            ctx.code.push_back(op_push_int);
            ctx.code.push_back(ival);
        }
        break;
    case flt_const_id:
        // push a float constant
        {
            stringstream d;
            double dval;
            d << val;
            d >> dval;
            ctx.code.push_back(op_push_float);
            ctx.code.push_back(ctx.floats.insert(dval));
        }
        break;
    default:
        throw compile_error<TreeIterT>("Unknown constant node type",iter);
        break;
    };
}

template<typename TreeIterT>
void compile_aidx(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == aidx_id);
    TreeIterT expr = iter->children.begin();
    TreeIterT end = iter->children.end();
    for( ; expr != end; ++expr )
    {
        compile_expr(expr,ctx);
        // push the cat_aidx op
        ctx.code.push_back(op_cat_aidx_expr);
    }
}

template<typename TreeIterT>
void compile_var(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == var_id);
    if(iter->children.size() == 1)
    {
        // push the variable's value
        typedef TreeIterT::value_type node_t;
        const node_t& leaf = get_first_leaf(*iter);
        string token(leaf.value.begin(),leaf.value.end());
        ctx.code.push_back(op_push_var);
        ctx.code.push_back(ctx.strings.insert(token));
    }
    else
    {
        assert(iter->children.size() == 2);

        TreeIterT child = iter->children.begin();

        // the prefix
        typedef TreeIterT::value_type node_t;
        const node_t& leaf = get_first_leaf(*child);
        string pref_token(leaf.value.begin(),leaf.value.end());
        // push the name of the variable as a string
        ctx.code.push_back(op_push_str);
        ctx.code.push_back(ctx.strings.insert(pref_token));
        // move on to the array index
        ++child;
        // and compile it
        compile_aidx(child,ctx);
        // this op takes the name of the variable on the top
        // of the stack, and replaces it with the value
        // of that variable
        ctx.code.push_back(op_push_var_value);
    }
}

template<typename TreeIterT>
void compile_expr_atom(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == expr_atom_id);

    // gonna be a constant, var, or expr
    // no children
    assert(iter->children.size() == 1);

    // the atom!
    const TreeIterT& atom = iter->children.begin();

    switch(atom->value.id().to_long())
    {
    case var_id:
        compile_var(atom,ctx);
        break;
    case constant_id:
        compile_constant(atom,ctx);
        break;
    case expr_id:
        compile_expr(atom,ctx);
        break;
    case func_call_id:
        compile_func_call(atom,ctx);
        // load the return value to the top of the stack
        ctx.code.push_back(op_load_ret);
        break;
    default:
        throw compile_error<TreeIterT>("Unknown expr_atom node",iter);
    }
}

template<typename TreeIterT>
void compile_inc_dec_expr(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == inc_dec_expr_id);
    assert(iter->children.size() == 2); // op and var
    // get a node type
    typedef TreeIterT::value_type node_t;

    // post or pre inc/dec?
    if(iter->children.begin()->value.id() == var_id)
    {
        // post
        assert((iter->children.begin() + 1)->value.id() == inc_dec_op_id);

        const node_t& op = get_first_leaf(*(iter->children.begin()+1));
        const node_t& var = get_first_leaf(*(iter->children.begin()));

        string op_token(op.value.begin(),op.value.end());
        string var_token(var.value.begin(),var.value.end());

        compile_var(iter->children.begin(),ctx);

        if(op_token[0] == '+') // increment
            ctx.code.push_back(op_inc_var);
        else
            ctx.code.push_back(op_dec_var);

        ctx.code.push_back(ctx.strings.insert(var_token));
    }
    else
    {
        // pre
        assert((iter->children.begin())->value.id() == inc_dec_op_id);
        assert((iter->children.begin()+1)->value.id() == var_id);

        const node_t& op = get_first_leaf(*(iter->children.begin()));
        const node_t& var = get_first_leaf(*(iter->children.begin()+1));

        string op_token(op.value.begin(),op.value.end());
        string var_token(var.value.begin(),var.value.end());

        if(op_token[0] == '+')
            ctx.code.push_back(op_inc_var);
        else // decrement
            ctx.code.push_back(op_dec_var);
        ctx.code.push_back(ctx.strings.insert(var_token));

        compile_var(iter->children.begin()+1,ctx);
    }
}

template<typename TreeIterT>
void compile_unary_expr(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == unary_expr_id);
    // This compiles all the !x ~x +x -x expr

    // get a node type
    typedef TreeIterT::value_type node_t;

    // If we have a unary op
    TreeIterT i = iter->children.begin();
    parser_ids id = static_cast<parser_ids>(i->value.id().to_long());
    switch(id)
    {
    case expr_atom_id:
        // only child should be the expr_atom
        compile_expr_atom(iter->children.begin(),ctx);
        break;
    case unary_op_id:
        {
            // first is op
            const node_t& op = get_first_leaf(*(iter->children.begin()));
            // second is the expr_atom
            compile_expr_atom(iter->children.begin() + 1,ctx);
            // perform the op
            switch(*(op.value.begin()))
            {
            case '+':
                // no need to do anything, positive is default
                break;
            case '-':
                ctx.code.push_back(op_neg);
                break;
            case '!':
                ctx.code.push_back(op_log_not);
                break;
            case '~':
                ctx.code.push_back(op_bit_not);
                break;
            default:
                throw compile_error<TreeIterT>("Unknown unary operator",iter);
            }
        }
        break;
    case inc_dec_expr_id:
        compile_inc_dec_expr(iter->children.begin(),ctx);
        break;
    default:
        throw compile_error<TreeIterT>("Unknown Unary Expr Node",iter);
    }
}

template<typename TreeIterT>
void compile_mul_expr(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == mul_expr_id);
    // This compiles  < > <= >= expressions
    // always an odd number of children
    assert(iter->children.size() % 2 == 1);

    // get a node type
    typedef TreeIterT::value_type node_t;

    TreeIterT sub_expr = iter->children.begin();
    TreeIterT end = iter->children.end();
    // compile the left sub_expr
    compile_unary_expr(sub_expr,ctx);
    for(++sub_expr;sub_expr != end;++sub_expr)
    {
        // get the node representing the op
        const node_t& op = get_first_leaf(*sub_expr);
        ++sub_expr; // next sub_expr (skip op)
        // compile the next sub_expr
        compile_unary_expr(sub_expr,ctx);
        // perform the op
        switch(*(op.value.begin()))
        {
        case '*':
            ctx.code.push_back(op_mul);
            break;
        case '/':
            ctx.code.push_back(op_div);
            break;
        case '%':
            ctx.code.push_back(op_mod);
            break;
        default:
            throw compile_error<TreeIterT>("Unknown mul op",iter);
        }
    }
}

template<typename TreeIterT>
void compile_add_expr(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == add_expr_id);
    // This compiles  < > <= >= expressions
    // always an odd number of children
    assert(iter->children.size() % 2 == 1);

    // get a node type
    typedef TreeIterT::value_type node_t;

    TreeIterT sub_expr = iter->children.begin();
    TreeIterT end = iter->children.end();
    // compile the left sub_expr
    compile_mul_expr(sub_expr,ctx);
    for(++sub_expr;sub_expr != end;++sub_expr)
    {
        // get the node representing the op
        const node_t& op = get_first_leaf(*sub_expr);
        ++sub_expr; // next sub_expr (skip op)
        // compile the next sub_expr
        compile_mul_expr(sub_expr,ctx);
        // perform the op
        switch(*(op.value.begin()))
        {
        case '+': // ==
            ctx.code.push_back(op_add);
            break;
        case '-': // !=
            ctx.code.push_back(op_sub);
            break;
        case '@':
            ctx.code.push_back(op_cat);
            break;
        default:
            throw compile_error<TreeIterT>("Unknown add op",iter);
        }
    }
}

template<typename TreeIterT>
void compile_shift_expr(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == shift_expr_id);
    // This compiles  < > <= >= expressions
    // always an odd number of children
    assert(iter->children.size() % 2 == 1);

    // get a node type
    typedef TreeIterT::value_type node_t;

    TreeIterT sub_expr = iter->children.begin();
    TreeIterT end = iter->children.end();
    // compile the left sub_expr
    compile_add_expr(sub_expr,ctx);
    for(++sub_expr;sub_expr != end;++sub_expr)
    {
        // get the node representing the op
        const node_t& op = get_first_leaf(*sub_expr);
        ++sub_expr; // next sub_expr (skip op)
        // compile the next sub_expr
        compile_add_expr(sub_expr,ctx);
        // perform the op
        switch(*(op.value.begin()))
        {
        case '<': // ==
            ctx.code.push_back(op_shl);
            break;
        case '>': // !=
            ctx.code.push_back(op_shr);
            break;
        default:
            throw compile_error<TreeIterT>("Unknown shift op",iter);
        }
    }
}

template<typename TreeIterT>
void compile_compare_expr(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == compare_expr_id);
    // This compiles  < > <= >= expressions
    // always an odd number of children
    assert(iter->children.size() % 2 == 1);

    // get a node type
    typedef TreeIterT::value_type node_t;

    TreeIterT sub_expr = iter->children.begin();
    TreeIterT end = iter->children.end();
    // compile the left sub_expr
    compile_shift_expr(sub_expr,ctx);
    for(++sub_expr;sub_expr != end;++sub_expr)
    {
        // get the node representing the op
        const node_t& op = get_first_leaf(*sub_expr);
        ++sub_expr; // next sub_expr (skip op)
        // compile the next sub_expr
        compile_shift_expr(sub_expr,ctx);
        // perform the op
        string op_token(op.value.begin(),op.value.end());
        switch(op_token[0])
        {
        case '<': // ==
            if(op_token.length() > 1 && op_token[1] == '=')
                ctx.code.push_back(op_cmp_less_eq);
            else
                ctx.code.push_back(op_cmp_less);
            break;
        case '>': // !=
            if(op_token.length() > 1 && op_token[1] == '=')
                ctx.code.push_back(op_cmp_grtr_eq);
            else
                ctx.code.push_back(op_cmp_grtr);
            break;
        default:
            throw compile_error<TreeIterT>("Unknown comparison op",iter);
        }
    }
}

template<typename TreeIterT>
void compile_equality_expr(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == equality_expr_id);
    // This compiles == !=
    // always an odd number of children
    assert(iter->children.size() % 2 == 1);

    // get a node type
    typedef TreeIterT::value_type node_t;

    TreeIterT sub_expr = iter->children.begin();
    TreeIterT end = iter->children.end();
    // compile the left sub_expr
    compile_compare_expr(sub_expr,ctx);
    for(++sub_expr;sub_expr != end;++sub_expr)
    {
        // get the node representing the op
        const node_t& op = get_first_leaf(*sub_expr);
        ++sub_expr; // next sub_expr (skip op)
        // compile the next sub_expr
        compile_compare_expr(sub_expr,ctx);
        // perform the op
        switch(*(op.value.begin()))
        {
        case '=': // ==
            ctx.code.push_back(op_eq);
            break;
        case '!': // !=
            ctx.code.push_back(op_neq);
            break;
        default:
            throw compile_error<TreeIterT>("Unknown equality op",iter);
        }
    }
}

template<typename TreeIterT>
void compile_bitwise_expr(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == bitwise_expr_id);
    // This compiles & | ^ expressions
    // always an odd number of children
    assert(iter->children.size() % 2 == 1);

    // get a node type
    typedef TreeIterT::value_type node_t;

    TreeIterT sub_expr = iter->children.begin();
    TreeIterT end = iter->children.end();
    // compile the left sub_expr
    compile_equality_expr(sub_expr,ctx);
    for(++sub_expr;sub_expr != end;++sub_expr)
    {
        // get the node representing the op
        const node_t& op = get_first_leaf(*sub_expr);
        ++sub_expr; // next sub_expr (skip op)
        // compile the next sub_expr
        compile_equality_expr(sub_expr,ctx);
        // perform the op
        switch(*(op.value.begin()))
        {
        case '&': // ==
            ctx.code.push_back(op_bit_and);
            break;
        case '|': // !=
            ctx.code.push_back(op_bit_or);
            break;
        case '^':
            ctx.code.push_back(op_bit_xor);
            break;
        default:
            throw compile_error<TreeIterT>("Unknown bitwise op",iter);
        }
    }
}

template<typename TreeIterT>
void compile_expr(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == expr_id);
    // the main expr handles && and ||
    // always an odd number
    assert(iter->children.size() % 2 == 1);

    // get a node type
    typedef TreeIterT::value_type node_t;

    TreeIterT sub_expr = iter->children.begin();
    TreeIterT end = iter->children.end();
    // compile the left sub_expr
    compile_bitwise_expr(sub_expr,ctx);
    for(++sub_expr;sub_expr != end;++sub_expr)
    {
        // get the node representing the op
        const node_t& op = get_first_leaf(*sub_expr);
        ++sub_expr; // next sub_expr (skip op)
        // compile the next sub_expr
        compile_bitwise_expr(sub_expr,ctx);
        // perform the op
        switch(*(op.value.begin()))
        {
        case '&':
            ctx.code.push_back(op_log_and);
            break;
        case '|':
            ctx.code.push_back(op_log_or);
            break;
        default:
            throw compile_error<TreeIterT>("Unknown expr op",iter);
        }
    }
}

template<typename TreeIterT>
void compile_func_decl(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == func_decl_id);

    // increment the function depth
    ++ctx.func_depth;

    const TreeIterT::value_type& ident = get_first_leaf(*iter);
    string name(ident.value.begin(),ident.value.end());
    // start the function declaration
    ctx.code.push_back(op_decl_func);
    ctx.code.push_back(ctx.strings.insert(name));
    size_t resolve = ctx.code.size();
    ctx.code.push_back(0); // unknown right now, will use resolve later to set it

    // ctx.instr_count += 3; // one for op, one for name, and one for instr_count

    TreeIterT arg = iter->children.begin() + 1;
    TreeIterT end = iter->children.end();
    for(; arg != end && (arg->value.id() != stmt_block_id); ++arg)
    {
        assert(arg->value.id() == lvar_id);
        const TreeIterT::value_type& leaf = get_first_leaf(*arg);
        // out << "op_pop_param " << string(leaf.value.begin(),leaf.value.end()) << endl;
        // ctx.instr_count += 2; // one for op, one for operand
        string arg_name(leaf.value.begin(),leaf.value.end());
        ctx.code.push_back(op_pop_param);
        ctx.code.push_back(ctx.strings.insert(arg_name));
    }
    // we should have a stmt_block
    if(arg != end)
    {
        assert(arg != iter->children.end());

        // good, compile the stmt_list
        compile_stmt_block(arg,ctx);
    }

    // in case there's no explicit return statement
    ctx.code.push_back(op_return);

    // resolve now the offset of the end of the function
    ctx.code[resolve] = ctx.code.size();

    // remove a function depth
    --ctx.func_depth;
}

template<typename TreeIterT>
void compile_return_stmt(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == return_stmt_id);
    // The only child (if there is one) is going to be a expr
    if(iter->children.size() > 1)
    {
        assert(iter->children.size() == 2);
        TreeIterT expr = iter->children.begin() + 1;
        compile_expr(expr,ctx);
        // store it in the return value register
        ctx.code.push_back(op_store_ret);
    }
    ctx.code.push_back(op_return);
}

template<typename TreeIterT>
void compile_inc_dec_stmt(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == inc_dec_stmt_id);
    TreeIterT op = (iter->children.begin()->value.id() == var_id) ? (iter->children.begin()+1) : (iter->children.begin());
    TreeIterT var = (iter->children.begin()->value.id() == var_id) ? (iter->children.begin()) : (iter->children.begin()+1);
    string var_token(get_first_leaf(*var).value.begin(),get_first_leaf(*var).value.end());
    string op_token(get_first_leaf(*op).value.begin(),get_first_leaf(*op).value.end());

    if(op_token[0] == '+')
        ctx.code.push_back(op_inc_var);
    else
        ctx.code.push_back(op_dec_var);
    ctx.code.push_back(ctx.strings.insert(var_token));
}

template<typename TreeIterT>
void compile_assign_stmt(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == assign_stmt_id);

    // as assign statement either has 3 nodes, or 1
    assert(iter->children.size() == 3 || iter->children.size() == 1);
    if(iter->children.size() == 3)
    {
        typedef TreeIterT::value_type node_t;

        const node_t& op = get_first_leaf(iter->children[1]);
        const TreeIterT& expr = iter->children.begin() + 2;

        TreeIterT var = iter->children.begin();
        string var_tok(
                get_first_leaf(*var).value.begin(),
                get_first_leaf(*var).value.end()
            );
        if(var->children.size() > 1)
        {
            // array index
            TreeIterT aidx = var->children.begin() + 1;
            // push the name of the var
            // out << "op_push_var_name " << var_tok << endl;
            // ctx.instr_count += 2;
            ctx.code.push_back(op_push_str);
            ctx.code.push_back(ctx.strings.insert(var_tok));
            // compile the aidx
            compile_aidx(aidx,ctx);
        }

        // now, compile the expression
        compile_expr(expr,ctx);

        // second, check what kind of op it is:
        string op_tok(op.value.begin(),op.value.end());

        // op_code
        op_code opcode = op_invalid;
        bool do_var = var->children.size() == 1;

        // only need to check the first char:
        switch(op_tok[0])
        {
        case '=':
            opcode = do_var ? op_assign_var : op_assign;
            break;
        case '*':
            opcode = do_var ? op_mul_asn_var : op_mul_asn;
            break;
        case '/':
            opcode = do_var ? op_div_asn_var : op_div_asn;
            break;
        case '%':
            opcode = do_var ? op_mod_asn_var : op_mod_asn;
            break;
        case '+':
            opcode = do_var ? op_add_asn_var : op_add_asn;
            break;
        case '-':
            opcode = do_var ? op_sub_asn_var : op_sub_asn;
            break;
        case '@':
            opcode = do_var ? op_cat_asn_var : op_cat_asn;
            break;
        case '&':
            opcode = do_var ? op_band_asn_var : op_band_asn;
            break;
        case '|':
            opcode = do_var ? op_bor_asn_var : op_bor_asn;
            break;
        case '^':
            opcode = do_var ? op_bxor_asn_var : op_bxor_asn;
            break;
        case '<':
            opcode = do_var ? op_shl_asn_var : op_shl_asn;
            break;
        case '>':
            opcode = do_var ? op_shr_asn_var : op_shr_asn;
            break;
        default:
            throw compile_error<TreeIterT>("Unknown assignment operator",iter);
        }

        // add the appropriate opcode
        ctx.code.push_back(opcode);
        // without the var, the top value on the stack
        // gets placed into the variable named by the value at (top - 1)
        // and both values get popped off the stack
        if(do_var) // otherwise the variable is explicitly stated in the next instr
            ctx.code.push_back(ctx.strings.insert(var_tok));
    }
    else // it's an inc_dec_stmt
        compile_inc_dec_stmt(iter->children.begin(),ctx);
}

template<typename TreeIterT>
void compile_if_stmt(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == if_stmt_id);

    // Get a node type
    typedef TreeIterT::value_type node_t;

    // only gonna have 2 or 3 children, that's all we can have
    assert(iter->children.size() >= 2 && iter->children.size() <= 3);

    // first child is going to be the expression
    TreeIterT expr = iter->children.begin();

    // compile the expression
    compile_expr(expr,ctx);
    // jmp if false
    ctx.code.push_back(op_jmp_false);
    size_t end_if = ctx.code.size();
    ctx.code.push_back(0); // to be resolved later

    // now compile the body for the true block
    TreeIterT true_stmt = expr + 1;
    compile_stmt(true_stmt,ctx);
    // if there's an else block
    if(iter->children.size() == 3)
    {
        // set the jmp to the end of the else block
        ctx.code.push_back(op_jmp);
        size_t end_else = ctx.code.size();
        // make this exist
        ctx.code.push_back(0); // to resolve later

        // resolve the end_if
        ctx.code[end_if] = ctx.code.size();

        TreeIterT false_stmt = true_stmt + 1;
        // now compile it
        compile_stmt(false_stmt,ctx);
        ctx.code[end_else] = ctx.code.size();
    }
    else
        // resolve the end_if
        ctx.code[end_if] = ctx.code.size();
}

template<typename TreeIterT>
void compile_while_stmt(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == while_stmt_id);
    // has to have 2 children
    assert(iter->children.size() == 2);

    // increase the loop count
    ++ctx.loop_count;

    // push a new stack of resolve points onto each
    ctx.break_indices.push(stack<size_t>());
    ctx.continue_indices.push(stack<size_t>());

    // this is the point where the loop continues
    size_t continue_index = ctx.code.size();

    // compile the test expression
    TreeIterT expr = iter->children.begin();
    compile_expr(expr,ctx);
    // jump if false to end of loop
    ctx.code.push_back(op_jmp_false);
    // to break point
    ctx.break_indices.top().push(ctx.code.size());
    ctx.code.push_back(0); // resolve later

    // compile the body statement(s)
    TreeIterT stmt = expr + 1;
    compile_stmt(stmt,ctx);

    // jump back to the beginning
    ctx.code.push_back(op_jmp);
    ctx.code.push_back(continue_index);

    // this is where the loop breaks
    size_t break_index = ctx.code.size();

    // resolve the break and continue points

    // continue indices
    while(ctx.continue_indices.top().size() > 0)
    {
        ctx.code[ctx.continue_indices.top().top()] = continue_index;
        ctx.continue_indices.top().pop();
    }

    // break indices
    while(ctx.break_indices.top().size() > 0)
    {
        ctx.code[ctx.break_indices.top().top()] = break_index;
        ctx.break_indices.top().pop();
    }

    // decrease the loop count
    --ctx.loop_count;

    // pop the resolve stacks
    ctx.continue_indices.pop();
    ctx.break_indices.pop();
}

template<typename TreeIterT>
void compile_break_stmt(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == break_stmt_id);
    if(ctx.loop_count == 0)
        throw compile_error<TreeIterT>("break encountered outside of loop",iter);
    else
    {
        // push an op_jmp
        ctx.code.push_back(op_jmp);
        // push the current index to be resolved
        ctx.break_indices.top().push(ctx.code.size());
        // now make it exist :)
        ctx.code.push_back(0); // to be resolved
    }
}

template<typename TreeIterT>
void compile_continue_stmt(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == continue_stmt_id);
    if(ctx.loop_count == 0)
        throw compile_error<TreeIterT>("continue encountered outside of loop",iter);
    else
    {
        // push an op_jmp
        ctx.code.push_back(op_jmp);
        // push the current index to be resolved
        ctx.continue_indices.top().push(ctx.code.size());
        // now make it exist :)
        ctx.code.push_back(0); // to be resolved
    }
}

template<typename TreeIterT>
void compile_for_stmt(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == for_stmt_id);

    // inc loop count
    ++ctx.loop_count;

    TreeIterT child = iter->children.begin();
    TreeIterT end = iter->children.end();

    // push a new stack of resolve points onto each
    ctx.break_indices.push(stack<size_t>());
    ctx.continue_indices.push(stack<size_t>());

    // if we have an assign init statement, compile it here
    if(child->value.id() == assign_stmt_id)
    {
        compile_assign_stmt(iter->children.begin(),ctx);
        ++child;
    }

    assert(child->value.id() == for_stmt_id); // first ;
    ++child;

    // loop start
    size_t test_expr_start = ctx.code.size();
    if(child->value.id() == expr_id)
    {
        // compile the test expression
        compile_expr(child,ctx);
        // add the jump if false
        ctx.code.push_back(op_jmp_false);
        // this gets resolved to the break index
        ctx.break_indices.top().push(ctx.code.size());
        ctx.code.push_back(0); // to resolve later
        // next
        ++child;
    }
    assert(child->value.id() == for_stmt_id); // second ;
    ++child;

    // Check for the iter statement
    // if it exists, this gets put at the continue point
    bool iter_stmt = false;
    if(child->value.id() == assign_stmt_id)
    {
        iter_stmt = true;
        ++child;
    }
    assert(child->value.id() == stmt_id);

    // compile the body statement
    compile_stmt(child,ctx);

    // now we're at the continue point
    size_t continue_index = ctx.code.size();

    // resolve all the continue points
    while(ctx.continue_indices.top().size() > 0)
    {
        ctx.code[ctx.continue_indices.top().top()] = continue_index;
        ctx.continue_indices.top().pop();
    }

    // If there's an iterative statement, compile it
    if(iter_stmt)
    {
        --child;
        assert(child->value.id() == assign_stmt_id);
        compile_assign_stmt(child,ctx);
    }

    // Jump unconditionally to the loop start
    ctx.code.push_back(op_jmp);
    ctx.code.push_back(test_expr_start);

    // Now we're at the break point (end of the loop), so resolve 'em
    while(ctx.break_indices.top().size() > 0)
    {
        ctx.code[ctx.break_indices.top().top()] = ctx.code.size();
        ctx.break_indices.top().pop();
    }

    // decrease the loop count
    --ctx.loop_count;

    // pop the resolve stacks
    ctx.continue_indices.pop();
    ctx.break_indices.pop();
}

template<typename TreeIterT>
void compile_stmt(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == stmt_id);
    TreeIterT child = iter->children.begin();
    parser_ids id = static_cast<parser_ids>(child->value.id().to_long());
    switch(id)
    {
    case func_call_id:
        compile_func_call(child,ctx);
        break;
    case func_decl_id:
        compile_func_decl(child,ctx);
        break;
    case return_stmt_id:
        if(ctx.func_depth == 0)
            throw compile_error<TreeIterT>("return statement found at global scope",iter);
        else
            compile_return_stmt(child,ctx);
        break;
    case assign_stmt_id:
        compile_assign_stmt(child,ctx);
        break;
    case stmt_block_id:
        compile_stmt_block(child,ctx);
        break;
    case if_stmt_id:
        compile_if_stmt(child,ctx);
        break;
    case while_stmt_id:
        compile_while_stmt(child,ctx);
        break;
    case break_stmt_id:
        compile_break_stmt(child,ctx);
        break;
    case continue_stmt_id:
        compile_continue_stmt(child,ctx);
        break;
    case for_stmt_id:
        compile_for_stmt(child,ctx);
        break;
    case stmt_id:
        if(*(get_first_leaf(*child).value.begin()) == ';')
        {
            // null statement (noop)
            break;
        }
    default:
        throw compile_error<TreeIterT>("Unknown Statement Node (SANITY CHECK)",iter);
    }
}

template<typename TreeIterT>
void compile_stmt_list(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == stmt_list_id);
    TreeIterT child = iter->children.begin();
    TreeIterT end = iter->children.end();
    for(; child != end; ++child)
        compile_stmt(child,ctx);
}

template<typename TreeIterT>
void compile_stmt_block(const TreeIterT& iter, compile_context& ctx)
{
    assert(iter->value.id() == stmt_block_id);
    // this is just the same as stmt_list
    TreeIterT child = iter->children.begin();
    TreeIterT end = iter->children.end();
    for(; child != end; ++child)
        compile_stmt(child,ctx);
}

// compile the parse tree to ostream&
template<typename TreeNodeT>
void compile_parse_tree(const TreeNodeT& tree,compile_context& ctx)
{
    // start from the top
    TreeNodeT::const_iterator iter = tree.begin();
	compile_stmt_list(iter,ctx);
}

// compile a string into a codeblock, using the passed string and float table
codeblock_t compile(const string& code,string_table& strings,float_table& floats)
{
    // Create a compile context
    compile_context ctx(strings,floats);
    // Attempt to parse the string
    typedef position_iterator<string::const_iterator> iter_t;

    dscript::grammar grammar;
    dscript::skip_grammar skip;

#ifdef BOOST_SPIRIT_DEBUG
    BOOST_SPIRIT_DEBUG_GRAMMAR(grammar);
    BOOST_SPIRIT_DEBUG_GRAMMAR(skip);
#endif

    iter_t first(code.begin(),code.end());
    iter_t last;

    typedef node_iter_data_factory<> fact_t;
    tree_parse_info<iter_t,fact_t> info = pt_parse<fact_t>(first, last, grammar, skip);
    if(info.match)
    {
		if(info.length > 0)
		{
			typedef tree_node<
				tree_match<
					iter_t,
					fact_t,
					nil_t
				>::parse_node_t
			> node_t;
			typedef vector<node_t>::const_iterator tree_iter_t;

			try
			{
				compile_parse_tree(info.trees,ctx);
			}
			catch(compile_error<tree_iter_t>& e)
			{
				// construct a new compiler_error and toss it
				file_position fp = e.node->value.begin().get_position();
				throw compiler_error(e.what(),code_position(fp.line,fp.column));
			}
		}
		else
		{
			// Empty code. All comments or some such
		}
    }
    else
    {
        throw compiler_error(
            "Parse Error",
            code_position(
                info.stop.get_position().line,
                info.stop.get_position().column
                )
        );
    }
    return ctx.code;
}

} // end namespace dscript

