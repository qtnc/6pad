-- Table utils: general utilities for tables, lists, arrays, maps and sets
-- Author: QuentinC
-- Last modified: 12.01.2012

list = setmetatable({},table)
map = setmetatable({},table)

-- Store all arguments passed to the function in a new table and return it
function table.collect (...)
local t = L{}
for _,v in ... do table.insert(t,v) end
return t
end

-- Clone a table and preserve its metatable
function table:clone ()
local t = setmetatable({},getmetatable(self))
for k,v in pairs(self) do t[k]=v end
return t
end

-- Apply a function to each entry of a table and return a new table
function table:map (f, ...)
local c = setmetatable({}, getmetatable(self))
for k,v in pairs(self) do
c[k] = f(v, k, ...)
end
return c
end

-- Apply a function to each entry of a table and overwrite existing values
function table:foreach (f, ...)
for k,v in pairs(self) do
self[k] = f(v, k, ...)
end
return self
end

-- Return a new table containing all entries for which the given function returns true
function table:filter (f, ...)
local t = setmetatable({},getmetatable(self))
for k,v in pairs(self) do
if f(v,k,...) then t[k]=v end
end
return t
end

-- Return true if the given function returns true for at least one entry of the table
function table:some (f, ...)
for k,v in pairs(self) do
if f(v,k,...) then return true end
end 
return false end

-- Return true if the given function returns true for all entries in the table, false otherwise
function table:every (f, ...)
for k,v in pairs(self) do
if not f(v,k,...) then return false end
end 
return true end

-- Reduce a table to a single value using a kind of suming function
function table:reduce (f, initial, ...)
local value = initial
for k,v in pairs(self) do
value = f(value,v,k,...)
end
return value
end

-- Return a new table whose keys and values are reversed (i.e. keys become values and vice-versa)
function table:flip ()
local t = setmetatable({},getmetatable(self))
for k,v in pairs(self) do t[v]=k end
return t
end

-- Return true if the table contains the value given
function table:contains (value)
for k,v in pairs(self) do
if value==v then return k end
end end

-- Return the number of times that the value given appear on the table
function table:count (value)
local c = 0
for _,v in pairs(self) do
if v==value then c=c+1 end
end
return c
end

-- Look for a particular value in the list and return its index, or nil if not found
function list:find (x,first,last)
if not first then first=1 end
if not last then last = #self end
if first>#self then first=#self+1+first end
if last>#self then last=#self+1+last end
for i=first, last do
if self[i]==x then return i end
end  end

-- Look for a particular value in the list in reverse direction and return its index, or nil if not found
function list:rfind (x,last,first)
if not first then first=1 end
if not last then last = #self end
if first>#self then first=#self+1+first end
if last>#self then last=#self+1+last end
for i=last,first,-1 do
if self[i]==x then return i end
end  end

-- Look for a particular value in the list using binary search algorithm. Return true and its index if found, or false and the index where the value should be inserted if not found.
function list:bfind (x,first,last)
if not first then first=1 end
if not last then last = #self end
if first>#self then first=#self+1+first end
if last>#self then last=#self+1+last end
----print('search', x, 'between', self[first], 'and', self[last], first, last)
if first>=last then 
if self[first]==x then return true, first 
elseif x<self[first] then return false, first
else return false, first+1 end
end
local n = math.floor( (last+first)/2 )
if self[n]==x then return true, n
elseif x<self[n] then return list.bfind(self, x, first, n -1)
else return list.bfind(self, x, n+1, last)
end end

-- Insert given element to the list
function list:add (pos,x)
if type(x)=='nil' then
x = pos
pos = #self
end
if type(x)=='table' then
local len = #x
for i = #self, pos, -1 do self[len+i] = self[i] end
for i=1, len do self[pos+i -1] = x[i] end
else table.insert(self,pos,x) end
return self
end

-- Return a copy of the list in reverse order
function list:reverse ()
local t = setmetatable({},getmetatable(self))
for i=#self,1,-1 do table.insert(t,self[i]) end
return t
end

-- Insert one or more elements at the end of the list
function list:push (...)
for _,x in ipairs{...} do
self:insert(x)
end
return self
end

-- Insert one or more elements at the beginning of the list
function list:unshift (...)
for _,x in ipairs{...} do
self:insert(1,x)
end
return self
end

-- Remove one or more elements from the beginning of the list and return them.
function list:shift (n)
if not n or n<=0 then n=1 end
if n==1 then return self:remove(1) end
local c = {}
for i=n,1,-1 do table.insert(c, self:remove(1)) end
return unpack(c)
end

-- Remove one or more elements from the end of the list and return them.
function list:pop (n)
if not n or n<=0 then n=1 end
if n==1 then return self:remove() end
local c = {}
for i=1,n do table.insert(c, 1, self:remove()) end
return unpack(c)
end

-- Merge one or more lists into this list
function list:merge (...)
for _,tbl in ipairs{...} do
for _,item in ipairs(tbl) do
self:insert(item)
end end
return self
end

-- Return a sublist containing elements from first to last.
function list:sub (first, last)
if not first then first=1 end
if not last then last=#self end
if first>#self then first=#self+1+first end
if last>#self then last=#self+1+last end
local r = setmetatable({},getmetatable(self))
for i = first, last  do r:insert(self[i]) end
return r
end

-- Remove a part of the list and optionnally replace it with elements coming from another list
function list:splice (first, last, replacement)
if type(replacement)=='nil' then replacement = {} 
elseif type(replacement)!='table' then replacement = {replacement} end
if not last then last = #self end
if first>#self then first = #self+1+first end
if last>#self then last = #self +1 +last end
local t = self:sub(first,last)
local delta = #replacement +first -last
if delta<=0 then
for i=last, #self-delta do  self[i+delta] = self[i+1]  end
for i=1,#replacement do self[i+first -1] = replacement[i] end
else
for i=#self, last, -1 do self[i+delta-1] = self[i] end
for i=1,#replacement do self[i+first -1] = replacement[i] end
end
return t
end

-- Randomly permute the items of the list
function list:shuffle ()
for i = #self, 2, -1 do
local j = math.random(i-1)
self[i], self[j] = self[j], self[i]
end
return self
end

-- Randomly choose one or more elements from the list and return them
function list:random (n,i, j)
if not n then n=1 end
if not i then i=1 end
if not j then j=#self end
if i>#self then i=#self+i+1 end
if j>#self then j=#self+j+1 end
if n<=1 then return self[math.random(i,j)] end
local t = setmetatable({},getmetatable(self))
local used = {}
while #t<n do
local x = math.random(i,j)
if not used[x] then
table.insert(t,self[x])
used[x]=true
end end
return t
end

-- Friendly string representation of a list
function list:__tostring ()
return '{' .. self:concat(', ') .. '}'
end

-- Return a list of all keys contained in the map
function map:keys ()
local t = L{}
for k,_ in pairs(self) do t:insert(k) end
return t
end

-- Return a list of all values contained in the map
function map:values ()
local t = L{}
for k,_ in pairs(self) do t:insert(v) end
return t
end

-- Merge one or more maps into this map
function map:merge (...)
local replace = true
for _,tbl in ipairs{...} do
if type(tbl)=='boolean' then replace = tbl
else for k,v in tbl do
if replace or not self[k] then self[k]=v end
end end end
return self
end

-- Remove from the map all keys contained in the table given
function map:remove (l)
for k,_ in pairs(self) do
if l[k] then self[k]=nil end
end
return self
end

-- Remove from the map all keys that are not in the table given
function map:retain (l)
for k,_ in pairs(self) do
if not l[k] then self[k]=nil end
end
return self
end

-- Return a new map containing all keys that are both in this map and in m.
function map:intersection (m)
local t = setmetatable({},getmetatable(self))
for k,v in pairs(self) do
if m[k] then t[k]=v end
end
return t
end

-- Return a new map containing all keys of this map that aren't in m
function map:difference (m)
local t = setmetatable({},getmetatable(self))
for k,v in pairs(self) do
if not m[k] then t[k]=v end
end
return t
end

-- Return a new map containing all keys of all maps
function map:union (...)
local m = self:clone()
return m:merge(...)
end

-- Friendly string representation of a map
function map:__tostring ()
local s = '{'
for k,v in pairs(self) do
s=s..k..' = '..v
if next(self,k) then s=s..', ' end
end
return s..'}'
end

-- map, set, list and array constructors
function L (t) return setmetatable(t or {}, list) end
function A (t) return setmetatable(t or {}, list) end
function M (t) return setmetatable(t or {}, map) end
function S (t) return setmetatable(t or {}, map) end

-- Iterator that return the keys in sorted order
function spairs (t,f)
local keys = map.keys(t)
table.sort(keys)
local i=0
return function()
i=i+1
return keys[i], t[keys[i]]
end end

-- Return an iterator that return only elements for which the given function returns true
function fpairs (t,f)
local k = nil
local it; it = function ()
k = next(t,k)
if k==nil then return 
elseif not f(k,t[k]) then return it()
else return k, t[k]
end end 
return it
end

-- Idem but for ipairs instead of pairs
function fipairs (t,f)
local k = 0
local it; it = function ()
k=k+1
if k>#t then return
elseif not f(k,t[k]) then return it()
else return k, t[k]
end end 
return it
end

list.slice = list.sub
table.collect = list.collect
map.__index=map
map.__ipairs=pairs
list.__index=list
list.__pairs=ipairs
table.unpack = unpack
table.__index=table

