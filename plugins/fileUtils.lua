function io.fileSuffix  (s) 
return s:match('%.(%w+)$'):lower()
end

function io.dirname (s)
local i = s:crfind('\\') or s:crfind('/')
if not i then return s
else return s:sub(1,i)
end end

function io.basename (s)
local i = s:crfind('\\') or s:crfind('/')
if not i then return s
else return s:sub(i+1)
end end

function io.splitname (s)
local i = s:crfind('\\') or s:crfind('/')
if not i then return s, s
else return s:sub(1,i), s:sub(i+1)
end end

