MkCCD["Ctrl+Shift+T"] = function(Tx) Tx:IC(os.date()) end
MkCCD["Ctrl+Shift++"] = function(Tx)
  local result = loadstring("local X="..Tx:gtl()..";return X")()
  Tx:IC("= "..tostring(result))
end
