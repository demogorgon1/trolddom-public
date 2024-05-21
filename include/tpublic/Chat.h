#pragma once

#include "SourceNode.h"

namespace tpublic
{

	class Chat
	{
	public:
		enum Type : uint8_t
		{
			INVALID_TYPE,

			TYPE_SAY,
			TYPE_YELL
		};

		static Type
		SourceToType(
			const SourceNode*	aSource)
		{
			std::string t(aSource->GetIdentifier());
			if(t == "say")
				return TYPE_SAY;
			else if(t == "yell")
				return TYPE_YELL;
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid chat type.", aSource->GetIdentifier());
			return INVALID_TYPE;
		}

		Chat()
		{

		}

		Chat(
			const SourceNode*		aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "type")
					m_type = SourceToType(aChild);
				else if(aChild->m_name == "text")
					m_text = aChild->GetString();
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}

		void
		ToStream(
			IWriter*				aWriter) const
		{
			aWriter->WritePOD(m_type);
			aWriter->WriteString(m_text);
		}

		bool
		FromStream(
			IReader*				aReader)
		{
			if(!aReader->ReadPOD(m_type))
				return false;
			if(!aReader->ReadString(m_text))
				return false;
			return true;
		}

		// Public data
		Type			m_type = INVALID_TYPE;
		std::string		m_text;
	};

}