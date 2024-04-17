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

				DESTINATION_TARGET_ADJACENT
			};

			enum MoveFlag : uint8_t
			{
				MOVE_FLAG_WALKABLE_PATH_REQUIRED = 0x01
			};

			static Destination
			SourceToDestination(
				const SourceNode*						aSource)
			{
				std::string_view t(aSource->GetIdentifier());
				if(t == "target_adjacent")
					return DESTINATION_TARGET_ADJACENT;
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

			// EffectBase implementation
			void			FromSource(
								const SourceNode*				aSource) override;
			void			ToStream(
								IWriter*						aStream) const override;
			bool			FromStream(
								IReader*						aStream) override;
			CombatEvent::Id	Resolve(
								int32_t							aTick,	
								std::mt19937&					aRandom,
								const Manifest*					aManifest,
								CombatEvent::Id					aId,
								uint32_t						aAbilityId,
								EntityInstance*					aSource,
								EntityInstance*					aTarget,
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