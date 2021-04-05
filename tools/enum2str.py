#!/usr/bin/env python3

import sys
from pyparsing import *

COLON,LBRACE,RBRACE,EQ,COMMA = map(Suppress,":{}=,")
identifier = Word(alphas, alphanums + '_ ')
enumValue = Group(identifier('name') + Optional(EQ + Word(nums+'abcdefABCDEFx')('value')))
enumList = Group(enumValue + ZeroOrMore(COMMA + enumValue))
enum = Suppress('enum') + Suppress(ZeroOrMore('class')) + identifier('enum') + Suppress(ZeroOrMore(COLON + identifier)) + LBRACE + enumList('names') + ZeroOrMore(COMMA) + RBRACE

header = """
#pragma once

#include <cstdint>
#include <ostream>
#include <vector>

#include "{include}"

#ifndef _ENUM_VALUE
#define _ENUM_VALUE 1
static bool _enum_value = false;
inline std::ostream& enum_value_show(std::ostream& os) {{ _enum_value = true; return os; }}
inline std::ostream& enum_value_hide(std::ostream& os) {{ _enum_value = false; return os; }}
#endif
"""

fstart = """
std::ostream& operator<<(std::ostream& os, {ident} val) {{
	switch(val) {{
"""
fcase = '		case {ident}::{name}: return os << (_enum_value ? "{name} ({value})" : "{name}");'
fend = """	}};
	if (_enum_value) os << "({ident})";
	return os << static_cast<std::uintptr_t>(val);
}}

static std::initializer_list<{ident}> _enum_values_{enum} = {{{list}}};
"""

print(header.format(include = sys.argv[1]))
for item, _, _ in enum.scanString(sys.stdin.read()):
    ident = '::'.join(sys.argv[2:] + [item.enum])
    print(fstart.format(ident = ident))
    values = set()
    enums = set()
    val = 0
    for entry in item.names:
        if entry.value != '':
            val = int(entry.value, 0)
        if not val in values:
            name = entry.name
            print(fcase.format(ident = ident, name = entry.name, value = val if val < 256 else hex(val)))
            values.add(val)
            enums.add(f'{ident}::{entry.name}')
        val += 1
    print(fend.format(ident = ident, enum = item.enum, list = ', '.join(enums)))
