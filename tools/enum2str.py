#!/usr/bin/env python3

import sys
from pyparsing import *

COLON,LBRACE,RBRACE,EQ,COMMA = map(Suppress,":{}=,")
identifier = Word(alphas, alphanums + '_ ')
enumValue = Group(identifier('name') + Optional(EQ + Word(nums+'abcdefABCDEFx')('value')))
enumList = Group(enumValue + ZeroOrMore(COMMA + enumValue))
enum = Suppress('enum') + Suppress(ZeroOrMore('class')) + identifier('enum') + Suppress(ZeroOrMore(COLON + identifier)) + LBRACE + enumList('names') + ZeroOrMore(COMMA) + RBRACE


print('#pragma once')
print()
print('#include <cstdint>')
print('#include <ostream>')
print('#include <vector>')
print()
print('#include "{}"'.format(sys.argv[1]))
print()
print('#ifndef _ENUM_VALUE');
print('#define _ENUM_VALUE 1');
print('static bool _enum_value = false;');
print('inline std::ostream& enum_value_show(std::ostream& os) { _enum_value = true; return os; }')
print('inline std::ostream& enum_value_hide(std::ostream& os) { _enum_value = false; return os; }')
print('#endif');

for item, _, _ in enum.scanString(sys.stdin.read()):
    ident = '::'.join(sys.argv[2:] + [item.enum])
    print()
    print(f'std::ostream& operator<<(std::ostream& os, {ident} val) {{')
    print('	switch(val) {')
    values = set()
    enums = set()
    val = 0
    for entry in item.names:
        if entry.value != '':
            val = int(entry.value, 0)
        if not val in values:
            name = entry.name
            value = val if val < 256 else hex(val)
            print(f'		case {ident}::{name}: return os << (_enum_value ? "{name} ({value})" : "{name}");')
            values.add(val)
            enums.add(f'{ident}::{name}')
        val += 1
    print('	};')
    print(f'	if (_enum_value) os << "({ident})";')
    print(f'	return os << static_cast<std::uintptr_t>(val);')
    print('};')
    print()
    enumsList = ', '.join(enums)
    print(f'static std::initializer_list<{ident}> _enum_values_{item.enum} = {{{enumsList}}};')
    print()
