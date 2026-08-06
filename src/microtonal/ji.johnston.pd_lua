local johnston = pd.Class:new():register("ji.johnston")

-- ─────────────────────────────────────
function johnston:initialize(_, atoms)
	self.inlets = 3
	self.outlets = 1

	self.ratio = 3 / 2
	if #atoms == 3 then
		if type(atoms[1]) == "string" then
			local x = atoms[1]
			local a, b = x:match("^(%-?%d+)%s*/%s*(%-?%d+)$")
			self.ratio = a / b
		elseif type(atoms[1]) == "number" then
			self.ratio = atoms[1]
		end

		self.ratio = type(atoms[1]) == "number" and atoms[1] or 3 / 2
		self.sobreposition = type(atoms[2]) == "number" and atoms[2] or 3
		self.fundamental = type(atoms[3]) == "number" and atoms[3] or 72

		self.fundamental_freq = self:midi2freq(self.fundamental)
	end

	return true
end

-- ─────────────────────────────────────
function johnston:midi2freq(midi)
	-- Standard fractional MIDI note to frequency conversion.
	return 440 * (2 ^ ((midi - 69) / 12))
end

-- ─────────────────────────────────────
-- Helper: Convert frequency to a fractional MIDI note.
function johnston:freq2midi(freq)
	return 69 + 12 * math.log(freq / 440) / math.log(2)
end

-- ─────────────────────────────────────
-- Helper: Generate arithmetic sequence (1, 2, 3, ..., n)
function johnston:arithm_ser(start, count, step)
	local result = {}
	for i = 1, count do
		result[i] = start + (i - 1) * step
	end
	return result
end

-- ─────────────────────────────────────
-- Main processing method
function johnston:process()
	-- Utonal interval (inverse of the ratio)
	local utonal_sobr = (1 / self.ratio)

	-- Generate sobreposition sequence: 1, 2, 3, ..., sobreposition
	local sobr_seq = self:arithm_ser(1, self.sobreposition, 1)

	-- Calculate Otonal sobreposition (stacking above fundamental)
	-- Multiply fundamental frequency by ratio^n for each n
	local otonal = {}
	for i = 1, #sobr_seq do
		local factor = self.ratio ^ sobr_seq[i]
		local freq = self.fundamental_freq * factor
		otonal[i] = self:freq2midi(freq)
	end

	-- Calculate Utonal sobreposition (stacking below fundamental)
	-- Multiply fundamental frequency by (1/ratio)^n for each n
	local utonal = {}
	for i = 1, #sobr_seq do
		local factor = utonal_sobr ^ sobr_seq[i]
		local freq = self.fundamental_freq * factor
		utonal[i] = self:freq2midi(freq)
	end

	-- Output the combined list: utonal + fundamental + otonal
	-- (as per the original OM function)
	local result = {}
	local idx = 1

	-- Add utonal overtones (descending)
	for i = 1, #utonal do
		result[idx] = utonal[i]
		idx = idx + 1
	end

	-- Add fundamental
	result[idx] = self.fundamental
	idx = idx + 1

	-- Add otonal overtones (ascending)
	for i = 1, #otonal do
		result[idx] = otonal[i]
		idx = idx + 1
	end

	return result
end

-- ─────────────────────────────────────
-- Inlet handlers
function johnston:in_1_ratio(ratio)
	if type(ratio) == "number" and ratio > 0 then
		self.ratio = ratio
		pd.post(string.format("johnston: ratio set to %g", self.ratio))
	else
		self:error(string.format("johnston: invalid ratio %s", tostring(ratio)))
	end
end

-- ─────────────────────────────────────
function johnston:in_2_sobreposition(n)
	if type(n) == "number" and n >= 1 then
		self.sobreposition = math.floor(n)
		pd.post(string.format("johnston: sobreposition set to %d", self.sobreposition))
	else
		self:error(string.format("johnston: invalid sobreposition %s", tostring(n)))
	end
end

-- ─────────────────────────────────────
function johnston:in_3_fundamental(midi)
	if type(midi) == "number" then
		self.fundamental = midi
		self.fundamental_freq = self:midi2freq(midi)
		pd.post(string.format("johnston: fundamental set to MIDI %g", self.fundamental))
	else
		self:error(string.format("johnston: invalid fundamental %s", tostring(midi)))
	end
end

-- ─────────────────────────────────────
-- Bang triggers output
function johnston:in_1_bang()
	local result = self:process()
	-- Output as a list
	self:outlet(1, "list", result)
end

-- ─────────────────────────────────────
-- Float on first inlet triggers processing
function johnston:in_1_float(ratio)
	self:in_1_ratio(ratio)
	self:in_1_bang()
end

-- ─────────────────────────────────────
-- Bang handler for other inlets
function johnston:in_2_bang()
	self:in_1_bang()
end

-- ─────────────────────────────────────
function johnston:in_3_bang()
	self:in_1_bang()
end

-- ─────────────────────────────────────
-- Reload support for live coding
function johnston:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize(nil, {})
end
