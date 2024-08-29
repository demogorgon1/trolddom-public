#pragma once

#include "../DataBase.h"
#include "../Image.h"
#include "../TerrainModifier.h"

namespace tpublic
{

	namespace Data
	{

		struct Terrain
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_TERRAIN;
			static const bool TAGGED = false;

			struct TileThreshold
			{
				enum Type : uint8_t
				{
					INVALID_TYPE, 

					TYPE_MORE_THAN,
					TYPE_LESS_THAN
				};

				TileThreshold()
				{

				}

				TileThreshold(
					const SourceNode* aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing terrain threshold annotation.");
					m_terrainModifierId = TerrainModifier::StringToId(aSource->m_annotation->GetIdentifier());
					TP_VERIFY(m_terrainModifierId != TerrainModifier::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid terrain modifier.", aSource->m_annotation->GetIdentifier());
					TP_VERIFY(aSource->GetObject()->m_children.size() == 1, aSource->m_debugInfo, "Not a valid terrain tile threshold.");
					const SourceNode* item = aSource->GetObject()->m_children[0].get();
					
					if (item->m_name == "more_than")
						m_type = TYPE_MORE_THAN;
					else if (item->m_name == "less_than")
						m_type = TYPE_LESS_THAN;
					else
						TP_VERIFY(false, item->m_debugInfo, "'%s' is not a valid item.", item->m_name.c_str());

					m_value = item->GetInt32();
				}

				void
				ToStream(
					IWriter* aWriter) const
				{
					aWriter->WritePOD(m_terrainModifierId);
					aWriter->WritePOD(m_type);
					aWriter->WriteInt(m_value);
				}

				bool
				FromStream(
					IReader* aReader)
				{
					if (!aReader->ReadPOD(m_terrainModifierId))
						return false;
					if (!aReader->ReadPOD(m_type))
						return false;
					if (!aReader->ReadInt(m_value))
						return false;
					return true;
				}

				bool
				Check(
					int32_t	aValue) const
				{
					switch(m_type)
					{
					case TYPE_MORE_THAN:	return aValue > m_value;
					case TYPE_LESS_THAN:	return aValue < m_value;
					default:				return false;
					}
				}

				// Public data
				TerrainModifier::Id				m_terrainModifierId = TerrainModifier::INVALID_ID;
				Type							m_type = INVALID_TYPE;
				int32_t							m_value = 0;				
			};

			struct Tile
			{
				Tile()
				{

				}

				Tile(
					const SourceNode* aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing terrain annotation.");
					m_tileSpriteId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aSource->m_annotation->GetIdentifier());
					
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "threshold")
							m_thresholds.push_back(TileThreshold(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter* aWriter) const
				{
					aWriter->WriteUInt(m_tileSpriteId);
					aWriter->WriteObjects(m_thresholds);
				}

				bool
				FromStream(
					IReader* aReader)
				{
					if (!aReader->ReadUInt(m_tileSpriteId))
						return false;
					if (!aReader->ReadObjects(m_thresholds))
						return false;
					return true;
				}

				// Public data
				uint32_t						m_tileSpriteId = 0;
				std::vector<TileThreshold>		m_thresholds;
			};

			struct Border
			{
				Border()
				{

				}

				Border(
					const SourceNode* aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing border terrain annotation.");
					m_terrainId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TERRAIN, aSource->m_annotation->GetIdentifier());
					aSource->GetIdArray(DataType::ID_TERRAIN, m_neighborTerrain);
				}

				void
				ToStream(
					IWriter* aWriter) const
				{
					aWriter->WriteUInt(m_terrainId);
					aWriter->WriteUInts(m_neighborTerrain);
				}

				bool
				FromStream(
					IReader* aReader)
				{
					if (!aReader->ReadUInt(m_terrainId))
						return false;
					if (!aReader->ReadUInts(m_neighborTerrain))
						return false;
					return true;
				}

				// Public data
				uint32_t						m_terrainId = 0;
				std::vector<uint32_t>			m_neighborTerrain;
			};

			struct GrowsInto
			{
				GrowsInto()
				{

				}

				GrowsInto(
					const SourceNode*	aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing probability annotation.");
					m_probability = aSource->m_annotation->GetUInt32();
					m_terrainId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TERRAIN, aSource->GetIdentifier());
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_probability);
					aWriter->WriteUInt(m_terrainId);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if (!aReader->ReadUInt(m_probability))
						return false;
					if (!aReader->ReadUInt(m_terrainId))
						return false;
					return true;
				}

				// Public data
				uint32_t						m_terrainId = 0;
				uint32_t						m_probability = 0;
			};

			struct Mutation
			{
				Mutation()
				{

				}

				Mutation(
					const SourceNode*	aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing weight annotation.");
					m_weight = aSource->m_annotation->GetUInt32();
					m_terrainId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TERRAIN, aSource->GetIdentifier());
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_weight);
					aWriter->WriteUInt(m_terrainId);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if (!aReader->ReadUInt(m_weight))
						return false;
					if (!aReader->ReadUInt(m_terrainId))
						return false;
					return true;
				}

				// Public data
				uint32_t						m_weight = 0;
				uint32_t						m_terrainId = 0;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			uint32_t
			Mutate(
				std::mt19937&			aRandom) const
			{
				// Should we mutate at all?
				{
					tpublic::UniformDistribution<uint32_t> distribution(1, 100);
					if(distribution(aRandom) > m_mutationChance)
						return m_id;
				}

				// Weighted random mutation
				uint32_t terrainId = 0;

				{
					tpublic::UniformDistribution<uint32_t> distribution(1, m_mutationTotalWeight);
					uint32_t roll = distribution(aRandom);
					uint32_t sum = 0;
					for(const Mutation& mutation : m_mutations)
					{
						sum += mutation.m_weight;
						if(roll <= sum)
						{
							terrainId = mutation.m_terrainId;
							break;
						}
					}
				}

				assert(terrainId != 0);

				return terrainId;
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "mutation")
							m_mutations.push_back(Mutation(aChild));
						else if (aChild->m_name == "grows_into")
							m_growsInto.push_back(GrowsInto(aChild));
						else if (aChild->m_name == "border")
							m_borders.push_back(Border(aChild));
						else if (aChild->m_name == "tile")
							m_tiles.push_back(std::make_unique<Tile>(aChild));
						else if (aChild->m_name == "mutation_chance")
							m_mutationChance = aChild->GetUInt32();
						else if (aChild->m_name == "debug_color")
							m_debugColor = Image::RGBA(aChild);
						else if(aChild->m_name == "walkable")
							m_walkable = aChild->GetBool();
						else if (aChild->m_name == "connect_cost")
							m_connectCost = aChild->GetUInt32();
						else if (aChild->m_name == "connect_conversion")
							m_connectConversion = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TERRAIN, aChild->GetIdentifier());
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});

				for(const Mutation& t : m_mutations)
					m_mutationTotalWeight += t.m_weight;
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteUInt(m_mutationChance);
				aWriter->WriteUInt(m_mutationTotalWeight);
				aWriter->WriteObjects(m_mutations);
				aWriter->WriteObjects(m_growsInto);
				aWriter->WriteObjects(m_borders);
				aWriter->WriteObjectPointers(m_tiles);
				aWriter->WritePOD(m_debugColor);
				aWriter->WritePOD(m_walkable);
				aWriter->WriteUInt(m_connectCost);
				aWriter->WriteUInt(m_connectConversion);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if (!aReader->ReadUInt(m_mutationChance))
					return false;
				if (!aReader->ReadUInt(m_mutationTotalWeight))
					return false;
				if (!aReader->ReadObjects(m_mutations))
					return false;
				if (!aReader->ReadObjects(m_growsInto))
					return false;
				if (!aReader->ReadObjects(m_borders))
					return false;
				if (!aReader->ReadObjectPointers(m_tiles))
					return false;
				if(!aReader->ReadPOD(m_debugColor))
					return false;
				if(!aReader->ReadPOD(m_walkable))
					return false;
				if (!aReader->ReadUInt(m_connectCost))
					return false;
				if (!aReader->ReadUInt(m_connectConversion))
					return false;
				return true;
			}

			// Public data
			uint32_t							m_mutationChance = 0;
			uint32_t							m_mutationTotalWeight = 0;
			std::vector<Mutation>				m_mutations;
			std::vector<GrowsInto>				m_growsInto;
			std::vector<Border>					m_borders;
			std::vector<std::unique_ptr<Tile>>	m_tiles;
			Image::RGBA							m_debugColor;
			bool								m_walkable = false;
			uint32_t							m_connectCost = 1;
			uint32_t							m_connectConversion = 0;
		};

	}

}