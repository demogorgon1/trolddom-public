#pragma once

#include "../Component.h"

namespace tpublic
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
				int32_t				aTick)
			{
				m_targets[aEntityInstanceId] = aTick;
			}

			void
			RemoveTarget(
				uint32_t			aEntityInstanceId)
			{
				m_targets.erase(aEntityInstanceId);
			}

			uint32_t
			GetFirstTarget() const 
			{
				std::unordered_map<uint32_t, int32_t>::const_iterator i = m_targets.cbegin();
				assert(i != m_targets.cend());
				return i->first;
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
			std::unordered_map<uint32_t, int32_t>			m_targets;
		};
	}

}