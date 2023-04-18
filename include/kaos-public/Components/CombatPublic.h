#pragma once

#include "../ComponentBase.h"

namespace kaos_public
{

	namespace Components
	{

		struct CombatPublic
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_COMBAT_PUBLIC;
			static const uint8_t FLAGS = FLAG_PRIVATE | FLAG_PUBLIC;

			struct Resource
			{	
				void
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WriteUInt(m_id);
					aStream->WriteUInt(m_current);
					aStream->WriteUInt(m_max);
				}

				bool
				FromStream(
					IReader*			aStream) 
				{
					if (!aStream->ReadUInt(m_id))
						return false;
					if (!aStream->ReadUInt(m_current))
						return false;
					if (!aStream->ReadUInt(m_max))
						return false;
					return true;
				}

				// Public data
				uint32_t	m_id = 0;
				uint32_t	m_current = 0;
				uint32_t	m_max = 0;
			};
						
			CombatPublic()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual 
			~CombatPublic()
			{

			}

			void
			AddResourceMax(
				uint32_t				aResourceId,
				uint32_t				aValue)
			{
				for(Resource& t : m_resources)
				{
					if(t.m_id == aResourceId)
					{
						t.m_max += aValue;

						if(t.m_current > t.m_max)
							t.m_current = t.m_max;
						return;
					}
				}
				m_resources.push_back({ aResourceId, 0, aValue });
			}

			uint32_t
			GetResource(
				uint32_t				aResourceId,
				uint32_t*				aOutMax = NULL) const
			{
				for (const Resource& t : m_resources)
				{
					if (t.m_id == aResourceId)
					{
						if(aOutMax != NULL)
							*aOutMax = t.m_max;
						
						return t.m_current;
					}
				}
				return 0;
			}

			bool
			GetResourceIndex(
				uint32_t				aResourceId,
				size_t&					aOut) const
			{
				size_t i = 0;
				for (const Resource& t : m_resources)
				{
					if (t.m_id == aResourceId)
					{
						aOut = i;
						return true;
					}
					i++;
				}
				return false;
			}

			// ComponentBase implementation
			void
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if (aChild->m_name == "faction")
					{
						m_factionId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_FACTION, aChild->GetIdentifier());
					}
					else if (aChild->m_name == "weapon_damage_range")
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
				aStream->WriteUInt(m_targetEntityInstanceId);
				aStream->WriteUInt(m_level);
				aStream->WriteUInt(m_factionId);
				aStream->WriteUInt(m_weaponDamageRangeMin);
				aStream->WriteUInt(m_weaponDamageRangeMax);
				aStream->WriteUInt(m_physicalCriticalStrikeChance);
				aStream->WriteUInt(m_magicalCriticalStrikeChance);
				aStream->WriteObjects(m_resources);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInt(m_targetEntityInstanceId))
					return false;
				if (!aStream->ReadUInt(m_level))
					return false;
				if (!aStream->ReadUInt(m_factionId))
					return false;
				if (!aStream->ReadUInt(m_weaponDamageRangeMin))
					return false;
				if (!aStream->ReadUInt(m_weaponDamageRangeMax))
					return false;
				if (!aStream->ReadUInt(m_physicalCriticalStrikeChance))
					return false;
				if (!aStream->ReadUInt(m_magicalCriticalStrikeChance))
					return false;
				if(!aStream->ReadObjects(m_resources))
					return false;
				return true;
			}

			// Public data
			uint32_t				m_targetEntityInstanceId = 0;

			uint32_t				m_level = 1;
			uint32_t				m_factionId = 0;

			// FIXME: some of this should be moved to other components
			uint32_t				m_weaponDamageRangeMin = 0;
			uint32_t				m_weaponDamageRangeMax = 0;
			uint32_t				m_physicalCriticalStrikeChance = 0;
			uint32_t				m_magicalCriticalStrikeChance = 0;
			std::vector<Resource>	m_resources;
		};

	}

}