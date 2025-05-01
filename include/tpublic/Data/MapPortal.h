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
			static const bool TAGGED = false;

			struct Animation
			{
				Animation()
				{

				}

				Animation(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "frames")
							aChild->GetIdArray(DataType::ID_SPRITE, m_frames);
						else if(aChild->m_name == "frame_interval")
							m_frameInterval = aChild->GetUInt32();
						else if(aChild->m_name == "color")
							m_color = Image::RGBA(aChild);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInts(m_frames);
					aWriter->WriteUInt(m_frameInterval);
					aWriter->WritePOD(m_color);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInts(m_frames))
						return false;
					if(!aReader->ReadUInt(m_frameInterval))
						return false;
					if (!aReader->ReadPOD(m_color))
						return false;
					return true;
				}

				// Public data
				std::vector<uint32_t>		m_frames;
				uint32_t					m_frameInterval = 100;
				Image::RGBA					m_color = { 255, 255, 255, 255 };
			};

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
						if(aChild->m_name == "map")
							m_mapId = aChild->GetId(DataType::ID_MAP);
						else if (aChild->m_name == "spawn")
							m_mapPlayerSpawnId = aChild->GetId(DataType::ID_MAP_PLAYER_SPAWN);
						else if(aChild->m_name == "animation")
							m_animation = Animation(aChild);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_mapId);
				aStream->WriteUInt(m_mapPlayerSpawnId);
				aStream->WriteOptionalObject(m_animation);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInt(m_mapId))
					return false;
				if (!aStream->ReadUInt(m_mapPlayerSpawnId))
					return false;
				if(!aStream->ReadOptionalObject(m_animation))
					return false;
				return true;
			}

			// Public data
			uint32_t					m_mapId = 0;
			uint32_t					m_mapPlayerSpawnId = 0;
			std::optional<Animation>	m_animation;
		};

	}

}