#include "core/commands/LoopedCommand.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Pools.hpp"

namespace YimMenu::Features
{
	class PedsRiot : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		virtual void OnTick() override
		{
			for (auto npcs : Pools::GetPeds())
			{

				PED::_SET_PED_INTERACTION_PERSONALITY(npcs.GetPed().GetHandle(), "AGGRESSIVE"_J);
				PED::_SET_PED_PERSONALITY(npcs.GetPed().GetHandle(), "STANDARD_PED_AGRO_GUARD"_J);
				PED::SET_PED_COMBAT_MOVEMENT(npcs.GetPed().GetHandle(), 2);
				PED::REGISTER_HATED_TARGETS_AROUND_PED(npcs.GetPed().GetHandle(), 1000.0f);
			}
		}
	};

	static PedsRiot _PedsRiot{"pedsriot", "Peds Riot", "Make peds riot"};
}