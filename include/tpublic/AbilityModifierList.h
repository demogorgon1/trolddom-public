#pragma once

#include "DirectEffect.h"

namespace tpublic
{

	namespace Data
	{
		struct AbilityModifier;
	}

	class AbilityModifierList
	{
	public:
		void														Reset();
		bool														SetAbilityModifierIds(
																		const Manifest*						aManifest,
																		const std::vector<uint32_t>&		aAbilityModifierIds);
		void														GetAbilityModifierIds(
																		std::vector<uint32_t>&				aOut) const;
		const std::vector<const Data::AbilityModifier*>*			GetAbility(
																		uint32_t							aAbilityId) const;
		int32_t														GetAbilityModifyAuraUpdateCount(
																		uint32_t							aAbilityId) const;
		int32_t														GetAbilityModifyRange(
																		uint32_t							aAbilityId) const;
		int32_t														GetAbilityModifyCastTime(
																		uint32_t							aAbilityId) const;
		DirectEffect::DamageType									GetAbilityModifyDamageType(
																		uint32_t							aAbilityId) const;

	private:

		struct Ability
		{
			std::vector<const Data::AbilityModifier*>			m_modifiers;
		};

		typedef std::unordered_map<uint32_t, std::unique_ptr<Ability>> AbilityTable;
		AbilityTable											m_abilityTable;
		std::vector<uint32_t>									m_abilityModifierIds;
	};

}