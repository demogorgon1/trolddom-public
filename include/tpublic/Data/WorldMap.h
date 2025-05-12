#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct WorldMap
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_WORLD_MAP;
			static const bool TAGGED = false;

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
			}
			
			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadUInt(m_landTileSpriteId))
					return false;
				if (!aStream->ReadUInt(m_oceanTileSpriteId))
					return false;
				return true;
			}

			// Public data
			uint32_t					m_landTileSpriteId = 0;
			uint32_t					m_oceanTileSpriteId = 0;
		};

	}

}