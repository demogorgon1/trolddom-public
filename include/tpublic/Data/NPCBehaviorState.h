#pragma once

#include "../DataBase.h"
#include "../NPCBehavior.h"

namespace tpublic
{

	namespace Data
	{

		struct NPCBehaviorState
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_NPC_BEHAVIOR_STATE;
			static const bool TAGGED = false;

			void
			Verify() const
			{
				VerifyBase();

				TP_VERIFY(m_behavior != NPCBehavior::INVALID_ID, m_debugInfo, "Missing NPC behavior.");
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
						if (aChild->m_name == "behavior")
						{
							m_behavior = NPCBehavior::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_behavior != NPCBehavior::INVALID_ID, aChild->m_debugInfo, "'%s' is not NPC behavior.", aChild->GetIdentifier());
						}
						else if (aChild->m_name == "max_range")
						{
							m_maxRange = aChild->GetUInt32();
						}
						else if (aChild->m_name == "max_ticks")
						{
							m_maxTicks = aChild->GetUInt32();
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
				ToStreamBase(aStream);
				aStream->WritePOD(m_behavior);
				aStream->WriteUInt(m_maxRange);
				aStream->WriteUInt(m_maxTicks);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if (!aStream->ReadPOD(m_behavior))
					return false;
				if (!aStream->ReadUInt(m_maxRange))
					return false;
				if (!aStream->ReadUInt(m_maxTicks))
					return false;
				return true;
			}

			// Public data
			NPCBehavior::Id			m_behavior = NPCBehavior::INVALID_ID;
			uint32_t				m_maxRange = 0;
			uint32_t				m_maxTicks = 0;
		};

	}

}