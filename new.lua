--  see :/help, line 523 for ʁLua SCRIPTINGʀ reference (use Meta+Enter to open)
--  :/auto.lua, line 19 for the source of auto-loaded script (incl. openXref)
--
Mk['^J,K'] = function(Tx,count)
  local Rx = Txt.openXref(Tx)
  for N,line in Tx:lines() do
    if ... then
      Rx:IL(line); Tx:Xref(N)
    end
  end
end
