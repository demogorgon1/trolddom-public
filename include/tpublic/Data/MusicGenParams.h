#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		// Shared settings for generated music. Sounds reference these and add their own seed.
		// The dials are the base values for a place (0-1), with gameplay adding on top.
		// The instruments can differ per track.
		struct MusicGenParams
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MUSIC_GEN_PARAMS;
			static const bool TAGGED = false;

			void
			Verify() const
			{
				VerifyBase();
				TP_VERIFY(m_instrumentsId != 0, m_debugInfo, "'%s' needs instruments.", m_name.c_str());
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
						if(aChild->m_name == "instruments")
							m_instrumentsId = aChild->GetId(DataType::ID_INSTRUMENTS);
						else if(aChild->m_name == "intensity")
							m_intensity = _GetDial(aChild);
						else if(aChild->m_name == "danger")
							m_danger = _GetDial(aChild);
						else if(aChild->m_name == "darkness")
							m_darkness = _GetDial(aChild);
						else if(aChild->m_name == "activity")
							m_activity = _GetDial(aChild);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteUInt(m_instrumentsId);
				aWriter->WriteFloat(m_intensity);
				aWriter->WriteFloat(m_danger);
				aWriter->WriteFloat(m_darkness);
				aWriter->WriteFloat(m_activity);
			}

			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadUInt(m_instrumentsId))
					return false;
				if(!aReader->ReadFloat(m_intensity))
					return false;
				if(!aReader->ReadFloat(m_danger))
					return false;
				if(!aReader->ReadFloat(m_darkness))
					return false;
				if(!aReader->ReadFloat(m_activity))
					return false;
				return true;
			}

			// Public data
			uint32_t			m_instrumentsId = 0;
			float				m_intensity = 0.0f;
			float				m_danger = 0.0f;
			float				m_darkness = 0.5f;
			float				m_activity = 1.0f;

		private:

			static float
			_GetDial(
				const SourceNode*		aSource)
			{
				float value = aSource->GetFloat();
				TP_VERIFY(value >= 0.0f && value <= 1.0f, aSource->m_debugInfo, "'%s' must be between 0 and 1.", aSource->m_name.c_str());
				return value;
			}
		};

	}

}
