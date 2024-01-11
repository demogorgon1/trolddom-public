#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	class PlayerWorship
	{
	public:
		void
		ToStream(
			IWriter*			aWriter) const
		{
			aWriter->WriteUInt(m_table.size());
			for(Table::const_iterator i = m_table.cbegin(); i != m_table.cend(); i++)
			{
				aWriter->WriteUInt(i->first);
				aWriter->WriteUInt(i->second);
			}
		}

		bool
		FromStream(
			IReader*			aReader)
		{
			size_t count;
			if(!aReader->ReadUInt(count))
				return false;

			for(size_t i = 0; i < count; i++)
			{
				uint32_t pantheonId;
				if (!aReader->ReadUInt(pantheonId))
					return false;

				uint32_t deityId;
				if (!aReader->ReadUInt(deityId))
					return false;

				m_table[pantheonId] = deityId;
			}
			return true;
		}

		void
		SetDeity(
			uint32_t			aPantheonId,
			uint32_t			aDeityId)
		{
			m_table[aPantheonId] = aDeityId;
		}

		uint32_t
		GetDeity(
			uint32_t			aPantheonId) const
		{
			Table::const_iterator i = m_table.find(aPantheonId);
			if(i == m_table.cend())
				return 0;
			return i->second;
		}

		void
		Clear(
			uint32_t			aPantheonId)
		{
			m_table.erase(aPantheonId);
		}

		// Public data
		typedef std::unordered_map<uint32_t, uint32_t> Table;
		Table				m_table;
	};

}