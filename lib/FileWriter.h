#pragma once

#include <tpublic/IWriter.h>

namespace tpublic
{

	class FileWriter
		: public IWriter
	{
	public:
					FileWriter(
						const char*			aPath);
		virtual		~FileWriter();

		// IWriter implementation
		void		Write(
						const void*			aBuffer,
						size_t				aBufferSize) override;

	private:

		std::string	m_path;
		FILE*		m_f;
	};

}