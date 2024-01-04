#pragma once

//#include "WordContext.h"

#include "Hash.h"
#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"
#include "WeightedRandom.h"

namespace tpublic
{
	class Manifest;
}

namespace tpublic::WordList
{

	struct Word
	{
		enum Flag : uint8_t
		{
			FLAG_HIDDEN = 0x01
		};

		void		
		ToStream(
			IWriter*									aWriter) const
		{
			aWriter->WriteString(m_word);
			aWriter->WritePOD(m_flags);
			aWriter->WriteUInts(m_explicitTags);
		}
			
		bool
		FromStream(
			IReader*									aReader)
		{
			if(!aReader->ReadString(m_word))
				return false;
			if(!aReader->ReadPOD(m_flags))
				return false;
			if(!aReader->ReadUInts(m_explicitTags))
				return false;
			return true;
		}

		// Public data
		std::string								m_word;
		uint8_t									m_flags = 0;
		std::vector<uint32_t>					m_explicitTags;
	};

	struct DataChunk
	{
		static const size_t SIZE = 64;

		Word*
		AllocateWord()
		{
			assert(m_numWords < SIZE);
			return &m_words[m_numWords++];
		}

		// Public data			
		size_t									m_numWords = 0;
		Word									m_words[SIZE];
	};

	struct Tag
	{
		enum Flag : uint8_t
		{
			FLAG_EXPLICIT = 0x01
		};
	
		// Public data
		uint32_t								m_id = 0;
		std::unordered_set<uint32_t>			m_words;
		uint8_t									m_flags = 0;
	};

	class Data
	{
	public:
		static void FromSource(
						const SourceNode*				aSource,
						Data*							aData);

		void		ToStream(
						IWriter*						aWriter) const;
		bool		FromStream(
						IReader*						aReader);
		void		AddWord(
						const char*						aWord,
						uint8_t							aFlags,
						const std::vector<uint32_t>&	aTags);
		const Word*	GetWordByIndex(
						uint32_t						aIndex) const;
		Word*		GetWordByIndex(
						uint32_t						aIndex);
		const Tag*	TryGetTag(
						uint32_t						aTagId) const;
		void		Prepare(
						const Manifest*					aManifest);

		// Public data
		typedef std::unordered_map<uint32_t, std::unique_ptr<Tag>> TagTable;
		TagTable								m_tagTable;
		std::vector<std::unique_ptr<DataChunk>>	m_dataChunks;
	};

	struct QueryParams
	{
		bool
		Compare(
			const QueryParams&							aOther) const
		{
			assert(m_prepared);
			assert(aOther.m_prepared);
			return m_mustHaveTags == aOther.m_mustHaveTags && m_mustNotHaveTags == aOther.m_mustNotHaveTags && m_tagScoring == aOther.m_tagScoring;
		}

		//void
		//AddContext(
		//	const WordContext::Type*				aContext)
		//{
		//	m_mustHaveTags.Add(aContext->m_mustHaveTags);
		//	m_mustNotHaveTags.Add(aContext->m_mustNotHaveTags);
		//	for(const WordContext::Scoring& scoring : aContext->m_scorings)
		//		m_tagScoring.Add(nc::MakePair(scoring.m_tagId, scoring.m_weight));
		//}

		uint32_t
		GetHash() const
		{
			assert(m_prepared);
			Hash::CheckSum hash;
			if(m_mustHaveTags.size() > 0)
				hash.AddData(&m_mustHaveTags[0], m_mustHaveTags.size() * sizeof(uint32_t));
			if (m_mustNotHaveTags.size() > 0)
				hash.AddData(&m_mustNotHaveTags[0], m_mustNotHaveTags.size() * sizeof(uint32_t));
			if (m_tagScoring.size() > 0)
				hash.AddData(&m_tagScoring[0], m_tagScoring.size() * sizeof(uint32_t) * 2);
			return hash.m_hash;
		}

		void
		Prepare()
		{
			assert(!m_prepared);
			std::sort(m_mustHaveTags.begin(), m_mustHaveTags.end(), [](uint32_t aLHS, uint32_t aRHS) { return aLHS < aRHS; });
			std::sort(m_mustNotHaveTags.begin(), m_mustNotHaveTags.end(), [](uint32_t aLHS, uint32_t aRHS) { return aLHS < aRHS; });

			std::sort(m_tagScoring.begin(), m_tagScoring.end(), [](
				const std::pair<uint32_t, uint32_t>& aLHS, 
				const std::pair<uint32_t, uint32_t>& aRHS) 
			{ 
				if(aLHS.first == aRHS.first)
					return aLHS.second < aRHS.second;
				return aLHS.first < aRHS.first;
			});

			m_prepared = true;
		}

		uint32_t
		GetTagScore(
			uint32_t									aTagId) const
		{
			if(m_tagScoring.size() == 0)
				return 1;

			// FIXME: assuming that only a few tags are scored (otherwise we'll need a hashtable)
			for (const std::pair<uint32_t, uint32_t>& tagScoring : m_tagScoring)
			{
				if(tagScoring.first == aTagId)
					return tagScoring.second;
			}
			return 0;
		}

		bool
		HasTag(
			uint32_t									aTagId) const
		{
			for(uint32_t mustHaveTag : m_mustHaveTags)
			{
				if(mustHaveTag == aTagId)
					return true;
			}

			for (const std::pair<uint32_t, uint32_t>& tagScoring : m_tagScoring)
			{
				if(tagScoring.first == aTagId)
					return true;
			}

			return false;
		}

		void
		ForEachTag(
			std::function<void(uint32_t)>				aCallback) const
		{
			for(uint32_t tagId : m_mustHaveTags)
				aCallback(tagId);

			for (const std::pair<uint32_t, uint32_t>& tagScoring : m_tagScoring)
				aCallback(tagScoring.first);
		}

		// Public data
		std::vector<uint32_t>							m_mustHaveTags;
		std::vector<uint32_t>							m_mustNotHaveTags;			
		std::vector<std::pair<uint32_t, uint32_t>>		m_tagScoring;
		bool											m_prepared = false;
	};

	class QueryCache
	{
	public:
		struct ResultBucket
		{
			ResultBucket(
				uint32_t								aScore)
				: m_score(aScore)
			{

			}

			const Word*
			GetRandom(
				std::mt19937&							aRandom) const
			{
				if(m_words.size() == 0)
					return NULL;
				
				if(m_words.size() == 1)
					return m_words[0];

				std::uniform_int_distribution<size_t> distribution(0, m_words.size() - 1);
				return m_words[distribution(aRandom)];
			}

			// Public data
			uint32_t								m_score;
			std::vector<const Word*>				m_words;
		};

		struct Query
		{
			Query(
				const QueryParams&					aParams)
				: m_params(aParams)
			{

			}

			const Word*
			GetRandomWord(
				std::mt19937&						aRandom) const
			{
				const ResultBucket* resultBucket;
				if(!m_resultBuckets.Pick(aRandom, resultBucket))
					return NULL;

				return resultBucket->GetRandom(aRandom);
			}

			// Public data
			QueryParams								m_params;

			typedef std::unordered_map<uint32_t, std::unique_ptr<ResultBucket>> ResultBucketTable;
			ResultBucketTable						m_resultBucketTable;
			WeightedRandom<const ResultBucket*>		m_resultBuckets;
		};

						QueryCache(
							const Data*				aData);
						~QueryCache();
	
		const Query*	PerformQuery(
							const QueryParams&		aParams);

	private:

		const Data*									m_data;
			
		struct QueryBucket
		{
			const Query*
			TryGetQueryByParams(
				const QueryParams&					aParams) const
			{
				for(const std::unique_ptr<Query>& query : m_queries)
				{
					if(query->m_params.Compare(aParams))
						return query.get();
				}
				return NULL;
			}

			// Public data
			std::vector<std::unique_ptr<Query>>		m_queries;
		};

		typedef std::unordered_map<uint32_t, std::unique_ptr<QueryBucket>> QueryBucketTable;
		QueryBucketTable							m_queryBucketTable;

		Query*			_NewQuery(
							const QueryParams&		aParams);
	};

}