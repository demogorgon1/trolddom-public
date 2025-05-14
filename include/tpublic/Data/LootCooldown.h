#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct LootCooldown
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_LOOT_COOLDOWN;
			static const bool TAGGED = false;

			struct MapMessage
			{
				MapMessage()
				{

				}

				MapMessage(
					const SourceNode*	aSource)
				{
					m_mapId = aSource->m_sourceContext->m_persistentIdTable->GetId(aSource->m_debugInfo, DataType::ID_MAP, aSource->m_name.c_str());
					m_string = aSource->GetString();
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_mapId);
					aWriter->WriteString(m_string);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_mapId))
						return false;
					if(!aReader->ReadString(m_string))
						return false;
					return true;
				}

				// Public data
				uint32_t		m_mapId = 0;
				std::string		m_string;
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
					if (!FromSourceBase(aChild))
					{
						if (aChild->m_name == "seconds")
						{
							if(aChild->IsIdentifier("fixed_daily_timer"))
								m_seconds = 0;
							else
								m_seconds = aChild->GetUInt32();
						}
						else if(aChild->m_tag == "map_message")
						{
							m_mapMessage = MapMessage(aChild);
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_seconds);
				aStream->WriteOptionalObject(m_mapMessage);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInt(m_seconds))
					return false;
				if(!aStream->ReadOptionalObject(m_mapMessage))
					return false;
				return true;
			}

			// Public data
			uint32_t					m_seconds = 0;
			std::optional<MapMessage>	m_mapMessage;
		};

	}

}