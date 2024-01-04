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
							aChild->GetIdArray(DataType::ID_SPRITE, m_sprites);
						else if(aChild->m_name == "flags")
							m_flags = aChild->GetFlags<uint8_t>([](const char* aString) { return Particle::StringToFlag(aString); });
						else if (aChild->m_name == "count")
							m_count = aChild->GetUInt32();
						else if (aChild->m_name == "sprite_interval")
							m_spriteInterval = aChild->GetUInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInts(m_sprites);
					aWriter->WritePOD(m_flags);
					aWriter->WriteUInt(m_count);
					aWriter->WriteUInt(m_spriteInterval);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInts(m_sprites))
						return false;
					if(!aReader->ReadPOD(m_flags))
						return false;
					if (!aReader->ReadUInt(m_count))
						return false;
					if (!aReader->ReadUInt(m_spriteInterval))
						return false;
					return true;
				}

				// Public data
				std::vector<uint32_t>				m_sprites;
				uint8_t								m_flags = 0;
				uint32_t							m_count = 0;
				uint32_t							m_spriteInterval = 100; // ms
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
				ToStreamBase(aStream);
				aStream->WriteObjectPointers(m_particles);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadObjectPointers(m_particles))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Entry>>	m_particles;
		};


	}

}