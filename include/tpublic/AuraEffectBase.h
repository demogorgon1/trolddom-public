#pragma once

#include "AuraEffect.h"
#include "DirectEffect.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"
#include "Stat.h"
#include "SystemBase.h"

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
			const SourceNode*							aSource)
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
			uint32_t									aSourceEntityInstanceId,
			uint32_t									aTargetEntityInstanceId,
			SystemBase::Context*						aContext,
			const Manifest*								aManifest)
		{
			if(!m_applied)
			{
				if (!OnApplication(aSourceEntityInstanceId, aTargetEntityInstanceId, aContext, aManifest))
					return false;

				m_applied = true;
			}

			if(m_updateCount == 0)
				return true; // Effects that don't require updates will have this initialized to zero

			if(aSourceEntityInstanceId == 0)
				return false;

			int32_t ticksSinceLastUpdate = aContext->m_tick - m_lastUpdate;
			if(ticksSinceLastUpdate >= m_updateInterval)
			{
				if(!OnUpdate(aSourceEntityInstanceId, aTargetEntityInstanceId, aContext, aManifest))
					return false;

				m_lastUpdate = aContext->m_tick;
				m_updateCount--;

				if(m_updateCount == 0)
					return false;
			}

			return true;
		}

		// Virtual methods
		virtual void			FromSource(
									const SourceNode*			/*aSource*/) { assert(false); }
		virtual void			ToStream(
									IWriter*					/*aStream*/) const { assert(false); }
		virtual bool			FromStream(
									IReader*					/*aStream*/) { assert(false); return true; }
		virtual bool			OnApplication(
									uint32_t					/*aSourceEntityInstanceId*/,
									uint32_t					/*aTargetEntityInstanceId*/,
									SystemBase::Context*		/*aContext*/,
									const Manifest*				/*aManifest*/) { return true; }
		virtual bool			OnUpdate(
									uint32_t					/*aSourceEntityInstanceId*/,
									uint32_t					/*aTargetEntityInstanceId*/,
									SystemBase::Context*		/*aContext*/,
									const Manifest*				/*aManifest*/) { return false; }
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

		// Internal
		int32_t			m_lastUpdate = 0;
		AuraEffect::Id	m_id = AuraEffect::INVALID_ID;
		bool			m_applied = false;
	};

}