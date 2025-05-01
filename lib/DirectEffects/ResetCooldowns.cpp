#include "../Pcheader.h"

#include <tpublic/Components/PlayerCooldowns.h>

#include <tpublic/Data/Cooldown.h>

#include <tpublic/DirectEffects/ResetCooldowns.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic::DirectEffects
{

	void
	ResetCooldowns::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if (aChild->m_name == "talent_trees")
					aChild->GetIdArray(DataType::ID_TALENT_TREE, m_talentTreeIds);
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
			}
		});
	}

	void
	ResetCooldowns::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);
		aStream->WriteUInts(m_talentTreeIds);
	}

	bool
	ResetCooldowns::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if (!aStream->ReadUInts(m_talentTreeIds))
			return false;
		return true;
	}

	DirectEffectBase::Result
	ResetCooldowns::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					/*aRandom*/,
		const Manifest*					aManifest,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		const SourceEntityInstance&		/*aSourceEntityInstance*/,
		EntityInstance*					/*aSource*/,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			/*aCombatResultQueue*/,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					/*aEventQueue*/,
		const IWorldView*				/*aWorldView*/) 
	{				
		if(aTarget->IsPlayer())
		{
			Components::PlayerCooldowns* playerCooldowns = aTarget->GetComponent<Components::PlayerCooldowns>();

			for(size_t i = 0; i < playerCooldowns->m_cooldowns.m_entries.size(); i++)
			{
				Cooldowns::Entry& entry = playerCooldowns->m_cooldowns.m_entries[i];
				const Data::Cooldown* cooldown = aManifest->GetById<Data::Cooldown>(entry.m_cooldownId);
				if(cooldown->m_talentTreeId != 0)
				{
					bool match = false;
					for (uint32_t talentTreeId : m_talentTreeIds)
					{
						if (talentTreeId == cooldown->m_talentTreeId)
						{
							match = true;
							break;
						}
					}

					if (match)
					{
						playerCooldowns->m_cooldowns.m_entries.erase(playerCooldowns->m_cooldowns.m_entries.begin() + i);
						i--;
						
						playerCooldowns->SetPendingPersistenceUpdate(ComponentBase::PENDING_PERSISTENCE_UPDATE_MEDIUM_PRIORITY);
					}
				}
			}
		}

		return Result();
	}

}