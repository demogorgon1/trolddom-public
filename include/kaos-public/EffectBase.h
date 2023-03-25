#pragma once

#include "Effect.h"
#include "Parser.h"

namespace kaos_public
{

	class EffectBase
	{
	public:
		EffectBase()
		{

		}

		virtual 
		~EffectBase()
		{

		}

		bool
		FromSourceBase(
			const Parser::Node*		aSource)
		{
			if(aSource->m_name == "flags")
			{
				aSource->GetArray()->ForEachChild([&](
					const Parser::Node* aArrayItem)
				{
					uint32_t flag = Effect::StringToFlag(aArrayItem->GetIdentifier());
					KP_VERIFY(flag != 0, aArrayItem->m_debugInfo, "'%s' is not a valid effect flag.", aArrayItem->m_name.c_str());
					m_flags |= flag;
				});
				return true;
			}

			return false;
		}

		// Virtual methods
		virtual void	FromSource(
							const Parser::Node*		/*aSource*/) { assert(false); }

		// Public data
		uint32_t		m_flags;		
	};

}