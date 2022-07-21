local infra = require('infra')

local function test()
    os.exit(-1)
end

thread = infra.thread_create(test)
infra.thread_join(thread)
