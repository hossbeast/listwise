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

#ifndef _LISTWISE_OPERATOR_INTERNAL_H
#define _LISTWISE_OPERATOR_INTERNAL_H

#include "xapi.h"

#include "operator.h"

struct narrator;

#define restrict __restrict

/// listwise_lwop_say
//
// SUMMARY
//  write a description of the specified optype
//
// PARAMETERS
//  optype    -
//  effectual - whether to include only effectual bits and ignore informational-only bits
//  [dst]     -
//  [sz]      -
//  [z]       -
//  [ps]      -
//  writer    -
//
xapi listwise_lwop_say(uint64_t optype, int effectual, struct narrator * const restrict N)
  __attribute__((nonnull));

#undef restrict
#endif
