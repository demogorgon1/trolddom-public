#pragma once

#include "DataType.h"
#include "DataErrorHandling.h"
#include "PersistentIdTable.h"
#include "SourceContext.h"

namespace tpublic
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
				TP_VERIFY(m_type == TYPE_STRING, m_debugInfo, "Not a string.");
				return m_value.c_str();
			}

			const char*
			GetIdentifier() const
			{
				TP_VERIFY(m_type == TYPE_IDENTIFIER, m_debugInfo, "Not an identifier.");
				return m_value.c_str();
			}

			uint32_t
			GetProbability() const 
			{
				float f = GetFloat();
				TP_VERIFY(f >= 0.0f && f <= 100.0f, m_debugInfo, "Probability must be between 0 and 100.");
				return (uint32_t)((f / 100.0f) * (float)UINT32_MAX);
			}

			uint32_t
			GetUInt32() const
			{
				TP_VERIFY(m_type == TYPE_NUMBER, m_debugInfo, "Not a number.");
				uintmax_t v = strtoumax(m_value.c_str(), NULL, 10);
				TP_VERIFY(v <= UINT32_MAX, m_debugInfo, "Number out of bounds.");
				return (uint32_t)v;
			}

			int32_t
			GetInt32() const
			{
				TP_VERIFY(m_type == TYPE_NUMBER, m_debugInfo, "Not a number.");
				intmax_t v = strtoimax(m_value.c_str(), NULL, 10);
				TP_VERIFY(v <= INT32_MAX && v >= INT32_MIN, m_debugInfo, "Number out of bounds.");
				return (int32_t)v;
			}

			uint8_t
			GetUInt8() const
			{
				uint32_t v = GetUInt32();
				TP_VERIFY(v <= UINT8_MAX, m_debugInfo, "Number out of bounds.");
				return (uint8_t)v;
			}

			float
			GetFloat() const
			{
				TP_VERIFY(m_type == TYPE_NUMBER, m_debugInfo, "Not a number.");
				return strtof(m_value.c_str(), NULL);
			}

			bool
			GetBool() const
			{
				if (m_value == "true" || m_value == "on" || m_value == "yes" || m_value == "1")
					return true;
				if (m_value == "false" || m_value == "off" || m_value == "no" || m_value == "0")
					return false;
				TP_VERIFY(false, m_debugInfo, "Not a bool.");
				return false;			
			}

			const Node*
			GetArray() const
			{
				TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
				return this;
			}

			const Node*
			GetObject() const
			{
				TP_VERIFY(m_type == TYPE_OBJECT, m_debugInfo, "Not an object.");
				return this;
			}

			void
			GetIdArray(
				DataType::Id				aDataType,
				std::vector<uint32_t>&		aOut) const
			{
				TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
				for (const std::unique_ptr<Node>& child : m_children)
					aOut.push_back(m_sourceContext->m_persistentIdTable->GetId(aDataType, child->GetIdentifier()));
			}

			template <typename _T>
			_T
			GetFlags(
				std::function<_T(const char*)> aLookup) const
			{
				TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
				_T flags = 0;
				for (const std::unique_ptr<Node>& child : m_children)
				{
					_T flag = aLookup(child->GetIdentifier());
					TP_VERIFY(flag != _T(0), m_debugInfo, "'%s' is not a valid flag.", child->GetIdentifier());
					flags |= flag;
				}
				return flags;
			}

			template <typename _T, _T _Invalid>
			void
			GetIdArrayWithLookup(
				std::vector<_T>&				aOut,
				std::function<_T(const char*)>	aLookup) const
			{
				TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
				for (const std::unique_ptr<Node>& child : m_children)
				{
					_T id = aLookup(child->GetIdentifier());
					TP_VERIFY(id != _Invalid, m_debugInfo, "'%s' is not a valid identifier.", child->GetIdentifier());
					aOut.push_back(id);
				}
			}

			bool
			IsIdentifier(
				const char*					aString) const
			{
				return m_type == TYPE_IDENTIFIER && m_value == aString;
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