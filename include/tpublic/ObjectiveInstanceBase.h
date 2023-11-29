#pragma once

#include "EntityObjectiveEvent.h"
#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	class ObjectiveInstanceBase
	{
	public:
		virtual			~ObjectiveInstanceBase() {}

		void		
		Init(
			uint32_t									aPlayerEntityInstanceId,
			uint32_t									aObjectiveId)
		{
			assert(m_playerEntityInstanceId == 0);
			assert(m_objectiveId == 0);
			m_playerEntityInstanceId = aPlayerEntityInstanceId;
			m_objectiveId = aObjectiveId;
		}

		// Virtual methods
		virtual void	OnEntityObjectiveEvent(
							EntityObjectiveEvent::Type	/*aEntityObjectiveEvent*/) {}

		// Virtual interface
		virtual void	ToStream(
							IWriter*					aWriter) const = 0;
		virtual bool	FromStream(
							IReader*					aReader) = 0;

		// Data access
		uint32_t		GetPlayerEntityInstanceId() const { assert(m_playerEntityInstanceId != 0); return m_playerEntityInstanceId; }
		uint32_t		GetObjectiveId() const { assert(m_objectiveId != 0); return m_objectiveId; }

	private:

		uint32_t	m_playerEntityInstanceId = 0;
		uint32_t	m_objectiveId = 0;
	};

}