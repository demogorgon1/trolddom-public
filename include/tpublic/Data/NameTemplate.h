#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct NameTemplate
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_NAME_TEMPLATE;
			static const bool TAGGED = true;

			struct Component
			{
				enum Type : uint8_t
				{
					INVALID_TYPE,

					TYPE_WORD,
					TYPE_STRING,
					TYPE_GENERATED_NAME
				};

				Component()
				{

				}

				Component(
					Type				aType,
					const SourceNode*	aSource)
					: m_type(aType)
				{
					switch(m_type)
					{
					case TYPE_WORD:				
						aSource->GetIdArray(DataType::ID_TAG_CONTEXT, m_wordTagContextId); 
						TP_VERIFY(m_wordTagContextId.size() > 0, aSource->m_debugInfo, "Array is empty.");
						break;
					
					case TYPE_STRING:			
						aSource->GetStringArray(m_string); 
						TP_VERIFY(m_string.size() > 0, aSource->m_debugInfo, "Array is empty.");
						break;
					
					case TYPE_GENERATED_NAME:	
						aSource->GetIdArray(DataType::ID_WORD_GENERATOR, m_nameWordGeneratorId); 
						TP_VERIFY(m_nameWordGeneratorId.size() > 0, aSource->m_debugInfo, "Array is empty.");
						break;
					
					default:		
						assert(false);
						break;
					}
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_type);
					switch(m_type)
					{
					case TYPE_WORD:
						aWriter->WriteUInts(m_wordTagContextId);
						break;

					case TYPE_STRING:
						aWriter->WriteStrings(m_string);
						break;

					case TYPE_GENERATED_NAME:
						aWriter->WriteUInts(m_nameWordGeneratorId);
						break;

					default:
						assert(false);
						break;
					}
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadPOD(m_type))
						return false;
					switch (m_type)
					{
					case TYPE_WORD:
						if (!aReader->ReadUInts(m_wordTagContextId))
							return false;
						break;

					case TYPE_STRING:
						if(!aReader->ReadStrings(m_string))
							return false;
						break;

					case TYPE_GENERATED_NAME:
						if(!aReader->ReadUInts(m_nameWordGeneratorId))
							return false;
						break;

					default:
						return false;
					}
					return true;
				}

				const char*
				GetString(
					std::mt19937&		aRandom) const
				{
					assert(m_type == TYPE_STRING && m_string.size() > 0);
					std::uniform_int_distribution<size_t> distribution(0, m_string.size() - 1);
					return m_string[distribution(aRandom)].c_str();
				}

				uint32_t
				GetWordTagContextId(
					std::mt19937&		aRandom) const
				{
					assert(m_type == TYPE_WORD && m_wordTagContextId.size() > 0);
					std::uniform_int_distribution<size_t> distribution(0, m_wordTagContextId.size() - 1);
					return m_wordTagContextId[distribution(aRandom)];
				}

				uint32_t
				GetNameGeneratorId(
					std::mt19937&		aRandom) const
				{
					assert(m_type == TYPE_GENERATED_NAME && m_nameWordGeneratorId.size() > 0);
					std::uniform_int_distribution<size_t> distribution(0, m_nameWordGeneratorId.size() - 1);
					return m_nameWordGeneratorId[distribution(aRandom)];
				}

				// Public data
				Type						m_type = INVALID_TYPE;
				std::vector<std::string>	m_string;
				std::vector<uint32_t>		m_wordTagContextId;
				std::vector<uint32_t>		m_nameWordGeneratorId;
			};

			struct Possibility
			{
				Possibility()
				{

				}

				Possibility(
					const SourceNode*	aSource)
				{
					if(aSource->m_annotation)
						m_weight = aSource->m_annotation->GetUInt32();

					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "word")
							m_components.push_back(Component(Component::TYPE_WORD, aChild));
						else if(aChild->m_name == "string")
							m_components.push_back(Component(Component::TYPE_STRING, aChild));
						else if(aChild->m_name == "generated_name")
							m_components.push_back(Component(Component::TYPE_GENERATED_NAME, aChild));
						else if(aChild->m_name == "tags")
							aChild->GetIdArray(DataType::ID_TAG, m_tags);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_weight);
					aWriter->WriteObjects(m_components);
					aWriter->WriteUInts(m_tags);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_weight))
						return false;
					if(!aReader->ReadObjects(m_components))
						return false;
					if(!aReader->ReadUInts(m_tags))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_weight = 1;
				std::vector<Component>	m_components;
				std::vector<uint32_t>	m_tags;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "possibility")
							m_possibilities.push_back(std::make_unique<Possibility>(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});

				for(const std::unique_ptr<Possibility>& t : m_possibilities)
					m_totalWeight += t->m_weight;
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteObjectPointers(m_possibilities);
				aWriter->WriteUInt(m_totalWeight);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadObjectPointers(m_possibilities))
					return false;
				if(!aReader->ReadUInt(m_totalWeight))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Possibility>>	m_possibilities;
			uint32_t									m_totalWeight = 0;
		};

	}

}