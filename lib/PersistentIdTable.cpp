#include "Pcheader.h"

#include <map>

#include <tpublic/DataErrorHandling.h>
#include <tpublic/PersistentIdTable.h>

namespace tpublic
{

	PersistentIdTable::PersistentIdTable()
	{
	}
	
	PersistentIdTable::~PersistentIdTable()
	{

	}

	uint32_t		
	PersistentIdTable::GetId(
		const DataErrorHandling::DebugInfo&	aDebugInfo,
		DataType::Id						aDataType,
		const char*							aName,
		bool								aIsDefinition)
	{
		TP_CHECK(DataType::ValidateId(aDataType), "Invalid data type.");
		Type& t = m_types[aDataType];

		uint32_t id = 0;

		{
			std::unordered_map<std::string, uint32_t>::iterator i = t.m_table.find(aName);
			if (i == t.m_table.end())
			{
				id = t.m_nextId++;
				t.m_table.insert(std::pair<std::string, uint32_t>(aName, id));
			}
			else
			{
				id = i->second;
			}
		}

		{
			ReferenceDebugInfo referenceDebugInfo = { aDebugInfo, aIsDefinition };

			std::unordered_map<uint32_t, std::vector<ReferenceDebugInfo>>::iterator i = t.m_debugInfoReferences.find(id);
			if(i == t.m_debugInfoReferences.cend())
				t.m_debugInfoReferences[id] = { referenceDebugInfo };
			else
				i->second.push_back(referenceDebugInfo);				
		}

		return id;
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
						TP_CHECK(DataType::ValidateId(dataType), "Invalid data type.");

						Type& t = m_types[dataType];

						const char* name = tokens[1].c_str();
						uint32_t id = strtoul(tokens[2].c_str(), NULL, 10);

						if(id + 1 > t.m_nextId)
							t.m_nextId = id + 1;

						t.m_table[name] = id;
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
				Type& t = m_types[i];

				// Sort by id
				typedef std::map<uint32_t, std::pair<std::string, std::string>> SortedMap;
				SortedMap sortedMap;

				for (std::pair<std::string, uint32_t> it : t.m_table)
					sortedMap[it.second] = std::make_pair<std::string, std::string>(dataTypeString, it.first.c_str());

				for (SortedMap::const_iterator j = sortedMap.cbegin(); j != sortedMap.cend(); j++)
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

	void			
	PersistentIdTable::ValidateAndPrune(
		UndefinedCallback												aUndefinedCallback)
	{
		for (uint8_t i = 1; i < (uint8_t)DataType::NUM_IDS; i++)
		{
			const char* dataTypeString = DataType::IdToString((DataType::Id)i);
			Type& t = m_types[i];

			uint32_t numPruned = 0;

			for(std::unordered_map<std::string, uint32_t>::const_iterator j = t.m_table.cbegin(); j != t.m_table.cend();)
			{
				const char* name = j->first.c_str();
				uint32_t id = j->second;

				std::unordered_map<uint32_t, std::vector<ReferenceDebugInfo>>::const_iterator k = t.m_debugInfoReferences.find(id);
				if (k != t.m_debugInfoReferences.cend())
				{
					bool isDefined = false;

					for(const ReferenceDebugInfo& reference : k->second)
					{
						if(reference.m_isDefinition)
						{
							isDefined = true;
							break;
						}
					}

					if(!isDefined)
					{
						for (const ReferenceDebugInfo& reference : k->second)
							aUndefinedCallback(dataTypeString, name, reference.m_debugInfo);
					}

					j++;
				}
				else
				{
					// No longer defined or referenced - can be pruned
					numPruned++;
					t.m_table.erase(j++);
				}
			}

			if(numPruned > 0)
				printf("Pruned %u unused %s identifier%s.\n", numPruned, dataTypeString, numPruned == 1 ? "" : "s");
		}
	}

}