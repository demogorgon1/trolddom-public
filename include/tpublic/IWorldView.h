#pragma once

#include "Survival.h"
#include "Vec2.h"

namespace tpublic
{

	class CastInProgress;
	class MapData;
	class RealmModifierList;

	class IWorldView
	{
	public:
		struct EntityQuery
		{
			enum Flag : uint8_t
			{
				FLAG_LINE_OF_SIGHT = 0x01
			};

			Vec2								m_position;
			int32_t								m_maxDistance = INT32_MAX;
			uint8_t								m_flags = 0;
			const std::vector<uint32_t>*		m_entityIds = NULL;
		};

		typedef std::function<bool(const EntityInstance*, int32_t)> EntityDistanceCallback;
		typedef std::function<bool(const EntityInstance*)> EntityCallback;

		virtual										~IWorldView() {}	

		// Virtual interface
		virtual const MapData*						WorldViewGetMapData() const = 0;
		virtual size_t								WorldViewQueryEntityInstances(
														const EntityQuery&		aEntityQuery,
														EntityDistanceCallback	aEntityCallback) const = 0;
		virtual const EntityInstance*				WorldViewSingleEntityInstance(
														uint32_t				aEntityInstanceId) const = 0;
		virtual size_t								WorldViewEntityInstancesAtPosition(
														const Vec2&				aPosition,
														EntityCallback			aEntityCallback) const = 0;
		virtual void								WorldViewGroupEntityInstances(
														uint64_t				aGroupId,
														EntityCallback			aEntityCallback) const = 0;						
		virtual bool								WorldViewLineOfSight(
														const Vec2&				aFrom,
														const Vec2&				aTo) const = 0;
		virtual bool								WorldViewIsLineWalkable(
														const Vec2&				aFrom,
														const Vec2&				aTo,
														bool					aBlockedByDoors) const = 0;
		virtual uint32_t							WorldViewGetPlayerWorldCharacterId() const = 0;
		virtual bool								WorldViewIsEncounterActive(
														uint32_t				aEncounterId) const = 0;
		virtual bool								WorldViewIsAnyEncounterActive() const = 0;
		virtual bool								WorldViewGetChanneling(
														uint32_t				aSourceEntityInstanceId,
														CastInProgress&			aOut) const = 0;
		virtual uint32_t							WorldViewGetTopDeityId(
														uint32_t				aPantheonId) const = 0;
		virtual int32_t								WorldViewGetRealmBalance(
														uint32_t				aRealmBalanceId) const = 0;
		virtual size_t								WorldViewGetPVPFactionControlPointCount(
														uint32_t				aFactionId) const = 0;
		virtual bool								WorldViewHasWorldAura(
														uint32_t				aWorldAuraId) const = 0;
		virtual const std::vector<Vec2>*			WorldViewQueryZonePositions(
														uint32_t				aZoneId) const = 0;											
		virtual bool								WorldViewIsSurvivalActive() const = 0;
		virtual Survival::State						WorldViewGetSurvivalState() const = 0;
		virtual const std::unordered_set<uint32_t>&	WorldViewGetGlobalAuras() const = 0;
		virtual Vec2								WorldViewGetNearbyFreePosition(
														const Vec2&				aPosition,
														int32_t					aRadius,
														std::mt19937&			aRandom) const = 0;
		virtual const RealmModifierList*			WorldViewGetRealmModifierList() const = 0;
		virtual bool								WorldViewIsEncounterBlockingNPCAbility(
														uint32_t				aEncounterId,
														uint32_t				aAbilityId) const = 0;
		virtual bool								WorldViewIsAnyEncounterCancellingAura(
														uint32_t				aAuraId) const = 0;
		virtual bool								WorldViewIsSeasonalEventActive(
														uint32_t				aSeasonalEventId) const = 0;
		virtual bool								WorldViewIsHeroic() const = 0;

	};

}