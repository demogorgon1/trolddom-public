#pragma once

#include "../EffectBase.h"

namespace kaos_public
{

	namespace Effects
	{

		struct Damage
			: public EffectBase
		{
			static const Effect::Id ID = Effect::ID_DAMAGE;

			Damage()
			{

			}

			virtual 
			~Damage()
			{

			}

			// EffectBase implementation
			void
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if(aChild->m_name == "damage_type")
							m_damageType = Effect::StringToDamageType(aChild->GetIdentifier());
						else
							KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
					}
				});
			}

			// Public data
			Effect::DamageType	m_damageType = Effect::DAMAGE_TYPE_PHYSICAL;
		};

	}

}