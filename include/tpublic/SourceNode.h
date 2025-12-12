#pragma once

#include "DataErrorHandling.h"
#include "Vec2.h"

namespace tpublic
{

	class Tokenizer;

	struct SourceContext;

	class SourceNode
	{
	public:
		enum Type
		{
			TYPE_NONE,
			TYPE_OBJECT,
			TYPE_ARRAY,
			TYPE_NUMBER,
			TYPE_STRING,
			TYPE_IDENTIFIER,
			TYPE_MACRO_INVOCATION,
			TYPE_MACRO_OBJECT_IMPORT,
			TYPE_REFERENCE,
			TYPE_REFERENCE_OBJECT,
			TYPE_EMBEDDED_DATA_OBJECT,
			TYPE_CONTEXT_TAG,
			TYPE_EXPRESSION,
			TYPE_EXPRESSION_OR,
			TYPE_EXPRESSION_AND,
			TYPE_EXPRESSION_EQUAL,
			TYPE_EXPRESSION_ADD,
			TYPE_EXPRESSION_SUBTRACT,
			TYPE_EXPRESSION_MULTIPLY,
			TYPE_EXPRESSION_DIVIDE,
			TYPE_EXPRESSION_POWER,
			TYPE_EXPRESSION_EXISTS,
			TYPE_EXPRESSION_NEGATE,
			TYPE_EXPRESSION_NOT,
			TYPE_EXPRESSION_VALUE,
		};

							SourceNode(
								SourceContext*							aSourceContext,
								const DataErrorHandling::DebugInfo&		aDebugInfo,
								const char*								aRealPath,
								const char*								aPath,
								const char*								aPathWithFileName);
							SourceNode(
								SourceContext*							aSourceContext,
								const Tokenizer&						aTokenizer);

		uint64_t			GetSortKey() const;
		void				Copy(
								const SourceNode*						aOther);
		void				ForEachChild(
								std::function<void(const SourceNode*)>	aCallback) const;
		const SourceNode*	TryGetChildByName(
								const char*								aName) const;
		const char*			GetString() const;
		const char*			GetIdentifier() const;
		uint32_t			GetProbability() const;
		uint64_t			GetUInt64() const;
		int64_t				GetInt64() const;
		uint32_t			GetUInt32() const;
		int32_t				GetInt32() const;
		uint8_t				GetUInt8() const;
		Vec2				GetVec2() const;
		float				GetFloat() const;
		char				GetCharacter() const;
		bool				GetBool() const;
		const SourceNode*	GetArray() const;
		const SourceNode*	GetObject() const;
		const SourceNode*	GetArrayIndex(
								size_t									aIndex) const;
		uint32_t			GetId(
								DataType::Id							aDataType) const;
		void				GetIdArray(
								DataType::Id							aDataType,
								std::vector<uint32_t>&					aOut) const;
		void				GetUniqueIdArray(
								DataType::Id							aDataType,
								std::vector<uint32_t>&					aOut) const;
		void				GetIdSet(
								DataType::Id							aDataType,
								std::unordered_set<uint32_t>&			aOut) const;
		void				GetNullOptionalIdArray(
								DataType::Id							aDataType,
								std::vector<uint32_t>&					aOut) const;
		bool				IsIdentifier(
								const char*								aString) const;
		bool				IsAnonymousObject() const;
		const SourceNode*	GetAnnotation() const;
		bool				IsArrayType(
								Type									aType) const;

		template <typename _T>
		_T
		GetFlags(
			std::function<_T(const char*)>								aLookup) const
		{
			TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
			_T flags = 0;
			for (const std::unique_ptr<SourceNode>& child : m_children)
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
			for (const std::unique_ptr<SourceNode>& child : m_children)
			{
				_T id = aLookup(child->GetIdentifier());
				TP_VERIFY(id != _Invalid, m_debugInfo, "'%s' is not a valid identifier.", child->GetIdentifier());
				aOut.push_back(id);
			}
		}

		template <typename _T>
		void
		GetObjectArray(
			std::vector<_T>&				aOut) const
		{
			TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
			for (const std::unique_ptr<SourceNode>& child : m_children)
			{
				_T object(child.get());
				aOut.push_back(object);
			}
		}

		template <typename _T>
		void
		GetUIntArray(
			std::vector<_T>&				aOut) const
		{
			TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
			for (const std::unique_ptr<SourceNode>& child : m_children)
				aOut.push_back((_T)child->GetUInt32());
		}

		template <typename _T>
		void
		GetIntArray(
			std::vector<_T>&				aOut) const
		{
			TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
			for (const std::unique_ptr<SourceNode>& child : m_children)
				aOut.push_back((_T)child->GetInt32());
		}

		template <typename _T>
		void
		GetUIntSet(
			std::vector<_T>&				aOut) const
		{
			TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
			for (const std::unique_ptr<SourceNode>& child : m_children)
				aOut.insert((_T)child->GetUInt32());
		}

		void
		GetStringArray(
			std::vector<std::string>&		aOut) const
		{
			TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
			for (const std::unique_ptr<SourceNode>& child : m_children)
				aOut.push_back(child->GetString());
		}

		template <typename _T>
		void
		GetIntRange(
			_T&								aOutMin,
			_T&								aOutMax) const
		{
			TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
			if(m_children.size() == 1)
			{
				aOutMin = (_T)m_children[0]->GetInt32();
				aOutMax = aOutMin;
			}
			else if(m_children.size() == 2)
			{
				aOutMin = (_T)m_children[0]->GetInt32();
				aOutMax = (_T)m_children[1]->GetInt32();
			}
			else
			{
				TP_VERIFY(false, m_debugInfo, "Not a valid range.");
			}
		}

		template <typename _T>
		void
		GetUIntRange(
			_T&								aOutMin,
			_T&								aOutMax) const
		{
			TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
			if(m_children.size() == 1)
			{
				aOutMin = (_T)m_children[0]->GetUInt32();
				aOutMax = aOutMin;
			}
			else if(m_children.size() == 2)
			{
				aOutMin = (_T)m_children[0]->GetUInt32();
				aOutMax = (_T)m_children[1]->GetUInt32();
			}
			else
			{
				TP_VERIFY(false, m_debugInfo, "Not a valid range.");
			}
		}

		// Public data
		Type										m_type;
		SourceContext*								m_sourceContext;

		std::string									m_name;
		std::string									m_tag;
		std::string									m_value;
		std::string									m_realPath;
		std::string									m_path;
		std::string									m_pathWithFileName;
		DataErrorHandling::DebugInfo				m_debugInfo;

		std::vector<std::unique_ptr<SourceNode>>	m_children;
		std::unique_ptr<SourceNode>					m_annotation;
		std::unique_ptr<SourceNode>					m_extraAnnotation;
		std::unique_ptr<SourceNode>					m_condition;
	};

}