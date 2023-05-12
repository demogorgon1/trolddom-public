#include "Pcheader.h"

#include <kpublic/Components/Abilities.h>
#include <kpublic/Components/ActionBars.h>
#include <kpublic/Components/Auras.h>
#include <kpublic/Components/CombatPrivate.h>
#include <kpublic/Components/CombatPublic.h>
#include <kpublic/Components/DisplayName.h>
#include <kpublic/Components/EquippedItems.h>
#include <kpublic/Components/Inventory.h>
#include <kpublic/Components/NPC.h>
#include <kpublic/Components/PlayerCooldowns.h>
#include <kpublic/Components/PlayerPrivate.h>
#include <kpublic/Components/PlayerPublic.h>
#include <kpublic/Components/Position.h>
#include <kpublic/Components/Sprite.h>
#include <kpublic/Components/Talents.h>
#include <kpublic/Components/ThreatSource.h>
#include <kpublic/Components/ThreatTarget.h>
#include <kpublic/Components/VisibleAuras.h>
#include <kpublic/Components/Wallet.h>

#include <kpublic/ComponentFactory.h>

namespace tpublic
{

	ComponentFactory::ComponentFactory()
	{
		_Register<Components::Abilities>();
		_Register<Components::ActionBars>();
		_Register<Components::Auras>();
		_Register<Components::CombatPrivate>();
		_Register<Components::CombatPublic>();
		_Register<Components::DisplayName>();
		_Register<Components::EquippedItems>();
		_Register<Components::Inventory>();
		_Register<Components::NPC>();
		_Register<Components::PlayerPrivate>();
		_Register<Components::PlayerCooldowns>();
		_Register<Components::PlayerPublic>();
		_Register<Components::Position>();
		_Register<Components::Sprite>();
		_Register<Components::Talents>();
		_Register<Components::ThreatSource>();
		_Register<Components::ThreatTarget>();
		_Register<Components::VisibleAuras>();
		_Register<Components::Wallet>();
	}

	ComponentFactory::~ComponentFactory()
	{

	}

	ComponentBase*
	ComponentFactory::Create(
		uint32_t			aId) const
	{
		if(!m_functions[aId])
			return NULL;

		return m_functions[aId]();
	}

}