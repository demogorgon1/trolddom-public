#pragma once

#include "../DataBase.h"
#include "../DialogueScript.h"
#include "../Requirement.h"

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
					const SourceNode*	aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "goto")
						{
							m_dialogueScreenId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_DIALOGUE_SCREEN, aChild->GetIdentifier());
						}
						else if (aChild->m_name == "quest")
						{
							if(aChild->m_annotation && aChild->m_annotation->IsIdentifier("completion_only"))
								m_questOffer = false;
							else if (aChild->m_annotation && aChild->m_annotation->IsIdentifier("offer_only"))
								m_questCompletion = false;

							m_questId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_QUEST, aChild->GetIdentifier());
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
						else if (aChild->m_name == "condition")
						{
							m_conditionExpressionId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_EXPRESSION, aChild->GetIdentifier());
						}
						else if(aChild->m_tag == "requirement")
						{
							m_requirements.push_back(Requirement(aChild));
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
					aWriter->WriteUInt(m_questId);
					aWriter->WritePOD(m_dialogueScript);
					aWriter->WriteUInt(m_conditionExpressionId);
					aWriter->WriteBool(m_questOffer);
					aWriter->WriteBool(m_questCompletion);
					aWriter->WriteObjects(m_requirements);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadString(m_string))
						return false;
					if (!aReader->ReadUInt(m_dialogueScreenId))
						return false;
					if (!aReader->ReadUInt(m_questId))
						return false;
					if (!aReader->ReadPOD(m_dialogueScript))
						return false;
					if (!aReader->ReadUInt(m_conditionExpressionId))
						return false;
					if (!aReader->ReadBool(m_questOffer))
						return false;
					if (!aReader->ReadBool(m_questCompletion))
						return false;
					if(!aReader->ReadObjects(m_requirements))
						return false;
					return true;
				}

				// Public data
				std::string					m_string;
				uint32_t					m_dialogueScreenId = 0;		
				uint32_t					m_questId = 0;
				bool						m_questOffer = true;
				bool						m_questCompletion = true;
				DialogueScript::Id			m_dialogueScript = DialogueScript::ID_NONE;
				uint32_t					m_conditionExpressionId = 0;
				std::vector<Requirement>	m_requirements;
			};

			struct Sell
			{
				Sell()
				{

				}

				Sell(
					const SourceNode* aSource)
				{
					m_itemId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ITEM, aSource->m_name.c_str());
					aSource->ForEachChild([&](
						const SourceNode* aChild)
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

			struct TrainProfession
			{
				TrainProfession()
				{

				}

				TrainProfession(
					const SourceNode* aSource)
				{
					m_professionId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_PROFESSION, aSource->GetIdentifier());
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing profession level annotation.");
					m_professionLevel = aSource->m_annotation->GetUInt32();
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_professionId);
					aWriter->WriteUInt(m_professionLevel);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if (!aReader->ReadUInt(m_professionId))
						return false;
					if (!aReader->ReadUInt(m_professionLevel))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_professionId = 0;
				uint32_t			m_professionLevel = 0;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// DataBase implementation
			void
			FromSource(
				const SourceNode*		aNode) override
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
					const SourceNode* aChild)
				{
					if(aChild->m_name == "text")
						aChild->GetArray()->ForEachChild([&](const SourceNode* aLine) { appendString(m_text, aLine->GetString()); });
					else if(aChild->m_name == "options")
						aChild->GetArray()->ForEachChild([&](const SourceNode* aOption) { m_options.push_back(Option(aOption)); });
					else if(aChild->m_tag == "sell")
						m_sell.push_back(Sell(aChild));
					else if (aChild->m_name == "train_profession")
						m_trainProfessions.push_back(TrainProfession(aChild));
					else if (aChild->m_name == "train_ability")
						m_trainAbilities.push_back(aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aChild->GetIdentifier()));
					else if (aChild->m_name == "train_abilities")
						aChild->GetIdArray(DataType::ID_ABILITY, m_trainAbilities);
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
				aStream->WriteObjects(m_trainProfessions);
				aStream->WriteUInts(m_trainAbilities);
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
				if (!aStream->ReadObjects(m_trainProfessions))
					return false;
				if(!aStream->ReadUInts(m_trainAbilities))
					return false;
				return true;
			}

			// Public data
			std::string							m_text;
			std::vector<Option>					m_options;
			std::vector<Sell>					m_sell;
			std::vector<TrainProfession>		m_trainProfessions;
			std::vector<uint32_t>				m_trainAbilities;
		};

	}

}