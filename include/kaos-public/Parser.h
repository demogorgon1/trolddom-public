#pragma once

#include "DataType.h"
#include "DataErrorHandling.h"
#include "PersistentIdTable.h"
#include "SourceContext.h"

namespace kaos_public
{

	class Tokenizer;

	class Parser
	{
	public:
		struct Node
		{
			enum Type
			{
				TYPE_NONE,
				TYPE_OBJECT,
				TYPE_ARRAY,
				TYPE_NUMBER,
				TYPE_STRING,
				TYPE_IDENTIFIER
			};

			Node(
				SourceContext*						aSourceContext,
				const DataErrorHandling::DebugInfo&	aDebugInfo,
				const char*							aPath)
				: m_type(TYPE_NONE)
				, m_sourceContext(aSourceContext)
				, m_debugInfo(aDebugInfo)
				, m_path(aPath)
			{

			}

			void
			ForEachChild(
				std::function<void(const Node*)>	aCallback) const
			{
				for(const std::unique_ptr<Node>& child : m_children)
					aCallback(child.get());
			}

			const char*
			GetString() const
			{
				KP_VERIFY(m_type == TYPE_STRING, m_debugInfo, "Not a string.");
				return m_value.c_str();
			}

			const char*
			GetIdentifier() const
			{
				KP_VERIFY(m_type == TYPE_IDENTIFIER, m_debugInfo, "Not an identifier.");
				return m_value.c_str();
			}

			uint32_t
			GetUInt32() const
			{
				KP_VERIFY(m_type == TYPE_NUMBER, m_debugInfo, "Not a number.");
				uintmax_t v = strtoumax(m_value.c_str(), NULL, 10);
				KP_VERIFY(v <= UINT32_MAX, m_debugInfo, "Number out of bounds.");
				return (uint32_t)v;
			}

			int32_t
			GetInt32() const
			{
				KP_VERIFY(m_type == TYPE_NUMBER, m_debugInfo, "Not a number.");
				intmax_t v = strtoimax(m_value.c_str(), NULL, 10);
				KP_VERIFY(v <= INT32_MAX && v >= INT32_MIN, m_debugInfo, "Number out of bounds.");
				return (int32_t)v;
			}

			uint8_t
			GetUInt8() const
			{
				uint32_t v = GetUInt32();
				KP_VERIFY(v <= UINT8_MAX, m_debugInfo, "Number out of bounds.");
				return (uint8_t)v;
			}

			const Node*
			GetArray() const
			{
				KP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
				return this;
			}

			const Node*
			GetObject() const
			{
				KP_VERIFY(m_type == TYPE_OBJECT, m_debugInfo, "Not an object.");
				return this;
			}

			void
			GetIdArray(
				DataType::Id				aDataType,
				std::vector<uint32_t>&		aOut) const
			{
				KP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
				for (const std::unique_ptr<Node>& child : m_children)
					aOut.push_back(m_sourceContext->m_persistentIdTable->GetId(aDataType, child->GetIdentifier()));
			}

			// Public data
			Type								m_type;
			SourceContext*						m_sourceContext;

			std::string							m_name;
			std::string							m_tag;
			std::string							m_value;
			std::string							m_path;
			DataErrorHandling::DebugInfo		m_debugInfo;

			std::vector<std::unique_ptr<Node>>	m_children;
		};

					Parser(
						SourceContext*		aSourceContext);
					~Parser();

		void		Parse(
						Tokenizer&			aTokenizer);
		void		DebugPrint() const;

		// Data access
		const Node*	GetRoot() const { return &m_root; }

	private:
		
		Node		m_root;
		
		void	_ParseObject(
					Tokenizer&				aTokenizer,
					Node*					aObject);
		void	_ParseArray(
					Tokenizer&				aTokenizer,
					Node*					aArray);
		void	_ParseValue(
					Tokenizer&				aTokenizer,
					Node*					aParent);
		void	_DebugPrint(
					const Node*				aNode,
					uint32_t				aLevel) const;
	};
	
}