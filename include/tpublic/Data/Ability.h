#pragma once

#include "../DataBase.h"
#include "../DirectEffectFactory.h"
#include "../DirectEffectBase.h"
#include "../EntityState.h"
#include "../Requirement.h"
#include "../Resource.h"
#include "../SoundEffect.h"
#include "../UIntRange.h"

namespace tpublic
{

	namespace Data
	{

		struct Ability
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ABILITY; 
			static const bool TAGGED = true;

			enum Flag : uint32_t
			{
				FLAG_TARGET_SELF					= 0x00000001,
				FLAG_TARGET_OTHER					= 0x00000002,
				FLAG_TARGET_AOE						= 0x00000004,
				FLAG_TARGET_HOSTILE					= 0x00000008,
				FLAG_TARGET_FRIENDLY				= 0x00000010,
				FLAG_CAN_MISS						= 0x00000020,
				FLAG_CAN_BE_DODGED					= 0x00000040,
				FLAG_CAN_BE_PARRIED					= 0x00000080,
				FLAG_CAN_BE_BLOCKED					= 0x00000100,
				FLAG_ATTACK							= 0x00000200,
				FLAG_USE_WEAPON_ICON				= 0x00000400,				
				FLAG_AOE_LOW_HEALTH_ONLY			= 0x00000800,
				FLAG_AOE_LOW_HEALTH_PRIO			= 0x00001000,
				FLAG_OFFENSIVE						= 0x00002000,
				FLAG_SPELL							= 0x00004000,
				FLAG_MELEE							= 0x00008000,
				FLAG_RANGED							= 0x00010000,
				FLAG_ITEM							= 0x00020000,
				FLAG_ALWAYS_IN_RANGE				= 0x00040000,
				FLAG_ALWAYS_IN_LINE_OF_SIGHT		= 0x00080000,
				FLAG_CRAFTING						= 0x00100000,
				FLAG_HIDDEN							= 0x00200000,
				FLAG_LATE_COOLDOWN_TRIGGER			= 0x00400000,
				FLAG_TARGET_AOE_FRIENDLY			= 0x00800000,
				FLAG_TARGET_AOE_HOSTILE				= 0x01000000,
				FLAG_TRIGGER_MOVE_COOLDOWN			= 0x02000000,
			};

			static inline Resource::Id
			GetResourceId(
				const SourceNode*			aSource)
			{
				Resource::Id resourceId = Resource::StringToId(aSource->m_name.c_str());
				TP_VERIFY(resourceId != Resource::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid resource id.", aSource->m_name.c_str());
				return resourceId;
			}

			static inline uint32_t
			GetFlags(
				const SourceNode*			aSource)
			{
				uint32_t flags = 0;
				aSource->GetArray()->ForEachChild([&](
					const SourceNode*		aChild)
				{
					const char* identifier = aChild->GetIdentifier();
					if (strcmp(identifier, "target_self") == 0)
						flags |= FLAG_TARGET_SELF;
					else if (strcmp(identifier, "target_other") == 0)
						flags |= FLAG_TARGET_OTHER;
					else if (strcmp(identifier, "target_aoe") == 0)
						flags |= FLAG_TARGET_AOE;
					else if (strcmp(identifier, "target_hostile") == 0)
						flags |= FLAG_TARGET_HOSTILE;
					else if (strcmp(identifier, "target_friendly") == 0)
						flags |= FLAG_TARGET_FRIENDLY;
					else if (strcmp(identifier, "can_miss") == 0)
						flags |= FLAG_CAN_MISS;
					else if (strcmp(identifier, "can_be_dodged") == 0)
						flags |= FLAG_CAN_BE_DODGED;
					else if (strcmp(identifier, "can_be_parried") == 0)
						flags |= FLAG_CAN_BE_PARRIED;
					else if (strcmp(identifier, "can_be_blocked") == 0)
						flags |= FLAG_CAN_BE_BLOCKED;
					else if (strcmp(identifier, "attack") == 0)
						flags |= FLAG_ATTACK;
					else if (strcmp(identifier, "use_weapon_icon") == 0)
						flags |= FLAG_USE_WEAPON_ICON;
					else if (strcmp(identifier, "aoe_low_health_only") == 0)
						flags |= FLAG_AOE_LOW_HEALTH_ONLY;
					else if (strcmp(identifier, "aoe_low_health_prio") == 0)
						flags |= FLAG_AOE_LOW_HEALTH_PRIO;
					else if (strcmp(identifier, "offensive") == 0)
						flags |= FLAG_OFFENSIVE;
					else if (strcmp(identifier, "item") == 0)
						flags |= FLAG_ITEM;
					else if (strcmp(identifier, "spell") == 0)
						flags |= FLAG_SPELL;
					else if (strcmp(identifier, "melee") == 0)
						flags |= FLAG_MELEE;
					else if (strcmp(identifier, "ranged") == 0)
						flags |= FLAG_RANGED;
					else if (strcmp(identifier, "always_in_range") == 0)
						flags |= FLAG_ALWAYS_IN_RANGE;
					else if (strcmp(identifier, "always_in_line_of_sight") == 0)
						flags |= FLAG_ALWAYS_IN_LINE_OF_SIGHT;
					else if (strcmp(identifier, "crafting") == 0)
						flags |= FLAG_CRAFTING;
					else if (strcmp(identifier, "hidden") == 0)
						flags |= FLAG_HIDDEN;
					else if (strcmp(identifier, "late_cooldown_trigger") == 0)
						flags |= FLAG_LATE_COOLDOWN_TRIGGER;
					else if (strcmp(identifier, "target_aoe_hostile") == 0)
						flags |= FLAG_TARGET_AOE_HOSTILE;
					else if (strcmp(identifier, "target_aoe_friendly") == 0)
						flags |= FLAG_TARGET_AOE_FRIENDLY;
					else if (strcmp(identifier, "trigger_move_cooldown") == 0)
						flags |= FLAG_TRIGGER_MOVE_COOLDOWN;
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid ability flag.", identifier);
				});
				return flags;
			}

			struct DirectEffectEntry
			{
				DirectEffectEntry()
				{

				}

				DirectEffectEntry(
					const SourceNode*		aSource)
				{
					m_directEffectId = DirectEffect::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_directEffectId != DirectEffect::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid direct effect.", aSource->m_name.c_str());

					std::unique_ptr<DirectEffectBase> effect(aSource->m_sourceContext->m_directEffectFactory->Create(m_directEffectId));
					assert(effect);

					if (!aSource->m_children.empty() || aSource->m_annotation)
						effect->FromSource(aSource);

					m_directEffectBase = std::move(effect);
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_directEffectId);
					aStream->WriteObjectPointer(m_directEffectBase);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadUInt(m_directEffectId))
						return false;

					m_directEffectBase.reset(aStream->GetDirectEffectFactory()->Create(m_directEffectId));
					if(!m_directEffectBase->FromStream(aStream))
						return false;

					return true;
				}

				// Public data
				uint32_t							m_directEffectId = 0;
				std::unique_ptr<DirectEffectBase>	m_directEffectBase;
			};

			struct AOEEntitySpawnEntry
			{
				AOEEntitySpawnEntry()
				{

				}

				AOEEntitySpawnEntry(
					const SourceNode*		aSource)
				{
					m_entityId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ENTITY, aSource->m_name.c_str());
					
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "probability")
						{
							m_probability = aChild->GetProbability();
							if(m_probability == UINT32_MAX)
								m_probability = 0; // 0 means always
						}
						else if(aChild->m_name == "init_state")
						{
							m_initState = EntityState::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_initState != EntityState::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid entity state.", aChild->GetIdentifier());
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					});
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_entityId);
					aStream->WriteUInt(m_probability);
					aStream->WritePOD(m_initState);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadUInt(m_entityId))
						return false;
					if(!aStream->ReadUInt(m_probability))
						return false;
					if(!aStream->ReadPOD(m_initState))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_entityId = 0;
				uint32_t							m_probability = 0;
				EntityState::Id						m_initState = EntityState::ID_DEFAULT;
			};

			struct Item
			{
				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_itemId);
					aStream->WriteUInt(m_quantity);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadUInt(m_itemId))
						return false;
					if(!aStream->ReadUInt(m_quantity))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_itemId = 0;
				uint32_t							m_quantity = 0;
			};

			struct Items
			{
				Items()
				{

				}

				Items(
					const SourceNode*		aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						Item t;
						t.m_itemId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ITEM, aChild->m_name.c_str());
						t.m_quantity = aChild->GetUInt32();
						m_items.push_back(t);
					});
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteObjects(m_items);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadObjects(m_items))
						return false;
					return true;
				}

				// Public data
				std::vector<Item>			m_items;				
			};

			struct RequiredProfession
			{
				RequiredProfession()
				{

				}

				RequiredProfession(
					const SourceNode*		aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing skill annotation.");
					m_skill = aSource->m_annotation->GetUInt32();
					m_professionId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_PROFESSION, aSource->GetIdentifier());
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_professionId);
					aStream->WriteUInt(m_skill);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if (!aStream->ReadUInt(m_professionId))
						return false;
					if (!aStream->ReadUInt(m_skill))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_professionId = 0;	
				uint32_t							m_skill = 0;
			};

			void
			Verify() const
			{
				VerifyBase();

				TP_VERIFY(!m_displayName.empty(), m_debugInfo, "'%s' has no 'display_name'.", m_name.c_str());
			}

			// Helpers
			bool TargetSelf() const { return m_flags & FLAG_TARGET_SELF; }
			bool TargetOther() const { return m_flags & FLAG_TARGET_OTHER; }
			bool TargetAOE() const { return m_flags & FLAG_TARGET_AOE; }
			bool TargetFriendly() const { return m_flags & FLAG_TARGET_FRIENDLY; }
			bool TargetHostile() const { return m_flags & FLAG_TARGET_HOSTILE; }
			bool TargetAOEFriendly() const { return m_flags & FLAG_TARGET_AOE_FRIENDLY; }
			bool TargetAOEHostile() const { return m_flags & FLAG_TARGET_AOE_HOSTILE; }
			bool CanMiss() const { return m_flags & FLAG_CAN_MISS; }
			bool CanBeDodged() const { return m_flags & FLAG_CAN_BE_DODGED; }
			bool CanBeParried() const { return m_flags & FLAG_CAN_BE_PARRIED; }
			bool CanBeBlocked() const { return m_flags & FLAG_CAN_BE_BLOCKED; }
			bool IsAttack() const { return m_flags & FLAG_ATTACK; }
			bool IsSpell() const { return m_flags & FLAG_SPELL; }
			bool AlwaysInRange() const { return m_flags & FLAG_ALWAYS_IN_RANGE; }
			bool IsMelee() const { return m_flags & FLAG_MELEE; }
			bool AlwaysInLineOfSight() const { return m_flags & FLAG_ALWAYS_IN_LINE_OF_SIGHT; }
			bool IsInstantMelee() const { return m_range == 1 && m_castTime == 0; }
			bool IsCrafting() const { return m_flags & FLAG_CRAFTING; }
			bool IsHidden() const { return m_flags & FLAG_HIDDEN; }
			bool IsLateCooldownTrigger() const { return m_flags & FLAG_LATE_COOLDOWN_TRIGGER; }
			bool IsChanneled() const { return m_channelTicks != 0 && m_channelTickAbilityId != 0; }
			bool IsOffensive() const { return m_flags & FLAG_OFFENSIVE; }
			bool IsItem() const { return m_flags & FLAG_ITEM; }
			bool ShouldTriggerMoveCooldown() const { return m_flags & FLAG_TRIGGER_MOVE_COOLDOWN; }
			
			bool 
			IsUsableInState(
				EntityState::Id			aEntityState) const
			{
				for(EntityState::Id entityState : m_entityStates)
				{
					if(aEntityState == entityState)
						return true;
				}
				return false;
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{
				aNode->ForEachChild([&](
					const SourceNode* aMember)
				{
					if(!FromSourceBase(aMember))
					{
						if (aMember->m_name == "string")
							m_displayName = aMember->GetString();
						else if (aMember->m_name == "description")
							m_description = aMember->GetString();
						else if (aMember->m_name == "talent_tree")
							m_talentTreeId = aMember->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TALENT_TREE, aMember->GetIdentifier());
						else if (aMember->m_name == "range")
							m_range = aMember->GetUInt32();
						else if (aMember->m_name == "level")
							m_level = aMember->GetUInt32();
						else if (aMember->m_name == "channel_ticks")
							m_channelTicks = aMember->GetUInt32();
						else if (aMember->m_name == "channel_interval")
							m_channelInterval = aMember->GetInt32();
						else if (aMember->m_name == "channel_tick_ability")
							m_channelTickAbilityId = aMember->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aMember->GetIdentifier());
						else if (aMember->m_name == "aoe_radius")
							m_aoeRadius = aMember->GetUInt32();
						else if (aMember->m_name == "aoe_cap")
							m_aoeCap = aMember->GetUInt32();
						else if (aMember->m_name == "required_level")
							m_requiredLevel = aMember->GetUInt32();
						else if (aMember->m_name == "speed")
							m_speed = aMember->GetInt32();
						else if (aMember->m_name == "delay")
							m_delay = aMember->GetInt32();
						else if (aMember->m_name == "cooldowns")
							aMember->GetIdArray(DataType::ID_COOLDOWN, m_cooldowns);
						else if (aMember->m_name == "cast_time")
							m_castTime = aMember->GetInt32();
						else if (aMember->m_name == "icon")
							m_iconSpriteId = aMember->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aMember->GetIdentifier());
						else if (aMember->m_name == "projectile")
							m_projectileParticleSystemId = aMember->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_PARTICLE_SYSTEM, aMember->GetIdentifier());
						else if (aMember->m_name == "source_particle_system")
							m_sourceParticleSystemId = aMember->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_PARTICLE_SYSTEM, aMember->GetIdentifier());
						else if (aMember->m_name == "target_particle_system")
							m_targetParticleSystemId = aMember->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_PARTICLE_SYSTEM, aMember->GetIdentifier());
						else if (aMember->m_name == "flags")
							m_flags = GetFlags(aMember);
						else if (aMember->m_tag == "direct_effect")
							m_directEffects.push_back(std::make_unique<DirectEffectEntry>(aMember));
						else if (aMember->m_tag == "aoe_entity_spawn")
							m_aoeEntitySpawns.push_back(std::make_unique<AOEEntitySpawnEntry>(aMember->GetObject()));
						else if (aMember->m_name == "states")
							aMember->GetIdArrayWithLookup<EntityState::Id, EntityState::INVALID_ID>(m_entityStates, [&](const char* aIdentifier) { return EntityState::StringToId(aIdentifier); });
						else if (aMember->m_tag == "resource_cost")
							m_resourceCosts[GetResourceId(aMember)] = aMember->GetUInt32();
						else if (aMember->m_name == "consume_items")
							m_consumeItems = std::make_unique<Items>(aMember);
						else if (aMember->m_name == "produce_items")
							m_produceItems = std::make_unique<Items>(aMember);
						else if (aMember->m_name == "required_profession")
							m_requiredProfession = RequiredProfession(aMember);
						else if (aMember->m_tag == "requirement")
							m_requirements.push_back(Requirement(aMember));
						else if (aMember->m_tag == "aoe_requirement")
							m_aoeRequirements.push_back(Requirement(aMember));
						else if (aMember->m_name == "sound_effects")
							m_soundEffects.FromSource(aMember);
						else if(aMember->m_name == "npc_level_range")
							m_npcLevelRange = UIntRange(aMember);
						else
							TP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteString(m_displayName);
				aWriter->WriteString(m_description);
				aWriter->WriteUInt(m_talentTreeId);
				aWriter->WriteUInt(m_range);
				aWriter->WriteInt(m_speed);
				aWriter->WriteInt(m_delay);
				aWriter->WriteUInts(m_cooldowns);
				aWriter->WriteUInt(m_iconSpriteId);
				aWriter->WriteObjectPointers(m_directEffects);
				aWriter->WritePOD(m_flags);
				aWriter->WriteUInt(m_projectileParticleSystemId);
				aWriter->WriteUInt(m_sourceParticleSystemId);
				aWriter->WriteUInt(m_targetParticleSystemId);
				aWriter->WriteInt(m_castTime);
				aWriter->WriteUInt(m_aoeRadius);
				aWriter->WriteUInt(m_aoeCap);
				aWriter->WriteObjectPointers(m_aoeEntitySpawns);
				aWriter->WriteUInts(m_entityStates);
				aWriter->WriteOptionalObjectPointer(m_consumeItems);
				aWriter->WriteOptionalObjectPointer(m_produceItems);
				aWriter->WriteOptionalObject(m_requiredProfession);
				aWriter->WriteUInt(m_level);
				aWriter->WriteUInt(m_channelTicks);
				aWriter->WriteInt(m_channelInterval);
				aWriter->WriteUInt(m_channelTickAbilityId);
				aWriter->WriteObjects(m_requirements);
				aWriter->WriteObjects(m_aoeRequirements);
				m_soundEffects.ToStream(aWriter);
				aWriter->WriteOptionalObject(m_npcLevelRange);
				aWriter->WriteUInt(m_requiredLevel);

				for(uint32_t i = 1; i < (uint32_t)Resource::NUM_IDS; i++)
					aWriter->WriteUInt(m_resourceCosts[i]);
			}
			
			bool	
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadString(m_displayName))
					return false;
				if (!aReader->ReadString(m_description))
					return false;
				if (!aReader->ReadUInt(m_talentTreeId))
					return false;
				if (!aReader->ReadUInt(m_range))
					return false;
				if (!aReader->ReadInt(m_speed))
					return false;
				if (!aReader->ReadInt(m_delay))
					return false;
				if (!aReader->ReadUInts(m_cooldowns))
					return false;
				if (!aReader->ReadUInt(m_iconSpriteId))
					return false;
				if(!aReader->ReadObjectPointers(m_directEffects))
					return false;
				if(!aReader->ReadPOD(m_flags))
					return false;
				if (!aReader->ReadUInt(m_projectileParticleSystemId))
					return false;
				if (!aReader->ReadUInt(m_sourceParticleSystemId))
					return false;
				if (!aReader->ReadUInt(m_targetParticleSystemId))
					return false;
				if (!aReader->ReadInt(m_castTime))
					return false;
				if (!aReader->ReadUInt(m_aoeRadius))
					return false;
				if (!aReader->ReadUInt(m_aoeCap))
					return false;
				if (!aReader->ReadObjectPointers(m_aoeEntitySpawns))
					return false;
				if(!aReader->ReadUInts(m_entityStates))
					return false;
				if (!aReader->ReadOptionalObjectPointer(m_consumeItems))
					return false;
				if (!aReader->ReadOptionalObjectPointer(m_produceItems))
					return false;
				if(!aReader->ReadOptionalObject(m_requiredProfession))
					return false;
				if (!aReader->ReadUInt(m_level))
					return false;
				if (!aReader->ReadUInt(m_channelTicks))
					return false;
				if (!aReader->ReadInt(m_channelInterval))
					return false;
				if (!aReader->ReadUInt(m_channelTickAbilityId))
					return false;
				if (!aReader->ReadObjects(m_requirements))
					return false;
				if (!aReader->ReadObjects(m_aoeRequirements))
					return false;
				if(!m_soundEffects.FromStream(aReader))
					return false;
				if(!aReader->ReadOptionalObject(m_npcLevelRange))
					return false;
				if (!aReader->ReadUInt(m_requiredLevel))
					return false;

				for (uint32_t i = 1; i < (uint32_t)Resource::NUM_IDS; i++)
				{
					if(!aReader->ReadUInt(m_resourceCosts[i]))
						return false;
				}

				return true;
			}

			// Public data
			std::string											m_displayName;
			std::string											m_description;
			uint32_t											m_range = 1;
			int32_t												m_speed = 0;
			int32_t												m_delay = 0;
			std::vector<uint32_t>								m_cooldowns;
			int32_t												m_castTime = 0;
			uint32_t											m_flags = 0;
			uint32_t											m_iconSpriteId = 0;
			uint32_t											m_projectileParticleSystemId = 0;
			uint32_t											m_sourceParticleSystemId = 0;
			uint32_t											m_targetParticleSystemId = 0;
			uint32_t											m_aoeRadius = 0;
			uint32_t											m_aoeCap = 0;
			std::vector<std::unique_ptr<DirectEffectEntry>>		m_directEffects;
			std::vector<std::unique_ptr<AOEEntitySpawnEntry>>	m_aoeEntitySpawns;
			std::vector<EntityState::Id>						m_entityStates;
			uint32_t											m_resourceCosts[Resource::NUM_IDS] = { 0 };
			uint32_t											m_talentTreeId = 0;
			std::unique_ptr<Items>								m_consumeItems;
			std::unique_ptr<Items>								m_produceItems;
			std::optional<RequiredProfession>					m_requiredProfession;
			uint32_t											m_level = 1;
			uint32_t											m_channelTicks = 0;
			int32_t												m_channelInterval = 10;
			uint32_t											m_channelTickAbilityId = 0;
			std::vector<Requirement>							m_requirements;
			std::vector<Requirement>							m_aoeRequirements;
			SoundEffect::Collection								m_soundEffects;
			std::optional<UIntRange>							m_npcLevelRange;
			uint32_t											m_requiredLevel = 0;
		};

	}

}