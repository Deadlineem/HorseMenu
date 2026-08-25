#include "core/commands/LoopedCommand.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Pools.hpp"
#include "game/rdr/data/WeaponTypes.hpp"

namespace YimMenu::Features
{
	static const char* GetRandomNonMPWeapon()
	{
		static std::vector<const char*> validWeapons;

		if (validWeapons.empty()) // Build the list once
		{
			for (int i = 0; i < Data::g_WeaponTypesCount; i++)
			{
				const char* weapon = Data::g_WeaponTypes[i];
				std::string weaponStr(weapon);

				// Exclude _MP weapons AND non-combat items
				if (weaponStr.find("_MP") == std::string::npos && weaponStr.find("KIT_") == std::string::npos && // Binoculars, Camera, Detectors
				    weaponStr.find("FISHINGROD") == std::string::npos &&   // Fishing Rod
				    weaponStr.find("LANTERN") == std::string::npos &&      // Lanterns
				    weaponStr.find("TORCH") == std::string::npos &&        // Torches
				    weaponStr.find("DETECTOR") == std::string::npos &&     // Metal Detector
				    weaponStr.find("BINOCULARS") == std::string::npos &&   // Binoculars
				    weaponStr.find("CAMERA") == std::string::npos
					)            // Optional: Exclude bows
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
					static const char* randomWeapon = GetRandomNonMPWeapon();
					Hash hasWeapon = WEAPON::_GET_PED_CURRENT_HELD_WEAPON(npcs.GetHandle());
					bool isUnarmed = (hasWeapon == 0 || hasWeapon == Joaat("WEAPON_UNARMED"));
					if (isUnarmed)
					{
						npcs.ForceControl();
						Hash riotGroup = "REL_CRIMINALS"_J;
						PED::SET_PED_RELATIONSHIP_GROUP_HASH(npcs.GetHandle(), riotGroup);
						PED::SET_RELATIONSHIP_BETWEEN_GROUPS(6, riotGroup, riotGroup);
						PED::SET_RELATIONSHIP_BETWEEN_GROUPS(1, riotGroup, "REL_NO_RELATIONSHIP"_J);
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
						PED::SET_PED_MAX_HEALTH(npcs.GetHandle(), 999999);
						PED::SET_PED_ACCURACY(npcs.GetHandle(), 100);
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
						PED::_REGISTER_HATED_TARGETS_IN_AREA(npcs.GetHandle(), 0.0f, 0.0f, 0.0f, 100000.0f);
						PED::REGISTER_HATED_TARGETS_AROUND_PED(npcs.GetHandle(), 100000.0f);
						WEAPON::SET_PED_DROPS_WEAPONS_WHEN_DEAD(npcs.GetHandle(), 0);
						STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(npcs.GetHandle());
					}
				}
			}
		}
	};

	static PedsRiot _PedsRiot{"pedsriot", "Peds Riot", "Make peds riot"};
}