#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Critter
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_CRITTER;
			static const bool TAGGED = false;

			enum Flag : uint8_t
			{
				FLAG_CLIENT		= 0x01
			};

			static uint8_t
			SourceToFlags(
				const SourceNode*							aSource)
			{
				uint8_t flags = 0;
				aSource->GetArray()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->IsIdentifier("client"))
						flags |= FLAG_CLIENT;
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid flag.", aChild->GetIdentifier());
				});
				return flags;
			}

			void
			Verify() const
			{
				VerifyBase();
			}

			bool
			IsValidTile(
				uint32_t				aTileSpriteId) const
			{
				for(uint32_t tileSpriteId : m_tileSpriteIds)
				{
					if(tileSpriteId == aTileSpriteId)
						return true;
				}
				return false;
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{	
				aNode->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "flags")
							m_flags |= SourceToFlags(aChild);
						else if (aChild->m_name == "tiles")
							aChild->GetIdArray(DataType::ID_SPRITE, m_tileSpriteIds);
						else if (aChild->m_name == "max_distance")
							m_maxDistance = aChild->GetInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WritePOD(m_flags);
				aStream->WriteUInts(m_tileSpriteIds);
				aStream->WriteInt(m_maxDistance);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadPOD(m_flags))
					return false;
				if (!aStream->ReadUInts(m_tileSpriteIds))
					return false;
				if (!aStream->ReadInt(m_maxDistance))
					return false;
				return true;
			}

			// Public data
			uint8_t					m_flags = 0;
			std::vector<uint32_t>	m_tileSpriteIds;
			int32_t					m_maxDistance = 0;
		};

	}

}