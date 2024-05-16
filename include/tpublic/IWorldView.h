#pragma once

namespace tpublic
{

	class MapData;

	class IWorldView
	{
	public:
		struct EntityQuery
		{
			enum Flag : uint8_t
			{
				FLAG_LINE_OF_SIGHT = 0x01
			};

			Vec2		m_position;
			int32_t		m_maxDistance = INT32_MAX;
			uint8_t		m_flags = 0;
		};

		typedef std::function<bool(const EntityInstance*, int32_t)> EntityDistanceCallback;
		typedef std::function<bool(const EntityInstance*)> EntityCallback;

		virtual							~IWorldView() {}	

		// Virtual interface
		virtual const MapData*			WorldViewGetMapData() const = 0;
		virtual size_t					WorldViewQueryEntityInstances(
											const EntityQuery&		aEntityQuery,
											EntityDistanceCallback	aEntityCallback) const = 0;
		virtual const EntityInstance*	WorldViewSingleEntityInstance(
											uint32_t				aEntityInstanceId) const = 0;
		virtual size_t					WorldViewEntityInstancesAtPosition(
											const Vec2&				aPosition,
											EntityCallback			aEntityCallback) const = 0;
		virtual void					WorldViewGroupEntityInstances(
											uint64_t				aGroupId,
											EntityCallback			aEntityCallback) const = 0;						
		virtual bool					WorldViewLineOfSight(
											const Vec2&				aFrom,
											const Vec2&				aTo) const = 0;
		virtual uint32_t				WorldViewGetPlayerWorldCharacterId() const = 0;
	};

}