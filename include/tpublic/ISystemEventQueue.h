#pragma once

#include "PlayerTag.h"

namespace tpublic
{

	namespace Components
	{
		struct Lootable;
	}

	class ISystemEventQueue
	{
	public:
		virtual				~ISystemEventQueue() {}

		// Virtual interface
		virtual void		AddKillXPEvent(
								const PlayerTag&		aPlayerTag,
								uint32_t				aKillLevel) = 0;
		virtual void		AddLootGenerationEvent(
								const PlayerTag&		aPlayerTag, 
								Components::Lootable*	aLootable) = 0;
	};

}