#pragma once

#include "EntityObjectiveEvent.h"
#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	namespace Components
	{
		struct Inventory;
	}

	class ObjectiveTypeBase;

	class ObjectiveInstanceBase
	{
	public:
		struct Progress
		{
			uint32_t									m_current = 0;
			uint32_t									m_goal = 0;
		};

		virtual						~ObjectiveInstanceBase() {}

		void		
		Init(
			const ObjectiveTypeBase*									aType,
			uint32_t													aCharacterId,
			uint32_t													aObjectiveId,
			std::shared_ptr<std::atomic_uint32_t>&						aUpdateSeq)
		{
			assert(m_type == NULL);
			assert(m_characterId == 0);
			assert(m_objectiveId == 0);
			m_type = aType;
			m_characterId = aCharacterId;
			m_objectiveId = aObjectiveId;
			m_updateSeq = aUpdateSeq;
		}

		void
		Detach()
		{
			m_detached = true;
		}

		// Virtual methods
		virtual void				OnEntityObjectiveEvent(
										EntityObjectiveEvent::Type		/*aEntityObjectiveEvent*/) {}
		virtual void				OnInventoryUpdate(
										const Components::Inventory*	/*aInventory*/) {}
		virtual bool				GetProgress(
										Progress&						/*aOut*/) { return false; }							

		// Virtual interface
		virtual bool				IsCompleted() const = 0;
		virtual uint32_t			GetHash() const = 0;
		virtual void				ToStream(
										IWriter*						aWriter) const = 0;
		virtual bool				FromStream(
										IReader*						aReader) = 0;

		// Data access
		uint32_t					GetCharacterId() const { assert(m_characterId != 0); return m_characterId; }
		uint32_t					GetObjectiveId() const { assert(m_objectiveId != 0); return m_objectiveId; }
		const ObjectiveTypeBase*	GetType() const { assert(m_type != NULL); return m_type; }
		bool						IsDetached() const { return m_detached; }

	protected:

		void			
		OnUpdate()
		{
			m_updateSeq.get()->operator++();
		}

	private:

		const ObjectiveTypeBase*						m_type = NULL;
		uint32_t										m_characterId = 0;
		uint32_t										m_objectiveId = 0;
		uint32_t										m_objectiveIndex = 0;
		std::atomic_bool								m_detached = false;
		std::shared_ptr<std::atomic_uint32_t>			m_updateSeq;
	};

}