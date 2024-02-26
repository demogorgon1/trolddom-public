#pragma once

#include "Data/Noise.h"

#include "Perlin.h"
#include "Vec2.h"

namespace tpublic
{

	class NoiseInstance
	{
	public:
					NoiseInstance(
						const Data::Noise*			aNoise,
						std::mt19937&				aRandom);
					~NoiseInstance();

		int32_t		Sample(
						const Vec2&					aPosition) const;

	private:
		
		const Data::Noise*							m_noise;
		std::vector<std::unique_ptr<Perlin::Seed>>	m_perlinSeeds;

		void		_Init(
						std::mt19937&				aRandom,
						const Data::Noise::Node*	aNode);
		int32_t		_Sample(
						const Vec2&					aPosition,
						const Data::Noise::Node*	aNode,
						size_t&						aNextPerlinSeedIndex) const;
	};

}