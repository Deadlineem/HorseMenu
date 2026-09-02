#include "core/commands/Command.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/data/WeaponTypes.hpp"

namespace YimMenu::Features
{
	class GiveAllWeapons : public Command
	{
		using Command::Command;

		virtual void OnCall() override
		{
			for (int i = 0; i < Data::g_WeaponTypesCount; i++)
			{
				auto selfPlayer = Self::GetPed().GetHandle();
				if (!selfPlayer)
					selfPlayer = PLAYER::PLAYER_PED_ID();
				WEAPON::GIVE_WEAPON_TO_PED(selfPlayer, Joaat(Data::g_WeaponTypes[i]), 9999, false, false, 0, true, 0.5f, 1.0f, 0x2CD419DC, true, 0.0f, false);
			}
		}
	};

	static GiveAllWeapons _GiveAllWeapons{"giveallweapons", "Give All Weapons", "Gives you all of the weapons"};
}