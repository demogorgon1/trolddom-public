#pragma once

#include "DataType.h"

namespace tpublic
{

	class PersistentIdTable
	{
	public:
						PersistentIdTable();
						~PersistentIdTable();

		uint32_t		GetId(
							DataType::Id		aDataType,
							const char*			aName);
		void			Load(
							const char*			aPath);
		void			Save();

	private:

		std::string									m_path;
		std::unordered_map<std::string, uint32_t>	m_tables[DataType::NUM_IDS];
		uint32_t									m_nextId[DataType::NUM_IDS];
	};

}