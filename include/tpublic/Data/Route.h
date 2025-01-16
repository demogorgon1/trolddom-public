#pragma once

#include "../Chat.h"
#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Route
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ROUTE;
			static const bool TAGGED = false;

			struct Trigger
			{
				Trigger()
				{

				}

				Trigger(
					const SourceNode*	aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing index annotation.");
					if(aSource->m_annotation->IsIdentifier("max"))
						m_index = UINT32_MAX;
					else
						m_index = aSource->m_annotation->GetUInt32();

					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "chat")
							m_chat = Chat(aChild);
						else if(aChild->m_name == "event")
							m_event = aChild->GetBool();
						else if(aChild->m_name == "despawn")
							m_despawn = aChild->GetBool();
						else if (aChild->m_name == "ability")
							m_abilityId = aChild->GetId(DataType::ID_ABILITY);
						else if(aChild->m_name == "reset")
							m_reset = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_index);
					aWriter->WriteOptionalObject(m_chat);
					aWriter->WriteBool(m_event);
					aWriter->WriteBool(m_despawn);
					aWriter->WriteUInt(m_abilityId);
					aWriter->WriteBool(m_reset);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_index))
						return false;
					if(!aReader->ReadOptionalObject(m_chat))
						return false;
					if (!aReader->ReadBool(m_event))
						return false;
					if(!aReader->ReadBool(m_despawn))
						return false;
					if (!aReader->ReadUInt(m_abilityId))
						return false;
					if (!aReader->ReadBool(m_reset))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_index = 0;

				std::optional<Chat>	m_chat;
				bool				m_event = false;
				bool				m_despawn = false;
				bool				m_reset = false;
				uint32_t			m_abilityId = 0;
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
						if(aChild->m_name == "trigger")
							m_triggers.push_back(std::make_unique<Trigger>(aChild));
						else if(aChild->m_name == "max_direction_field_distance")
							m_maxDirectionFieldDistance = aChild->GetUInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteObjectPointers(m_triggers);
				aStream->WriteUInt(m_maxDirectionFieldDistance);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadObjectPointers(m_triggers))
					return false;
				if(!aStream->ReadUInt(m_maxDirectionFieldDistance))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Trigger>>	m_triggers;
			uint32_t								m_maxDirectionFieldDistance = 0;
		};

	}

}