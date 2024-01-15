#include "../Pcheader.h"

#include <tpublic/Components/DisplayName.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/Shrine.h>

#include <tpublic/Data/Deity.h>
#include <tpublic/Data/Pantheon.h>

#include <tpublic/Systems/Shrine.h>

#include <tpublic/Manifest.h>
#include <tpublic/Pantheons.h>

namespace tpublic::Systems
{

	Shrine::Shrine(
		const SystemData* aData)
		: SystemBase(aData, UPDATE_INTERVAL)
	{
		RequireComponent<Components::DisplayName>();
		RequireComponent<Components::Position>();
		RequireComponent<Components::Shrine>();
	}
	
	Shrine::~Shrine()
	{

	}

	//-----------------------------------------------------------------------------

	void			
	Shrine::Init(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		ComponentBase**		aComponents,
		int32_t				/*aTick*/) 
	{
		Components::Shrine* shrine = GetComponent<Components::Shrine>(aComponents);

		const Data::Deity* deity = NULL;

		if (shrine->m_deityId == 0)
		{
			// No deity specified, pick one
			Components::Position* position = GetComponent<Components::Position>(aComponents);

			const Pantheons::Entry* pantheonEntry = GetData()->m_pantheons->GetEntry(shrine->m_pantheonId);
			TP_CHECK(pantheonEntry != NULL, "Invalid pantheon id: %u", shrine->m_pantheonId);

			// The deity is derived from map and position (so a specific shrine will always have the same associated deity)
			Hash::CheckSum locationHash;
			locationHash.AddPOD(position->m_position);
			locationHash.AddPOD(GetData()->m_mapId);
			locationHash.AddPOD(GetData()->m_mapSeed);
			deity = pantheonEntry->m_deities[((uint32_t)pantheonEntry->m_deities.size() * (locationHash.m_hash >> 16)) / 0x10000];					
			shrine->m_deityId = deity->m_id;
		}
		else
		{
			deity = GetManifest()->GetById<Data::Deity>(shrine->m_deityId);
		}

		assert(deity != NULL);
		const Data::Pantheon* pantheon = GetManifest()->GetById<Data::Pantheon>(shrine->m_pantheonId);

		Components::DisplayName* displayName = GetComponent<Components::DisplayName>(aComponents);		
		displayName->m_string = pantheon->m_shrineDisplayNamePrefix + deity->m_string;
	}
	
	EntityState::Id	
	Shrine::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		int32_t				aTicksInState,
		ComponentBase**		/*aComponents*/,
		Context*			/*aContext*/) 
	{
		if (aEntityState == EntityState::ID_DESPAWNING)
			return aTicksInState < DESPAWN_TICKS ? EntityState::CONTINUE : EntityState::ID_DESPAWNED;

		return EntityState::CONTINUE;
	}
	
}
