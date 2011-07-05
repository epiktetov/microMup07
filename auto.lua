Mk["Ctrl+Shift+T"] = function(Tx) Tx:IC(os.date()) end
Mk["Ctrl+Shift++"] = function(Tx)
  local result = loadstring("local X="..Tx:gtl()..";return X")()
  Tx:IC("= "..tostring(result))
end
for _,n in pairs{"F7","F8","F9"} do  -- inserts current value of F7/F8/F9 macro
  Mk["^J,"..n] = function(Tx)        -- into current text (edit and re-execute)
    local pref,Fn = "Mk."..n.."=",Mk[n]
    if type(Fn) == 'string' then Tx:IL(pref..'"'..Fn..'"')
                            else Tx:IL(pref..'none') end
  end
end
