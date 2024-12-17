#pragma once

#include "../DataBase.h"
#include "../DialogueScript.h"

namespace tpublic
{

	namespace Data
	{

		struct DialogueRoot
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_DIALOGUE_ROOT;
			static const bool TAGGED = true;

			struct Verb
			{
				enum Type : uint8_t
				{
					TYPE_TALK,
					TYPE_READ,
					TYPE_OPEN
				};

				void
				FromSource(
					const SourceNode*	aSource)
				{
					std::string_view t(aSource->GetIdentifier());
					if (t == "talk")
						m_type = TYPE_TALK;
					else if (t == "read")
						m_type = TYPE_READ;
					else if (t == "open")
						m_type = TYPE_OPEN;
					else
						TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid verb.", aSource->GetIdentifier());
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_type);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadPOD(m_type))
						return false;
					return true;
				}

				const char*
				GetDisplayString() const
				{
					switch(m_type)
					{
					case TYPE_TALK:		return "Talk";
					case TYPE_READ:		return "Read";
					case TYPE_OPEN:		return "Open";
					default:			break;
					}
					assert(false);
					return "";
				}

				// Public data
				Type				m_type = TYPE_TALK;
			};

			struct Entry
			{
				Entry()
				{

				}

				Entry(
					const SourceNode*	aSource)
				{
					m_dialogueScreenId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_DIALOGUE_SCREEN, aSource->m_name.c_str());

					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "conditions")
						{
							aChild->GetIdArray(DataType::ID_EXPRESSION, m_conditionExpressionIds);
						}
						else if (aChild->m_name == "inv_conditions")
						{
							aChild->GetIdArray(DataType::ID_EXPRESSION, m_invConditionExpressionIds);
						}
						else if (aChild->m_name == "script")
						{
							m_dialogueScript = DialogueScript::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_dialogueScript != DialogueScript::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid dialogue script.", aChild->GetIdentifier());
						}
						else if(aChild->m_name == "is_class")
						{
							aChild->GetIdArray(DataType::ID_CLASS, m_isClassId);
						}
						else if (aChild->m_name == "is_not_class")
						{
							aChild->GetIdArray(DataType::ID_CLASS, m_isNotClassId);
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_dialogueScreenId);
					aWriter->WriteUInts(m_conditionExpressionIds);
					aWriter->WriteUInts(m_invConditionExpressionIds);
					aWriter->WritePOD(m_dialogueScript);
					aWriter->WriteUInts(m_isClassId);
					aWriter->WriteUInts(m_isNotClassId);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadUInt(m_dialogueScreenId))
						return false;
					if (!aReader->ReadUInts(m_conditionExpressionIds))
						return false;
					if (!aReader->ReadUInts(m_invConditionExpressionIds))
						return false;
					if(!aReader->ReadPOD(m_dialogueScript))
						return false;
					if(!aReader->ReadUInts(m_isClassId))
						return false;
					if (!aReader->ReadUInts(m_isNotClassId))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_dialogueScreenId = 0;
				std::vector<uint32_t>	m_conditionExpressionIds;
				std::vector<uint32_t>	m_invConditionExpressionIds;
				DialogueScript::Id		m_dialogueScript = DialogueScript::ID_NONE;
				std::vector<uint32_t>	m_isClassId;
				std::vector<uint32_t>	m_isNotClassId;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if(aChild->m_name == "verb")
							m_verb.FromSource(aChild);
						else
							m_entries.push_back(std::make_unique<Entry>(aChild));
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteObjectPointers(m_entries);
				m_verb.ToStream(aStream);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadObjectPointers(m_entries))
					return false;
				if(!m_verb.FromStream(aStream))
					return false;
				return true;
			}

			// Public data			
			std::vector<std::unique_ptr<Entry>>		m_entries;
			Verb									m_verb;
		};

	}

}