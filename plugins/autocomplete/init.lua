require 'plugins.tableUtils'
require 'plugins.stringUtils'
require 'plugins.fileUtils'

autocomplete = {
dicts = {},
mapping = { 
default = 'dictionnary' 
}}

local oldOnTabNew = window.onTabNew
window.onTabNew = function (t)
--if oldOnTabNew then oldOnTabNew(t) end
local suffix = 'default'
if t.filename then suffix = io.fileSuffix(t.filename) end
local dictName = autocomplete.mapping[suffix] or autocomplete.mapping.default
if dictName then autocomplete.setup(t, dictName) end
end

local function loadDictionnary (fn)
local t = L{}
local status, f, s, v = pcall(io.lines, fn)
if not status then return t end
for l in f,s,v do t:insert(l) end
t:sort()
return t
end

local function checkForAutocomplete (dict, line, startPos, endPos)
local c = line:byte(startPos)
if not c or c<=32 then return true end
c = line:findLastOf(' \t\r\n.,;:()[]{}<>/+*=|&#%~\'"@\\', startPos) or 0
local word  = line:sub(c+1, endPos)
local found, index = dict:bfind(word)
if found then
local partialWord = line:sub(c+1,startPos)
if dict[index+1] and dict[index+1]:startsWith(partialWord) then
return dict[index+1]:sub(1+startPos-c), true
end 
found, index = dict:bfind(partialWord) 
word = partialWord
end
if not found and dict[index] and dict[index]:startsWith(word) then
return dict[index]:sub(1+startPos-c), true
else return false end
end

local function addWords (dict, text)
for w in text:gmatch('[%aÀ-ÿ][%a%d_%-À-ÿ]+') do
if #w>3 then
local found, pos = dict:bfind(w)
if not found then dict:insert(pos,w) end
end end end

function autocomplete.setup (tab, dictName)
if not autocomplete.dicts[dictNames] then 
autocomplete.dicts[dictName] = loadDictionnary(window.basedir .. '\\plugins\\autocomplete\\' .. dictName .. '.txt')
end
local dict = autocomplete.dicts[dictName]

tab.onTab = function (line, lineNum, startPos, endPos, lineOffset)
startPos = startPos -lineOffset
endPos = endPos -lineOffset
if startPos==endPos then
return checkForAutocomplete(dict, line, startPos, endPos)
elseif endPos-startPos<#line then 
return checkForAutocomplete(dict, line, startPos, endPos)
else return true end
end

local oldenter = tab.onEnter
tab.onEnter = function (line, ...)
addWords(dict, line)
if oldenter then return oldenter(line, ...)
else return true end
end

local oldao = tab.onAfterOpen
tab.onAFterOpen = function (t)
addWords(dict, t.text)
if oldao then return oldao(t) 
else return true end
end

end--of setup function
