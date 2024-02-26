#include "Pcheader.h"

#include <tpublic/NoiseInstance.h>

namespace tpublic
{

	NoiseInstance::NoiseInstance(
		const Data::Noise*			aNoise,
		std::mt19937&				aRandom)
		: m_noise(aNoise)
	{
		_Init(aRandom, m_noise->m_root.get());
	}
	
	NoiseInstance::~NoiseInstance()
	{

	}

	int32_t		
	NoiseInstance::Sample(
		const Vec2&					aPosition) const
	{	
		size_t nextPerlinSeedIndex = 0;
		return _Sample(aPosition, m_noise->m_root.get(), nextPerlinSeedIndex);
	}

	//----------------------------------------------------------------

	void		
	NoiseInstance::_Init(
		std::mt19937&				aRandom,
		const Data::Noise::Node*	aNode)
	{
		if(aNode->m_type == Data::Noise::NODE_TYPE_PERLIN)
		{
			std::unique_ptr<Perlin::Seed> perlinSeed = std::make_unique<Perlin::Seed>();
			Perlin::InitSeed(perlinSeed.get(), aRandom);
			m_perlinSeeds.push_back(std::move(perlinSeed));
		}
		else
		{
			for(const std::unique_ptr<Data::Noise::Node>& child : aNode->m_children)
				_Init(aRandom, child.get());
		}
	}

	int32_t		
	NoiseInstance::_Sample(
		const Vec2&					aPosition,
		const Data::Noise::Node*	aNode,
		size_t&						aNextPerlinSeedIndex) const
	{
		int32_t sample = 0;

		switch(aNode->m_type)
		{
		case Data::Noise::NODE_TYPE_ADD:
		case Data::Noise::NODE_TYPE_SUBTRACT:
		case Data::Noise::NODE_TYPE_MULTIPLY:
		case Data::Noise::NODE_TYPE_DIVIDE:
			TP_CHECK(aNode->m_children.size() > 0, "Invalid noise node operation.");
			sample = _Sample(aPosition, aNode->m_children[0].get(), aNextPerlinSeedIndex);
			for(size_t i = 1; i < aNode->m_children.size(); i++)
			{
				int32_t childSample = _Sample(aPosition, aNode->m_children[i].get(), aNextPerlinSeedIndex);

				switch (aNode->m_type)
				{
				case Data::Noise::NODE_TYPE_ADD:		sample += childSample; break;
				case Data::Noise::NODE_TYPE_SUBTRACT:	sample -= childSample; break;
				case Data::Noise::NODE_TYPE_MULTIPLY:	sample *= childSample; break;
				case Data::Noise::NODE_TYPE_DIVIDE:		TP_CHECK(childSample != 0, "Noise node division by zero."); sample /= childSample; break;
				default:	
					assert(false);
				}
			}
			break;

		case Data::Noise::NODE_TYPE_CONSTANT:
			sample = aNode->m_constant;
			break;

		case Data::Noise::NODE_TYPE_PERLIN:
			sample = Perlin::Sample(	
				aPosition.m_x * aNode->m_scale.m_x,
				aPosition.m_y * aNode->m_scale.m_y,
				0,
				m_perlinSeeds[aNextPerlinSeedIndex++].get());
			break;

		default:
			break;
		}

		if (sample < aNode->m_min)
			sample = aNode->m_min;
		if (sample > aNode->m_max)
			sample = aNode->m_max;

		return sample;
	}

}