#pragma once

#include "../ComponentBase.h"
#include "../ComponentFactory.h"
#include "../DataBase.h"
#include "../System.h"

namespace kaos_public
{

	namespace Data
	{

		struct Entity
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ENTITY;

			struct ComponentEntry
			{
				ComponentEntry(
					const Parser::Node* aSource)
				{
					m_componentId = Component::StringToId(aSource->m_name.c_str());
					KP_VERIFY(m_componentId != Component::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid component.", aSource->m_name.c_str());

					std::unique_ptr<ComponentBase> component(aSource->m_sourceContext->m_componentFactory->Create(m_componentId));
					assert(component);

					if(!aSource->m_children.empty())
						component->FromSource(aSource);

					m_componentBase = std::move(component);
				}

				uint32_t							m_componentId;
				std::unique_ptr<ComponentBase>		m_componentBase;
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
				aNode->ForEachChild([&](
					const Parser::Node*	aMember)
				{
					if (aMember->m_name == "string")
					{
						m_displayName = aMember->GetString();
					}
					else if(aMember->m_name == "components")
					{
						aMember->GetObject()->ForEachChild([&](
							const Parser::Node* aComponentMember)
						{
							m_components.push_back(std::make_unique<ComponentEntry>(aComponentMember));
						});
					}
					else if (aMember->m_name == "systems")
					{
						aMember->GetArray()->ForEachChild([&](
							const Parser::Node* aArrayItem)
						{
							uint32_t systemId = System::StringToId(aArrayItem->GetIdentifier());
							KP_VERIFY(systemId != System::INVALID_ID, aArrayItem->m_debugInfo, "'%s' not a valid system.", aArrayItem->m_name.c_str());
							m_systems.push_back(systemId);
						});
					}
					else
					{
						KP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
					}
				});
			}

			// Public data
			std::string										m_displayName;
			std::vector<uint32_t>							m_systems;
			std::vector<std::unique_ptr<ComponentEntry>>	m_components;
		};

	}

}