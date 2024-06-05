#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	class MapInfo
	{
	public:
		enum OverviewMapType : uint8_t
		{
			OVERVIEW_MAP_TYPE_NONE,
			OVERVIEW_MAP_TYPE_ZONES,
			OVERVIEW_MAP_TYPE_DYNAMIC
		};

		bool
		FromSourceItem(
			const SourceNode*	aItem)
		{
			if(aItem->m_name == "string")
			{
				m_displayName = aItem->GetString();
				return true;
			}
			else if (aItem->m_name == "default_tile")
			{
				m_defaultTileSpriteId = aItem->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aItem->GetIdentifier());
				return true;
			}
			else if (aItem->m_name == "default_player_spawn")
			{
				m_defaultPlayerSpawnId = aItem->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_PLAYER_SPAWN, aItem->GetIdentifier());
				return true;
			}
			else if (aItem->m_name == "default_exit_portal")
			{
				m_defaultExitPortalId = aItem->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_PORTAL, aItem->GetIdentifier());
				return true;
			}
			else if (aItem->m_name == "view_attenuation")
			{
				m_viewAttenuation = aItem->GetUInt32();
				return true;
			}
			else if (aItem->m_name == "view_attenuation_bias")
			{
				m_viewAttenuationBias = aItem->GetUInt32();
				return true;
			}
			else if (aItem->m_name == "view_hidden_visibility")
			{
				m_viewHiddenVisibility = aItem->GetUInt32();
				return true;
			}
			else if (aItem->m_name == "max_players")
			{
				m_maxPlayers = aItem->GetUInt32();
				return true;
			}
			else if (aItem->m_name == "level")
			{
				m_level = aItem->GetUInt32();
				return true;
			}
			else if (aItem->m_name == "default_fishing_loot_table")
			{
				m_defaultFishingLootTableId = aItem->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_LOOT_TABLE, aItem->GetIdentifier());
				return true;
			}
			else if (aItem->m_name == "has_overview_map")
			{
				if(aItem->GetBool())
					m_overviewMapType = OVERVIEW_MAP_TYPE_ZONES;
				return true;
			}
			else if (aItem->m_name == "has_dynamic_overview_map")
			{
				if (aItem->GetBool())
					m_overviewMapType = OVERVIEW_MAP_TYPE_DYNAMIC;
				return true;
			}
			else if(aItem->m_name == "auto_doodads")
			{
				m_autoDoodads = aItem->GetBool();
				return true;
			}
			else if(aItem->m_name == "map_loot_tables")
			{
				aItem->GetIdArray(DataType::ID_LOOT_TABLE, m_mapLootTableIds);
				return true;
			}
			return false;
		}

		void
		ToStream(
			IWriter*			aWriter) const
		{
			aWriter->WriteString(m_displayName);
			aWriter->WriteUInt(m_defaultTileSpriteId);
			aWriter->WriteUInt(m_defaultPlayerSpawnId);
			aWriter->WriteUInt(m_defaultExitPortalId);
			aWriter->WriteUInt(m_viewAttenuation);
			aWriter->WriteUInt(m_viewAttenuationBias);
			aWriter->WriteUInt(m_viewHiddenVisibility);
			aWriter->WriteUInt(m_level);
			aWriter->WriteUInt(m_defaultFishingLootTableId);
			aWriter->WritePOD(m_overviewMapType);
			aWriter->WriteBool(m_autoDoodads);
			aWriter->WriteUInts(m_mapLootTableIds);
			aWriter->WriteUInt(m_maxPlayers);
		}

		bool
		FromStream(
			IReader*			aReader)
		{
			if(!aReader->ReadString(m_displayName))
				return false;
			if (!aReader->ReadUInt(m_defaultTileSpriteId))
				return false;
			if (!aReader->ReadUInt(m_defaultPlayerSpawnId))
				return false;
			if (!aReader->ReadUInt(m_defaultExitPortalId))
				return false;
			if (!aReader->ReadUInt(m_viewAttenuation))
				return false;
			if (!aReader->ReadUInt(m_viewAttenuationBias))
				return false;
			if (!aReader->ReadUInt(m_viewHiddenVisibility))
				return false;
			if (!aReader->ReadUInt(m_level))
				return false;
			if (!aReader->ReadUInt(m_defaultFishingLootTableId))
				return false;
			if (!aReader->ReadPOD(m_overviewMapType))
				return false;
			if (!aReader->ReadBool(m_autoDoodads))
				return false;
			if (!aReader->ReadUInts(m_mapLootTableIds))
				return false;
			if (!aReader->ReadUInt(m_maxPlayers))
				return false;
			return true;
		}

		// Public data
		std::string									m_displayName;
		uint32_t									m_defaultTileSpriteId = 0;
		uint32_t									m_defaultPlayerSpawnId = 0;
		uint32_t									m_defaultExitPortalId = 0;
		uint32_t									m_viewAttenuation = 0;
		uint32_t									m_viewAttenuationBias = 0;
		uint32_t									m_viewHiddenVisibility = 0;
		uint32_t									m_level = 0;
		uint32_t									m_defaultFishingLootTableId = 0;
		OverviewMapType								m_overviewMapType = OVERVIEW_MAP_TYPE_NONE;
		bool										m_autoDoodads = false;
		std::vector<uint32_t>						m_mapLootTableIds;
		uint32_t									m_maxPlayers = 5;
	};

}