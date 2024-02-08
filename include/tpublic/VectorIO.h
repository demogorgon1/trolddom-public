#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic::VectorIO
{

	class Reader
		: public IReader
	{
	public:
		Reader(
			const std::vector<uint8_t>&		aBuffer,
			const IReader*					aParentStream = NULL)
			: m_p(aBuffer.size() > 0 ? &aBuffer[0] : NULL)
			, m_remaining(aBuffer.size())
			, m_parentStream(aParentStream)
		{

		}

		virtual
		~Reader()
		{

		}

		// IReader implementation
		bool						
		IsEnd() const override
		{
			return m_remaining == 0;
		}

		size_t						
		Read(
			void*							aBuffer,
			size_t							aBufferSize) override
		{
			size_t toCopy = aBufferSize <= m_remaining ? aBufferSize : m_remaining;
			memcpy(aBuffer, m_p, toCopy);
			m_remaining -= toCopy;
			m_p += toCopy;
			return toCopy;
		}

		// Redirect to parent stream (this is awful)
		const AuraEffectFactory* GetAuraEffectFactory() const override { assert(m_parentStream != NULL); return m_parentStream->GetAuraEffectFactory(); }
		const ComponentManager* GetComponentManager() const override { assert(m_parentStream != NULL); return m_parentStream->GetComponentManager(); }
		const DirectEffectFactory* GetDirectEffectFactory() const override { assert(m_parentStream != NULL); return m_parentStream->GetDirectEffectFactory(); }
		const MapGeneratorFactory* GetMapGeneratorFactory() const override { assert(m_parentStream != NULL); return m_parentStream->GetMapGeneratorFactory(); }
		const ObjectiveTypeFactory* GetObjectiveTypeFactory() const override { assert(m_parentStream != NULL); return m_parentStream->GetObjectiveTypeFactory(); }

	private:

		const uint8_t*			m_p;
		size_t					m_remaining;
		const IReader*			m_parentStream;
	};

	class Writer
		: public IWriter
	{
	public:
		Writer(
			std::vector<uint8_t>&			aBuffer)
			: m_buffer(aBuffer)
		{

		}

		virtual
		~Writer()
		{

		}

		// IWriter implementation
		void		
		Write(
			const void*						aBuffer,
			size_t							aBufferSize) 
		{
			if(aBufferSize == 0)
				return;

			size_t oldSize = m_buffer.size();
			size_t newSize = oldSize + aBufferSize;
			m_buffer.resize(newSize);
			memcpy(&m_buffer[oldSize], aBuffer, aBufferSize);
		}

	private:

		std::vector<uint8_t>&	m_buffer;
	};

}