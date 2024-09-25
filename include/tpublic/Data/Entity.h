#pragma once

#include "../ComponentBase.h"
#include "../ComponentManager.h"
#include "../DataBase.h"
#include "../EntityInstance.h"
#include "../Image.h"
#include "../Resource.h"
#include "../System.h"
#include "../UniqueComponent.h"

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
				enum Flag : uint8_t
				{
					FLAG_STATIC = 0x01
				};

				static uint8_t
				SourceToFlag(
					const SourceNode*		aSource)
				{
					std::string_view t(aSource->GetIdentifier());
					if(t == "static")
						return FLAG_STATIC;
					TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid component entry flag.", aSource->GetIdentifier());
					return 0;
				}

				static uint8_t
				SourceToFlags(
					const SourceNode*		aSource)
				{
					uint8_t flags = 0;
					if(aSource->m_type == SourceNode::TYPE_IDENTIFIER)
					{
						flags = SourceToFlag(aSource);
					}
					else
					{
						aSource->GetArray()->ForEachChild([&flags](
							const SourceNode* aChild)
						{
							flags |= SourceToFlag(aChild);
						});
					}

					return flags;
				}

				ComponentEntry()
					: m_componentId(0)
				{

				}

				ComponentEntry(
					const SourceNode* aSource)
				{
					m_componentId = Component::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_componentId != Component::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid component.", aSource->m_name.c_str());

					if(aSource->m_annotation)
						m_flags = SourceToFlags(aSource->m_annotation.get());

					const ComponentManager* componentManager = aSource->m_sourceContext->m_componentManager.get();

					componentManager->AllocateUniqueComponent(m_componentId, m_component);

					uint8_t flags = componentManager->GetComponentFlags(m_componentId);
					TP_VERIFY((flags & ComponentBase::FLAG_PLAYER_ONLY) == 0, aSource->m_debugInfo, "'%s' is a player-only component.", aSource->m_name.c_str());

					if(!aSource->m_children.empty())
						aSource->m_sourceContext->m_componentManager->ReadSource(aSource, m_component.GetPointer());
				}

				void	
				ToStream(
					const ComponentManager*	aComponentManager,
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_componentId);
					aStream->WritePOD(m_flags);
					aComponentManager->WriteNetwork(aStream, m_component.GetPointer());
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadUInt(m_componentId))
						return false;
					if(!aStream->ReadPOD(m_flags))
						return false;

					aStream->GetComponentManager()->AllocateUniqueComponent(m_componentId, m_component);
					if(!aStream->GetComponentManager()->ReadNetwork(aStream, m_component.GetPointer()))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_componentId = 0;
				uint8_t								m_flags = 0;
				UniqueComponent						m_component;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			EntityInstance*
			CreateInstance(
				ComponentManager*		aComponentManager,
				uint32_t				aEntityInstanceId) const 
			{
				std::unique_ptr<EntityInstance> entity = std::make_unique<EntityInstance>(aComponentManager, m_id, aEntityInstanceId);

				for(const std::unique_ptr<ComponentEntry>& componentEntry : m_components)
				{
					EntityInstance::ComponentEntry t;
					
					if(componentEntry->m_flags & ComponentEntry::FLAG_STATIC)
						t.m_static = componentEntry->m_component.GetPointer();
					else
						t.m_allocated = aComponentManager->AllocateComponent(componentEntry->m_componentId);

					entity->AddComponent(t);
				}

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
					if((componentEntry->m_flags & ComponentEntry::FLAG_STATIC) == 0)
					{
						aWriter->WriteUInt(index);
						aComponentManager->WriteNetwork(aWriter, componentEntry->m_component.GetPointer());
					}

					index++;
				}
			}

			template <typename _T>
			_T*
			TryGetComponent()
			{
				for (std::unique_ptr<ComponentEntry>& componentEntry : m_components)
				{
					if(componentEntry->m_component.GetPointer()->Is<_T>())
						return componentEntry->m_component.GetPointer()->Cast<_T>();
				}
				return NULL;
			}

			template <typename _T>
			const _T*
			TryGetComponent() const
			{
				for (const std::unique_ptr<ComponentEntry>& componentEntry : m_components)
				{
					if(componentEntry->m_component.GetPointer()->Is<_T>())
						return componentEntry->m_component.GetPointer()->Cast<_T>();
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

			void
			ToPropertyTable(
				PropertyTable&			aOut) const override
			{
				ToPropertyTableBase(aOut);
				aOut["string"] = m_displayName;
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