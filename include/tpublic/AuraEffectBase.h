#pragma once

#include "AuraEffect.h"
#include "CombatEvent.h"
#include "DirectEffect.h"
#include "IReader.h"
#include "IWriter.h"
#include "MoveSpeed.h"
#include "Parser.h"
#include "Stat.h"
#include "SystemBase.h"

namespace tpublic
{

	class SecondaryAbility;

	class AuraEffectBase
	{
	public:
		typedef std::function<void(const SecondaryAbility&)> SecondaryAbilityCallback;
		
		enum CombatEventType : uint8_t
		{
			INVALID_COMBAT_EVENT_TYPE,

			COMBAT_EVENT_TYPE_SOURCE,
			COMBAT_EVENT_TYPE_TARGET
		};

		static CombatEventType
		SourceToCombatEventType(
			const SourceNode*							aSource)
		{
			std::string_view t(aSource->GetIdentifier());
			if(t == "source")
				return COMBAT_EVENT_TYPE_SOURCE;
			else if (t == "target")
				return COMBAT_EVENT_TYPE_TARGET;
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid combat event type.", aSource->GetIdentifier());
			return INVALID_COMBAT_EVENT_TYPE;
		}

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
									const SourceNode*				/*aSource*/) { assert(false); }
		virtual void			ToStream(
									IWriter*						/*aStream*/) const { assert(false); }
		virtual bool			FromStream(
									IReader*						/*aStream*/) { assert(false); return true; }
		virtual bool			OnApplication(
									uint32_t						/*aSourceEntityInstanceId*/,
									uint32_t						/*aTargetEntityInstanceId*/,
									SystemBase::Context*			/*aContext*/,
									const Manifest*					/*aManifest*/) { return true; }
		virtual bool			OnUpdate(
									uint32_t						/*aSourceEntityInstanceId*/,
									uint32_t						/*aTargetEntityInstanceId*/,
									SystemBase::Context*			/*aContext*/,
									const Manifest*					/*aManifest*/) { return false; }
		virtual AuraEffectBase*	Copy() const { assert(false); return NULL; }
		virtual int32_t			FilterDamageInput(
									DirectEffect::DamageType		/*aDamageType*/,
									int32_t							aDamage) const { return aDamage; }
		virtual int32_t			FilterDamageOutput(
									DirectEffect::DamageType		/*aDamageType*/,
									int32_t							aDamage) const { return aDamage; }
		virtual int32_t			FilterHealInput(
									int32_t							aHeal) const { return aHeal; }
		virtual int32_t			FilterHealOutput(
									int32_t							aHeal) const { return aHeal; }
		virtual int32_t			FilterThreat(
									int32_t							aThreat) const { return aThreat; }
		virtual MoveSpeed::Id	GetMoveSpeedModifier() const { return MoveSpeed::INVALID_ID; }
		virtual void			OnCombatEvent(
									CombatEventType					/*aType*/,
									CombatEvent::Id					/*aCombatEventId*/,
									SecondaryAbilityCallback		/*aCallback*/) const { }
		virtual bool			GetStatModifier(
									Stat::Id						/*aStat*/,
									uint32_t&						/*aOutNum*/,
									uint32_t&						/*aOutDenom*/) const { return false; }		

		// Public data
		int32_t			m_updateInterval = 0;
		uint32_t		m_updateCount = 0;

		// Internal
		int32_t			m_lastUpdate = 0;
		AuraEffect::Id	m_id = AuraEffect::INVALID_ID;
		bool			m_applied = false;
	};

}