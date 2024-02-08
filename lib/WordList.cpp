#include "Pcheader.h"

#include <tpublic/Data/Tag.h>

#include <tpublic/DataErrorHandling.h>
#include <tpublic/Helpers.h>
#include <tpublic/Manifest.h>
#include <tpublic/UTF8.h>
#include <tpublic/WordList.h>

namespace tpublic::WordList
{

	void 
	Data::FromSource(
		const SourceNode*	aSource,
		Data*				aData)
	{
		std::vector<uint32_t> tagIds;
		std::vector<std::string> strings;
		bool capitalize = false;
		uint8_t flags = 0;

		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "import")
			{
				std::string path = aChild->m_realPath;
				path += "/";
				path += aChild->GetString();

				std::vector<std::string> lines;
				bool ok = Helpers::LoadTextFile(path.c_str(), lines);
				TP_VERIFY(ok, aChild->m_debugInfo, "Failed to load text file.");

				for(std::string& line : lines)
				{
					Helpers::TrimString(line);
					if(!line.empty())
						strings.push_back(line);
				}				
			}
			else if(aChild->m_name == "strings")
			{
				aChild->GetArray()->ForEachChild([&](
					const SourceNode* aString)
				{
					strings.push_back(aString->GetString());
				});
			}
			else if(aChild->m_name == "tags")
			{
				aChild->GetIdArray(DataType::ID_TAG, tagIds);
			}						
			else if(aChild->m_name == "capitalize")
			{
				capitalize = aChild->GetBool();
			}
			else if(aChild->m_name == "hidden")
			{
				flags |= Word::FLAG_HIDDEN;
			}
			else 
			{
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			}
		});

		for(const std::string& t : strings)
		{
			if(!capitalize)
			{
				aData->AddWord(t.c_str(), flags, tagIds);
			}
			else
			{
				UTF8::Decoder in(t.c_str()); 
				UTF8::Encoder out;

				bool first = true;
				uint32_t character;
				while(in.GetNextCharacterCode(character))
				{					
					if(first)
					{
						character = (uint32_t)toupper((int)character);
						first = false;
					}
					else
					{
						character = (uint32_t)tolower((int)character);
					}

					out.EncodeCharacter(character);
				}

				aData->AddWord(out.Finalize(), flags, tagIds);
			}
		}
	}

	void	
	Data::ToStream(
		IWriter*		aWriter) const
	{
		// We have to transform the word list into a version with hidden words removed. All the absolute indices will need to be updated, obviously.
		struct OutputWord
		{
			const Word*	m_word;
			uint32_t	m_newIndex;
		};

		typedef std::unordered_map<uint32_t, std::unique_ptr<OutputWord>> OutputWordTable;
		OutputWordTable outputWordTable;
		std::vector<const OutputWord*> outputWordArray;

		aWriter->WriteUInt(m_tagTable.size());
	
		for(TagTable::const_iterator i = m_tagTable.cbegin(); i != m_tagTable.cend(); i++)
		{
			const Tag* tag = i->second.get();

			aWriter->WriteUInt(tag->m_id);
			aWriter->WritePOD(tag->m_flags);
			
			for(uint32_t wordIndex : tag->m_words)
			{
				const Word* word = GetWordByIndex(wordIndex);
				if((word->m_flags & Word::FLAG_HIDDEN) == 0)
				{
					OutputWordTable::const_iterator j = outputWordTable.find(wordIndex);
					uint32_t index = 0;
					if(j != outputWordTable.cend())
					{
						index = j->second->m_newIndex;
					}
					else
					{
						OutputWord* outputWord = new OutputWord();
						outputWordTable[wordIndex] = std::unique_ptr<OutputWord>(outputWord);
						outputWord->m_newIndex = (uint32_t)outputWordArray.size();
						outputWord->m_word = word;
						outputWordArray.push_back(outputWord);

						index = outputWord->m_newIndex;
					}

					aWriter->WriteUInt(index + 1); // 1-based index because 0 marks end of list
				}
			}

			aWriter->WriteUInt(0); // Marks end
		}

		aWriter->WriteUInt(outputWordArray.size());
		for(const OutputWord* outputWord : outputWordArray)
			outputWord->m_word->ToStream(aWriter);
	}
		
	bool
	Data::FromStream(
		IReader*		aReader)
	{
		{
			size_t count;
			if(!aReader->ReadUInt(count))
				return false;

			for(size_t i = 0; i < count; i++)
			{
				std::unique_ptr<Tag> tag = std::make_unique<Tag>();
				if(!aReader->ReadUInt(tag->m_id))
					return false;

				if(!aReader->ReadPOD(tag->m_flags))
					return false;

				for(;;)
				{
					uint32_t wordIndex; 
					if(!aReader->ReadUInt(wordIndex))
						 return false;

					if(wordIndex == 0)
						break;

					tag->m_words.insert(wordIndex - 1); // 1-based to 0-based index
				}

				uint32_t tagId = tag->m_id;
				m_tagTable[tagId] = std::move(tag);
			}
		}

		{
			size_t count;
			if (!aReader->ReadUInt(count))
				return false;

			DataChunk* dataChunk = NULL;

			for (size_t i = 0; i < count; i++)
			{
				if(dataChunk == NULL || dataChunk->m_numWords == DataChunk::SIZE)
					m_dataChunks.push_back(std::unique_ptr<DataChunk>(dataChunk = new DataChunk()));

				if(!dataChunk->m_words[dataChunk->m_numWords++].FromStream(aReader))
					return false;
			}
		}

		return true;
	}

	void		
	Data::AddWord(
		const char*						aWord,
		uint8_t							aFlags,
		const std::vector<uint32_t>&	aTags)
	{
		uint32_t index = 0;
		Word* word = NULL;

		// Add word to list
		{
			if (m_dataChunks.size() > 0)
				index = (uint32_t)((m_dataChunks.size() - 1) * DataChunk::SIZE) + (uint32_t)m_dataChunks[m_dataChunks.size() - 1]->m_numWords;
			else
				index = 0;

			if (m_dataChunks.size() == 0 || m_dataChunks[m_dataChunks.size() - 1]->m_numWords == DataChunk::SIZE)
				m_dataChunks.push_back(std::make_unique<DataChunk>());

			word = m_dataChunks[m_dataChunks.size() - 1]->AllocateWord();

			word->m_word = aWord;
			word->m_flags = aFlags;
			word->m_allTags = aTags;
		}

		// Add to tag index
		for(uint32_t tagId : aTags)
		{
			Tag* tag;
			TagTable::iterator i = m_tagTable.find(tagId);
			if(i != m_tagTable.end())
			{
				tag = i->second.get();
			}
			else
			{
				tag = new Tag();
				tag->m_id = tagId;
				m_tagTable[tagId] = std::unique_ptr<Tag>(tag);
			}

			tag->m_words.insert(index);

			//if(tag->m_flags & Tag::FLAG_EXPLICIT)
			//	word->m_explicitTags.Add(tagId);
		}
	}

	//void		
	//Manifest::MakeTagExplicit(
	//	nc::UInt32						aTagId)
	//{
	//	Tag* tag;
	//	if (!m_tagTable.Get(aTagId, tag))
	//		tag = m_tagTable.Set(aTagId, new Tag(aTagId));

	//	tag->m_flags |= Tag::FLAG_EXPLICIT;
	//}

	//void		
	//Manifest::MakeTagsMutuallyExclusive(
	//	const nc::IArray<nc::UInt32>&	aTags)
	//{
	//	for(nc::UInt32 tagId : aTags)
	//	{
	//		Tag* tag;
	//		if (!m_tagTable.Get(tagId, tag))
	//			tag = m_tagTable.Set(tagId, new Tag(tagId));

	//		for (nc::UInt32 otherTagId : aTags)
	//		{
	//			if(otherTagId != tagId)
	//				tag->m_mutuallyExclusive.Add(otherTagId);
	//		}
	//	}
	//}

	const Word* 
	Data::GetWordByIndex(
		uint32_t						aIndex) const
	{
		size_t i = (size_t)aIndex / DataChunk::SIZE;
		size_t j = (size_t)aIndex % DataChunk::SIZE;
		TP_CHECK(i < m_dataChunks.size() && j < m_dataChunks[i]->m_numWords, "Word index out of bounds.");
		return &m_dataChunks[i]->m_words[j];
	}

	Word* 
	Data::GetWordByIndex(
		uint32_t						aIndex) 
	{
		size_t i = (size_t)aIndex / DataChunk::SIZE;
		size_t j = (size_t)aIndex % DataChunk::SIZE;
		TP_CHECK(i < m_dataChunks.size() && j < m_dataChunks[i]->m_numWords, "Word index out of bounds.");
		return &m_dataChunks[i]->m_words[j];
	}

	const Tag* 
	Data::TryGetTag(
		uint32_t						aTagId) const
	{
		TagTable::const_iterator i = m_tagTable.find(aTagId);
		if(i == m_tagTable.cend())
			return NULL;
		return i->second.get();
	}

	void		
	Data::Prepare(
		const Manifest*					aManifest)
	{
		for(TagTable::iterator i = m_tagTable.begin(); i != m_tagTable.end(); i++)
		{
			Tag* tag = i->second.get();
		
			const tpublic::Data::Tag* tagData = aManifest->GetById<tpublic::Data::Tag>(tag->m_id);
			if(tagData->m_excplicit)
			{
				tag->m_flags |= Tag::FLAG_EXPLICIT;
			}

			if(tagData->m_mutuallyExclusive.size() > 0)
			{
				for(uint32_t wordIndex : tag->m_words)
				{
					Word* word = GetWordByIndex(wordIndex);
					word->m_explicitTags.push_back(tag->m_id);
				}
			}

			if(tagData->m_randomAssociatedStatWeights)
			{
				for (uint32_t wordIndex : tag->m_words)
				{
					Word* word = GetWordByIndex(wordIndex);
					uint32_t wordHash = Hash::String(word->m_word.c_str());
					
					if(!word->m_associatedStatWeights)
						word->m_associatedStatWeights = std::make_unique<Stat::Collection>();

					Helpers::GetRandomStatWeights(wordHash, *word->m_associatedStatWeights);
				}
			}

			if(tagData->m_associatedStatWeights)
			{
				for (uint32_t wordIndex : tag->m_words)
				{
					Word* word = GetWordByIndex(wordIndex);

					if (!word->m_associatedStatWeights)
						word->m_associatedStatWeights = std::make_unique<Stat::Collection>();

					word->m_associatedStatWeights->Add(*tagData->m_associatedStatWeights);
				}
			}
		}
	}

	//----------------------------------------------------------------------------

	QueryCache::QueryCache(
		const Data* aData)
		: m_data(aData)
	{

	}
		
	QueryCache::~QueryCache()
	{

	}

	const QueryCache::Query*
	QueryCache::PerformQuery(
		const QueryParams&				aParams)
	{
		uint32_t hash = aParams.GetHash();

		QueryBucket* queryBucket;
		QueryBucketTable::iterator i = m_queryBucketTable.find(hash);
		if(i != m_queryBucketTable.end())
		{
			queryBucket = i->second.get();
		}
		else
		{
			queryBucket = new QueryBucket();
			m_queryBucketTable[hash] = std::unique_ptr<QueryBucket>(queryBucket);
		}

		const Query* query = queryBucket->TryGetQueryByParams(aParams);

		if(query == NULL)
		{
			queryBucket->m_queries.push_back(std::unique_ptr<Query>(_NewQuery(aParams)));
			query = queryBucket->m_queries[queryBucket->m_queries.size() - 1].get();
		}

		return query;
	}

	QueryCache::Query* 
	QueryCache::_NewQuery(
		const QueryParams&				aParams)
	{
		std::unique_ptr<Query> query = std::make_unique<Query>(aParams);

		std::unordered_map<uint32_t, uint32_t> result;

		// Get all words with all must have tags
		if(aParams.m_mustHaveTags.size() > 0)
		{
			bool firstTag = true;

			for (uint32_t tagId : aParams.m_mustHaveTags)
			{
				const Tag* tag = m_data->TryGetTag(tagId);

				if (tag == NULL)
				{
					result.clear();
					break;
				}

				uint32_t tagScore = aParams.GetTagScore(tagId);

				if (firstTag)
				{
					for(uint32_t wordIndex : tag->m_words)
					{
						std::unordered_map<uint32_t, uint32_t>::iterator i = result.find(wordIndex);
						if(i != result.end())
							i->second += tagScore;
						else
							result[wordIndex] = tagScore;
					}
				}
				else
				{
					for(std::unordered_map<uint32_t, uint32_t>::iterator i = result.begin(); i != result.end();)
					{
						if(tag->m_words.contains(i->first))
						{
							i->second += tagScore;
							i++;
						}
						else
						{
							result.erase(i++);
						}
					}
				}

				firstTag = false;
			}
		}
		else
		{
			// No must-have-tags specified, include all tags
			bool firstTag = true;

			for(Data::TagTable::const_iterator allTagsIterator = m_data->m_tagTable.cbegin(); allTagsIterator != m_data->m_tagTable.cend(); allTagsIterator++)
			{
				const Tag* tag = allTagsIterator->second.get();

				uint32_t tagScore = aParams.GetTagScore(tag->m_id);

				if(tagScore > 0)
				{
					if (firstTag)
					{
						for (uint32_t wordIndex : tag->m_words)
						{
							std::unordered_map<uint32_t, uint32_t>::iterator i = result.find(wordIndex);
							if (i != result.end())
								i->second += tagScore;
							else
								result[wordIndex] = tagScore;
						}
					}
					else
					{
						for(std::unordered_map<uint32_t, uint32_t>::iterator i = result.begin(); i != result.end(); i++)
						{
							if(tag->m_words.contains(i->first))
								i->second += tagScore;
						}
					}
	
					firstTag = false;
				}
			}
		}

		// Remove words with must not have tags
		{
			std::vector<uint32_t> excludeTags;
			excludeTags = aParams.m_mustNotHaveTags;
					
			//aParams.ForEachTag([&](
			//	uint32_t aTagId)
			//{
			//	const Tag* tag = m_manifest->TryGetTag(aTagId);
			//	assert(tag != NULL);

			//	//excludeTags.Add(tag->m_mutuallyExclusive);
			//});

			for (uint32_t tagId : excludeTags)
			{
				const Tag* tag = m_data->TryGetTag(tagId);
				if (tag != NULL)
				{
					for(uint32_t wordIndex : tag->m_words)
						result.erase(wordIndex);
				}
			}
		}
			
		// Create results
		{
			std::vector<const ResultBucket*> sortedResultBucketArray;

			for(std::unordered_map<uint32_t, uint32_t>::const_iterator i = result.cbegin(); i != result.cend(); i++)
			{
				uint32_t wordIndex = i->first;
				uint32_t score = i->second;

				const Word* word = m_data->GetWordByIndex(wordIndex);

				for(uint32_t tagId : word->m_explicitTags)
				{
					if(!aParams.HasTag(tagId))
					{
						query->m_resultBuckets.Clear();
						query->m_resultBucketTable.clear();
						return query.release();
					}
				}

				ResultBucket* resultBucket = NULL;
				Query::ResultBucketTable::iterator j = query->m_resultBucketTable.find(score);

				if(j == query->m_resultBucketTable.end())
				{
					resultBucket = new ResultBucket(score);
					query->m_resultBucketTable[score] = std::unique_ptr<ResultBucket>(resultBucket);					
				}
				else
				{
					resultBucket = j->second.get();
				}

				resultBucket->m_words.push_back(word);
			}

			for(Query::ResultBucketTable::const_iterator i = query->m_resultBucketTable.cbegin(); i != query->m_resultBucketTable.cend(); i++)
			{
				uint32_t score = i->first;
				const ResultBucket* resultBucket = i->second.get();

				uint32_t weight = score * (uint32_t)resultBucket->m_words.size();

				query->m_resultBuckets.AddPossibility(weight, resultBucket);
			}
		}

		return query.release();
	}

}

