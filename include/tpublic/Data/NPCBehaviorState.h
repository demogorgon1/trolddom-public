#pragma once

#include "../DataBase.h"
#include "../NPCBehavior.h"

namespace tpublic
{

	namespace Data
	{

		struct NPCBehaviorState
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_NPC_BEHAVIOR_STATE;
			static const bool TAGGED = false;

			struct OnRoute
			{
				OnRoute()
				{

				}

				OnRoute(
					const SourceNode*	aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing route annotation.");
					m_routeId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ROUTE, aSource->m_annotation->GetIdentifier());
					m_npcBehaviorStateId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_NPC_BEHAVIOR_STATE, aSource->GetIdentifier());
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_routeId);
					aWriter->WriteUInt(m_npcBehaviorStateId);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if (!aReader->ReadUInt(m_routeId))
						return false;
					if (!aReader->ReadUInt(m_npcBehaviorStateId))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_routeId = 0;
				uint32_t			m_npcBehaviorStateId = 0;
			};

			void
			Verify() const
			{
				VerifyBase();

				TP_VERIFY(m_behavior != NPCBehavior::INVALID_ID, m_debugInfo, "Missing NPC behavior.");
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{	
				aNode->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "behavior")
						{
							m_behavior = NPCBehavior::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_behavior != NPCBehavior::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid NPC behavior.", aChild->GetIdentifier());
						}
						else if (aChild->m_name == "max_range")
						{
							m_maxRange = aChild->GetUInt32();
						}
						else if (aChild->m_name == "combat_event_pause_ticks")
						{
							m_combatEventPauseTicks = aChild->GetInt32();
						}
						else if (aChild->m_name == "max_ticks")
						{
							m_maxTicks = aChild->GetUInt32();
						}
						else if(aChild->m_name == "pause_when_targeted_by_nearby_player")
						{
							m_pauseWhenTargetedByNearbyPlayer = aChild->GetBool();
						}
						else if(aChild->m_name == "on_route")
						{
							m_onRoute = OnRoute(aChild);
						}
						else if(aChild->m_name == "despawn_if_lost_player")
						{
							m_despawnIfLostPlayer = aChild->GetBool();
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});
				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WritePOD(m_behavior);
				aStream->WriteUInt(m_maxRange);
				aStream->WriteUInt(m_maxTicks);
				aStream->WriteBool(m_pauseWhenTargetedByNearbyPlayer);
				aStream->WriteOptionalObject(m_onRoute);
				aStream->WriteInt(m_combatEventPauseTicks);
				aStream->WriteBool(m_despawnIfLostPlayer);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadPOD(m_behavior))
					return false;
				if (!aStream->ReadUInt(m_maxRange))
					return false;
				if (!aStream->ReadUInt(m_maxTicks))
					return false;
				if(!aStream->ReadBool(m_pauseWhenTargetedByNearbyPlayer))
					return false;
				if (!aStream->ReadOptionalObject(m_onRoute))
					return false;
				if (!aStream->ReadInt(m_combatEventPauseTicks))
					return false;
				if (!aStream->ReadBool(m_despawnIfLostPlayer))
					return false;
				return true;
			}

			// Public data
			NPCBehavior::Id			m_behavior = NPCBehavior::INVALID_ID;
			uint32_t				m_maxRange = 0;
			uint32_t				m_maxTicks = 0;
			bool					m_pauseWhenTargetedByNearbyPlayer = false;
			int32_t					m_combatEventPauseTicks = 0;
			std::optional<OnRoute>	m_onRoute;
			bool					m_despawnIfLostPlayer = false;
		};

	}

}