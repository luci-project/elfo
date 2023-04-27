#!/usr/bin/env python3
# Elfo - a lightweight parser for the Executable and Linking Format
# Copyright 2021-2023 by Bernhard Heinloth <heinloth@cs.fau.de>
# SPDX-License-Identifier: AGPL-3.0-or-later

import sys
from pyparsing import *

COLON,LBRACE,RBRACE,EQ,COMMA = map(Suppress,":{}=,")
identifier = Word(alphas, alphanums + '_ ')
enumValue = Group(identifier('name') + Optional(EQ + Word(nums+'abcdefABCDEFx')('value')))
enumList = Group(enumValue + ZeroOrMore(COMMA + enumValue))
enum = Suppress('enum') + Suppress(ZeroOrMore('class')) + identifier('enum') + Suppress(ZeroOrMore(COLON + identifier)) + LBRACE + enumList('names') + ZeroOrMore(COMMA) + RBRACE

header = """
// Elfo - a lightweight parser for the Executable and Linking Format
// Copyright 2021-2023 by Bernhard Heinloth <heinloth@cs.fau.de>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#ifdef USE_DLH
#include <dlh/types.hpp>
#include <dlh/stream/buffer.hpp>
#include <dlh/utility.hpp>
using ostream = BufferStream;
#else
#include <cstdint>
#include <ostream>
using std::ostream;
#endif

#include "{include}"

#ifndef _ENUM_VALUE
#define _ENUM_VALUE 1
static bool _enum_value = false;
inline ostream& enum_value_show(ostream& os) {{ _enum_value = true; return os; }}
inline ostream& enum_value_hide(ostream& os) {{ _enum_value = false; return os; }}
#endif
"""

fstart = """
inline ostream& operator<<(ostream& os, {ident} val) {{
	switch(val) {{"""
fcase = '		case {ident}::{name}: return os << (_enum_value ? "{name} ({value})" : "{name}");'
fend = """	}}
	if (_enum_value) os << "({ident})";
	return os << static_cast<uintptr_t>(val);
}}

static {ident} _enum_values_{enum}[] = {{{list}}};
"""

print(header.format(include = sys.argv[1]))
for item, _, _ in enum.scanString(sys.stdin.read()):
    ident = '::'.join(sys.argv[2:] + [item.enum])
    print(fstart.format(ident = ident.strip()))
    values = set()
    enums = set()
    val = 0
    for entry in item.names:
        if entry.value != '':
            val = int(entry.value, 0)
        if not val in values:
            name = entry.name
            print(fcase.format(ident = ident.strip(), name = entry.name.strip(), value = val if val < 256 else hex(val)))
            values.add(val)
            enums.add(f'{ident}::{entry.name}')
        val += 1
    print(fend.format(ident = ident.strip(), enum = item.enum.strip(), list = ', '.join(sorted(enums))))
