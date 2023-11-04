--  see :/help, line 564 for ʁLua SCRIPTINGʀ reference (use Meta+Enter to open)
--  :/auto.lua, line 19 for the source of auto-loaded script (incl. openXref)
--
Mk['^J,X'] = function(Tx,count)
  local Rx = Txt.openXref(Tx)
  for N,line in Tx:lines() do
    if ... then
      Rx:IL(line); Tx:Xref(N)
    end
  end
end
MkLF["filter"] = function(filename,addText)
  local filter = Re[[...]];
  for str in io.lines(filename) do
    if str:len() > 0 then addText( str ) end
  end
end
