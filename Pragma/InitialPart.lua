--------------------------------------------------------------------------------------
--  Aerials Initial Part
--     We document and prepare some global variables used in Aerials here,
--     And require this "module" in the render script.
--  Copyright (c) 2024- 1dealGas, under the MIT License.
--------------------------------------------------------------------------------------


-- Input & GUI System
--
WindowActive = true
WithinInterlude = true
LayoutModeLandscape = false

CurrentGuiX = 0
CurrentGuiY = 0
CurrentGuiPhase = nil   -- 0:Pressed, 1:OnScreen, 2:Released, 3:Invalid

Nodes = AcUtil.NewTable(8, 0)
NodeMaxIndex = 0
NodeCount = 0
--
-- GuiDoFeedback(with_hitsound)
-- InterludeIn(fn, is_async)
-- InterludeOut(fn)
--
function AppendNode(url)
	NodeCount, NodeMaxIndex = NodeCount+1, NodeMaxIndex+1
	Nodes[NodeMaxIndex]	= url or msg.url("#")
	return NodeMaxIndex
end

function RemoveNode(idx)
	NodeCount, Nodes[idx] = NodeCount-1, false
	if NodeCount==0 then NodeMaxIndex=0 end
end


-- Track Managing & Resources Related
--
-- Tracks = {}                   -- Unused in the Demo
CurrentTrackId = 1011
CurrentTrackIndex = 0            -- Unused in the Demo

CurrentAudioRes = nil
CurrentAudioUnit = nil
CurrentAudioBuffer = nil

CurrentPlate = nil               -- Unused in the Demo
CurrentFumenScript = nil         -- Unused in the Demo


-- Fumen Context & Scoring
--
ContextTime = nil                -- msTime or nil
WishSprites = nil                -- No need to expose other context info here
Hit, Early, Late = 0, 0, 0
TotalJud = 0


-- User Options
--
OffsetType = 1
AudioLatency, InputDelta = 0, 0
HapticFeedbackEnabled = true
HitSoundEnabled = false           -- Unused in the Demo


-- Save(nil before the initialization) & Credits
--
local B64 = require("Reference/lbase64")
local SAVE_PATH = sys.get_save_file("Aerials Demo", "SAVE")
function CopyCredit() clipboard.copy( sys.load_resource("/Ar.license") ) end
function ExportSave() clipboard.copy( B64.encode( sys.serialize(Save) ) ) end
function ImportSave()
	local OK, SaveStr = pcall( B64.decode, clipboard.paste() )
	local OK, SaveTable = pcall(sys.deserialize, SaveStr)

	if OK and SaveTable.Aerials and SaveTable.Aerials=="Save" then
		ExportSave()
		Save = SaveTable

		OffsetType = Save.Options.OffsetType
		AudioLatency = (OffsetType==1 and Save.Options.AudioLatency1) or (OffsetType==2 and Save.Options.AudioLatency2) or Save.Options.AudioLatency3
		HapticFeedbackEnabled, HitSoundEnabled = Save.Options.HapticFeedbackEnabled, Save.Options.HitSoundEnabled

		InputDelta = Save.Options.InputDelta
		Arf2.SetIDelta(InputDelta)
		return true
	else
		return false
	end
end

function SyncSave(options_updated)
	if options_updated then
		Save.Options.OffsetType = OffsetType
		Save.Options.InputDelta = InputDelta
		Save.Options.HitSoundEnabled = HitSoundEnabled
		Save.Options.HapticFeedbackEnabled = HapticFeedbackEnabled
		if OffsetType == 1 then			Save.Options.AudioLatency1 = AudioLatency
		elseif OffsetType == 2 then		Save.Options.AudioLatency2 = AudioLatency
		else							Save.Options.AudioLatency3 = AudioLatency
		end
	end
	sys.save(SAVE_PATH, Save)
end


-- FumenScript
-- Provide the TriggerFns table like this, or nil:
--    {
--        10000, function(canvas) end,   -- TriggerMs, TriggerFn
--        20000, function(canvas) end
--    }
--
-- Provide TaskFns table like this, or nil:
--    {
--        10000, 18000, function(canvas) end,   -- StartMs, EndMs, TaskFn
--        12000, 24000, function(canvas) end
--    }
--
local hash, type, gm, F = hash, type, debug.getmetatable, "function"
local function is_callable(x)
	if x then
		if type(x) == F then									return true
		else
			local meta = gm(x)
			if meta.__call and type(meta.__call) == F then		return true
			end
		end
	end															return false
end
function DeclareFumenScript(FmInitFn, FmFinalFn, TriggerFns, TaskFns, SpecialHintJudgedFn)
	TaskFns = type(TaskFns)=="table" and TaskFns or {}
	TriggerFns = type(TriggerFns)=="table" and TriggerFns or {}

	local trigger_count, task_count = #TriggerFns/2, #TaskFns/3
	local nt, time_sorter = AcUtil.NewTable, function(a,b) return a[1]<b[1] end

	-- Sort Triggers
	--
	local cnt = 1
	local trigger_tables = nt(trigger_count, 0)
	for i=1, #TriggerFns, 2 do
		if is_callable(TriggerFns[i+1]) then
			trigger_tables[cnt] = { TriggerFns[i], TriggerFns[i+1] }      -- TriggerMs, TriggerFn
			cnt = cnt + 1
		end
	end
	table.sort(trigger_tables, time_sorter)

	-- Lump Tasks
	--
	local cnt = 1
	local task_tables = nt(task_count, 0)
	for i=1, #TaskFns, 3 do
		if (TaskFns[i]<=TaskFns[i+1]) and is_callable(TaskFns[i+2]) then
			task_tables[cnt] = { TaskFns[i], TaskFns[i+1], TaskFns[i+2] }   -- StartMs, EndMs, TaskFn
			cnt = cnt + 1
		else
			task_count = task_count - 1
		end
	end
	table.sort(task_tables, time_sorter)   -- Sort by StartMs

	-- Create Registers & Unregisters
	--
	local register_tables, unregister_tables = nt(task_count, 0), nt(task_count, 0)
	for i=1, task_count do
		register_tables[i] = {task_tables[i][1], task_tables[i][3]}   -- StartMs, TaskFn
		unregister_tables[i] = {task_tables[i][2], i}   -- EndMs, UnregisterWhich
	end
	table.sort(unregister_tables, time_sorter)   -- register_tables sorted

	-- Expand Stuff
	--
	local Trigger = nt(trigger_count*2, 0)   -- Alternative TriggerMs, TriggerFn
	local Register = nt(task_count*2, 0)   -- Alternative StartMs, RegisterFn
	local Unregister = nt(task_count*2, 0)   -- Alternative EndMs, UnregisterWhich

	for i=1, trigger_count do
		Trigger[i*2-1] = trigger_tables[i][1]
		Trigger[i*2] = trigger_tables[i][2]
	end

	for i=1, task_count do
		Register[i*2-1] = register_tables[i][1]
		Register[i*2] = register_tables[i][2]
		Unregister[i*2-1] = unregister_tables[i][1]
		Unregister[i*2] = unregister_tables[i][2]
	end

	TriggerFns, TaskFns = nil, nil
	trigger_tables, task_tables = nil, nil
	register_tables, unregister_tables = nil, nil

	-- Do "Macro-Like" Stuff
	--
	do
		local TriggerWhich, RegisterWhich, UnregisterWhich = 1, 1, 1
		local Tasks, TaskCount, TaskMaxIndex, UPDATE, FINAL = nt(task_count,0), 0, 0
		local Trigger, Register, Unregister = Trigger, Register, Unregister

		if is_callable(FmFinalFn) then
			FINAL = function(canvas)
				FmFinalFn(canvas)
				TriggerWhich, RegisterWhich, UnregisterWhich = 1, 1, 1
				Tasks, TaskCount, TaskMaxIndex = nt(task_count,0), 0, 0
			end
		else
			FINAL = function()
				TriggerWhich, RegisterWhich, UnregisterWhich = 1, 1, 1
				Tasks, TaskCount, TaskMaxIndex = nt(task_count,0), 0, 0
			end
		end

		if task_count > 0 then
			UPDATE = function(canvas)   -- Will be called only when ContextTime exists
				local ContextTime = ContextTime
				local current_trigger_time = Trigger[TriggerWhich]                             -- Trigger
				while (current_trigger_time  and  ContextTime > current_trigger_time) do
					TriggerWhich = TriggerWhich + 2
					Trigger[TriggerWhich - 1](canvas)
					current_trigger_time = Trigger[TriggerWhich]
				end

				local current_register_time = Register[RegisterWhich]                          -- Register
				while (current_register_time  and  ContextTime > current_register_time) do
					TaskCount = TaskCount + 1
					TaskMaxIndex = TaskMaxIndex + 1
					RegisterWhich = RegisterWhich + 2
					Tasks[TaskMaxIndex] = Register[RegisterWhich - 1]
					current_register_time = Register[RegisterWhich]
				end

				for i=TaskMaxIndex, 1, -1 do                                                   -- Do Tasks
					if Tasks[i] then					Tasks[i](canvas)
					elseif i == TaskMaxIndex then		TaskMaxIndex = i - 1
					end
				end

				local current_unregister_time = Unregister[UnregisterWhich]                    -- Unregister
				while (current_unregister_time  and  ContextTime > current_unregister_time) do
					TaskCount = TaskCount - 1
					UnregisterWhich = UnregisterWhich + 2
					if TaskCount == 0 then TaskMaxIndex = 0 end
					Tasks[ Unregister[UnregisterWhich - 1] ] = false
					current_unregister_time = Unregister[UnregisterWhich]
				end
			end
		else
			UPDATE = function(canvas)
				local ContextTime, current_trigger_time = ContextTime, Trigger[TriggerWhich]                             -- Trigger
				while (current_trigger_time  and  ContextTime > current_trigger_time) do
					TriggerWhich = TriggerWhich + 2
					Trigger[TriggerWhich - 1](canvas)
					current_trigger_time = Trigger[TriggerWhich]
				end
			end
		end

		local MSG_FUNCS = {
			[hash("ar_init")] = is_callable(FmInitFn) and FmInitFn or nil,
			[hash("ar_special_hint_judged")] = is_callable(SpecialHintJudgedFn) and SpecialHintJudgedFn or nil,
			[hash("ar_update")] = UPDATE,
			[hash("ar_final")] = FINAL
		}

		if is_callable(init) then
			local original_init = init
			init = function(self)
				original_init(self)
				CurrentFumenScript = msg.url("#")
			end
		else
			function init() CurrentFumenScript = msg.url("#") end
		end

		if is_callable(final) then
			local original_final = final
			final = function(self)
				original_final(self)
				if CurrentFumenScript.fragment==msg.url("#").fragment then CurrentFumenScript=nil end
			end
		else
			function final()  if CurrentFumenScript.fragment==msg.url("#").fragment then CurrentFumenScript=nil end  end
		end

		if is_callable(on_message) then
			local original_on_message = on_message
			on_message = function(self, message_id, message, sender)
				if MSG_FUNCS[message_id] then MSG_FUNCS[message_id](sender) end
				original_on_message(self, message_id, message, sender)
			end
		else
			function on_message(self, message_id, message, sender)
				if MSG_FUNCS[message_id] then MSG_FUNCS[message_id](sender) end
			end
		end
	end
		collectgarbage()
end


-- Tween based on FumenScript & go.animate
-- Declare Tween Keyframes & Inject them like this:
--     Tween(url, property)(   -- Supports all Defold-Style Syntax for arg url & property
--         ms1, value1, et1,
--         ms2, value2, et2,
--         ···
--     )
--     ···
--     DeclareFumenScript(···, ···, Tweens / TriggerFns, ···, ···)
--
-- The Singleton "Tween" is NOT A TABLE, so passing it directly will cause all tweens declared above INVALID
--  and FAILED TO GET RESETED PROPERLY. Just keep the "Tween / TriggerFns" clause if no TriggerFns needed,
--  which should be safe when TriggerFns == nil.
--
local tween_cache, tween_capacity = {}, 0
local msg_post, sprite_play_flipbook = msg.post, sprite.play_flipbook
local STRING, ENABLE, DISABLE = "string", hash("enable"), hash("disable")

Tween = debug.setmetatable( AcUtil.PushNullptr(), {
	__call = function(url, property)   -- Declare a Tween
		property = (type(property)==STRING) and hash(property) or property
		return function(...)
			local decl, go_set, go_animate, go_cancel_animations = {...}, go.set, go.animate, go.cancel_animations
			for i=1, #decl, 3 do
				tween_cache[tween_capacity+1] = decl[i]
				if decl[i+3] then
					local tovalue, easetype, delta_second = decl[i+1], decl[i+2], (decl[i+3]-decl[i])/1000
					tween_cahce[tween_capacity+2] = function(canvas)
						url = url or canvas   -- Safe.
						go_cancel_animations(url, property)
						go_animate(url, property, 1, tovalue, easetype, delta_second)
					end
				else
					local final_value = decl[i+1]
					tween_cahce[tween_capacity+2] = function(canvas)
						url = url or canvas   -- Safe.
						go_cancel_animations(url, property)
						go_set(url, property, final_value)
					end
				end
					tween_capacity = tween_capacity + 2
			end
		end
	end,

	__div = function(lnum, rnum)   -- Merge Tweens
		local merge_target = (type(lnum)=="table" and lnum) or (type(rnum)=="table" and rnum) or AcUtil.NewTable(#tween_cache, 0)
		local merge_target_size = #merge_target
		for i=1, tween_capacity do  merge_target[merge_target_size + i] = tween_cache[i]  end
		tween_cache, tween_capacity = {}, 0
		return merge_target
	end
})
Tweens = Tween


-- TriggerFn Shorthands
-- Example:
--     TriggerFns = { 666, TriggerEnable("#x", "#y") }
--     TriggerFns = { 114514, TriggerPlayFlipbook("#m","#n")("some_anim", NO_CALLBACK, {playback_rate=2}) }
--
local function tpf_rtn_1(url)
	return function(u,v,w)		return function() sprite_play_flipbook(url,u,v,w) end									end
end
local function tpf_rtn_m(urls, urllen)
	return function(u,v,w)		return function()  for i=1, urllen do sprite_play_flipbook(urls[i],u,v,w) end  end		end
end

function TriggerEnable(...)
	local urls = {...}			local urllen = #urls
	if urllen == 1 then			return function() msg_post(urls[1], ENABLE) end
	else						return function() for i=1, urllen do msg_post(urls[i], ENABLE) end end
	end
end
function TriggerDisable(...)
	local urls = {...}			local urllen = #urls
	if urllen == 1 then			return function() msg_post(urls[1], DISABLE) end
	else						return function() for i=1, urllen do msg_post(urls[i], DISABLE) end end
	end
end
function TriggerPlayFlipbook(...)
	local urls = {...}			local urllen = #urls
	if urllen == 1 then			return tpf_rtn_1(urls[1])
	else						return tpf_rtn_m(urls, urllen)
	end
end


-- Function Concat Operator
--
function FuncConcatDisable()  debug.setmetatable(type, nil)  end
function FuncConcatEnable(trigger_only)
	local assert = assert
	if trigger_only then
		debug.setmetatable(assert, {
			__concat = function(l, r)
				assert(is_callable(l), "FuncConcat: The left operand is not callable")
				assert(is_callable(r), "FuncConcat: The right operand is not callable")
				return (function() l() r() end)
			end
		})
	else
		debug.setmetatable(assert, {
			__concat = function(l, r)
				assert(is_callable(l), "FuncConcat: The left operand is not callable")
				assert(is_callable(r), "FuncConcat: The right operand is not callable")
				return (function(...)  return { l(...) }, { r(...) }  end)
			end
		})
	end
end


-- Initialization
--
do
	FuncConcatEnable(true)
	Save = sys.load(SAVE_PATH)

	if not Save.Aerials then
		Save = {
			Aerials = "Save",  Wish = 0,  Hint = {},  Challenges = {},
			Options = {
				OffsetType = 1, 
				AudioLatency1 = 0,
				AudioLatency2 = 0,
				AudioLatency3 = 0,
				InputDelta = 0,
				HapticFeedbackEnabled = true,
				HitSoundEnabled = false
			}
		}
		sys.save(SAVE_PATH, Save)
	end

	OffsetType = Save.Options.OffsetType
	AudioLatency = (OffsetType==1 and Save.Options.AudioLatency1) or (OffsetType==2 and Save.Options.AudioLatency2) or Save.Options.AudioLatency3
	HapticFeedbackEnabled = Save.Options.HapticFeedbackEnabled
	HitSoundEnabled = Save.Options.HitSoundEnabled

	InputDelta = Save.Options.InputDelta
	Arf2.SetIDelta(InputDelta)
end