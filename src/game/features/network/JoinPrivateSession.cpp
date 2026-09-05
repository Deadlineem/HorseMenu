#include "core/commands/Command.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/ScriptGlobal.hpp"

namespace YimMenu::Features
{
	// Session switcher command
	class SwitchToPrivateFreeroam : public Command
	{
		using Command::Command;

		virtual void OnCall() override
		{
			// JOAAT hashes for session types
			const int FREEROAM = Joaat("FREEROAM");           // -1557193583
			const int PRIVATE = Joaat("PRIVATE");             // 889819601
			const int SINGLE_PLAYER = Joaat("SINGLE_PLAYER"); // 1530881170

			// Global offsets based on your script
			// These are from Global_1572887 and Global_1572864
			const int SESSION_TARGET = 1102813 + 3919; // Global_1572887.f_405
			const int SESSION_PARAM = 1102813 + 3920;  // Global_1572887.f_405.f_1[0]
			const int SESSION_FLAGS = 1102813 + 3917;  // Global_1572864.f_3
			const int SESSION_TYPE = 1102813 + 3918;   // Global_1572864.f_1

			// Clear previous session parameters (f_1 array size 3)
			for (int i = 0; i < 3; i++)
			{
				*ScriptGlobal(SESSION_PARAM + i).As<int*>() = 0;
			}

			// Set the target session to FREEROAM
			*ScriptGlobal(SESSION_TARGET).As<int*>() = FREEROAM;

			// Set parameter to PRIVATE
			*ScriptGlobal(SESSION_PARAM).As<int*>() = PRIVATE;

			// Set session flags (68 = online mode)
			*ScriptGlobal(SESSION_FLAGS).As<int*>() = 68;

			// Set transition type (3 = online transition)
			*ScriptGlobal(SESSION_TYPE).As<int*>() = 3;

			// Optional: Fade out screen for smooth transition
			// CAM::DO_SCREEN_FADE_OUT(0);
			// MISC::SET_BIT(&Global_1572864.f_3, 0); // If you want to force the switch
		}
	};

	// Alternative: Switch to public freeroam
	class SwitchToPublicFreeroam : public Command
	{
		using Command::Command;

		virtual void OnCall() override
		{
			const int FREEROAM = Joaat("FREEROAM");

			const int SESSION_TARGET = 1102813 + 3919;
			const int SESSION_PARAM = 1102813 + 3920;
			const int SESSION_FLAGS = 1102813 + 3917;
			const int SESSION_TYPE = 1102813 + 3918;

			// Clear parameters
			for (int i = 0; i < 3; i++)
			{
				*ScriptGlobal(SESSION_PARAM + i).As<int*>() = 0;
			}

			// Set target to FREEROAM with no parameters (public)
			*ScriptGlobal(SESSION_TARGET).As<int*>() = FREEROAM;
			*ScriptGlobal(SESSION_FLAGS).As<int*>() = 68;
			*ScriptGlobal(SESSION_TYPE).As<int*>() = 3;
		}
	};

	// Switch to single player
	class SwitchToSinglePlayer : public Command
	{
		using Command::Command;

		virtual void OnCall() override
		{
			const int SINGLE_PLAYER = Joaat("SINGLE_PLAYER");

			const int SESSION_TARGET = 1102813 + 3919;
			const int SESSION_PARAM = 1102813 + 3920;
			const int SESSION_FLAGS = 1102813 + 3917;
			const int SESSION_TYPE = 1102813 + 3918;

			// Clear parameters
			for (int i = 0; i < 3; i++)
			{
				*ScriptGlobal(SESSION_PARAM + i).As<int*>() = 0;
			}

			// Set target to SINGLE_PLAYER
			*ScriptGlobal(SESSION_TARGET).As<int*>() = SINGLE_PLAYER;
			*ScriptGlobal(SESSION_FLAGS).As<int*>() = 4; // Single player flags
			*ScriptGlobal(SESSION_TYPE).As<int*>() = 2;  // Single player transition
		}
	};

	// Switch to mission creator
	class SwitchToMissionCreator : public Command
	{
		using Command::Command;

		virtual void OnCall() override
		{
			const int MISSION_CREATOR = Joaat("MISSION_CREATOR");
			const int SHIFT_F = Joaat("SHIFT_F");

			const int SESSION_TARGET = 1102813 + 3919;
			const int SESSION_PARAM = 1102813 + 3920;
			const int SESSION_FLAGS = 1102813 + 3917;
			const int SESSION_TYPE = 1102813 + 3918;

			// Clear parameters
			for (int i = 0; i < 3; i++)
			{
				*ScriptGlobal(SESSION_PARAM + i).As<int*>() = 0;
			}

			// Set target to MISSION_CREATOR with SHIFT_F parameter
			*ScriptGlobal(SESSION_TARGET).As<int*>() = MISSION_CREATOR;
			*ScriptGlobal(SESSION_PARAM).As<int*>() = SHIFT_F;
			*ScriptGlobal(SESSION_FLAGS).As<int*>() = 68; // Online mode
			*ScriptGlobal(SESSION_TYPE).As<int*>() = 3;   // Online transition
		}
	};

	// Alternative: Switch using raw hashes (bypass Joaat)
	class SwitchPrivateFreeroamRaw : public Command
	{
		using Command::Command;

		virtual void OnCall() override
		{
			// Using the exact values from your decompiled script
			const int SESSION_TARGET = 1102813 + 3919;
			const int SESSION_PARAM = 1102813 + 3920;
			const int SESSION_FLAGS = 1102813 + 3917;
			const int SESSION_TYPE = 1102813 + 3918;

			// Clear parameters
			for (int i = 0; i < 3; i++)
			{
				*ScriptGlobal(SESSION_PARAM + i).As<int*>() = 0;
			}

			// Use hash values directly from your script
			// These correspond to the script's ScriptParam_0.f_1 checks
			*ScriptGlobal(SESSION_TARGET).As<int*>() = -1557193583; // FREEROAM
			*ScriptGlobal(SESSION_PARAM).As<int*>() = -821438348;   // Private session variant 1
			// Or use -968517323 for variant 2

			*ScriptGlobal(SESSION_FLAGS).As<int*>() = 68;
			*ScriptGlobal(SESSION_TYPE).As<int*>() = 3;
		}
	};

	// Register commands
	static SwitchToPrivateFreeroam _SwitchPrivateFreeroam{"switchprivatefreeroam", "Switch to Private Freeroam", "Switches you to a private freeroam session"};
	static SwitchToPublicFreeroam _SwitchPublicFreeroam{"switchpublicfreeroam", "Switch to Public Freeroam", "Switches you to a public freeroam session"};
	static SwitchToSinglePlayer _SwitchSinglePlayer{"switchsingleplayer", "Switch to Single Player", "Switches you to single player mode"};
	static SwitchToMissionCreator _SwitchMissionCreator{"switchmissioncreator", "Switch to Mission Creator", "Switches you to the mission creator"};
	static SwitchPrivateFreeroamRaw _SwitchPrivateRaw{"switchprivatefreeroamraw", "Switch Private (Raw Hashes)", "Uses raw hash values from the script"};
}