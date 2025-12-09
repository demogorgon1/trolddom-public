#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"
#include "Stat.h"
#include "WeightedRandom.h"

namespace tpublic
{
	class Manifest;
}

namespace tpublic::Data
{
	struct TagContext;
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
			aWriter->WriteOptionalObjectPointer(m_associatedStatWeights);
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
			if(!aReader->ReadOptionalObjectPointer(m_associatedStatWeights))
				return false;
			return true;
		}

		// Public data
		std::string								m_word;
		uint8_t									m_flags = 0;
		std::vector<uint32_t>					m_explicitTags;
		std::vector<uint32_t>					m_allTags;
		std::unique_ptr<Stat::Collection>		m_associatedStatWeights;
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
		bool		Compare(
						const QueryParams&							aOther) const;
		void		AddTagContext(
						const tpublic::Data::TagContext*			aTagContext);
		uint32_t	GetHash() const;
		void		Prepare();
		uint32_t	GetTagScore(
						uint32_t									aTagId) const;
		bool		HasTag(
						uint32_t									aTagId) const;
		void		ForEachTag(
						std::function<void(uint32_t)>				aCallback) const;

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

				tpublic::UniformDistribution<size_t> distribution(0, m_words.size() - 1);
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