#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Faction
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_FACTION;
			static const bool TAGGED = true;

			struct ReputationFromKill
			{
				ReputationFromKill()
				{

				}

				ReputationFromKill(
					const SourceNode*	aSource)
				{
					m_factionId = aSource->m_sourceContext->m_persistentIdTable->GetId(aSource->m_debugInfo, DataType::ID_FACTION, aSource->m_name.c_str());
					m_reputation = aSource->GetInt32();
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_factionId);
					aWriter->WriteInt(m_reputation);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_factionId))
						return false;
					if(!aReader->ReadInt(m_reputation))
						return false;
					return true;
				}

				// Public data
				uint32_t		m_factionId = 0;
				int32_t			m_reputation = 0;
			};

			enum Flag : uint8_t
			{
				FLAG_NEUTRAL = 0x01,
				FLAG_FRIENDLY = 0x02,
				FLAG_REPUTATION = 0x04,
				FLAG_SHOW = 0x08,
				FLAG_PANTHEON = 0x10,
				FLAG_PVP = 0x20
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{	
				aNode->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "flags")
						{
							aChild->GetArray()->ForEachChild([&](
								const SourceNode* aFlag)
							{
								if (aFlag->IsIdentifier("neutral"))
									m_flags |= FLAG_NEUTRAL;
								else if (aFlag->IsIdentifier("friendly"))
									m_flags |= FLAG_FRIENDLY;
								else if (aFlag->IsIdentifier("reputation"))
									m_flags |= FLAG_REPUTATION;
								else if (aFlag->IsIdentifier("show"))
									m_flags |= FLAG_SHOW;
								else if (aFlag->IsIdentifier("pantheon"))
									m_flags |= FLAG_PANTHEON;
								else if (aFlag->IsIdentifier("pvp"))
									m_flags |= FLAG_PVP;
								else
									TP_VERIFY(false, aFlag->m_debugInfo, "'%s' is not a valid faction flag.", aFlag->m_value.c_str());
							});
						}
						else if(aChild->m_name == "string")
						{
							m_string = aChild->GetString();
						}
						else if (aChild->m_name == "default_reputation")
						{
							m_defaultReputation = aChild->GetInt32();
						}
						else if(aChild->m_tag == "reputation_from_kill")
						{
							m_reputationFromKill.push_back(ReputationFromKill(aChild));
						}
						else if (aChild->m_name == "influence_tile_transform")
						{
							uint32_t fromSpriteId = 0;
							uint32_t toSpriteId = 0;

							aChild->GetObject()->ForEachChild([&](
								const SourceNode* aChild2)
							{
								if (aChild2->m_name == "from")
									fromSpriteId = aChild2->GetId(DataType::ID_SPRITE);
								else if (aChild2->m_name == "to")
									toSpriteId = aChild2->GetId(DataType::ID_SPRITE);
								else
									TP_VERIFY(false, aChild2->m_debugInfo, "'%s' is not a valid item.", aChild2->m_name.c_str());
							});		
							
							TP_VERIFY(fromSpriteId != 0 && toSpriteId != 0, aChild->m_debugInfo, "Need to specify both 'from' and 'to' tile sprites.");
							m_influenceTileTransformTable[fromSpriteId] = toSpriteId;
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
				aStream->WritePOD(m_flags);
				aStream->WriteString(m_string);
				aStream->WriteInt(m_defaultReputation);
				aStream->WriteObjects(m_reputationFromKill);
				aStream->WriteUInt(m_influenceTileTransformTable.size());
				for(InfluenceTileTransformTable::const_iterator i = m_influenceTileTransformTable.cbegin(); i != m_influenceTileTransformTable.cend(); i++)
				{
					aStream->WriteUInt(i->first);
					aStream->WriteUInt(i->second);
				}
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadPOD(m_flags))
					return false;
				if(!aStream->ReadString(m_string))
					return false;
				if(!aStream->ReadInt(m_defaultReputation))
					return false;
				if(!aStream->ReadObjects(m_reputationFromKill))
					return false;
				
				{
					size_t count;
					if(!aStream->ReadUInt(count))
						return false;
					for(size_t i = 0; i < count; i++)
					{
						uint32_t fromSpriteId;
						if(!aStream->ReadUInt(fromSpriteId))
							return false;
						uint32_t toSpriteId;
						if (!aStream->ReadUInt(toSpriteId))
							return false;
						m_influenceTileTransformTable[fromSpriteId] = toSpriteId;
					}
				}
				return true;
			}

			uint32_t
			GetInfluenceTileTransform(
				uint32_t				aFromSpriteId) const
			{
				InfluenceTileTransformTable::const_iterator i = m_influenceTileTransformTable.find(aFromSpriteId);
				if(i == m_influenceTileTransformTable.cend())
					return 0;
				return i->second;
			}

			// Helpers
			bool IsNeutralOrFriendly() const { return (m_flags & FLAG_NEUTRAL) != 0 || (m_flags & FLAG_FRIENDLY) != 0; }
			bool IsFriendly() const { return (m_flags & FLAG_FRIENDLY) != 0; }
			bool IsReputation() const { return (m_flags & FLAG_REPUTATION) != 0; }
			bool ShouldShow() const { return (m_flags & FLAG_SHOW) != 0; }
			bool IsPantheon() const { return (m_flags & FLAG_PANTHEON) != 0; }
			bool IsPVP() const { return (m_flags & FLAG_PVP) != 0; }

			// Public data
			uint8_t							m_flags = 0;
			std::string						m_string;
			int32_t							m_defaultReputation = 0;
			std::vector<ReputationFromKill>	m_reputationFromKill;
			
			typedef std::unordered_map<uint32_t, uint32_t> InfluenceTileTransformTable;
			InfluenceTileTransformTable		m_influenceTileTransformTable;
		};

	}

}