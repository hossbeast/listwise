/* Copyright (c) 2012-2015 Todd Freed <todd.freed@gmail.com>

   This file is part of fab.

   fab is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   fab is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with fab.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdlib.h>
#include <string.h>

#include "listwise/operator.h"
#include "listwise/lwx.h"

#include "macros.h"

/*

f operator - fields : window fields delimited by pairs of windows

ARGUMENTS (any number)

 offset - offset, in fields
 length - number of fields
 flags  - D to include delimiters

*/

static xapi op_exec(operation*, lwx*, int**, int*);

operator op_desc[] = {
  {
      .s            = "f"
    , .optype       = LWOP_WINDOWS_ACTIVATE | LWOP_SELECTION_STAGE | LWOP_ARGS_CANHAVE | LWOP_MODIFIERS_CANHAVE
    , .op_exec      = op_exec
    , .mnemonic     = "fields"
    , .desc         = "window delimited fields"
  }
  , {}
};

xapi op_exec(operation* o, lwx* lx, int** ovec, int* ovec_len)
{
  enter;

  int argsl = o->argsl;
  if(argsl && o->args[argsl - 1]->itype != ITYPE_I64)
    argsl--;

  char isdelim = 0;
  if(argsl != o->argsl)
    isdelim = !!strchr(o->args[argsl]->s, 'D');

  int x;
  LSTACK_ITERATE(lx, x, go)
  if(go)
  {
    lwx_windows * win;
    if(lstack_windows_state(lx, x, &win) == LWX_WINDOWS_SOME)
    {
      // number of delimited fields
      int pairs = win->l / 2;

      // whether this row has been reset
      int wasreset = 0;

      int i = 0;
      do
      {
        int win_off = 0;
        int win_len = 0;

        if(i == argsl)
        {
          // first and only iteration of this loop when this operation has no arguments
        }
        else
        {
          win_off = o->args[i]->i64;
          win_len = 0;

          if(argsl > (i + 1))
            win_len = o->args[i + 1]->i64;
        }

        int off = win_off;
        int len = win_len;

        if(win_off < 0)
          off = (pairs + win_off);
        if(win_len == 0)
          len = pairs - off;
        else if(win_len < 0)
          len = ((pairs + win_len)) - off + 1;

        if(off >= 0 && off <= pairs && len > 0)
        {
          if(!wasreset)
          {
            wasreset = 1;

            // record this index was hit
            fatal(lstack_selection_stage, lx, x);
          }

          // append delimited segment(s)
          int j;
          for(j = 0; j < len; j++)
          {
            int A = win->s[((off + j) * 2) + 0].o;
            if(!isdelim)
              A += win->s[((off + j) * 2) + 0].l;

            int B = win->s[((off + j) * 2) + 1].o;
            if(isdelim)
              B += win->s[((off + j) * 2) + 1].l;

            fatal(lstack_windows_stage, lx, x, A, B - A);
          }
        }

        i += 2;
      } while(i < argsl);
    }
  }
  LSTACK_ITEREND

  finally : coda;
}
