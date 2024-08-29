#pragma once

namespace tpublic
{

	template <typename _T>
	class WeightedRandom
	{
	public:
		struct Possibility 
		{			
			_T									m_item;
			uint32_t							m_weight;
		};

		WeightedRandom()
			: m_totalWeight(0)
		{

		}

		~WeightedRandom()
		{

		}

		void
		AddPossibility(
			uint32_t		aWeight,
			const _T&		aItem)
		{
			m_possibilities.push_back(Possibility{ aItem, aWeight });
			m_totalWeight += aWeight;
		}

		bool
		Pick(
			std::mt19937&	aRandom,
			_T&				aOut) const
		{
			if(m_possibilities.size() == 0)
				return false;

			if(m_possibilities.size() == 1)
			{
				aOut = m_possibilities[0].m_item;
				return true;
			}

			tpublic::UniformDistribution<uint32_t> distribution(1, m_totalWeight);
			uint32_t roll = distribution(aRandom);		
			uint32_t sum = 0;

			for(const Possibility& t : m_possibilities)
			{
				sum += t.m_weight;

				if(roll <= sum)
				{
					aOut = t.m_item;
					return true;
				}
			}

			assert(false);
			return false;
		}

		_T
		Get(
			std::mt19937&	aRandom) const
		{
			_T t;
			if(!Pick(aRandom, t))
				assert(false);
			return t;
		}

		void
		Clear()
		{
			m_possibilities.clear();
			m_totalWeight = 0;
		}

		// Data access
		uint32_t						GetTotalWeight() const { return m_totalWeight; }
		const std::vector<Possibility>&	GetPossibilities() const { return m_possibilities; }
		size_t							GetPossibilityCount() const { return m_possibilities.size(); }
		bool							IsEmpty() const { return m_possibilities.size() == 0; }

	private:

		std::vector<Possibility>	m_possibilities;
		uint32_t					m_totalWeight;
	};

}