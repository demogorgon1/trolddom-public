#pragma once

namespace kpublic
{

	namespace Components
	{
		struct Position;
	}

	struct Vec2;

	namespace Helpers
	{

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