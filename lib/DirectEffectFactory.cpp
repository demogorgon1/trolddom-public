#include "Pcheader.h"

#include <tpublic/DirectEffects/ApplyAura.h>
#include <tpublic/DirectEffects/ConsumeAndProduce.h>
#include <tpublic/DirectEffects/Damage.h>
#include <tpublic/DirectEffects/Fishing.h>
#include <tpublic/DirectEffects/Heal.h>
#include <tpublic/DirectEffects/Interrupt.h>
#include <tpublic/DirectEffects/LearnProfessionAbility.h>
#include <tpublic/DirectEffects/ModifyFaction.h>
#include <tpublic/DirectEffects/ModifyResource.h>
#include <tpublic/DirectEffects/Move.h>
#include <tpublic/DirectEffects/RemoveAura.h>
#include <tpublic/DirectEffects/Simple.h>
#include <tpublic/DirectEffects/SpawnEntity.h>
#include <tpublic/DirectEffects/Threat.h>

#include <tpublic/DirectEffectFactory.h>

namespace tpublic
{

	DirectEffectFactory::DirectEffectFactory()
	{
		_Register<DirectEffects::ApplyAura>();
		_Register<DirectEffects::ConsumeAndProduce>();
		_Register<DirectEffects::Damage>();
		_Register<DirectEffects::Fishing>();
		_Register<DirectEffects::Heal>();
		_Register<DirectEffects::Interrupt>();
		_Register<DirectEffects::LearnProfessionAbility>();
		_Register<DirectEffects::ModifyFaction>();
		_Register<DirectEffects::ModifyResource>();
		_Register<DirectEffects::Move>();
		_Register<DirectEffects::RemoveAura>();
		_Register<DirectEffects::Simple>();
		_Register<DirectEffects::SpawnEntity>();
		_Register<DirectEffects::Threat>();
	}

	DirectEffectFactory::~DirectEffectFactory()
	{

	}

	DirectEffectBase*
	DirectEffectFactory::Create(
		uint32_t			aId) const
	{
		if (!m_functions[aId])
			return NULL;

		return m_functions[aId]();
	}

}

