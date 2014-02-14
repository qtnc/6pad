--[[
require 'luacom'
local srapi = luacom.CreateObject('ScreenReaderAPI.Interface')
srapi:SayString('Ca marche !', false)
print(srapi:SapiGetVoiceName(0))
]]
