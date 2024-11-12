#pragma once

#include <tpublic/Components/Lootable.h>

#include "EntityState.h"
#include "ItemInstanceReference.h"
#include "UniformDistribution.h"
#include "Vec2.h"

namespace tpublic
{

	namespace Data
	{
		struct Ability;
		struct AbilityModifier;
	}

	class AbilityModifierList;
	class CastInProgress;
	class Chat;
	class EntityInstance;
	class SecondaryAbility;
	class SourceEntityInstance;

	class IEventQueue
	{
	public:
		virtual ~IEventQueue() {}

		enum EventQueueAbilityResult
		{
			EVENT_QUEUE_ABILITY_RESULT_OK,
			EVENT_QUEUE_ABILITY_RESULT_TOO_FAR_AWAY,
			EVENT_QUEUE_ABILITY_RESULT_TOO_CLOSE,
			EVENT_QUEUE_ABILITY_RESULT_NO_LINE_OF_SIGHT,
			EVENT_QUEUE_ABILITY_RESULT_MISSING_INGREDIENTS,
			EVENT_QUEUE_ABILITY_RESULT_NOT_ENOUGH_SPACE_IN_INVENTORY,
			EVENT_QUEUE_ABILITY_RESULT_TOO_LOW_LEVEL,
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
				const Vec2&		aDirection)
			{
				assert(m_priorityListLength < MAX_PRIORITY_LIST_LENGTH);
				m_priorityList[m_priorityListLength++] = aDirection;
			}

			void
			ShufflePriorityList(
				std::mt19937&	aRandom)
			{
				if(m_priorityListLength > 1)
				{
					// Fisher-yates shuffle
					for (uint32_t i = m_priorityListLength - 1; i > 0; i--)
					{
						UniformDistribution<uint32_t> distribution(0, i);
						uint32_t j = distribution(aRandom);
						std::swap(m_priorityList[i], m_priorityList[j]);
					}
				}
			}

			// Public data
			Type								m_type = Type(0);
			uint32_t							m_entityInstanceId = 0;
			Vec2								m_priorityList[MAX_PRIORITY_LIST_LENGTH];
			uint32_t							m_priorityListLength = 0;
			bool								m_canMoveOnAllNonViewBlockingTiles = false;
			bool								m_emptySpacePrio = false;
			bool								m_setUpdatedOnServerFlag = false;
			bool								m_setTeleportedFlag = false;
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
												const SourceEntityInstance&									aSourceEntityInstance,
												uint32_t													aTargetEntityInstanceId,
												const Vec2&													aAOETarget,
												const Data::Ability*										aAbility,
												const ItemInstanceReference&								aItem = ItemInstanceReference(),
												const AbilityModifierList*									aAbilityModifierList = NULL,
												const std::optional<tpublic::Vec2>&							aOrigin = std::optional<tpublic::Vec2>()) = 0;
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
												uint32_t													aTargetEntityInstanceId,
												uint32_t													aCooldownId,
												int32_t														aLockoutTicks) = 0;
		virtual uint32_t					EventQueueGetNextGroupRoundRobinCharacterId(
												uint64_t													aGroupId) = 0;
		virtual void						EventQueueThreat(
												const SourceEntityInstance&									aSourceEntityInstance,
												uint32_t													aTargetEntityInstanceId,
												int32_t														aThreat,
												int32_t														aTick,
												const std::optional<float>&									aMultiply = std::optional<float>()) = 0;
		virtual void						EventQueueThreatClear(
												uint32_t													aEntityInstanceId) = 0;
		virtual tpublic::EntityInstance*	EventQueueSpawnEntity(
												uint32_t													aEntityId,
												tpublic::EntityState::Id									aInitState,
												uint32_t													aMapEntitySpawnId,
												bool														aDetachedFromSpawn) = 0;
		virtual void						EventQueueMakeOffering(
												uint32_t													aSourceEntityInstanceId,
												uint32_t													aTargetEntityInstanceId,
												const ItemInstanceReference&								aItem) = 0;
		virtual void						EventQueueGenerateLoot(
												uint32_t													aLootableEntityInstanceId) = 0;
		virtual void						EventQueueChat(
												uint32_t													aEntityInstanceId,
												const Chat&													aChat) = 0;
		virtual void						EventQueueLearnProfessionAbility(
												uint32_t													aEntityInstanceId,
												uint32_t													aProfessionId,
												uint32_t													aAbilityId) = 0;
		virtual void						EventQueueStartEncounter(
												uint32_t													aEntityId,	
												uint32_t													aEntityInstanceId,
												uint32_t													aEncounterId) = 0;
		virtual void						EventQueueBuyFromVendor(
												uint32_t													aPlayerEntityInstanceId,
												uint32_t													aVendorEntityInstanceId,
												uint32_t													aItemId,
												uint32_t													aQuantity,
												int64_t														aCost) = 0;
	};

}