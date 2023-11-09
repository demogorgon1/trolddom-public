#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct MapPortal
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP_PORTAL;

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "map")
						m_mapId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP, aChild->GetIdentifier());
					else if (aChild->m_name == "spawn")
						m_mapPlayerSpawnId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_PLAYER_SPAWN, aChild->GetIdentifier());
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteUInt(m_mapId);
				aStream->WriteUInt(m_mapPlayerSpawnId);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if (!aStream->ReadUInt(m_mapId))
					return false;
				if (!aStream->ReadUInt(m_mapPlayerSpawnId))
					return false;
				return true;
			}

			// Public data
			uint32_t		m_mapId = 0;
			uint32_t		m_mapPlayerSpawnId = 0;
		};

	}

}