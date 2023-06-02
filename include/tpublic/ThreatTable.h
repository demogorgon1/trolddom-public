#pragma once

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
		static const uint32_t TIMEOUT_TICKS = 100;

		struct Entry
		{
			uint32_t					m_entityInstanceId = 0;
			int32_t						m_threat = 0;
			uint32_t					m_tick = 0;
			Entry*						m_next = NULL;
			Entry*						m_prev = NULL;
		};

						ThreatTable();
						~ThreatTable();

		void			Update(
							uint32_t					aTick,
							std::vector<uint32_t>&		aOutRemoved);
		void			Add(
							uint32_t					aTick,
							uint32_t					aEntityInstanceId,
							int32_t						aThreat);
		void			Remove(
							uint32_t					aEntityInstanceId);
		void			Clear();

		// Data access
		bool			IsEmpty() const { return m_head == NULL; }
		const Entry*	GetTop() const { return m_head; }

	private:

		typedef std::unordered_map<uint32_t, Entry*> Table;
		Table							m_table;
		Entry*							m_head;
		Entry*							m_tail;

		Entry*	_FindHighestLessThan(
					int32_t						aThreat);
		void	_InsertAtEnd(
					Entry*						aEntry);
		void	_InsertBefore(
					Entry*						aEntry,
					Entry*						aInsertBefore);
		void	_Remove(
					Entry*						aEntry);
	};

}