#pragma once

#include "../AuraEffectBase.h"
#include "../AuraEffectFactory.h"
#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Aura
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_AURA;

			struct AuraEffectEntry
			{
				AuraEffectEntry()
				{

				}

				AuraEffectEntry(
					const Parser::Node*		aSource)
				{
					m_auraEffectId = AuraEffect::StringToId(aSource->m_name.c_str());
					KP_VERIFY(m_auraEffectId != AuraEffect::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid aura effect.", aSource->m_name.c_str());

					std::unique_ptr<AuraEffectBase> effect(aSource->m_sourceContext->m_auraEffectFactory->Create(m_auraEffectId));
					assert(effect);

					if (!aSource->m_children.empty())
						effect->FromSource(aSource);

					m_auraEffectBase = std::move(effect);
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_auraEffectId);
					aStream->WriteObjectPointer(m_auraEffectBase);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadUInt(m_auraEffectId))
						return false;

					m_auraEffectBase.reset(aStream->GetAuraEffectFactory()->Create(m_auraEffectId));
					if(!m_auraEffectBase->FromStream(aStream))
						return false;

					return true;
				}

				// Public data
				uint32_t							m_auraEffectId;
				std::unique_ptr<AuraEffectBase>		m_auraEffectBase;
			};

			enum Type : uint8_t
			{	
				TYPE_HIDDEN,
				TYPE_DEBUFF,
				TYPE_BUFF
			};

			enum Flag : uint8_t
			{
				FLAG_UNIQUE = 0x01
			};

			static Type
			SourceToType(
				const Parser::Node*		aSource)
			{
				const char* string = aSource->GetIdentifier();
				if (strcmp(string, "hidden") == 0)
					return TYPE_HIDDEN;
				if (strcmp(string, "debuff") == 0)
					return TYPE_DEBUFF;
				if (strcmp(string, "buff") == 0)
					return TYPE_BUFF;
				KP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid aura type.", string);
				return Type(0);
			}

			static uint8_t
			SourceToFlags(
				const Parser::Node*		aSource)
			{
				uint8_t flags = 0;
				aSource->GetArray()->ForEachChild([&](
					const Parser::Node*	aFlag)
				{
					const char* string = aFlag->GetIdentifier();
					if (strcmp(string, "unique") == 0)
						flags |= FLAG_UNIQUE;
					KP_VERIFY(false, aFlag->m_debugInfo, "'%s' is not a valid aura flag.", string);
				});
				return flags;
			}

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		aNode) override
			{
				aNode->ForEachChild([&](
					const Parser::Node* aChild)
				{
					if(aChild->m_name == "icon")
						m_iconSpriteId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->GetIdentifier());
					else if (aChild->m_name == "duration")
						m_duration = aChild->GetUInt32();
					else if (aChild->m_name == "type")
						m_type = SourceToType(aChild);
					else if (aChild->m_name == "flags")
						m_flags |= SourceToFlags(aChild);
					else if (aChild->m_tag == "aura_effect")
						m_auraEffects.push_back(std::make_unique<AuraEffectEntry>(aChild));
					else if(aChild->m_name == "string")
						m_string = aChild->GetString();
					else
						KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteUInt(m_iconSpriteId);
				aStream->WriteUInt(m_duration);
				aStream->WritePOD(m_type);
				aStream->WritePOD(m_flags);
				aStream->WriteObjectPointers(m_auraEffects);
				aStream->WriteString(m_string);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if (!aStream->ReadUInt(m_iconSpriteId))
					return false;
				if (!aStream->ReadUInt(m_duration))
					return false;
				if(!aStream->ReadPOD(m_type))
					return false;
				if (!aStream->ReadPOD(m_flags))
					return false;
				if(!aStream->ReadObjectPointers(m_auraEffects))
					return false;
				if(!aStream->ReadString(m_string))
					return false;
				return true;
			}

			// Public data
			std::string										m_string;
			uint32_t										m_iconSpriteId = 0;
			uint32_t										m_duration = 0;
			Type											m_type = TYPE_HIDDEN;
			uint8_t											m_flags = 0;
			std::vector<std::unique_ptr<AuraEffectEntry>>	m_auraEffects;
		};

	}

}