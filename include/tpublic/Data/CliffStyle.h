#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct CliffStyle
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_CLIFF_STYLE;
			static const bool TAGGED = true;			

			enum Cell : uint8_t
			{
				CELL_ANY,
				CELL_LOW,
				CELL_HIGH,
				CELL_RAMP
			};

			struct Tile
			{
				Tile()
				{
					memset(m_cells, 0, sizeof(m_cells));
				}

				Tile(
					const SourceNode*	aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing sprite array annotation.");
					aSource->m_annotation->GetIdArray(DataType::ID_SPRITE, m_sprites);

					size_t cellIndex = 0;

					aSource->GetArray()->ForEachChild([&](
						const SourceNode* aChild)
					{
						TP_VERIFY(aChild->m_type == SourceNode::TYPE_STRING, aChild->m_debugInfo, "Not a valid cell array item.");
						for(size_t i = 0, length = aChild->m_value.length(); i < length; i++)
						{
							Cell v = CELL_ANY;
							char c = aChild->m_value[i];
							switch(c)
							{
							case 'x':	v = CELL_ANY; break;
							case '0':	v = CELL_LOW; break;
							case '1':	v = CELL_HIGH; break;
							case 'R':	v = CELL_RAMP; break;
							default:	TP_VERIFY(false, aChild->m_debugInfo, "'%c' is not a valid cell.", c); break;
							}
							TP_VERIFY(cellIndex < 9, aChild->m_debugInfo, "Too many cells.");
							m_cells[cellIndex++] = v;
						}
					});

					TP_VERIFY(cellIndex == 9, aSource->m_debugInfo, "Too few cells.");
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInts(m_sprites);
					aWriter->Write(m_cells, sizeof(m_cells));
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadUInts(m_sprites))
						return false;
					if(aReader->Read(m_cells, sizeof(m_cells)) != sizeof(m_cells))
						return false;
					return true;
				}

				// Public data
				std::vector<uint32_t>		m_sprites;
				Cell						m_cells[9];
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
						if (aChild->m_name == "tile")
							m_tiles.push_back(std::make_unique<Tile>(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteObjectPointers(m_tiles);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadObjectPointers(m_tiles))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Tile>>	m_tiles;
		};

	}

}