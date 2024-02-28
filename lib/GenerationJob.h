#pragma once

#include <tpublic/Data/Aura.h>
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
		
		const SourceNode*								m_source;

		struct GeneratedSource
		{
			void
			PrintF(
				uint32_t								aIndent,
				const char*								aFormat,
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
			std::vector<std::string>					m_lines;
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
				TYPE_LEVEL_RANGE
			};

			struct RandomTags
			{
				std::vector<uint32_t>					m_tags;
				uint32_t								m_propability = 0;
				uint32_t								m_count = 0;
			};

			struct ItemClass
			{
				std::vector<EquipmentSlot::Id>			m_slots;
				std::vector<ItemType::Id>				m_types;
				uint32_t								m_minLevel = 0;
				uint32_t								m_maxLevel = 0;
				uint32_t								m_weight = 1;
			};

			struct ItemSpecial
			{
				Stat::Collection						m_rawStats;
				uint32_t								m_weight = 1;
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
						if(aSource->m_type == SourceNode::TYPE_IDENTIFIER)
							m_tagId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TAG, aSource->GetIdentifier());
						else
							m_string = aSource->GetString();
					}

					uint32_t							m_tagId = 0;
					std::string							m_string;
				};

				std::vector<Segment>					m_prefix;
				std::vector<Segment>					m_suffix;
				uint32_t								m_weight = 1;
			};

			Type										m_type;
			RandomTags									m_randomTags;
			ItemClass									m_itemClass;
			ItemSpecial									m_itemSpecial;
			Designation									m_designation;
			UIntRange									m_range;
			std::unique_ptr<std::mt19937>				m_randomNumberGenerator;
		};

		std::vector<std::unique_ptr<StackObject>>		m_stack;
		const Manifest*									m_manifest;
		std::string										m_outputPath;		
		std::mt19937									m_defaultRandomNumberGenerator;
		std::vector<std::unique_ptr<GeneratedSource>>	m_generatedSource;
		std::unique_ptr<WordList::QueryCache>			m_wordListQueryCache;
		std::unique_ptr<TaggedDataCache<Data::Sprite>>	m_taggedSpriteData;
		std::unique_ptr<TaggedDataCache<Data::Aura>>	m_taggedAuraData;

		uint32_t										m_nextItemNumber;
		uint32_t										m_nextDeityNumber;
		uint32_t										m_nextNPCNumber;

		template <typename _T>
		_T
		_GetRandomIntInRange(
			_T											aMin,
			_T											aMax)
		{
			std::uniform_int_distribution<_T> distribution(aMin, aMax);
			return distribution(_GetRandom());
		}

		template <typename _T>
		_T
		_GetRandomItemInVector(
			const std::vector<_T>&						aVector)
		{
			assert(aVector.size() > 0);
			if(aVector.size() == 1)
				return aVector[0];
			return aVector[_GetRandomIntInRange<size_t>(0, aVector.size() - 1)];
		}

		void							_ReadSource();
		void							_ReadCompound(
											const SourceNode*				aSource);
		void							_ReadStackObject(
											StackObject::Type				aType,
											const SourceNode*				aSource);
		void							_ReadStackObjectArray(
											StackObject::Type				aType,
											const SourceNode*				aSource);
		void							_ReadItems(
											const SourceNode*				aSource);
		void							_ReadDeities(
											const SourceNode*				aSource);
		void							_ReadNPCs(
											const SourceNode*				aSource);
		void							_GetContextTags(
											std::vector<uint32_t>&			aOut);
		std::mt19937&					_GetRandom();		
		const UIntRange&				_GetLevelRange() const;
		const char*						_PickIconName(
											uint32_t						aMustHaveTagId,
											const std::vector<uint32_t>&	aTags);
		GeneratedSource*				_CreateGeneratedSource();
		void							_CreateDesignation(
											const char*						aBaseName,
											const StackObject::Designation*	aDesignation,
											const std::vector<uint32_t>&	aContextTags,
											std::vector<uint32_t>&			aTags,
											Stat::Collection&				aStatWeights,
											std::string&					aOut);
		const StackObject::ItemSpecial*	_PickItemSpecial();
	};

}