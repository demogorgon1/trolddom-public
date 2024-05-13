#pragma once

#include "DataType.h"

namespace tpublic
{

	class IReader;
	class IWriter;
	class SourceNode;
	
	class DataReference
	{
	public:
				DataReference();
				DataReference(
					const SourceNode*		aSource);
				~DataReference();

		void	ToStream(
					IWriter*				aWriter) const;
		bool	FromStream(
					IReader*				aReader);
		bool	IsSet() const;

		// Public data
		DataType::Id	m_type = DataType::INVALID_ID;	
		uint32_t		m_id = 0;
	};

}