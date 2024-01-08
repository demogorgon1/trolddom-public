#pragma once

namespace tpublic
{

	namespace Components
	{
		struct Position;
	}

	namespace Stat
	{
		struct Collection;
	}

	struct Vec2;

	namespace Helpers
	{

		void		StringAppend(
						char*						aBuffer,
						size_t						aBufferSize,
						const char*					aAppend);
		bool		IsWithinDistance(
						const Vec2&					aA,
						const Vec2&					aB,
						int32_t						aDistance);
		bool		IsWithinDistance(
						const Components::Position*	aA,
						const Components::Position*	aB,
						int32_t						aDistance);
		float		RandomFloat(
						std::mt19937&				aRandom);
		bool		LoadTextFile(
						const char*					aPath,
						std::vector<std::string>&	aOut);
		void		TrimString(
						std::string&				aString);
		void		GetRandomStatWeights(
						uint32_t					aSeed,
						Stat::Collection&			aOut);

		template <typename _T>
		void	
		RemoveCyclicFromVector(
			std::vector<_T>&						aVector, 
			size_t									aIndex)
		{
			assert(aIndex < aVector.size());
			size_t newSize = aVector.size() - 1;

			if(aIndex < newSize)
				aVector[aIndex] = std::move(aVector[newSize]);

			aVector.resize(newSize);
		}

		template <typename _T>
		_T
		RandomInRange(
			std::mt19937&							aRandom,
			_T										aMin,
			_T										aMax)
		{
			if(aMin == aMax)
				return aMin;

			std::uniform_int_distribution<_T> distribution(aMin, aMax);
			return distribution(aRandom);
		}	
		
	}

}