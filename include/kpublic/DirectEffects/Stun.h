#pragma once

#include "../DirectEffectBase.h"

namespace kpublic
{

	namespace Effects
	{

		struct Stun
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_STUN;

			Stun()
			{

			}

			virtual 
			~Stun()
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
						if (aChild->m_name == "duration")
							m_duration = aChild->GetUInt32();
						else
							KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteUInt(m_duration);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				if (!aStream->ReadUInt(m_duration))
					return false;
				return true;
			}

			// Public data
			uint32_t		m_duration = 1;
		};

	}

}