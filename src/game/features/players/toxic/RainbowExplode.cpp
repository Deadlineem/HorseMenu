#include "core/commands/BoolCommand.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/commands/PlayerCommand.hpp"
#include "game/pointers/Pointers.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"

namespace YimMenu::Features
{
	BoolCommand _RainbowExplosionsActive{"rainbowactive", "Rainbow Explosions Enabled", "Turn off to stop rainbow explosions", false};
	Player _RainbowExplosionsTarget;

	class RainbowExplosionLoop : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (!player.IsValid())
				return;

			// Toggle the state
			bool newState = !_RainbowExplosionsActive.GetState();
			_RainbowExplosionsActive.SetState(newState);

			if (newState)
			{
				_RainbowExplosionsTarget = player;
				// Start the loop in a separate thread
					if (!_RainbowExplosionsActive.GetState())
					{
						_RainbowExplosionsActive.SetState(false);
						return;
					}

					if (!player.IsValid() || PED::IS_PED_DEAD_OR_DYING(player.GetPed().GetHandle(), true)
					    || PLAYER::IS_PLAYER_DEAD(player.GetPed().GetHandle()))
					{
						_RainbowExplosionsActive.SetState(false);
						return;
					}

					auto playerCoords = player.GetPed().GetPosition();

					// Cycle through explosion types
					static int explosionIndex = 0;
					FIRE::ADD_EXPLOSION(playerCoords.x, playerCoords.y, playerCoords.z, explosionIndex, 5.0f, true, false, 5.0f);

					explosionIndex++;
					if (explosionIndex > 36) // MAX_EXPLOSION_TYPES
						explosionIndex = 0;

					ScriptMgr::Yield(100ms);
			}
			else
			{
				_RainbowExplosionsActive.SetState(false);
			}
		}
	};

	static RainbowExplosionLoop _RainbowExplosionLoop{"rainbowloop", "Rainbow Explode", "Cycles 36 different explosions on the selected target"};
}