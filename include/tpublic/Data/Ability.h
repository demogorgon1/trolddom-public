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

			enum Flag : uint8_t
			{
				FLAG_TARGET_SELF		= 0x01,
				FLAG_TARGET_OTHER		= 0x02,
				FLAG_CAN_MISS			= 0x04,
				FLAG_CAN_BE_DODGED		= 0x08,
				FLAG_CAN_BE_PARRIED		= 0x10,
				FLAG_CAN_BE_BLOCKED		= 0x20,
				FLAG_ATTACK				= 0x40,
				FLAG_USE_WEAPON_ICON	= 0x80
			};

			static inline uint8_t
			GetFlags(
				const Parser::Node*			aSource)
			{
				uint8_t flags = 0;
				aSource->GetArray()->ForEachChild([&](
					const Parser::Node*		aChild)
				{
					const char* identifier = aChild->GetIdentifier();
					if (strcmp(identifier, "target_self") == 0)
						flags |= FLAG_TARGET_SELF;
					else if (strcmp(identifier, "target_other") == 0)
						flags |= FLAG_TARGET_OTHER;
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
				uint32_t							m_directEffectId;
				std::unique_ptr<DirectEffectBase>	m_directEffectBase;
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
			bool CanMiss() const { return m_flags & FLAG_CAN_MISS; }
			bool CanBeDodged() const { return m_flags & FLAG_CAN_BE_DODGED; }
			bool CanBeParried() const { return m_flags & FLAG_CAN_BE_PARRIED; }
			bool CanBeBlocked() const { return m_flags & FLAG_CAN_BE_BLOCKED; }
			bool IsAttack() const { return m_flags & FLAG_ATTACK; }

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
					else if (aMember->m_name == "cooldown")
						m_cooldown = aMember->GetUInt32();
					else if (aMember->m_name == "icon")
						m_iconSpriteId = aMember->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aMember->GetIdentifier());
					else if (aMember->m_name == "flags")
						m_flags = GetFlags(aMember);
					else if(aMember->m_tag == "direct_effect")
						m_directEffects.push_back(std::make_unique<DirectEffectEntry>(aMember->GetObject()));
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
				aStream->WriteUInt(m_cooldown);
				aStream->WriteUInt(m_iconSpriteId);
				aStream->WriteObjectPointers(m_directEffects);
				aStream->WritePOD(m_flags);
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
				if (!aStream->ReadUInt(m_cooldown))
					return false;
				if (!aStream->ReadUInt(m_iconSpriteId))
					return false;
				if(!aStream->ReadObjectPointers(m_directEffects))
					return false;
				if(!aStream->ReadPOD(m_flags))
					return false;
				return true;
			}

			// Public data
			std::string										m_displayName;
			uint32_t										m_range = 1;
			uint32_t										m_cooldown = 10;
			uint8_t											m_flags = 0;
			uint32_t										m_iconSpriteId = 0;
			std::vector<std::unique_ptr<DirectEffectEntry>>	m_directEffects;
		};


	}

}