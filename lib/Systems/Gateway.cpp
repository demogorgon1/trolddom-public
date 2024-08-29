#include "../Pcheader.h"

#include <tpublic/Components/Gateway.h>
#include <tpublic/Components/Position.h>

#include <tpublic/Systems/Gateway.h>

#include <tpublic/TimeSeed.h>
#include <tpublic/UniformDistribution.h>

namespace tpublic::Systems
{

	Gateway::Gateway(
		const SystemData* aData)
		: SystemBase(aData, UPDATE_INTERVAL)
	{
		RequireComponent<Components::Gateway>();
		RequireComponent<Components::Position>();
	}
	
	Gateway::~Gateway()
	{

	}

	//-----------------------------------------------------------------------------
	
	EntityState::Id	
	Gateway::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		Components::Gateway* gateway = GetComponent<Components::Gateway>(aComponents);
		const Components::Position* position = GetComponent<Components::Position>(aComponents);

		TimeSeed timeSeed;
		timeSeed.Update(gateway->m_schedule.m_timeSeedType, (uint64_t)time(NULL));

		if(gateway->m_usePositionForRandomization)
			timeSeed.m_seed ^= position->m_position.GetHash32();

		if(gateway->m_activeGatewayInstance.m_timeSeed != timeSeed)
		{
			// Pick map 
			if(gateway->m_maps.size() > 1)
			{
				std::mt19937 random(timeSeed.m_seed);
				UniformDistribution<size_t> distribution(0, gateway->m_maps.size() - 1);
				gateway->m_activeGatewayInstance.m_mapId = gateway->m_maps[distribution(random)];
			}
			else
			{
				TP_CHECK(gateway->m_maps.size() == 1, "No maps defined for gateway.");
				gateway->m_activeGatewayInstance.m_mapId = gateway->m_maps[0];
			}

			// Update seed
			gateway->m_activeGatewayInstance.m_timeSeed = timeSeed;

			gateway->SetDirty();
		}

		return EntityState::CONTINUE;
	}
	
}
