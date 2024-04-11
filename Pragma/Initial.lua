--------------------------------------------------------------------------------------
--  Aerials Initial Part
--     We document and prepare some global variables used in Aerials here,
--     And require this "module" in the render script.
--  Copyright (c) 2024- 1dealGas, under the MIT License.
--------------------------------------------------------------------------------------
-- Interlude
--                               -- Order of interlude calls:
InterludeCallbackIn = nil        -- InterludeIn -> InterludeCallbackIn
InterludeCallbackOut = nil       --             -> InterludeOut -> InterludeCallbackOut


-- Input & GUI System
--
-- Rule 1:
--    Phase 0 / Pressed
--    Phase 1 / OnScreen
--    Phase 2 / Released
--
-- Rule 2:
--    When adding a GUI Node, cache its index and update NodeMaxIndex,
--    and then increase the NodeCount, like this:
--        NodeMaxIndex = NodeMaxIndex + 1
--        local insert_index	= NodeMaxIndex
--        Nodes[insert_index]	= msg.url()
--        NodeCount = NodeCount + 1
--
-- Rule 3:
--    Remove a GUI Node like this:
--        Nodes[insert_index] = nil
--        NodeCount = NodeCount - 1
--    Then it's ok to do other clean-ups.
--
WindowActive = true
WithinInterlude = true
LayoutModeLandscape = false

Nodes = AcUtil.NewTable(8, 0)
NodeMaxIndex = 0
NodeCount = 0

CurrentGuiX = 0
CurrentGuiY = 0
CurrentGuiPhase = nil
-- There is also a GuiDoFeedback() function.


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


-- FumenScript
-- Provide the TriggerFns table like this, or nil:
--    {
--        10000, function(canvas) end,   -- TriggerMs, TriggerFn
--        20000, function(canvas) end
--    }
-- Provide TaskFns table like this, or nil:
--    {
--        10000, 18000, function(canvas) end,   -- StartMs, EndMs, TaskFn
--        12000, 24000, function(canvas) end
--    }
--
function DeclareFumenScript(FmInitFn, FmFinalFn, TriggerFns, TaskFns, SpecialHintJudgedFn)
	local type, nt = type, AcUtil.NewTable
	local table_sort, time_sorter = table.sort, function(a,b) return a[1]<b[1] end

	TriggerFns = type(TriggerFns)=="table" and TriggerFns or {}
	TaskFns = type(TaskFns)=="table" and TaskFns or {}
	local trigger_count, task_count = #TriggerFns/2, #TaskFns/3

	-- Sort Triggers
	--
	local cnt = 1
	local trigger_tables = nt(trigger_count, 0)
	for i=1, #TriggerFns, 2 do
		trigger_tables[cnt] = { TriggerFns[i], TriggerFns[i+1] }      -- TriggerMs, TriggerFn
		cnt = cnt + 1
	end
	table_sort(trigger_tables, time_sorter)

	-- Lump Tasks
	--
	local cnt = 1
	local task_tables = nt(task_count, 0)
	for i=1, #TaskFns, 3 do
		if TaskFns[i] <= TaskFns[i+1] then
			task_tables[cnt] = { TaskFns[i], TaskFns[i+1], TaskFns[i+2] }   -- StartMs, EndMs, TaskFn
			cnt = cnt + 1
		else
			task_count = task_count - 1
		end
	end
	table_sort(task_tables, time_sorter)   -- Sort by StartMs

	-- Create Registers & Unregisters
	--
	local register_tables, unregister_tables = nt(task_count, 0), nt(task_count, 0)
	for i=1, task_count do
		register_tables[i] = {task_tables[i][1], task_tables[i][3]}   -- StartMs, TaskFn
		unregister_tables[i] = {task_tables[i][2], i}   -- EndMs, UnregisterWhich
	end
	table_sort(unregister_tables, time_sorter)   -- register_tables sorted

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
		local Tasks, TaskCount, TaskMaxIndex, FINAL = nt(task_count,0), 0, 0
		local TriggerWhich, RegisterWhich, UnregisterWhich, FUNC = 1, 1, 1, "function"
		local Trigger, Register, Unregister = Trigger, Register, Unregister

		if type(FmFinalFn) == FUNC then
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

		local MSG_FUNCS = {
			[hash("ar_init")] = type(FmInitFn)==FUNC and FmInitFn or nil,
			[hash("ar_special_hint_judged")] = type(SpecialHintJudgedFn)==FUNC and SpecialHintJudgedFn or nil,
			[hash("ar_update")] = function(canvas)   -- Will be called only when ContextTime exists
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
					Tasks[ Unregister[UnregisterWhich - 1] ] = nil
					current_unregister_time = Unregister[UnregisterWhich]
				end
			end,

			[hash("ar_final")] = FINAL
		}

		if type(init)==FUNC then
			local original_init = init
			init = function(self)
				original_init(self)
				CurrentFumenScript = msg.url("#")
			end
		else
			function init() CurrentFumenScript = msg.url("#") end
		end

		if type(final)==FUNC then
			local original_final = final
			final = function(self)
				original_final(self)
				if CurrentFumenScript.fragment==msg.url("#").fragment then CurrentFumenScript=nil end
			end
		else
			function final()  if CurrentFumenScript.fragment==msg.url("#").fragment then CurrentFumenScript=nil end  end
		end

		if type(on_message)==FUNC then
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


-- Initialization
--
do
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