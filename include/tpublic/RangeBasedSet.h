#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	template <typename _T>
	class RangeBasedSet
	{
	public:
		bool
		HasValue(
			_T			aValue) const
		{
			size_t searchMin = 0;
			size_t searchMax = m_array.size();

			while (searchMax != searchMin)
			{
				assert(searchMax > searchMin);
				size_t i = (searchMin + searchMax) / 2;
				assert(i < m_array.size());
				const Range& range = m_array[i];

				if (aValue >= range.m_min && aValue <= range.m_max)
					return true;
					
				if (aValue < range.m_min)
					searchMax = i;
				else if (aValue > range.m_max)
					searchMin = i + 1;
			}

			return false;
		}

		bool
		Insert(
			_T			aValue)
		{
			if(m_array.empty())
			{
				m_array.push_back({ aValue, aValue });
			}
			else
			{
				size_t searchMin = 0;
				size_t searchMax = m_array.size();

				while(searchMax != searchMin)
				{
					assert(searchMax > searchMin);
					size_t i = (searchMin + searchMax) / 2;
					assert(i < m_array.size());
					Range& range = m_array[i];
					
					if(aValue >= range.m_min && aValue <= range.m_max)
					{
						return false; // Already in set
					}
					else if(aValue == range.m_min - 1)
					{
						range.m_min = aValue;

						// Do we need to merge range with the previous one?
						if(i > 0 && range.m_min == m_array[i - 1].m_max + 1)
						{
							range.m_min = m_array[i - 1].m_min;
							m_array.erase(m_array.begin() + i - 1);
						}
						return true;
					}
					else if (aValue == range.m_max + 1)
					{
						range.m_max = aValue;

						// Do we need to merge range with the next one?
						if (i < m_array.size() - 1 && range.m_max == m_array[i + 1].m_min - 1)
						{
							range.m_max = m_array[i + 1].m_max;
							m_array.erase(m_array.begin() + i + 1);
						}
						return true;
					}
					else if(aValue < range.m_min)
					{
						searchMax = i;
					}
					else if (aValue > range.m_max)
					{
						searchMin = i + 1;
					}
				}

				m_array.insert(m_array.begin() + searchMin, { aValue, aValue });
			}

			return true;
		}

		void
		Reset()
		{
			m_array.clear();
		}

		// Serialization
		void		ToStream(
						IWriter*			aWriter) const;
		bool		FromStream(
						IReader*			aReader);

	private:

		struct Range
		{
			// Public data
			_T					m_min = _T(0);
			_T					m_max = _T(0);
		};

		std::vector<Range>		m_array;
	};

	template <>
	inline void
	RangeBasedSet<uint32_t>::ToStream(
		IWriter*							aWriter) const
	{
		aWriter->WriteUInt(m_array.size());
		for(const Range& t : m_array)
		{
			aWriter->WriteUInt(t.m_min);
			aWriter->WriteUInt(t.m_max);
		}
	}

	template <>
	inline bool
	RangeBasedSet<uint32_t>::FromStream(
		IReader*							aReader) 
	{
		size_t count;
		if(!aReader->ReadUInt(count))
			return false;

		if(count > 10000)
			return false;

		m_array.resize(count);

		for(size_t i = 0; i < count; i++)
		{
			Range& t = m_array[i];
			if (!aReader->ReadUInt(t.m_min))
				return false;
			if (!aReader->ReadUInt(t.m_max))
				return false;
		}

		return true;
	}

}