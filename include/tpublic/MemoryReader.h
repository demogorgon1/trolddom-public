#pragma once

#include "IReader.h"

namespace tpublic
{

	class MemoryReader
		: public IReader
	{
	public:
					MemoryReader(
						const void*			aBuffer,
						size_t				aBufferSize);
		virtual		~MemoryReader();

		// IReader implementation
		bool		IsEnd() const override;
		size_t		Read(
						void*				aBuffer,
						size_t				aBufferSize) override;

	private:

		const uint8_t*	m_p;
		size_t			m_remaining;
	};

}