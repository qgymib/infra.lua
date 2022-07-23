local infra = require('infra')

local t1 = {}
t1.a = 'a'
t1.b = { b = 'b', c = 'c' }

local t2 = {}
t2.a = 'a'
t2.b = t1

infra.dump_value_as_string(t2)
