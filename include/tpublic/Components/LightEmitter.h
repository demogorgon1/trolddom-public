#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		// Light around the entity, centered on its tile plus an offset. Only used by the client for rendering.
		struct LightEmitter
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_LIGHT_EMITTER;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_NONE;

			struct Color
			{
				void
				FromSource(
					const SourceNode*		aSource)
				{
					const SourceNode* p = aSource->GetArray();
					TP_VERIFY(p->m_children.size() == 3, aSource->m_debugInfo, "Light color must be [ r g b ].");
					m_r = p->m_children[0]->GetFloat();
					m_g = p->m_children[1]->GetFloat();
					m_b = p->m_children[2]->GetFloat();
				}

				void
				ToStream(
					IWriter*				aWriter) const
				{
					aWriter->WriteFloat(m_r);
					aWriter->WriteFloat(m_g);
					aWriter->WriteFloat(m_b);
				}

				bool
				FromStream(
					IReader*				aReader)
				{
					if(!aReader->ReadFloat(m_r))
						return false;
					if(!aReader->ReadFloat(m_g))
						return false;
					if(!aReader->ReadFloat(m_b))
						return false;
					return true;
				}

				// Public data
				float						m_r = 1.0f;		// 1 is full brightness
				float						m_g = 1.0f;
				float						m_b = 1.0f;
			};

			// Where the light is relative to the entity's tile, e.g. [ 0 1 ] for a fireplace built into a wall
			struct Offset
			{
				void
				FromSource(
					const SourceNode*		aSource)
				{
					const SourceNode* p = aSource->GetArray();
					TP_VERIFY(p->m_children.size() == 2, aSource->m_debugInfo, "Light offset must be [ x y ].");
					m_x = p->m_children[0]->GetFloat();
					m_y = p->m_children[1]->GetFloat();
				}

				void
				ToStream(
					IWriter*				aWriter) const
				{
					aWriter->WriteFloat(m_x);
					aWriter->WriteFloat(m_y);
				}

				bool
				FromStream(
					IReader*				aReader)
				{
					if(!aReader->ReadFloat(m_x))
						return false;
					if(!aReader->ReadFloat(m_y))
						return false;
					return true;
				}

				// Public data
				float						m_x = 0.0f;		// In tiles
				float						m_y = 0.0f;
			};

			enum Field
			{
				FIELD_COLOR,
				FIELD_RADIUS,
				FIELD_OFFSET,
				FIELD_MERGE_DISTANCE,		// Lights of the same entity type this close to each other (also through others in between) are
											// drawn as one light, e.g. for an AOE that spawns one entity per tile
				FIELD_SHADOWS				// False: lights everything within its radius, through walls and rocks (e.g. a faint light to
											// brighten up a dark cave). It still doesn't light roofs seen from outside if it's under one.
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObject<Color>(FIELD_COLOR, "color", offsetof(LightEmitter, m_color));
				aSchema->Define(ComponentSchema::TYPE_FLOAT, FIELD_RADIUS, "radius", offsetof(LightEmitter, m_radius));
				aSchema->DefineCustomObject<Offset>(FIELD_OFFSET, "offset", offsetof(LightEmitter, m_offset));
				aSchema->Define(ComponentSchema::TYPE_FLOAT, FIELD_MERGE_DISTANCE, "merge_distance", offsetof(LightEmitter, m_mergeDistance));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_SHADOWS, "shadows", offsetof(LightEmitter, m_shadows));
			}

			void
			Reset()
			{
				m_color = Color();
				m_radius = 0.0f;
				m_offset = Offset();
				m_mergeDistance = 0.0f;
				m_shadows = true;
			}

			// Public data
			Color					m_color;
			float					m_radius = 0.0f;			// In tiles
			Offset					m_offset;
			float					m_mergeDistance = 0.0f;		// In tiles, see FIELD_MERGE_DISTANCE
			bool					m_shadows = true;			// See FIELD_SHADOWS
		};
	}

}
