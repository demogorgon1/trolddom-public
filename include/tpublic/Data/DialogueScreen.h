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
			static const bool TAGGED = false;

			struct MapTransfer		
			{
				MapTransfer()
				{

				}

				MapTransfer(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if (aChild->m_name == "cost")
							m_cost = aChild->GetUInt32();
						else if (aChild->m_name == "map")
							m_mapId = aChild->GetId(DataType::ID_MAP);
						else if (aChild->m_name == "map_player_spawn")
							m_mapPlayerSpawnId = aChild->GetId(DataType::ID_MAP_PLAYER_SPAWN);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_cost);
					aWriter->WriteUInt(m_mapId);
					aWriter->WriteUInt(m_mapPlayerSpawnId);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if (!aReader->ReadUInt(m_cost))
						return false;
					if (!aReader->ReadUInt(m_mapId))
						return false;
					if (!aReader->ReadUInt(m_mapPlayerSpawnId))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_cost = 0;
				uint32_t				m_mapId = 0;
				uint32_t				m_mapPlayerSpawnId = 0;
			};

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
							m_dialogueScreenId = aChild->GetId(DataType::ID_DIALOGUE_SCREEN);
						}
						else if (aChild->m_name == "quest")
						{
							if(aChild->m_annotation && aChild->m_annotation->IsIdentifier("completion_only"))
								m_questOffer = false;
							else if (aChild->m_annotation && aChild->m_annotation->IsIdentifier("offer_only"))
								m_questCompletion = false;

							m_questId = aChild->GetId(DataType::ID_QUEST);
						}
						else if (aChild->m_name == "text")
						{
							m_string = aChild->GetString();
						}
						else if (aChild->m_name == "confirmation")
						{
							m_confirmation = aChild->GetString();
						}
						else if(aChild->m_name == "script")
						{
							m_dialogueScript = DialogueScript::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_dialogueScript != DialogueScript::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid dialogue script.", aChild->GetIdentifier());
						}
						else if (aChild->m_name == "condition")
						{
							m_conditionExpressionId = aChild->GetId(DataType::ID_EXPRESSION);
						}
						else if(aChild->m_tag == "requirement")
						{
							m_requirements.push_back(Requirement(aChild));
						}
						else if (aChild->m_name == "map_transfer")
						{
							m_mapTransfer = MapTransfer(aChild);
						}
						else if (aChild->m_name == "no_point_of_interest")
						{
							m_noPointOfInterest = aChild->GetBool();
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
					aWriter->WriteString(m_confirmation);
					aWriter->WriteUInt(m_dialogueScreenId);
					aWriter->WriteUInt(m_questId);
					aWriter->WritePOD(m_dialogueScript);
					aWriter->WriteUInt(m_conditionExpressionId);
					aWriter->WriteBool(m_questOffer);
					aWriter->WriteBool(m_questCompletion);
					aWriter->WriteObjects(m_requirements);
					aWriter->WriteOptionalObject(m_mapTransfer);
					aWriter->WriteBool(m_noPointOfInterest);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadString(m_string))
						return false;
					if (!aReader->ReadString(m_confirmation))
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
					if(!aReader->ReadOptionalObject(m_mapTransfer))
						return false;
					if (!aReader->ReadBool(m_noPointOfInterest))
						return false;
					return true;
				}

				// Public data
				std::string					m_string;
				std::string					m_confirmation;
				uint32_t					m_dialogueScreenId = 0;		
				uint32_t					m_questId = 0;
				bool						m_questOffer = true;
				bool						m_questCompletion = true;
				DialogueScript::Id			m_dialogueScript = DialogueScript::ID_NONE;
				uint32_t					m_conditionExpressionId = 0;
				std::vector<Requirement>	m_requirements;
				std::optional<MapTransfer>	m_mapTransfer;
				bool						m_noPointOfInterest = false;
			};

			struct SellReputationLevelRequirement
			{
				SellReputationLevelRequirement()
				{

				}

				SellReputationLevelRequirement(
					const SourceNode*	aSource)
				{
					m_factionId = aSource->m_sourceContext->m_persistentIdTable->GetId(aSource->m_debugInfo, DataType::ID_FACTION, aSource->m_name.c_str());
					m_level = aSource->GetUInt32();
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_factionId);
					aWriter->WriteUInt(m_level);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_factionId))
						return false;
					if (!aReader->ReadUInt(m_level))
						return false;
					return true;
				}

				// Public data
				uint32_t					m_factionId = 0;
				uint32_t					m_level = 0;
			};

			struct SellItemCost
			{
				SellItemCost()
				{

				}

				SellItemCost(
					const SourceNode*	aSource)
				{
					m_itemId = aSource->GetAnnotation()->GetArrayIndex(0)->GetId(DataType::ID_ITEM);
					m_currencySpriteId = aSource->GetAnnotation()->GetArrayIndex(1)->GetId(DataType::ID_SPRITE);
					m_quantity = aSource->GetUInt32();
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_itemId);
					aWriter->WriteUInt(m_quantity);
					aWriter->WriteUInt(m_currencySpriteId);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_itemId))
						return false;
					if (!aReader->ReadUInt(m_quantity))
						return false;
					if (!aReader->ReadUInt(m_currencySpriteId))
						return false;
					return true;
				}

				// Public data
				uint32_t					m_itemId = 0;
				uint32_t					m_quantity = 0;
				uint32_t					m_currencySpriteId = 0;
			};

			struct Sell
			{
				Sell()
				{

				}

				Sell(
					const SourceNode* aSource)
				{
					m_itemId = aSource->m_sourceContext->m_persistentIdTable->GetId(aSource->m_debugInfo, DataType::ID_ITEM, aSource->m_name.c_str());
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "cost")
							m_cost = aChild->GetUInt32();
						else if(aChild->m_name == "quantity")
							m_quantity = aChild->GetUInt32();
						else if(aChild->m_tag == "reputation_level_requirement")
							m_reputationLevelRequirement = SellReputationLevelRequirement(aChild);
						else if (aChild->m_name == "item_cost")
							m_itemCost = SellItemCost(aChild);
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
					aWriter->WriteOptionalObject(m_reputationLevelRequirement);
					aWriter->WriteOptionalObject(m_itemCost);
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
					if(!aReader->ReadOptionalObject(m_reputationLevelRequirement))
						return false;
					if (!aReader->ReadOptionalObject(m_itemCost))
						return false;
					return true;
				}

				// Public data
				uint32_t										m_itemId = 0;
				uint32_t										m_cost = 0;
				uint32_t										m_quantity = 1;
				std::optional<SellReputationLevelRequirement>	m_reputationLevelRequirement;				
				std::optional<SellItemCost>						m_itemCost;
			};

			struct TrainProfession
			{
				TrainProfession()
				{

				}

				TrainProfession(
					const SourceNode* aSource)
				{
					m_professionId = aSource->GetId(DataType::ID_PROFESSION);
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

			struct Oracle
			{
				Oracle()
				{

				}

				Oracle(
					const SourceNode* aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "item_tag")
							m_itemTag = aChild->GetId(DataType::ID_TAG);
						else if(aChild->m_name == "search")
							m_search = aChild->GetString();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_itemTag);
					aWriter->WriteString(m_search);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if (!aReader->ReadUInt(m_itemTag))
						return false;
					if (!aReader->ReadString(m_search))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_itemTag = 0;
				std::string			m_search;
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
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "text")
							aChild->GetArray()->ForEachChild([&](const SourceNode* aLine) { appendString(m_text, aLine->GetString()); });
						else if (aChild->m_name == "options")
							aChild->GetArray()->ForEachChild([&](const SourceNode* aOption) { m_options.push_back(Option(aOption)); });
						else if (aChild->m_tag == "sell")
							m_sell.push_back(Sell(aChild));
						else if (aChild->m_name == "train_profession")
							m_trainProfessions.push_back(TrainProfession(aChild));
						else if (aChild->m_name == "train_ability")
							m_trainAbilities.push_back(aChild->GetId(DataType::ID_ABILITY));
						else if (aChild->m_name == "train_abilities")
							aChild->GetIdArray(DataType::ID_ABILITY, m_trainAbilities);
						else if(aChild->m_name == "random_item_vendor")
							m_randomItemVendor = aChild->GetBool();
						else if(aChild->m_name == "oracle")
							m_oracle = std::make_unique<Oracle>(aChild);
						else if(aChild->m_name == "nothing_to_train_dialogue_screen")
							m_nothingToTrainDialogueScreenId = aChild->GetId(DataType::ID_DIALOGUE_SCREEN);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteString(m_text);
				aStream->WriteObjects(m_options);
				aStream->WriteObjects(m_sell);
				aStream->WriteObjects(m_trainProfessions);
				aStream->WriteUInts(m_trainAbilities);
				aStream->WriteBool(m_randomItemVendor);
				aStream->WriteOptionalObjectPointer(m_oracle);
				aStream->WriteUInt(m_nothingToTrainDialogueScreenId);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
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
				if(!aStream->ReadBool(m_randomItemVendor))
					return false;
				if (!aStream->ReadOptionalObjectPointer(m_oracle))
					return false;
				if(!aStream->ReadUInt(m_nothingToTrainDialogueScreenId))
					return false;
				return true;
			}

			// Public data
			std::string							m_text;
			std::vector<Option>					m_options;
			std::vector<Sell>					m_sell;
			std::vector<TrainProfession>		m_trainProfessions;
			std::vector<uint32_t>				m_trainAbilities;
			uint32_t							m_nothingToTrainDialogueScreenId = 0;
			bool								m_randomItemVendor = false;
			std::unique_ptr<Oracle>				m_oracle;
		};

	}

}