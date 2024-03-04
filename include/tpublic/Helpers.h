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
		std::string	Format(
						const char*					aFormat,
						...);
		void		MakeLowerCase(
						std::string&				aString);
		bool		StringContains(
						const std::string&			aString,
						const std::string&			aContains);

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
		bool
		GetAndRemoveCyclicFromVector(
			std::mt19937&							aRandom,
			std::vector<_T>&						aVector,
			_T&										aOut)
		{
			if(aVector.size() == 0)
				return false;

			size_t i = 0;

			if(aVector.size() > 1)
			{
				std::uniform_int_distribution<size_t> distribution(0, aVector.size() - 1);
				i = distribution(aRandom);
			}

			aOut = aVector[i];
			RemoveCyclicFromVector(aVector, i);
			return true;
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