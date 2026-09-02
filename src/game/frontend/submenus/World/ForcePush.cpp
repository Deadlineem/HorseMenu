// ForcePush.hpp
#pragma once
#include "core/commands/LoopedCommand.hpp"
#include "game/backend/NativeHooks.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/commands/PlayerCommand.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/ScriptGlobal.hpp"
#include "game/rdr/Scripts.hpp"
#include "core/frontend/Notifications.hpp"

#include <cmath>

namespace YimMenu::Features
{
	class ForcePush : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		static inline Entity m_TargetEntity = 0;
		static inline bool m_IsPushing = false;
		static inline bool m_IsCharging = false;
		static inline float m_ChargeTime = 0.0f;
		static inline float m_PushCooldown = 0.0f;
		static inline int m_AnimationTick = 0;
		static inline Vector3 m_PushDirection = {0, 0, 0};
		static inline float m_PushStrength = 0.0f;

		static constexpr float MAX_CHARGE_TIME = 1.5f;
		static constexpr float BASE_PUSH_FORCE = 25.0f;
		static constexpr float COOLDOWN_TIME = 1.0f;
		static constexpr int MAX_ANIMATION_TICKS = 60;

		virtual void OnTick() override
		{
			// Update cooldown
			if (m_PushCooldown > 0.0f)
			{
				m_PushCooldown -= 0.016f;
				if (m_PushCooldown < 0.0f)
					m_PushCooldown = 0.0f;
			}

			// Check for activation when unarmed and aiming
			if (IsPlayerUnarmed() && IsPlayerAiming() && IsPlayerShooting() && !m_IsPushing && m_PushCooldown <= 0.0f)
			{
				Entity aimedEntity;
				// RDR2 native for getting the entity the player is aiming at
				if (PLAYER::IS_PLAYER_FREE_AIMING_AT_ENTITY(PLAYER::PLAYER_ID(), &aimedEntity))
				{
					if (ENTITY::IS_ENTITY_A_PED(aimedEntity) && !PED::IS_PED_A_PLAYER(aimedEntity))
					{
						m_TargetEntity = aimedEntity;
						m_IsCharging = true;
						m_ChargeTime = 0.0f;
						m_PushStrength = 0.0f;
						// RDR2 sound native - using a generic interaction sound
						AUDIO::PLAY_SOUND_FRONTEND("CABIN_WIND_01", 0x0CED0D77, true, 0);
					}
				}
			}

			// Handle charging
			if (m_IsCharging)
			{
				m_ChargeTime += 0.016f;
				m_PushStrength = (m_ChargeTime / MAX_CHARGE_TIME) * BASE_PUSH_FORCE;
				if (m_PushStrength > BASE_PUSH_FORCE * 2.0f)
					m_PushStrength = BASE_PUSH_FORCE * 2.0f;

				if (m_TargetEntity != 0)
				{
					Vector3 targetPos = ENTITY::GET_ENTITY_COORDS(m_TargetEntity, true, false);
					DrawForceEffect(targetPos, m_PushStrength / (BASE_PUSH_FORCE * 2.0f));
				}

				if (IsPlayerShooting() || m_ChargeTime >= MAX_CHARGE_TIME)
				{
					if (m_TargetEntity != 0)
						ProcessForcePush();
					else
						ResetPushState();
				}
			}

			// Animation update
			if (m_IsPushing)
			{
				m_AnimationTick++;
				PlayPushAnimation();

				if (m_AnimationTick < MAX_ANIMATION_TICKS && m_TargetEntity != 0)
				{
					ApplyForceToTarget();
					Vector3 targetPos = ENTITY::GET_ENTITY_COORDS(m_TargetEntity, true, false);
					DrawForceEffect(targetPos, 1.0f - (float)m_AnimationTick / MAX_ANIMATION_TICKS);
				}
				else
				{
					ResetPushState();
				}
			}
		}

		virtual void OnDisable() override
		{
			ResetPushState();
			// Clean up any lingering effects if needed
			Ped playerPed = Self::GetPed();
			if (playerPed)
			{
				// Clear any tasks that might have been started
				TASK::CLEAR_PED_TASKS(playerPed, false, false);
			}
		}

	private:
		bool IsPlayerUnarmed()
		{
			Ped playerPed = Self::GetPed();
			if (!playerPed)
				return false;

			Hash currentWeapon;
			// RDR2 native for getting current weapon
			if (!WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &currentWeapon, true, 0, false))
				return true;

			// Unarmed in RDR2 is typically 0x9D07F764 or just 0
			return currentWeapon == 0 || currentWeapon == 0x9D07F764;
		}

		bool IsPlayerAiming()
		{
			// RDR2 native for checking if player is aiming
			return PAD::IS_CONTROL_PRESSED(0, 0x07CE1E61) || // INPUT_AIM
			    PAD::IS_CONTROL_ENABLED(0, 0x07CE1E61);
		}

		bool IsPlayerShooting()
		{
			// RDR2 uses PAD natives for controller input
			return PAD::IS_CONTROL_JUST_PRESSED(0, 0x07B8BEAF) || // INPUT_ATTACK
			    PAD::IS_CONTROL_JUST_PRESSED(0, 0x07CE1E61);      // INPUT_AIM (shoot while aiming)
		}

		void ProcessForcePush()
		{
			if (m_TargetEntity == 0 || !ENTITY::DOES_ENTITY_EXIST(m_TargetEntity))
			{
				ResetPushState();
				return;
			}

			Ped playerPed = Self::GetPed();
			if (!playerPed)
				return;

			Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(playerPed, true, false);
			Vector3 targetPos = ENTITY::GET_ENTITY_COORDS(m_TargetEntity, true, false);

			m_PushDirection.x = targetPos.x - playerPos.x;
			m_PushDirection.y = targetPos.y - playerPos.y;
			m_PushDirection.z = (targetPos.z - playerPos.z) + 1.0f;

			float length = sqrt(m_PushDirection.x * m_PushDirection.x + m_PushDirection.y * m_PushDirection.y
			    + m_PushDirection.z * m_PushDirection.z);
			if (length > 0.0f)
			{
				m_PushDirection.x /= length;
				m_PushDirection.y /= length;
				m_PushDirection.z /= length;
			}

			m_IsPushing = true;
			m_IsCharging = false;
			m_AnimationTick = 0;

			// RDR2 sound native
			AUDIO::PLAY_SOUND_FRONTEND("CABIN_WIND_01", 0x0CED0D77, true, 0);
			ApplyForceToTarget();
		}

		void ApplyForceToTarget()
		{
			if (m_TargetEntity == 0 || !ENTITY::DOES_ENTITY_EXIST(m_TargetEntity))
				return;

			float forceMagnitude = m_PushStrength * (1.0f - (float)m_AnimationTick / MAX_ANIMATION_TICKS * 0.5f);

			// RDR2 force application native - note the different parameter order
			ENTITY::APPLY_FORCE_TO_ENTITY(m_TargetEntity, // Entity
			    1,                                        // Force type (0=force, 1=impulse, 2=torque, 3=angular)
			    m_PushDirection.x * forceMagnitude,       // X force
			    m_PushDirection.y * forceMagnitude,       // Y force
			    m_PushDirection.z * forceMagnitude,       // Z force
			    0.0f,
			    0.0f,
			    0.0f,  // Offset
			    0,     // Bone index
			    false, // Is direction relative
			    true,  // Ignore up vector
			    false, // Is force relative
			    false,
			    false // P12, P13
			);

			// Add some ragdoll impulse for dramatic effect
			ENTITY::APPLY_FORCE_TO_ENTITY(m_TargetEntity, 1, 0.0f, 0.0f, 0.0f, (rand() % 100 - 50) / 50.0f, (rand() % 100 - 50) / 50.0f, (rand() % 100 - 50) / 50.0f, 0, false, true, false, false, false);
		}

		void PlayPushAnimation()
		{
			Ped playerPed = Self::GetPed();
			if (!playerPed)
				return;

			float progress = (float)m_AnimationTick / MAX_ANIMATION_TICKS;
			float heading = ENTITY::GET_ENTITY_HEADING(playerPed);
			float radHeading = heading * 3.14159f / 180.0f;

			Vector3 handPos = ENTITY::GET_ENTITY_COORDS(playerPed, true, false);
			handPos.x += sin(radHeading) * 0.8f;
			handPos.y += cos(radHeading) * 0.8f;
			handPos.z += 0.7f;

			float extension = 0.3f + progress * 0.7f;
			Vector3 extendedPos = handPos;
			extendedPos.x += sin(radHeading) * extension;
			extendedPos.y += cos(radHeading) * extension;

			DrawForceEffect(extendedPos, 1.0f - progress * 0.5f);

			if (m_TargetEntity != 0)
			{
				Vector3 targetPos = ENTITY::GET_ENTITY_COORDS(m_TargetEntity, true, false);

				// RDR2 drawing native
				CFX::DRAW_LINE(extendedPos.x, extendedPos.y, extendedPos.z, targetPos.x, targetPos.y, targetPos.z, 100, 150, 255, 255);

				for (int i = 0; i < 10; i++)
				{
					float t = (float)i / 10.0f;
					Vector3 particlePos = {extendedPos.x + (targetPos.x - extendedPos.x) * t,
					    extendedPos.y + (targetPos.y - extendedPos.y) * t,
					    extendedPos.z + (targetPos.z - extendedPos.z) * t + (float)(rand() % 20 - 10) / 100.0f};
					DrawForceEffect(particlePos, 0.3f);
				}
			}

			// RDR2 camera shake native
			if (progress < 0.3f)
			{
				CAM::SHAKE_GAMEPLAY_CAM("HAND_SHAKE", 0.3f * (1.0f - progress / 0.3f));
			}
		}

		void DrawForceEffect(Vector3 position, float intensity)
		{
			// RDR2 marker drawing - similar to GTA but verify the parameters
			CFX::DRAW_MARKER(28, // Sphere marker
			    position.x,
			    position.y,
			    position.z,
			    0.0f,
			    0.0f,
			    0.0f,
			    0.0f,
			    0.0f,
			    0.0f,
			    0.5f + intensity * 0.5f,
			    0.5f + intensity * 0.5f,
			    0.5f + intensity * 0.5f,
			    100,
			    150,
			    255,
			    100 * intensity,
			    false,
			    false,
			    2,
			    false,
			    false,
			    false,
			    false);

			// Sparkle particles
			for (int i = 0; i < 5 * intensity; i++)
			{
				Vector3 sparklePos = {position.x + (float)(rand() % 100 - 50) / 100.0f,
				    position.y + (float)(rand() % 100 - 50) / 100.0f,
				    position.z + (float)(rand() % 100 - 50) / 100.0f};
				CFX::DRAW_MARKER(28, sparklePos.x, sparklePos.y, sparklePos.z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.1f, 255, 255, 255, 150 * intensity, false, false, 2, false, false, false, false);
			}
		}

		void ResetPushState()
		{
			m_IsPushing = false;
			m_IsCharging = false;
			m_ChargeTime = 0.0f;
			m_PushStrength = 0.0f;
			m_TargetEntity = 0;
			m_AnimationTick = 0;
			m_PushCooldown = COOLDOWN_TIME;
			// RDR2 camera shake stop
			CAM::STOP_GAMEPLAY_CAM_SHAKING(false);
		}
	};

	// Command registration - follows the same pattern as godmode
	static ForcePush _ForcePush{"forcepush", "Force Push", "Push enemies with the power of the Force when unarmed and aiming"};
}