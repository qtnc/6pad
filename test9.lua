do
local ffi = require 'ffi'
ffi.cdef[[ int __stdcall sayStringA (const char*, int) ; ]]
local srapi = ffi.load('plugins/ScreenReaderAPI.dll')
sayString = function (s, i)
if type(i)!='boolean' then i=false end
return 0!=srapi.sayStringA(s,i)
end
end
do
local curIndentLevel = -1
window.onLineChange = function ()
local n = #window.edit[0]:match('^%s*') +1
if n!=curIndentLevel then sayString('Niveau '..n, false) end
curIndentLevel=n
return true
end
end

