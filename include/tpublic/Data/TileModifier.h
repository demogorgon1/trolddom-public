#pragma once

#include "../DataBase.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Data
	{

		struct TileModifier
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_TILE_MODIFIER;
			static const bool TAGGED = false;

			struct Requirement
			{
				enum Type
				{
					INVALID_TYPE,

					TYPE_CENTER_MUST_BE,
					TYPE_EAST_MUST_BE,
					TYPE_EAST_MUST_NOT_BE,
					TYPE_WEST_MUST_BE,
					TYPE_WEST_MUST_NOT_BE,
					TYPE_SOUTH_MUST_BE,
					TYPE_SOUTH_MUST_NOT_BE,
					TYPE_NORTH_MUST_BE,
					TYPE_NORTH_MUST_NOT_BE,
					TYPE_NORTH_EAST_MUST_BE,
					TYPE_NORTH_EAST_MUST_NOT_BE,
					TYPE_NORTH_WEST_MUST_BE,
					TYPE_NORTH_WEST_MUST_NOT_BE,
					TYPE_SOUTH_EAST_MUST_BE,
					TYPE_SOUTH_EAST_MUST_NOT_BE,
					TYPE_SOUTH_WEST_MUST_BE,
					TYPE_SOUTH_WEST_MUST_NOT_BE
				};

				static Vec2
				TypeToOffset(
					Type				aType)
				{
					switch(aType)
					{
					case TYPE_CENTER_MUST_BE:			return Vec2{0, 0};
					case TYPE_EAST_MUST_BE:				return Vec2{1, 0};
					case TYPE_EAST_MUST_NOT_BE:			return Vec2{1, 0};
					case TYPE_WEST_MUST_BE:				return Vec2{-1, 0};
					case TYPE_WEST_MUST_NOT_BE:			return Vec2{-1, 0};
					case TYPE_SOUTH_MUST_BE:			return Vec2{0, 1};
					case TYPE_SOUTH_MUST_NOT_BE:		return Vec2{0, 1};
					case TYPE_NORTH_MUST_BE:			return Vec2{0, -1};
					case TYPE_NORTH_MUST_NOT_BE:		return Vec2{0, -1};
					case TYPE_NORTH_EAST_MUST_BE:		return Vec2{1, -1};
					case TYPE_NORTH_EAST_MUST_NOT_BE:	return Vec2{1, -1};
					case TYPE_NORTH_WEST_MUST_BE:		return Vec2{-1, -1};
					case TYPE_NORTH_WEST_MUST_NOT_BE:	return Vec2{-1, -1};
					case TYPE_SOUTH_EAST_MUST_BE:		return Vec2{1, 1};
					case TYPE_SOUTH_EAST_MUST_NOT_BE:	return Vec2{1, 1};
					case TYPE_SOUTH_WEST_MUST_BE:		return Vec2{-1, 1};
					case TYPE_SOUTH_WEST_MUST_NOT_BE:	return Vec2{-1, 1};
					default:							break;
					}

					TP_CHECK(false, "Invalid tile modifier requirement type.");
					return Vec2();
				}

				static bool
				TypeToMustBe(
					Type				aType)
				{
					switch(aType)
					{
					case TYPE_CENTER_MUST_BE:
					case TYPE_EAST_MUST_BE:
					case TYPE_WEST_MUST_BE:
					case TYPE_SOUTH_MUST_BE:
					case TYPE_NORTH_MUST_BE:
					case TYPE_NORTH_EAST_MUST_BE:
					case TYPE_NORTH_WEST_MUST_BE:
					case TYPE_SOUTH_EAST_MUST_BE:
					case TYPE_SOUTH_WEST_MUST_BE:
						return true;

					case TYPE_EAST_MUST_NOT_BE:
					case TYPE_WEST_MUST_NOT_BE:
					case TYPE_SOUTH_MUST_NOT_BE:
					case TYPE_NORTH_MUST_NOT_BE:
					case TYPE_NORTH_EAST_MUST_NOT_BE:
					case TYPE_NORTH_WEST_MUST_NOT_BE:
					case TYPE_SOUTH_EAST_MUST_NOT_BE:
					case TYPE_SOUTH_WEST_MUST_NOT_BE:
						return false;

					default:					
						break;
					}

					TP_CHECK(false, "Invalid tile modifier requirement type.");
					return false;
				}

				static Type
				SourceToType(
					const SourceNode*	aSource)
				{
					std::string_view t(aSource->GetIdentifier());
					if(t == "center_must_be")
						return TYPE_CENTER_MUST_BE;
					else if(t == "east_must_be")
						return TYPE_EAST_MUST_BE;
					else if (t == "east_must_not_be")
						return TYPE_EAST_MUST_NOT_BE;
					else if (t == "west_must_be")
						return TYPE_WEST_MUST_BE;
					else if (t == "west_must_not_be")
						return TYPE_WEST_MUST_NOT_BE;
					else if (t == "south_must_be")
						return TYPE_SOUTH_MUST_BE;
					else if (t == "south_must_not_be")
						return TYPE_SOUTH_MUST_NOT_BE;
					else if (t == "north_must_be")
						return TYPE_NORTH_MUST_BE;
					else if (t == "north_must_not_be")
						return TYPE_NORTH_MUST_NOT_BE;
					else if (t == "north_west_must_be")
						return TYPE_NORTH_WEST_MUST_BE;
					else if (t == "north_west_must_not_be")
						return TYPE_NORTH_WEST_MUST_NOT_BE;
					else if (t == "north_east_must_be")
						return TYPE_NORTH_EAST_MUST_BE;
					else if (t == "north_east_must_not_be")
						return TYPE_NORTH_EAST_MUST_NOT_BE;
					else if (t == "south_west_must_be")
						return TYPE_SOUTH_WEST_MUST_BE;
					else if (t == "south_west_must_not_be")
						return TYPE_SOUTH_WEST_MUST_NOT_BE;
					else if (t == "south_east_must_be")
						return TYPE_SOUTH_EAST_MUST_BE;
					else if (t == "south_east_must_not_be")
						return TYPE_SOUTH_EAST_MUST_NOT_BE;
					TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid type.", aSource->GetIdentifier());
					return INVALID_TYPE;
				}

				Requirement()
				{

				}

				Requirement(
					const SourceNode*	aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing type annotation.");
					m_type = SourceToType(aSource->m_annotation.get());
					aSource->GetIdArray(DataType::ID_SPRITE, m_spriteIds);
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_type);
					aWriter->WriteUInts(m_spriteIds);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadPOD(m_type))
						return false;
					if(!aReader->ReadUInts(m_spriteIds))
						return false;
					return true;
				}

				// Public data
				Type					m_type = INVALID_TYPE;
				std::vector<uint32_t>	m_spriteIds;
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
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (!FromSourceBase(aChild))
					{
						if (aChild->m_name == "requirement")
							m_requirements.push_back(Requirement(aChild));
						else if(aChild->m_name == "sprites")
							aChild->GetIdArray(DataType::ID_SPRITE, m_spriteIds);
						else if(aChild->m_name == "probability")
							m_probability = aChild->GetUInt32();
						else if(aChild->m_name == "priority")
							m_priority = aChild->GetBool();
						else if (aChild->m_name == "check_other_tile_map")
							m_checkOtherTileMap = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInts(m_spriteIds);
				aStream->WriteObjects(m_requirements);
				aStream->WriteUInt(m_probability);
				aStream->WriteBool(m_priority);
				aStream->WriteBool(m_checkOtherTileMap);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadUInts(m_spriteIds))
					return false;
				if (!aStream->ReadObjects(m_requirements))
					return false;
				if(!aStream->ReadUInt(m_probability))
					return false;
				if (!aStream->ReadBool(m_priority))
					return false;
				if (!aStream->ReadBool(m_checkOtherTileMap))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>		m_spriteIds;
			std::vector<Requirement>	m_requirements;			
			uint32_t					m_probability = 100;
			bool						m_priority = false;
			bool						m_checkOtherTileMap = false;
		};

	}

}
