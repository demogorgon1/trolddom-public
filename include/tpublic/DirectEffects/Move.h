#pragma once

#include "../DirectEffectBase.h"
#include "../SecondaryAbility.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct Move
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_MOVE;

			enum Destination : uint8_t
			{
				INVALID_DESTINATION,

				DESTINATION_TARGET_ADJACENT,
				DESTINATION_AOE_CENTER
			};

			enum MoveFlag : uint8_t
			{
				MOVE_FLAG_WALKABLE_PATH_REQUIRED	= 0x01,
				MOVE_FLAG_SET_TELEPORTED			= 0x02
			};

			static Destination
			SourceToDestination(
				const SourceNode*						aSource)
			{
				std::string_view t(aSource->GetIdentifier());
				if(t == "target_adjacent")
					return DESTINATION_TARGET_ADJACENT;
				else if (t == "aoe_center")
					return DESTINATION_AOE_CENTER;
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid destination.", aSource->GetIdentifier());
				return INVALID_DESTINATION;
			}

			static uint8_t
			SourceToMoveFlags(
				const SourceNode*						aSource)
			{
				uint8_t moveFlags = 0;
				aSource->GetArray()->ForEachChild([&](
					const SourceNode* aChild)
				{
					std::string_view t(aChild->GetIdentifier());
					if (t == "walkable_path_required")
						moveFlags |= MOVE_FLAG_WALKABLE_PATH_REQUIRED;
					else if (t == "set_teleported")
						moveFlags |= MOVE_FLAG_SET_TELEPORTED;
					else
						TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid move flag.", aSource->GetIdentifier());
				});
				return moveFlags;
			}

			Move()
			{

			}

			virtual 
			~Move()
			{

			}

			// DirectEffectBase implementation
			void			FromSource(
								const SourceNode*				aSource) override;
			void			ToStream(
								IWriter*						aStream) const override;
			bool			FromStream(
								IReader*						aStream) override;
			Result			Resolve(
								int32_t							aTick,	
								std::mt19937&					aRandom,
								const Manifest*					aManifest,
								CombatEvent::Id					aId,
								uint32_t						aAbilityId,
								const SourceEntityInstance&		aSourceEntityInstance,
								EntityInstance*					aSource,
								EntityInstance*					aTarget,
								const Vec2&						aAOETarget,
								const ItemInstanceReference&	aItem,
								IResourceChangeQueue*			aCombatResultQueue,
								IAuraEventQueue*				aAuraEventQueue,
								IEventQueue*					aEventQueue,
								const IWorldView*				aWorldView) override;

			// Public data
			Destination						m_destination = INVALID_DESTINATION;			
			uint8_t							m_moveFlags = 0;
			uint32_t						m_maxSteps = 0;
			std::optional<SecondaryAbility>	m_triggerAbilitiesOnResolve;
		};

	}

}