#pragma once

#include "../Component.h"

namespace kpublic
{

	namespace Components
	{

		struct ThreatSource
			: public ComponentBase
		{			
			static const Component::Id ID = Component::ID_THREAT_SOURCE;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			ThreatSource()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~ThreatSource()
			{

			}

			void
			AddTarget(
				uint32_t			aEntityInstanceId,
				uint32_t			aTick)
			{
				m_targets[aEntityInstanceId] = aTick;
			}

			void
			RemoveTarget(
				uint32_t			aEntityInstanceId)
			{
				m_targets.erase(aEntityInstanceId);
			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* /*aStream*/) const override
			{
			}

			bool
			FromStream(
				IReader* /*aStream*/) override
			{
				return true;
			}

			// Public data
			std::unordered_map<uint32_t, uint32_t>			m_targets;
		};
	}

}