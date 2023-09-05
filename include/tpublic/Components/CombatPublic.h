#pragma once

#include "../Data/Ability.h"

#include "../CastInProgress.h"
#include "../ComponentBase.h"
#include "../Resource.h"

namespace tpublic
{

	namespace Components
	{

		struct CombatPublic
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_COMBAT_PUBLIC;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS | FLAG_PUBLIC;
			static const Persistence::Id PERSISTENCE = Persistence::ID_VOLATILE;

			struct ResourceEntry
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
				: ComponentBase(ID, FLAGS, PERSISTENCE)
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
				for(ResourceEntry& t : m_resources)
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
				for (const ResourceEntry& t : m_resources)
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

			void
			SetResourceToMax(
				uint32_t				aResourceId)
			{
				for (ResourceEntry& t : m_resources)
				{
					if (t.m_id == aResourceId)
					{
						t.m_current = t.m_max;
						return;
					}
				}
			}

			bool
			GetResourceIndex(
				uint32_t				aResourceId,
				size_t&					aOut) const
			{
				size_t i = 0;
				for (const ResourceEntry& t : m_resources)
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

			void
			SetResource(
				uint32_t				aResourceId,
				uint32_t				aValue)
			{
				for (ResourceEntry& t : m_resources)
				{
					if (t.m_id == aResourceId)
					{
						t.m_current = aValue;
						return;
					}
				}
			}

			bool
			HasResourcesForAbility(
				const Data::Ability*	aAbility) const
			{
				for(uint32_t resourceId = 1; resourceId < (uint32_t)Resource::NUM_IDS; resourceId++)
				{
					uint32_t cost = aAbility->m_resourceCosts[resourceId];
					if(cost > 0 && cost > GetResource(resourceId))
						return false;
				}
				return true;
			}

			bool
			SubtractResourcesForAbility(
				const Data::Ability*	aAbility)
			{
				for (uint32_t resourceId = 1; resourceId < (uint32_t)Resource::NUM_IDS; resourceId++)
				{
					uint32_t cost = aAbility->m_resourceCosts[resourceId];
					if(cost > 0)
					{
						size_t index;
						if(!GetResourceIndex(resourceId, index))
							return false;

						if(m_resources[index].m_current < cost)
							return false;

						m_resources[index].m_current -= cost;						
					}				
				}

				return true;
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
						m_factionId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_FACTION, aChild->GetIdentifier());
					else if (aChild->m_name == "level")
						m_level = aChild->GetUInt32();
					else if (aChild->m_name == "dialogue_screen")
						m_dialogueScreenId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_DIALOGUE_SCREEN, aChild->GetIdentifier());
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_targetEntityInstanceId);
				aStream->WriteUInt(m_level);
				aStream->WriteUInt(m_factionId);
				aStream->WriteUInt(m_dialogueScreenId);
				aStream->WriteUInt(m_combatGroupId);
				aStream->WriteObjects(m_resources);
				aStream->WriteOptionalObject(m_castInProgress);
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
				if (!aStream->ReadUInt(m_dialogueScreenId))
					return false;
				if (!aStream->ReadUInt(m_combatGroupId))
					return false;
				if(!aStream->ReadObjects(m_resources))
					return false;
				if(!aStream->ReadOptionalObject(m_castInProgress))
					return false;			
				return true;
			}

			void
			DebugPrint() const override
			{
				printf("combat_public: target=%u level=%u faction=%u combat_group=%zu dialogue_screen=%u", m_targetEntityInstanceId, m_level, m_factionId, m_combatGroupId, m_dialogueScreenId);
				for(const ResourceEntry& resource : m_resources)
					printf(" %s=%u/%u", Resource::GetInfo((Resource::Id)resource.m_id)->m_name, resource.m_current, resource.m_max);
				printf("\n");
			}

			// Public data
			uint32_t						m_targetEntityInstanceId = 0;

			uint32_t						m_level = 1;
			uint32_t						m_factionId = 0;
			uint32_t						m_dialogueScreenId = 0;
			uint64_t						m_combatGroupId = 0;
			std::vector<ResourceEntry>		m_resources;
			std::optional<CastInProgress>	m_castInProgress;
		};

	}

}