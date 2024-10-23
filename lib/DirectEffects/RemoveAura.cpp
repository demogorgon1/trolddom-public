#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>

#include <tpublic/DirectEffects/RemoveAura.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>

namespace tpublic
{

	namespace DirectEffects
	{

		void
		RemoveAura::FromSource(
			const SourceNode*				aSource) 
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if (!FromSourceBase(aChild))
				{
					if (aChild->m_name == "aura_type")
						m_auraType = Data::Aura::SourceToType(aChild);
					else if(aChild->m_name == "aura_flags")
						m_auraFlags = Data::Aura::SourceToFlags(aChild);
					else if (aChild->m_name == "aura")
						m_auraId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_AURA, aChild->GetIdentifier());
					else if (aChild->m_name == "aura_group")
						m_auraGroupId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_AURA_GROUP, aChild->GetIdentifier());
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				}
			});
		}

		void
		RemoveAura::ToStream(
			IWriter*						aStream) const 
		{
			ToStreamBase(aStream);
			aStream->WritePOD(m_auraFlags);
			aStream->WritePOD(m_auraType);
			aStream->WriteUInt(m_auraId);
			aStream->WriteUInt(m_auraGroupId);
		}

		bool
		RemoveAura::FromStream(
			IReader*						aStream) 
		{
			if (!FromStreamBase(aStream))
				return false;
			if (!aStream->ReadPOD(m_auraFlags))
				return false;
			if (!aStream->ReadPOD(m_auraType))
				return false;
			if (!aStream->ReadUInt(m_auraId))
				return false;
			if (!aStream->ReadUInt(m_auraGroupId))
				return false;
			return true;
		}

		DirectEffectBase::Result
		RemoveAura::Resolve(
			int32_t							/*aTick*/,
			std::mt19937&					aRandom,
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
			Components::Auras* targetAuras = aTarget->GetComponent<Components::Auras>();
			
			if(m_auraId != 0)
			{
				// Removing a specific aura
				for (std::unique_ptr<Components::Auras::Entry>& entry : targetAuras->m_entries)
				{
					if(entry->m_auraId == m_auraId)
					{
						// Hack, see comment below
						entry->m_cancel = true;
					}
				}
			}
			else if(m_auraGroupId != 0)
			{
				// Removing all auras within aura group
				for (std::unique_ptr<Components::Auras::Entry>& entry : targetAuras->m_entries)
				{
					const Data::Aura* aura = aManifest->GetById<Data::Aura>(entry->m_auraId);

					if(aura->m_auraGroupId == m_auraGroupId)
					{
						// Also hack, see comment below
						entry->m_cancel = true;
					}
				}
			}
			else
			{
				// Remove an aura based on type and flags
				std::vector<Components::Auras::Entry*> candidates;

				for(std::unique_ptr<Components::Auras::Entry>& entry : targetAuras->m_entries)
				{
					const Data::Aura* aura = aManifest->GetById<Data::Aura>(entry->m_auraId);
					if(aura->m_type == m_auraType && (aura->m_flags & m_auraFlags) == m_auraFlags)
						candidates.push_back(entry.get());
				}

				if(candidates.size() > 0)
				{
					size_t i = Helpers::RandomInRange<size_t>(aRandom, 0, candidates.size() - 1);

					// This is a bit of a hack as we're not supposed to update entities directly here. 
					// We can, however, signal a bool safely - which is easier than having to post an update on an event queue.
					candidates[i]->m_cancel = true;
				}
			}

			return Result();
		}

	}

}