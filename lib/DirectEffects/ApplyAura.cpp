#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/PlayerPrivate.h>
#include <tpublic/Components/Position.h>

#include <tpublic/Data/Aura.h>

#include <tpublic/DirectEffects/ApplyAura.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>
#include <tpublic/IAuraEventQueue.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace DirectEffects
	{

		void
		ApplyAura::FromSource(
			const SourceNode*				aSource) 
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if (!FromSourceBase(aChild))
				{
					if (aChild->m_name == "aura")
						m_auraId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_AURA, aChild->GetIdentifier());
					else if (aChild->m_name == "threat")
						m_threat = aChild->GetInt32();
					else if(aChild->m_name == "apply_to_party_members_in_range")
						m_applyToPartyMembersInRange = aChild->GetUInt32();
					else if (aChild->m_name == "target_self")
						m_targetSelf = aChild->GetBool();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				}
			});
		}

		void
		ApplyAura::ToStream(
			IWriter*						aStream) const 
		{
			ToStreamBase(aStream);
			aStream->WriteUInt(m_auraId);
			aStream->WriteInt(m_threat);
			aStream->WriteUInt(m_applyToPartyMembersInRange);
			aStream->WriteBool(m_targetSelf);
		}

		bool
		ApplyAura::FromStream(
			IReader*						aStream) 
		{
			if (!FromStreamBase(aStream))
				return false;
			if (!aStream->ReadUInt(m_auraId))
				return false;
			if (!aStream->ReadInt(m_threat))
				return false;
			if (!aStream->ReadUInt(m_applyToPartyMembersInRange))
				return false;
			if(!aStream->ReadBool(m_targetSelf))
				return false;
			return true;
		}

		DirectEffectBase::Result
		ApplyAura::Resolve(
			int32_t							aTick,
			std::mt19937&					/*aRandom*/,
			const Manifest*					aManifest,
			CombatEvent::Id					/*aId*/,
			uint32_t						aAbilityId,
			const SourceEntityInstance&		aSourceEntityInstance,
			EntityInstance*					aSource,
			EntityInstance*					aTarget,
			const Vec2&						/*aAOETarget*/,
			const ItemInstanceReference&	/*aItem*/,
			IResourceChangeQueue*			/*aCombatResultQueue*/,
			IAuraEventQueue*				aAuraEventQueue,
			IEventQueue*					aEventQueue,
			const IWorldView*				aWorldView)
		{
			EntityInstance* target = m_targetSelf ? aSource : aTarget;

			std::vector<const EntityInstance*> targetEntities = { target };
			const Data::Aura* aura = aManifest->GetById<tpublic::Data::Aura>(m_auraId);

			if(m_applyToPartyMembersInRange != 0)
			{
				const Components::CombatPublic* combatPublic = target->GetComponent<Components::CombatPublic>();
				if(combatPublic->m_combatGroupId != 0)
				{
					const Components::Position* targetPosition = target->GetComponent<Components::Position>();

					aWorldView->WorldViewGroupEntityInstances(combatPublic->m_combatGroupId, [&](
						const EntityInstance* aEntityInstance) -> bool
					{
						if(aEntityInstance != target)
						{
							const Components::Position* position = aEntityInstance->GetComponent<Components::Position>();
							if(Helpers::IsWithinDistance(position, targetPosition, (int32_t)m_applyToPartyMembersInRange))
								targetEntities.push_back(aEntityInstance);
						}
						return false;
					});
				}
			}

			const Components::PlayerPrivate* playerPrivate = aSource->GetComponent<Components::PlayerPrivate>();
			const AbilityModifierList* abilityModifierList = playerPrivate != NULL ? playerPrivate->m_abilityModifierList : NULL;
			int32_t modifyAuraUpdateCount = 0;
			if (abilityModifierList != NULL)
				modifyAuraUpdateCount = abilityModifierList->GetAbilityModifyAuraUpdateCount(aAbilityId);

			for(const EntityInstance* targetEntity : targetEntities)
			{
				if (m_threat != 0 && targetEntity->GetEntityId() != 0)
					aEventQueue->EventQueueThreat(aSourceEntityInstance, targetEntity->GetEntityInstanceId(), m_threat, aTick);

				std::vector<std::unique_ptr<AuraEffectBase>> effects;
				for (const std::unique_ptr<Data::Aura::AuraEffectEntry>& t : aura->m_auraEffects)
				{
					std::unique_ptr<AuraEffectBase> effect(t->m_auraEffectBase->Copy());

					if (effect->m_updateCount != 0 && modifyAuraUpdateCount != 0)
						effect->m_updateCount = (uint32_t)((int32_t)effect->m_updateCount + modifyAuraUpdateCount);

					effects.push_back(std::move(effect));
				}

				aAuraEventQueue->ApplyAura(aAbilityId, m_auraId, aSourceEntityInstance, targetEntity->GetEntityInstanceId(), effects);
			}

			return Result();
		}

	}

}