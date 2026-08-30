#include "core/commands/Command.hpp"
#include "core/commands/HotkeySystem.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Self.hpp"
#include "game/frontend/items/Items.hpp"
#include "game/rdr/Natives.hpp"

namespace YimMenu::Submenus
{
	// Native hashes for music functions
	constexpr auto PREPARE_MUSIC_EVENT = 0x1E5185B72EF5158A; // BOOL PREPARE_MUSIC_EVENT(char* eventName)
	constexpr auto TRIGGER_MUSIC_EVENT = 0x706D57B0F50DA710; // BOOL TRIGGER_MUSIC_EVENT(char* eventName)

	// Generic music events (shortened list - add more as needed)
	static const std::vector<std::string> g_MusicEvents = {
	    // Combat/Intense
	    "DUEL_GENERAL_START",
	    "DUEL_GENERAL_STOP",
	    "DUEL_GENERAL_DRAW",
	    "DUEL_GENERAL_END_OS",
	    "MC_ATTACKING",
	    "MC_DEFENDING",
	    "MC_SUSPENSE_HIGH",
	    "MC_SUSPENSE_LOW",

	    // Exploration/Riding
	    "FIN1_START",
	    "FIN1_STOP",
	    "ARM2_RIDE",
	    "BRAN_RIDE",
	    "MAGDEMO_START",

	    // Ambient
	    "MC_IDLE_HIGH",
	    "MC_IDLE_LOW",
	    "MC_MUSIC_STOP", // Stop all music

	    // Mission-specific
	    "AB21_START",
	    "AB21_STOP",
	    "BRT1_START",
	    "BRT1_STOP",
	    "CABR01_START",
	    "CABR01_STOP",
	    "HIDEOUT_SPC_START",
	    "HIDEOUT_SPC_ABANDON",
	    "HIDEOUT_SPC_FAIL",

	    // Minigames
	    "POKER_START_MUSIC",
	    "POKER_STOP_MUSIC",
	    "DOMINOES_START_MUSIC",
	    "DOMINOES_STOP_MUSIC",

	    // Wanted/Combat
	    "ME_GAINED_WANTED_L3",
	    "ME_GAINED_WANTED_L4",
	    "ME_GAINED_WANTED_L5",
	    "ME_ESCAPED_WANTED_L3",
	    "ME_ESCAPED_WANTED_L4",
	    "ME_ESCAPED_WANTED_L5",
	    "ME_STOP_WANTED_MUSIC_MUTED",

	    // Debug/Testing
	    "DEBUG_LOOPING_START_TRACK",
	    "DEBUG_STOP_MUSIC_EVENT",
	    "DEBUG_STOP_ONESHOT",
	    "DEBUG_STOP_TRACK",

	    // Cutscenes/Story
	    "FIN3_END_CREDITS",
	    "FIN3_END_STOP",
	    "STOP_TITLE_SCREEN_MUSIC",
	};

	// Input callback for autocomplete
	static int MusicEventInputCallback(ImGuiInputTextCallbackData* data)
	{
		if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
		{
			std::string inputLower = data->Buf;
			std::transform(inputLower.begin(), inputLower.end(), inputLower.begin(), ::tolower);

			for (const auto& event : g_MusicEvents)
			{
				std::string eventLower = event;
				std::transform(eventLower.begin(), eventLower.end(), eventLower.begin(), ::tolower);
				if (eventLower.find(inputLower) != std::string::npos)
				{
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, event.c_str());
					break;
				}
			}
			return 1;
		}
		return 0;
	}

	void RenderMusicMenu()
	{
		ImGui::PushID("music_events"_J);

		static std::string eventBuffer;
		static bool prepareFirst = true;
		static bool networkSync = true;

		// Event name input with autocomplete
		InputTextWithHint("##musicevent", "Music Event Name", &eventBuffer, ImGuiInputTextFlags_CallbackCompletion, nullptr, MusicEventInputCallback)
		    .Draw();

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Press Tab to autocomplete from known events");

		// Show matching events in dropdown
		if (!eventBuffer.empty())
		{
			bool foundMatch = false;
			std::string bufferLower = eventBuffer;
			std::transform(bufferLower.begin(), bufferLower.end(), bufferLower.begin(), ::tolower);

			ImGui::BeginListBox("##musiclist", ImVec2(300, 150));
			for (const auto& event : g_MusicEvents)
			{
				std::string eventLower = event;
				std::transform(eventLower.begin(), eventLower.end(), eventLower.begin(), ::tolower);
				if (eventLower.find(bufferLower) != std::string::npos)
				{
					foundMatch = true;
					if (ImGui::Selectable(event.c_str()))
					{
						eventBuffer = event;
					}
				}
			}
			if (!foundMatch)
			{
				ImGui::TextDisabled("No matching events found");
				ImGui::TextDisabled("Type any event name manually");
			}
			ImGui::EndListBox();
		}

		// Options
		ImGui::Checkbox("Prepare Event First", &prepareFirst);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Call PREPARE_MUSIC_EVENT before TRIGGER_MUSIC_EVENT");

		ImGui::Checkbox("Network Sync", &networkSync);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Sync music event across network (for multiplayer)");

		ImGui::Separator();

		// Action buttons
		if (ImGui::Button("Trigger Music Event"))
		{
			if (eventBuffer.empty())
			{
				ImGui::OpenPopup("Error##musicevent");
				return;
			}

			FiberPool::Push([eventName = eventBuffer, prepareFirst, networkSync] {
				// Convert to const char*
				const char* event = eventName.c_str();

				// Prepare if needed
				if (prepareFirst)
				{
					MISC::_CALL_NATIVE(PREPARE_MUSIC_EVENT, event);
				}

				// Trigger the event
				MISC::_CALL_NATIVE(TRIGGER_MUSIC_EVENT, event);

				// Sync across network if enabled
				if (networkSync && Self::GetPlayer().IsValid())
				{
					// In a real implementation, you'd use a network event system here
					// For example: TriggerNetworkEvent("MusicEvent", eventName);
				}
			});
		}

		ImGui::SameLine();

		if (ImGui::Button("Stop Music"))
		{
			FiberPool::Push([] {
				MISC::_CALL_NATIVE(TRIGGER_MUSIC_EVENT, "MC_MUSIC_STOP");
			});
		}

		ImGui::SameLine();

		if (ImGui::Button("Clear Input"))
		{
			eventBuffer.clear();
		}

		// Error popup
		if (ImGui::BeginPopupModal("Error##musicevent", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Please enter a valid music event name.");
			ImGui::Separator();
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		// Quick reference list
		ImGui::Separator();
		if (ImGui::CollapsingHeader("Event Reference (Common)", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("##eventlist", ImVec2(0, 150), true);

			// Show common events in categories
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Combat/Intense:");
			ImGui::Text("  DUEL_GENERAL_START, MC_ATTACKING, MC_SUSPENSE_HIGH");

			ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Exploration:");
			ImGui::Text("  FIN1_START, ARM2_RIDE, BRAN_RIDE");

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "Ambient:");
			ImGui::Text("  MC_IDLE_HIGH, MC_IDLE_LOW");

			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.5f, 1.0f), "Stop Events:");
			ImGui::Text("  MC_MUSIC_STOP, DUEL_GENERAL_STOP, DEBUG_STOP_MUSIC_EVENT");

			ImGui::EndChild();
		}

		ImGui::PopID();
	}

	// Register the command
	class TriggerMusicEvent : public Command
	{
	public:
		TriggerMusicEvent() :
		    Command("triggermusic", "Trigger a music event", "Plays a music event via AUDIO::TRIGGER_MUSIC_EVENT")
		{
			AddArgument<std::string>("event", "Music event name");
			AddArgument<bool>("prepare", "Prepare event first", false);
		}

		virtual void OnCall() override
		{
			auto event = GetArg<std::string>(0);
			auto prepare = GetArg<bool>(1);

			if (event.empty())
				return;

			FiberPool::Push([event, prepare] {
				if (prepare)
				{
					MISC::_CALL_NATIVE(PREPARE_MUSIC_EVENT, event.c_str());
				}
				MISC::_CALL_NATIVE(TRIGGER_MUSIC_EVENT, event.c_str());
			});
		}
	};

	static TriggerMusicEvent _TriggerMusicEvent;
}