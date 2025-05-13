#pragma once

#include "../DataBase.h"
#include "../Image.h"

namespace tpublic
{

	namespace Data
	{

		struct WorldMap
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_WORLD_MAP;
			static const bool TAGGED = false;

			struct SubMap
			{
				void
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WriteUInt(m_mapId);
					aStream->WriteObjects(m_outline);
				}

				bool
				FromStream(
					IReader*			aStream)
				{
					if (!aStream->ReadUInt(m_mapId))
						return false;
					if (!aStream->ReadObjects(m_outline, 32768))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_mapId = 0;
				std::vector<Vec2>		m_outline;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{
				aNode->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if(aChild->m_name == "land_tile")
							m_landTileSpriteId = aChild->GetId(DataType::ID_SPRITE);
						else if (aChild->m_name == "ocean_tile")
							m_oceanTileSpriteId = aChild->GetId(DataType::ID_SPRITE);
						else if (aChild->m_name == "margins")
							aChild->GetIntArray(m_margins);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_landTileSpriteId);
				aStream->WriteUInt(m_oceanTileSpriteId);
				aStream->WriteInts(m_margins);
				aStream->WriteObjectPointers(m_subMaps);
				aStream->WriteObjectPointer(m_image);
			}
			
			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadUInt(m_landTileSpriteId))
					return false;
				if (!aStream->ReadUInt(m_oceanTileSpriteId))
					return false;
				if(!aStream->ReadInts(m_margins))
					return false;
				if(!aStream->ReadObjectPointers(m_subMaps))
					return false;
				if(!aStream->ReadObjectPointer(m_image))
					return false;
				return true;
			}

			// Public data
			uint32_t								m_landTileSpriteId = 0;
			uint32_t								m_oceanTileSpriteId = 0;
			std::vector<int32_t>					m_margins;
			std::vector<std::unique_ptr<SubMap>>	m_subMaps;
			std::unique_ptr<Image>					m_image;
		};

	}

}