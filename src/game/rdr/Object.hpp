#pragma once
#include "Entity.hpp"
#include "game/rdr/Natives.hpp"

namespace YimMenu
{
	class Object : public Entity
	{
	public:
		using Entity::Entity;

		static Object Create(uint32_t model, rage::fvector3 coords);
	};
}