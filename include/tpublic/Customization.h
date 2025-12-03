#pragma once

#include "SourceNode.h"

namespace tpublic
{

	class Customization
	{
	public:
		static const uint8_t VERSION = 0;

		Customization()
		{

		}

		Customization(
			const SourceNode*		aSource)
		{
			aSource->GetObject()->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "body")
					m_body = aChild->GetUInt8();
				else if(aChild->m_name == "hair")
					m_hair = aChild->GetUInt8();
				else if (aChild->m_name == "hair_color")
					m_hairColor = aChild->GetUInt8();
				else if(aChild->m_name == "beard")
					m_beard = aChild->GetUInt8();
				else if (aChild->m_name == "beard_color")
					m_beardColor = aChild->GetUInt8();
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}

		void
		ToStream(
			IWriter*				aWriter) const
		{
			uint8_t packed[3];
			packed[0] = VERSION | (m_hair << 4);
			packed[1] = m_beard | (m_hairColor << 4);
			packed[2] = m_beardColor | (m_body << 4);
			aWriter->Write(packed, sizeof(packed));
		}

		bool
		FromStream(
			IReader*				aReader) 
		{
			uint8_t packed[3];
			if(aReader->Read(packed, sizeof(packed)) != sizeof(packed))
				return false;

			uint8_t version = packed[0] & 0x0F;

			if(version == 0)
			{
				m_hair = (packed[0] & 0xF0) >> 4;
				m_beard = packed[1] & 0x0F;
				m_hairColor = (packed[1] & 0xF0) >> 4;
				m_beardColor = packed[2] & 0x0F;
				m_body = (packed[2] & 0xF0) >> 4;
			}

			return true;
		}

		bool
		operator==(
			const Customization&	aOther) const
		{
			return m_body == aOther.m_body && m_hair == aOther.m_hair && m_beard == aOther.m_beard && m_hairColor == aOther.m_hairColor && m_beardColor == aOther.m_beardColor;
		}

		bool
		operator!=(
			const Customization&	aOther) const
		{
			return !this->operator==(aOther);
		}

		// Public data
		uint8_t		m_body = 0;
		uint8_t		m_hair = 0;
		uint8_t		m_beard = 0;
		uint8_t		m_hairColor = 0;
		uint8_t		m_beardColor = 0;
	};

}