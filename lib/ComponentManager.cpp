#include "Pcheader.h"

#include <tpublic/Components/Abilities.h>
#include <tpublic/Components/AbilityModifiers.h>
#include <tpublic/Components/Achievements.h>
#include <tpublic/Components/AccountAchievements.h>
#include <tpublic/Components/ActionBars.h>
#include <tpublic/Components/ActiveQuests.h>
#include <tpublic/Components/Auctioneer.h>
#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CharacterInfoView.h>
#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/CompletedQuests.h>
#include <tpublic/Components/ControlPoint.h>
#include <tpublic/Components/Critter.h>
#include <tpublic/Components/DeityDiscovery.h>
#include <tpublic/Components/DeityRelations.h>
#include <tpublic/Components/DisplayName.h>
#include <tpublic/Components/Environment.h>
#include <tpublic/Components/EquippedItems.h>
#include <tpublic/Components/Gateway.h>
#include <tpublic/Components/GuildName.h>
#include <tpublic/Components/GuildRegistrar.h>
#include <tpublic/Components/HoverText.h>
#include <tpublic/Components/InteractableObject.h>
#include <tpublic/Components/Inventory.h>
#include <tpublic/Components/KillContribution.h>
#include <tpublic/Components/Lootable.h>
#include <tpublic/Components/MinionPrivate.h>
#include <tpublic/Components/MinionPublic.h>
#include <tpublic/Components/NPC.h>
#include <tpublic/Components/Openable.h>
#include <tpublic/Components/Owner.h>
#include <tpublic/Components/PlayerAuctions.h>
#include <tpublic/Components/PlayerCooldowns.h>
#include <tpublic/Components/PlayerInbox.h>
#include <tpublic/Components/PlayerMinions.h>
#include <tpublic/Components/PlayerOutbox.h>
#include <tpublic/Components/PlayerPrivate.h>
#include <tpublic/Components/PlayerPublic.h>
#include <tpublic/Components/PlayerStats.h>
#include <tpublic/Components/PlayerWorldStash.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/PVPRift.h>
#include <tpublic/Components/RandomItemVendor.h>
#include <tpublic/Components/RecentAchievements.h>
#include <tpublic/Components/Reputation.h>
#include <tpublic/Components/ResurrectionPoint.h>
#include <tpublic/Components/Shrine.h>
#include <tpublic/Components/SoundSource.h>
#include <tpublic/Components/SpiritStoneDiscovery.h>
#include <tpublic/Components/Sprite.h>
#include <tpublic/Components/Stash.h>
#include <tpublic/Components/SurvivalInfo.h>
#include <tpublic/Components/Tag.h>
#include <tpublic/Components/Talents.h>
#include <tpublic/Components/ThreatSource.h>
#include <tpublic/Components/ThreatTarget.h>
#include <tpublic/Components/Trade.h>
#include <tpublic/Components/Trigger.h>
#include <tpublic/Components/VendorBuyback.h>
#include <tpublic/Components/VendorStock.h>
#include <tpublic/Components/VisibleAuras.h>
#include <tpublic/Components/Visibility.h>
#include <tpublic/Components/Wallet.h>
#include <tpublic/Components/ZoneDiscovery.h>

#include <tpublic/ComponentManager.h>
#include <tpublic/UniqueComponent.h>

namespace tpublic
{

	ComponentManager::ComponentManager()
	{
		_Register<Components::Abilities>();
		_Register<Components::AbilityModifiers>();
		_Register<Components::Achievements>();
		_Register<Components::AccountAchievements>();
		_Register<Components::ActionBars>();
		_Register<Components::ActiveQuests>();
		_Register<Components::Auctioneer>();
		_Register<Components::Auras>();
		_Register<Components::CharacterInfoView>();
		_Register<Components::CombatPrivate>();
		_Register<Components::CombatPublic>();
		_Register<Components::CompletedQuests>();
		_Register<Components::ControlPoint>();
		_Register<Components::Critter>();
		_Register<Components::DeityDiscovery>();
		_Register<Components::DeityRelations>();
		_Register<Components::DisplayName>();
		_Register<Components::Environment>();
		_Register<Components::EquippedItems>();
		_Register<Components::Gateway>();
		_Register<Components::GuildName>();
		_Register<Components::GuildRegistrar>();
		_Register<Components::HoverText>();
		_Register<Components::InteractableObject>();
		_Register<Components::Inventory>();
		_Register<Components::KillContribution>();
		_Register<Components::Lootable>();
		_Register<Components::MinionPrivate>();
		_Register<Components::MinionPublic>();
		_Register<Components::NPC>();
		_Register<Components::Openable>();
		_Register<Components::Owner>();
		_Register<Components::PlayerAuctions>();
		_Register<Components::PlayerCooldowns>();
		_Register<Components::PlayerInbox>();
		_Register<Components::PlayerMinions>();
		_Register<Components::PlayerOutbox>();
		_Register<Components::PlayerPublic>();
		_Register<Components::PlayerPrivate>();
		_Register<Components::PlayerStats>();
		_Register<Components::PlayerWorldStash>();
		_Register<Components::Position>();
		_Register<Components::PVPRift>();
		_Register<Components::RandomItemVendor>();
		_Register<Components::RecentAchievements>();
		_Register<Components::Reputation>();
		_Register<Components::ResurrectionPoint>();
		_Register<Components::Shrine>();
		_Register<Components::SoundSource>();
		_Register<Components::SpiritStoneDiscovery>();
		_Register<Components::Sprite>();
		_Register<Components::Stash>();
		_Register<Components::SurvivalInfo>();
		_Register<Components::Tag>();
		_Register<Components::Talents>();
		_Register<Components::ThreatSource>();
		_Register<Components::ThreatTarget>();
		_Register<Components::Trade>();
		_Register<Components::Trigger>();
		_Register<Components::VendorBuyback>();
		_Register<Components::VendorStock>();
		_Register<Components::VisibleAuras>();
		_Register<Components::Visibility>();
		_Register<Components::Wallet>();
		_Register<Components::ZoneDiscovery>();
	}

	ComponentManager::~ComponentManager()
	{

	}

	void						
	ComponentManager::AllocateUniqueComponent(
		uint32_t				aId,
		UniqueComponent&		aOut) const
	{
		assert(aId > 0 && aId < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aId];
		assert(t.m_create);
		assert(t.m_delete);
		aOut.Set(t.m_create(), t.m_delete);

		aOut.GetPointer()->InitAllocation(aId, NULL, 0);
	}

	ComponentBase*
	ComponentManager::AllocateComponent(
		uint32_t				aId) 
	{
		assert(aId > 0 && aId < Component::NUM_IDS);
		ComponentType& t = m_componentTypes[aId];
		assert(t.m_pool);
		ComponentBase* component = t.m_pool->Allocate();
		return component;
	}

	void						
	ComponentManager::ReleaseComponent(
		ComponentBase*			aComponent)
	{
		assert(aComponent->GetComponentId() > 0 && aComponent->GetComponentId() < Component::NUM_IDS);
		ComponentType& t = m_componentTypes[aComponent->GetComponentId()];
		assert(t.m_pool);
		t.m_pool->Release(aComponent);
	}

	const ComponentSchema* 
	ComponentManager::GetComponentSchema(
		uint32_t				aId) const
	{
		assert(aId > 0 && aId < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aId];
		return &t.m_schema;
	}

	uint8_t				
	ComponentManager::GetComponentFlags(
		uint32_t				aId) const
	{
		assert(aId > 0 && aId < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aId];
		return t.m_flags;
	}
	
	Persistence::Id		
	ComponentManager::GetComponentPersistence(
		uint32_t				aId) const
	{
		assert(aId > 0 && aId < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aId];
		return t.m_persistence;
	}

	ComponentBase::Replication
	ComponentManager::GetComponentReplication(
		uint32_t				aId) const
	{
		assert(aId > 0 && aId < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aId];
		return t.m_replication;
	}

	std::string
	ComponentManager::AsDebugString(
		const ComponentBase*	aComponent) const
	{
		assert(aComponent->GetComponentId() > 0 && aComponent->GetComponentId() < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aComponent->GetComponentId()];

		return t.m_schema.AsDebugString(aComponent);
	}

	void				
	ComponentManager::WriteNetwork(
		IWriter*				aWriter,
		const ComponentBase*	aComponent) const
	{
		assert(aComponent->GetComponentId() > 0 && aComponent->GetComponentId() < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aComponent->GetComponentId()];

		t.m_schema.WriteNetwork(aWriter, aComponent);
	}
	
	bool				
	ComponentManager::ReadNetwork(
		IReader*				aReader,
		ComponentBase*			aComponent) const
	{
		assert(aComponent->GetComponentId() > 0 && aComponent->GetComponentId() < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aComponent->GetComponentId()];

		return t.m_schema.ReadNetwork(aReader, aComponent);
	}
	
	void				
	ComponentManager::WriteStorage(
		IWriter*				aWriter,
		const ComponentBase*	aComponent) const
	{
		assert(aComponent->GetComponentId() > 0 && aComponent->GetComponentId() < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aComponent->GetComponentId()];

		t.m_schema.WriteStorage(aWriter, aComponent);
	}
	
	bool				
	ComponentManager::ReadStorage(
		IReader*				aReader,
		ComponentBase*			aComponent,
		const Manifest*			aManifest) const
	{
		assert(aComponent->GetComponentId() > 0 && aComponent->GetComponentId() < Component::NUM_IDS);
		const ComponentType& t = m_componentTypes[aComponent->GetComponentId()];

		return t.m_schema.ReadStorage(aManifest, aReader, aComponent);
	}

	void				
	ComponentManager::ReadSource(
		const SourceNode*		aSource,
		ComponentBase*			aComponent) const
	{
		assert(aComponent->GetComponentId() > 0 && aComponent->GetComponentId() < Component::NUM_IDS);
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
