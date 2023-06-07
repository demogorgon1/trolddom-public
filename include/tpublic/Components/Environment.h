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
			}

			bool
			FromStream(
				IReader*			aStream) override
			{
				if (!aStream->ReadInt(m_tickInterval))
					return false;
				if (!aStream->ReadInt(m_duration))
					return false;
				return true;
			}

			// Public data
			int32_t			m_tickInterval = 5;
			int32_t			m_duration = 10;
			int32_t			m_lastUpdateTick = 0;
			int32_t			m_destroyTick = 0;
		};
	}

}