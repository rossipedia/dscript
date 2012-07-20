////////////////////////////////////////////////////////////////////////////////
// DScript Scripting Language
// Copyright (C) 2003 Bryan "daerid" Ross
//
// Permission to copy, use, modify, sell and distribute this software is
// granted provided this copyright notice appears in all copies. This
// software is provided "as is" without express or implied warranty, and
// with no claim as to its suitability for any purpose.
////////////////////////////////////////////////////////////////////////////////

#ifndef __DSCRIPT_STDLIB_H__
#define __DSCRIPT_STDLIB_H__

////////////////////////////////////////////////////////////////////////////////
// DScript Includes
#include "context.h"
////////////////////////////////////////////////////////////////////////////////

namespace dscript
{
    /// Links the DScript standard library
    void link_stdlib(context& ctx);
}

#endif//__DSCRIPT_STDLIB_H__