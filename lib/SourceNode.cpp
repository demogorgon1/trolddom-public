#include "Pcheader.h"

#include <tpublic/Hash.h>
#include <tpublic/PersistentIdTable.h>
#include <tpublic/SourceContext.h>
#include <tpublic/SourceNode.h>
#include <tpublic/Tokenizer.h>

namespace tpublic
{

	SourceNode::SourceNode(
		SourceContext*							aSourceContext,
		const DataErrorHandling::DebugInfo&		aDebugInfo,
		const char*								aRealPath,
		const char*								aPath,
		const char*								aPathWithFileName)
		: m_type(TYPE_NONE)
		, m_sourceContext(aSourceContext)
		, m_debugInfo(aDebugInfo)
		, m_path(aPath)
		, m_realPath(aRealPath)
		, m_pathWithFileName(aPathWithFileName)
	{

	}

	SourceNode::SourceNode(
		SourceContext*							aSourceContext,
		const Tokenizer&						aTokenizer)
		: m_type(TYPE_NONE)
		, m_sourceContext(aSourceContext)
		, m_debugInfo(aTokenizer.Next().m_debugInfo)
		, m_path(aTokenizer.GetPath())
		, m_realPath(aTokenizer.GetRealPath())
		, m_pathWithFileName(aTokenizer.GetPathWithFileName())
	{

	}

	uint64_t
	SourceNode::GetSortKey() const
	{
		return (((uint64_t)Hash::String(m_debugInfo.m_file.c_str()) << 32ULL) | (uint64_t)m_debugInfo.m_line) | 0xFFFFULL;
	}

	void
	SourceNode::Copy(
		const SourceNode*						aOther)
	{
		m_children.clear();
		m_annotation.reset();
		m_extraAnnotation.reset();
		m_condition.reset();

		m_type = aOther->m_type;
		m_value = aOther->m_value;

		for(const std::unique_ptr<SourceNode>& otherChild : aOther->m_children)
		{
			std::unique_ptr<SourceNode> child = std::make_unique<SourceNode>(m_sourceContext, m_debugInfo, m_realPath.c_str(), m_path.c_str(), m_pathWithFileName.c_str());

			child->m_name = otherChild->m_name;
			child->m_tag = otherChild->m_tag;

			child->Copy(otherChild.get());

			m_children.push_back(std::move(child));
		}

		if(aOther->m_annotation)
		{
			std::unique_ptr<SourceNode> annotation = std::make_unique<SourceNode>(m_sourceContext, m_debugInfo, m_realPath.c_str(), m_path.c_str(), m_pathWithFileName.c_str());
				
			annotation->m_name = aOther->m_annotation->m_name;
			annotation->m_tag = aOther->m_annotation->m_tag;

			annotation->Copy(aOther->m_annotation.get());

			m_annotation = std::move(annotation);
		}

		if (aOther->m_extraAnnotation)
		{
			std::unique_ptr<SourceNode> annotation = std::make_unique<SourceNode>(m_sourceContext, m_debugInfo, m_realPath.c_str(), m_path.c_str(), m_pathWithFileName.c_str());

			annotation->m_name = aOther->m_extraAnnotation->m_name;
			annotation->m_tag = aOther->m_extraAnnotation->m_tag;

			annotation->Copy(aOther->m_extraAnnotation.get());

			m_extraAnnotation = std::move(annotation);
		}

		if(aOther->m_condition)
		{
			std::unique_ptr<SourceNode> condition = std::make_unique<SourceNode>(m_sourceContext, m_debugInfo, m_realPath.c_str(), m_path.c_str(), m_pathWithFileName.c_str());
				
			condition->m_name = aOther->m_condition->m_name;
			condition->m_tag = aOther->m_condition->m_tag;

			condition->Copy(aOther->m_condition.get());

			m_condition = std::move(condition);
		}
	}

	void
	SourceNode::ForEachChild(
		std::function<void(const SourceNode*)>	aCallback) const
	{
		for(const std::unique_ptr<SourceNode>& child : m_children)
			aCallback(child.get());
	}

	const SourceNode*
	SourceNode::TryGetChildByName(
		const char*								aName) const
	{
		for (const std::unique_ptr<SourceNode>& child : m_children)
		{
			if(child->m_name == aName)
				return child.get();
		}
		return NULL;
	}

	const char*
	SourceNode::GetString() const
	{
		TP_VERIFY(m_type == TYPE_STRING, m_debugInfo, "Not a string.");
		return m_value.c_str();
	}

	const char*
	SourceNode::GetIdentifier() const
	{
		TP_VERIFY(m_type == TYPE_IDENTIFIER, m_debugInfo, "Not an identifier.");
		return m_value.c_str();
	}

	uint32_t
	SourceNode::GetProbability() const 
	{
		float f = GetFloat();
		TP_VERIFY(f >= 0.0f && f <= 100.0f, m_debugInfo, "Probability must be between 0 and 100.");
		return (uint32_t)((f / 100.0f) * (float)UINT32_MAX);
	}

	uint64_t
	SourceNode::GetUInt64() const
	{
		TP_VERIFY(m_type == TYPE_NUMBER, m_debugInfo, "Not a number.");
		uintmax_t v = strtoumax(m_value.c_str(), NULL, 10);
		TP_VERIFY(v <= UINT64_MAX, m_debugInfo, "Number out of bounds.");
		return (uint64_t)v;
	}

	int64_t
	SourceNode::GetInt64() const
	{
		TP_VERIFY(m_type == TYPE_NUMBER, m_debugInfo, "Not a number.");
		intmax_t v = strtoimax(m_value.c_str(), NULL, 10);
		TP_VERIFY(v <= INT64_MAX && v >= INT64_MIN, m_debugInfo, "Number out of bounds.");
		return (int64_t)v;
	}

	uint32_t
	SourceNode::GetUInt32() const
	{
		TP_VERIFY(m_type == TYPE_NUMBER, m_debugInfo, "Not a number.");
		uintmax_t v = strtoumax(m_value.c_str(), NULL, 10);
		TP_VERIFY(v <= UINT32_MAX, m_debugInfo, "Number out of bounds.");
		return (uint32_t)v;
	}

	int32_t
	SourceNode::GetInt32() const
	{
		TP_VERIFY(m_type == TYPE_NUMBER, m_debugInfo, "Not a number.");
		intmax_t v = strtoimax(m_value.c_str(), NULL, 10);
		TP_VERIFY(v <= INT32_MAX && v >= INT32_MIN, m_debugInfo, "Number out of bounds.");
		return (int32_t)v;
	}

	uint8_t
	SourceNode::GetUInt8() const
	{
		uint32_t v = GetUInt32();
		TP_VERIFY(v <= UINT8_MAX, m_debugInfo, "Number out of bounds.");
		return (uint8_t)v;
	}

	Vec2
	SourceNode::GetVec2() const
	{
		TP_VERIFY(m_type == TYPE_ARRAY && m_children.size() == 2, m_debugInfo, "Not an two-component vector.");
		return Vec2{ m_children[0]->GetInt32(), m_children[1]->GetInt32() };
	}

	float
	SourceNode::GetFloat() const
	{
		TP_VERIFY(m_type == TYPE_NUMBER, m_debugInfo, "Not a number.");
		return strtof(m_value.c_str(), NULL);
	}

	char
	SourceNode::GetCharacter() const
	{
		TP_VERIFY(m_type == TYPE_STRING && m_value.length() == 1, m_debugInfo, "Not a character.");
		return m_value[0];
	}

	bool
	SourceNode::GetBool() const
	{
		if (m_value == "true" || m_value == "on" || m_value == "yes" || m_value == "1")
			return true;
		if (m_value == "false" || m_value == "off" || m_value == "no" || m_value == "0")
			return false;
		TP_VERIFY(false, m_debugInfo, "Not a bool.");
		return false;			
	}

	const SourceNode*
	SourceNode::GetArray() const
	{
		TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
		return this;
	}

	const SourceNode*
	SourceNode::GetObject() const
	{
		TP_VERIFY(m_type == TYPE_OBJECT, m_debugInfo, "Not an object.");
		return this;
	}

	const SourceNode*
	SourceNode::GetArrayIndex(
		size_t						aIndex) const
	{
		TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
		TP_VERIFY(aIndex < m_children.size(), m_debugInfo, "Array index out of bounds.");
		return m_children[aIndex].get();
	}

	uint32_t
	SourceNode::GetId(
		DataType::Id				aDataType) const
	{
		return m_sourceContext->m_persistentIdTable->GetId(m_debugInfo, aDataType, GetIdentifier());
	}

	void
	SourceNode::GetIdArray(
		DataType::Id				aDataType,
		std::vector<uint32_t>&		aOut) const
	{
		TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
		for (const std::unique_ptr<SourceNode>& child : m_children)
			aOut.push_back(m_sourceContext->m_persistentIdTable->GetId(child->m_debugInfo, aDataType, child->GetIdentifier()));
	}

	void
	SourceNode::GetUniqueIdArray(
		DataType::Id				aDataType,
		std::vector<uint32_t>&		aOut) const
	{
		TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
		for (const std::unique_ptr<SourceNode>& child : m_children)
		{
			uint32_t id = m_sourceContext->m_persistentIdTable->GetId(child->m_debugInfo, aDataType, child->GetIdentifier());
			if(Helpers::FindItem(aOut, id) == SIZE_MAX)
				aOut.push_back(id);
		}
	}

	void
	SourceNode::GetIdSet(
		DataType::Id					aDataType,
		std::unordered_set<uint32_t>&	aOut) const
	{
		TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
		for (const std::unique_ptr<SourceNode>& child : m_children)
			aOut.insert(m_sourceContext->m_persistentIdTable->GetId(child->m_debugInfo, aDataType, child->GetIdentifier()));
	}

	void
	SourceNode::GetNullOptionalIdArray(
		DataType::Id				aDataType,
		std::vector<uint32_t>&		aOut) const
	{
		TP_VERIFY(m_type == TYPE_ARRAY, m_debugInfo, "Not an array.");
		for (const std::unique_ptr<SourceNode>& child : m_children)
		{
			if(child->m_type == TYPE_NUMBER && child->m_value == "0")
				aOut.push_back(0);
			else
				aOut.push_back(m_sourceContext->m_persistentIdTable->GetId(child->m_debugInfo, aDataType, child->GetIdentifier()));
		}
	}

	bool
	SourceNode::IsIdentifier(
		const char*						aString) const
	{
		return m_type == TYPE_IDENTIFIER && m_value == aString;
	}

	bool
	SourceNode::IsAnonymousObject() const
	{
		return m_type == TYPE_OBJECT && m_name.empty();
	}

	const SourceNode*
	SourceNode::GetAnnotation() const 
	{
		TP_VERIFY(m_annotation, m_debugInfo, "Missing annotation.");
		return m_annotation.get();
	}

	bool
	SourceNode::IsArrayType(
		SourceNode::Type				aType) const
	{
		if(m_type != TYPE_ARRAY)
			return false;
		for(const std::unique_ptr<SourceNode>& child : m_children)
		{
			if(child->m_type != aType)
				return false;
		}
		return true;
	}

}