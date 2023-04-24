#pragma once

#include "CombatEvent.h"
#include "DirectEffect.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace kpublic
{

	namespace Components
	{
		struct CombatPublic;
		struct CombatPrivate;
	}

	class EntityInstance;
	class IAuraEventQueue;
	class IResourceChangeQueue;
	class IThreatEventQueue;
	class Manifest;

	class DirectEffectBase
	{
	public:
		DirectEffectBase()
		{

		}

		virtual 
		~DirectEffectBase()
		{

		}

		bool
		FromSourceBase(
			const Parser::Node*							aSource)
		{
			if(aSource->m_name == "flags")
			{
				aSource->GetArray()->ForEachChild([&](
					const Parser::Node* aArrayItem)
				{
					uint32_t flag = DirectEffect::StringToFlag(aArrayItem->GetIdentifier());
					KP_VERIFY(flag != 0, aArrayItem->m_debugInfo, "'%s' is not a valid effect flag.", aArrayItem->m_name.c_str());
					m_flags |= flag;
				});
				return true;
			}

			return false;
		}

		void	
		ToStreamBase(
			IWriter*									aStream) const 
		{
			aStream->WriteUInt(m_flags);
		}
		
		bool	
		FromStreamBase(
			IReader*									aStream) 
		{
			if(!aStream->ReadUInt(m_flags))
				return false;
			return true;
		}

		// Virtual methods
		virtual void	FromSource(
							const Parser::Node*			/*aSource*/) { assert(false); }
		virtual void	ToStream(
							IWriter*					/*aStream*/) const { assert(false); }
		virtual bool	FromStream(
							IReader*					/*aStream*/) { assert(false); return true; }
		virtual void	Resolve(
							std::mt19937&				/*aRandom*/,
							const Manifest*				/*aManifest*/,
							CombatEvent::Id				/*aId*/,
							uint32_t					/*aAbilityId*/,
							const EntityInstance*		/*aSource*/,
							EntityInstance*				/*aTarget*/,
							IResourceChangeQueue*		/*aCombatResultQueue*/,
							IAuraEventQueue*			/*aAuraEventQueue*/,
							IThreatEventQueue*			/*aThreatEventQueue*/) { }

		// Public data
		uint32_t		m_flags = 0;		
	};

}