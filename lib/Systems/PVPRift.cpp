#include "../Pcheader.h"

#include <tpublic/Components/Position.h>
#include <tpublic/Components/PVPRift.h>

#include <tpublic/Data/Deity.h>

#include <tpublic/Systems/PVPRift.h>

#include <tpublic/IEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/TimeSeed.h>

namespace tpublic::Systems
{

	namespace
	{

		enum Announcement
		{
			ANNOUNCEMENT_NOW,
			ANNOUNCEMENT_SOON
		};

		void
		_PostAnnouncement(
			const Manifest*				aManifest,
			Announcement				aAnnouncement,
			SystemBase::Context*		aContext,
			const Components::PVPRift*	aPVPRift,
			const Components::Position*	aPosition)
		{
			uint32_t topDeityId = aContext->m_worldView->WorldViewGetTopDeityId(aPVPRift->m_pantheonId);

			if (topDeityId == 0)
				topDeityId = aPVPRift->m_announcementDefaultDeityId;

			if (topDeityId != 0)
			{				
				const Data::Deity* deity = aManifest->GetById<Data::Deity>(topDeityId);
				const char* announcementText = NULL;

				switch(aAnnouncement)
				{
				case ANNOUNCEMENT_NOW:	announcementText = deity->m_announcementNow.c_str(); break;
				case ANNOUNCEMENT_SOON:	announcementText = deity->m_announcementSoon.c_str(); break;
				default:				break;
				}

				std::string deityName = Helpers::Format("%s%s", deity->m_string.c_str(), deity->m_titleSuffix.c_str());
				aContext->m_eventQueue->EventQueueChatAnnouncement(deityName.c_str(), aPosition->m_position, aPVPRift->m_announcementRange, true, announcementText);
			}
		}

	}

	//---------------------------------------------------------------------------

	PVPRift::PVPRift(
		const SystemData*	aData)
		: SystemBase(aData, UPDATE_INTERVAL)
	{
		RequireComponent<Components::Position>();
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
		Context*			aContext) 
	{
		EntityState::Id returnValue = EntityState::CONTINUE;

		Components::PVPRift* pvpRift = GetComponent<Components::PVPRift>(aComponents);

		switch(aEntityState)
		{
		case EntityState::ID_SPAWNING:
			{
				uint64_t currentTime = (uint64_t)time(NULL);

				if(pvpRift->m_openTimeStamp == 0)
				{
					pvpRift->m_openTimeStamp = TimeSeed::GetNextTimeStamp(currentTime, TimeSeed::TYPE_HOURLY);
					pvpRift->m_announced = false;
					pvpRift->SetDirty();
				}

				if(currentTime >= pvpRift->m_openTimeStamp)
				{
					pvpRift->m_openTimeStamp = TimeSeed::GetNextTimeStamp(currentTime, TimeSeed::TYPE_HOURLY);
					pvpRift->m_announced = false;
					pvpRift->SetDirty();

					returnValue = EntityState::ID_DEFAULT;

					const Components::Position* position = GetComponent<Components::Position>(aComponents);
					_PostAnnouncement(GetManifest(), ANNOUNCEMENT_NOW, aContext, pvpRift, position);
				}
				else if(pvpRift->m_pantheonId != 0 && !pvpRift->m_announced && pvpRift->m_openTimeStamp - currentTime < 5 * 60)
				{
					pvpRift->m_announced = true;					

					const Components::Position* position = GetComponent<Components::Position>(aComponents);
					_PostAnnouncement(GetManifest(), ANNOUNCEMENT_SOON, aContext, pvpRift, position);
				}
			}
			break;

		case EntityState::ID_DEFAULT:
			if(pvpRift->m_activated)
			{
				pvpRift->m_activated = false;

				returnValue = EntityState::ID_SPAWNING;
			}
			break;

		default:
			break;
		}

		return returnValue;
	}

}