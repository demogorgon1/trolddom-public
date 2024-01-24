#include "Pcheader.h"

#include <tpublic/Data/WordGenerator.h>

#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace
	{

		void
		_PostProcessWordGenerator(
			const Manifest*			aManifest,
			Data::WordGenerator*	aNameGenerator)
		{
			for(std::unique_ptr<Data::WordGenerator::Source>& source : aNameGenerator->m_sources)
			{
				MarkovChainText::Source markovChainTextSource(source->m_order, source->m_exclude);

				for(uint32_t tagId : source->m_tags)
				{
					const WordList::Tag* wordListTag = aManifest->m_wordList.TryGetTag(tagId);
					TP_VERIFY(wordListTag != NULL, aNameGenerator->m_debugInfo, "Invalid tag (%u).", tagId);

					for(uint32_t wordIndex : wordListTag->m_words)
					{
						const WordList::Word* word = aManifest->m_wordList.GetWordByIndex(wordIndex);
						
						markovChainTextSource.AddText(word->m_word.c_str());
					}
				}

				aNameGenerator->m_generator.AddSource(&markovChainTextSource);

				if(source->m_order > aNameGenerator->m_maxOrder)
					aNameGenerator->m_maxOrder = source->m_order;
			}

			aNameGenerator->m_generator.SetOrder(aNameGenerator->m_maxOrder);
		}

	}

	namespace PostProcessWordGenerators
	{

		void		
		Run(
			Manifest* aManifest)
		{
			aManifest->GetContainer<Data::WordGenerator>()->ForEach([aManifest](
				Data::WordGenerator* aNameGenerator)
			{
				_PostProcessWordGenerator(aManifest, aNameGenerator);
				return true;
			});
		}

	}

}