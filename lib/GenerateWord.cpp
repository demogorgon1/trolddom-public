#include "Pcheader.h"

#include <tpublic/Data/WordGenerator.h>

#include <tpublic/UTF8.h>

namespace tpublic
{

	namespace
	{
		bool 
		_IsVowel(
			uint32_t		aCharacterCode)
		{
			// We only care about basic ASCII and Latin-1 unicode supplement
			// 256 "is vowel?" bits packed into 8 uint32s
			static const uint32_t VOWEL_TABLE[8] = { 0x00000000, 0x00000000, 0x02208222, 0x02208222, 0x00000000, 0x00000000, 0x3F7CFF7F, 0xBF7CFF7F };
			return aCharacterCode < 256 && (VOWEL_TABLE[aCharacterCode >> 5] & (1 << (aCharacterCode & 0x1F))) != 0;
		}

		void
		_Analyze(
			const char*		aString,
			uint32_t&		aOutNumSyllables,
			uint32_t&		aOutLongestConsonantSequence)
		{
			aOutNumSyllables = 0;
			aOutLongestConsonantSequence = 0;

			UTF8::Decoder utf8(aString);
			uint32_t characterCode;
			bool isVowel = false;
			uint32_t currentConsonantSequence = 0;

			while(utf8.GetNextCharacterCode(characterCode))
			{
				if(_IsVowel(characterCode))
				{
					if(!isVowel)
					{
						isVowel = true;
						aOutNumSyllables++;
					}

					currentConsonantSequence = 0;
				}
				else
				{
					isVowel = false;

					currentConsonantSequence++;

					if(currentConsonantSequence > aOutLongestConsonantSequence)
						aOutLongestConsonantSequence = currentConsonantSequence;
				}
			}
		}

	}

	void
	GenerateWord(
		std::mt19937&				aRandom,
		const Data::WordGenerator*	aWordGenerator,
		std::string&				aOut)
	{
		static const size_t MAX_TRIES = 100;

		bool ok = false;

		for(size_t i = 0; i < MAX_TRIES && !ok; i++)
		{
			aWordGenerator->m_generator.CreateText(aRandom, aOut);

			uint32_t numSyllables = 0;
			uint32_t longestConsonantSequence = 0;
			_Analyze(aOut.c_str(), numSyllables, longestConsonantSequence);

			ok = numSyllables >= aWordGenerator->m_syllables.m_min && 
				numSyllables <= aWordGenerator->m_syllables.m_max &&
				longestConsonantSequence <= aWordGenerator->m_maxConsonantSequence;

			if(ok && aWordGenerator->m_excludeSource)
				ok = !aWordGenerator->m_generator.IsSourceText(aOut.c_str());

			if(ok && aWordGenerator->m_excludeLengthBuckets.size() > 0)
				ok = aWordGenerator->ValidateWordEnding(aOut.c_str());
		}
	}

}