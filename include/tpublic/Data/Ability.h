#pragma once

#include "../DataBase.h"
#include "../DirectEffectFactory.h"
#include "../DirectEffectBase.h"

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
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadUInt(m_entityId))
						return false;
					if(!aStream->ReadUInt(m_probability))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_entityId = 0;
				uint32_t							m_probability = 0;
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
					else
						TP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteString(m_displayName);
				aStream->WriteUInt(m_range);
				aStream->WriteInt(m_speed);
				aStream->WriteInt(m_delay);
				aStream->WriteInt(m_cooldown);
				aStream->WriteUInt(m_iconSpriteId);
				aStream->WriteObjectPointers(m_directEffects);
				aStream->WritePOD(m_flags);
				aStream->WriteUInt(m_projectileParticleSystemId);
				aStream->WriteInt(m_castTime);
				aStream->WriteUInt(m_aoeRadius);
				aStream->WriteUInt(m_aoeCap);
				aStream->WriteObjectPointers(m_aoeEntitySpawns);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadString(m_displayName))
					return false;
				if (!aStream->ReadUInt(m_range))
					return false;
				if (!aStream->ReadInt(m_speed))
					return false;
				if (!aStream->ReadInt(m_delay))
					return false;
				if (!aStream->ReadInt(m_cooldown))
					return false;
				if (!aStream->ReadUInt(m_iconSpriteId))
					return false;
				if(!aStream->ReadObjectPointers(m_directEffects))
					return false;
				if(!aStream->ReadPOD(m_flags))
					return false;
				if (!aStream->ReadUInt(m_projectileParticleSystemId))
					return false;
				if (!aStream->ReadInt(m_castTime))
					return false;
				if (!aStream->ReadUInt(m_aoeRadius))
					return false;
				if (!aStream->ReadUInt(m_aoeCap))
					return false;
				if (!aStream->ReadObjectPointers(m_aoeEntitySpawns))
					return false;
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
		};


	}

}