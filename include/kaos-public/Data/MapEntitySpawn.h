#pragma once

#include "../DataBase.h"

namespace kaos_public
{

	namespace Data
	{

		struct MapEntitySpawn
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP_ENTITY_SPAWN;

			struct Entity
			{
				void
				FromSource(
					const Parser::Node*	aNode)
				{
					m_entityId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ENTITY, aNode->m_name.c_str());

					aNode->GetObject()->ForEachChild([&](
						const Parser::Node* aChild)
					{
						if(aChild->m_name == "weight")
							m_weight = aChild->GetUInt32();
						else 
							KP_VERIFY(false, aChild->m_debugInfo, "Invalid 'entity' item.");
					});
				}

				// Public data
				uint32_t		m_entityId;
				uint32_t		m_weight;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		aNode) override
			{
				aNode->GetObject()->ForEachChild([&](
					const Parser::Node* aChild)
				{
					if(aChild->m_tag == "entity")
					{
						Entity entity;
						entity.FromSource(aChild);
						m_entities.push_back(entity);
					}
					else
					{
						KP_VERIFY(false, aChild->m_debugInfo, "Invalid 'map_entity_spawn' item.");
					}
				});
			}

			// Public data
			std::vector<Entity>		m_entities;
		};

	}

}