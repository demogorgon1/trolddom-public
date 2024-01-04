#pragma once

#include "Hash.h"
#include "IReader.h"
#include "IWriter.h"

namespace tpublic::MarkovChainText
{

	struct Key
	{
		struct Hasher
		{
			uint32_t
			operator()(
				const Key&																						aKey) const
			{
				return (uint32_t)Hash::Splitmix_2_32(aKey.m_order, aKey.m_hash);
			}
		};

		bool 
		operator==(
			const Key&																							aOther) const 
		{ 
			return m_order == aOther.m_order && m_hash == aOther.m_hash; 
		}

		void
		ToStream(
			IWriter*																							aWriter) const
		{
			aWriter->WriteUInt(m_order);
			aWriter->WriteUInt(m_hash);
		}

		bool
		FromStream(
			IReader*																							aReader) 
		{
			if(!aReader->ReadUInt(m_order))
				return false;
			if(!aReader->ReadUInt(m_hash))
				return false;
			return true;
		}

		// Public data
		uint32_t						m_order;
		uint32_t						m_hash;
	};

	class Source
	{
	public:
		static const uint32_t HASH_SALT = 0x8429A591;

		struct CharacterCounter
		{
			CharacterCounter()
			{

			}

			void
			AddCharacter(
				uint32_t																						aCharacterCode)
			{
				std::unordered_map<uint32_t, uint32_t>::iterator i = m_table.find(aCharacterCode);
				if(i != m_table.end())
					i->second++;
				else
					m_table[aCharacterCode] = 1;
			}

			void
			DebugPrint() const
			{
				for(std::unordered_map<uint32_t, uint32_t>::const_iterator i = m_table.cbegin(); i != m_table.cend(); i++)
					printf("'%c' : %u\n", i->first, i->second);
			}

			void
			ForEach(
				std::function<void(uint32_t, uint32_t)>															aCallback) const
			{
				for (std::unordered_map<uint32_t, uint32_t>::const_iterator i = m_table.cbegin(); i != m_table.cend(); i++)
					aCallback(i->first, i->second);
			}

			// Public data
			std::unordered_map<uint32_t, uint32_t>	m_table;
		};

											Source(
												uint32_t														aOrder,
												bool															aExclude);
											~Source();

		void								AddText(
												const char*														aText);
		void								ForEachKey(
												std::function<void(const Key&, const CharacterCounter*)>		aCallback) const;

		// Data access
		uint32_t							GetOrder() const { return m_order; }
		const CharacterCounter*				GetRoot() const { return &m_root; }
		const std::unordered_set<uint32_t>&	GetSourceHashes() const { return m_sourceHashes; }
		bool								GetExclude() const { return m_exclude; }

	private:

		uint32_t									m_order;
		
		typedef std::unordered_map<Key, std::unique_ptr<CharacterCounter>, Key::Hasher> Table;
		Table										m_table;
		CharacterCounter							m_root;
		std::unordered_set<uint32_t>				m_sourceHashes;
		bool										m_exclude;
	};

	class Generator
	{
	public:
								Generator();
								~Generator();

		void					AddSource(
									const Source*												aSource);
		void					SetOrder(
									uint32_t													aOrder);
		void					CreateText(
									std::mt19937&												aRandom,
									std::string&												aOut) const;
		void					ToStream(
									IWriter*													aWriter) const;
		bool					FromStream(
									IReader*													aReader);
		bool					IsSourceText(
									const char*													aText) const;

	private:

		uint32_t									m_order;

		struct WeightedRandomEntry
		{
			void
			ToStream(
				IWriter*																		aWriter) const
			{
				aWriter->WriteUInt(m_character);
				aWriter->WriteUInt(m_weight);
			}

			bool
			FromStream(
				IReader*																		aReader) 
			{
				if(!aReader->ReadUInt(m_character))
					return false;
				if(!aReader->ReadUInt(m_weight))
					return false;
				return true;
			}

			// Public data
			uint32_t								m_character = 0;
			uint32_t								m_weight = 0;
		};

		struct WeightedRandom
		{
			void
			AddPossibility(
				uint32_t																		aWeight,
				uint32_t																		aCharacter)
			{
				m_entries.push_back({ aCharacter, aWeight });
				m_totalWeight += aWeight;
			}

			uint32_t
			Get(
				std::mt19937&																	aRandom) const
			{
				std::uniform_int_distribution<uint32_t> distribution(1, m_totalWeight);
				uint32_t roll = distribution(aRandom);

				// FIXME: sort entries and do binary search
				uint32_t sum = 0;
				for(const WeightedRandomEntry& t : m_entries)
				{
					sum += t.m_weight;

					if(roll <= sum)
						return t.m_character;
				}

				assert(false);
				return 0;
			}

			void
			ToStream(
				IWriter*																		aWriter) const
			{
				aWriter->WriteUInt(m_totalWeight);
				aWriter->WriteObjects(m_entries);
			}

			bool
			FromStream(
				IReader*																		aReader) 
			{
				if(!aReader->ReadUInt(m_totalWeight))
					return false;
				if(!aReader->ReadObjects(m_entries))
					return false;
				return true;
			}

			// Public data
			uint32_t								m_totalWeight = 0;
			std::vector<WeightedRandomEntry>		m_entries;
		};

		typedef std::unordered_map<Key, std::unique_ptr<WeightedRandom>, Key::Hasher> Table;
		Table										m_table;	
		WeightedRandom								m_root;
		std::unordered_set<uint32_t>				m_sourceHashes;
	};

}

