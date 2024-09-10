#pragma once

namespace tpublic
{

	// We need our own of these since std::uniform_int_distribution isn't generating the same numbers across different platforms
	template <typename _T>
	class UniformDistribution
	{
	public:
		UniformDistribution(
			_T				aMin = _T(0),
			_T				aMax = _T(0))
			: m_min(aMin)
			, m_max(aMax)
		{

		}

		~UniformDistribution()
		{

		}

		template <typename _RandomGenType>
		_T
		operator()(
			_RandomGenType&	aRandom) const
		{
			return Generate(aRandom());
		}

		_T
		Generate(
			uint32_t		aRandom) const
		{
			uint64_t r = (uint64_t)aRandom;
			return m_min + (_T)(((uint64_t)(m_max - m_min + _T(1)) * r) / 0x100000000ULL);
		}
		
	private:
		
		_T	m_min;
		_T	m_max;
	};

}