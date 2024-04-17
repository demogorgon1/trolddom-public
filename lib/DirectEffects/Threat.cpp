#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/ThreatSource.h>

#include <tpublic/DirectEffects/Threat.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>

namespace tpublic::DirectEffects
{

	void
	Threat::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "apply_to")
			{
				if(strcmp(aChild->GetIdentifier(), "all") == 0)
					m_applyTo = APPLY_TO_ALL;
				else if (strcmp(aChild->GetIdentifier(), "target") == 0)
					m_applyTo = APPLY_TO_TARGET;
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid identifier.", aChild->GetIdentifier());
			}
			else if(aChild->m_name == "multiply")
			{
				m_multiply = aChild->GetFloat();
			}
			else if (aChild->m_name == "add")
			{
				m_add = aChild->GetInt32();
			}
			else
			{
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
			}
		});
	}

	void
	Threat::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);

		aStream->WritePOD(m_applyTo);
		aStream->WriteInt(m_add);
		aStream->WriteBool(m_multiply.has_value());
		if(m_multiply.has_value())
			aStream->WriteFloat(m_multiply.value());
	}

	bool
	Threat::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;

		if(!aStream->ReadPOD(m_applyTo))
			return false;

		if (!aStream->ReadInt(m_add))
			return false;

		{
			bool hasValue;
			if (!aStream->ReadBool(hasValue))
				return false;

			if (hasValue)
			{
				float value;
				if (!aStream->ReadFloat(value))
					return false;

				m_multiply = value;
			}
		}

		return true;
	}

	CombatEvent::Id
	Threat::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					/*aRandom*/,
		const Manifest*					/*aManifest*/,
		CombatEvent::Id					aId,
		uint32_t						/*aAbilityId*/,
		EntityInstance*					/*aSource*/,
		EntityInstance*					aTarget,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			/*aCombatResultQueue*/,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				/*aWorldView*/) 
	{				
		switch (m_applyTo)
		{
		case APPLY_TO_ALL:		
			{
				Components::ThreatSource* threatSource = aTarget->GetComponent<Components::ThreatSource>();

				if (threatSource != NULL)
				{
					for (std::unordered_map<uint32_t, int32_t>::const_iterator i = threatSource->m_targets.cbegin(); i != threatSource->m_targets.cend(); i++)
					{
						uint32_t entityInstanceId = i->first;

						aEventQueue->EventQueueThreat(aTarget->GetEntityInstanceId(), entityInstanceId, m_add, m_multiply);
					}
				}
			}
			break;

		case APPLY_TO_TARGET:	
			{
				const Components::CombatPublic* combatPublic = aTarget->GetComponent<Components::CombatPublic>();

				if(combatPublic->m_targetEntityInstanceId != 0)
				{
					aEventQueue->EventQueueThreat(aTarget->GetEntityInstanceId(), combatPublic->m_targetEntityInstanceId, m_add, m_multiply);
				}
			}
			break;

		default:
			assert(false);
		}

		return aId;
	}

}