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
						else if (aChild->m_name == "walls")
						{
							aChild->GetIdArray(DataType::ID_WALL, m_walls);
						}
						else if (aChild->m_name == "view_attenuation")
						{
							m_viewAttenuation = aChild->GetUInt32();
						}
						else if (aChild->m_name == "view_attenuation_bias")
						{
							m_viewAttenuationBias = aChild->GetUInt32();
						}
						else if (aChild->m_name == "view_hidden_visibility")
						{
							m_viewHiddenVisibility = aChild->GetUInt32();
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
				aWriter->WriteUInts(m_walls);
				aWriter->WriteUInt(m_viewAttenuation);
				aWriter->WriteUInt(m_viewAttenuationBias);
				aWriter->WriteUInt(m_viewHiddenVisibility);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadString(m_string))
					return false;
				if(!aReader->ReadPOD(m_size))
					return false;
				if (!aReader->ReadUInts(m_tiles))
					return false;
				if (!aReader->ReadUInts(m_walls))
					return false;
				if (!aReader->ReadUInt(m_viewAttenuation))
					return false;
				if (!aReader->ReadUInt(m_viewAttenuationBias))
					return false;
				if (!aReader->ReadUInt(m_viewHiddenVisibility))
					return false;
				return true;
			}

			// Public data			
			std::string				m_string;
			PlayerWorld::Size		m_size = PlayerWorld::INVALID_SIZE;
			std::vector<uint32_t>	m_tiles;
			std::vector<uint32_t>	m_walls;
			uint32_t				m_viewAttenuation = 0;
			uint32_t				m_viewAttenuationBias = 0;
			uint32_t				m_viewHiddenVisibility = 0;
		};

	}

}