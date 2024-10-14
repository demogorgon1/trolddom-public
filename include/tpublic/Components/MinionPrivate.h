#pragma once

#include "../CastInProgress.h"
#include "../Component.h"
#include "../ComponentBase.h"
#include "../NPCMovement.h"
#include "../Resource.h"
#include "../SourceEntityInstance.h"

namespace tpublic
{

	namespace Components
	{

		struct MinionPrivate
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_MINION_PRIVATE;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_NONE;

			struct OwnerRequestAbility
			{
				uint32_t							m_targetEntityInstanceId = 0;
				uint32_t							m_abilityId = 0;
				std::optional<Vec2>					m_aoeTarget;
			};

			struct ResourceEntry
			{
				void
				ToStream(
					IWriter*				aStream) const
				{
					aStream->WriteUInt(m_id);
					aStream->WriteUInt(m_max);
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if (!aStream->ReadUInt(m_id))
						return false;
					if (!aStream->ReadUInt(m_max))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_id = 0;
				uint32_t							m_max = 0;
			};

			struct Resources
			{				
				void
				FromSource(
					const SourceNode*		aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						ResourceEntry t;
						t.m_id = Resource::StringToId(aChild->m_name.c_str());
						TP_VERIFY(t.m_id != 0, aChild->m_debugInfo, "'%s' is not a valid resource.", aChild->m_name.c_str());
						t.m_max = aChild->GetUInt32();
						m_entries.push_back(t);
					});
				}

				void
				ToStream(
					IWriter*				aStream) const
				{
					aStream->WriteObjects(m_entries);
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if(!aStream->ReadObjects(m_entries))
						return false;
					return true;
				}

				ResourceEntry*
				GetResourceEntry(
					uint32_t				aResourceId)
				{
					for(ResourceEntry& t : m_entries)
					{
						if(t.m_id == aResourceId)
							return &t;
					}
					return NULL;
				}

				const ResourceEntry*
				GetResourceEntry(
					uint32_t				aResourceId) const
				{
					for(const ResourceEntry& t : m_entries)
					{
						if(t.m_id == aResourceId)
							return &t;
					}
					return NULL;
				}

				// Public data
				std::vector<ResourceEntry>			m_entries;
			};

			enum Field
			{
				FIELD_SEED,
				FIELD_NAME_TEMPLATE,
				FIELD_RESOURCES,
				FIELD_ABILITY_PRIO
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_SEED, "seed", offsetof(MinionPrivate, m_seed));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_NAME_TEMPLATE, "name_template", offsetof(MinionPrivate, m_nameTemplateId))->SetDataType(DataType::ID_NAME_TEMPLATE);
				aSchema->DefineCustomObject<Resources>(FIELD_RESOURCES, "resources", offsetof(MinionPrivate, m_resources));
				aSchema->Define(ComponentSchema::TYPE_UINT32_ARRAY, FIELD_ABILITY_PRIO, "ability_prio", offsetof(MinionPrivate, m_abilityPrio))->SetDataType(DataType::ID_ABILITY);
			}

			void
			Reset()
			{
				m_seed = 0;
				m_nameTemplateId = 0;
				m_resources.m_entries.clear();
				m_abilityPrio.clear();

				m_npcMovement.Reset(0);
				m_moveCooldownUntilTick = 0;
				m_castInProgress.reset();
				m_channeling.reset();
				m_targetEntity = SourceEntityInstance();
				m_ownerPositionAtLastMoveCommand = Vec2();
				m_playerCombat = false;
				m_ownerRequestAbility.reset();
				m_lastAggroPingTick = 0;
			}

			// Public data
			uint32_t							m_seed = 0;
			uint32_t							m_nameTemplateId = 0;
			Resources							m_resources;
			std::vector<uint32_t>				m_abilityPrio;

			// Internal
			NPCMovement							m_npcMovement;
			int32_t								m_moveCooldownUntilTick = 0;
			std::optional<CastInProgress>		m_castInProgress;
			std::optional<CastInProgress>		m_channeling;
			SourceEntityInstance				m_targetEntity;
			Vec2								m_ownerPositionAtLastMoveCommand;
			bool								m_playerCombat = false;
			std::optional<OwnerRequestAbility>	m_ownerRequestAbility;
			uint32_t							m_lastAggroPingTick = 0;
		};
	}

}