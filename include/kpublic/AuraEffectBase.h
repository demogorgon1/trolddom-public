#pragma once

#include "AuraEffect.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace kpublic
{

	class AuraEffectBase
	{
	public:
		AuraEffectBase()
		{

		}

		virtual 
		~AuraEffectBase()
		{

		}

		bool
		FromSourceBase(
			const Parser::Node*							aSource)
		{
			if(aSource->m_name == "update_interval")
			{
				m_updateInterval = aSource->GetUInt32();
				return true;
			}
			else if (aSource->m_name == "update_count")
			{
				m_updateCount = aSource->GetUInt32();
				return true;
			}
			return false;
		}

		void	
		ToStreamBase(
			IWriter*									aStream) const 
		{
			aStream->WriteUInt(m_updateInterval);
			aStream->WriteUInt(m_updateCount);
		}
		
		bool	
		FromStreamBase(
			IReader*									aStream) 
		{
			if (!aStream->ReadUInt(m_updateInterval))
				return false;
			if (!aStream->ReadUInt(m_updateCount))
				return false;
			return true;
		}

		void
		CopyBase(
			const AuraEffectBase*						aOther)
		{
			m_updateInterval = aOther->m_updateInterval;
			m_updateCount = aOther->m_updateCount;
		}

		// Virtual methods
		virtual void			FromSource(
									const Parser::Node*	/*aSource*/) { assert(false); }
		virtual void			ToStream(
									IWriter*			/*aStream*/) const { assert(false); }
		virtual bool			FromStream(
									IReader*			/*aStream*/) { assert(false); return true; }
		virtual void			Update() { }
		virtual AuraEffectBase*	Copy() const { assert(false); return NULL; }

		// Public data
		uint32_t		m_updateInterval = 0;
		uint32_t		m_updateCount = 0;
	};

}