#pragma once

#include "AuraEffect.h"
#include "DirectEffect.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"
#include "Stat.h"

namespace tpublic
{

	class AuraEffectBase
	{
	public:
		AuraEffectBase(
			AuraEffect::Id								aId)
			: m_id(aId)
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
				m_updateInterval = aSource->GetInt32();
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
			aStream->WriteInt(m_updateInterval);
			aStream->WriteUInt(m_updateCount);
		}
		
		bool	
		FromStreamBase(
			IReader*									aStream) 
		{
			if (!aStream->ReadInt(m_updateInterval))
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

		bool
		Update(
			int32_t										aTick)
		{
			if(m_updateCount == 0)
				return true; // Effects that don't require updates will have this initialized to zero

			int32_t ticksSinceLastUpdate = aTick - m_lastUpdate;
			if(ticksSinceLastUpdate >= m_updateInterval)
			{
				if(!OnUpdate())
					return false;

				m_lastUpdate = aTick;
				m_updateCount--;

				if(m_updateCount == 0)
					return false;
			}

			return true;
		}

		// Virtual methods
		virtual void			FromSource(
									const Parser::Node*	/*aSource*/) { assert(false); }
		virtual void			ToStream(
									IWriter*			/*aStream*/) const { assert(false); }
		virtual bool			FromStream(
									IReader*			/*aStream*/) { assert(false); return true; }
		virtual bool			OnUpdate() { return false; }
		virtual AuraEffectBase*	Copy() const { assert(false); return NULL; }
		virtual int32_t			FilterDamageInput(
									DirectEffect::DamageType	/*aDamageType*/,
									int32_t						aDamage) const { return aDamage; }
		virtual bool			GetStatModifier(
									Stat::Id					/*aStat*/,
									uint32_t&					/*aOutNum*/,
									uint32_t&					/*aOutDenom*/) const { return false; }		

		// Public data
		int32_t			m_updateInterval = 0;
		uint32_t		m_updateCount = 0;

		int32_t			m_lastUpdate = 0;
		AuraEffect::Id	m_id = AuraEffect::INVALID_ID;
	};

}