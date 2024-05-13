#include "Pcheader.h"

#include <tpublic/DataReference.h>
#include <tpublic/IReader.h>
#include <tpublic/IWriter.h>
#include <tpublic/SourceNode.h>

namespace tpublic
{

	DataReference::DataReference()
	{

	}
	
	DataReference::DataReference(
		const SourceNode*	aSource)
	{
		TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing data type annotation.");
		m_type = DataType::StringToId(aSource->m_annotation->GetIdentifier());
		TP_VERIFY(m_type != DataType::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid data type.", aSource->m_annotation->GetIdentifier());
		m_id = aSource->m_sourceContext->m_persistentIdTable->GetId(m_type, aSource->GetIdentifier());
	}
	
	DataReference::~DataReference()
	{

	}

	void	
	DataReference::ToStream(
		IWriter*			aWriter) const
	{
		aWriter->WritePOD(m_type);
		aWriter->WriteUInt(m_id);
	}
	
	bool	
	DataReference::FromStream(
		IReader*			aReader)
	{
		if(!aReader->ReadPOD(m_type))
			return false;
		if(!aReader->ReadUInt(m_id))
			return false;
		return true;
	}

	bool	
	DataReference::IsSet() const
	{
		return m_type != DataType::INVALID_ID;
	}

}