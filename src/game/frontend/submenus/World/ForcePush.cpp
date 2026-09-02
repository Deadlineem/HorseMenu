#pragma once
#include "core/commands/LoopedCommand.hpp"
#include "core/frontend/Notifications.hpp"
#include "game/backend/NativeHooks.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/commands/PlayerCommand.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/ScriptGlobal.hpp"
#include "game/rdr/Scripts.hpp"

#include <cmath>

namespace YimMenu::Features
{
	class ForcePush : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		Entity m_HeldEntity = 0;
		bool m_WasShooting = false;
		bool m_HasShownNotification = false;

		/*
		 * Distance from the camera at which the entity is held.
		 *
		 * This is the point the crosshair is effectively projecting
		 * into the world.
		 */
		static constexpr float HOLD_DISTANCE = 10.0f;

		/*
		 * Convert camera rotation into a forward direction.
		 *
		 * X = pitch
		 * Z = yaw
		 */
		Vector3 getCameraDirection()
		{
			Vector3 cameraRotation = CAM::GET_GAMEPLAY_CAM_ROT(0);

			float pitch = cameraRotation.x * 0.0174532924f;
			float yaw = cameraRotation.z * 0.0174532924f;

			float cosPitch = std::cos(pitch);

			Vector3 direction{-std::sin(yaw) * cosPitch, std::cos(yaw) * cosPitch, std::sin(pitch)};

			return direction;
		}

		Vector3 getCrosshairCoords(float distance)
		{
			// Get absolute camera position (world space)
			Vector3 cameraCoords = CAM::GET_GAMEPLAY_CAM_COORD();

			// Get absolute camera rotation (world space)
			// This is exactly what getCameraDirection uses
			Vector3 cameraRotation = CAM::GET_GAMEPLAY_CAM_ROT(0);

			// Convert to radians (RDR2 uses degrees)
			float pitch = cameraRotation.x * 0.0174532924f;
			float yaw = cameraRotation.z * 0.0174532924f;

			// Calculate direction using RDR2's world coordinate system
			float cosPitch = std::cos(pitch);
			float sinPitch = std::sin(pitch);
			float cosYaw = std::cos(yaw);
			float sinYaw = std::sin(yaw);

			Vector3 direction;
			direction.x = -sinYaw * cosPitch; // East/West
			direction.y = cosYaw * cosPitch;  // North/South
			direction.z = sinPitch;           // Up/Down

			Vector3 targetPos = {cameraCoords.x + direction.x * distance,
			    cameraCoords.y + direction.y * distance,
			    cameraCoords.z + direction.z * distance};

			return targetPos;
		}

		/*
		 * Get the exact direction the active camera is facing.
		 *
		 * This is used when the entity is thrown.
		 */
		Vector3 getThrowDirection()
		{
			return getCameraDirection();

			/*
			 * Fallback to gameplay camera rotation if there isn't
			 * a rendering camera handle.
			 */
			Vector3 rotation = CAM::GET_GAMEPLAY_CAM_ROT(0);

			float pitch = rotation.x * 0.0174532924f;

			float yaw = rotation.z * 0.0174532924f;

			float cosPitch = std::cos(pitch);

			return {-std::sin(yaw) * cosPitch, std::cos(yaw) * cosPitch, std::sin(pitch)};
		}

		/*
		 * Attach the entity directly to the world coordinates
		 * projected from the camera/crosshair.
		 */
		void attachEntityToCrosshair(Entity target)
		{
			if (target.GetHandle() == 0 || !ENTITY::DOES_ENTITY_EXIST(target.GetHandle()))
			{
				return;
			}

			target.ForceControl();

			/*
			 * Calculate the world-space point at the crosshair.
			 */
			Vector3 coords = getCrosshairCoords(HOLD_DISTANCE);

			coords.z -= 0.3f;

			Ped npc = ENTITY::GET_PED_INDEX_FROM_ENTITY_INDEX(target.GetHandle());
			TASK::CLEAR_PED_TASKS_IMMEDIATELY(ENTITY::GET_PED_INDEX_FROM_ENTITY_INDEX(target.GetHandle()), true, true);

			/*
			 * Attach directly to those WORLD coordinates.
			 */
			ENTITY::SET_ENTITY_COORDS(target.GetHandle(), coords.x, coords.y, coords.z, false, false, false, false);
		}

		/*
		 * This is your original force behavior, but the directional
		 * portion now comes from the camera/crosshair.
		 */
		void applyForceToTarget(Entity target, float scale)
		{
			if (target.GetHandle() == 0 || !ENTITY::DOES_ENTITY_EXIST(target.GetHandle()))
			{
				return;
			}

			Ped npc = ENTITY::GET_PED_INDEX_FROM_ENTITY_INDEX(target.GetHandle());

			target.ForceControl();

			// Get the direction you are looking
			Vector3 direction = getThrowDirection();

			// Scale the direction
			float forceScale = scale * 2.5f; // Keep your scale
			Vector3 scaledDir = {direction.x * forceScale, direction.y * forceScale, direction.z * forceScale};

			/*
     * CRITICAL FIX #1: Reset the Ragdoll BEFORE throwing.
     * This prevents the game from misinterpreting your WORLD vector 
     * as a LOCAL vector inside the current ragdoll rotation.
     */
			PED::SET_PED_TO_RAGDOLL(npc.GetHandle(), 1, 1, 0, false, false, "DraggedByCart"); // Short, clean ragdoll timer

			/*
     * CRITICAL FIX #2: Use SET_ENTITY_VELOCITY.
     * This native applies a pure world-space velocity. 
     * It completely ignores the ped's internal physics rotation, 
     * so the ped will ALWAYS fly exactly where you are looking.
     */
			ENTITY::SET_ENTITY_VELOCITY(target.GetHandle(), scaledDir.x, scaledDir.y, scaledDir.z);

			/*
     * CRITICAL FIX #3: Apply upward force separately for the "launch" feel.
     * This bypasses the local vector issue completely.
     */
			ENTITY::APPLY_FORCE_TO_ENTITY(target.GetHandle(), 1, 0.0f, 0.0f, 2500.0f, 0.0f, 0.0f, 0.0f, 0, false, true, false, false, false);
		}

		/*
		 * Release the entity and throw it.
		 */
		void releaseHeldEntity()
		{
			if (m_HeldEntity.GetHandle() == 0)
			{
				return;
			}

			if (!ENTITY::DOES_ENTITY_EXIST(m_HeldEntity.GetHandle()))
			{
				m_HeldEntity = 0;
				return;
			}

			/*
			 * Detach from the crosshair.
			 */
			ENTITY::DETACH_ENTITY(m_HeldEntity.GetHandle(), true, true);

			/*
			 * Throw in the direction the camera was facing
			 * when Attack was released.
			 */
			applyForceToTarget(m_HeldEntity, 100000.0f);

			m_HeldEntity = 0;
		}

		virtual void OnTick() override
		{
			auto playerPed = Self::GetPed().GetHandle();

			PLAYER::DISABLE_PLAYER_FIRING(Self::GetPlayer().GetId(), true);

			//PAD::DISABLE_CONTROL_ACTION(0, (int)NativeInputs::INPUT_AIM, true);

			PAD::DISABLE_CONTROL_ACTION(0, (int)NativeInputs::INPUT_ATTACK, true);

			bool isAimingHeld = PAD::IS_CONTROL_PRESSED(0, (int)NativeInputs::INPUT_AIM);

			bool isShootingHeld = PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)NativeInputs::INPUT_ATTACK);

			bool isShootingReleased = PAD::IS_DISABLED_CONTROL_JUST_RELEASED(0, (int)NativeInputs::INPUT_ATTACK);

			/*
			 * ========================================================
			 * ATTACK RELEASE
			 * ========================================================
			 */
			if (m_WasShooting && isShootingReleased)
			{
				releaseHeldEntity();

				m_HasShownNotification = false;
			}

			/*
			 * ========================================================
			 * AIM + ATTACK HELD
			 * ========================================================
			 */
			if (isAimingHeld && isShootingHeld)
			{
				/*
				 * Acquire an entity only once.
				 */
				if (m_HeldEntity.GetHandle() == 0)
				{
					auto entityID = 0;
					PLAYER::SET_PLAYER_TARGETING_MODE(3);
					PLAYER::GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(Self::GetPlayer().GetId(), &entityID);

					if (entityID != 0 && ENTITY::DOES_ENTITY_EXIST(entityID) && ENTITY::IS_ENTITY_A_PED(entityID))
					{
						m_HeldEntity = entityID;

						m_HeldEntity.ForceControl();

						if (!m_HasShownNotification)
						{
							Notifications::Show("Force Push", "Force Push grabbed entity: " + std::to_string(entityID), NotificationType::Success, 2000);

							m_HasShownNotification = true;
						}
					}
				}

				/*
				 * Keep the entity physically attached to the point
				 * where the crosshair is currently pointing.
				 */
				if (m_HeldEntity.GetHandle() != 0 && ENTITY::DOES_ENTITY_EXIST(m_HeldEntity.GetHandle()))
				{
					attachEntityToCrosshair(m_HeldEntity);
				}
			}

			/*
			 * Safety cleanup.
			 */
			if (isShootingReleased && m_HeldEntity.GetHandle() != 0)
			{
				releaseHeldEntity();

				m_HasShownNotification = false;
			}

			/*
			 * Remember Attack state for the next tick.
			 */
			m_WasShooting = isShootingHeld;
		}

		virtual void OnDisable() override
		{
			/*
			 * Don't leave an entity attached if the feature is
			 * disabled while holding it.
			 */
			if (m_HeldEntity.GetHandle() != 0)
			{
				releaseHeldEntity();
			}

			m_HeldEntity = 0;
			m_WasShooting = false;
			m_HasShownNotification = false;

			/*
			 * Re-enable controls.
			 */
			PAD::DISABLE_CONTROL_ACTION(0, (int)NativeInputs::INPUT_AIM, false);

			PAD::DISABLE_CONTROL_ACTION(0, (int)NativeInputs::INPUT_ATTACK, false);
		}
	};

	static ForcePush _ForcePush{"forcepush", "Force Push", "Grab enemies at the crosshair and throw them when attack is released"};
}