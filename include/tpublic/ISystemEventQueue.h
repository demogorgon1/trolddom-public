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
								const PlayerTag&				aPlayerTag,
								uint32_t						aKillLevel) = 0;
		virtual void		AddGroupLootEvent(
								uint32_t						aEntityInstanceId,
								Components::Lootable*			aLootable,
								const std::vector<uint32_t>&	aGroupMemberEntityInstanceIds) = 0;
	};

}