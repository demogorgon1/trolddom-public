#pragma once

#include "../Data/Ability.h"
#include "../Data/AbilityModifier.h"

#include "../CastInProgress.h"
#include "../ComponentBase.h"
#include "../LootRule.h"
#include "../MoveSpeed.h"
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
				COMBAT_FLAG_PUSHABLE = 0x08,
				COMBAT_FLAG_ELITE = 0x10,
				COMBAT_FLAG_HIDE_LEVEL = 0x20,
				COMBAT_FLAG_NOT_TARGETABLE = 0x40
			};

			struct Interrupt
			{
				uint32_t		m_cooldownId = 0;
				int32_t			m_ticks = 0;
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
				FIELD_COMBAT_FLAGS,
				FIELD_CREATURE_TYPE_ID,
				FIELD_MOVE_SPEED,
				FIELD_STEALTH_LEVEL,
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_TARGET_ENTITY_INSTANCE_ID, NULL, offsetof(CombatPublic, m_targetEntityInstanceId))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_LEVEL, "level", offsetof(CombatPublic, m_level));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_FACTION_ID, "faction", offsetof(CombatPublic, m_factionId))->SetDataType(DataType::ID_FACTION);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_DIALOGUE_ROOT_ID, "dialogue_root", offsetof(CombatPublic, m_dialogueRootId))->SetDataType(DataType::ID_DIALOGUE_ROOT)->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->Define(ComponentSchema::TYPE_UINT64, FIELD_COMBAT_GROUP_ID, NULL, offsetof(CombatPublic, m_combatGroupId))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->DefineCustomObjectsNoSource<ResourceEntry>(FIELD_RESOURCES, offsetof(CombatPublic, m_resources));
				aSchema->DefineCustomOptionalObjectNoSource<CastInProgress>(FIELD_CAST_IN_PROGRESS, offsetof(CombatPublic, m_castInProgress))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->DefineCustomPODNoSource<LootRule::Id>(FIELD_LOOT_RULE, offsetof(CombatPublic, m_lootRule))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->DefineCustomPODNoSource<Rarity::Id>(FIELD_LOOT_THRESHOLD, offsetof(CombatPublic, m_lootThreshold))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->DefineCustomPODNoSource<uint8_t>(FIELD_COMBAT_FLAGS, offsetof(CombatPublic, m_combatFlags));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_CREATURE_TYPE_ID, "creature_type", offsetof(CombatPublic, m_creatureTypeId))->SetDataType(DataType::ID_CREATURE_TYPE)->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->DefineCustomPODNoSource<MoveSpeed::Id>(FIELD_MOVE_SPEED, offsetof(CombatPublic, m_moveSpeed))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
				aSchema->DefineCustomPODNoSource<uint8_t>(FIELD_STEALTH_LEVEL, offsetof(CombatPublic, m_stealthLevel))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);

				aSchema->AddSourceModifier<CombatPublic>("not_pushable", [](
					CombatPublic*		aCombatPublic,
					const SourceNode*	/*aSource*/)
				{
					aCombatPublic->m_combatFlags &= ~COMBAT_FLAG_PUSHABLE;
				});

				aSchema->AddSourceModifier<CombatPublic>("elite", [](
					CombatPublic*		aCombatPublic,
					const SourceNode*	/*aSource*/)
				{
					aCombatPublic->m_combatFlags |= COMBAT_FLAG_ELITE;
				});

				aSchema->AddSourceModifier<CombatPublic>("hide_level", [](
					CombatPublic*		aCombatPublic,
					const SourceNode*	/*aSource*/)
				{
					aCombatPublic->m_combatFlags |= COMBAT_FLAG_HIDE_LEVEL;
				});

				aSchema->AddSourceModifier<CombatPublic>("not_targetable", [](
					CombatPublic*		aCombatPublic,
					const SourceNode*	/*aSource*/)
				{
					aCombatPublic->m_combatFlags |= COMBAT_FLAG_NOT_TARGETABLE;
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
			GetResourcePercentage(
				uint32_t				aResourceId) const
			{
				for (const ResourceEntry& t : m_resources)
				{
					if(t.m_id == aResourceId && t.m_max > 0)
						return (100 * t.m_current) / t.m_max;
				}
				return 0;
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

			uint32_t
			GetResourceMax(
				uint32_t				aResourceId) const
			{
				for (const ResourceEntry& t : m_resources)
				{
					if (t.m_id == aResourceId)
						return t.m_max;
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

			const ResourceEntry*
			GetResourceEntry(
				uint32_t				aResourceId) const
			{
				for (const ResourceEntry& t : m_resources)
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
				const Data::Ability*								aAbility,
				const std::vector<const Data::AbilityModifier*>*	aModifiers,
				uint32_t											aBaseMana,
				float												aCostMultiplier = 1.0f) const
			{
				for(uint32_t resourceId = 1; resourceId < (uint32_t)Resource::NUM_IDS; resourceId++)
				{
					int32_t cost = (int32_t)aAbility->m_resourceCosts[resourceId];

					if(aModifiers != NULL && cost != 0)
					{
						for (const Data::AbilityModifier* modifier : *aModifiers)
						{
							if (modifier->m_modifyResourceCost.has_value() && modifier->m_modifyResourceCost->m_resourceId == resourceId)
								cost += modifier->m_modifyResourceCost->m_value;
						}						
					}

					if (resourceId == (uint32_t)Resource::ID_MANA)
					{
						// Mana cost is always as percentage of base mana
						cost = ((int32_t)aBaseMana * cost) / 100;
					}

					if(cost > 0 && aCostMultiplier != 1.0f)
						cost = (int32_t)((float)cost * aCostMultiplier);

					if(cost > 0 && (uint32_t)cost > GetResource(resourceId))
						return false;
				}
				return true;
			}

			bool
			SubtractResourcesForAbility(
				const Data::Ability*								aAbility,
				const std::vector<const Data::AbilityModifier*>*	aModifiers,
				uint32_t											aBaseMana,
				float												aCostMultiplier = 1.0f)
			{
				for (uint32_t resourceId = 1; resourceId < (uint32_t)Resource::NUM_IDS; resourceId++)
				{
					uint32_t cost = aAbility->m_resourceCosts[resourceId];

					if(aModifiers != NULL && cost != 0)
					{
						for (const Data::AbilityModifier* modifier : *aModifiers)
						{
							if (modifier->m_modifyResourceCost.has_value() && modifier->m_modifyResourceCost->m_resourceId == resourceId)
								cost += modifier->m_modifyResourceCost->m_value;
						}						
					}

					if (resourceId == (uint32_t)Resource::ID_MANA)
					{
						// Mana cost is always as percentage of base mana
						cost = ((int32_t)aBaseMana * cost) / 100;
					}

					if (cost > 0 && aCostMultiplier != 1.0f)
						cost = (int32_t)((float)cost * aCostMultiplier);

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

			void
			SetResurrectResources()
			{				
				for (ResourceEntry& t : m_resources)
				{
					const Resource::Info* resourceInfo = Resource::GetInfo((Resource::Id)t.m_id);
					t.m_current = resourceInfo->m_resurrectValue;
				}
			}

			void
			SetDefaultResources()
			{
				for (ResourceEntry& t : m_resources)
				{
					const Resource::Info* resourceInfo = Resource::GetInfo((Resource::Id)t.m_id);

					if(resourceInfo->m_flags & Resource::FLAG_DEFAULT_TO_MAX)
						t.m_current = t.m_max;
					else
						t.m_current = 0;
				}
			}

			void
			SetLevelUpResources()
			{
				for (ResourceEntry& t : m_resources)
				{
					const Resource::Info* resourceInfo = Resource::GetInfo((Resource::Id)t.m_id);

					if(resourceInfo->m_flags & Resource::FLAG_DEFAULT_TO_MAX)
						t.m_current = t.m_max;
				}
			}

			bool
			SetZeroResources()
			{
				bool changed = false;
				for (ResourceEntry& t : m_resources)
				{
					if(t.m_current > 0)
					{
						changed = true;
						t.m_current = 0;
					}
				}
				return changed;
			}

			bool
			IsOneOfCreatureTypes(
				const std::vector<uint32_t>&	aCreatureTypeIds) const
			{
				for(uint32_t id : aCreatureTypeIds)
				{
					if(m_creatureTypeId == id)
						return true;
				}
				return false;
			}

			void
			Reset()
			{
				m_targetEntityInstanceId = 0;
				m_level = 1;
				m_factionId = 0;
				m_dialogueRootId = 0;
				m_creatureTypeId = 0;
				m_combatGroupId = 0;
				m_resources.clear();
				m_castInProgress.reset();
				m_lootRule = LootRule::INVALID_ID;
				m_lootThreshold = Rarity::INVALID_ID;
				m_combatFlags = COMBAT_FLAG_PUSHABLE;
				m_moveSpeed = MoveSpeed::ID_NORMAL;
				m_stealthLevel = 0;

				m_lastCombatEventTick = 0;
				m_lastCastingTick = -50;
				m_last5SecRuleManaRegenTick = 0;
				m_lastBaseManaRegenTick = 0;
				m_interrupt.reset();
				m_damageAccum = 0;
				m_singleTargetAuraEntityInstanceId = 0;
				m_lastDamageFromEntityId = 0;
				m_doNotZeroResources = false;
				m_pvpCombatEvent = false;
				m_lastPVPCombatEventTick = 0;
			}

			// Helpers
			bool IsMasterLooter() const { return (m_combatFlags & COMBAT_FLAG_MASTER_LOOTER) != 0; }
			bool IsElite() const { return (m_combatFlags & COMBAT_FLAG_ELITE) != 0; }
			bool ShouldHideLevel() const { return (m_combatFlags & COMBAT_FLAG_HIDE_LEVEL) != 0; }
			bool IsNotTargetable() const { return (m_combatFlags & COMBAT_FLAG_NOT_TARGETABLE) != 0; }
			bool IsPVPFlagged() const { return (m_combatFlags & COMBAT_FLAG_PVP) != 0; }

			// Public data
			uint32_t						m_targetEntityInstanceId = 0;
			uint32_t						m_level = 1;
			uint32_t						m_factionId = 0;
			uint32_t						m_dialogueRootId = 0;
			uint32_t						m_creatureTypeId = 0;
			uint64_t						m_combatGroupId = 0;
			std::vector<ResourceEntry>		m_resources;
			std::optional<CastInProgress>	m_castInProgress;
			LootRule::Id					m_lootRule = LootRule::INVALID_ID;
			Rarity::Id						m_lootThreshold = Rarity::INVALID_ID;
			uint8_t							m_combatFlags = COMBAT_FLAG_PUSHABLE;
			MoveSpeed::Id					m_moveSpeed = MoveSpeed::ID_NORMAL;
			uint8_t							m_stealthLevel = 0;

			// Internal server-side only
			int32_t							m_lastCombatEventTick = 0;
			int32_t							m_lastCastingTick = -50;
			int32_t							m_last5SecRuleManaRegenTick = 0;
			int32_t							m_lastBaseManaRegenTick = 0;
			std::optional<Interrupt>		m_interrupt;
			uint32_t						m_damageAccum = 0;
			uint32_t						m_singleTargetAuraEntityInstanceId = 0;
			uint32_t						m_lastDamageFromEntityId = 0;
			bool							m_doNotZeroResources = false;
			bool							m_pvpCombatEvent = false;
			int32_t							m_lastPVPCombatEventTick = 0;
		};

	}

}