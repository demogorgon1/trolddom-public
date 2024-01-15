#include "Pcheader.h"

#include <tpublic/Components/Position.h>

#include <tpublic/Helpers.h>
#include <tpublic/Stat.h>
#include <tpublic/Vec2.h>

namespace tpublic::Helpers
{

	void		
	StringAppend(
		char*						aBuffer,
		size_t						aBufferSize,
		const char*					aAppend)
	{
		size_t length = strlen(aBuffer);
		size_t appendLength = strlen(aAppend);
		size_t maxLength = aBufferSize - 1;

		if(length + appendLength <= maxLength)
		{
			memcpy(aBuffer + length, aAppend, appendLength);
			length += appendLength;
		}
		else if(length < maxLength)
		{
			appendLength = maxLength - length;
			memcpy(aBuffer + length, aAppend, appendLength);
			length = maxLength;
		}
	}

	bool		
	IsWithinDistance(
		const Vec2&					aA,
		const Vec2&					aB,
		int32_t						aDistance)
	{
		int32_t dx = aA.m_x - aB.m_x;
		int32_t dy = aA.m_y - aB.m_y;

		return dx * dx + dy * dy <= aDistance * aDistance;
	}

	bool	
	IsWithinDistance(
		const Components::Position* aA,
		const Components::Position* aB,
		int32_t						aDistance)
	{
		if(aA == NULL || aB == NULL)
			return false;

		return IsWithinDistance(aA->m_position, aB->m_position, aDistance);
	}

	float		
	RandomFloat(
		std::mt19937&				aRandom)
	{
		return (float)((aRandom() & 0xFFFF0000) >> 16) / (float)0x0000FFFF;
	}

	bool
	LoadTextFile(
		const char*					aPath,
		std::vector<std::string>&	aOut)
	{
		FILE* f = fopen(aPath, "rb");
		if (f == NULL)
			return false;

		char line[1024];

		while (feof(f) == 0 && fgets(line, sizeof(line), f) != NULL)
			aOut.push_back(line);

		fclose(f);
		return true;
	}

	void
	TrimString(
		std::string&				aString)
	{
		size_t length = aString.length();
		size_t removeFrontCount = 0;

		for (size_t i = 0; i < length; i++)
		{
			char c = aString[i];
			if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
				removeFrontCount++;
			else
				break;
		}

		size_t removeBackCount = 0;

		for (size_t i = 0; i < length; i++)
		{
			char c = aString[length - i - 1];
			if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
				removeBackCount++;
			else
				break;
		}

		if (removeFrontCount == length || removeBackCount == length)
		{
			aString.clear();
		}
		else
		{
			if (removeFrontCount > 0)
			{
				aString.erase(0, removeFrontCount);
				length -= removeFrontCount;
			}

			if (removeBackCount > 0)
			{
				assert(length > removeBackCount);
				aString.erase(length - removeBackCount, removeBackCount);
			}
		}
	}

	void		
	GetRandomStatWeights(
		uint32_t					aSeed,
		Stat::Collection&			aOut)
	{
		std::vector<Stat::Id> possibleStatIds =
		{
			Stat::ID_CONSTITUTION,
			Stat::ID_DEXTERITY,
			Stat::ID_STRENGTH,
			Stat::ID_SPIRIT,
			Stat::ID_WISDOM
		};

		std::mt19937 wordRandom(aSeed);

		uint32_t differentStatWeightsCount = RandomInRange<uint32_t>(wordRandom, 1, 2);
		for (uint32_t j = 0; j < differentStatWeightsCount; j++)
		{
			size_t possibleStatIdIndex = RandomInRange<size_t>(wordRandom, 0, possibleStatIds.size() - 1);

			aOut.m_stats[possibleStatIds[possibleStatIdIndex]]++;
		}
	}

	std::string
	Format(
		const char*					aFormat,
		...)
	{
		char buffer[1024];
		TP_STRING_FORMAT_VARARGS(buffer, sizeof(buffer), aFormat);
		return buffer;
	}

}