#pragma once

#include "IReader.h"
#include "IWriter.h"
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

		enum Flag : uint8_t
		{
			FLAG_NO_CHAT_BUBBLE = 0x01,
			FLAG_INSTANCE = 0x02
		};

		static Type
		SourceToType(
			const SourceNode*		aSource)
		{
			std::string t(aSource->GetIdentifier());
			if(t == "say")
				return TYPE_SAY;
			else if(t == "yell")
				return TYPE_YELL;
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid chat type.", aSource->GetIdentifier());
			return INVALID_TYPE;
		}

		static uint8_t
		SourceToFlags(
			const SourceNode*		aSource)
		{
			uint8_t flags = 0;

			aSource->GetArray()->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->IsIdentifier("no_chat_bubble"))
					flags |= FLAG_NO_CHAT_BUBBLE;
				else if (aChild->IsIdentifier("instance"))
					flags |= FLAG_INSTANCE;
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid flag.", aChild->GetIdentifier());
			});

			return flags;
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
				else if (aChild->m_name == "flags")
					m_flags = SourceToFlags(aChild);
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
			aWriter->WritePOD(m_flags);
		}

		bool
		FromStream(
			IReader*				aReader)
		{
			if(!aReader->ReadPOD(m_type))
				return false;
			if(!aReader->ReadString(m_text))
				return false;
			if (!aReader->ReadPOD(m_flags))
				return false;
			return true;
		}

		// Public data
		Type			m_type = INVALID_TYPE;
		std::string		m_text;
		uint8_t			m_flags = 0;
	};

}