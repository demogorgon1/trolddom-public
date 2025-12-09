#include "Pcheader.h"

#include <tpublic/Data/NameTemplate.h>
#include <tpublic/Data/Tag.h>
#include <tpublic/Data/TagContext.h>
#include <tpublic/Data/WordGenerator.h>

#include <tpublic/GenerateWord.h>
#include <tpublic/Manifest.h>
#include <tpublic/WordList.h>

namespace tpublic
{

	void		
	CreateName(
		const Manifest*					aManifest,
		const Data::NameTemplate*		aNameTemplate,
		WordList::QueryCache*			aWordListQueryCache,
		std::mt19937&					aRandom,
		std::string&					aOutName,
		std::unordered_set<uint32_t>&	aOutTags)
	{
		TP_CHECK(aNameTemplate->m_possibilities.size() > 0, "No possibilities defined.");

		// Pick possibility
		const Data::NameTemplate::Possibility* possibility = NULL;
		
		{
			tpublic::UniformDistribution<uint32_t> distribution(1, aNameTemplate->m_totalWeight);
			uint32_t roll = distribution(aRandom);
			uint32_t sum = 0;
			for(const std::unique_ptr<Data::NameTemplate::Possibility>& p : aNameTemplate->m_possibilities)
			{
				sum += p->m_weight;
				if(roll <= sum)
				{
					possibility = p.get();
					break;
				}
			}
			assert(possibility != NULL);
		}

		// Concatenate components together
		for(const Data::NameTemplate::Component& component : possibility->m_components)
		{
			switch(component.m_type)	
			{
			case Data::NameTemplate::Component::TYPE_WORD:
				{
					WordList::QueryParams params;
					params.AddTagContext(aManifest->GetById<Data::TagContext>(component.GetWordTagContextId(aRandom)));
					params.Prepare();
					const WordList::QueryCache::Query* query = aWordListQueryCache->PerformQuery(params);
					const WordList::Word* word = query->GetRandomWord(aRandom);
					TP_CHECK(word != NULL, "Unable to find word.");
					aOutName += word->m_word;

					for(uint32_t tagId : word->m_allTags)
					{
						const Data::Tag* tag = aManifest->GetById<Data::Tag>(tagId);
						if(tag->m_transferable)
							aOutTags.insert(tagId);
					}
				}
				break;

			case Data::NameTemplate::Component::TYPE_STRING:
				aOutName += component.GetString(aRandom);
				break;

			case Data::NameTemplate::Component::TYPE_GENERATED_NAME:
				{
					const Data::WordGenerator* wordGenerator = aManifest->GetById<Data::WordGenerator>(component.GetNameGeneratorId(aRandom));
					std::string text;
					GenerateWord(aRandom, wordGenerator, text);
					aOutName += text;
				}
				break;

			default:
				TP_CHECK(false, "Invalid possibility component type.");
				break;
			}
		}

		// Get extra tags
		for(uint32_t tagId : possibility->m_tags)
			aOutTags.insert(tagId);
	}

}