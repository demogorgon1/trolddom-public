#pragma once

#include "DataType.h"
#include "Hash.h"

namespace tpublic
{

	class Manifest;

	class TaggedData
	{
	public:
		struct Tag
		{
			std::vector<uint32_t>		m_dataIds;
		};

		struct Query
		{
			struct Hasher
			{
				uint32_t
				operator()(
					const Query&								aQuery) const
				{
					static_assert(sizeof(TagScoring) == sizeof(uint32_t) * 2);

					Hash::CheckSum hash;
					hash.AddPODVector(aQuery.m_mustHaveTagIds);
					hash.AddPODVector(aQuery.m_mustNotHaveTagIds);
					hash.AddPODVector(aQuery.m_tagScoring);
					return hash.m_hash;
				}
			};

			struct TagScoring
			{
				bool
				operator ==(
					const TagScoring&							aOther) const
				{
					return m_tagId == aOther.m_tagId && m_score == aOther.m_score;
				}

				// Public data
				uint32_t				m_tagId = 0;
				uint32_t				m_score = 0;
			};

			Query()
			{

			}

			Query(
				const Query&									aOther)
			{
				m_mustHaveTagIds = aOther.m_mustHaveTagIds;
				m_mustNotHaveTagIds = aOther.m_mustNotHaveTagIds;
				m_tagScoring = aOther.m_tagScoring;
			}

			Query(
				Query&&											aOther)
			{
				m_mustHaveTagIds = std::move(aOther.m_mustHaveTagIds);
				m_mustNotHaveTagIds = std::move(aOther.m_mustNotHaveTagIds);
				m_tagScoring = std::move(aOther.m_tagScoring);
			}

			bool 
			operator ==(
				const Query&									aOther) const
			{
				return m_mustHaveTagIds == aOther.m_mustHaveTagIds && m_mustNotHaveTagIds == aOther.m_mustNotHaveTagIds && m_tagScoring == aOther.m_tagScoring;
			}

			bool
			HasMustNotHaveTagId(
				uint32_t										aTagId) const
			{
				for(uint32_t t : m_mustNotHaveTagIds)
				{
					if(t == aTagId)
						return true;
				}
				return false;
			}

			uint32_t
			GetScore(
				uint32_t										aTagId) const
			{
				for(const TagScoring& t : m_tagScoring)
				{
					if(t.m_tagId == aTagId)
						return t.m_score;
				}

				return 0;
			}

			uint32_t
			CalculateScore(
				const std::vector<uint32_t>&					aTagIds)  const
			{
				uint32_t sum = 0;
				for(uint32_t tagId : aTagIds)
				{
					uint32_t score = GetScore(tagId);
					sum += score > 0 ? score : 1;
				}
				return sum;
			}

			// Public data
			std::vector<uint32_t>		m_mustHaveTagIds;
			std::vector<uint32_t>		m_mustNotHaveTagIds;
			std::vector<TagScoring>		m_tagScoring;			
		};

		struct QueryResult
		{
			struct Entry
			{
				uint32_t				m_id = 0;
				uint32_t				m_weight = 0;
				std::vector<uint32_t>	m_tags;
			};

			typedef std::function<bool(uint32_t)> Filter;

			const Entry*
			TryPickRandomWithFilter(
				std::mt19937&									aRandom,
				Filter											aFilter) const
			{
				if (m_entries.empty())
					return NULL;

				uint32_t filteredTotalWeight = 0;
				for (const Entry& t : m_entries)
				{
					if(aFilter(t.m_id))
						filteredTotalWeight += t.m_weight;
				}

				tpublic::UniformDistribution<uint32_t> distribution(1, filteredTotalWeight);
				uint32_t roll = distribution(aRandom);

				uint32_t sum = 0;
				for (const Entry& t : m_entries)
				{
					if(aFilter(t.m_id))
					{
						sum += t.m_weight;

						if (roll <= sum)
							return &t;
					}
				}

				assert(false);
				return NULL;
			}

			const Entry*
			TryPickRandom(
				std::mt19937&									aRandom) const
			{
				if(m_entries.empty())
					return NULL;

				assert(m_totalWeight > 0);

				tpublic::UniformDistribution<uint32_t> distribution(1, m_totalWeight);
				uint32_t roll = distribution(aRandom);
				
				uint32_t sum = 0;
				for(const Entry& t : m_entries)
				{
					sum += t.m_weight;

					if(roll <= sum)
						return &t;
				}

				assert(false);
				return NULL;
			}

			const Entry*
			PickRandom(
				std::mt19937&									aRandom) const
			{
				const Entry* t = TryPickRandom(aRandom);
				TP_CHECK(t != NULL, "Empty query result.");
				return t;
			}

			const Entry*
			TryPickRandomFiltered(
				std::mt19937&									aRandom,
				std::function<bool(const Entry*)>				aFilter) const
			{
				std::vector<const Entry*> filtered;
				uint32_t filteredTotalWeight = 0;

				for (const Entry& t : m_entries)
				{
					if(aFilter(&t))
					{
						filtered.push_back(&t);
						filteredTotalWeight += t.m_weight;
					}
				}

				if (filtered.empty())
					return NULL;

				assert(filteredTotalWeight > 0);

				tpublic::UniformDistribution<uint32_t> distribution(1, filteredTotalWeight);
				uint32_t roll = distribution(aRandom);

				uint32_t sum = 0;
				for (const Entry* t : filtered)
				{
					sum += t->m_weight;

					if (roll <= sum)
						return t;
				}

				assert(false);
				return NULL;
			}

			// Public data
			std::vector<Entry>			m_entries;
			uint32_t					m_totalWeight = 0;
		};

							TaggedData(
								const Manifest*					aManifest,
								DataType::Id					aDataType);
							~TaggedData();

		void				Add(
								uint32_t						aDataId,
								const std::vector<uint32_t>&	aTagIds);
		const Tag*			GetTag(
								uint32_t						aTagId) const;
		const QueryResult*	PerformQuery(
								const Query&					aQuery);
		const QueryResult*	PerformQueryWithTagContext(
								const Data::TagContext*			aTagContext);

	private:

		const Manifest*					m_manifest;
		DataType::Id					m_dataType;
	
		typedef std::unordered_map<uint32_t, std::unique_ptr<Tag>> TagTable;
		TagTable						m_tagTable;

		typedef std::unordered_map<Query, std::unique_ptr<QueryResult>, Query::Hasher> QueryTable;
		QueryTable						m_queryTable;

		Tag*				_GetOrCreateTag(
								uint32_t						aTagId);
		const Tag*			_GetTag(
								uint32_t						aTagId) const;
		void				_Query(
								const Query&					aQuery,
								QueryResult*					aOut) const;
	};

}