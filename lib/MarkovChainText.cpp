#include "Pcheader.h"

#include <tpublic/MarkovChainText.h>
#include <tpublic/UTF8.h>

namespace tpublic::MarkovChainText
{

	namespace
	{

		template <uint32_t _MaxOrder>
		struct Hasher
		{
			Hasher(
				uint32_t							aOrder)
				: m_offset(0)
				, m_count(0)
				, m_order(aOrder)
			{
				assert(aOrder <= _MaxOrder);
			}

			void
			AddCharacter(
				uint32_t							aCharacterCode)
			{
				m_buffer[m_offset] = aCharacterCode;
				m_offset = (m_offset + 1) % m_order;
				if(m_count < m_order)
					m_count++;					
			}

			void
			Calculate(
				std::function<bool(uint32_t, uint32_t)>	aCallback) const
			{
				uint32_t offset = m_offset;

				Hash::CheckSum hash;

				uint32_t hashBuffer[_MaxOrder];

				for(uint32_t i = 0; i < m_count; i++)
				{
					if(offset == 0)
						offset = m_order - 1;
					else 
						offset--;

					hash.AddData(&m_buffer[offset], sizeof(uint32_t));

					hashBuffer[i] = hash.m_hash;
				}

				for(uint32_t i = 0; i < m_count; i++)
				{
					bool done = aCallback(m_count - i, hashBuffer[i]);
					if(done)
						break;
				}
			}

			// Public data
			uint32_t			m_order;
			uint32_t			m_buffer[_MaxOrder];
			uint32_t			m_offset;
			uint32_t			m_count;
		};

	}

	//---------------------------------------------------------------------------------------------

	Source::Source(
		uint32_t		aOrder,
		bool			aExclude)
		: m_order(aOrder)
		, m_exclude(aExclude)
	{

	}

	Source::~Source()
	{

	}

	void	
	Source::AddText(
		const char*													aText)
	{
		// Add text
		{
			Hasher<16> hasher(m_order);

			UTF8::Decoder utf8(aText);
			utf8.SetEmitNullTermination(true);

			uint32_t characterCode;
			while (utf8.GetNextCharacterCode(characterCode))
			{
				if (hasher.m_count == 0)
				{
					m_root.AddCharacter(characterCode);
				}
				else
				{
					hasher.Calculate([&](
						uint32_t aOrder,
						uint32_t aHash) -> bool
					{
						Key key{ aOrder, aHash };

						CharacterCounter* characterCounter;
						Table::iterator i = m_table.find(key);
						if(i != m_table.end())
						{
							characterCounter = i->second.get();
						}
						else
						{
							characterCounter = new CharacterCounter();
							m_table[key] = std::unique_ptr<CharacterCounter>();
						}

						characterCounter->AddCharacter(characterCode);

						return false;
					});
				}

				hasher.AddCharacter(characterCode);
			}
		}

		// Add hash of text to exclude list
		if(m_exclude)
		{
			Hash::CheckSum hash;
			hash.AddData(aText, strlen(aText));
			hash.AddData(&HASH_SALT, sizeof(HASH_SALT));
			m_sourceHashes.insert(hash.m_hash);
		}
	}

	void		
	Source::ForEachKey(
		std::function<void(const Key&, const CharacterCounter*)>	aCallback) const
	{
		for(Table::const_iterator i = m_table.cbegin(); i != m_table.cend(); i++)
			aCallback(i->first, i->second.get());
	}

	//---------------------------------------------------------------------------------------------

	Generator::Generator()
		: m_order(1)
	{

	}

	Generator::~Generator()
	{

	}

	void	
	Generator::AddSource(
		const Source*	aSource)
	{
		aSource->ForEachKey([&](
			const Key&						aKey,
			const Source::CharacterCounter*	aCharacterCounter)
		{
			Table::iterator i = m_table.find(aKey);
			WeightedRandom* weightedRandom;
			if(i != m_table.end())
			{
				weightedRandom = i->second.get();
			}
			else
			{
				weightedRandom = new WeightedRandom();
				m_table[aKey] = std::unique_ptr<WeightedRandom>(weightedRandom);
			}

			aCharacterCounter->ForEach([&](
				uint32_t					aCharacterCode,
				uint32_t					aCount)
			{
				weightedRandom->AddPossibility(aCount, aCharacterCode);
			});
		});

		aSource->GetRoot()->ForEach([&](
			uint32_t						aCharacterCode,
			uint32_t						aCount)
		{
			m_root.AddPossibility(aCount, aCharacterCode);
		});

		for(uint32_t hash : aSource->GetSourceHashes())
			m_sourceHashes.insert(hash);
	}

	void					
	Generator::SetOrder(
		uint32_t		aOrder)
	{
		m_order = aOrder;
	}

	void	
	Generator::CreateText(
		std::mt19937&	aRandom,
		std::string&	aOut) const
	{
		if(m_root.m_entries.empty())
			return;

		UTF8::Encoder utf8;
		Hasher<16> hasher(m_order);

		// Initial character
		{
			uint32_t firstCharacter = m_root.Get(aRandom);

			utf8.EncodeCharacter(firstCharacter);
			hasher.AddCharacter(firstCharacter);
		}

		// Remaining characters
		for(;;)
		{			
			uint32_t nextCharacter = UINT32_MAX;

			hasher.Calculate([&](
				uint32_t aOrder,
				uint32_t aHash) -> bool
			{
				Table::const_iterator i = m_table.find(Key{ aOrder, aHash });
				if(i == m_table.end())
					return false;

				nextCharacter = i->second->Get(aRandom);
				return true;
			});
				
			if(nextCharacter == UINT32_MAX || nextCharacter == 0)
				break;

			utf8.EncodeCharacter(nextCharacter);
			hasher.AddCharacter(nextCharacter);
		}

		utf8.Finalize();

		aOut = utf8.GetBuffer();
	}

	void					
	Generator::ToStream(
		IWriter*		aWriter) const
	{
		aWriter->WriteUInt(m_order);
		m_root.ToStream(aWriter);

		aWriter->WriteUInt(m_table.size());
		for(Table::const_iterator i = m_table.cbegin(); i != m_table.cend(); i++)
		{
			i->first.ToStream(aWriter);
			i->second->ToStream(aWriter);
		}

		aWriter->WriteUInt(m_sourceHashes.size());
		for(uint32_t sourceHash : m_sourceHashes)
			aWriter->WritePOD(sourceHash);
	}

	bool
	Generator::FromStream(
		IReader*		aReader) 
	{
		if(!aReader->ReadUInt(m_order))
			return false;
		if(!m_root.FromStream(aReader))
			return false;

		{
			size_t count;
			if(!aReader->ReadUInt(count))
				return false;

			for(size_t i = 0; i < count; i++)
			{
				Key key;
				if(!key.FromStream(aReader))
					return false;

				std::unique_ptr<WeightedRandom> t = std::make_unique<WeightedRandom>();
				if(!t->FromStream(aReader))
					return false;

				m_table[key] = std::move(t);
			}
		}

		{
			size_t count;
			if (!aReader->ReadUInt(count))
				return false;

			for (size_t i = 0; i < count; i++)
			{
				uint32_t hash;
				if(!aReader->ReadUInt(hash))
					return false;

				m_sourceHashes.insert(hash);
			}
		}

		return true;
	}

	bool					
	Generator::IsSourceText(
		const char*		aText) const
	{
		Hash::CheckSum hash;
		hash.AddData(aText, strlen(aText));
		hash.AddData(&Source::HASH_SALT, sizeof(Source::HASH_SALT));
		return m_sourceHashes.contains(hash.m_hash);
	}

}