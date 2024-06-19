#include "Pcheader.h"

#include <tpublic/Document.h>
#include <tpublic/IReader.h>
#include <tpublic/IWriter.h>
#include <tpublic/SourceNode.h>

namespace tpublic
{

	Document::Node::Node(
		const SourceNode*	aSource)
	{
		if(aSource->m_name == "heading")
		{
			m_type = TYPE_HEADING;
			m_string = aSource->GetString();
		}
		else if(aSource->m_name == "text")
		{
			m_type = TYPE_TEXT;
			m_string = aSource->GetString();
		}
		else if(aSource->m_name == "list")
		{
			m_type = TYPE_LIST;				
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{					
				m_children.push_back(std::make_unique<Node>(aChild->GetObject()));
			});
		}
		else if(aSource->m_name == "newline")
		{
			m_type = TYPE_NEWLINE;
		}
		else if (aSource->m_type == SourceNode::TYPE_OBJECT)
		{
			m_type = TYPE_COMPOUND;
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				m_children.push_back(std::make_unique<Node>(aChild));
			});
		}
		else
		{
			TP_VERIFY(false, aSource->m_debugInfo, "Syntax error.");
		}		
	}
	
	void	
	Document::Node::ToStream(
		IWriter*			aWriter) const
	{
		aWriter->WritePOD(m_type);
		aWriter->WriteString(m_string);
		aWriter->WriteObjectPointers(m_children);
	}
	
	bool	
	Document::Node::FromStream(
		IReader*			aReader)
	{
		if(!aReader->ReadPOD(m_type))
			return false;
		if(!aReader->ReadString(m_string))
			return false;
		if(!aReader->ReadObjectPointers(m_children))
			return false;
		return true;
	}

	uint32_t	
	Document::Node::CalculateHash() const
	{
		Hash::CheckSum hash;
		hash.AddPOD(m_type);
		hash.AddString(m_string.c_str());
		for(const std::unique_ptr<Node>& child : m_children)
		{
			uint32_t childHash = child->CalculateHash();
			hash.AddPOD(childHash);
		}
		return hash.m_hash;
	}
	
	//-------------------------------------------------------------------------------

	Document::Document(
		const SourceNode*	aSource)
	{
		m_root = std::make_unique<Node>(aSource);
	}

	void	
	Document::ToStream(
		IWriter*			aWriter) const
	{
		aWriter->WriteObjectPointer(m_root);
	}
	
	bool	
	Document::FromStream(
		IReader*			aReader)
	{
		if(!aReader->ReadObjectPointer(m_root))
			return false;
		return true;
	}

}