#pragma once

#include "../DataBase.h"
#include "../MarkovChainText.h"
#include "../UIntRange.h"

namespace tpublic
{

	namespace Data
	{

		struct WordGenerator
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_WORD_GENERATOR;
			static const bool TAGGED = true;

			struct ExcludeLengthBucket
			{
				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_length);
					aWriter->WriteUInt(m_hashes.size());
					for(uint32_t hash : m_hashes)
						aWriter->WriteUInt(hash);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_length))
						return false;

					size_t count;
					if(!aReader->ReadUInt(count))
						return false;

					for(size_t i = 0; i < count; i++)
					{
						uint32_t hash;
						if(!aReader->ReadUInt(hash))
							return false;

						m_hashes.insert(hash);
					}

					return true;
				}

				// Public data
				uint32_t									m_length = 0;
				std::unordered_set<uint32_t>				m_hashes;
			};

			struct Source
			{
				Source()
				{

				}

				Source(
					const SourceNode*	aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "tags")
							aChild->GetIdArray(DataType::ID_TAG, m_tags);
						else if(aChild->m_name == "order")
							m_order = aChild->GetUInt32();
						else if(aChild->m_name == "exclude")
							m_exclude = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				// Public data
				std::vector<uint32_t>						m_tags;
				uint32_t									m_order = 4;
				bool										m_exclude = false;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			bool
			ValidateWordEnding(
				const char*				aWord) const
			{
				size_t length = strlen(aWord);

				// FIXME: possible optimization: calculate hashes backwards, then we don't need to hash every length seperately 				
				for(const std::unique_ptr<ExcludeLengthBucket>& excludeLengthBucket : m_excludeLengthBuckets)
				{
					if(length >= excludeLengthBucket->m_length)
					{
						uint32_t hash = Hash::String(&aWord[length - excludeLengthBucket->m_length]);
						if(excludeLengthBucket->m_hashes.contains(hash))
							return false;
					}
				}

				return true;
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
						if (aChild->m_name == "source")
						{
							m_sources.push_back(std::make_unique<Source>(aChild));
						}
						else if(aChild->m_name == "max_order")
						{
							m_maxOrder = aChild->GetUInt32();
						}
						else if(aChild->m_name == "syllables")
						{
							m_syllables = UIntRange(aChild);
						}
						else if(aChild->m_name == "max_consonant_sequence")
						{
							m_maxConsonantSequence = aChild->GetUInt32();
						}
						else if(aChild->m_name == "exclude_source")
						{
							m_excludeSource = aChild->GetBool();
						}
						else if(aChild->m_name == "exclude_endings")
						{
							aChild->GetArray()->ForEachChild([&](
								const SourceNode* aEnding)
							{
								const char* ending = aEnding->GetString();
								uint32_t endingLength = (uint32_t)strlen(ending);

								ExcludeLengthBucket* excludeLengthBucket = NULL;
								for(std::unique_ptr<ExcludeLengthBucket>& t : m_excludeLengthBuckets)
								{
									if(t->m_length == endingLength)
									{
										excludeLengthBucket = t.get();
										break;
									}
								}

								if(excludeLengthBucket == NULL)
								{
									m_excludeLengthBuckets.push_back(std::make_unique<ExcludeLengthBucket>());
									excludeLengthBucket = m_excludeLengthBuckets[m_excludeLengthBuckets.size() - 1].get();
									excludeLengthBucket->m_length = endingLength;
								}

								excludeLengthBucket->m_hashes.insert(Hash::String(ending));
							});
						}							
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});

				std::sort(m_excludeLengthBuckets.begin(), m_excludeLengthBuckets.end(), [](
					const std::unique_ptr<ExcludeLengthBucket>& aLHS,
					const std::unique_ptr<ExcludeLengthBucket>& aRHS)
				{
					return aLHS->m_length < aRHS->m_length;
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				ToStreamBase(aWriter);

				m_generator.ToStream(aWriter);
				aWriter->WriteUInt(m_maxOrder);
				m_syllables.ToStream(aWriter);
				aWriter->WriteUInt(m_maxConsonantSequence);
				aWriter->WriteObjectPointers(m_excludeLengthBuckets);
				aWriter->WriteBool(m_excludeSource);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if (!FromStreamBase(aReader))
					return false;

				if(!m_generator.FromStream(aReader))
					return false;
				if(!aReader->ReadUInt(m_maxOrder))
					return false;
				if(!m_syllables.FromStream(aReader))
					return false;
				if(!aReader->ReadUInt(m_maxConsonantSequence))
					return false;
				if(!aReader->ReadObjectPointers(m_excludeLengthBuckets))
					return false;
				if(!aReader->ReadBool(m_excludeSource))
					return false;
				return true;
			}

			// Public data
			MarkovChainText::Generator							m_generator;
			uint32_t											m_maxOrder = 0;
			UIntRange											m_syllables = UIntRange(1, 8);
			uint32_t											m_maxConsonantSequence = 4;
			std::vector<std::unique_ptr<ExcludeLengthBucket>>	m_excludeLengthBuckets;
			bool												m_excludeSource = false;

			// Not serialized
			std::vector<std::unique_ptr<Source>>				m_sources;
		};

	}

}