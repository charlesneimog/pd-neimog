local granulator = pd.Class:new():register("x.gui.granulator")

-- ─────────────────────────────────────
function granulator:initialize(name, args)
	self.inlets = 1
	self.outlets = 1
	self:set_size(250, 100)

	if #args == 0 or type(args[1]) ~= "string" then
		self:error("Table name is required")
		return false
	end

	self.args = args
	self.array = pd.Table:new():sync(args[1])

	if not self.array then
		self:error("Table '" .. args[1] .. "' was not found")
		return false
	end

	-- grain
	self.timeline_ms = 0
	self.gran = false

	self.grain_count = 4

	-- Grain length in array samples
	local array_length = self.array:length()
	self.grain_size_min = math.max(1, math.floor(array_length * 0.02))
	self.grain_size_max = math.max(1, math.floor(array_length * 0.08))
	self.sample_rate = tonumber(args[2]) or 48000

	self.silence_min_ms = 5
	self.silence_max_ms = 20

	-- Independent interval range in milliseconds
	self.interval_min_ms = 100
	self.interval_max_ms = 250

	self.grain_position = {}
	self:reset_grains()

	self.clock = pd.Clock:new():register(self, "tic")

	return true
end
-- ─────────────────────────────────────
function granulator:reset_grains()
	local _, h = self:get_size()

	self.grain_position = {}

	for index = 1, self.grain_count do
		self.grain_position[index] = {
			0, -- x
			math.random(1, math.max(1, h - 6)), -- y
			1, -- width
			5, -- height
			1, -- stroke width
			self.timeline_ms + math.random(0, self.interval_max_ms), -- next trigger time
		}
	end

	self:repaint(2)
end

-- grains <amount>
-- ─────────────────────────────────────
function granulator:in_1_grains(atoms)
	local amount = math.floor(tonumber(atoms[1]) or 0)

	if amount < 1 then
		self:error("grains: amount must be at least 1")
		return
	end

	self.grain_count = amount
	self:reset_grains()
end

-- size <minimum-samples> <maximum-samples>
-- ─────────────────────────────────────
function granulator:in_1_size(atoms)
	local minimum = math.floor(tonumber(atoms[1]) or 0)
	local maximum = math.floor(tonumber(atoms[2]) or minimum)

	if minimum < 1 or maximum < 1 then
		self:error("size: values must be at least 1 sample")
		return
	end

	if minimum > maximum then
		minimum, maximum = maximum, minimum
	end

	self.grain_size_min = minimum
	self.grain_size_max = maximum
end

-- silence <minimum-ms> <maximum-ms>
-- ─────────────────────────────────────
function granulator:in_1_silence(atoms)
	local minimum = math.floor(tonumber(atoms[1]) or 0)
	local maximum = math.floor(tonumber(atoms[2]) or minimum)

	if minimum < 0 or maximum < 0 then
		self:error("silence: values cannot be negative")
		return
	end

	if minimum > maximum then
		minimum, maximum = maximum, minimum
	end

	self.silence_min_ms = minimum
	self.silence_max_ms = maximum
end

-- interval <minimum-ms> <maximum-ms>
-- ─────────────────────────────────────
function granulator:in_1_interval(atoms)
	local minimum = math.floor(tonumber(atoms[1]) or 0)
	local maximum = math.floor(tonumber(atoms[2]) or minimum)

	if minimum < 1 or maximum < 1 then
		self:error("interval: values must be at least 1 ms")
		return
	end

	if minimum > maximum then
		minimum, maximum = maximum, minimum
	end

	self.interval_min_ms = minimum
	self.interval_max_ms = maximum

	-- Reschedule every grain independently.
	for _, grain in ipairs(self.grain_position) do
		grain[6] = self.timeline_ms + math.random(minimum, maximum)
	end
end

-- ─────────────────────────────────────
function granulator:in_1_float(f)
	if f == 1 and not self.gran then
		self.gran = true
		self.timeline_ms = 0
		self:reset_grains()
		self.clock:delay(1)
	else
		self.gran = false
		self.clock:unset()
	end
end

-- ─────────────────────────────────────
function granulator:tic()
	if not self.gran then
		return
	end

	self.timeline_ms = self.timeline_ms + 1

	local w, h = self:get_size()
	local array_length = self.array:length()
	local changed = false

	if not array_length or array_length < 1 then
		self.clock:delay(1)
		return
	end

	local drawable_width = math.max(1, w - 4)
	local grain_height = 5
	local sample_rate = self.sample_rate or 48000
	local silence_min = self.silence_min_ms or 5
	local silence_max = self.silence_max_ms or 20

	for index, grain in ipairs(self.grain_position) do
		if self.timeline_ms >= grain[6] then
			local minimum = math.max(1, math.min(self.grain_size_min, array_length))
			local maximum = math.max(1, math.min(self.grain_size_max, array_length))

			if minimum > maximum then
				minimum, maximum = maximum, minimum
			end

			local sample_length

			if minimum == maximum then
				sample_length = minimum
			else
				sample_length = math.random(minimum, maximum)
			end

			local maximum_start = array_length - sample_length
			local sample_index = math.random(0, maximum_start)

			-- Convert the sample position and length to GUI coordinates.
			grain[1] = 2 + (sample_index / array_length) * drawable_width

			grain[2] = math.random(1, math.max(1, h - grain_height - 1))

			grain[3] = math.max(1, (sample_length / array_length) * drawable_width)

			grain[4] = grain_height

			-- Output:
			-- <grain index> <sample index> <sample length>
			self:outlet(1, "list", {
				index,
				sample_index,
				sample_length,
			})

			-- Calculate the grain duration in milliseconds.
			local grain_duration_ms = math.max(1, math.ceil((sample_length / sample_rate) * 1000))

			-- Add an independent silence after this grain.
			local silence_ms = math.random(silence_min, silence_max)

			grain[6] = self.timeline_ms + grain_duration_ms + silence_ms

			changed = true
		end
	end

	if changed then
		self:repaint(2)
	end

	-- Timeline resolution: 1 millisecond.
	self.clock:delay(1)
end

-- ─────────────────────────────────────
function granulator:paint(g)
	local w, h = self:get_size()

	g:set_color(245, 245, 245)
	g:fill_all()

	if not self.array then
		return
	end

	local len = self.array:length()

	if not len or len == 0 or w <= 4 or h <= 2 then
		return
	end

	local left = 2
	local right = w - 2
	local drawable_width = right - left
	local path = nil

	g:set_color(205, 205, 205)
	g:draw_line(2, h / 2, w - 2, h / 2, 2)

	for x = left, right do
		local position = (x - left) / drawable_width
		local index = math.floor(position * (len - 1))
		local sample = self.array:get(index) or 0

		-- Keep unexpected values inside the display.
		sample = math.max(-1, math.min(1, sample))

		-- Map -1..1 to bottom..top, with a one-pixel margin.
		local y = 1 + (1 - (sample + 1) / 2) * (h - 2)

		if not path then
			path = Path(x, y)
		else
			path:line_to(x, y)
		end
	end

	if path then
		g:set_color(0, 0, 0)
		g:stroke_path(path, 1)
	end
end

-- ─────────────────────────────────────
function granulator:paint_layer_2(g)
	g:set_color(255, 0, 0)

	for _, grain in ipairs(self.grain_position) do
		g:stroke_rect(grain[1], grain[2], grain[3], grain[4], grain[5])
	end
end

-- ─────────────────────────────────────
function granulator:in_1_reload()
	self:dofilex(self._scriptname)

	-- Keep the existing object state and clock.
	self:repaint()
end
