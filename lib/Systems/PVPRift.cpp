#include "../Pcheader.h"

#include <tpublic/Components/PVPRift.h>

#include <tpublic/Systems/PVPRift.h>

#include <tpublic/TimeSeed.h>

namespace tpublic::Systems
{

	PVPRift::PVPRift(
		const SystemData*	aData)
		: SystemBase(aData, UPDATE_INTERVAL)
	{
		RequireComponent<Components::PVPRift>();
	}
	
	PVPRift::~PVPRift()
	{

	}

	//---------------------------------------------------------------------------

	EntityState::Id
	PVPRift::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		Components::PVPRift* pvpRift = GetComponent<Components::PVPRift>(aComponents);

		switch(aEntityState)
		{
		case EntityState::ID_SPAWNING:
			{
				uint64_t currentTime = (uint64_t)time(NULL);

				if(pvpRift->m_openTimeStamp == 0)
				{
					pvpRift->m_openTimeStamp = TimeSeed::GetNextTimeStamp(currentTime, TimeSeed::TYPE_HOURLY);
					pvpRift->SetDirty();
				}

				if(currentTime >= pvpRift->m_openTimeStamp)
				{
					pvpRift->m_openTimeStamp = TimeSeed::GetNextTimeStamp(currentTime, TimeSeed::TYPE_HOURLY);
					pvpRift->SetDirty();

					return EntityState::ID_DEFAULT;
				}
			}
			break;

		default:
			break;
		}

		return EntityState::CONTINUE;
	}

}