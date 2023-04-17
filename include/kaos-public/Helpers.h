#pragma once

namespace kaos_public
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
		
	}

}