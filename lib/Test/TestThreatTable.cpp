#include "../Pcheader.h"

#include <tpublic/DataErrorHandling.h>
#include <tpublic/ThreatTable.h>

namespace tpublic::Test
{
	
	namespace
	{
		
		void
		_Check(
			const ThreatTable&									aTable,
			const std::vector<std::pair<uint32_t, int32_t>>&	aExpected)
		{
			size_t i = 0;
			for(const ThreatTable::Entry* t = aTable.GetTop(); t != NULL; t = t->m_next)
			{
				TP_TEST(i < aExpected.size());
				TP_TEST(t->m_key.m_entityInstanceId == aExpected[i].first);
				TP_TEST(t->m_threat == aExpected[i].second);
				i++;
			}
			TP_TEST(i == aExpected.size());
		}

		void
		_CheckReference(
			const ThreatTable&									aTable,
			const std::map<uint32_t, int32_t>&					aReference)
		{		
			std::map<uint32_t, int32_t> table;
			for(const ThreatTable::Entry* t = aTable.GetTop(); t != NULL; t = t->m_next)
				table[t->m_key.m_entityInstanceId] = t->m_threat;

			TP_TEST(table == aReference);
		}

		void
		_MakeTopReference(
			uint32_t											aEntityInstanceId,
			std::map<uint32_t, int32_t>&						aReference)
		{
			if(aReference.size() == 0)
			{
				aReference[aEntityInstanceId] = 1;
			}
			else
			{
				int32_t maxThreat = 0;

				for (std::map<uint32_t, int32_t>::const_iterator i = aReference.cbegin(); i != aReference.cend(); i++)
				{
					if(i->second > maxThreat)
					{
						maxThreat = i->second;
					}
				}

				if(aReference[aEntityInstanceId] < maxThreat)
					aReference[aEntityInstanceId] = maxThreat + 1;
			}
		}
	}

	void
	TestThreatTable()
	{
		ThreatTable table;

		// Add some stuff
		{
			table.Add(0, { 1, 0 }, 100);
			table.Validate();
			_Check(table, { { 1, 100 } });

			table.Add(0, { 2, 0 }, 200);
			table.Validate();
			_Check(table, { { 2, 200 }, { 1, 100 } });

			table.Add(0, { 3, 0 }, 50);
			table.Validate();
			_Check(table, { { 2, 200 }, { 1, 100 }, { 3, 50 } });

			table.Add(0, { 4, 0 }, 150);
			table.Validate();
			_Check(table, { { 2, 200 }, { 4, 150 }, { 1, 100 }, { 3, 50 } });

		}

		// Remove it again
		{
			table.Remove({ 2, 0 });
			table.Validate();
			_Check(table, { { 4, 150 }, { 1, 100 }, { 3, 50 } });

			table.Remove({ 1, 0 });
			table.Validate();
			_Check(table, { { 4, 150 }, { 3, 50 } });

			table.Remove({ 3, 0 });
			table.Validate();
			_Check(table, { { 4, 150 } });

			table.Remove({ 4, 0 });
			table.Validate();
			_Check(table, { });
		}

		// Add and update
		{
			table.Add(0, { 1, 0 }, 100);
			table.Validate();
			_Check(table, { { 1, 100 } });

			table.Add(0, { 2, 0 }, 50);
			table.Validate();
			_Check(table, { { 1, 100 }, { 2, 50 } });

			table.Add(0, { 2, 0 }, 200);
			table.Validate();
			_Check(table, { { 2, 250 }, { 1, 100 } });

			table.Add(0, { 2, 0 }, -200);
			table.Validate();
			_Check(table, { { 1, 100 }, { 2, 50 } });

			table.MakeTop(0, { 2, 0 });
			table.Validate();
			_Check(table, { { 2, 101 }, { 1, 100 } });

			table.Multiply(0, { 1, 0 }, 3.0f);
			table.Validate();
			_Check(table, { { 1, 300 }, { 2, 101 } });
		}

		// Clear
		{
			table.Clear();
			table.Validate();
			_Check(table, { });
		}

		// Random stress test
		{
			std::map<uint32_t, int32_t> referenceTable;
			std::mt19937 random;

			for(uint32_t i = 0; i < 50000; i++)
			{
				//if(i % 1023 == 0)
				//	printf("%u\n", i);

				uint32_t action = random() % 100;
				uint32_t entityInstanceId = random() % 10;

				if(action < 8)
				{
					// Remove
					//printf("REMOVE %u\n", entityInstanceId);

					table.Remove({ entityInstanceId, 0 });
					table.Validate();

					referenceTable.erase(entityInstanceId);
					_CheckReference(table, referenceTable);
				}
				else if(action < 20)
				{
					// Subtract
					uint32_t threatToSubtract = 1 + random() % 50;
					//printf("SUB %u %d\n", entityInstanceId, -(int32_t)threatToSubtract);

					table.Add(0, { entityInstanceId, 0 }, -(int32_t)threatToSubtract);
					table.Validate();

					referenceTable[entityInstanceId] -= (int32_t)threatToSubtract;
					if(referenceTable[entityInstanceId] < 0)	
						referenceTable[entityInstanceId] = 0;

					_CheckReference(table, referenceTable);
				}
				else if (action < 30)
				{
					// Make top
					//printf("TOP %u\n", entityInstanceId);

					table.MakeTop(0, { entityInstanceId, 0 });
					table.Validate();

					_MakeTopReference(entityInstanceId, referenceTable);
					_CheckReference(table, referenceTable);
				}
				else if (action < 40)
				{
					// Multiply
					//printf("MULTIPLY %u 0.5\n", entityInstanceId);

					int32_t change = table.Multiply(0, { entityInstanceId, 0 }, 0.5f);
					table.Validate();

					if(change != INT32_MAX)
					{
						referenceTable[entityInstanceId] += change;
						_CheckReference(table, referenceTable);
					}
				}
				else
				{
					// Add
					uint32_t threatToAdd = 1 + random() % 100;
					//printf("ADD %u %u\n", entityInstanceId, threatToAdd);

					table.Add(0, { entityInstanceId, 0 }, (int32_t)threatToAdd);
					table.Validate();

					referenceTable[entityInstanceId] += (int32_t)threatToAdd;
					_CheckReference(table, referenceTable);
				}

				//table.DebugPrint();
			}
		}
	}

}