#include "Pcheader.h"

#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/MinionPublic.h>
#include <tpublic/Components/PlayerPublic.h>
#include <tpublic/Components/Position.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Stat.h>
#include <tpublic/UTF8.h>
#include <tpublic/Vec2.h>

namespace tpublic::Helpers
{

	namespace
	{
		int32_t
		_CalculateDistanceSquaredToLargeEntity(
			const Vec2&					aFrom,
			const Components::Position* aTo)
		{
			assert(aTo->IsLarge());

			Vec2 p = aTo->m_position;

			if (aFrom.m_x > p.m_x)
				p.m_x++;
			if(aFrom.m_y > p.m_y)
				p.m_y++;

			return p.DistanceSquared(aFrom);
		}
	}

	//---------------------------------------------------------------------------------

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

	int32_t		
	CalculateDistanceSquared(
		const Components::Position* aA,
		const Components::Position* aB)
	{
		bool largeA = aA->IsLarge();
		bool largeB = aB->IsLarge();

		// Small/small
		if(!largeA && !largeB)
			return aA->m_position.DistanceSquared(aB->m_position);

		// Large/small
		if(largeA && !largeB)
			return _CalculateDistanceSquaredToLargeEntity(aB->m_position, aA);

		// Small/large
		if (!largeA && largeB)
			return _CalculateDistanceSquaredToLargeEntity(aA->m_position, aB);

		// Large/large
		// FIXME: we don't need this for now. Only used for large->large self casts.
		return 0;
	}
	
	int32_t		
	CalculateDistanceSquared(
		const Components::Position* aA,
		const Vec2&					aB)
	{
		if(!aA->IsLarge())
			return aA->m_position.DistanceSquared(aB);

		return _CalculateDistanceSquaredToLargeEntity(aB, aA);
	}

	float		
	RandomFloat(
		std::mt19937&				aRandom)
	{
		return (float)((aRandom() & 0xFFFF0000) >> 16) / (float)0x0000FFFF;
	}

	bool
	RandomRoll(
		std::mt19937&				aRandom,
		uint32_t					aProbability)
	{
		if (aProbability == 100)
			return true;
		UniformDistribution<uint32_t> distribution(0, 99);
		return distribution(aRandom) < aProbability;
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
			// Appears 3 times each
			Stat::ID_CONSTITUTION,
			Stat::ID_DEXTERITY,
			Stat::ID_STRENGTH,
			Stat::ID_SPIRIT,
			Stat::ID_WISDOM,
			Stat::ID_CONSTITUTION,
			Stat::ID_DEXTERITY,
			Stat::ID_STRENGTH,
			Stat::ID_SPIRIT,
			Stat::ID_WISDOM,
			Stat::ID_CONSTITUTION,
			Stat::ID_DEXTERITY,
			Stat::ID_STRENGTH,
			Stat::ID_SPIRIT,
			Stat::ID_WISDOM,

			// Appears twice each
			Stat::ID_SPELL_DAMAGE,
			Stat::ID_HEALING,
			Stat::ID_MANA_PER_5_SECONDS,
			Stat::ID_BLOCK_VALUE,
			Stat::ID_SPELL_DAMAGE,
			Stat::ID_HEALING,
			Stat::ID_MANA_PER_5_SECONDS,
			Stat::ID_BLOCK_VALUE,

			// Appears only once (rarest)
			Stat::ID_SPELL_HASTE,
			Stat::ID_ATTACK_HASTE
		};

		std::mt19937 wordRandom(aSeed);

		uint32_t differentStatWeightsCount = RandomInRange<uint32_t>(wordRandom, 1, 2);
		for (uint32_t j = 0; j < differentStatWeightsCount; j++)
		{
			size_t possibleStatIdIndex = RandomInRange<size_t>(wordRandom, 0, possibleStatIds.size() - 1);

			aOut.m_stats[possibleStatIds[possibleStatIdIndex]] += 1.0f;
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

	void
	MakeLowerCase(
		std::string&				aString)
	{
		UTF8::Decoder utf8(aString.c_str());
		UTF8::Encoder out;
		uint32_t characterCode;
		while (utf8.GetNextCharacterCode(characterCode))
			out.EncodeCharacter((uint32_t)tolower((int)characterCode));
		out.Finalize();
		aString = out.GetBuffer();
	}

	bool		
	StringContains(
		const std::string&			aString,
		const std::string&			aContains)
	{
		size_t cLen = aContains.length();
		if(cLen == 0)
			return true;
		
		size_t sLen = aString.length();
		if(sLen < cLen)
			return false;

		const char* s = aString.c_str();
		const char* c = aContains.c_str();

		while(sLen >= cLen)
		{
			if(memcmp(s, c, cLen) == 0)
				return true;

			s++;
			sLen--;
		}

		return false;
	}

	bool		
	IsPlayerOrMinion(
		const EntityInstance*		aEntityInstance)
	{
		if(aEntityInstance == NULL)
			return false;

		if (aEntityInstance->IsPlayer())
			return true;

		return aEntityInstance->HasComponent<Components::MinionPublic>();
	}

	uint64_t	
	GetCombatGroupInfo(
		const EntityInstance*		aEntityInstance,
		const IWorldView*			aWorldView,
		LootRule::Id&				aOutLootRule,
		Rarity::Id&					aOutLootThreshold)
	{
		if (aEntityInstance == NULL)
			return 0;

		const EntityInstance* entity = aEntityInstance;
		const Components::MinionPublic* minionPublic = entity->GetComponent<Components::MinionPublic>();

		if(minionPublic != NULL)
		{
			entity = aWorldView->WorldViewSingleEntityInstance(minionPublic->m_ownerEntityInstanceId);
			if(entity == NULL)
				return 0;
		}

		const Components::CombatPublic* combatPublic = entity->GetComponent<Components::CombatPublic>();
		if(combatPublic == NULL)
			return 0;

		aOutLootRule = combatPublic->m_lootRule;
		aOutLootThreshold = combatPublic->m_lootThreshold;

		return combatPublic->m_combatGroupId;
	}

	bool		
	GetControllingPlayerInfo(
		const EntityInstance*		aEntityInstance,
		const IWorldView*			aWorldView,
		uint32_t&					aOutCharacterId,
		uint32_t&					aOutEntityInstanceId,
		uint32_t&					aOutLevel)
	{
		if (aEntityInstance == NULL)
			return false;

		const EntityInstance* entity = aEntityInstance;
		const Components::MinionPublic* minionPublic = entity->GetComponent<Components::MinionPublic>();

		if (minionPublic != NULL)
		{
			entity = aWorldView->WorldViewSingleEntityInstance(minionPublic->m_ownerEntityInstanceId);
			if (entity == NULL)
				return false;
		}

		const Components::CombatPublic* combatPublic = entity->GetComponent<Components::CombatPublic>();
		if (combatPublic == NULL)
			return false;

		const Components::PlayerPublic* playerPublic = entity->GetComponent<Components::PlayerPublic>();
		if (playerPublic == NULL)
			return false;

		aOutCharacterId = playerPublic->m_characterId;
		aOutEntityInstanceId = entity->GetEntityInstanceId();
		aOutLevel = combatPublic->m_level;
		
		return true;
	}

	bool		
	IsMinionOfPlayer(
		const EntityInstance*		aEntityInstance,
		uint32_t					aPlayerEntityInstanceId)
	{
		if(aEntityInstance == NULL)
			return false;

		const Components::MinionPublic* minionPublic = aEntityInstance->GetComponent<Components::MinionPublic>();
		if (minionPublic == NULL)
			return false;

		return minionPublic->m_ownerEntityInstanceId == aPlayerEntityInstanceId;
	}

}