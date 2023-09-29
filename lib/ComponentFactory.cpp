#include "Pcheader.h"

#include <tpublic/Components/Abilities.h>
#include <tpublic/Components/ActionBars.h>
#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/DisplayName.h>
#include <tpublic/Components/Environment.h>
#include <tpublic/Components/EquippedItems.h>
#include <tpublic/Components/GuildRegistrar.h>
#include <tpublic/Components/Inventory.h>
#include <tpublic/Components/Lootable.h>
#include <tpublic/Components/NPC.h>
#include <tpublic/Components/Owner.h>
#include <tpublic/Components/PlayerCooldowns.h>
#include <tpublic/Components/PlayerPrivate.h>
#include <tpublic/Components/PlayerPublic.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/ResurrectionPoint.h>
#include <tpublic/Components/Sprite.h>
#include <tpublic/Components/Tag.h>
#include <tpublic/Components/Talents.h>
#include <tpublic/Components/ThreatSource.h>
#include <tpublic/Components/ThreatTarget.h>
#include <tpublic/Components/Trade.h>
#include <tpublic/Components/VendorBuyback.h>
#include <tpublic/Components/VisibleAuras.h>
#include <tpublic/Components/Wallet.h>

#include <tpublic/ComponentFactory.h>

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
		_Register<Components::Environment>();
		_Register<Components::EquippedItems>();
		_Register<Components::GuildRegistrar>();
		_Register<Components::Inventory>();
		_Register<Components::Lootable>();
		_Register<Components::NPC>();
		_Register<Components::Owner>();
		_Register<Components::PlayerPrivate>();
		_Register<Components::PlayerCooldowns>();
		_Register<Components::PlayerPublic>();
		_Register<Components::Position>();
		_Register<Components::ResurrectionPoint>();
		_Register<Components::Sprite>();
		_Register<Components::Tag>();
		_Register<Components::Talents>();
		_Register<Components::ThreatSource>();
		_Register<Components::ThreatTarget>();
		_Register<Components::Trade>();
		_Register<Components::VendorBuyback>();
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