#pragma once

#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct CombatPrivate
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_COMBAT_PRIVATE;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			CombatPrivate()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~CombatPrivate()
			{

			}

			// ComponentBase implementation
			void
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if (aChild->m_name == "weapon_damage_range")
					{
						if(aChild->m_type == Parser::Node::TYPE_ARRAY && aChild->m_children.size() == 1)
						{
							m_weaponDamageRangeMin = aChild->m_children[0]->GetUInt32();
							m_weaponDamageRangeMax = m_weaponDamageRangeMin;
						}
						else if (aChild->m_type == Parser::Node::TYPE_ARRAY && aChild->m_children.size() == 2)
						{
							m_weaponDamageRangeMin = aChild->m_children[0]->GetUInt32();
							m_weaponDamageRangeMax = aChild->m_children[1]->GetUInt32();
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid weapon damage range.", aChild->m_name.c_str());
						}
					}
					else if (aChild->m_name == "weapon_cooldown")
					{
						m_weaponCooldown = aChild->GetInt32();
					}
					else if(aChild->m_name == "physical_critical_strike_chance")
					{
						m_physicalCriticalStrikeChance = aChild->GetProbability();
					}
					else if (aChild->m_name == "magical_critical_strike_chance")
					{
						m_magicalCriticalStrikeChance = aChild->GetProbability();
					}
					else if (aChild->m_name == "dodge_chance")
					{
						m_dodgeChance = aChild->GetProbability();
					}
					else if (aChild->m_name == "parry_chance")
					{
						m_parryChance = aChild->GetProbability();
					}
					else if (aChild->m_name == "miss_chance")
					{
						m_missChance = aChild->GetProbability();
					}
					else if (aChild->m_name == "block_chance")
					{
						m_blockChance = aChild->GetProbability();
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_weaponDamageRangeMin);
				aStream->WriteUInt(m_weaponDamageRangeMax);
				aStream->WriteInt(m_weaponCooldown);
				aStream->WriteUInt(m_physicalCriticalStrikeChance);
				aStream->WriteUInt(m_magicalCriticalStrikeChance);
				aStream->WriteUInt(m_dodgeChance);
				aStream->WriteUInt(m_parryChance);
				aStream->WriteUInt(m_missChance);
				aStream->WriteUInt(m_blockChance);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInt(m_weaponDamageRangeMin))
					return false;
				if (!aStream->ReadUInt(m_weaponDamageRangeMax))
					return false;
				if (!aStream->ReadInt(m_weaponCooldown))
					return false;
				if (!aStream->ReadUInt(m_physicalCriticalStrikeChance))
					return false;
				if (!aStream->ReadUInt(m_magicalCriticalStrikeChance))
					return false;
				if (!aStream->ReadUInt(m_dodgeChance))
					return false;
				if (!aStream->ReadUInt(m_parryChance))
					return false;
				if (!aStream->ReadUInt(m_missChance))
					return false;
				if (!aStream->ReadUInt(m_blockChance))
					return false;
				return true;
			}

			// Public data
			uint32_t							m_weaponDamageRangeMin = 0;
			uint32_t							m_weaponDamageRangeMax = 0;
			int32_t								m_weaponCooldown = 0;
			uint32_t							m_physicalCriticalStrikeChance = (5 * uint64_t(UINT32_MAX) / 100);
			uint32_t							m_magicalCriticalStrikeChance = (5 * uint64_t(UINT32_MAX) / 100);
			uint32_t							m_dodgeChance = (5 * uint64_t(UINT32_MAX) / 100);
			uint32_t							m_parryChance = (5 * uint64_t(UINT32_MAX) / 100);
			uint32_t							m_missChance = (5 * uint64_t(UINT32_MAX) / 100);
			uint32_t							m_blockChance = (5 * uint64_t(UINT32_MAX) / 100);
		};

	}

}