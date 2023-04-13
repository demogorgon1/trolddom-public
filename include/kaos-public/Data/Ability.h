#pragma once

#include "../DataBase.h"
#include "../EffectBase.h"
#include "../EffectFactory.h"

namespace kaos_public
{

	namespace Data
	{

		struct Ability
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ABILITY; 

			enum Flag : uint8_t
			{
				FLAG_TARGET_SELF	= 0x01,
				FLAG_TARGET_OTHER	= 0x02
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
					else
						KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid ability flag.", identifier);
				});
				return flags;
			}

			struct EffectEntry
			{
				EffectEntry()
				{

				}

				EffectEntry(
					const Parser::Node*		aSource)
				{
					m_effectId = Effect::StringToId(aSource->m_name.c_str());
					KP_VERIFY(m_effectId != Effect::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid effect.", aSource->m_name.c_str());

					std::unique_ptr<EffectBase> effect(aSource->m_sourceContext->m_effectFactory->Create(m_effectId));
					assert(effect);

					if (!aSource->m_children.empty())
						effect->FromSource(aSource);

					m_effectBase = std::move(effect);
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_effectId);
					aStream->WriteObjectPointer(m_effectBase);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadUInt(m_effectId))
						return false;

					m_effectBase.reset(aStream->GetEffectFactory()->Create(m_effectId));
					if(!m_effectBase->FromStream(aStream))
						return false;

					return true;
				}

				// Public data
				uint32_t							m_effectId;
				std::unique_ptr<EffectBase>			m_effectBase;
			};

			void
			Verify() const
			{
				VerifyBase();

				KP_VERIFY(!m_displayName.empty(), m_debugInfo, "'%s' has no 'display_name'.", m_name.c_str());
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
					else if (aMember->m_name == "cooldown")
						m_cooldown = aMember->GetUInt32();
					else if (aMember->m_name == "flags")
						m_flags = GetFlags(aMember);
					else if(aMember->m_tag == "effect")
						m_effects.push_back(std::make_unique<EffectEntry>(aMember->GetObject()));
					else
						KP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
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
				aStream->WriteObjectPointers(m_effects);
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
				if(!aStream->ReadObjectPointers(m_effects))
					return false;
				if(!aStream->ReadPOD(m_flags))
					return false;
				return true;
			}

			// Public data
			std::string									m_displayName;
			uint32_t									m_range = 1;
			uint32_t									m_cooldown = 10;
			uint8_t										m_flags = 0;
			std::vector<std::unique_ptr<EffectEntry>>	m_effects;
		};


	}

}