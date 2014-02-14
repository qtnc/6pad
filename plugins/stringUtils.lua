function string:substrCount (pattern)
local i,j,c = 0, 0, -1
repeat
i, j = self:find(pattern,i+1,true)
c=c+1
until not i
return c
end

function string:startsWith (s) 
return self:sub(1,#s)==s
end

function string:endsWith (s)
return self:sub(-#s)==s
end
