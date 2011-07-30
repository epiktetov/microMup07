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
do local function breakable(c) return (c == ' ' or c == ',' or c == ';') end
   local function squeeze(line,atN)
     return line:sub(1,atN-1)..line:sub(atN):gsub("%s%s+"," ")
   end
  local function BlockFormat(Tx)
    if Tx.X < Tx.reX then Tx.fmtFromX,Tx.fmtToX = Tx.X,Tx.reX
                     else Tx.fmtFromX,Tx.fmtToX = Tx.reX,Tx.X
    end
    local tline = squeeze(Tx:line(),Tx.fmtFromX); Tx:DL()
    local nline
    if tline:len() < Tx.fmtToX-1 then -- not enough text, try to fill that from
      nline = Tx:line()               --       next line (return if it's empty)
      if nline:len() == 0 then Tx:IL(tline);
                               Tx.reX = nil; return end
      Tx:DL()
      tline = squeeze(tline.." "..nline,Tx.fmtFromX); nline = nil
    end
    if tline:len() >= Tx.fmtToX then -- too much text, find the place to split
      local Nsp
      if tline:byte(Tx.fmtToX) == 0x20 then Nsp = Tx.fmtToX -- space is right
      else                                                  --  after the edge
        for i=Tx.fmtToX-1,2,-1 do
          if breakable(tline:sub(i,i)) then Nsp = i; break end
        end
      end                                         -- no good break found, makes
      Nsp = Nsp or tline:find(" ",Tx.fmtToX,true) -- line wider than margins :(
      if Nsp then
        nline = (" "):rep(Tx.fmtFromX-1)..tline:sub(Nsp+1)
        tline =                           tline:sub(1,Nsp)
      end
    end           Tx:IL(tline)
    if nline then Tx:IL(nline); Tx:go(-1) end; Tx.reY = Tx.Y
  end
  local function DoBlockFormat(Tx)
    if Tx.fmtFromX then Tx.reX,Tx.X = Tx.fmtFromX,Tx.fmtToX
                        Tx.reY      = Tx.Y; BlockFormat(Tx) end
  end
  local function TryBlockFormat(Tx)
    if Tx.reX then  BlockFormat(Tx) else Mk:XEQ(0xf5035) end
  end
  Mk["^J,F6"] = function(Tx,count)
        if           count then Tx.fmtFromX,Tx.fmtToX = 1,count+1
    elseif not Tx.fmtFromX then Tx.fmtFromX,Tx.fmtToX = 1,76 end
    Tx.reX,Tx.X = Tx.fmtFromX, Tx.fmtToX
    Tx.reY      = Tx.Y
    Mk["F6"]     = TryBlockFormat
    Mk["Ctrl+F6"] = DoBlockFormat
    Mk["Shift+F6"] = function(Tx)    DoBlockFormat(Tx)
                       while Tx.reX do BlockFormat(Tx) end end
  end
end
