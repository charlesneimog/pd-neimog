local orchidea = pd.Class:new():register("l.orchidea")
local csv = require("csv")

-- ─────────────────────────────────────
function orchidea:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	self.objects = {}

	if #args < 2 then
		self.instrument = args[1]
		self.technique = args[2]
	end

	local csvpath = pd._pathnames["l.orchidea"] .. ".csv"

	local data, err = self:parse_csv(csvpath)
	if not data then
		self:error(err)
	else
		self.f = data -- Store the parsed CSV data
	end

	-- Check if config file exists
	local fullpath = pd._pathnames["l.orchidea"] .. ".cfg"
	local file = io.open(fullpath, "r")
	if file then
		self.orchidea_path = file:read("*a")
		file:close()
	else
		self:error("[l.orchidea] Set orchidea path to use the object!")
	end

	return true
end

-- ─────────────────────────────────────
function orchidea:parse_csv(csvpath)
	local file = io.open(csvpath, "r")
	if not file then
		return nil, "File not found"
	end

	local data = {}
	local headers = {}

	-- Read each line from the CSV file
	for line in file:lines() do
		local row = {}
		-- Split each line by commas (assuming no commas inside quoted fields)
		for value in line:gmatch("([^,]+)") do
			table.insert(row, value)
		end

		-- The first line contains headers
		if not headers[1] then
			headers = row
		else
			-- Create a table with header names as keys and values from each row
			local entry = {}
			for i, header in ipairs(headers) do
				entry[header] = row[i]
			end
			table.insert(data, entry)
		end
	end

	file:close()
	return data
end

-- ─────────────────────────────────────
function orchidea:in_1_inst(args)
	self.instrument = args[1]
end

-- ─────────────────────────────────────
function orchidea:in_1_tech(args)
	self.technique = args[1]
end
-- ─────────────────────────────────────
function orchidea:in_1_note(args)
	if #args < 1 then
		self:error("Missing arguments")
	end

	local note, dyn
	if #args == 1 then
		note = args[1]
	elseif #args == 2 then
		note = args[1]
		dyn = args[2]
	end

	local filtered = {}
	for _, row in ipairs(self.f) do
		if
			row["Instrument (in full)"] == self.instrument
			and row["Pitch"] == note
			and row["Dynamics"] == dyn
			and row["Technique (in full)"] == self.technique
		then
			table.insert(filtered, self.orchidea_path .. row["Path"])
		end
	end

	if #filtered > 0 then
		self:outlet(1, "list", filtered)
	else
		self:error("No matching records found for Instrument: " .. self.instrument .. " and Pitch: " .. note)
	end
end

-- ─────────────────────────────────────
function orchidea:in_1_setpath(arg)
	local fullpath = pd._pathnames["l.orchidea"] .. ".cfg"
	local path = arg[1]
	for i = 2, #arg do
		path = path .. " " .. arg[i]
	end
	local file = io.open(fullpath, "w")
	if file then
		file:write(path)
		file:close() -- Ensure file is properly saved
	else
		pd.error("Failed to open file: " .. fullpath)
	end
end

function orchidea:in_1_reload()
	self:dofilex(self._scriptname)
end
