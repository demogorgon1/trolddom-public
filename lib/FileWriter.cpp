#include "Pcheader.h"

#include <kpublic/DataErrorHandling.h>

#include "FileWriter.h"

namespace kpublic
{

	FileWriter::FileWriter(
		const char*		aPath)
		: m_f(NULL)
		, m_path(aPath)
	{
		m_f = fopen(aPath, "wb");
		KP_CHECK(m_f != NULL, "Failed to open file for output: %s", aPath);
	}

	FileWriter::~FileWriter()
	{
		assert(m_f != NULL);
		fclose(m_f);
	}

	//------------------------------------------------------------------------

	void		
	FileWriter::Write(
		const void*		aBuffer,
		size_t			aBufferSize) 
	{
		assert(m_f != NULL);
		size_t bytes = fwrite(aBuffer, 1, aBufferSize, m_f);
		KP_CHECK(bytes == aBufferSize, "Failed to write to file: %s", m_path.c_str());
	}

}