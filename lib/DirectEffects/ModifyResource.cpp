#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>

#include <tpublic/DirectEffects/ModifyResource.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IResourceChangeQueue.h>
#include <tpublic/Resource.h>

namespace tpublic::DirectEffects
{

	void
	ModifyResource::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode*	aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if(aChild->m_name == "range")
				{
					m_range = IntRange(aChild);
				}
				else if(aChild->m_name == "resource")
				{
					m_resourceId = Resource::StringToId(aChild->GetIdentifier());
					TP_VERIFY(m_resourceId != Resource::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid resource.", aChild->GetIdentifier());
				}
				else if(aChild->m_name == "target_self")
				{
					m_targetSelf = aChild->GetBool();
				}
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				}
			}
		});
	}

	void	
	ModifyResource::ToStream(
		IWriter*						aStream) const 
	{
		ToStreamBase(aStream);
		aStream->WritePOD(m_resourceId);
		m_range.ToStream(aStream);
		aStream->WriteBool(m_targetSelf);
	}
			
	bool	
	ModifyResource::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if (!aStream->ReadPOD(m_resourceId))
			return false;
		if(!m_range.FromStream(aStream))
			return false;
		if(!aStream->ReadBool(m_targetSelf))
			return false;
		return true;
	}

	void
	ModifyResource::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					aRandom,
		const Manifest*					/*aManifest*/,
		CombatEvent::Id					aId,
		uint32_t						aAbilityId,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			aResourceChangeQueue,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					/*aEventQueue*/,
		const IWorldView*				/*aWorldView*/) 
	{
		EntityInstance* target = m_targetSelf ? aSource : aTarget;
		Components::CombatPublic* combatPublic = target->GetComponent<Components::CombatPublic>();

		size_t resourceIndex;
		if(combatPublic->GetResourceIndex(m_resourceId, resourceIndex))
		{
			aResourceChangeQueue->AddResourceChange(
				aId,
				aAbilityId,
				aSource->GetEntityInstanceId(),
				target->GetEntityInstanceId(),
				combatPublic,
				resourceIndex,
				m_range.GetRandom(aRandom),
				0);
		}
	}

}