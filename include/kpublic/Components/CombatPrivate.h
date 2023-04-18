#pragma once

#include "../ComponentBase.h"

namespace kpublic
{

	namespace Components
	{

		struct CombatPrivate
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_COMBAT_PRIVATE;
			static const uint8_t FLAGS = FLAG_PRIVATE;

			CombatPrivate()
				: ComponentBase(ID, FLAGS)
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
							KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid weapon damage range.", aChild->m_name.c_str());
						}
					}
					else
					{
						KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_weaponDamageRangeMin);
				aStream->WriteUInt(m_weaponDamageRangeMax);
				aStream->WriteUInt(m_physicalCriticalStrikeChance);
				aStream->WriteUInt(m_magicalCriticalStrikeChance);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInt(m_weaponDamageRangeMin))
					return false;
				if (!aStream->ReadUInt(m_weaponDamageRangeMax))
					return false;
				if (!aStream->ReadUInt(m_physicalCriticalStrikeChance))
					return false;
				if (!aStream->ReadUInt(m_magicalCriticalStrikeChance))
					return false;
				return true;
			}

			// Public data
			uint32_t				m_weaponDamageRangeMin = 0;
			uint32_t				m_weaponDamageRangeMax = 0;
			uint32_t				m_physicalCriticalStrikeChance = (5 * uint64_t(UINT32_MAX) / 100) / (uint64_t)UINT32_MAX;
			uint32_t				m_magicalCriticalStrikeChance = (5 * uint64_t(UINT32_MAX) / 100) / (uint64_t)UINT32_MAX;
		};

	}

}