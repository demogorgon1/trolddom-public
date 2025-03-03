#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace tpublic
{

	namespace Expr
	{

		enum Op : uint8_t
		{
			INVALID_OP,

			// Logical
			OP_AND,
			OP_OR,
			OP_NOT,
			OP_EQUAL,
			OP_NOT_EQUAL,
			OP_LESS_THAN_OR_EQUAL,
			OP_GREATER_THAN_OR_EQUAL,
			OP_LESS_THAN,
			OP_GREATER_THAN,

			// Client 
			OP_CLIENT_PLAYER_IS_IN_GUILD,
			OP_CLIENT_WALLET_CASH,
			OP_CLIENT_GUILD_REGISTRATION_PRICE,
			OP_CLIENT_UNTRAIN_TALENTS_PRICE,
			OP_CLIENT_REPUTATION,
			OP_CLIENT_HAS_ITEM,
			OP_CLIENT_HAS_COMPLETED_QUEST,
			OP_CLIENT_IS_DEMO,

			NUM_OPS
		};

		// IMPORTANT: must match Op enum
		static constexpr const char* OPS[] =
		{
			NULL,

			// Logical
			"and",
			"or",
			"not",
			"equal",
			"not_equal",
			"less_than_or_equal",
			"greater_than_or_equal",
			"less_than",
			"greater_than",

			// Client
			"client_player_is_in_guild",
			"client_wallet_cash",
			"client_guild_registration_price",
			"client_untrain_talents_price",
			"client_reputation",
			"client_has_item",
			"client_has_completed_quest",
			"client_is_demo"
		};

		static_assert(sizeof(OPS) / sizeof(const char*) == (size_t)NUM_OPS);

		inline constexpr Op
		StringToOp(
			const char* aString)
		{
			std::string_view s(aString);
			for (uint8_t i = 1; i < (uint8_t)NUM_OPS; i++)
			{
				if (s == OPS[i])
					return (Op)i;
			}

			return INVALID_OP;
		}

		struct Node
		{
			Node()
			{

			}

			Node(
				int64_t				aConstant)
				: m_constant(aConstant)
			{
				
			}

			Node(
				const SourceNode*	aSource,
				Op					aOp)
				: m_op(aOp)
			{
				TP_VERIFY(m_op != INVALID_OP, aSource->m_debugInfo, "'%s' is not a valid operation.", aSource->m_name.c_str());

				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{					
					if(aChild->m_name == "constant")					
					{
						m_children.push_back(std::make_unique<Node>(aChild->GetInt64()));
					}
					else if (aChild->m_name == "id")
					{
						TP_VERIFY(aChild->m_annotation, aChild->m_debugInfo, "Missing data type annotation.");
						DataType::Id dataTypeId = DataType::StringToId(aChild->m_annotation->GetIdentifier());
						TP_VERIFY(dataTypeId != DataType::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid data type.", aChild->m_annotation->GetIdentifier());
						uint32_t id = aChild->GetId(dataTypeId);
						m_children.push_back(std::make_unique<Node>((int64_t)id));
					}
					else
					{
						m_children.push_back(std::make_unique<Node>(aChild, StringToOp(aChild->m_name.c_str())));
					}
				});
			}

			void
			ToStream(
				IWriter*			aWriter) const
			{
				aWriter->WriteBool(m_constant.has_value());
				if(m_constant.has_value())
				{
					aWriter->WriteInt(m_constant.value());
				}
				else
				{
					aWriter->WritePOD(m_op);
					aWriter->WriteObjectPointers(m_children);
				}
			}

			bool
			FromStream(
				IReader*			aReader)
			{
				bool hasConstant;
				if(!aReader->ReadBool(hasConstant))
					return false;

				if(hasConstant)
				{
					int64_t constant;
					if(!aReader->ReadInt(constant))
						return false;
					m_constant = constant;
				}
				else
				{
					if(!aReader->ReadPOD(m_op))
						return false;
					if(!aReader->ReadObjectPointers(m_children))
						return false;
				}
				return true;
			}			

			// Public data
			std::optional<int64_t>					m_constant;
			Op										m_op = INVALID_OP;
			std::vector<std::unique_ptr<Node>>		m_children;
		};

		struct Args
		{
			static const size_t MAX_COUNT = 8;

			// Public data
			size_t		m_count = 0; 
			int64_t		m_values[MAX_COUNT] = { 0 };	
		};

		typedef std::function<int64_t(const Args*)> OpFunction;

		class Context
		{
		public:
			void			InitDefault();
			void			SetParent(
								const Context*	aParent);
			void			SetOpFunction(
								Op				aOp,
								OpFunction		aOpFunction);
			int64_t			ExecuteNode(
									const Node*		aNode) const;
			int64_t			ExecuteOp(
								Op				aOp,
								const Args*		aArgs) const;
		
		private:

			OpFunction								m_opFunctions[NUM_OPS];
			const Context*							m_parent = NULL;
		};
	}

}