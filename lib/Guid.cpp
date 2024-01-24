#include "Pcheader.h"

#include <tpublic/DataErrorHandling.h>
#include <tpublic/Guid.h>

namespace tpublic
{

	namespace
	{
		
		uint8_t
		_CharToHex(
			char		aChar)
		{
			if(aChar >= '0' && aChar <= '9')
				return (uint8_t)(aChar - '0');
			if (aChar >= 'A' && aChar <= 'F')
				return 10 + (uint8_t)(aChar - 'A');
			if (aChar >= 'a' && aChar <= 'f')
				return 10 + (uint8_t)(aChar - 'a');
			return 0xFF;
		}

		bool
		_TryHexDecode(			
			const char*	aString,
			size_t		aStringLength,
			uint8_t		aOutData[Guid::SIZE])
		{
			if(aStringLength != Guid::SIZE * 2)
				return false;

			for(size_t i = 0; i < Guid::SIZE; i++)
			{
				const char* p = aString + i * 2;
				uint8_t p0 = _CharToHex(p[0]);
				uint8_t p1 = _CharToHex(p[1]);
				if(p0 == 0xFF || p1 == 0xFF)
					return false;

				aOutData[i] = (p0 << 4) | p1;
			}

			return true;
		}

	}

	//-------------------------------------------------------------------------------------

	Guid 
	Guid::Null()		
	{ 
		return Guid(); 
	}

	//-------------------------------------------------------------------------------------

	Guid::Guid()
	{
		memset(m_data, 0, sizeof(m_data));
	}

	Guid::Guid(
		const Guid&		aCopy)
	{
		*this = aCopy;
	}

	Guid::Guid(
		const char*		aString)
	{
		size_t len = strlen(aString);

		if(len == 0)
		{
			memset(m_data, 0, sizeof(m_data));
		}
		else if (!_TryHexDecode(aString, len, m_data))
		{
			TP_CHECK(len < SIZE - 2, "Invalid string guid.");

			m_data[0] = 0xFF;

			size_t tailingZeroes = SIZE - len - 1;
			if (tailingZeroes > 0)
				memset(m_data + len + 1, 0, tailingZeroes);

			memcpy(m_data + 1, aString, len);
		}
	}

	Guid::~Guid()
	{
	}

	Guid&
	Guid::operator=(
		const Guid&		aCopy)
	{
		memcpy(m_data, aCopy.m_data, sizeof(m_data));
		return *this;
	}

	bool
	Guid::IsSet() const
	{
		return m_data[0] != 0;
	}

	const uint8_t*
	Guid::GetBuffer() const
	{
		return m_data;
	}

	uint8_t*
	Guid::GetBuffer()
	{
		return m_data;
	}

	void
	Guid::ToString(
		std::string&	aOut) const
	{
		if(m_data[0] == 0xFF)
		{
			aOut = ((const char*)m_data) + 1;
		}
		else
		{
			char buffer[SIZE * 2 + 1];
			for(size_t i = 0; i < SIZE; i++)
				TP_STRING_FORMAT(&buffer[i * 2], 3, "%02x", m_data[i]);			
			aOut = buffer;
		}
	}

	std::string
	Guid::AsString() const 
	{
		std::string t;
		ToString(t);
		return t;
	}

	uint32_t
	Guid::GetHash32() const
	{
		const uint32_t* t = (const uint32_t*)m_data;
		assert(sizeof(uint32_t) * 4 == SIZE);
		return t[0] ^ t[1] ^ t[2] ^ t[3];
	}

}
