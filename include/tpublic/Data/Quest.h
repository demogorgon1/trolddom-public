#pragma once

#include "../CombatFunction.h"
#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Quest
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_QUEST;
			static const bool TAGGED = true;

			enum Type : uint8_t
			{
				INVALID_TYPE,

				TYPE_NORMAL,
				TYPE_ELITE,
				TYPE_DUNGEON
			};

			enum RepeatMode : uint8_t
			{
				INVALID_REPEAT_MODE,

				REPEAT_MODE_NONE,
				REPEAT_MODE_ALWAYS,
				REPEAT_MODE_DAILY
			};

			static Type
			SourceToType(
				const SourceNode*		aSource)
			{
				std::string_view t(aSource->GetIdentifier());
				if(t == "normal")
					return TYPE_NORMAL;
				else if(t == "elite")
					return TYPE_ELITE;
				else if(t == "dungeon")
					return TYPE_DUNGEON;
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid type.", aSource->GetIdentifier());
				return INVALID_TYPE;
			}

			static RepeatMode
			SourceToRepeatMode(
				const SourceNode*		aSource)
			{
				std::string_view t(aSource->GetIdentifier());
				if(t == "none")
					return REPEAT_MODE_NONE;
				else if(t == "always")
					return REPEAT_MODE_ALWAYS;
				else if(t == "daily")
					return REPEAT_MODE_DAILY;
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid repeat mode.", aSource->GetIdentifier());
				return INVALID_REPEAT_MODE;
			}

			struct RewardRealmBalance
			{
				RewardRealmBalance()
				{

				}

				RewardRealmBalance(
					const SourceNode*	aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing realm balance annotation.");
					m_realmBalanceId = aSource->m_annotation->GetId(DataType::ID_REALM_BALANCE);
					m_function = CombatFunction(aSource);
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_realmBalanceId);
					m_function.ToStream(aWriter);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_realmBalanceId))
						return false;
					if(!m_function.FromStream(aReader))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_realmBalanceId = 0;
				CombatFunction		m_function;
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
				auto appendString = [&](
					std::string& aString,
					const char*  aAppend)
				{
					size_t length = aString.length();
					if(length > 0 && aString[length - 1] != '\n')
						aString.push_back(' ');
					aString.append(aAppend);
				};				

				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "string")
							m_string = aChild->GetString();
						else if (aChild->m_name == "text")
							aChild->GetArray()->ForEachChild([&](const SourceNode* aLine) { appendString(m_text, aLine->GetString()); });
						else if (aChild->m_name == "description")
							aChild->GetArray()->ForEachChild([&](const SourceNode* aLine) { appendString(m_description, aLine->GetString()); });
						else if (aChild->m_name == "progress")
							aChild->GetArray()->ForEachChild([&](const SourceNode* aLine) { appendString(m_progress, aLine->GetString()); });
						else if (aChild->m_name == "completion")
							aChild->GetArray()->ForEachChild([&](const SourceNode* aLine) { appendString(m_completion, aLine->GetString()); });
						else if (aChild->m_name == "level")
							m_level = aChild->GetUInt32();
						else if (aChild->m_name == "cost")
							m_cost = aChild->GetUInt32();
						else if (aChild->m_name == "reward_cash")
							m_rewardCash = aChild->GetUInt32();
						else if (aChild->m_name == "reward_cash_multiplier")
							m_rewardCashMultiplier = aChild->GetFloat();
						else if (aChild->m_name == "reward_xp_multiplier")
							m_rewardXPMultiplier = aChild->GetFloat();
						else if (aChild->m_name == "reward_reputation_multiplier")
							m_rewardReputationMultiplier = aChild->GetFloat();
						else if (aChild->m_name == "type")
							m_type = SourceToType(aChild);
						else if (aChild->m_name == "repeat_mode")
							m_repeatMode = SourceToRepeatMode(aChild);
						else if (aChild->m_name == "objectives")
							aChild->GetIdArray(DataType::ID_OBJECTIVE, m_objectives);
						else if (aChild->m_name == "prerequisites")
							aChild->GetIdArray(DataType::ID_QUEST, m_prerequisites);
						else if (aChild->m_name == "quest_items")
							aChild->GetIdArray(DataType::ID_ITEM, m_questItems);
						else if (aChild->m_name == "reward_one_item")
							aChild->GetIdArray(DataType::ID_ITEM, m_rewardOneItem);
						else if (aChild->m_name == "reward_all_items")
							aChild->GetIdArray(DataType::ID_ITEM, m_rewardAllItems);
						else if (aChild->m_name == "next_quest")
							m_nextQuestId = aChild->GetId(DataType::ID_QUEST);
						else if(aChild->m_name == "reward_realm_balance")
							m_rewardRealmBalance = RewardRealmBalance(aChild);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteString(m_string);
				aStream->WriteString(m_text);
				aStream->WriteString(m_description);
				aStream->WriteString(m_progress);
				aStream->WriteString(m_completion);
				aStream->WriteUInts(m_objectives);
				aStream->WriteUInts(m_prerequisites);
				aStream->WriteUInt(m_level);
				aStream->WritePOD(m_type);
				aStream->WriteUInts(m_questItems);
				aStream->WriteUInts(m_rewardOneItem);
				aStream->WriteUInts(m_rewardAllItems);
				aStream->WriteUInt(m_nextQuestId);
				aStream->WriteFloat(m_rewardCashMultiplier);
				aStream->WriteFloat(m_rewardXPMultiplier);
				aStream->WriteUInt(m_cost);
				aStream->WriteUInt(m_rewardCash);
				aStream->WriteFloat(m_rewardReputationMultiplier);
				aStream->WritePOD(m_repeatMode);
				aStream->WriteOptionalObject(m_rewardRealmBalance);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadString(m_string))
					return false;
				if (!aStream->ReadString(m_text, 16 * 1024))
					return false;
				if (!aStream->ReadString(m_description, 16 * 1024))
					return false;
				if (!aStream->ReadString(m_progress, 16 * 1024))
					return false;
				if (!aStream->ReadString(m_completion, 16 * 1024))
					return false;
				if (!aStream->ReadUInts(m_objectives))
					return false;
				if (!aStream->ReadUInts(m_prerequisites))
					return false;
				if(!aStream->ReadUInt(m_level))
					return false;
				if(!aStream->ReadPOD(m_type))
					return false;
				if (!aStream->ReadUInts(m_questItems))
					return false;
				if (!aStream->ReadUInts(m_rewardOneItem))
					return false;
				if (!aStream->ReadUInts(m_rewardAllItems))
					return false;
				if (!aStream->ReadUInt(m_nextQuestId))
					return false;
				if (!aStream->ReadFloat(m_rewardCashMultiplier))
					return false;
				if (!aStream->ReadFloat(m_rewardXPMultiplier))
					return false;
				if (!aStream->ReadUInt(m_cost))
					return false;
				if (!aStream->ReadUInt(m_rewardCash))
					return false;
				if (!aStream->ReadFloat(m_rewardReputationMultiplier))
					return false;
				if (!aStream->ReadPOD(m_repeatMode))
					return false;
				if(!aStream->ReadOptionalObject(m_rewardRealmBalance))
					return false;
				return true;
			}

			void
			ToPropertyTable(
				PropertyTable&			aOut) const override
			{
				ToPropertyTableBase(aOut);
				aOut["string"] = m_string;
				aOut["level"] = Helpers::Format("%u", m_level).c_str();
			}

			// Public data
			std::string							m_string;
			std::string							m_text;
			std::string							m_description;
			std::string							m_progress;
			std::string							m_completion;
			std::vector<uint32_t>				m_objectives;
			std::vector<uint32_t>				m_prerequisites;			
			uint32_t							m_level = 1;
			Type								m_type = TYPE_NORMAL;
			std::vector<uint32_t>				m_questItems;
			uint32_t							m_rewardCash = 0;
			std::vector<uint32_t>				m_rewardOneItem;
			std::vector<uint32_t>				m_rewardAllItems;
			uint32_t							m_nextQuestId = 0;
			float								m_rewardCashMultiplier = 1.0f;
			float								m_rewardXPMultiplier = 1.0f;
			float								m_rewardReputationMultiplier = 1.0f;
			uint32_t							m_cost = 0;
			RepeatMode							m_repeatMode = REPEAT_MODE_NONE;
			std::optional<RewardRealmBalance>	m_rewardRealmBalance;
		};

	}

}