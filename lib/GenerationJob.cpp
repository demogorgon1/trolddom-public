#include "Pcheader.h"

#include <tpublic/Data/Aura.h>
#include <tpublic/Data/CreatureType.h>
#include <tpublic/Data/Faction.h>
#include <tpublic/Data/LootGroup.h>
#include <tpublic/Data/LootTable.h>
#include <tpublic/Data/NameTemplate.h>
#include <tpublic/Data/Pantheon.h>
#include <tpublic/Data/Tag.h>
#include <tpublic/Data/WordGenerator.h>

#include <tpublic/CreateName.h>
#include <tpublic/DataErrorHandling.h>
#include <tpublic/FloatRange.h>
#include <tpublic/GenerateWord.h>
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
		, m_nextItemNumber(0)
		, m_nextDeityNumber(0)
		, m_nextNPCNumber(0)
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
		m_taggedAuraData = std::make_unique<TaggedDataCache<Data::Aura>>(m_manifest);

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
			else if (aChild->m_name == "item_specials")
				_ReadStackObjectArray(StackObject::TYPE_ITEM_SPECIAL, aChild);
			else if (aChild->m_name == "items")
				_ReadItems(aChild);
			else if (aChild->m_name == "deities")
				_ReadDeities(aChild);
			else if (aChild->m_name == "npcs")
				_ReadNPCs(aChild);
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

		case StackObject::TYPE_ITEM_SPECIAL:
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_tag == "raw_stat")
				{
					Stat::Id statId = Stat::StringToId(aChild->m_name.c_str());
					TP_VERIFY(statId != Stat::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid stat.", aChild->m_name.c_str());
					stackObject->m_itemSpecial.m_rawStats.m_stats[statId] = aChild->GetFloat();
				}
				else if(aChild->m_name == "weight")
				{
					stackObject->m_itemSpecial.m_weight = aChild->GetUInt32();
				}
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				}
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
		std::vector<uint32_t> specialPropabilities;		
		int32_t budgetBias = 0;

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
			else if(aChild->m_name == "special_propabilities")
				aChild->GetUIntArray(specialPropabilities);
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

					Stat::Collection rawStats;

					std::unordered_set<uint32_t> allTagsSet;
					for(uint32_t tagId : allTags)
					{
						const Data::Tag* tag = m_manifest->GetById<Data::Tag>(tagId);

						if(tag->m_transferable)
							allTagsSet.insert(tagId);
					}

					{	
						// Roll for specials
						for (uint32_t propability : specialPropabilities)
						{
							if(_GetRandomIntInRange<uint32_t>(1, 100) < propability)
							{
								const StackObject::ItemSpecial* itemSpecial = _PickItemSpecial();
								if(itemSpecial == NULL)
									break;
								
								rawStats.Add(itemSpecial->m_rawStats);
							}
							else
							{
								break;
							}
						}
					}

					int32_t itemBudgetBias = budgetBias - (int32_t)rawStats.GetTotalBudgetCost();

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

					if(itemBudgetBias != 0)
						output->PrintF(1, "budget_bias: %d", itemBudgetBias);

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
						if(statWeights.m_stats[j] != 0.0f)
						{
							const Stat::Info* statInfo = Stat::GetInfo((Stat::Id)j);
							output->PrintF(1, "stat_weight %s: %f", statInfo->m_name, statWeights.m_stats[j]);
						}

						if(rawStats.m_stats[j] != 0.0f)
						{
							const Stat::Info* statInfo = Stat::GetInfo((Stat::Id)j);
							output->PrintF(1, "stat %s: %f", statInfo->m_name, rawStats.m_stats[j]);
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
	GenerationJob::_ReadDeities(
		const SourceNode*		aSource)
	{
		uint32_t count = 0;
		uint32_t nameWordGeneratorId = 0;
		uint32_t pantheonId = 0;
		std::vector<uint32_t> mustHaveTags;
		std::vector<uint32_t> mustNotHaveTags;
		std::vector<uint32_t> blessingRankTags;
		int32_t blessingDuration = 10 * 60 * 30;

		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "count")
				count = aChild->GetUInt32();
			else if(aChild->m_name == "name_generator")
				nameWordGeneratorId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_WORD_GENERATOR, aChild->GetIdentifier());
			else if (aChild->m_name == "pantheon")
				pantheonId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_PANTHEON, aChild->GetIdentifier());
			else if(aChild->m_name == "must_have_tags")
				aChild->GetIdArray(DataType::ID_TAG, mustHaveTags);
			else if (aChild->m_name == "must_not_have_tags")
				aChild->GetIdArray(DataType::ID_TAG, mustNotHaveTags);
			else if (aChild->m_name == "blessing_rank_tags")
				aChild->GetIdArray(DataType::ID_TAG, blessingRankTags);
			else if (aChild->m_name == "blessing_duration")
				blessingDuration = aChild->GetInt32();
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
		});

		TP_VERIFY(pantheonId != 0, aSource->m_debugInfo, "No pantheon defined.");
		const Data::Pantheon* pantheon = m_manifest->GetById<Data::Pantheon>(pantheonId);

		TP_VERIFY(nameWordGeneratorId != 0, aSource->m_debugInfo, "No name generator defined.");
		const Data::WordGenerator* nameWordGenerator = m_manifest->GetById<Data::WordGenerator>(nameWordGeneratorId);

		for(uint32_t i = 0; i < count; i++)
		{
			std::vector<uint32_t> contextTags;
			_GetContextTags(contextTags);

			std::string name;
			GenerateWord(_GetRandom(), nameWordGenerator, name);

			std::string descriptionNoun;

			std::unordered_set<uint32_t> tags;
			for (uint32_t tagId : contextTags)
				tags.insert(tagId);

			{
				WordList::QueryParams wordListQuery;
				wordListQuery.m_mustHaveTags = mustHaveTags;
				wordListQuery.m_mustHaveTags.push_back(m_manifest->GetExistingIdByName<Data::Tag>("noun"));
				for (uint32_t tagId : contextTags)
					wordListQuery.m_tagScoring.push_back({ tagId, 1 });

				wordListQuery.m_mustNotHaveTags = mustNotHaveTags;

				wordListQuery.Prepare();
				const WordList::Word* word = m_wordListQueryCache->PerformQuery(wordListQuery)->GetRandomWord(_GetRandom());
				if(word != NULL)
				{
					descriptionNoun = word->m_word;

					for(uint32_t tagId : word->m_allTags)
						tags.insert(tagId);
				}
			}

			std::string descriptionAdjective;

			{
				WordList::QueryParams wordListQuery;
				wordListQuery.m_mustHaveTags = mustHaveTags;
				wordListQuery.m_mustHaveTags.push_back(m_manifest->GetExistingIdByName<Data::Tag>("adjective"));
				for (uint32_t tagId : contextTags)
					wordListQuery.m_tagScoring.push_back({ tagId, 1 });

				wordListQuery.m_mustNotHaveTags = mustNotHaveTags;

				wordListQuery.Prepare();
				const WordList::Word* word = m_wordListQueryCache->PerformQuery(wordListQuery)->GetRandomWord(_GetRandom());
				if (word != NULL)
				{
					descriptionAdjective = word->m_word;

					for (uint32_t tagId : word->m_allTags)
						tags.insert(tagId);
				}
			}

			uint32_t roll = _GetRandomIntInRange<uint32_t>(1, 100);

			std::string titlePrefix;
			std::string titleSuffix;
			std::string title;

			if(!descriptionAdjective.empty() && !descriptionNoun.empty())
			{
				if(roll < 15)
				{
					titlePrefix = descriptionAdjective + " ";
					titleSuffix = " the " + descriptionNoun;
				}
				else if (roll < 30)
				{
					titleSuffix = " the " + descriptionAdjective + " " + descriptionNoun;
				}
				else if (roll < 50)
				{
					titleSuffix = " the " + descriptionAdjective;
				}
				else if (roll < 70)
				{
					titleSuffix = " the " + descriptionNoun;
				}
				else if (roll < 90)
				{
					titleSuffix = ", " + descriptionAdjective + " " + descriptionNoun;
				}
				else
				{
					titlePrefix = descriptionAdjective + " ";
				}

				title = descriptionAdjective + " " + descriptionNoun;
			}

			std::vector<std::string> localBlessingRankNames;
			uint32_t blessingRank = 0;

			Data::Aura combinedAura;
			combinedAura.m_string = Helpers::Format("Blessing of %s", name.c_str());
			combinedAura.m_iconSpriteId = pantheon->m_iconSpriteId;
			combinedAura.m_duration = blessingDuration;
			combinedAura.m_type = Data::Aura::TYPE_BUFF;
			combinedAura.m_flags = Data::Aura::FLAG_UNIQUE;

			for(uint32_t blessingRankTagId : blessingRankTags)
			{
				TaggedData::Query query;
				query.m_mustHaveTagIds = { blessingRankTagId };

				for (uint32_t tagId : contextTags)
				{
					TaggedData::Query::TagScoring scoring;
					scoring.m_tagId = tagId;
					scoring.m_score = 1;
					query.m_tagScoring.push_back(scoring);
				}

				const TaggedData::QueryResult* result = m_taggedAuraData->Get()->PerformQuery(query);
				TP_VERIFY(result->m_entries.size() > 0, aSource->m_debugInfo, "Unable to get blessing aura for rank tag id: %u", blessingRankTagId);

				uint32_t auraId = result->PickRandom(_GetRandom())->m_id;
				const Data::Aura* auraData = m_manifest->GetById<Data::Aura>(auraId);

				combinedAura.m_description.clear();

				if(auraData->m_statModifiers)
				{
					if(!combinedAura.m_statModifiers)
						combinedAura.m_statModifiers = std::make_unique<StatModifiers>();

					combinedAura.m_statModifiers->Combine(*auraData->m_statModifiers);

					for (uint32_t k = 1; k < (uint32_t)tpublic::Stat::NUM_IDS; k++)
					{
						const std::optional<tpublic::Modifier>& modifier = auraData->m_statModifiers->m_modifiers[k];
						if (modifier.has_value())
						{
							const tpublic::Stat::Info* statInfo = tpublic::Stat::GetInfo((tpublic::Stat::Id)k);
							int32_t add = (int32_t)modifier->m_add;
							int32_t addP = (int32_t)modifier->m_addPercent;
							if (add > 0)
								combinedAura.m_description += Helpers::Format("+%d %s\n", add, statInfo->m_longName);
							else if (add < 0)
								combinedAura.m_description += Helpers::Format("%d %s\n", add, statInfo->m_longName);

							if (addP > 0)
								combinedAura.m_description += Helpers::Format("+%d%% %s\n", add, statInfo->m_longName);
							else if (addP < 0)
								combinedAura.m_description += Helpers::Format("%d%% %s\n", add, statInfo->m_longName);
						}
					}
				}

				// FIXME: copy aura effects				

				std::vector<uint8_t> binary;
				VectorIO::Writer writer(binary);
				combinedAura.ToStream(&writer);
				std::string base64;
				Base64::Encode(&binary[0], binary.size(), base64);
								
				GeneratedSource* output = _CreateGeneratedSource();
				std::string auraName = Helpers::Format(".%s_deity_%u_blessing_rank_%u", m_source->m_name.c_str(), m_nextDeityNumber, blessingRank);
				localBlessingRankNames.push_back(auraName);

				output->PrintF(0, "aura %s: { binary: \"%s\" }", auraName.c_str(), base64.c_str());

				blessingRank++;
			}

			// Deity source
			{
				GeneratedSource* output = _CreateGeneratedSource();

				output->PrintF(0, "deity %s_deity_%u:", m_source->m_name.c_str(), m_nextDeityNumber);
				output->PrintF(0, "{");
				output->PrintF(1, "string: \"%s\"", name.c_str());

				if (!title.empty())
					output->PrintF(1, "title: \"%s\"", title.c_str());

				if (!titlePrefix.empty())
					output->PrintF(1, "title_prefix: \"%s\"", titlePrefix.c_str());

				if (!titleSuffix.empty())
					output->PrintF(1, "title_suffix: \"%s\"", titleSuffix.c_str());

				if (tags.size() > 0)
				{
					output->PrintF(1, "tags:");
					output->PrintF(1, "[");
					for (uint32_t tagId : tags)
					{
						const Data::Tag* dataTag = m_manifest->GetById<Data::Tag>(tagId);
						if (dataTag->m_transferable)
							output->PrintF(2, "%s", dataTag->m_name.c_str());
					}
					output->PrintF(1, "]");
				}

				output->PrintF(1, "pantheon: %s", pantheon->m_name.c_str());

				output->PrintF(1, "blessing_auras:");
				output->PrintF(1, "[");
				for (const std::string& localBlessingRankName : localBlessingRankNames)
				{
					output->PrintF(2, "%s", localBlessingRankName.c_str());
				}
				output->PrintF(1, "]");

				output->PrintF(0, "}");
			}

			m_nextDeityNumber++;
		}
	}

	void
	GenerationJob::_ReadNPCs(
		const SourceNode*	aSource)
	{
		uint32_t count = 0;
		uint32_t nameTemplateId = 0;
		uint32_t spriteTagContextId = 0;
		uint32_t creatureTypeId = 0;
		FloatRange weaponDamageMultiplierRange = { 1.0f, 1.0f };
		FloatRange healthMultiplierRange = { 1.0f, 1.0f };
		bool elite = false;
		uint32_t factionId = 0;
		uint32_t lootTableId = 0;
		std::vector<uint32_t> extraTags;

		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if (aChild->m_name == "count")
				count = aChild->GetUInt32();
			else if (aChild->m_name == "elite")
				elite = aChild->GetBool();
			else if (aChild->m_name == "weapon_damage_multiplier_range")
				weaponDamageMultiplierRange = FloatRange(aChild);
			else if (aChild->m_name == "health_multiplier_range")
				healthMultiplierRange = FloatRange(aChild);
			else if (aChild->m_name == "name_template")
				nameTemplateId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_NAME_TEMPLATE, aChild->GetIdentifier());
			else if (aChild->m_name == "faction")
				factionId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_FACTION, aChild->GetIdentifier());
			else if (aChild->m_name == "sprite")
				spriteTagContextId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TAG_CONTEXT, aChild->GetIdentifier());
			else if (aChild->m_name == "creature_type")
				creatureTypeId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_CREATURE_TYPE, aChild->GetIdentifier());
			else if (aChild->m_name == "loot_table")
				lootTableId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_LOOT_TABLE, aChild->GetIdentifier());
			else if (aChild->m_name == "extra_tags")
				aChild->GetIdArray(DataType::ID_TAG, extraTags);
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
		});

		TP_VERIFY(nameTemplateId != 0, aSource->m_debugInfo, "No name template defined.");
		const Data::NameTemplate* nameTemplate = m_manifest->GetById<Data::NameTemplate>(nameTemplateId);

		TP_VERIFY(creatureTypeId != 0, aSource->m_debugInfo, "No creature type defined.");
		const Data::CreatureType* creatureType = m_manifest->GetById<Data::CreatureType>(creatureTypeId);

		TP_VERIFY(factionId != 0, aSource->m_debugInfo, "No faction defined.");
		const Data::Faction* faction = m_manifest->GetById<Data::Faction>(factionId);

		TP_VERIFY(lootTableId != 0, aSource->m_debugInfo, "No loot table defined.");
		const Data::LootTable* lootTable = m_manifest->GetById<Data::LootTable>(lootTableId);

		TP_VERIFY(spriteTagContextId != 0, aSource->m_debugInfo, "No sprite tag context defined.");
		const Data::TagContext* spriteTagContext = m_manifest->GetById<Data::TagContext>(spriteTagContextId);
		const TaggedData::QueryResult* spriteQueryResult = m_taggedSpriteData->Get()->PerformQueryWithTagContext(spriteTagContext);

		for(uint32_t i = 0; i < count; i++)
		{
			const UIntRange& levelRange = _GetLevelRange();
			uint32_t level = _GetRandomIntInRange(levelRange.m_min, levelRange.m_max);

			std::unordered_set<uint32_t> tags;
			for (uint32_t tagId : extraTags)
				tags.insert(tagId);

			std::string name;
			CreateName(m_manifest, nameTemplate, m_wordListQueryCache.get(), _GetRandom(), name, tags);

			const TaggedData::QueryResult::Entry* spriteEntry = spriteQueryResult->PickRandom(_GetRandom());
			const Data::Sprite* sprite = m_manifest->GetById<Data::Sprite>(spriteEntry->m_id);
			const Data::Sprite* spriteDead = m_manifest->GetById<Data::Sprite>(sprite->m_info.m_deadSpriteId);

			for(uint32_t tagId : spriteEntry->m_tags)
				tags.insert(tagId);

			float weaponDamageMultiplier = weaponDamageMultiplierRange.GetRandom(_GetRandom());
			float healthMultiplier = healthMultiplierRange.GetRandom(_GetRandom());

			{
				GeneratedSource* output = _CreateGeneratedSource();

				output->PrintF(0, "entity %s_npc_%u: !NPC", m_source->m_name.c_str(), m_nextNPCNumber);
				output->PrintF(0, "{");
				output->PrintF(1, "_string: \"%s\"", name.c_str());
				output->PrintF(1, "_level: %u", level);
				output->PrintF(1, "_elite: %s", elite ? "true" : "false");
				output->PrintF(1, "_faction: %s", faction->m_name.c_str());
				output->PrintF(1, "_sprite: %s", sprite->m_name.c_str());
				output->PrintF(1, "_sprite_dead: %s", spriteDead->m_name.c_str());
				output->PrintF(1, "_weapon_damage: %f", weaponDamageMultiplier);
				output->PrintF(1, "_resource_health: %f", healthMultiplier);
				output->PrintF(1, "_creature_type: %s", creatureType->m_name.c_str());
				output->PrintF(1, "_loot_table: %s", lootTable->m_name.c_str());

				if (tags.size() > 0)
				{
					output->PrintF(1, "tags:");
					output->PrintF(1, "[");
					for (uint32_t tagId : tags)
					{
						const Data::Tag* dataTag = m_manifest->GetById<Data::Tag>(tagId);
						output->PrintF(2, "%s", dataTag->m_name.c_str());
					}
					output->PrintF(1, "]");
				}

				output->PrintF(0, "}");
			}

			m_nextNPCNumber++;
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
		uint32_t iconSpriteId = result->PickRandom(_GetRandom())->m_id;

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

	const GenerationJob::StackObject::ItemSpecial*
	GenerationJob::_PickItemSpecial()
	{
		WeightedRandom<const StackObject::ItemSpecial*> possibilities;

		for (const std::unique_ptr<StackObject>& stackObject : m_stack)
		{
			if (stackObject->m_type == StackObject::TYPE_ITEM_SPECIAL)
				possibilities.AddPossibility(stackObject->m_itemSpecial.m_weight, &stackObject->m_itemSpecial);
		}

		const StackObject::ItemSpecial* picked;
		if(!possibilities.Pick(_GetRandom(), picked))
			return NULL;

		return picked;
	}

}