#include "../Pcheader.h"

#include <tpublic/AuraEffects/Repeat.h>

#include <tpublic/IAbilityQueue.h>
#include <tpublic/Manifest.h>
#include <tpublic/Vec2.h>

namespace tpublic::AuraEffects
{

	bool
	Repeat::OnUpdate(
		uint32_t				aSourceEntityInstanceId,
		uint32_t				aTargetEntityInstanceId,
		SystemBase::Context*	aContext,
		const Manifest*			aManifest) 
	{
		const Data::Ability* ability = aManifest->GetById<tpublic::Data::Ability>(m_abilityId);
		aContext->m_abilityQueue->AddAbility(aSourceEntityInstanceId, aTargetEntityInstanceId, Vec2(), ability);
		return true;
	}

}