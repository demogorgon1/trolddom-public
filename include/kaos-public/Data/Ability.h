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

			struct EffectEntry
			{
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
					else if(aMember->m_name == "range")
						m_range = aMember->GetUInt32();
					else if(aMember->m_tag == "effect")
						m_effects.push_back(std::make_unique<EffectEntry>(aMember->GetObject()));
					else
						KP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
				});
			}

			// Public data
			std::string									m_displayName;
			uint32_t									m_range = 1;
			std::vector<std::unique_ptr<EffectEntry>>	m_effects;
		};


	}

}