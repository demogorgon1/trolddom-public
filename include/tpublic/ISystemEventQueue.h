#pragma once

#include <tpublic/Components/Lootable.h>

#include "PlayerTag.h"

namespace tpublic
{

	class ISystemEventQueue
	{
	public:
		virtual				~ISystemEventQueue() {}

		// Virtual interface
		virtual void		AddIndividualKillXPEvent(
								uint32_t											aCharacterId,
								uint32_t											aCharacterLevel,
								uint32_t											aKillLevel) = 0;
		virtual void		AddGroupKillXPEvent(
								uint64_t											aGroupId,
								uint32_t											aKillLevel) = 0;
		virtual void		AddGroupLootEvent(
								uint32_t											aEntityInstanceId,
								uint64_t											aGroupId,
								const tpublic::Components::Lootable::AvailableLoot& aLoot,
								size_t												aLootIndex,
								const std::vector<uint32_t>&						aGroupMemberEntityInstanceIds) = 0;
	};

}