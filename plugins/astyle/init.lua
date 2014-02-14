require 'filedir'
require 'process'
require 'plugins.fileUtils'

astyle = {
options = {
java = '--mode=java ',
cs = '--mode=cs ',
c = '--mode=c '
},
suffixes = {
java = 'java', cs = 'cs',
c = 'c', cpp = 'c', h = 'c', hpp = 'c', cxx = 'c', hxx = 'c'
},
run = function ()
local suffix = astyle.suffixes[io.fileSuffix(window.filename)]
if not suffix then return end
local usesel  = false
if window.edit.selectionEnd-window.edit.selectionStart != 0 then usesel = true end
local text = usesel and window.edit.selectedText or window.edit.text
local p = process.open('plugins/astyle/astyle.exe ' .. astyle.options[suffix])
local result = ''
while #text>1024 do
p:write(text:sub(1,1024))
p:flush()
text=text:sub(1024)
local r = p:read('*a')
if r then result=result..r end
end
if #text>0 then p:write(text) end
p:wait()
local r = p:read('*a')
if r then result = result.. r end
p:close()
if usesel then window.selectedText = result
else window.edit.text = result end
end
}

window.menubar[3]:add(4, '&Mise en forme automatique', 'Ctrl+I', astyle.run)
