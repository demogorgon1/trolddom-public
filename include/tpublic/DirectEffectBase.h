#pragma once

#include "CombatEvent.h"
#include "DirectEffect.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"
#include "Requirement.h"

namespace tpublic
{

	namespace Components
	{
		struct CombatPublic;
		struct CombatPrivate;
	}

	class EntityInstance;
	class IAuraEventQueue;
	class IEventQueue;
	class IResourceChangeQueue;
	class IWorldView;
	class Manifest;

	struct ItemInstanceReference;

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
			const SourceNode*										aSource)
		{
			if(aSource->m_name == "flags")
			{
				aSource->GetArray()->ForEachChild([&](
					const SourceNode* aArrayItem)
				{
					uint32_t flag = DirectEffect::StringToFlag(aArrayItem->GetIdentifier());
					TP_VERIFY(flag != 0, aArrayItem->m_debugInfo, "'%s' is not a valid effect flag.", aArrayItem->m_name.c_str());
					m_flags |= flag;
				});
				return true;
			}
			else if(aSource->m_tag == "requirement")
			{
				m_requirements.push_back(Requirement(aSource));
				return true;
			}
			else if (aSource->m_name == "must_have_ability_modifier")
			{
				m_mustHaveAbilityModifierId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY_MODIFIER, aSource->GetIdentifier());
				return true;
			}
			else if (aSource->m_name == "must_not_have_ability_modifier")
			{
				m_mustNotHaveAbilityModifierId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY_MODIFIER, aSource->GetIdentifier());
				return true;
			}

			return false;
		}

		void	
		ToStreamBase(	
			IWriter*												aStream) const 
		{
			aStream->WriteUInt(m_flags);
			aStream->WriteUInt(m_mustHaveAbilityModifierId);
			aStream->WriteUInt(m_mustNotHaveAbilityModifierId);
			aStream->WriteObjects(m_requirements);
		}
		
		bool	
		FromStreamBase(
			IReader*												aStream) 
		{
			if (!aStream->ReadUInt(m_flags))
				return false;
			if (!aStream->ReadUInt(m_mustHaveAbilityModifierId))
				return false;
			if (!aStream->ReadUInt(m_mustNotHaveAbilityModifierId))
				return false;
			if(!aStream->ReadObjects(m_requirements))
				return false;
			return true;
		}

		// Virtual methods
		virtual void			FromSource(
									const SourceNode*				/*aSource*/) { assert(false); }
		virtual void			ToStream(
									IWriter*						/*aStream*/) const { assert(false); }
		virtual bool			FromStream(
									IReader*						/*aStream*/) { assert(false); return true; }
		virtual CombatEvent::Id	Resolve(
									int32_t							/*aTick*/,
									std::mt19937&					/*aRandom*/,
									const Manifest*					/*aManifest*/,
									CombatEvent::Id					aId,
									uint32_t						/*aAbilityId*/,
									EntityInstance*					/*aSource*/,
									EntityInstance*					/*aTarget*/,
									const ItemInstanceReference&	/*aItem*/,
									IResourceChangeQueue*			/*aCombatResultQueue*/,
									IAuraEventQueue*				/*aAuraEventQueue*/,
									IEventQueue*					/*aEventQueue*/,
									const IWorldView*				/*aWorldView*/) { return aId; }

		// Public data
		uint32_t					m_flags = 0;		
		std::vector<Requirement>	m_requirements;
		uint32_t					m_mustHaveAbilityModifierId = 0;
		uint32_t					m_mustNotHaveAbilityModifierId = 0;
	};

}