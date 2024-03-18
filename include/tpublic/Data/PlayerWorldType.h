#pragma once

#include "../DataBase.h"
#include "../PlayerWorld.h"

namespace tpublic
{

	namespace Data
	{

		struct PlayerWorldType
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_PLAYER_WORLD_TYPE;
			static const bool TAGGED = true;

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
					if(!FromSourceBase(aChild))
					{
						if(aChild->m_name == "string")
						{
							m_string = aChild->GetString();
						}
						else if(aChild->m_name == "size")
						{
							m_size = PlayerWorld::StringToSize(aChild->GetIdentifier());
							TP_VERIFY(m_size != PlayerWorld::INVALID_SIZE, aChild->m_debugInfo, "'%s' is not a valid size.", aChild->GetIdentifier());
						}
						else if (aChild->m_name == "tiles")
						{
							aChild->GetIdArray(DataType::ID_SPRITE, m_tiles);
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
				IWriter*				aWriter) const override
			{
				aWriter->WriteString(m_string);
				aWriter->WritePOD(m_size);
				aWriter->WriteUInts(m_tiles);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadString(m_string))
					return false;
				if(!aReader->ReadPOD(m_size))
					return false;
				if(!aReader->ReadUInts(m_tiles))
					return false;
				return true;
			}

			// Public data			
			std::string				m_string;
			PlayerWorld::Size		m_size = PlayerWorld::INVALID_SIZE;
			std::vector<uint32_t>	m_tiles;
		};

	}

}