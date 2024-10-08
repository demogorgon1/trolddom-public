#include "Pcheader.h"

#include <map>

#include <tpublic/DataErrorHandling.h>
#include <tpublic/PersistentIdTable.h>

namespace tpublic
{

	PersistentIdTable::PersistentIdTable()
	{
		for (uint8_t i = 1; i < (uint8_t)DataType::NUM_IDS; i++)
			m_nextId[i] = 1;
	}
	
	PersistentIdTable::~PersistentIdTable()
	{

	}

	uint32_t		
	PersistentIdTable::GetId(
		DataType::Id		aDataType,
		const char*			aName)
	{
		TP_CHECK(aDataType != DataType::INVALID_ID, "Invalid data type.");
		std::unordered_map<std::string, uint32_t>::iterator it = m_tables[aDataType].find(aName);
		if(it == m_tables[aDataType].end())
		{
			uint32_t id = m_nextId[aDataType]++;
			m_tables[aDataType].insert(std::pair<std::string, uint32_t>(aName, id));
			return id;
		}
		
		return it->second;
	}

	void			
	PersistentIdTable::Load(
		const char*			aPath)
	{
		m_path = aPath;

		FILE* f = fopen(aPath, "rb");
		if (f != NULL)
		{
			try
			{
				char line[1024];
				uint32_t lineNum = 1;
				while (feof(f) == 0 && fgets(line, sizeof(line), f) != NULL)
				{
					if (line[0] != '#')
					{
						std::stringstream tokenizer(line);
						std::string token;
						std::vector<std::string> tokens;
						while (std::getline(tokenizer, token, ' '))
							tokens.push_back(token);

						TP_CHECK(tokens.size() == 3, "Syntax error in persistent id table file at line %u.", lineNum);

						DataType::Id dataType = DataType::StringToId(tokens[0].c_str());
						const char* name = tokens[1].c_str();
						uint32_t id = strtoul(tokens[2].c_str(), NULL, 10);

						if(id + 1 > m_nextId[dataType])
							m_nextId[dataType] = id + 1;

						m_tables[dataType][name] = id;
					}

					lineNum++;
				}

				fclose(f);
			}
			catch (...)
			{
				fclose(f);
				TP_CHECK(false, "Failed to load persistent id table: %s", aPath);
			}
		}
	}

	void			
	PersistentIdTable::Save()
	{
		char tmpPath[1024];
		TP_STRING_FORMAT(tmpPath, sizeof(tmpPath), "%s.tmp", m_path.c_str());

		FILE* f = fopen(tmpPath, "wb");
		TP_CHECK(f != NULL, "Failed to open file for output: %s", tmpPath);

		try
		{
			fprintf(f, "# Do not modify this file!\r\n");

			for(uint8_t i = 1; i < (uint8_t)DataType::NUM_IDS; i++)
			{
				const char* dataTypeString = DataType::IdToString((DataType::Id)i);

				// Sort by id
				typedef std::map<uint32_t, std::pair<std::string, std::string>> SortedMap;
				SortedMap sortedMap;

				for(std::pair<std::string, uint32_t> it : m_tables[i])
					sortedMap[it.second] = std::make_pair<std::string, std::string>(dataTypeString, it.first.c_str());

				for(SortedMap::const_iterator j = sortedMap.cbegin(); j != sortedMap.cend(); j++)
					fprintf(f, "%s %s %u\r\n", j->second.first.c_str(), j->second.second.c_str(), j->first);
			}

			fclose(f);

			for(uint32_t i = 0; i < 10; i++)
			{
				std::error_code ec;
				std::filesystem::rename(tmpPath, m_path, ec);
				if(!ec)
					break;

				printf("failed, will try again\n");
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
		catch(std::exception& e)
		{
			fclose(f);
			TP_CHECK(false, "Failed to save persistent id table: %s (%s)", tmpPath, e.what());
		}
	}

}