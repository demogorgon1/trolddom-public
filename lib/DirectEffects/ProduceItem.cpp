#include "../Pcheader.h"

#include <tpublic/DirectEffects/ProduceItem.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IResourceChangeQueue.h>

namespace tpublic::DirectEffects
{

	void
	ProduceItem::FromSource(
		const SourceNode*			aSource) 
	{	
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "item")
				m_itemId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ITEM, aChild->GetIdentifier());
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
		});
	}

	void
	ProduceItem::ToStream(
		IWriter*					aStream) const 
	{
		ToStreamBase(aStream);

		aStream->WriteUInt(m_itemId);
	}

	bool
	ProduceItem::FromStream(
		IReader*					aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;

		if(!aStream->ReadUInt(m_itemId))
			return false;
		return true;
	}

	void
	ProduceItem::Resolve(
		int32_t						/*aTick*/,
		std::mt19937&				/*aRandom*/,
		const Manifest*				/*aManifest*/,
		CombatEvent::Id				/*aId*/,
		uint32_t					/*aAbilityId*/,
		const EntityInstance*		/*aSource*/,
		EntityInstance*				/*aTarget*/,
		IResourceChangeQueue*		/*aCombatResultQueue*/,
		IAuraEventQueue*			/*aAuraEventQueue*/,
		IThreatEventQueue*			/*aThreatEventQueue*/) 
	{
		printf("produce item: %u\n", m_itemId);
	}

}