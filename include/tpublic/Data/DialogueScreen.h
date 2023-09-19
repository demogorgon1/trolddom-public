#pragma once

#include "../DataBase.h"
#include "../DialogueScript.h"

namespace tpublic
{

	namespace Data
	{

		struct DialogueScreen
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_DIALOGUE_SCREEN;

			struct Option
			{
				Option()
				{

				}

				Option(
					const Parser::Node*	aSource)
				{
					aSource->ForEachChild([&](
						const Parser::Node* aChild)
					{
						if(aChild->m_name == "goto")
						{
							m_dialogueScreenId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_DIALOGUE_SCREEN, aChild->GetIdentifier());
						}
						else if (aChild->m_name == "text")
						{
							m_string = aChild->GetString();
						}
						else if(aChild->m_name == "script")
						{
							m_dialogueScript = DialogueScript::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_dialogueScript != DialogueScript::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid dialogue script.", aChild->GetIdentifier());
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
						}
					});				
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteString(m_string);
					aWriter->WriteUInt(m_dialogueScreenId);
					aWriter->WritePOD(m_dialogueScript);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadString(m_string))
						return false;
					if(!aReader->ReadUInt(m_dialogueScreenId))
						return false;
					if (!aReader->ReadPOD(m_dialogueScript))
						return false;
					return true;
				}

				// Public data
				std::string			m_string;
				uint32_t			m_dialogueScreenId = 0;		
				DialogueScript::Id	m_dialogueScript = DialogueScript::ID_NONE;
			};

			struct Sell
			{
				Sell()
				{

				}

				Sell(
					const Parser::Node* aSource)
				{
					m_itemId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ITEM, aSource->m_name.c_str());
					aSource->ForEachChild([&](
						const Parser::Node* aChild)
					{
						if(aChild->m_name == "cost")
							m_cost = aChild->GetUInt32();
						else if(aChild->m_name == "quantity")
							m_quantity = aChild->GetUInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_itemId);
					aWriter->WriteUInt(m_cost);
					aWriter->WriteUInt(m_quantity);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if (!aReader->ReadUInt(m_itemId))
						return false;
					if (!aReader->ReadUInt(m_cost))
						return false;
					if (!aReader->ReadUInt(m_quantity))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_itemId = 0;
				uint32_t			m_cost = 0;
				uint32_t			m_quantity = 1;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// DataBase implementation
			void
			FromSource(
				const Parser::Node*		aNode) override
			{
				auto appendString = [&](
					std::string& aString,
					const char*  aAppend)
				{
					size_t length = aString.length();
					if(length > 0 && aString[length - 1] != '\n')
						aString.push_back(' ');
					aString.append(aAppend);
				};				

				aNode->ForEachChild([&](
					const Parser::Node* aChild)
				{
					if(aChild->m_name == "text")
						aChild->GetArray()->ForEachChild([&](const Parser::Node* aLine) { appendString(m_text, aLine->GetString()); });
					else if(aChild->m_name == "options")
						aChild->GetArray()->ForEachChild([&](const Parser::Node* aOption) { m_options.push_back(Option(aOption)); });
					else if(aChild->m_tag == "sell")
						m_sell.push_back(Sell(aChild));
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteString(m_text);
				aStream->WriteObjects(m_options);
				aStream->WriteObjects(m_sell);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadString(m_text, 16 * 1024))
					return false;
				if (!aStream->ReadObjects(m_options))
					return false;
				if (!aStream->ReadObjects(m_sell))
					return false;
				return true;
			}

			// Public data
			std::string							m_text;
			std::vector<Option>					m_options;
			std::vector<Sell>					m_sell;
		};

	}

}