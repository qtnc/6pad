require 'fileUtils'
do
local function createClosure (event)
window.functions = {}
return function (file, ...)
local extension = io.fileSuffix(file)
if window.functions[extension] and window.functions[extension][event] then 
return window.functions[extension][event](file, ...)
else return true end
end --closure
end --createClosure

for _, event in ipairs{'onBeforeSave', 'onAfterSave', 'onBeforeOpen', 'onAfterOpen', 'onClose'} do
window[event] = createClosure(event)
end

end

