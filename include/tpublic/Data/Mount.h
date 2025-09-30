#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Mount
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MOUNT;
			static const bool TAGGED = false;

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
					const SourceNode*	aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "sprite")
							m_spriteId = aChild->GetId(DataType::ID_SPRITE);
						else if (aChild->m_name == "walk_sprites")
							aChild->GetIdArray(DataType::ID_SPRITE, m_walkSpriteIds);
						else if(aChild->m_name == "y_offset")
							m_yOffset = aChild->GetInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_spriteId);
				aStream->WriteUInts(m_walkSpriteIds);
				aStream->WriteInt(m_yOffset);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadUInt(m_spriteId))
					return false;
				if (!aStream->ReadUInts(m_walkSpriteIds))
					return false;
				if (!aStream->ReadInt(m_yOffset))
					return false;
				return true;
			}

			// Public data
			uint32_t				m_spriteId = 0;
			std::vector<uint32_t>	m_walkSpriteIds;
			int32_t					m_yOffset = 0;
		};


	}

}