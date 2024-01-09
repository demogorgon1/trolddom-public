#include "../Pcheader.h"

#include <tpublic/Components/DisplayName.h>
#include <tpublic/Components/Shrine.h>

#include <tpublic/Data/Deity.h>

#include <tpublic/Systems/Shrine.h>

#include <tpublic/Pantheons.h>

namespace tpublic::Systems
{

	Shrine::Shrine(
		const SystemData* aData)
		: SystemBase(aData, UPDATE_INTERVAL)
	{
		RequireComponent<Components::DisplayName>();
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
		Components::DisplayName* displayName = GetComponent<Components::DisplayName>(aComponents);
		Components::Shrine* shrine = GetComponent<Components::Shrine>(aComponents);		

		const Pantheons::Entry* pantheonEntry = GetData()->m_pantheons->GetEntry(shrine->m_pantheonId);
		TP_CHECK(pantheonEntry != NULL, "Invalid pantheon id: %u", shrine->m_pantheonId);

		const Data::Deity* deity = pantheonEntry->m_deities[0];

		displayName->m_string = "Shrine of " + deity->m_string;
		shrine->m_deityId = deity->m_id;
	}
	
	EntityState::Id	
	Shrine::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		int32_t				/*aTicksInState*/,
		ComponentBase**		/*aComponents*/,
		Context*			/*aContext*/) 
	{
		return EntityState::CONTINUE;
	}
	
	void			
	Shrine::UpdatePublic(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		int32_t				/*aTicksInState*/,
		ComponentBase**		/*aComponents*/,
		Context*			/*aContext*/) 
	{

	}

}