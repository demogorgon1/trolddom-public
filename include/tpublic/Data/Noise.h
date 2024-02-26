#pragma once

#include "../DataBase.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Data
	{

		struct Noise
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_NOISE;
			static const bool TAGGED = false;

			enum NodeType : uint8_t
			{
				INVALID_NODE_TYPE,

				NODE_TYPE_ADD,
				NODE_TYPE_SUBTRACT,
				NODE_TYPE_MULTIPLY,
				NODE_TYPE_DIVIDE,
				NODE_TYPE_CONSTANT,
				NODE_TYPE_PERLIN
			};

			struct Node
			{
				Node()
				{

				}

				Node(
					const SourceNode*	aSource)
				{
					if(aSource->m_tag == "noise")
					{
						m_type = NODE_TYPE_ADD; // Root
					}
					else
					{
						std::string_view t(aSource->m_name.c_str());
						if (t == "add")
							m_type = NODE_TYPE_ADD;
						else if (t == "subtract")
							m_type = NODE_TYPE_SUBTRACT;
						else if (t == "multiply")
							m_type = NODE_TYPE_MULTIPLY;
						else if (t == "divide")
							m_type = NODE_TYPE_DIVIDE;
						else if (t == "constant")
							m_type = NODE_TYPE_CONSTANT;
						else if (t == "perlin")
							m_type = NODE_TYPE_PERLIN;
					}

					TP_VERIFY(m_type != INVALID_NODE_TYPE, aSource->m_debugInfo, "'%s' is not a valid node type.", aSource->m_name.c_str());

					if(aSource->m_annotation)
					{
						TP_VERIFY(aSource->m_annotation->m_type == SourceNode::TYPE_ARRAY && aSource->m_annotation->m_children.size() == 2, aSource->m_debugInfo, "Not a valid clamp annotation.");
						m_min = aSource->m_annotation->m_children[0]->GetInt32();
						m_max = aSource->m_annotation->m_children[1]->GetInt32();
					}

					switch(m_type)
					{
					case NODE_TYPE_ADD:
					case NODE_TYPE_SUBTRACT:
					case NODE_TYPE_MULTIPLY:
					case NODE_TYPE_DIVIDE:
						aSource->GetObject()->ForEachChild([&](
							const SourceNode* aChild)
						{
							std::unique_ptr<Node> t = std::make_unique<Node>(aChild);
							m_children.push_back(std::move(t));
						});
						break;

					case NODE_TYPE_CONSTANT:
						m_constant = aSource->GetInt32();
						break;

					case NODE_TYPE_PERLIN:
						m_scale = { 1, 1 };
						aSource->GetObject()->ForEachChild([&](
							const SourceNode* aChild)
						{
							if(aChild->m_name == "scale" && aChild->m_type == SourceNode::TYPE_ARRAY && aChild->m_children.size() == 2)
								m_scale = { aChild->GetArrayIndex(0)->GetInt32(), aChild->GetArrayIndex(1)->GetInt32() };
							else if (aChild->m_name == "scale")
								m_scale = { aChild->GetInt32(), aChild->GetInt32() };
							else
								TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());								
						});
						break;
					}
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_type);
					aWriter->WriteInt(m_min);
					aWriter->WriteInt(m_max);

					switch(m_type)
					{
					case NODE_TYPE_ADD:
					case NODE_TYPE_SUBTRACT:
					case NODE_TYPE_MULTIPLY:
					case NODE_TYPE_DIVIDE:
						aWriter->WriteObjectPointers(m_children);
						break;

					case NODE_TYPE_CONSTANT:
						aWriter->WriteInt(m_constant);
						break;

					case NODE_TYPE_PERLIN:
						m_scale.ToStream(aWriter);
						break;

					default:
						assert(false);
						break;
					}
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadPOD(m_type))
						return false;
					if (!aReader->ReadInt(m_min))
						return false;
					if (!aReader->ReadInt(m_max))
						return false;

					switch(m_type)
					{
					case NODE_TYPE_ADD:
					case NODE_TYPE_SUBTRACT:
					case NODE_TYPE_MULTIPLY:
					case NODE_TYPE_DIVIDE:
						if(!aReader->ReadObjectPointers(m_children))
							return false;
						break;

					case NODE_TYPE_CONSTANT:
						if(!aReader->ReadInt(m_constant))
							return false;
						break;

					case NODE_TYPE_PERLIN:
						if(!m_scale.FromStream(aReader))
							return false;
						break;

					default:
						return false;
					}
					return true;
				}

				// Public data
				NodeType							m_type = INVALID_NODE_TYPE;
				std::vector<std::unique_ptr<Node>>	m_children;
				int32_t								m_constant = 0;
				int32_t								m_min = INT32_MIN;
				int32_t								m_max = INT32_MAX;
				Vec2								m_scale;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				m_root = std::make_unique<Node>(aSource);
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteObjectPointer(m_root);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadObjectPointer(m_root))
					return false;
				return true;
			}

			// Public data
			std::unique_ptr<Node>		m_root;
		};

	}

}