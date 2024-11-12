#pragma once

#include "../DataBase.h"
#include "../Particle.h"

namespace tpublic
{

	namespace Data
	{

		struct ParticleSystem
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_PARTICLE_SYSTEM;
			static const bool TAGGED = false;

			struct Swarm
			{
				Swarm()
				{

				}

				Swarm(
					const SourceNode*	aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "radius")
							m_radius = aChild->GetInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteInt(m_radius);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadInt(m_radius))
						return false;
					return true;
				}

				// Public data
				int32_t				m_radius = 1;
			};

			struct Entry
			{
				Entry()
				{

				}

				Entry(
					const SourceNode*	aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "sprites")						
						{
							aChild->GetIdArray(DataType::ID_SPRITE, m_sprites);
						}
						else if(aChild->m_name == "flags")
						{
							m_flags = aChild->GetFlags<uint8_t>([](const char* aString) { return Particle::StringToFlag(aString); });
						}
						else if (aChild->m_name == "count")
						{
							m_count = aChild->GetUInt32();
						}
						else if (aChild->m_name == "sprite_interval")
						{
							m_spriteInterval = aChild->GetUInt32();
						}
						else if (aChild->m_name == "spawn_interval")
						{
							m_spawnInterval = aChild->GetUInt32();
						}
						else if (aChild->m_name == "fade_in")
						{
							m_fadeIn = aChild->GetUInt32();
						}
						else if (aChild->m_name == "fade_out")
						{
							m_fadeOut = aChild->GetUInt32();
						}
						else if (aChild->m_name == "duration")
						{
							m_duration = aChild->GetUInt32();
						}
						else if (aChild->m_name == "scale")
						{
							m_scale = aChild->GetFloat();
						}
						else if (aChild->m_name == "alpha")
						{
							m_alpha = aChild->GetFloat();
						}
						else if (aChild->m_name == "offset_x")
						{
							m_offsetX = aChild->GetFloat();
						}
						else if (aChild->m_name == "offset_y")
						{
							m_offsetY = aChild->GetFloat();
						}
						else if (aChild->m_name == "move_x")
						{
							m_moveX = aChild->GetFloat();
						}
						else if (aChild->m_name == "move_y")
						{
							m_moveY = aChild->GetFloat();
						}
						else if (aChild->m_name == "rotation_rate")
						{
							m_rotationRate = aChild->GetInt32();
						}
						else if (aChild->m_name == "color_mod")
						{
							m_colorModR = aChild->GetArrayIndex(0)->GetUInt8();
							m_colorModG = aChild->GetArrayIndex(1)->GetUInt8();
							m_colorModB = aChild->GetArrayIndex(2)->GetUInt8();
						}
						else if(aChild->m_name == "swarm")
						{
							m_swarm = Swarm(aChild);
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInts(m_sprites);
					aWriter->WritePOD(m_flags);
					aWriter->WritePOD(m_colorModR);
					aWriter->WritePOD(m_colorModG);
					aWriter->WritePOD(m_colorModB);
					aWriter->WriteUInt(m_count);
					aWriter->WriteUInt(m_spriteInterval);
					aWriter->WriteUInt(m_spawnInterval);
					aWriter->WriteUInt(m_fadeIn);
					aWriter->WriteUInt(m_fadeOut);
					aWriter->WriteUInt(m_duration);
					aWriter->WriteFloat(m_scale);
					aWriter->WriteFloat(m_alpha);
					aWriter->WriteInt(m_rotationRate);
					aWriter->WriteFloat(m_offsetX);
					aWriter->WriteFloat(m_offsetY);
					aWriter->WriteFloat(m_moveX);
					aWriter->WriteFloat(m_moveY);
					aWriter->WriteOptionalObject(m_swarm);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInts(m_sprites))
						return false;
					if (!aReader->ReadPOD(m_flags))
						return false;
					if (!aReader->ReadPOD(m_colorModR))
						return false;
					if (!aReader->ReadPOD(m_colorModG))
						return false;
					if (!aReader->ReadPOD(m_colorModB))
						return false;
					if (!aReader->ReadUInt(m_count))
						return false;
					if (!aReader->ReadUInt(m_spriteInterval))
						return false;
					if (!aReader->ReadUInt(m_spawnInterval))
						return false;
					if (!aReader->ReadUInt(m_fadeIn))
						return false;
					if (!aReader->ReadUInt(m_fadeOut))
						return false;
					if (!aReader->ReadUInt(m_duration))
						return false;
					if(!aReader->ReadFloat(m_scale))
						return false;
					if (!aReader->ReadFloat(m_alpha))
						return false;
					if (!aReader->ReadInt(m_rotationRate))
						return false;
					if (!aReader->ReadFloat(m_offsetX))
						return false;
					if (!aReader->ReadFloat(m_offsetY))
						return false;
					if (!aReader->ReadFloat(m_moveX))
						return false;
					if (!aReader->ReadFloat(m_moveY))
						return false;
					if(!aReader->ReadOptionalObject(m_swarm))
						return false;
					return true;
				}

				// Public data
				std::vector<uint32_t>				m_sprites;
				uint8_t								m_flags = 0;
				uint32_t							m_count = 0;
				uint32_t							m_spriteInterval = 100; // ms
				uint32_t							m_spawnInterval = 0; // ms
				uint32_t							m_duration = 0; // ms
				float								m_scale = 1.0f;
				float								m_alpha = 1.0f;
				int32_t								m_rotationRate = 0; // degrees/second
				uint32_t							m_fadeIn = 0; // ms
				uint32_t							m_fadeOut = 0; // ms
				uint8_t								m_colorModR = 255;
				uint8_t								m_colorModG = 255;
				uint8_t								m_colorModB = 255;
				float								m_offsetX = 0.0f;
				float								m_offsetY = 0.0f;
				float								m_moveX = 0.0f;
				float								m_moveY = 0.0f;
				std::optional<Swarm>				m_swarm;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{	
				aNode->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "particle")
							m_particles.push_back(std::make_unique<Entry>(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteObjectPointers(m_particles);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadObjectPointers(m_particles))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Entry>>	m_particles;
		};


	}

}