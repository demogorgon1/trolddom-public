#include "Pcheader.h"

#include <tpublic/UTF8.h>

namespace
{
	// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
	// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

	#define UTF8_ACCEPT 0
	#define UTF8_REJECT 1

	static const uint8_t UTF8D[] = 
	{
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
		8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
		0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
		0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
		0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
		1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
		1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
		1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
	};

	uint32_t
	_Decode(uint32_t* state, uint32_t* codep, uint32_t byte)
	{
		uint32_t type = UTF8D[byte];

		*codep = (*state != UTF8_ACCEPT) ?
			(byte & 0x3fu) | (*codep << 6) :
		(0xff >> type) & (byte);

		*state = UTF8D[256 + *state*16 + type];
		return *state;
	}
}

namespace tpublic::UTF8
{

	Decoder::Decoder(
		const char*		aUTF8Text)
		: m_pc((const uint8_t*)aUTF8Text)
		, m_state(0)
		, m_emitNullTermination(false)
		, m_ended(false)
	{

	}

	void		
	Decoder::SetEmitNullTermination(
		bool			aEmitNullTermination)
	{
		m_emitNullTermination = aEmitNullTermination;
	}

	bool		
	Decoder::GetNextCharacterCode(
		uint32_t&		aOut,
		size_t*			aOutSize)
	{	
		if(m_ended)
			return false;

		size_t size = 0;

		for(;;)
		{
			uint32_t t = (uint32_t)(*m_pc);
			if(t == 0)
			{
				if(m_emitNullTermination)
				{
					aOut = 0;

					if (aOutSize != NULL)
						*aOutSize = 1;

					m_ended = true;
					return true;
				}

				return false;
			}

			m_pc++;
			size++;

			if(!_Decode(&m_state, &aOut, t))
				break;
		}

		if(aOutSize != NULL)
			*aOutSize = size;

		return true;
	}

	bool		
	Decoder::IsEnd() const
	{
		return *m_pc == '\0';
	}

	//--------------------------------------------------------------------------------------

	// 1	7	U+0000	U+007F		0xxxxxxx
	// 2	11	U+0080	U+07FF		110xxxxx	10xxxxxx
	// 3	16	U+0800	U+FFFF		1110xxxx	10xxxxxx	10xxxxxx
	// 4	21	U+10000	U+10FFFF	11110xxx	10xxxxxx	10xxxxxx	10xxxxxx

	void		
	Encoder::EncodeCharacter(
		uint32_t		aCharacterCode)
	{
		assert(!m_finalized);
		if(aCharacterCode <= 0x007F)
		{
			size_t oldLen = m_utf8.size();
			m_utf8.resize(oldLen + 1);
			uint8_t* p = (uint8_t*)&m_utf8[oldLen];
			p[0] = (uint8_t)aCharacterCode;
		}
		else if(aCharacterCode <= 0x07FF)
		{
			size_t oldLen = m_utf8.size();
			m_utf8.resize(oldLen + 2);
			uint8_t* p = (uint8_t*)&m_utf8[oldLen];
			p[0] = (uint8_t)(((aCharacterCode >> 6) & 0x1F) | 0xC0);
			p[1] = (uint8_t)((aCharacterCode & 0x3F) | 0x80);
		}
		else if(aCharacterCode <= 0xFFFF)
		{
			size_t oldLen = m_utf8.size();
			m_utf8.resize(oldLen + 3);
			uint8_t* p = (uint8_t*)&m_utf8[oldLen];
			p[0] = (uint8_t)(((aCharacterCode >> 12) & 0x0F) | 0xE0);
			p[1] = (uint8_t)(((aCharacterCode >> 6) & 0x3F) | 0x80);
			p[2] = (uint8_t)((aCharacterCode & 0x3F) | 0x80);
		}
		else 
		{
			size_t oldLen = m_utf8.size();
			m_utf8.resize(oldLen + 4);
			uint8_t* p = (uint8_t*)&m_utf8[oldLen];
			p[0] = (uint8_t)(((aCharacterCode >> 18) & 0x07) | 0xF0);
			p[1] = (uint8_t)(((aCharacterCode >> 12) & 0x3F) | 0x80);
			p[2] = (uint8_t)(((aCharacterCode >> 6) & 0x3F) | 0x80);
			p[3] = (uint8_t)((aCharacterCode & 0x3F) | 0x80);
		}
	}
		
	void		
	Encoder::Clear()
	{
		m_utf8.clear();
		m_finalized = false;
	}

	const char*
	Encoder::Finalize()
	{
		assert(!m_finalized);
		m_utf8.push_back(0);
		m_finalized = true;
		return (const char*)&m_utf8[0];
	}

}