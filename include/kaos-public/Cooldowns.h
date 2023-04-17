#pragma once

namespace kaos_public
{

	namespace Data
	{
		struct Ability;
	}

	class Cooldowns
	{
	public:
		struct Entry
		{
			uint32_t		m_abilityId = 0;
			uint32_t		m_start = 0;
			uint32_t		m_end = 0;
		};

						Cooldowns();
						~Cooldowns();		

		void			Update(
							uint32_t				aTick);
		void			Add(
							const Data::Ability*	aAbility,
							uint32_t				aTick);
		const Entry*	Get(
							uint32_t				aAbilityId) const;

	private:

		std::vector<Entry>		m_entries;
	};

}