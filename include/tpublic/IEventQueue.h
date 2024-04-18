#pragma once

#include <tpublic/Components/Lootable.h>

#include "EntityState.h"
#include "ItemInstanceReference.h"
#include "Vec2.h"

namespace tpublic
{

	namespace Data
	{
		struct Ability;
		struct AbilityModifier;
	}

	class CastInProgress;
	class EntityInstance;
	class SecondaryAbility;

	class IEventQueue
	{
	public:
		virtual ~IEventQueue() {}

		enum EventQueueAbilityResult
		{
			EVENT_QUEUE_ABILITY_RESULT_OK,
			EVENT_QUEUE_ABILITY_RESULT_TOO_FAR_AWAY,
			EVENT_QUEUE_ABILITY_RESULT_NO_LINE_OF_SIGHT,
			EVENT_QUEUE_ABILITY_RESULT_MISSING_INGREDIENTS,
			EVENT_QUEUE_ABILITY_RESULT_NOT_ENOUGH_SPACE_IN_INVENTORY,
			EVENT_QUEUE_ABILITY_RESULT_FAILURE
		};

		struct EventQueueMoveRequest
		{
			static const uint32_t MAX_PRIORITY_LIST_LENGTH = 4;

			enum Type
			{
				TYPE_SIMPLE,
				TYPE_FIND_PATH
			};

			void
			AddToPriorityList(
				const Vec2& aDirection)
			{
				assert(m_priorityListLength < MAX_PRIORITY_LIST_LENGTH);
				m_priorityList[m_priorityListLength++] = aDirection;
			}

			// Public data
			Type								m_type = Type(0);
			uint32_t							m_entityInstanceId = 0;
			Vec2								m_priorityList[MAX_PRIORITY_LIST_LENGTH];
			uint32_t							m_priorityListLength = 0;
			bool								m_canMoveOnAllNonViewBlockingTiles = false;
		};

		struct EventQueueMoveAdjacentRequest
		{
			uint32_t							m_entityInstanceId = 0;
			uint32_t							m_adjacentEntityInstanceId = 0;
			uint32_t							m_maxSteps = 0;
			const SecondaryAbility*				m_triggerAbilityOnResolve = NULL;
		};

		struct EventQueueSpawnEntityRequest
		{
			uint32_t							m_entityId = 0;
			Vec2								m_position;
			uint32_t							m_onlyVisibleToPlayerEntityInstanceId = 0;
		};

		// Virtual interface
		virtual EventQueueAbilityResult		EventQueueAbility(
												uint32_t													aSourceEntityInstanceId,
												uint32_t													aTargetEntityInstanceId,
												const Vec2&													aAOETarget,
												const Data::Ability*										aAbility,
												const ItemInstanceReference&								aItem,
												const std::vector<const Data::AbilityModifier*>*			aModifiers) = 0;
		virtual void						EventQueueMove(
												const EventQueueMoveRequest&								aMoveRequest) = 0;
		virtual void						EventQueueMoveAdjacent(
												const EventQueueMoveAdjacentRequest&						aMoveAdjacentRequest) = 0;
		virtual void						EventQueueIndividualKillXP(
												uint32_t													aCharacterId,
												uint32_t													aCharacterLevel,
												uint32_t													aKillLevel,
												uint32_t													aKillEntityId) = 0;
		virtual void						EventQueueGroupKillXP(
												uint64_t													aGroupId,
												uint32_t													aKillLevel,
												uint32_t													aKillEntityId) = 0;
		virtual void						EventQueueGroupLoot(
												uint32_t													aEntityInstanceId,
												uint64_t													aGroupId,
												const tpublic::Components::Lootable::AvailableLoot&			aLoot,
												size_t														aLootIndex,
												const std::vector<uint32_t>&								aGroupMemberEntityInstanceIds) = 0;
		virtual void						EventQueueChanneling(
												uint32_t													aSourceEntityInstanceId,
												const CastInProgress&										aCastInProgress) = 0;
		virtual void						EventQueueCancelChanneling(
												uint32_t													aSourceEntityInstanceId,
												uint32_t													aTargetEntityInstanceId,
												uint32_t													aChanneledAbilityId,
												int32_t														aStart) = 0;
		virtual void						EventQueueInterrupt(
												uint32_t													aSourceEntityInstanceId,
												uint32_t													aTargetEntityInstanceId) = 0;
		virtual uint32_t					EventQueueGetNextGroupRoundRobinCharacterId(
												uint64_t													aGroupId) = 0;
		virtual void						EventQueueThreat(
												uint32_t													aSourceEntityInstanceId,
												uint32_t													aTargetEntityInstanceId,
												int32_t														aThreat,
												const std::optional<float>&									aMultiply = std::optional<float>()) = 0;
		virtual void						EventQueueThreatClear(
												uint32_t													aEntityInstanceId) = 0;
		virtual tpublic::EntityInstance*	EventQueueSpawnEntity(
												uint32_t													aEntityId,
												tpublic::EntityState::Id									aInitState) = 0;
		virtual void						EventQueueMakeOffering(
												uint32_t													aSourceEntityInstanceId,
												uint32_t													aTargetEntityInstanceId,
												const ItemInstanceReference&								aItem) = 0;
		virtual void						EventQueueGenerateLoot(
												uint32_t													aLootableEntityInstanceId) = 0;
	};

}