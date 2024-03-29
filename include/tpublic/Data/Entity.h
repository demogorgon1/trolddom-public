#pragma once

#include "../ComponentBase.h"
#include "../ComponentManager.h"
#include "../DataBase.h"
#include "../EntityInstance.h"
#include "../Image.h"
#include "../Resource.h"
#include "../System.h"

namespace tpublic
{

	namespace Data
	{

		struct Entity
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ENTITY;
			static const bool TAGGED = true;

			struct Modifiers
			{
				void
				FromSource(
					const SourceNode*			aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "weapon_damage")
						{
							m_weaponDamage = aChild->GetFloat();
						}
						else if(aChild->m_name == "resource")
						{
							TP_VERIFY(aChild->m_annotation, aChild->m_debugInfo, "Missing resource annotation.");
							Resource::Id resourceId = Resource::StringToId(aChild->m_annotation->GetIdentifier());
							TP_VERIFY(resourceId != Resource::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid resource.", aChild->m_annotation->GetIdentifier());
							m_resources[resourceId] = aChild->GetFloat();
						}
					});
				}

				// Public data - not serialized
				std::optional<float>					m_weaponDamage;

				typedef std::unordered_map<Resource::Id, float> Resources;
				Resources								m_resources;
			};

			struct ComponentEntry
			{
				ComponentEntry()
					: m_componentId(0)
				{

				}

				ComponentEntry(
					const SourceNode* aSource)
				{
					m_componentId = Component::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_componentId != Component::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid component.", aSource->m_name.c_str());

					const ComponentManager* componentManager = aSource->m_sourceContext->m_componentManager.get();

					std::unique_ptr<ComponentBase> component(componentManager->Create(m_componentId));
					assert(component);

					uint8_t flags = componentManager->GetComponentFlags(m_componentId);
					TP_VERIFY((flags & ComponentBase::FLAG_PLAYER_ONLY) == 0, aSource->m_debugInfo, "'%s' is a player-only component.", aSource->m_name.c_str());

					if(!aSource->m_children.empty())
						aSource->m_sourceContext->m_componentManager->ReadSource(aSource, component.get());

					m_componentBase = std::move(component);
				}

				void	
				ToStream(
					const ComponentManager*	aComponentManager,
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_componentId);
					aComponentManager->WriteNetwork(aStream, m_componentBase.get());
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadUInt(m_componentId))
						return false;

					m_componentBase.reset(aStream->GetComponentManager()->Create(m_componentId));
					if(!aStream->GetComponentManager()->ReadNetwork(aStream, m_componentBase.get()))
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
				const ComponentManager* aComponentManager,
				IWriter*				aWriter) const
			{
				aWriter->WritePOD(EntityState::ID_DEFAULT);
				uint32_t index = 0;
				for (const std::unique_ptr<ComponentEntry>& componentEntry : m_components)
				{
					aWriter->WriteUInt(index++);
					aComponentManager->WriteNetwork(aWriter, componentEntry->m_componentBase.get());
				}
			}

			template <typename _T>
			_T*
			TryGetComponent()
			{
				for (std::unique_ptr<ComponentEntry>& componentEntry : m_components)
				{
					if(componentEntry->m_componentBase->Is<_T>())
						return componentEntry->m_componentBase->Cast<_T>();
				}
				return NULL;
			}

			template <typename _T>
			const _T*
			TryGetComponent() const
			{
				for (const std::unique_ptr<ComponentEntry>& componentEntry : m_components)
				{
					if(componentEntry->m_componentBase->Is<_T>())
						return componentEntry->m_componentBase->Cast<_T>();
				}
				return NULL;
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{
				aNode->ForEachChild([&](
					const SourceNode*	aMember)
				{
					if(!FromSourceBase(aMember))
					{
						if (aMember->m_name == "string")
						{
							m_displayName = aMember->GetString();
						}
						else if (aMember->m_name == "components")
						{
							aMember->GetObject()->ForEachChild([&](
								const SourceNode* aComponentMember)
								{
									m_components.push_back(std::make_unique<ComponentEntry>(aComponentMember));
								});
						}
						else if (aMember->m_name == "systems")
						{
							aMember->GetArray()->ForEachChild([&](
								const SourceNode* aArrayItem)
								{
									uint32_t systemId = System::StringToId(aArrayItem->GetIdentifier());
									TP_VERIFY(systemId != System::INVALID_ID, aArrayItem->m_debugInfo, "'%s' not a valid system.", aArrayItem->m_name.c_str());
									m_systems.push_back(systemId);
								});
						}
						else if (aMember->m_name == "modifiers")
						{
							m_modifiers.FromSource(aMember);
						}
						else if (aMember->m_name == "debug_color")
						{
							m_debugColor = Image::RGBA(aMember);
						}
						else
						{
							TP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
						}
					}
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteString(m_displayName);
				aStream->WriteOptionalPOD(m_debugColor);
				aStream->WriteUInts(m_systems);

				{
					assert(m_componentManager != NULL);
					aStream->WriteUInt<size_t>(m_components.size());
					for(const std::unique_ptr<ComponentEntry>& t : m_components)
						t->ToStream(m_componentManager, aStream);
				}
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadString(m_displayName))
					return false;
				if (!aStream->ReadOptionalPOD(m_debugColor))
					return false;
				if(!aStream->ReadUInts(m_systems))
					return false;
				if(!aStream->ReadObjectPointers(m_components))
					return false;
				return true;
			}

			// Public data
			std::string										m_displayName;
			std::optional<Image::RGBA>						m_debugColor;
			std::vector<uint32_t>							m_systems;
			std::vector<std::unique_ptr<ComponentEntry>>	m_components;

			// Not serialized
			Modifiers										m_modifiers;
		};

	}

}