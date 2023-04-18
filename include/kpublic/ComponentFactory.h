#pragma once

#include "Components/Abilities.h"
#include "Components/CombatPrivate.h"
#include "Components/CombatPublic.h"
#include "Components/DisplayName.h"
#include "Components/EquippedItems.h"
#include "Components/Inventory.h"
#include "Components/NPC.h"
#include "Components/PlayerPrivate.h"
#include "Components/PlayerPublic.h"
#include "Components/Position.h"
#include "Components/Sprite.h"
#include "Components/Talents.h"
#include "Components/Wallet.h"

#include "Component.h"

namespace kpublic
{

	class ComponentFactory
	{
	public:
		ComponentFactory()
		{
			_Register<Components::Abilities>();
			_Register<Components::CombatPrivate>();
			_Register<Components::CombatPublic>();
			_Register<Components::DisplayName>();
			_Register<Components::EquippedItems>();
			_Register<Components::Inventory>();
			_Register<Components::NPC>();
			_Register<Components::PlayerPrivate>();
			_Register<Components::PlayerPublic>();
			_Register<Components::Position>();
			_Register<Components::Sprite>();
			_Register<Components::Talents>();
			_Register<Components::Wallet>();
		}

		~ComponentFactory()
		{

		}

		ComponentBase*
		Create(
			uint32_t			aId) const
		{
			if(!m_functions[aId])
				return NULL;

			return m_functions[aId]();
		}

	private:

		std::function<ComponentBase*()>				m_functions[Component::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = []() { return new _T(); };
		}

	};

}