#pragma once

#include "../Data/Ability.h"

#include "../CastInProgress.h"
#include "../ComponentBase.h"
#include "../LootRule.h"
#include "../Rarity.h"
#include "../Resource.h"

namespace tpublic
{

	class Manifest;

	namespace Components
	{

		struct CombatPublic
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_COMBAT_PUBLIC;
			static const uint8_t FLAGS = 0;
			static const Replication REPLICATION = REPLICATION_PUBLIC;
			static const Persistence::Id PERSISTENCE = Persistence::ID_VOLATILE;

			enum CombatFlag : uint8_t
			{
				COMBAT_FLAG_MASTER_LOOTER = 0x01,
				COMBAT_FLAG_PVP = 0x02,
				COMBAT_FLAG_PUSHABLE = 0x08
			};

			struct ResourceEntry
			{	
				void
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WriteUInt(m_id);
					aStream->WriteUInt(m_current);
					aStream->WriteUInt(m_max);
				}

				bool
				FromStream(
					IReader*			aStream) 
				{
					if (!aStream->ReadUInt(m_id))
						return false;
					if (!aStream->ReadUInt(m_current))
						return false;
					if (!aStream->ReadUInt(m_max))
						return false;
					return true;
				}

				// Public data
				uint32_t	m_id = 0;
				uint32_t	m_current = 0;
				uint32_t	m_max = 0;
			};
						
			enum Field
			{
				FIELD_TARGET_ENTITY_INSTANCE_ID,
				FIELD_LEVEL,
				FIELD_FACTION_ID,
				FIELD_DIALOGUE_ROOT_ID,
				FIELD_COMBAT_GROUP_ID,
				FIELD_RESOURCES,
				FIELD_CAST_IN_PROGRESS,
				FIELD_LOOT_RULE,
				FIELD_LOOT_THRESHOLD,
				FIELD_COMBAT_FLAGS
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_TARGET_ENTITY_INSTANCE_ID, NULL, offsetof(CombatPublic, m_targetEntityInstanceId));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_LEVEL, "level", offsetof(CombatPublic, m_level));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_FACTION_ID, "faction", offsetof(CombatPublic, m_factionId))->SetDataType(DataType::ID_FACTION);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_DIALOGUE_ROOT_ID, "dialogue_root", offsetof(CombatPublic, m_dialogueRootId))->SetDataType(DataType::ID_DIALOGUE_ROOT)->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->Define(ComponentSchema::TYPE_UINT64, FIELD_COMBAT_GROUP_ID, NULL, offsetof(CombatPublic, m_combatGroupId))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->DefineCustomObjectsNoSource<ResourceEntry>(FIELD_RESOURCES, offsetof(CombatPublic, m_resources));
				aSchema->DefineCustomOptionalObjectNoSource<CastInProgress>(FIELD_CAST_IN_PROGRESS, offsetof(CombatPublic, m_castInProgress))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->DefineCustomPODNoSource<LootRule::Id>(FIELD_LOOT_RULE, offsetof(CombatPublic, m_lootRule))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->DefineCustomPODNoSource<Rarity::Id>(FIELD_LOOT_THRESHOLD, offsetof(CombatPublic, m_lootThreshold))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->DefineCustomPODNoSource<uint8_t>(FIELD_COMBAT_FLAGS, offsetof(CombatPublic, m_combatFlags));

				aSchema->AddSourceModifier<CombatPublic>("not_pushable", [](
					CombatPublic*		aCombatPublic,
					const SourceNode*	/*aSource*/)
				{
					aCombatPublic->m_combatFlags &= ~COMBAT_FLAG_PUSHABLE;
				});
			}

			void
			AddResourceMax(
				uint32_t				aResourceId,
				uint32_t				aValue)
			{
				for(ResourceEntry& t : m_resources)
				{
					if(t.m_id == aResourceId)
					{
						t.m_max += aValue;
						return;
					}
				}
				m_resources.push_back({ aResourceId, 0, aValue });
			}

			uint32_t
			GetResource(
				uint32_t				aResourceId,
				uint32_t*				aOutMax = NULL) const
			{
				for (const ResourceEntry& t : m_resources)
				{
					if (t.m_id == aResourceId)
					{
						if(aOutMax != NULL)
							*aOutMax = t.m_max;
						
						return t.m_current;
					}
				}
				return 0;
			}

			ResourceEntry*
			GetResourceEntry(
				uint32_t				aResourceId)
			{
				for (ResourceEntry& t : m_resources)
				{
					if (t.m_id == aResourceId)
						return &t;
				}
				return NULL;
			}

			void
			SetResourceToMax(
				uint32_t				aResourceId)
			{
				for (ResourceEntry& t : m_resources)
				{
					if (t.m_id == aResourceId)
					{
						t.m_current = t.m_max;
						return;
					}
				}
			}

			bool
			GetResourceIndex(
				uint32_t				aResourceId,
				size_t&					aOut) const
			{
				size_t i = 0;
				for (const ResourceEntry& t : m_resources)
				{
					if (t.m_id == aResourceId)
					{
						aOut = i;
						return true;
					}
					i++;
				}
				return false;
			}

			bool
			GetSecondaryResourceIndex(
				size_t&					aOut) const
			{
				// Secondary resource is the first one that's not health
				size_t i = 0;
				for (const ResourceEntry& t : m_resources)
				{
					if (t.m_id != Resource::ID_HEALTH)
					{
						aOut = i;
						return true;
					}
					i++;
				}
				return false;
			}

			void
			SetResource(
				uint32_t				aResourceId,
				uint32_t				aValue)
			{
				for (ResourceEntry& t : m_resources)
				{
					if (t.m_id == aResourceId)
					{
						t.m_current = aValue;
						return;
					}
				}
			}

			bool
			HasResourcesForAbility(
				const Data::Ability*	aAbility) const
			{
				for(uint32_t resourceId = 1; resourceId < (uint32_t)Resource::NUM_IDS; resourceId++)
				{
					uint32_t cost = aAbility->m_resourceCosts[resourceId];
					if(cost > 0 && cost > GetResource(resourceId))
						return false;
				}
				return true;
			}

			bool
			SubtractResourcesForAbility(
				const Data::Ability*	aAbility)
			{
				for (uint32_t resourceId = 1; resourceId < (uint32_t)Resource::NUM_IDS; resourceId++)
				{
					uint32_t cost = aAbility->m_resourceCosts[resourceId];
					if(cost > 0)
					{
						size_t index;
						if(!GetResourceIndex(resourceId, index))
							return false;

						if(m_resources[index].m_current < cost)
							return false;

						m_resources[index].m_current -= cost;						
					}				
				}

				return true;
			}

			// Helpers
			bool IsMasterLooter() const { return (m_combatFlags & COMBAT_FLAG_MASTER_LOOTER) != 0; }

			// Public data
			uint32_t						m_targetEntityInstanceId = 0;
			uint32_t						m_level = 1;
			uint32_t						m_factionId = 0;
			uint32_t						m_dialogueRootId = 0;
			uint64_t						m_combatGroupId = 0;
			std::vector<ResourceEntry>		m_resources;
			std::optional<CastInProgress>	m_castInProgress;
			LootRule::Id					m_lootRule = LootRule::INVALID_ID;
			Rarity::Id						m_lootThreshold = Rarity::INVALID_ID;
			uint8_t							m_combatFlags = COMBAT_FLAG_PUSHABLE;

			// Internal server-side only
			int32_t							m_lastCombatEventTick = 0;
			int32_t							m_lastCastingTick = -50;
			int32_t							m_last5SecRuleManaRegenTick = 0;
			int32_t							m_lastBaseManaRegenTick = 0;
		};

	}

}