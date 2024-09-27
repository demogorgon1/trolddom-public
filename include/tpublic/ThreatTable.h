#pragma once

#include "Hash.h"
#include "SourceEntityInstance.h"

namespace tpublic
{

	namespace Components
	{
		struct CombatPrivate;
	}

	class EntityInstance;

	class ThreatTable
	{
	public:
		static const int32_t TIMEOUT_TICKS = 100;

		struct Entry
		{
			SourceEntityInstance		m_key;
			int32_t						m_threat = 0;
			Entry*						m_next = NULL;
			Entry*						m_prev = NULL;
		};

						ThreatTable();
						~ThreatTable();

		void			Update(
							int32_t									aTick,
							std::vector<SourceEntityInstance>&		aOutRemoved);
		void			Add(
							int32_t									aTick,
							const SourceEntityInstance&				aSourceEntityInstance,
							int32_t									aThreat);
		void			Multiply(
							int32_t									aTick,
							const SourceEntityInstance&				aSourceEntityInstance,
							float									aFactor);
		void			MakeTop(
							int32_t									aTick,
							const SourceEntityInstance&				aSourceEntityInstance);
		void			Remove(
							const SourceEntityInstance&				aSourceEntityInstance);
		void			Touch(
							int32_t									aTick);
		void			Clear();
		void			DebugPrint() const;

		// Data access
		bool			IsEmpty() const { return m_head == NULL; }
		const Entry*	GetTop() const { return m_head; }
		int32_t			GetTick() const { return m_tick; }

	private:

		typedef std::unordered_map<SourceEntityInstance, Entry*, SourceEntityInstance::Hasher> Table;
		Table							m_table;
		Entry*							m_head;
		Entry*							m_tail;
		int32_t							m_tick;

		Entry*	_FindHighestLessThan(
					int32_t						aThreat);
		void	_InsertAtEnd(
					Entry*						aEntry);
		void	_InsertBefore(
					Entry*						aEntry,
					Entry*						aInsertBefore);
		void	_Remove(
					Entry*						aEntry);
		void	_Detach(
					Entry*						aEntry);
		void	_Add(
					Entry*						aEntry,
					int32_t						aThreat);
	};

}