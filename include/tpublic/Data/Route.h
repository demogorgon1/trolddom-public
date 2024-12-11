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
					m_index = aSource->m_annotation->GetUInt32();
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "chat")
							m_chat = Chat(aChild);
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
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_index))
						return false;
					if(!aReader->ReadOptionalObject(m_chat))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_index = 0;

				std::optional<Chat>	m_chat;
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
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadObjectPointers(m_triggers))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Trigger>>	m_triggers;
		};

	}

}