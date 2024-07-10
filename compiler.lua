--a simple compiler lol
--WARNING: STUPID MISSPELINGS!!!!!

--I think this just outputs A to address $3000
local prg1 = [[
PSHL $0000
PSHL $41
SWP
PSHL $1
ADD
SWP
DUP2
STR $3000
SWP
DUP
ROT
ROT
PSHL $FF
NEQ
JPC $06
HLT
]]
--this just outputs the whole uppercase alphabet to the console output
local prg2 = [[
PSHL $0000
PSHL $41
STO $4000
DUP
PSHA $4000
STR $3000
PSHA $4000
INC
STO $4000
INC
PSHA $4000
PSHL $5B
NEQ
JPC $09
POP
PSHL $001B
PSHL $61
STO $4000
DUP
PSHA $4000
STR $3000
PSHA $4000
INC
STO $4000
INC
PSHA $4000
PSHL $7B
NEQ
JPC $2c
HLT
]]

--program 2 but fancy (plus lowercase)
local prg3 = [[
%main
	PSHL $0000
	PSHL &type-uppercase
	CAL
	PSHL &type-lowercase
	CAL
	HLT

%type-uppercase
	PSHL $41
	STO $4000
	%loop
		PSHL &type-letter
		CAL
		PSHA $4000
		PSHL $5B
		NEQ
		JPC &loop
		RET

%type-lowercase
	PSHL $61
	STO $4000
	%loop2
		PSHL &type-letter
		CAL
		PSHA $4000
		PSHL $7B
		NEQ
		JPC &loop2
		RET


%type-letter
	DUP
	PSHA $4000
	STR $3000
	PSHA $4000
	INC
	STO $4000
	INC
	RET
]]

local test = [[
PSHL $BEEF

PSHL &test
STO $3103

INT $03

PSHL &off-3
CAL

INT $03

PSHL $FEED

HLT

%off-3
	LIM
	PSHL $01
	LFS $03
	NOT
	AND
	SIM
	RET


%test
	PSHL "H
	PSHL &type-letter
	CAL
	PSHL "e
	PSHL &type-letter
	CAL
	PSHL "l
	PSHL &type-letter
	CAL
	PSHL "l
	PSHL &type-letter
	CAL
	PSHL "o
	PSHL &type-letter
	CAL
	RFI

%type-letter
	PSHA $4000
	SWP
	STR $3000
	PSHA $4000
	INC
	STO $4000
	RET

]]

local prg = test

local linked_exe = {}

local argz = {...}
local outfile = "out.rat"

if #argz>0 then
	prg = ""
	local mode = 0
	for i,v in ipairs(argz) do
		local begin = v:sub(1,1)
		if not(begin=="-") then
			if mode==0 then
				local f = assert(io.open(v, "rb"))

				prg = prg..f:read("*a").."\n"
				
				
				assert(f:close())
			elseif mode==1 then
				outfile = v
				mode = 0
			elseif mode==2 then
				linked_exe[#linked_exe+1] = v
				mode = 0
			end
		else
			if v == "--o" then
				mode = 1
			elseif v == "--l" then
				mode = 2
			
			end
		end
	end
	prg = prg:gsub("	", "")
	prg = prg:gsub("\n", " ")
	prg = prg:gsub("\r", " ")
	print(prg)

	print("\n")
	print("-------------------SIMPLE RAT ASM--------------------")
	print("-------------Programed by michealtheratz-------------")
	print("\n")
else
	prg = prg:gsub("	", "")
	prg = prg:gsub("\n", " ")
	print(prg)
	print("\n")
	print("-------------------SIMPLE RAT ASM--------------------")
	print("-------------Programed by michealtheratz-------------")
	print("\n")
end



--seperate string function
function rep( str,sep )
	if sep == nil then
    	sep = "%s"
    end
    local t={}
    for st in string.gmatch(str, "([^"..sep.."]+)") do
        table.insert(t, st)
    end
    return t
end

--Opcode tabel
local opcodes = {
	--hacky sh*t
	NIN = 0x0,
	NOP = 0x0,

	--rest should be pretty normal
	PSHL= 0x10,
	PSHA = 0x11,
	POP=0x12,
	SWH=0x14,
	STO=0x15,
	STR=0x16,
	SWP=0x17,
	ROT=0x18,
	DUP=0x19,
	OVR=0x1A,
	DUP2=0x1B,
	LIM= 0x1C,
	SIM= 0x1D,
	GTR= 0x1E,

	ADD= 0x20,
	SUB= 0x21,
	DIV= 0x22,
	MUL= 0x23,
	INC= 0x24,
	DEC= 0x25,
	AND= 0x26,
	OR= 0x27,
	XOR= 0x28,
	LFS= 0x29,
	RTS= 0x2a,
	NOT= 0x2B,

	CAL= 0x30,
	RET= 0x31,
	JMP= 0x32,
	JPC= 0x33,
	INT= 0x34,
	RFI= 0x35,
	JML= 0x36,

	EQU= 0x40,
	NEQ=0x41,
	GTH=0x42,
	LTH=0x43,

	HLT= 0xFF
}

local out = {}
local alias = {}

--replace newlines with spaces
prg = prg:gsub("\n", " ")
prg = prg:gsub("	", "")
--tokenize
local o = rep(prg," ")

--turn number into word (2 bytes)
function process_hex( fn )
	local out = {}
	--print(#fn)
	if #fn == 1 then
		--we will add a zero to the string
		out[#out+1] = 0x00
		out[#out+1] = tonumber("0"..fn,16)
	elseif #fn == 2 then
		--if hex is 8-bit, add a zero in front
		out[#out+1] = 0x00
		out[#out+1] = tonumber(fn,16)
	elseif #fn == 3 then
		local msb = fn:sub(1,1)
		local lsb = fn:sub(2,3)
		out[#out+1] = tonumber(msb,16)
		out[#out+1] = tonumber(lsb,16)
	elseif #fn == 4 then
		--split the 16-bit value
		local msb = fn:sub(1,2)
		local lsb = fn:sub(3,4)
		out[#out+1] = tonumber(msb,16)
		out[#out+1] = tonumber(lsb,16)
	else
		--dumbasses get this error
		error("Invalid value $"..fn)
		os.exit()
	end
	return out
end

local mode = true
local cm = false
local no = {}
--we will add non comments to no
for i,v in ipairs(o) do
	local begin = v:sub(1,1)
	local ending = v:sub(#v,#v)
	if (begin == "[") and (ending == "]") then
		mode = false
		cm = true
	else
		if begin == "[" then
			mode = false
			cm = false
	
		elseif (ending == "]") then
			
			cm = true
		end
	end
	
	
	

	--print(v, begin, ending)
	if mode then
		--print("add")
		no[#no+1] = v
	end
	

	if cm then
		mode = true
	end

end

o = no

--We will process labels first since they can be refrenced anywhere

local ao = 0
for i,v in ipairs(o) do
	local begin = v:sub(1,1)
	--we will add two to the address offset since these are 2 bytes
	if begin == "$" then
		ao = ao+1
	end
	if begin == "&" then
		ao = ao+1
	end
	if begin == '"' then
		ao = ao+1
	end
	--We will set this label to the respective memory address
	if begin=="%" then
		local name = v:sub(2,#v)
		alias[name] = (i-1)+ao
		--print(name, (i-1)+ao)
		
	end
	if begin=="@" then
		local name = v:sub(2,#v)
		alias[name] = ((i-1)+ao)
		print(name, ((i)+ao)+1)
		
	end
end




for i,v in ipairs(o) do
	local begin = v:sub(1,1)
	local ending = v:sub(#v,#v)
	local key = v
	local addr = i-1
	local val = 0x0
	local add = true

	if begin == "[" then
		mode = false
		cm = false
	
	elseif (begin == "]") or (ending == "]") then
		mode = true
		cm = false
	
	elseif (begin == "[") and (ending == "]") then
		mode = false
		cm = true
	end

	
	if begin == "$" then
		--handle hex values
		local fn = key:sub(2,#key)
		--print(fn)
		local p = process_hex( fn )
		out[#out+1] = p[1]
		val = p[2]
		


	elseif begin=="%" then

		--A label will add a 0 to the output. This was the only thing that worked lol.
		--(The vm basicly goes to the next instruction after a call/jump instruction)
		--The VM just skips over zeros (for now) so it should be fine
		--It's a bit hacky though
	elseif begin=="@" then
		add = false
	elseif begin == "&" then
		local name = key:sub(2,#key)
		if alias[name] ~= nil then
			local fn = string.format("%x",alias[name])
			local p = process_hex( fn )
			
			out[#out+1] = p[1]
			val = p[2]

			--It will output the least signifigant byte in the debug messages

		end
	elseif begin == '"' then
		local char = key:sub(2,2)
		local secchar = key:sub(3,3)
		local fn = nil
		
		if (char=="\\") and (secchar=="w") then
			fn = string.format("%x",string.byte(" "))
		elseif (char=="\\") and (secchar=="n") then
			fn = string.format("%x",string.byte("\n"))
		else
			fn = string.format("%x",string.byte(char))
		end

		local p = process_hex( fn )
			
		out[#out+1] = p[1]
		val = p[2]

		--It will output the least signifigant byte in the debug messages

		
	else
		if opcodes[key] ~= nil then
			--opcode has been found
			val = opcodes[key]
		end
	end

	if add then
		print(addr,key,"$"..string.format("%x",val))

		out[#out+1]=val
	end

end

for i,v in ipairs(linked_exe) do
	local f = assert(io.open(v, "rb"))

	local bytes = f:read("*a")
	for k,b in ipairs(bytes) do
		print(v)
		out[#out+1] = string.char(v)
	end
	
	assert(f:close())

end


local f = assert(io.open(outfile, "wb"))

for i,v in ipairs(out) do
	f:write(string.char(v))

end
assert(f:close())
local co = {}
for i,v in ipairs(out) do
	co[#co+1] = string.format("$%x",v)

end
print("\nCompiled output:\n")
print(table.unpack(co))
print("\n".."Saved to "..outfile)