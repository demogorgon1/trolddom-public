#pragma once

namespace tpublic
{

	class MapData;

	class IWorldView
	{
	public:
		typedef std::function<bool(const EntityInstance*)> EntityCallback;

		virtual							~IWorldView() {}	

		// Virtual interface
		virtual const MapData*			WorldViewGetMapData() const = 0;
		virtual size_t					WorldViewAllEntityInstances(
											EntityCallback			aEntityCallback) const = 0;
		virtual const EntityInstance*	WorldViewSingleEntityInstance(
											uint32_t				aEntityInstanceId) const = 0;
		virtual size_t					WorldViewEntityInstancesAtPosition(
											const tpublic::Vec2&	aPosition,
											EntityCallback			aEntityCallback) const = 0;
		virtual void					WorldViewGroupEntityInstances(
											uint64_t				aGroupId,
											EntityCallback			aEntityCallback) const = 0;						
		virtual bool					WorldViewLineOfSight(
											const tpublic::Vec2&	aFrom,
											const tpublic::Vec2&	aTo) const = 0;
		virtual uint32_t				WorldViewGetPlayerWorldCharacterId() const = 0;

	};

}