#include "Pcheader.h"

#include <tpublic/Components/Abilities.h>
#include <tpublic/Components/Achievements.h>
#include <tpublic/Components/ActionBars.h>
#include <tpublic/Components/ActiveQuests.h>
#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/CompletedQuests.h>
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
#include <tpublic/Components/PlayerStats.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/RecentAchievements.h>
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

#include <tpublic/ComponentManager.h>

namespace tpublic
{

	ComponentManager::ComponentManager()
	{
		_Register<Components::Abilities>();
		_Register<Components::Achievements>();
		_Register<Components::ActionBars>();
		_Register<Components::ActiveQuests>();
		_Register<Components::Auras>();
		_Register<Components::CombatPrivate>();
		_Register<Components::CombatPublic>();
		_Register<Components::CompletedQuests>();
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
		_Register<Components::PlayerStats>();
		_Register<Components::Position>();
		_Register<Components::RecentAchievements>();
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

	ComponentManager::~ComponentManager()
	{

	}

	ComponentBase*
	ComponentManager::Create(
		uint32_t				aId) const
	{
		assert(aId < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aId];

		if(!t.m_create)
			return NULL;

		ComponentBase* component = t.m_create();
		component->SetComponentId(aId);
		return component;
	}

	uint8_t				
	ComponentManager::GetComponentFlags(
		uint32_t				aId) const
	{
		assert(aId < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aId];
		return t.m_flags;
	}
	
	Persistence::Id		
	ComponentManager::GetComponentPersistence(
		uint32_t				aId) const
	{
		assert(aId < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aId];
		return t.m_persistence;
	}

	ComponentBase::Replication
	ComponentManager::GetComponentReplication(
		uint32_t				aId) const
	{
		assert(aId < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aId];
		return t.m_replication;
	}

	std::string
	ComponentManager::AsDebugString(
		const ComponentBase*	aComponent) const
	{
		const ComponentType& t = m_componentTypes[aComponent->GetComponentId()];

		return t.m_schema.AsDebugString(aComponent);
	}

	void				
	ComponentManager::WriteNetwork(
		IWriter*				aWriter,
		const ComponentBase*	aComponent) const
	{
		const ComponentType& t = m_componentTypes[aComponent->GetComponentId()];

		t.m_schema.WriteNetwork(aWriter, aComponent);
	}
	
	bool				
	ComponentManager::ReadNetwork(
		IReader*				aReader,
		ComponentBase*			aComponent) const
	{
		const ComponentType& t = m_componentTypes[aComponent->GetComponentId()];

		return t.m_schema.ReadNetwork(aReader, aComponent);
	}
	
	void				
	ComponentManager::WriteStorage(
		IWriter*				aWriter,
		const ComponentBase*	aComponent) const
	{
		const ComponentType& t = m_componentTypes[aComponent->GetComponentId()];

		t.m_schema.WriteStorage(aWriter, aComponent);
	}
	
	bool				
	ComponentManager::ReadStorage(
		IReader*				aReader,
		ComponentBase*			aComponent,
		const Manifest*			aManifest) const
	{
		const ComponentType& t = m_componentTypes[aComponent->GetComponentId()];

		return t.m_schema.ReadStorage(aManifest, aReader, aComponent);
	}

	void				
	ComponentManager::ReadSource(
		const SourceNode*		aSource,
		ComponentBase*			aComponent) const
	{
		const ComponentType& t = m_componentTypes[aComponent->GetComponentId()];

		t.m_schema.ReadSource(aSource, aComponent);
	}

	//----------------------------------------------------------------------------------

	void				 
	ComponentManager::_InitComponentType(
		ComponentType&			aComponentType)
	{
		aComponentType.m_schema.InitUpgradeChains();
		aComponentType.m_schema.Validate();
	}

}
