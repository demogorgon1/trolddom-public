#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Wall
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_WALL;
			static const bool TAGGED = true;			

			enum DirectionBit : size_t
			{
				DIRECTION_BIT_EAST = 0x1,
				DIRECTION_BIT_WEST = 0x2,
				DIRECTION_BIT_NORTH = 0x4,
				DIRECTION_BIT_SOUTH = 0x8
			};


			enum SpriteIndex : size_t
			{
				SPRITE_INDEX_BASE,
				SPRITE_INDEX_E,
				SPRITE_INDEX_W,
				SPRITE_INDEX_W_E,
				SPRITE_INDEX_N,
				SPRITE_INDEX_N_E,
				SPRITE_INDEX_W_N,
				SPRITE_INDEX_W_E_N,
				SPRITE_INDEX_S,
				SPRITE_INDEX_E_S,
				SPRITE_INDEX_W_S,
				SPRITE_INDEX_W_E_S,
				SPRITE_INDEX_N_S,
				SPRITE_INDEX_E_N_S,
				SPRITE_INDEX_W_N_S,
				SPRITE_INDEX_W_E_N_S,

				NUM_SPRITE_INDICES
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			uint32_t
			GetSprite(
				size_t					aSpriteIndex) const
			{
				TP_CHECK(aSpriteIndex < m_sprites.size(), "Sprite index out of bounds.");
				return m_sprites[aSpriteIndex];
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (!FromSourceBase(aChild))
					{
						if (aChild->m_name == "sprites")
						{
							aChild->GetIdArray(DataType::ID_SPRITE, m_sprites);
							TP_VERIFY(m_sprites.size() == NUM_SPRITE_INDICES, aChild->m_debugInfo, "Invalid number of sprites.");
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInts(m_sprites);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadUInts(m_sprites))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>		m_sprites;
		};

	}

}