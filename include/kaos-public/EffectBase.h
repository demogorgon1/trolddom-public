#pragma once

#include "Effect.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace kaos_public
{

	class EntityInstance;

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

		void	
		ToStreamBase(
			IWriter*				aStream) const 
		{
			aStream->WriteUInt(m_flags);
		}
		
		bool	
		FromStreamBase(
			IReader*				aStream) 
		{
			if(!aStream->ReadUInt(m_flags))
				return false;
			return true;
		}

		// Virtual methods
		virtual void	FromSource(
							const Parser::Node*		/*aSource*/) { assert(false); }
		virtual void	ToStream(
							IWriter*				/*aStream*/) const { assert(false); }
		virtual bool	FromStream(
							IReader*				/*aStream*/) { assert(false); return true; }
		virtual void	Apply(
							EntityInstance*			/*aEntity*/) { }							

		// Public data
		uint32_t		m_flags;		
	};

}