#pragma once

#include "../DataBase.h"
#include "../DirectEffectFactory.h"
#include "../DirectEffectBase.h"
#include "../EntityState.h"
#include "../Resource.h"

namespace tpublic
{

	namespace Data
	{

		struct Ability
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ABILITY; 

			enum Flag : uint16_t
			{
				FLAG_TARGET_SELF			= 0x0001,
				FLAG_TARGET_OTHER			= 0x0002,
				FLAG_TARGET_AOE				= 0x0004,
				FLAG_TARGET_HOSTILE			= 0x0008,
				FLAG_TARGET_FRIENDLY		= 0x0010,
				FLAG_CAN_MISS				= 0x0020,
				FLAG_CAN_BE_DODGED			= 0x0040,
				FLAG_CAN_BE_PARRIED			= 0x0080,
				FLAG_CAN_BE_BLOCKED			= 0x0100,
				FLAG_ATTACK					= 0x0200,
				FLAG_USE_WEAPON_ICON		= 0x0400,				
				FLAG_AOE_LOW_HEALTH_ONLY	= 0x0800,
				FLAG_AOE_LOW_HEALTH_PRIO	= 0x1000
			};

			static inline Resource::Id
			GetResourceId(
				const Parser::Node*			aSource)
			{
				Resource::Id resourceId = Resource::StringToId(aSource->m_name.c_str());
				TP_VERIFY(resourceId != Resource::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid resource id.", aSource->m_name.c_str());
				return resourceId;
			}

			static inline uint16_t
			GetFlags(
				const Parser::Node*			aSource)
			{
				uint16_t flags = 0;
				aSource->GetArray()->ForEachChild([&](
					const Parser::Node*		aChild)
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
					const Parser::Node*		aSource)
				{
					m_directEffectId = DirectEffect::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_directEffectId != DirectEffect::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid direct effect.", aSource->m_name.c_str());

					std::unique_ptr<DirectEffectBase> effect(aSource->m_sourceContext->m_directEffectFactory->Create(m_directEffectId));
					assert(effect);

					if (!aSource->m_children.empty())
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
					const Parser::Node*		aSource)
				{
					m_entityId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ENTITY, aSource->m_name.c_str());
					
					aSource->ForEachChild([&](
						const Parser::Node* aChild)
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
			bool CanMiss() const { return m_flags & FLAG_CAN_MISS; }
			bool CanBeDodged() const { return m_flags & FLAG_CAN_BE_DODGED; }
			bool CanBeParried() const { return m_flags & FLAG_CAN_BE_PARRIED; }
			bool CanBeBlocked() const { return m_flags & FLAG_CAN_BE_BLOCKED; }
			bool IsAttack() const { return m_flags & FLAG_ATTACK; }
			bool IsInstantMelee() const { return m_range == 1 && m_castTime == 0; }
			
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
				const Parser::Node*		aNode) override
			{
				aNode->ForEachChild([&](
					const Parser::Node* aMember)
				{
					if(aMember->m_name == "string")
						m_displayName = aMember->GetString();
					else if (aMember->m_name == "range")
						m_range = aMember->GetUInt32();
					else if (aMember->m_name == "aoe_radius")
						m_aoeRadius = aMember->GetUInt32();
					else if (aMember->m_name == "aoe_cap")
						m_aoeCap = aMember->GetUInt32();
					else if (aMember->m_name == "speed")
						m_speed = aMember->GetInt32();
					else if (aMember->m_name == "delay")
						m_delay = aMember->GetInt32();
					else if (aMember->m_name == "cooldown")
						m_cooldown = aMember->GetInt32();
					else if (aMember->m_name == "cast_time")
						m_castTime = aMember->GetInt32();
					else if (aMember->m_name == "icon")
						m_iconSpriteId = aMember->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aMember->GetIdentifier());
					else if (aMember->m_name == "projectile")
						m_projectileParticleSystemId = aMember->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_PARTICLE_SYSTEM, aMember->GetIdentifier());
					else if (aMember->m_name == "flags")
						m_flags = GetFlags(aMember);
					else if(aMember->m_tag == "direct_effect")
						m_directEffects.push_back(std::make_unique<DirectEffectEntry>(aMember->GetObject()));
					else if (aMember->m_tag == "aoe_entity_spawn")
						m_aoeEntitySpawns.push_back(std::make_unique<AOEEntitySpawnEntry>(aMember->GetObject()));
					else if (aMember->m_name == "states")
						aMember->GetIdArrayWithLookup<EntityState::Id, EntityState::INVALID_ID>(m_entityStates, [&](const char* aIdentifier) { return EntityState::StringToId(aIdentifier); });
					else if (aMember->m_tag == "resource_cost")
						m_resourceCosts[GetResourceId(aMember)] = aMember->GetUInt32();
					else
						TP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				ToStreamBase(aWriter);
				aWriter->WriteString(m_displayName);
				aWriter->WriteUInt(m_range);
				aWriter->WriteInt(m_speed);
				aWriter->WriteInt(m_delay);
				aWriter->WriteInt(m_cooldown);
				aWriter->WriteUInt(m_iconSpriteId);
				aWriter->WriteObjectPointers(m_directEffects);
				aWriter->WritePOD(m_flags);
				aWriter->WriteUInt(m_projectileParticleSystemId);
				aWriter->WriteInt(m_castTime);
				aWriter->WriteUInt(m_aoeRadius);
				aWriter->WriteUInt(m_aoeCap);
				aWriter->WriteObjectPointers(m_aoeEntitySpawns);
				aWriter->WriteUInts(m_entityStates);
				
				for(uint32_t i = 1; i < (uint32_t)Resource::NUM_IDS; i++)
					aWriter->WriteUInt(m_resourceCosts[i]);
			}
			
			bool	
			FromStream(
				IReader*				aReader) override
			{
				if(!FromStreamBase(aReader))
					return false;
				if(!aReader->ReadString(m_displayName))
					return false;
				if (!aReader->ReadUInt(m_range))
					return false;
				if (!aReader->ReadInt(m_speed))
					return false;
				if (!aReader->ReadInt(m_delay))
					return false;
				if (!aReader->ReadInt(m_cooldown))
					return false;
				if (!aReader->ReadUInt(m_iconSpriteId))
					return false;
				if(!aReader->ReadObjectPointers(m_directEffects))
					return false;
				if(!aReader->ReadPOD(m_flags))
					return false;
				if (!aReader->ReadUInt(m_projectileParticleSystemId))
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

				for (uint32_t i = 1; i < (uint32_t)Resource::NUM_IDS; i++)
				{
					if(!aReader->ReadUInt(m_resourceCosts[i]))
						return false;
				}

				return true;
			}

			// Public data
			std::string											m_displayName;
			uint32_t											m_range = 1;
			int32_t												m_speed = 0;
			int32_t												m_delay = 0;
			int32_t												m_cooldown = 10;
			int32_t												m_castTime = 0;
			uint16_t											m_flags = 0;
			uint32_t											m_iconSpriteId = 0;
			uint32_t											m_projectileParticleSystemId = 0;
			uint32_t											m_aoeRadius = 0;
			uint32_t											m_aoeCap = 0;
			std::vector<std::unique_ptr<DirectEffectEntry>>		m_directEffects;
			std::vector<std::unique_ptr<AOEEntitySpawnEntry>>	m_aoeEntitySpawns;
			std::vector<EntityState::Id>						m_entityStates;
			uint32_t											m_resourceCosts[Resource::NUM_IDS] = { 0 };
		};

	}

}