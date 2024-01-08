#include "Pcheader.h"

#include <tpublic/Data/LootGroup.h>
#include <tpublic/Data/Tag.h>

#include <tpublic/DataErrorHandling.h>
#include <tpublic/Helpers.h>
#include <tpublic/ItemBinding.h>
#include <tpublic/Manifest.h>
#include <tpublic/SourceNode.h>
#include <tpublic/WeightedRandom.h>

#include "GenerationJob.h"

namespace
{
	void
	_CreateDirectoryIfNotThere(
		const char*				aPath)
	{
		try
		{
			if (!std::filesystem::exists(aPath))
				std::filesystem::create_directory(aPath);
		}
		catch(...)
		{
			TP_CHECK(false, "Failed to create directory: %s", aPath);
		}
	}

	bool
	_IsDirectoryThere(
		const char*				aPath)
	{
		bool directoryExists = false;

		try
		{
			directoryExists = std::filesystem::exists(aPath);
		}
		catch (...)
		{
			TP_CHECK(false, "Failed to check if directory exists: %s", aPath);
		}

		return directoryExists;
	}
}

namespace tpublic
{

	GenerationJob::GenerationJob(
		const SourceNode*		aSource)
		: m_source(aSource)
		, m_manifest(NULL)
	{

	}
	
	GenerationJob::~GenerationJob()
	{

	}

	void		
	GenerationJob::Run(
		const Manifest*			aManifest,
		const char*				aOutputPath)
	{
		// Initialize stuff
		m_manifest = aManifest;
		m_wordListQueryCache = std::make_unique<WordList::QueryCache>(&m_manifest->m_wordList);
		m_taggedSpriteData = std::make_unique<TaggedDataCache<Data::Sprite>>(m_manifest);

		// Initialize output path
		{
			_CreateDirectoryIfNotThere(aOutputPath);

			m_outputPath = aOutputPath;
			m_outputPath += "/";
			m_outputPath += m_source->m_name.c_str();

			if(_IsDirectoryThere(m_outputPath.c_str()))
				return; // Already there, don't run generation job						
		}

		// Read source and generate stuff
		_ReadSource();

		// Write stuff
		{
			_CreateDirectoryIfNotThere(m_outputPath.c_str());

			std::string generatedSourceFilePath = m_outputPath;
			generatedSourceFilePath += "/generated_source.txt";

			FILE* f = fopen(generatedSourceFilePath.c_str(), "wb");
			TP_CHECK(f != NULL, "Failed to open file for output: %s", generatedSourceFilePath.c_str());

			try
			{
				for (const std::unique_ptr<GeneratedSource>& generated : m_generatedSource)
				{
					for (const std::string& line : generated->m_lines)
						fprintf(f, "%s\r\n", line.c_str());
					
					fprintf(f, "\r\n");
				}
			}
			catch(...)
			{
				TP_CHECK(false, "Failed to write file: %s", generatedSourceFilePath.c_str());
			}

			fclose(f);
		}
	}

	//------------------------------------------------------------------------------------

	void		
	GenerationJob::_ReadSource()
	{
		_ReadCompound(m_source);
	}

	void		
	GenerationJob::_ReadCompound(
		const SourceNode*		aSource)
	{
		size_t originalStackSize = m_stack.size();

		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->IsAnonymousObject())
				_ReadCompound(aChild);
			else if(aChild->m_name == "random_seed")
				_ReadStackObject(StackObject::TYPE_RANDOM_NUMBER_GENERATOR, aChild);
			else if(aChild->m_name == "random_tags")
				_ReadStackObject(StackObject::TYPE_RANDOM_TAGS, aChild);
			else if (aChild->m_name == "item_classes")
				_ReadStackObjectArray(StackObject::TYPE_ITEM_CLASS, aChild);
			else if (aChild->m_name == "designations")
				_ReadStackObjectArray(StackObject::TYPE_DESIGNATION, aChild);
			else if (aChild->m_name == "level_range")
				_ReadStackObject(StackObject::TYPE_LEVEL_RANGE, aChild);
			else if (aChild->m_name == "items")
				_ReadItems(aChild);
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
		});

		assert(originalStackSize <= m_stack.size());
		m_stack.resize(originalStackSize);
	}

	void		
	GenerationJob::_ReadStackObject(
		StackObject::Type		aType,
		const SourceNode*		aSource)
	{
		std::unique_ptr<StackObject> stackObject = std::make_unique<StackObject>();
		stackObject->m_type = aType;

		switch(aType)
		{
		case StackObject::TYPE_RANDOM_NUMBER_GENERATOR:
			{
				stackObject->m_randomNumberGenerator = std::make_unique<std::mt19937>(aSource->GetUInt32());
			}
			break;

		case StackObject::TYPE_RANDOM_TAGS:
			{
				TP_VERIFY(aSource->m_annotation && aSource->m_annotation->GetArray()->m_children.size() == 2, aSource->m_debugInfo, "Missing propability and count annotation.");
				stackObject->m_randomTags.m_propability = aSource->m_annotation->GetArrayIndex(0)->GetUInt32();
				stackObject->m_randomTags.m_count = aSource->m_annotation->GetArrayIndex(1)->GetUInt32();
				aSource->GetIdArray(DataType::ID_TAG, stackObject->m_randomTags.m_tags);
				TP_VERIFY(stackObject->m_randomTags.m_tags.size() > 0, aSource->m_debugInfo, "Tag array is empty.");
			}
			break;

		case StackObject::TYPE_LEVEL_RANGE:
			{
				stackObject->m_range = UIntRange(aSource);
			}
			break;

		case StackObject::TYPE_ITEM_CLASS:			
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "slots")
					aChild->GetIdArrayWithLookup<EquipmentSlot::Id, EquipmentSlot::INVALID_ID>(stackObject->m_itemClass.m_slots, [](const char* aString) { return EquipmentSlot::StringToId(aString); });
				else if(aChild->m_name == "types")
					aChild->GetIdArrayWithLookup<ItemType::Id, ItemType::INVALID_ID>(stackObject->m_itemClass.m_types, [](const char* aString) { return ItemType::StringToId(aString); });
				else if(aChild->m_name == "min_level")
					stackObject->m_itemClass.m_minLevel = aChild->GetUInt32();
				else if (aChild->m_name == "max_level")
					stackObject->m_itemClass.m_maxLevel = aChild->GetUInt32();
				else if (aChild->m_name == "weight")
					stackObject->m_itemClass.m_weight = aChild->GetUInt32();
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
			break;

		case StackObject::TYPE_DESIGNATION:			
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "prefix")
					aChild->GetObjectArray<StackObject::Designation::Segment>(stackObject->m_designation.m_prefix);
				else if (aChild->m_name == "suffix")
					aChild->GetObjectArray<StackObject::Designation::Segment>(stackObject->m_designation.m_suffix);
				else if (aChild->m_name == "weight")
					stackObject->m_designation.m_weight = aChild->GetUInt32();
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
			break;

		default:
			assert(false);
			break;
		}

		m_stack.push_back(std::move(stackObject));
	}

	void
	GenerationJob::_ReadStackObjectArray(
		StackObject::Type		aType,
		const SourceNode*		aSource)
	{
		aSource->GetArray()->ForEachChild([&](
			const SourceNode* aChild)
		{
			_ReadStackObject(aType, aChild);
		});
	}

	void		
	GenerationJob::_ReadItems(
		const SourceNode*		aSource)
	{
		uint32_t count = 0;
		Rarity::Id rarity = Rarity::ID_COMMON;
		std::vector<uint32_t> lootGroups;
		uint32_t maxDifferentStats = 3;
		Stat::Collection baseStatWeights;
		ItemBinding::Id itemBinding = ItemBinding::ID_WHEN_EQUIPPED;

		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "count")
				count = aChild->GetUInt32();
			else if (aChild->m_name == "max_different_stats")
				maxDifferentStats = aChild->GetUInt32();
			else if (aChild->m_name == "rarity")
				rarity = Rarity::StringToId(aChild->GetIdentifier());
			else if (aChild->m_name == "binds")
				itemBinding = ItemBinding::StringToId(aChild->GetIdentifier());
			else if(aChild->m_name == "loot_groups")
				aChild->GetIdArray(DataType::ID_LOOT_GROUP, lootGroups);
			else if(aChild->m_name == "stat_weights")
				baseStatWeights.FromSource(aChild);
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
		});

		TP_VERIFY(rarity != Rarity::INVALID_ID, aSource->m_debugInfo, "Invalid rarity.");
		TP_VERIFY(lootGroups.size() > 0, aSource->m_debugInfo, "No loot groups.");

		for(uint32_t i = 0; i < count; i++)
		{
			const UIntRange& levelRange = _GetLevelRange();
			uint32_t itemLevel = _GetRandomIntInRange(levelRange.m_min, levelRange.m_max);

			WeightedRandom<const StackObject::ItemClass*> possibleItemClasses;
			WeightedRandom<const StackObject::Designation*> possibleDesignations;

			for (const std::unique_ptr<StackObject>& stackObject : m_stack)
			{
				if(stackObject->m_type == StackObject::TYPE_ITEM_CLASS)
				{
					const StackObject::ItemClass* itemClass = &stackObject->m_itemClass;

					if(itemClass->m_minLevel != 0 && itemLevel < itemClass->m_minLevel )
						continue;

					if (itemClass->m_maxLevel != 0 && itemLevel > itemClass->m_maxLevel)
						continue;

					possibleItemClasses.AddPossibility(itemClass->m_weight, itemClass);
				}
			}

			for (const std::unique_ptr<StackObject>& stackObject : m_stack)
			{
				if (stackObject->m_type == StackObject::TYPE_DESIGNATION)
				{
					const StackObject::Designation* designation = &stackObject->m_designation;

					possibleDesignations.AddPossibility(designation->m_weight, designation);
				}
			}

			const StackObject::ItemClass* itemClass;
			const StackObject::Designation* designation;

			if(possibleItemClasses.Pick(_GetRandom(), itemClass) && possibleDesignations.Pick(_GetRandom(), designation))
			{
				ItemType::Id itemType = ItemType::ID_NONE;
				std::unordered_set<uint32_t> mustHaveTags;
				uint32_t lastEquipmentSlotTagId = 0;

				if(itemClass->m_types.size() > 0)
				{
					itemType = _GetRandomItemInVector(itemClass->m_types);

					uint32_t itemTypeTagId = m_manifest->TryGetExistingIdByName<Data::Tag>(ItemType::GetInfo(itemType)->m_name);
					if(itemTypeTagId != 0)
						mustHaveTags.insert(itemTypeTagId);
				}

				for(EquipmentSlot::Id equipmentSlot : itemClass->m_slots)
				{
					uint32_t equipmentSlotTagId = m_manifest->TryGetExistingIdByName<Data::Tag>(EquipmentSlot::GetInfo(equipmentSlot)->m_tag);
					if (equipmentSlotTagId != 0)
					{
						mustHaveTags.insert(equipmentSlotTagId);
						lastEquipmentSlotTagId = equipmentSlotTagId;
					}
				}

				WordList::QueryParams wordListQuery;
				for(uint32_t tagId : mustHaveTags)
					wordListQuery.m_mustHaveTags.push_back(tagId);
				wordListQuery.Prepare();
				const WordList::Word* baseNameWord = m_wordListQueryCache->PerformQuery(wordListQuery)->GetRandomWord(_GetRandom());

				if(baseNameWord != NULL)
				{
					std::vector<uint32_t> contextTags;
					_GetContextTags(contextTags);

					std::vector<uint32_t> allTags = baseNameWord->m_allTags;

					std::string itemString;
					Stat::Collection statWeights;
					_CreateDesignation(baseNameWord->m_word.c_str(), designation, contextTags, allTags, statWeights, itemString);

					if(statWeights.IsEmpty())
						Helpers::GetRandomStatWeights(_GetRandom()(), statWeights);

					statWeights.RemoveLowStats(maxDifferentStats);
					statWeights.Add(baseStatWeights);

					std::unordered_set<uint32_t> allTagsSet;
					for(uint32_t tagId : allTags)
					{
						const Data::Tag* tag = m_manifest->GetById<Data::Tag>(tagId);

						if(tag->m_transferable)
							allTagsSet.insert(tagId);
					}

					const char* iconName = _PickIconName(lastEquipmentSlotTagId, allTags);

					GeneratedSource* output = _CreateGeneratedSource();

					output->PrintF(0, "item %s_item_%u:", m_source->m_name.c_str(), m_nextItemNumber++);
					output->PrintF(0, "{");
					output->PrintF(1, "string: \"%s\"", itemString.c_str());
					output->PrintF(1, "icon: %s", iconName);
					output->PrintF(1, "rarity: %s", Rarity::GetInfo(rarity)->m_name);
					output->PrintF(1, "item_level: %u", itemLevel);
					output->PrintF(1, "required_level: %u", itemLevel);
					output->PrintF(1, "type: %s", ItemType::GetInfo(itemType)->m_name);
					output->PrintF(1, "binds: %s", ItemBinding::GetInfo(itemBinding)->m_name);

					if(itemClass->m_slots.size() > 0)
					{
						output->PrintF(1, "equipment_slots:");
						output->PrintF(1, "[");
						for (EquipmentSlot::Id equipmentSlot : itemClass->m_slots)
							output->PrintF(2, "%s", EquipmentSlot::GetInfo(equipmentSlot)->m_name);
						output->PrintF(1, "]");
					}

					output->PrintF(1, "tags:");
					output->PrintF(1, "[");
					for(uint32_t tagId : allTagsSet)
						output->PrintF(2, "%s", m_manifest->GetById<Data::Tag>(tagId)->m_name.c_str());
					output->PrintF(1, "]");

					for(uint32_t j = 1; j < (uint32_t)Stat::NUM_IDS; j++)
					{
						if(statWeights.m_stats[j] != 0)
						{
							const Stat::Info* statInfo = Stat::GetInfo((Stat::Id)j);
							output->PrintF(1, "stat_weight %s: %u", statInfo->m_name, statWeights.m_stats[j]);
						}
					}

					output->PrintF(1, "loot_groups:");
					output->PrintF(1, "[");
					for(uint32_t lootGroupId : lootGroups)
						output->PrintF(2, "%s", m_manifest->GetById<Data::LootGroup>(lootGroupId)->m_name.c_str());
					output->PrintF(1, "]");
				
					output->PrintF(0, "}");
				}
			}
		}
	}

	void		
	GenerationJob::_GetContextTags(
		std::vector<uint32_t>&	aOut)
	{
		for(const std::unique_ptr<StackObject>& stackObject : m_stack)
		{
			switch(stackObject->m_type)
			{
			case StackObject::TYPE_RANDOM_TAGS:
				for(uint32_t i = 0; i < stackObject->m_randomTags.m_count; i++)
				{
					if(_GetRandomIntInRange<uint32_t>(1, 100) <= stackObject->m_randomTags.m_propability)
						aOut.push_back(_GetRandomItemInVector(stackObject->m_randomTags.m_tags));
				}
				break;

			default:
				break;
			}
		}
	}

	std::mt19937& 
	GenerationJob::_GetRandom()
	{
		std::mt19937* randomNumberGenerator = &m_defaultRandomNumberGenerator;

		for (const std::unique_ptr<StackObject>& stackObject : m_stack)
		{
			if(stackObject->m_type == StackObject::TYPE_RANDOM_NUMBER_GENERATOR)
				randomNumberGenerator = stackObject->m_randomNumberGenerator.get();
		}

		assert(randomNumberGenerator != NULL);
		return *randomNumberGenerator;
	}

	const UIntRange& 
	GenerationJob::_GetLevelRange() const
	{
		UIntRange* levelRange = NULL;

		for (const std::unique_ptr<StackObject>& stackObject : m_stack)
		{
			if(stackObject->m_type == StackObject::TYPE_LEVEL_RANGE)
				levelRange = &stackObject->m_range;
		}

		TP_VERIFY(levelRange != NULL, m_source->m_debugInfo, "Missing level range.");
		return *levelRange;
	}

	const char* 
	GenerationJob::_PickIconName(
		uint32_t							aMustHaveTagId,
		const std::vector<uint32_t>&		aTags)
	{
		TaggedData::Query query;
		query.m_mustHaveTagIds = { m_manifest->GetExistingIdByName<Data::Tag>("icon") };

		if(aMustHaveTagId != 0)
			query.m_mustHaveTagIds.push_back(aMustHaveTagId);

		for(uint32_t tagId : aTags)
		{
			TaggedData::Query::TagScoring scoring;
			scoring.m_tagId = tagId;
			scoring.m_score = 1;
			query.m_tagScoring.push_back(scoring);
		}

		const TaggedData::QueryResult* result = m_taggedSpriteData->Get()->PerformQuery(query);
		uint32_t iconSpriteId = result->PickRandom(_GetRandom());

		if(iconSpriteId == 0)
			iconSpriteId = m_manifest->GetExistingIdByName<Data::Sprite>("icon_default");

		return m_manifest->GetById<Data::Sprite>(iconSpriteId)->m_name.c_str();
	}

	GenerationJob::GeneratedSource* 
	GenerationJob::_CreateGeneratedSource()
	{
		GeneratedSource* t = new GeneratedSource();
		m_generatedSource.push_back(std::unique_ptr<GeneratedSource>(t));
		return t;
	}

	void				
	GenerationJob::_CreateDesignation(
		const char*							aBaseName,
		const StackObject::Designation*		aDesignation,
		const std::vector<uint32_t>&		aContextTags,
		std::vector<uint32_t>&				aTags,
		Stat::Collection&					aStatWeights,
		std::string&						aOut)
	{
		std::string prefix;

		for(const StackObject::Designation::Segment& segment : aDesignation->m_prefix)
		{
			if(segment.m_tagId != 0)
			{
				WordList::QueryParams wordListQuery;
				wordListQuery.m_mustHaveTags.push_back(segment.m_tagId);

				for(uint32_t tagId : aContextTags)
					wordListQuery.m_tagScoring.push_back({ tagId, 1 });

				wordListQuery.Prepare();
				const WordList::Word* word = m_wordListQueryCache->PerformQuery(wordListQuery)->GetRandomWord(_GetRandom());

				if(word == NULL)
				{
					prefix = "";
					break;
				}

				prefix += word->m_word;

				if(word->m_associatedStatWeights)
					aStatWeights.Add(*word->m_associatedStatWeights);

				for(uint32_t tagId : word->m_allTags)
					aTags.push_back(tagId);
			}
			else
			{
				prefix += segment.m_string;
			}
		}

		std::string suffix;

		for (const StackObject::Designation::Segment& segment : aDesignation->m_suffix)
		{
			if (segment.m_tagId != 0)
			{
				WordList::QueryParams wordListQuery;
				wordListQuery.m_mustHaveTags.push_back(segment.m_tagId);

				for (uint32_t tagId : aContextTags)
					wordListQuery.m_tagScoring.push_back({ tagId, 1 });

				wordListQuery.Prepare();
				const WordList::Word* word = m_wordListQueryCache->PerformQuery(wordListQuery)->GetRandomWord(_GetRandom());

				if (word == NULL)
				{
					suffix = "";
					break;
				}

				suffix += word->m_word;

				if (word->m_associatedStatWeights)
					aStatWeights.Add(*word->m_associatedStatWeights);

				for (uint32_t tagId : word->m_allTags)
					aTags.push_back(tagId);
			}
			else
			{
				suffix += segment.m_string;
			}
		}

		aOut = prefix;
		aOut += aBaseName;
		aOut += suffix;
	}

}