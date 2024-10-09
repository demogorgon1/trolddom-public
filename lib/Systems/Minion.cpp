#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/DisplayName.h>
#include <tpublic/Components/MinionPrivate.h>
#include <tpublic/Components/MinionPublic.h>

#include <tpublic/Data/NameTemplate.h>

#include <tpublic/Systems/Minion.h>

#include <tpublic/CreateName.h>
#include <tpublic/Manifest.h>
#include <tpublic/WordList.h>

namespace tpublic::Systems
{

	Minion::Minion(
		const SystemData*	aData)
		: SystemBase(aData)
	{
		RequireComponent<Components::CombatPublic>();
		RequireComponent<Components::DisplayName>();
		RequireComponent<Components::MinionPrivate>();
		RequireComponent<Components::MinionPublic>();
	}
	
	Minion::~Minion()
	{

	}

	//---------------------------------------------------------------------------

	void			
	Minion::Init(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		ComponentBase**		aComponents,
		int32_t				/*aTick*/) 
	{
		Components::MinionPrivate* minionPrivate = GetComponent<Components::MinionPrivate>(aComponents);
		Components::DisplayName* displayName = GetComponent<Components::DisplayName>(aComponents);

		// Name
		if(displayName->m_string.empty())
		{
			std::mt19937 random(minionPrivate->m_seed);
			TP_CHECK(minionPrivate->m_nameTemplateId != 0, "Missing minion name template.");			
			const Data::NameTemplate* nameTemplate = GetManifest()->GetById<Data::NameTemplate>(minionPrivate->m_nameTemplateId);
			WordList::QueryCache queryCache(&GetManifest()->m_wordList);
			std::unordered_set<uint32_t> tags;
			CreateName(GetManifest(), nameTemplate, &queryCache, random, displayName->m_string, tags);
			displayName->SetDirty();
		}

		// Initialize resources		
		{
			Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);

			for (const Components::MinionPrivate::ResourceEntry& resource : minionPrivate->m_resources.m_entries)
			{
				combatPublic->AddResourceMax(resource.m_id, resource.m_max);

				const Resource::Info* info = Resource::GetInfo((Resource::Id)resource.m_id);
				if (info->m_flags & Resource::FLAG_DEFAULT_TO_MAX)
					combatPublic->SetResourceToMax(resource.m_id);
			}

			combatPublic->SetDirty();
		}
	}
	
	EntityState::Id	
	Minion::UpdatePrivate(
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
	Minion::UpdatePublic(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		int32_t				/*aTicksInState*/,
		ComponentBase**		/*aComponents*/,
		Context*			/*aContext*/) 
	{
	}

}