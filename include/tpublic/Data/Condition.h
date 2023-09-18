#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Condition
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_CONDITION;

			struct Node
			{
				enum Type : uint8_t
				{
					INVALID_TYPE,

					// Logic
					TYPE_AND,
					TYPE_OR,
					TYPE_NOT,

					// Player checks
					TYPE_PLAYER_IS_IN_GUILD
				};

				Node()
				{

				}

				Node(
					Type				aDefaultType,
					const Parser::Node*	aSource)
					: m_type(aDefaultType)
				{					
					if (aSource->m_name == "and")
						m_type = TYPE_AND;
					else if (aSource->m_name == "or")
						m_type = TYPE_OR;
					else if (aSource->m_name == "not")
						m_type = TYPE_NOT;
					else if (aSource->m_name == "player_is_in_guild")
						m_type = TYPE_PLAYER_IS_IN_GUILD;
				
					TP_VERIFY(m_type != INVALID_TYPE, aSource->m_debugInfo, "'%s' is not a valid condition node type.", aSource->m_name.c_str());

					aSource->ForEachChild([&](
						const Parser::Node* aChild)
					{	
						std::unique_ptr<Node> t = std::make_unique<Node>(INVALID_TYPE, aChild);
						m_children.push_back(std::move(t));
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_type);
					aWriter->WriteObjectPointers(m_children);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadPOD(m_type))
						return false;
					if(!aReader->ReadObjectPointers(m_children))
						return false;
					return true;
				}

				// Public data
				Type								m_type = INVALID_TYPE;
				std::vector<std::unique_ptr<Node>>	m_children;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		aSource) override
			{	
				m_root = std::make_unique<Node>(Node::TYPE_AND, aSource);
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteOptionalObjectPointer(m_root);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadOptionalObjectPointer(m_root))
					return false;
				return true;
			}

			// Public data
			std::unique_ptr<Node>	m_root;
		};

	}

}