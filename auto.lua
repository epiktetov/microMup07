if load and not loadstring then  loadstring = load end
Mk["Ctrl+Shift+T"] = function(Tx) Tx:IC(os.date()) end
Mk["Ctrl+Shift++"] = function(Tx)
  local expr = Tx:gtl()
  if expr then local result = loadstring("local X="..expr..";return X")()
               Tx:IC("= "..tostring(result))                          end
end
for _,n in pairs{"F7","F8","F9"} do -- inserts current value of Fn macro into
  Mk["^J,"..n] = function(Tx)       --  current text (to edit and re-execute)
    local pref,Fn = "Mk."..n.."=",Mk[n]
    if type(Fn) == 'string' then Tx:IL(pref..'"'..Fn..'"')
                            else Tx:IL(pref..'none') end
  end
end
function Txt.openXref(Tx)        Tx.refs = { }       -- open new (throw-away)
  local newTx = Txt.open(false); Tx.refTx = newTx.id -- text cross-referenced
        newTx.refs = { };        newTx.refTx = Tx.id -- to given one

  Tx.Xref = function(self,txN,rxN)            -- Suggested usage:
    local Rx = self.refTx and Txt[self.refTx] --
    txN = txN or self.Y                       -- Mk["keyseq"] = function(Tx)
    rxN = rxN or Rx.Y-1; self.refs[txN] = rxN --   local Rx = Txt.openXref(Tx)
                           Rx.refs[rxN] = txN --   for N,line in Tx:lines() do
  end; return newTx                           --     if ... then
end                                           --       Rx:IL(smth); Tx:Xref(N)  end
Mk["Ctrl+X"] = function(Tx)                   --     end
  local  Rx = Tx.refTx and Txt[Tx.refTx]      --   end  --  use Ctrl+X to jump
  if not Rx then return Mk:Do(0xc1004) end    -- end    -- between these texts
  for i=Tx.Y,1,-1 do
    if Tx.refs[i] then Rx:focus(); Rx.Y = Tx.refs[i]; return end
  end
end
function MkSyncMarks(Rx,Tx) -- called from (tm)SyncPos
  local ref = Re[[([+./0-9A-z-]+):(\d+):((\d+):)?(.*)]]
  local gre = Re"grep -\\w+ \'([^\\']+)\'"; local N = 4
  local Tname, grept
  if ref:ifind(Rx:line(Rx.Y)) then Tname = ref:cap(1) else return end
  if gre:ifind(Rx:line(1))    then grept = gre:cap(1)             end
  Tx.refTx = Rx.id;Tx.refs = { }
  for Y,line in Rx:lines(Rx.Y) do
    if ref:ifind(line) then
        local name,y,_,x,text = ref:caps()
        if name == Tname and tonumber(y) then
          if grept  and  not tonumber(x) then
            local Ts = Tx:line(tonumber(y))
            x = Ts and Ts:find(grept); if x then text = "" end
          elseif text:find("error:")        then text = "!"..text
          elseif text:find("note:")         then text = "@"..text end
          Tx.refs[tonumber(y)] = Y
          Tx:mark(N, tonumber(x) or 2, y, text)
          N = N+1;   if N == 20 then return end
    end end
  end
  while N < 20 do Tx:mark(N,0); N = N+1 end
end
----------------------------------------------------------------------------
do local function breakable(c) return (c == ' ' or c == ',' or c == ';') end
   local function squeeze(line,afterN)
     return line:sub(1,afterN-1)..line:sub(afterN):gsub("%s%s+"," ")
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
    if Tx.reX then  BlockFormat(Tx) else Mk:Do(0xf5035) end
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
-----------------------------------------------------------------------
Mk["Ctrl+Shift+E"] = function(Tx,count) -- encode using Vigenère cipher
  local key = Tx:gtl(); Tx:go(1)
  local it = count or 2147483647
  for i=1,it do                              local orig =  Tx:line();
    if orig and orig:len() > 0 then Tx:DL(); Tx:IL(encode(orig,key));
                               else return                        end
  end
end                                     -- to encode/decode, insert line,
Mk["Ctrl+Shift+D"] = function(Tx,count) -- enter key: [0-9A-Za-z+\-]+ and
  local key = Tx:gtl(); Tx:go(1)        -- invoke Ctrl+Shift+E/+D command
  local it = count or 2147483647
  for i=1,it do                              local orig =  Tx:line();
    if orig and orig:len() > 0 then Tx:DL(); Tx:IL(decode(orig,key));
                               else return                        end
  end --
end   -- Esc N,Ctrl+Shift+E/+D to process N lines (stop at empty line / EOF)
----------------------------------------------------------------------------
function Mk2html(Tx)        -- convert MicroMir text (with ʁboldʀ etc) to HTML
  local Hx = Txt.open(true) --
  Hx:IL("auto-generated from `"..Tx.name.."` at "..os.date())
  Hx:IL("")
  Hx:IL[[<span style="font-family:Consolas,Liberation Mono,Menlo,monospace">]]
  Hx:IL[[<font size="3">]]
  for N,line in Tx:lines() do
    line = line:gsub("<","&lt;"); line = line:gsub("  ","&nbsp;&nbsp;")
    line = line:gsub(">","&gt;")
    line = line:gsub("\202\129(.-)\202\128","<b>%1</b>") -- bold
    line = line:gsub("\202\129(.+)","<b>%1</b>")
    line = line:gsub("\202\130(.-)\202\128","%1") -- prompt
    line = line:gsub("\202\136(.-)\202\128","%1") -- super (sky blue chars)
    Hx:IL(line.."<br>")
  end
  Hx:IL[[</font></span>]]
end
