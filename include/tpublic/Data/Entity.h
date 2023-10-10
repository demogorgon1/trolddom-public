#pragma once

#include "../ComponentBase.h"
#include "../ComponentManager.h"
#include "../DataBase.h"
#include "../EntityInstance.h"
#include "../System.h"

namespace tpublic
{

	namespace Data
	{

		struct Entity
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ENTITY;

			struct ComponentEntry
			{
				ComponentEntry()
					: m_componentId(0)
				{

				}

				ComponentEntry(
					const Parser::Node* aSource)
				{
					m_componentId = Component::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_componentId != Component::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid component.", aSource->m_name.c_str());

					std::unique_ptr<ComponentBase> component(aSource->m_sourceContext->m_componentManager->Create(m_componentId));
					assert(component);

					TP_VERIFY((component->GetFlags() & ComponentBase::FLAG_PLAYER_ONLY) == 0, aSource->m_debugInfo, "'%s' is a player-only component.", aSource->m_name.c_str());

					if(!aSource->m_children.empty())
						component->FromSource(aSource);

					m_componentBase = std::move(component);
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_componentId);
					aStream->WriteObjectPointer(m_componentBase);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadUInt(m_componentId))
						return false;

					m_componentBase.reset(aStream->GetComponentManager()->Create(m_componentId));
					if(!m_componentBase->FromStream(aStream))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_componentId = 0;
				std::unique_ptr<ComponentBase>		m_componentBase;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			EntityInstance*
			CreateInstance(
				const ComponentManager*	aComponentManager,
				uint32_t				aEntityInstanceId) const 
			{
				std::unique_ptr<tpublic::EntityInstance> entity = std::make_unique<tpublic::EntityInstance>(m_id, aEntityInstanceId);

				for(const std::unique_ptr<ComponentEntry>& componentEntry : m_components)
					entity->AddComponent(aComponentManager->Create(componentEntry->m_componentId));

				return entity.release();
			}

			void
			SerializeInitData(
				IWriter*				aWriter) const
			{
				aWriter->WritePOD(EntityState::ID_DEFAULT);
				uint32_t index = 0;
				for (const std::unique_ptr<ComponentEntry>& componentEntry : m_components)
				{
					aWriter->WriteUInt(index++);
					componentEntry->m_componentBase->ToStream(aWriter);
				}
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
							TP_VERIFY(systemId != System::INVALID_ID, aArrayItem->m_debugInfo, "'%s' not a valid system.", aArrayItem->m_name.c_str());
							m_systems.push_back(systemId);
						});
					}
					else
					{
						TP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteString(m_displayName);
				aStream->WriteUInts(m_systems);
				aStream->WriteObjectPointers(m_components);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadString(m_displayName))
					return false;
				if(!aStream->ReadUInts(m_systems))
					return false;
				if(!aStream->ReadObjectPointers(m_components))
					return false;
				return true;
			}

			// Public data
			std::string										m_displayName;
			std::vector<uint32_t>							m_systems;
			std::vector<std::unique_ptr<ComponentEntry>>	m_components;
		};

	}

}