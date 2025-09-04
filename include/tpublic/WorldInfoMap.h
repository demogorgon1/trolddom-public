#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"
#include "Vec2.h"

namespace tpublic
{

	class Manifest;

	class WorldInfoMap
	{
	public:		
		enum Flag : uint8_t 
		{
			FLAG_DEMO	= 0x01,
			FLAG_INDOOR = 0x02
		};

		static void AutoIndoor(
						const Manifest*				aManifest,
						int32_t						aMapWidth,
						int32_t						aMapHeight,
						const uint32_t*				aCoverMap,
						uint8_t*					aFlags);

		static uint8_t
		SourceToFlags(
			const SourceNode*						aSource)
		{
			uint8_t flags = 0;
			aSource->GetArray()->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->IsIdentifier("demo"))
					flags |= FLAG_DEMO;
				else if (aChild->IsIdentifier("indoor"))
					flags |= FLAG_INDOOR;
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid flag.", aChild->GetIdentifier());					
			});
			return flags;
		}

		struct Entry
		{
			bool
			operator ==(
				const Entry&						aOther) const
			{
				return m_level == aOther.m_level && m_zoneId == aOther.m_zoneId && m_subZoneId == aOther.m_subZoneId && m_flags == aOther.m_flags;
			}

			void			
			ToStream(
				IWriter*							aWriter) const
			{
				aWriter->WriteUInt(m_level);
				aWriter->WriteUInt(m_zoneId);
				aWriter->WriteUInt(m_subZoneId);
				aWriter->WritePOD(m_flags);
			}
			
			bool			
			FromStream(	
				IReader*							aReader)
			{
				if (!aReader->ReadUInt(m_level))
					return false;
				if (!aReader->ReadUInt(m_zoneId))
					return false;
				if (!aReader->ReadUInt(m_subZoneId))
					return false;
				if (!aReader->ReadPOD(m_flags))
					return false;
				return true;
			}

			// Public data
			uint32_t				m_level = 0;
			uint32_t				m_zoneId = 0;
			uint32_t				m_subZoneId = 0;
			uint8_t					m_flags = 0;
		};		

		struct ZoneOutline
		{
			void			
			ToStream(
				IWriter*							aWriter) const
			{
				aWriter->WriteObjects(m_positions);
			}
			
			bool			
			FromStream(
				IReader*							aReader)
			{
				if(!aReader->ReadObjects(m_positions))
					return false;
				return true;
			}

			// Public data
			std::vector<Vec2>		m_positions;
		};

		struct ZonePositions
		{
			void			
			ToStream(
				IWriter*							aWriter) const
			{
				aWriter->WriteObjects(m_positions);
			}
			
			bool			
			FromStream(
				IReader*							aReader)
			{
				if(!aReader->ReadObjects(m_positions))
					return false;
				return true;
			}

			// Public data
			std::vector<Vec2>		m_positions;			
		};

		void					CopyFrom(
									const WorldInfoMap*	aWorldInfoMap);
		void					Build(
									const Manifest*		aManifest,
									int32_t				aWidth,
									int32_t				aHeight,
									const uint32_t*		aLevelMap,
									const uint32_t*		aZoneMap,
									const uint32_t*		aSubZoneMap,
									const uint8_t*		aFlagsMap);
		const Entry&			Get(
									const Vec2&			aPosition) const;
		const ZoneOutline*		GetZoneOutline(
									uint32_t			aZoneId) const;
		const ZonePositions*	GetZonePositions(
									uint32_t			aZoneId) const;
		void					ToStream(
									IWriter*			aWriter) const;
		bool					FromStream(
									IReader*			aReader);
	
	private:

		static const int32_t TOP_LEVEL_CELL_SIZE = 4;

		struct TopLevelCell
		{
			void			
			ToStream(
				IWriter*							aWriter) const
			{
				aWriter->WriteBool(m_hasDetails);
				if(!m_hasDetails)
					m_entry.ToStream(aWriter);
				else
					aWriter->WriteUInt(m_detailsIndex);
			}
			
			bool			
			FromStream(
				IReader*							aReader)
			{
				if(!aReader->ReadBool(m_hasDetails))
					return false;

				if(!m_hasDetails)
				{
					if(!m_entry.FromStream(aReader))
						return false;
				}
				else
				{
					if(!aReader->ReadUInt(m_detailsIndex))
						return false;
				}
				return true;
			}

			// Public  data
			bool					m_hasDetails = false;

			// FIXME: packed
			Entry					m_entry;
			uint32_t				m_detailsIndex = 0;
		};

		struct Details
		{			
			// FIXME: use a palette to further reduce memory usage

			void			
			ToStream(
				IWriter*							aWriter) const
			{
				for(int32_t i = 0; i < TOP_LEVEL_CELL_SIZE * TOP_LEVEL_CELL_SIZE; i++)
					m_entries[i].ToStream(aWriter);
			}
			
			bool			
			FromStream(
				IReader*							aReader)
			{
				for (int32_t i = 0; i < TOP_LEVEL_CELL_SIZE * TOP_LEVEL_CELL_SIZE; i++)
				{
					if(!m_entries[i].FromStream(aReader))
						return false;
				}

				return true;
			}

			// Public data
			Entry					m_entries[TOP_LEVEL_CELL_SIZE * TOP_LEVEL_CELL_SIZE];
		};

		std::vector<Details>		m_details;
		std::vector<TopLevelCell>	m_topLevelCells;

		Vec2						m_size;
		Vec2						m_topLevelCellsSize;

		Entry						m_blankEntry;

		typedef std::unordered_map<uint32_t, std::unique_ptr<ZoneOutline>> ZoneOutlineTable;
		ZoneOutlineTable			m_zoneOutlineTable;

		typedef std::unordered_map<uint32_t, std::unique_ptr<ZonePositions>> ZonePositionsTable;
		ZonePositionsTable			m_zonePositionsTable;

		void			_UpdateZonePositionsTable(
							const Manifest*				aManifest,
							uint32_t					aZoneId,
							const Vec2&					aPosition);
	};

}