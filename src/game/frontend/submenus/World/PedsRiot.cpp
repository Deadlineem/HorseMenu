#include "core/commands/LoopedCommand.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Pools.hpp"
#include "game/rdr/data/WeaponTypes.hpp"

namespace YimMenu::Features
{
	const char* GetRandomNonMPWeapon()
	{
		static std::vector<const char*> validWeapons;

		if (validWeapons.empty()) // Build the list once
		{
			for (const auto& weapon : Data::g_WeaponTypes)
			{
				std::string weaponStr(weapon);

				// Only exclude weapons containing "_MP"
				if (weaponStr.find("_MP") == std::string::npos)
				{
					validWeapons.push_back(weapon);
				}
			}
		}

		// Return a random weapon from the valid list
		if (!validWeapons.empty())
		{
			int randomIndex = rand() % validWeapons.size();
			return validWeapons[randomIndex];
		}

		// Fallback weapon (should never happen)
		return "WEAPON_REVOLVER_CATTLEMAN";
	}

	class PedsRiot : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		virtual void OnTick() override
		{
			for (auto npcs : Pools::GetPeds())
			{
				if (!npcs.IsPlayer())
				{
					npcs.ForceControl();
					Hash riotGroup = "REL_CRIMINALS"_J;
					PED::SET_PED_RELATIONSHIP_GROUP_HASH(npcs.GetHandle(), riotGroup);
					PED::SET_RELATIONSHIP_BETWEEN_GROUPS(6, riotGroup, riotGroup);
					PED::_SET_PED_INTERACTION_PERSONALITY(npcs.GetHandle(), "AGGRESSIVE"_J);
					PED::_SET_PED_PERSONALITY(npcs.GetHandle(), "BOUNTY_HUNTER"_J);
					PED::SET_PED_COMBAT_ATTRIBUTES(npcs.GetHandle(), 5, 1);
					PED::SET_PED_COMBAT_ATTRIBUTES(npcs.GetHandle(), 17, 0);
					PED::SET_PED_COMBAT_ATTRIBUTES(npcs.GetHandle(), 50, 1);
					PED::SET_PED_COMBAT_ATTRIBUTES(npcs.GetHandle(), 114, 1);
					PED::SET_PED_COMBAT_ATTRIBUTES(npcs.GetHandle(), 58, 1);
					PED::SET_PED_COMBAT_MOVEMENT(npcs.GetHandle(), 3);
					PED::SET_PED_COMBAT_ABILITY(npcs.GetHandle(), 3);
					PED::SET_PED_COMBAT_RANGE(npcs.GetHandle(), 100000);
					PED::SET_PED_CONFIG_FLAG(npcs.GetHandle(), 24, 0);
					PED::SET_PED_CONFIG_FLAG(npcs.GetHandle(), 8, 1);
					PED::SET_PED_CONFIG_FLAG(npcs.GetHandle(), 40, 1);
					PED::SET_PED_CONFIG_FLAG(npcs.GetHandle(), 569, 1);
					const char* randomWeapon = GetRandomNonMPWeapon();
					WEAPON::GIVE_WEAPON_TO_PED(npcs.GetHandle(),
					    Joaat(randomWeapon),
					    9999, // Ammo
					    1,    // Equip immediately
					    0,    // Give as current weapon
					    0,
					    0,
					    0,
					    0,
					    0,
					    0,
						0,
						0);
					WEAPON::SET_PED_CURRENT_WEAPON_VISIBLE(npcs.GetHandle(), 1, 0, 1, 0);
					PED::_REGISTER_HATED_TARGETS_IN_AREA(npcs.GetHandle(), 0.0f, 0.0f, 0.0f, 100000.0f);
					PED::REGISTER_HATED_TARGETS_AROUND_PED(npcs.GetHandle(), 100000.0f);
				    TASK::TASK_COMBAT_HATED_TARGETS(npcs.GetHandle(), 100000.0f);
				}
			}
		}
	};

	static PedsRiot _PedsRiot{"pedsriot", "Peds Riot", "Make peds riot"};
}