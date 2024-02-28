#include "Pcheader.h"

#include <tpublic/Data/Tag.h>

#include <tpublic/Manifest.h>
#include <tpublic/TaggedData.h>

namespace tpublic
{

	TaggedData::TaggedData(
		const Manifest*					aManifest,
		DataType::Id					aDataType)
		: m_manifest(aManifest)
		, m_dataType(aDataType)
	{
	}
	
	TaggedData::~TaggedData()
	{

	}

	void		
	TaggedData::Add(
		uint32_t						aDataId,
		const std::vector<uint32_t>&	aTagIds)
	{
		for(uint32_t tagId : aTagIds)
			_GetOrCreateTag(tagId)->m_dataIds.push_back(aDataId);
	}
	
	const TaggedData::Tag*
	TaggedData::GetTag(
		uint32_t						aTagId) const
	{
		return _GetTag(aTagId);
	}

	const TaggedData::QueryResult* 
	TaggedData::PerformQuery(
		const Query&					aQuery)
	{
		QueryTable::iterator i = m_queryTable.find(aQuery);
		if(i != m_queryTable.end())
			return i->second.get();

		std::unique_ptr<QueryResult> result = std::make_unique<QueryResult>();
		_Query(aQuery, result.get());

		QueryResult* t = result.get();
		m_queryTable[aQuery] = std::move(result);
		return t;
	}

	const TaggedData::QueryResult* 
	TaggedData::PerformQueryWithTagContext(
		const Data::TagContext*			aTagContext)
	{
		Query query;
		query.m_mustHaveTagIds = aTagContext->m_mustHaveTags;
		query.m_mustNotHaveTagIds = aTagContext->m_mustNotHaveTags;

		for(const Data::TagContext::Scoring& scoring : aTagContext->m_scoring)
			query.m_tagScoring.push_back({ scoring.m_tagId, scoring.m_score });

		return PerformQuery(query);
	}

	//---------------------------------------------------------------------------------------

	TaggedData::Tag*
	TaggedData::_GetOrCreateTag(
		uint32_t						aTagId)
	{
		TagTable::iterator i = m_tagTable.find(aTagId);
		if(i != m_tagTable.end())
			return i->second.get();

		Tag* t = new Tag();
		m_tagTable[aTagId] = std::unique_ptr<Tag>(t);
		return t;
	}

	const TaggedData::Tag*
	TaggedData::_GetTag(
		uint32_t						aTagId) const
	{
		TagTable::const_iterator i = m_tagTable.find(aTagId);
		if (i == m_tagTable.cend())
			return NULL;
		return i->second.get();
	}

	void		
	TaggedData::_Query(
		const Query&					aQuery,
		QueryResult*					aOut) const
	{
		std::unordered_set<uint32_t> dataIds;

		if(aQuery.m_mustHaveTagIds.size() > 0)
		{
			for(uint32_t mustHaveTagId : aQuery.m_mustHaveTagIds)
			{
				const Tag* tag = _GetTag(mustHaveTagId);
				if(tag != NULL)
				{
					for (uint32_t dataId : tag->m_dataIds)
						dataIds.insert(dataId);
				}
			}
		}
		else
		{
			for(const Query::TagScoring& tagScoring : aQuery.m_tagScoring)
			{
				const Tag* tag = _GetTag(tagScoring.m_tagId);
				if(tag != NULL)
				{
					for (uint32_t dataId : tag->m_dataIds)
						dataIds.insert(dataId);
				}
			}
		}

		for(uint32_t dataId : dataIds)
		{
			const DataBase* data = m_manifest->m_containers[m_dataType]->GetExistingBaseById(dataId);

			bool ok = true;

			if(aQuery.m_mustHaveTagIds.size() > 0)
			{
				for (uint32_t mustHaveTagId : aQuery.m_mustHaveTagIds)
				{
					if(!data->HasTag(mustHaveTagId))
					{
						ok = false;
						break;
					}
				}
			}

			if(ok)
			{
				for(uint32_t tagId : data->m_tagIds)
				{
					if(aQuery.HasMustNotHaveTagId(tagId))
					{
						ok = false;
						break;
					}
				}
			}

			if(ok)
			{
				QueryResult::Entry entry;
				entry.m_id = dataId;
				entry.m_weight = aQuery.CalculateScore(data->m_tagIds);
				
				for(uint32_t tagId : data->m_tagIds)
				{
					const Data::Tag* tagData = m_manifest->GetById<Data::Tag>(tagId);
					if(tagData->m_transferable)
						entry.m_tags.push_back(tagId);
				}

				aOut->m_entries.push_back(entry);
				aOut->m_totalWeight += entry.m_weight;
			}
		}
	}

}