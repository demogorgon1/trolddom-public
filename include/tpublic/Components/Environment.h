#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Environment
			: public ComponentBase
		{			
			static const Component::Id ID = Component::ID_ENVIRONMENT;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			enum Field
			{
				FIELD_TICK_INTERVAL,
				FIELD_DURATION,
				FIELD_ABILITY
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_TICK_INTERVAL, "tick_interval", offsetof(Environment, m_tickInterval));
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_DURATION, "tick_duration", offsetof(Environment, m_duration));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_ABILITY, "ability", offsetof(Environment, m_abilityId))->SetDataType(DataType::ID_ABILITY);
			}

			Environment()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~Environment()
			{

			}

			// ComponentBase implementation
			void
			FromSource( 
				const Parser::Node*	aSource)
			{
				aSource->ForEachChild([&](
					const Parser::Node* aChild)
				{
					if(aChild->m_name == "tick_interval")					
						m_tickInterval = aChild->GetInt32();
					else if (aChild->m_name == "duration")
						m_duration = aChild->GetInt32();
					else if (aChild->m_name == "ability")
						m_abilityId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aChild->GetIdentifier());
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*			aStream) const override
			{
				aStream->WriteInt(m_tickInterval);
				aStream->WriteInt(m_duration);
				aStream->WriteUInt(m_abilityId);
			}

			bool
			FromStream(
				IReader*			aStream) override
			{
				if (!aStream->ReadInt(m_tickInterval))
					return false;
				if (!aStream->ReadInt(m_duration))
					return false;
				if (!aStream->ReadUInt(m_abilityId))
					return false;
				return true;
			}

			// Public data
			int32_t			m_tickInterval = 5;
			int32_t			m_duration = 10;
			uint32_t		m_abilityId = 0;

			// Not serialized
			int32_t			m_lastUpdateTick = 0;
			int32_t			m_despawnTick = 0;
		};
	}

}