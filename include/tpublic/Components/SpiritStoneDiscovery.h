#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct SpiritStoneDiscovery
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_SPIRIT_STONE_DISCOVERY;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			struct SpiritStone
			{
				void
				ToStream(
					IWriter*		aWriter) const
				{
					aWriter->WriteUInt(m_mapId);
					m_position.ToStream(aWriter);
				}

				bool
				FromStream(
					IReader*		aReader) 
				{
					if(!aReader->ReadUInt(m_mapId))
						return false;
					if(!m_position.FromStream(aReader))
						return false;
					return true;
				}

				// Public data
				uint32_t	m_mapId = 0;
				Vec2		m_position;
			};

			enum Field
			{
				FIELD_SPIRIT_STONES
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->DefineCustomObjectsNoSource<SpiritStone>(FIELD_SPIRIT_STONES, offsetof(SpiritStoneDiscovery, m_spiritStones));
			}

			bool
			HasSpiritStone(
				uint32_t			aMapId,
				const Vec2&			aPosition) const
			{
				for(const SpiritStone& t : m_spiritStones)
				{
					if(t.m_mapId == aMapId && t.m_position == aPosition)
						return true;
				}
				return false;
			}

			// Public data
			std::vector<SpiritStone>	m_spiritStones;
			
		};
	}

}