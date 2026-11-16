#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Sound
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_SOUND;
			static const bool TAGGED = true;

			void
			Verify() const
			{
				VerifyBase();
				TP_VERIFY(m_source.empty() == m_musicGen, m_debugInfo, "'%s' needs either a source or music_gen, not both.", m_name.c_str());
				TP_VERIFY(!m_musicGen || m_musicGenParamsId != 0, m_debugInfo, "'%s' needs music_gen params.", m_name.c_str());
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
						if(aChild->m_name == "source")
							m_source = (aChild->m_realPath + "/") + aChild->GetString();
						else if(aChild->m_name == "streamed")
							m_streamed = aChild->GetBool();
						else if (aChild->m_name == "cooldown_ms")
							m_cooldownMS = aChild->GetUInt32();
						else if (aChild->m_name == "volume")
							m_volume = aChild->GetFloat();
						else if(aChild->m_name == "music_gen")
							_ReadMusicGen(aChild);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteBool(m_streamed);
				aWriter->WriteUInt(m_cooldownMS);
				aWriter->WriteFloat(m_volume);
				aWriter->WriteBool(m_musicGen);
				aWriter->WriteUInt(m_musicGenParamsId);
				aWriter->WriteUInt(m_musicGenSeed);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadBool(m_streamed))
					return false;
				if (!aReader->ReadUInt(m_cooldownMS))
					return false;
				if (!aReader->ReadFloat(m_volume))
					return false;
				if(!aReader->ReadBool(m_musicGen))
					return false;
				if(!aReader->ReadUInt(m_musicGenParamsId))
					return false;
				if(!aReader->ReadUInt(m_musicGenSeed))
					return false;
				return true;
			}

			// Public data			
			bool				m_streamed = false;
			uint32_t			m_cooldownMS = 80;
			float				m_volume = 1.0f;
			bool				m_musicGen = false;		// Generated music instead of a source file
			uint32_t			m_musicGenParamsId = 0;
			uint32_t			m_musicGenSeed = 0;
			std::string			m_source; // Not serialized

		private:

			void
			_ReadMusicGen(
				const SourceNode*		aSource)
			{
				m_musicGen = true;

				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "params")
						m_musicGenParamsId = aChild->GetId(DataType::ID_MUSIC_GEN_PARAMS);
					else if(aChild->m_name == "seed")
						m_musicGenSeed = aChild->GetUInt32();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}
		};

	}

}