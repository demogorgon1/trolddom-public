#include "Pcheader.h"

#include <tpublic/Data/Aura.h>
#include <tpublic/Data/CreatureType.h>
#include <tpublic/Data/Faction.h>
#include <tpublic/Data/LootGroup.h>
#include <tpublic/Data/LootTable.h>
#include <tpublic/Data/NameTemplate.h>
#include <tpublic/Data/Pantheon.h>
#include <tpublic/Data/Profession.h>
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
		m_taggedItemData = std::make_unique<TaggedDataCache<Data::Item>>(m_manifest);

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
			else if (aChild->m_name == "abilities")
				_ReadStackObjectArray(StackObject::TYPE_ABILITY, aChild);
			else if (aChild->m_name == "weapon_speeds")
				_ReadStackObjectArray(StackObject::TYPE_WEAPON_SPEED, aChild);
			else if (aChild->m_name == "item_suffixes")
				_ReadStackObjectArray(StackObject::TYPE_ITEM_SUFFIX, aChild);
			else if (aChild->m_name == "item_prefixes")
				_ReadStackObjectArray(StackObject::TYPE_ITEM_PREFIX, aChild);
			else if (aChild->m_name == "equipment_slots")
				_ReadStackObjectArray(StackObject::TYPE_EQUIPMENT_SLOT, aChild);
			else if (aChild->m_name == "loot_groups")
				_ReadStackObjectArray(StackObject::TYPE_LOOT_GROUP, aChild);
			else if (aChild->m_name == "items")
				_ReadItems(aChild);
			else if (aChild->m_name == "deities")
				_ReadDeities(aChild);
			else if (aChild->m_name == "npcs")
				_ReadNPCs(aChild);
			else if (aChild->m_name == "crafting")
				_ReadCrafting(aChild);
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
		case StackObject::TYPE_LOOT_GROUP:
			{
				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (aChild->m_name == "loot_group")
						stackObject->m_lootGroup.m_lootGroupId = aChild->GetId(DataType::ID_LOOT_GROUP);
					else if (aChild->m_name == "rarity")
						stackObject->m_lootGroup.m_rarity = Rarity::StringToId(aChild->GetIdentifier());
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}
			break;

		case StackObject::TYPE_EQUIPMENT_SLOT:
			{
				stackObject->m_equipmentSlot = EquipmentSlot::StringToId(aSource->GetIdentifier());
				TP_VERIFY(stackObject->m_equipmentSlot != EquipmentSlot::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid equipment slot.", aSource->GetIdentifier());
			}
			break;

		case StackObject::TYPE_ITEM_PREFIX:
			{
				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (aChild->m_name == "level_range")
						stackObject->m_itemPrefix.m_levelRange = UIntRange(aChild);
					else if (aChild->m_name == "rarity")
						stackObject->m_itemPrefix.m_rarity = Rarity::StringToId(aChild->GetIdentifier());
					else if (aChild->m_name == "string")
						stackObject->m_itemPrefix.m_string = aChild->GetString();
					else if (aChild->m_name == "material_multiplier")
						stackObject->m_itemPrefix.m_materialMultiplier = aChild->GetFloat();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}
			break;

		case StackObject::TYPE_ITEM_SUFFIX:
			{
				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (aChild->m_name == "string")
					{
						stackObject->m_itemSuffix.m_string = aChild->GetString();
					}
					else if (aChild->m_name == "budget_bias")
					{
						stackObject->m_itemSuffix.m_budgetBias = aChild->GetInt32();
					}
					else
					{
						Stat::Id id = Stat::StringToId(aChild->m_name.c_str());
						TP_VERIFY(id != Stat::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid stat or item.", aChild->m_name.c_str());
						stackObject->m_itemSuffix.m_stats.m_stats[id] = aChild->GetFloat();
					}
				});
			}
			break;

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

		case StackObject::TYPE_ABILITY:
			{
				if(aSource->m_type == SourceNode::TYPE_OBJECT)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if (aChild->m_name == "target" && aChild->IsIdentifier("random_player"))
							stackObject->m_ability.m_targetType = Components::NPC::AbilityEntry::TARGET_TYPE_RANDOM_PLAYER;
						else if (aChild->m_name == "target" && aChild->IsIdentifier("self"))
							stackObject->m_ability.m_targetType = Components::NPC::AbilityEntry::TARGET_TYPE_SELF;
						else if (aChild->m_name == "target" && aChild->IsIdentifier("low_health_friend_or_self"))
							stackObject->m_ability.m_targetType = Components::NPC::AbilityEntry::TARGET_TYPE_LOW_HEALTH_FRIEND_OR_SELF;
						else if(aChild->m_name == "id")
							stackObject->m_ability.m_abilityId = aChild->GetId(DataType::ID_ABILITY);
						else if(aChild->m_name == "target_must_not_have_aura")
							stackObject->m_ability.m_targetMustNotHaveAuraId = aChild->GetId(DataType::ID_AURA);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}
				else
				{
					stackObject->m_ability.m_abilityId = aSource->GetId(DataType::ID_ABILITY);
				}				
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
				else if (aChild->m_name == "min_rarity")
					stackObject->m_itemClass.m_minRarity = Rarity::StringToId(aChild->GetIdentifier());
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

		case StackObject::TYPE_WEAPON_SPEED:			
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "speed")
					stackObject->m_weaponSpeed.m_speed = aChild->GetInt32();
				else if (aChild->m_name == "weight")
					stackObject->m_weaponSpeed.m_weight = aChild->GetUInt32();
				else if (aChild->m_name == "types")
					aChild->GetIdArrayWithLookup<ItemType::Id, ItemType::INVALID_ID>(stackObject->m_weaponSpeed.m_types, [](const char* aString) { return ItemType::StringToId(aString); });
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
				else if(aChild->m_name == "exclude_item_types")
				{
					aChild->GetIdArrayWithLookup<ItemType::Id, ItemType::INVALID_ID>(stackObject->m_itemSpecial.m_excludeItemTypes, [](const char* aString) { return ItemType::StringToId(aString); });
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
		
		enum Type
		{
			INVALID_TYPE,

			TYPE_WEAPONS_AND_SHIELDS,
			TYPE_ARMOR_AND_JEWELRY
		};

		Type type = INVALID_TYPE;

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
			else if (aChild->m_name == "type" && aChild->IsIdentifier("weapons_and_shields"))
				type = TYPE_WEAPONS_AND_SHIELDS;
			else if (aChild->m_name == "type" && aChild->IsIdentifier("armor_and_jewelry"))
				type = TYPE_ARMOR_AND_JEWELRY;
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
		});

		TP_VERIFY(rarity != Rarity::INVALID_ID, aSource->m_debugInfo, "Invalid rarity.");
		TP_VERIFY(lootGroups.size() > 0, aSource->m_debugInfo, "No loot groups.");
		TP_VERIFY(type != INVALID_TYPE, aSource->m_debugInfo, "No type.");

		std::unordered_set<std::string> createdItemNames;

		for(uint32_t i = 0; i < count; i++)
		{
			const UIntRange& levelRange = _GetLevelRange();
			uint32_t itemLevel = _GetRandomIntInRange(levelRange.m_min, levelRange.m_max);

			WeightedRandom<const StackObject::ItemClass*> possibleItemClasses;
			WeightedRandom<const StackObject::Designation*> possibleDesignations;			

			for (const std::unique_ptr<StackObject>& stackObject : m_stack)
			{
				if (stackObject->m_type == StackObject::TYPE_ITEM_CLASS)
				{
					const StackObject::ItemClass* itemClass = &stackObject->m_itemClass;

					if (itemClass->m_minLevel != 0 && itemLevel < itemClass->m_minLevel)
						continue;

					if (itemClass->m_minRarity != Rarity::INVALID_ID && (uint32_t)rarity < (uint32_t)itemClass->m_minRarity)
						continue;

					if (itemClass->m_maxLevel != 0 && itemLevel > itemClass->m_maxLevel)
						continue;

					possibleItemClasses.AddPossibility(itemClass->m_weight, itemClass);
				}
				else if (stackObject->m_type == StackObject::TYPE_DESIGNATION)
				{
					const StackObject::Designation* designation = &stackObject->m_designation;

					possibleDesignations.AddPossibility(designation->m_weight, designation);
				}
			}

			const StackObject::ItemClass* itemClass = NULL;
			const StackObject::Designation* designation = NULL;

			if(possibleItemClasses.Pick(_GetRandom(), itemClass) && possibleDesignations.Pick(_GetRandom(), designation))
			{
				ItemType::Id itemType = ItemType::ID_NONE;
				std::unordered_set<uint32_t> mustHaveTags;

				if(itemClass->m_types.size() > 0)
				{
					itemType = _GetRandomItemInVector(itemClass->m_types);

					uint32_t itemTypeTagId = m_manifest->TryGetExistingIdByName<Data::Tag>(ItemType::GetInfo(itemType)->m_name);
					if(itemTypeTagId != 0)
						mustHaveTags.insert(itemTypeTagId);
				}				

				const StackObject::WeaponSpeed* weaponSpeed = NULL;

				if (type == TYPE_WEAPONS_AND_SHIELDS)
				{
					WeightedRandom<const StackObject::WeaponSpeed*> possibleWeaponSpeeds;
					for (const std::unique_ptr<StackObject>& stackObject : m_stack)
					{
						if (stackObject->m_type == StackObject::TYPE_WEAPON_SPEED && stackObject->m_weaponSpeed.HasType(itemType))
							possibleWeaponSpeeds.AddPossibility(stackObject->m_weaponSpeed.m_weight, &stackObject->m_weaponSpeed);
					}
					possibleWeaponSpeeds.Pick(_GetRandom(), weaponSpeed);
				}

				std::vector<uint32_t> allTags;
				Stat::Collection rawStats;
				std::string itemString;
				std::unordered_set<uint32_t> allTagsSet;
				Stat::Collection statWeights;

				if(type == TYPE_ARMOR_AND_JEWELRY)
				{
					for (EquipmentSlot::Id equipmentSlot : itemClass->m_slots)
					{
						uint32_t equipmentSlotTagId = m_manifest->TryGetExistingIdByName<Data::Tag>(EquipmentSlot::GetInfo(equipmentSlot)->m_tag);
						if (equipmentSlotTagId != 0)
						{
							mustHaveTags.insert(equipmentSlotTagId);
						}
					}
				}

				WordList::QueryParams wordListQuery;
				for (uint32_t tagId : mustHaveTags)
					wordListQuery.m_mustHaveTags.push_back(tagId);
				wordListQuery.Prepare();
				const WordList::Word* baseNameWord = m_wordListQueryCache->PerformQuery(wordListQuery)->GetRandomWord(_GetRandom());

				if(baseNameWord != NULL)
				{
					std::vector<uint32_t> contextTags;
					_GetContextTags(contextTags);

					allTags = baseNameWord->m_allTags;

					for(;;)
					{
						_CreateDesignation(baseNameWord->m_word.c_str(), designation, contextTags, allTags, statWeights, itemString);

						// Check we didn't already create an item with this name
						uint32_t existingIdWithName = m_manifest->TryGetExistingIdByName<Data::Item>(itemString.c_str());
						if(existingIdWithName == 0 && !createdItemNames.contains(itemString))
						{
							createdItemNames.insert(itemString);
							break;
						}
					}

					if(statWeights.IsEmpty())
					{
						std::unordered_set<Stat::Id> excludedStats;

						// FIXME: would be nicer to define this rule in data instead (cloth armor (besides capes) shouldn't have block value
						bool isBackSlot = (itemClass->m_slots.size() == 1 && itemClass->m_slots[0] == EquipmentSlot::ID_BACK);
						if(itemType == ItemType::ID_ARMOR_CLOTH && !isBackSlot)
							excludedStats.insert(Stat::ID_BLOCK_VALUE);

						Helpers::GetRandomStatWeights(excludedStats, _GetRandom()(), statWeights);
					}

					statWeights.RemoveLowStats(maxDifferentStats);
					statWeights.Add(baseStatWeights);

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
								const StackObject::ItemSpecial* itemSpecial = _PickItemSpecial(itemType);
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

					const char* iconName = _PickIconName(itemLevel, rarity, mustHaveTags, allTags);
					int32_t itemBudgetBias = budgetBias - (int32_t)rawStats.GetTotalBudgetCost();

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

					if (itemBudgetBias != 0)
						output->PrintF(1, "budget_bias: %d", itemBudgetBias);

					if(weaponSpeed != NULL && itemType != ItemType::ID_SHIELD)
						output->PrintF(1, "weapon_cooldown: %d", weaponSpeed->m_speed);

					if (itemClass->m_slots.size() > 0)
					{
						output->PrintF(1, "equipment_slots:");
						output->PrintF(1, "[");
						for (EquipmentSlot::Id equipmentSlot : itemClass->m_slots)
							output->PrintF(2, "%s", EquipmentSlot::GetInfo(equipmentSlot)->m_name);
						output->PrintF(1, "]");
					}

					output->PrintF(1, "tags:");
					output->PrintF(1, "[");
					for (uint32_t tagId : allTagsSet)
						output->PrintF(2, "%s", m_manifest->GetById<Data::Tag>(tagId)->m_name.c_str());
					output->PrintF(1, "]");

					for (uint32_t j = 1; j < (uint32_t)Stat::NUM_IDS; j++)
					{
						if (statWeights.m_stats[j] != 0.0f)
						{
							const Stat::Info* statInfo = Stat::GetInfo((Stat::Id)j);
							output->PrintF(1, "stat_weight %s: %f", statInfo->m_name, statWeights.m_stats[j]);
						}

						if (rawStats.m_stats[j] != 0.0f)
						{
							const Stat::Info* statInfo = Stat::GetInfo((Stat::Id)j);
							output->PrintF(1, "stat %s: %f", statInfo->m_name, rawStats.m_stats[j]);
						}
					}

					output->PrintF(1, "loot_groups:");
					output->PrintF(1, "[");
					for (uint32_t lootGroupId : lootGroups)
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
				nameWordGeneratorId = aChild->GetId(DataType::ID_WORD_GENERATOR);
			else if (aChild->m_name == "pantheon")
				pantheonId = aChild->GetId(DataType::ID_PANTHEON);
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
			combinedAura.m_flags = Data::Aura::FLAG_UNIQUE | Data::Aura::FLAG_HIDE_DESCRIPTION | Data::Aura::FLAG_BLESSING;

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
		UIntRange abilityCount;
		bool hasMana = false;
		uint32_t specialLootCooldownSeconds = 0; 

		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if (aChild->m_name == "count")
				count = aChild->GetUInt32();
			else if (aChild->m_name == "elite")
				elite = aChild->GetBool();
			else if (aChild->m_name == "has_mana")
				hasMana = aChild->GetBool();
			else if (aChild->m_name == "weapon_damage_multiplier_range")
				weaponDamageMultiplierRange = FloatRange(aChild);
			else if (aChild->m_name == "health_multiplier_range")
				healthMultiplierRange = FloatRange(aChild);
			else if (aChild->m_name == "name_template")
				nameTemplateId = aChild->GetId(DataType::ID_NAME_TEMPLATE);
			else if (aChild->m_name == "faction")
				factionId = aChild->GetId(DataType::ID_FACTION);
			else if (aChild->m_name == "sprite")
				spriteTagContextId = aChild->GetId(DataType::ID_TAG_CONTEXT);
			else if (aChild->m_name == "creature_type")
				creatureTypeId = aChild->GetId(DataType::ID_CREATURE_TYPE);
			else if (aChild->m_name == "loot_table")
				lootTableId = aChild->GetId(DataType::ID_LOOT_TABLE);
			else if (aChild->m_name == "extra_tags")
				aChild->GetIdArray(DataType::ID_TAG, extraTags);
			else if(aChild->m_name == "abilities")
				abilityCount = UIntRange(aChild);
			else if(aChild->m_name == "special_loot_cooldown_seconds")	
				specialLootCooldownSeconds = aChild->GetUInt32();				
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

			std::vector<const StackObject::Ability*> abilities;
			uint32_t numAbilities = abilityCount.GetRandom(_GetRandom());
			if(numAbilities > 0)
			{
				std::vector<const StackObject::Ability*> availableAbilities;
				_GetAbilities(availableAbilities);
				for(size_t j = 0; j < availableAbilities.size(); j++)
				{
					const Data::Ability* abilityData = m_manifest->GetById<Data::Ability>(availableAbilities[j]->m_abilityId);
				
					if(abilityData->m_npcLevelRange.has_value())
					{
						if (level < abilityData->m_npcLevelRange->m_min || level > abilityData->m_npcLevelRange->m_max)
						{
							Helpers::RemoveCyclicFromVector(availableAbilities, j);
							j--;
						}
					}
				}

				std::unordered_set<const StackObject::Ability*> abilitySet;
				std::vector<const StackObject::Ability*> tmp = availableAbilities;

				for (uint32_t j = 0; j < numAbilities; j++)
				{
					const StackObject::Ability* pickedAbility;
					if(Helpers::GetAndRemoveCyclicFromVector(_GetRandom(), tmp, pickedAbility))
						abilitySet.insert(pickedAbility);
				}

				for (size_t j = 0; j < availableAbilities.size(); j++)
				{
					const StackObject::Ability* availableAbility = availableAbilities[j];
					if(abilitySet.contains(availableAbility))
						abilities.push_back(availableAbility);
				}				
			}

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

				if(hasMana) 
					output->PrintF(1, "_resource_mana: 1");

				output->PrintF(1, "_creature_type: %s", creatureType->m_name.c_str());
				output->PrintF(1, "_loot_table: %s", lootTable->m_name.c_str());

				if(specialLootCooldownSeconds != 0)
					output->PrintF(1, "_special_loot_cooldown: $loot_cooldown { seconds: %u }", specialLootCooldownSeconds);

				output->PrintF(1, "_abilities:");
				output->PrintF(1, "[");
				for(const StackObject::Ability* ability : abilities)
				{
					const Data::Ability* abilityData = m_manifest->GetById<Data::Ability>(ability->m_abilityId);

					output->PrintF(2, "{");
					
					output->PrintF(3, "id: %s", abilityData->m_name.c_str());
					if(ability->m_targetType == Components::NPC::AbilityEntry::TARGET_TYPE_LOW_HEALTH_FRIEND_OR_SELF)
						output->PrintF(3, "target: low_health_friend_or_self");
					else if (ability->m_targetType == Components::NPC::AbilityEntry::TARGET_TYPE_RANDOM_PLAYER)
						output->PrintF(3, "target: random_player");
					else if (ability->m_targetType == Components::NPC::AbilityEntry::TARGET_TYPE_SELF)
						output->PrintF(3, "target: self");
					
					if(ability->m_targetMustNotHaveAuraId != 0)
						output->PrintF(3, "requirement target<must_not_have_aura>: %s", m_manifest->GetById<Data::Aura>(ability->m_targetMustNotHaveAuraId)->m_name.c_str());

					output->PrintF(2, "}");
				}
				output->PrintF(2, "{");
				output->PrintF(3, "id: npc_attack");
				output->PrintF(2, "}");
				output->PrintF(1, "]");

				if (tags.size() > 0)
				{
					output->PrintF(1, "_tags:");
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
	GenerationJob::_ReadCrafting(
		const SourceNode*	aSource)
	{
		uint32_t professionId = 0;
		ItemType::Id itemType = ItemType::INVALID_ID;
		std::string craftMacro;
		std::string learnMacro;
		std::string recipeMacro;
		std::string recipePrefix = "Recipe";
		std::unordered_map<std::string, bool> craftBools;
		std::unordered_map<std::string, uint32_t> baseMaterials;
		uint32_t reagentTagId = m_manifest->TryGetExistingIdByName<Data::Tag>("reagent");

		aSource->GetObject()->ForEachChild([&](
			const SourceNode* aChild)
		{
			if (aChild->m_name == "profession")
				professionId = aChild->GetId(DataType::ID_PROFESSION);
			else if (aChild->m_name == "item_type")
				itemType = ItemType::StringToId(aChild->GetIdentifier());
			else if (aChild->m_name == "craft_macro")
				craftMacro = aChild->GetIdentifier();
			else if (aChild->m_name == "recipe_macro")
				recipeMacro = aChild->GetIdentifier();
			else if (aChild->m_name == "learn_macro")
				learnMacro = aChild->GetIdentifier();
			else if (aChild->m_name == "recipe_prefix")
				recipePrefix = aChild->GetString();
			else if(aChild->m_tag == "craft")
				craftBools[aChild->m_name] = aChild->GetBool();
			else if (aChild->m_tag == "base_material")
				baseMaterials[aChild->m_name] = aChild->GetUInt32();
			else if(aChild->m_name == "reagent_tag")
				reagentTagId = aChild->GetId(DataType::ID_TAG);
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
		});

		TP_VERIFY(itemType != ItemType::INVALID_ID, aSource->m_debugInfo, "No item type defined.");
		TP_VERIFY(!craftMacro.empty() && !recipeMacro.empty() && !learnMacro.empty(), aSource->m_debugInfo, "Missing macros.");
		TP_VERIFY(professionId != 0, aSource->m_debugInfo, "No profession defined.");
		const Data::Profession* profession = m_manifest->GetById<Data::Profession>(professionId);
		uint32_t professionTagId = m_manifest->TryGetExistingIdByName<Data::Tag>(profession->m_name.c_str());

		std::vector<EquipmentSlot::Id> equipmentSlots;
		_GetEquipmentSlots(equipmentSlots);

		std::vector<const StackObject::ItemPrefix*> itemPrefixes;
		_GetItemPrefixes(itemPrefixes);

		std::vector<const StackObject::ItemSuffix*> itemSuffixes;
		_GetItemSuffixes(itemSuffixes);

		const char* itemTypeName = ItemType::GetInfo(itemType)->m_name;
		uint32_t itemTypeTagId = m_manifest->TryGetExistingIdByName<Data::Tag>(itemTypeName);
		TP_VERIFY(itemTypeTagId != 0, aSource->m_debugInfo, "No item type tag.");

		std::unordered_set<std::string> tt;

		for(EquipmentSlot::Id equipmentSlot : equipmentSlots)
		{
			const EquipmentSlot::Info* equipmentSlotInfo = EquipmentSlot::GetInfo(equipmentSlot);
			uint32_t equipmentSlotTagId = m_manifest->TryGetExistingIdByName<Data::Tag>(equipmentSlotInfo->m_tag);
			TP_VERIFY(equipmentSlotTagId != 0, aSource->m_debugInfo, "No equipment slot tag.");

			for (const StackObject::ItemSuffix* itemSuffix : itemSuffixes)
			{
				for (const StackObject::ItemPrefix* itemPrefix : itemPrefixes)
				{
					TP_VERIFY(itemPrefix->m_rarity != Rarity::INVALID_ID, aSource->m_debugInfo, "No rarity defined for item prefix.");

					std::vector<uint32_t> lootGroups;
					_GetLootGroups(itemPrefix->m_rarity, lootGroups);

					std::string lootGroupsString;
					for (uint32_t lootGroupId : lootGroups)
					{	
						if(!lootGroupsString.empty())
							lootGroupsString += " ";
						lootGroupsString += m_manifest->GetById<Data::LootGroup>(lootGroupId)->m_name;
					}

					uint32_t level = itemPrefix->m_levelRange.GetRandom(_GetRandom());

					uint32_t skill = level * 5;

					std::unordered_map<std::string, uint32_t> materials = baseMaterials;

					uint32_t commonMaterialCount = 1 + (uint32_t)(itemPrefix->m_materialMultiplier * (float)level);
					const char* commonMaterialName = _PickMaterialName(level, Rarity::ID_COMMON, professionTagId);
					assert(commonMaterialName != NULL);
					materials[commonMaterialName] = commonMaterialCount; 

					if((uint32_t)itemPrefix->m_rarity >= (uint32_t)Rarity::ID_UNCOMMON)
					{
						const char* reagentName = _PickMaterialName(level, Rarity::ID_COMMON, reagentTagId);
						uint32_t count = _GetRandomIntInRange<uint32_t>(1, 2);

						if ((uint32_t)itemPrefix->m_rarity >= (uint32_t)Rarity::ID_RARE)
							count *= _GetRandomIntInRange<uint32_t>(2, 3);

						materials[reagentName] = count;
					}
					
					if((uint32_t)itemPrefix->m_rarity >= (uint32_t)Rarity::ID_RARE)
					{
						const char* reagentName = _PickMaterialName(level, Rarity::ID_UNCOMMON, reagentTagId);
						materials[reagentName] = _GetRandomIntInRange<uint32_t>(1, 2);
					}

					std::string itemString;

					{
						WordList::QueryParams wordListQuery;
						wordListQuery.m_mustHaveTags.push_back(itemTypeTagId);
						wordListQuery.m_mustHaveTags.push_back(equipmentSlotTagId);
						wordListQuery.Prepare();
						const WordList::Word* baseNameWord = m_wordListQueryCache->PerformQuery(wordListQuery)->GetRandomWord(_GetRandom());
						TP_VERIFY(!baseNameWord->m_word.empty(), aSource->m_debugInfo, "Unable to pick word.");
						itemString = itemPrefix->m_string + " " + baseNameWord->m_word + " " + itemSuffix->m_string;
					}

					const char* iconSpriteName = NULL;

					{
						std::unordered_set<uint32_t> mustHaveTags;
						std::vector<uint32_t> allTags;

						mustHaveTags.insert(itemTypeTagId);
						mustHaveTags.insert(equipmentSlotTagId);

						iconSpriteName = _PickIconName(level, itemPrefix->m_rarity, mustHaveTags, allTags);
					}
				
					std::string itemName = Helpers::Format("_%u_%u_%u_%08x%08x", profession->m_id, equipmentSlot, itemType, itemPrefix->GetHash(), itemSuffix->GetHash());

					// Item source
					{
						GeneratedSource* output = _CreateGeneratedSource();

						output->PrintF(0, "item %s:", itemName.c_str());
						output->PrintF(0, "{");
						output->PrintF(1, "string: \"%s\"", itemString.c_str());
						output->PrintF(1, "required_level: %u", level);
						output->PrintF(1, "rarity: %s", Rarity::GetInfo(itemPrefix->m_rarity)->m_name);
						output->PrintF(1, "binds: when_equipped");
						output->PrintF(1, "equipment_slots: [ %s ]", equipmentSlotInfo->m_name);
						output->PrintF(1, "type: %s", itemTypeName);
						output->PrintF(1, "icon: %s", iconSpriteName);
						
						if(itemSuffix->m_budgetBias != 0)
							output->PrintF(1, "budget_bias: %d", itemSuffix->m_budgetBias);

						for(uint32_t i = 1; i < (uint32_t)Stat::NUM_IDS; i++)
						{
							float value = itemSuffix->m_stats.m_stats[i];
							if(value > 0.0f)
							{
								Stat::Id statId = (Stat::Id)i;
								const Stat::Info* statInfo = Stat::GetInfo(statId);

								output->PrintF(1, "%s %s: %.0f", statInfo->m_percentage ? "stat" : "stat_weight", statInfo->m_name, value);
							}
						}

						output->PrintF(0, "}");
					}

					// Craft ability source
					{
						GeneratedSource* output = _CreateGeneratedSource();

						output->PrintF(0, "ability c%s: !%s", itemName.c_str(), craftMacro.c_str());
						output->PrintF(0, "{");
						output->PrintF(1, "_string: \"%s\"", itemString.c_str());
						output->PrintF(1, "_item: %s", itemName.c_str());
						output->PrintF(1, "_produce: { %s: 1 }", itemName.c_str());
						output->PrintF(1, "_materials:");
						output->PrintF(1, "{");
						for (const std::pair<std::string, uint32_t> material : materials)
							output->PrintF(2, "%s: %u", material.first.c_str(), material.second);
						output->PrintF(1, "}");
						output->PrintF(1, "_skill: %u", skill);						
						for(const std::pair<std::string, bool> craftBool : craftBools)
							output->PrintF(1, "%s: %s", craftBool.first.c_str(), craftBool.second ? "true" : "false");
						output->PrintF(0, "}");
					}

					// Recipe item source
					{
						GeneratedSource* output = _CreateGeneratedSource();

						output->PrintF(0, "item r%s: !%s", itemName.c_str(), recipeMacro.c_str());
						output->PrintF(0, "{");
						output->PrintF(1, "_string: \"%s: %s\"", recipePrefix.c_str(), itemString.c_str());
						output->PrintF(1, "_level: %u", level);
						output->PrintF(1, "_rarity: %s", Rarity::GetInfo(itemPrefix->m_rarity)->m_name);
						output->PrintF(1, "_learn_ability: l%s", itemName.c_str());
						output->PrintF(1, "_level_range: [ %u %u ]", level, level + 3);
						output->PrintF(1, "_loot_groups: [ %s ]", lootGroupsString.c_str());							
						output->PrintF(0, "}");
					}

					// Learn ability source
					{
						GeneratedSource* output = _CreateGeneratedSource();

						output->PrintF(0, "ability l%s: !%s", itemName.c_str(), learnMacro.c_str());
						output->PrintF(0, "{");
						output->PrintF(1, "_string: \"%s: %s\"", recipePrefix.c_str(), itemString.c_str());
						output->PrintF(1, "_description: \"Teaches you how to make %s.\"", itemString.c_str());
						output->PrintF(1, "_consume_items: { r%s: 1 }", itemName.c_str());
						output->PrintF(1, "_skill: %u", skill);
						output->PrintF(1, "_craft_ability: c%s", itemName.c_str());
						output->PrintF(0, "}");
					}
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

	void							
	GenerationJob::_GetAbilities(
		std::vector<const StackObject::Ability*>&	aOut) const
	{
		for (const std::unique_ptr<StackObject>& stackObject : m_stack)
		{
			if (stackObject->m_type == StackObject::TYPE_ABILITY)
				aOut.push_back(&stackObject->m_ability);
		}
	}

	const char* 
	GenerationJob::_PickIconName(
		uint32_t							aLevel,
		Rarity::Id							aRarity,
		const std::unordered_set<uint32_t>& aMustHaveTags,
		const std::vector<uint32_t>&		aTags)
	{
		TaggedData::Query query;
		query.m_mustHaveTagIds = { m_manifest->GetExistingIdByName<Data::Tag>("icon") };

		for(uint32_t mustHaveTagId : aMustHaveTags)
			query.m_mustHaveTagIds.push_back(mustHaveTagId);

		for(uint32_t tagId : aTags)
		{
			TaggedData::Query::TagScoring scoring;
			scoring.m_tagId = tagId;
			scoring.m_score = 1;
			query.m_tagScoring.push_back(scoring);
		}

		const TaggedData::QueryResult* result = m_taggedSpriteData->Get()->PerformQuery(query);
		const TaggedData::QueryResult::Entry* entry = result->TryPickRandomWithFilter(_GetRandom(), [&](
			uint32_t aSpriteId) -> bool
		{
			const Data::Sprite* sprite = m_manifest->GetById<Data::Sprite>(aSpriteId);
			return sprite->m_info.FilterByIconMetaData(aLevel, aRarity);
		});

		TP_CHECK(entry != NULL, "Unable to pick icon.");
		uint32_t iconSpriteId = entry->m_id;

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
			if (segment.m_nameTemplateId != 0)
			{
				const Data::NameTemplate* nameTemplate = m_manifest->GetById<Data::NameTemplate>(segment.m_nameTemplateId);

				std::string generatedName;
				std::unordered_set<uint32_t> newTags;
				CreateName(m_manifest, nameTemplate, m_wordListQueryCache.get(), _GetRandom(), generatedName, newTags);

				for(uint32_t tagId : newTags)
					aTags.push_back(tagId);

				prefix += generatedName;
			}
			else if(segment.m_tagId != 0)
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
			if (segment.m_nameTemplateId != 0)
			{
				const Data::NameTemplate* nameTemplate = m_manifest->GetById<Data::NameTemplate>(segment.m_nameTemplateId);

				std::string generatedName;
				std::unordered_set<uint32_t> newTags;
				CreateName(m_manifest, nameTemplate, m_wordListQueryCache.get(), _GetRandom(), generatedName, newTags);

				for (uint32_t tagId : newTags)
					aTags.push_back(tagId);

				suffix += generatedName;
			}
			else if (segment.m_tagId != 0)
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
	GenerationJob::_PickItemSpecial(
		ItemType::Id		aItemType)
	{
		WeightedRandom<const StackObject::ItemSpecial*> possibilities;

		for (const std::unique_ptr<StackObject>& stackObject : m_stack)
		{
			if (stackObject->m_type == StackObject::TYPE_ITEM_SPECIAL)
			{
				bool possible = true;
				if(stackObject->m_itemSpecial.m_excludeItemTypes.size() > 0)
				{
					for(ItemType::Id excludedItemType : stackObject->m_itemSpecial.m_excludeItemTypes)
					{
						if(excludedItemType == aItemType)
						{
							possible = false;
							break;
						}
					}
				}

				if(possible)
					possibilities.AddPossibility(stackObject->m_itemSpecial.m_weight, &stackObject->m_itemSpecial);
			}
		}

		const StackObject::ItemSpecial* picked;
		if(!possibilities.Pick(_GetRandom(), picked))
			return NULL;

		return picked;
	}

	void							
	GenerationJob::_GetEquipmentSlots(
		std::vector<EquipmentSlot::Id>& aOut) const
	{
		for (const std::unique_ptr<StackObject>& stackObject : m_stack)
		{
			if (stackObject->m_type == StackObject::TYPE_EQUIPMENT_SLOT)
				aOut.push_back(stackObject->m_equipmentSlot);
		}
	}
	
	void							
	GenerationJob::_GetItemSuffixes(
		std::vector<const StackObject::ItemSuffix*>& aOut) const
	{
		for (const std::unique_ptr<StackObject>& stackObject : m_stack)
		{
			if (stackObject->m_type == StackObject::TYPE_ITEM_SUFFIX)
				aOut.push_back(&stackObject->m_itemSuffix);
		}
	}
	
	void							
	GenerationJob::_GetItemPrefixes(
		std::vector<const StackObject::ItemPrefix*>& aOut) const
	{
		for (const std::unique_ptr<StackObject>& stackObject : m_stack)
		{
			if (stackObject->m_type == StackObject::TYPE_ITEM_PREFIX)
				aOut.push_back(&stackObject->m_itemPrefix);
		}
	}

	void							
	GenerationJob::_GetLootGroups(
		Rarity::Id										aRarity,
		std::vector<uint32_t>&							aOut) const
	{
		for (const std::unique_ptr<StackObject>& stackObject : m_stack)
		{
			if (stackObject->m_type == StackObject::TYPE_LOOT_GROUP && stackObject->m_lootGroup.m_rarity == aRarity)
				aOut.push_back(stackObject->m_lootGroup.m_lootGroupId);
		}
	}

	const char* 
	GenerationJob::_PickMaterialName(
		uint32_t										aLevel,
		Rarity::Id										aRarity,
		uint32_t										aTagId)
	{
		const TaggedData::Tag* tag = m_taggedItemData->Get()->GetTag(aTagId);
		TP_VERIFY(tag != NULL, m_source->m_debugInfo, "No tagged item data: %u", aTagId);

		uint32_t bestLevelDiff = UINT32_MAX;
		std::vector<const Data::Item*> bestItems;

		for(uint32_t itemId : tag->m_dataIds)
		{
			const Data::Item* item = m_manifest->GetById<Data::Item>(itemId);
			assert(item->HasTag(aTagId));
			if(item->m_rarity == aRarity)
			{
				uint32_t levelDiff = (uint32_t)abs((int32_t)aLevel - (int32_t)item->m_itemLevel);
				if(levelDiff == bestLevelDiff)
				{
					bestItems.push_back(item);
				}
				else if(levelDiff < bestLevelDiff)
				{
					bestItems = { item };
					bestLevelDiff = levelDiff;
				}
			}
		}

		TP_VERIFY(bestItems.size() > 0, m_source->m_debugInfo, "Unable to pick material: %u", aTagId);
		const Data::Item* materialItem = _GetRandomItemInVector(bestItems);
		return materialItem->m_name.c_str();
	}
}