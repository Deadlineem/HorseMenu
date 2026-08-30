#include "core/commands/LoopedCommand.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Natives.hpp"
#include "core/commands/Commands.hpp"
#include "core/commands/BoolCommand.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/Self.hpp"

namespace YimMenu::Features
{
	class PlayMusic : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		virtual void OnTick() override
		{
			// Get selected event from dropdown
			static int selectedIndex = 0;
			const char* eventName = "MRY3_WAGON_CHASE";

			// Check if music is already playing
			static bool isPrepared = false;
			static bool isPlaying = false;

			// Only prepare if not already prepared
			if (!isPrepared)
			{
				isPrepared = AUDIO::PREPARE_MUSIC_EVENT(eventName);
			}

			// Check if music is currently playing
			isPlaying = AUDIO::AUDIO_IS_MUSIC_PLAYING();

			if (isPlaying)
			{
				AUDIO::TRIGGER_MUSIC_EVENT("MC_MUSIC_STOP");
			}

			// Only trigger if prepared and not already playing
			if (isPrepared && !isPlaying)
			{
				AUDIO::TRIGGER_MUSIC_EVENT(eventName);
				isPlaying = true; // Set flag to prevent retriggering
			}

			// Reset prepared state if music stopped or event changed
			if (isPlaying && !AUDIO::AUDIO_IS_MUSIC_PLAYING())
			{
				isPrepared = false;
				isPlaying = false;
			}
		}
	};
	static PlayMusic _PlayMusic{"playmusic", "Play Music", "Plays selected music event on loop"};
}