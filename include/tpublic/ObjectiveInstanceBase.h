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
			uint32_t									aCharacterId,
			uint32_t									aObjectiveId)
		{
			assert(m_characterId == 0);
			assert(m_objectiveId == 0);
			m_characterId = aCharacterId;
			m_objectiveId = aObjectiveId;
		}

		void
		Detach()
		{
			m_detached = true;
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
		uint32_t		GetCharacterId() const { assert(m_characterId != 0); return m_characterId; }
		uint32_t		GetObjectiveId() const { assert(m_objectiveId != 0); return m_objectiveId; }
		bool			IsDetached() const { return m_detached; }

	private:

		uint32_t			m_characterId = 0;
		uint32_t			m_objectiveId = 0;
		std::atomic_bool	m_detached;
	};

}