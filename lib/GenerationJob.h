#pragma once

#include <tpublic/Components/NPC.h>

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/Aura.h>
#include <tpublic/Data/Item.h>
#include <tpublic/Data/Sprite.h>

#include <tpublic/EquipmentSlot.h>
#include <tpublic/ItemType.h>
#include <tpublic/TaggedDataCache.h>
#include <tpublic/UIntRange.h>
#include <tpublic/WordList.h>

namespace tpublic
{

	class Manifest;
	class SourceNode;

	class GenerationJob
	{
	public:
					GenerationJob(
						const SourceNode*				aSource);
					~GenerationJob();

		void		Run(
						const Manifest*					aManifest,
						const char*						aOutputPath);

	private:
		
		const SourceNode*									m_source;

		struct GeneratedSource
		{
			void
			PrintF(
				uint32_t									aIndent,
				const char*									aFormat,
				...)
			{
				char buffer[1024];
				for(uint32_t i = 0; i < aIndent; i++)
					buffer[i] = '\t';

				char* p = buffer + aIndent;
				size_t size = sizeof(buffer) - (size_t)aIndent;

				TP_STRING_FORMAT_VARARGS(p, size, aFormat);
				m_lines.push_back(buffer);
			}

			// Public data
			std::vector<std::string>						m_lines;
		};

		struct StackObject
		{
			enum Type	
			{	
				TYPE_RANDOM_NUMBER_GENERATOR,
				TYPE_RANDOM_TAGS,
				TYPE_ITEM_CLASS,
				TYPE_ITEM_SPECIAL,
				TYPE_DESIGNATION,
				TYPE_LEVEL_RANGE,
				TYPE_ABILITY,
				TYPE_WEAPON_SPEED,
				TYPE_ITEM_SUFFIX,
				TYPE_ITEM_PREFIX,
				TYPE_EQUIPMENT_SLOT,
				TYPE_LOOT_GROUP,
			};

			struct RandomTags
			{
				std::vector<uint32_t>						m_tags;
				uint32_t									m_propability = 0;
				uint32_t									m_count = 0;
			};

			struct ItemClass
			{
				std::vector<EquipmentSlot::Id>				m_slots;
				std::vector<ItemType::Id>					m_types;
				Rarity::Id									m_minRarity = Rarity::ID_COMMON;
				uint32_t									m_minLevel = 0;
				uint32_t									m_maxLevel = 0;
				uint32_t									m_weight = 1;
			};

			struct ItemSpecial
			{
				Stat::Collection							m_rawStats;
				uint32_t									m_weight = 1;
				std::vector<ItemType::Id>					m_excludeItemTypes;
			};

			struct Designation
			{
				struct Segment
				{
					Segment()
					{

					}

					Segment(
						const SourceNode*				aSource)
					{
						if (aSource->m_type == SourceNode::TYPE_OBJECT)
						{
							aSource->GetObject()->ForEachChild([&](
								const SourceNode* aChild)
							{
								if(aChild->m_name == "name_template")
									m_nameTemplateId = aChild->GetId(DataType::ID_NAME_TEMPLATE);
								else 
									TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
							});
						}
						else if(aSource->m_type == SourceNode::TYPE_IDENTIFIER)
						{
							m_tagId = aSource->GetId(DataType::ID_TAG);
						}
						else
						{
							m_string = aSource->GetString();
						}
					}

					uint32_t								m_tagId = 0;
					std::string								m_string;
					uint32_t								m_nameTemplateId = 0;
				};

				std::vector<Segment>						m_prefix;
				std::vector<Segment>						m_suffix;
				uint32_t									m_weight = 1;
			};

			struct WeaponSpeed
			{
				bool 
				HasType(
					ItemType::Id						aType) const
				{
					for(ItemType::Id type : m_types)
					{
						if(type == aType)
							return true;
					}
					return false;
				}

				// Public data
				int32_t										m_speed = 0;
				std::vector<ItemType::Id>					m_types;
				uint32_t									m_weight = 1;
			};

			struct Ability
			{
				uint32_t									m_abilityId = 0;
				Components::NPC::AbilityEntry::TargetType	m_targetType = Components::NPC::AbilityEntry::TARGET_TYPE_DEFAULT;
				uint32_t									m_targetMustNotHaveAuraId = 0;
			};

			struct ItemSuffix
			{
				uint32_t
				GetHash() const
				{
					Hash::CheckSum hash;
					hash.AddString(m_string.c_str());
					hash.AddPOD(m_budgetBias);
					hash.AddPOD(m_stats.m_stats);
					return hash.m_hash;
				}

				// Public data
				std::string									m_string;
				int32_t										m_budgetBias = 0;
				Stat::Collection							m_stats;
			};

			struct ItemPrefix
			{
				uint32_t
				GetHash() const
				{
					Hash::CheckSum hash;
					hash.AddString(m_string.c_str());
					hash.AddPOD(m_rarity);
					hash.AddPOD(m_levelRange);
					hash.AddPOD(m_materialMultiplier);
					return hash.m_hash;
				}

				// Public data
				std::string									m_string;
				Rarity::Id									m_rarity = Rarity::INVALID_ID;
				UIntRange									m_levelRange;
				float										m_materialMultiplier = 1.0f;
			};

			struct LootGroup
			{
				Rarity::Id									m_rarity = Rarity::INVALID_ID;
				uint32_t									m_lootGroupId = 0;
			};

			Type											m_type;
			RandomTags										m_randomTags;
			ItemClass										m_itemClass;
			ItemSpecial										m_itemSpecial;
			Designation										m_designation;
			WeaponSpeed										m_weaponSpeed;
			Ability											m_ability;
			UIntRange										m_range;
			std::unique_ptr<std::mt19937>					m_randomNumberGenerator;
			ItemSuffix										m_itemSuffix;
			ItemPrefix										m_itemPrefix;
			LootGroup										m_lootGroup;
			EquipmentSlot::Id								m_equipmentSlot = EquipmentSlot::INVALID_ID;
		};

		std::vector<std::unique_ptr<StackObject>>			m_stack;
		const Manifest*										m_manifest;
		std::string											m_outputPath;		
		std::mt19937										m_defaultRandomNumberGenerator;
		std::vector<std::unique_ptr<GeneratedSource>>		m_generatedSource;
		std::unique_ptr<WordList::QueryCache>				m_wordListQueryCache;
		std::unique_ptr<TaggedDataCache<Data::Sprite>>		m_taggedSpriteData;
		std::unique_ptr<TaggedDataCache<Data::Aura>>		m_taggedAuraData;
		std::unique_ptr<TaggedDataCache<Data::Item>>		m_taggedItemData;

		uint32_t											m_nextItemNumber;
		uint32_t											m_nextDeityNumber;
		uint32_t											m_nextNPCNumber;

		template <typename _T>
		_T
		_GetRandomIntInRange(
			_T																			aMin,
			_T																			aMax)
		{
			tpublic::UniformDistribution<_T> distribution(aMin, aMax);
			return distribution(_GetRandom());
		}

		template <typename _T>
		_T
		_GetRandomItemInVector(
			const std::vector<_T>&														aVector)
		{
			assert(aVector.size() > 0);
			if(aVector.size() == 1)
				return aVector[0];
			return aVector[_GetRandomIntInRange<size_t>(0, aVector.size() - 1)];
		}

		void							_ReadSource();
		void							_ReadCompound(
											const SourceNode*								aSource);
		void							_ReadStackObject(
											StackObject::Type								aType,
											const SourceNode*								aSource);
		void							_ReadStackObjectArray(
											StackObject::Type								aType,
											const SourceNode*								aSource);
		void							_ReadItems(
											const SourceNode*								aSource);
		void							_ReadDeities(
											const SourceNode*								aSource);
		void							_ReadNPCs(
											const SourceNode*								aSource);
		void							_ReadCrafting(
											const SourceNode*								aSource);
		void							_GetContextTags(
											std::vector<uint32_t>&							aOut);
		std::mt19937&					_GetRandom();		
		const UIntRange&				_GetLevelRange() const;
		void							_GetAbilities(
											std::vector<const StackObject::Ability*>&		aOut) const;
		const char*						_PickIconName(
											uint32_t										aLevel,
											Rarity::Id										aRarity,
											const std::unordered_set<uint32_t>&				aMustHaveTags,
											const std::vector<uint32_t>&					aTags);
		GeneratedSource*				_CreateGeneratedSource();
		void							_CreateDesignation(
											const char*										aBaseName,
											const StackObject::Designation*					aDesignation,
											const std::vector<uint32_t>&					aContextTags,
											std::vector<uint32_t>&							aTags,
											Stat::Collection&								aStatWeights,
											std::string&									aOut);
		const StackObject::ItemSpecial*	_PickItemSpecial(
											ItemType::Id									aItemType);
		void							_GetEquipmentSlots(
											std::vector<EquipmentSlot::Id>&					aOut) const;
		void							_GetItemSuffixes(
											std::vector<const StackObject::ItemSuffix*>&	aOut) const;
		void							_GetItemPrefixes(
											std::vector<const StackObject::ItemPrefix*>&	aOut) const;
		void							_GetLootGroups(
											Rarity::Id										aRarity,
											std::vector<uint32_t>&							aOut) const;
		const char*						_PickMaterialName(
											uint32_t										aLevel,
											Rarity::Id										aRarity,
											uint32_t										aTagId);
	};

}