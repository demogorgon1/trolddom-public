#pragma once

#include "../Data/Aura.h"

#include "../AuraEffectBase.h"
#include "../ComponentBase.h"
#include "../Manifest.h"

namespace tpublic
{

	namespace Components
	{

		struct Auras
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_AURAS;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_VOLATILE;

			struct Entry
			{
				void
				ToStream(
					IWriter*					aWriter) const
				{
					aWriter->WriteUInt(m_auraId);
					aWriter->WriteIntDelta(aWriter->GetTick(), m_start);

					aWriter->WriteBool(m_end != 0);
					if(m_end != 0)
						aWriter->WriteIntDelta(aWriter->GetTick(), m_end);
				}

				bool
				FromStream(
					IReader*					aReader) 
				{
					if(!aReader->ReadUInt(m_auraId))
						return false;
					if (!aReader->ReadIntDelta(aReader->GetTick(), m_start))
						return false;
					
					bool hasEnd;
					if(!aReader->ReadBool(hasEnd))
						return false;

					if(hasEnd)
					{
						if (!aReader->ReadIntDelta(aReader->GetTick(), m_end))
							return false;
					}
					else
					{
						m_end = 0;
					}
					return true;
				}

				// Public data
				uint32_t										m_auraId = 0;
				int32_t											m_start = 0;
				int32_t											m_end = 0;

				// Not serialized
				uint32_t										m_entityInstanceId = 0; 
				std::vector<std::unique_ptr<AuraEffectBase>>	m_effects; 
			};

			enum Field
			{
				FIELD_ENTRIES
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectPointersNoSource<Entry>(FIELD_ENTRIES, offsetof(Auras, m_entries));
			}

			Auras()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~Auras()
			{

			}

			bool		HasEffect(
							AuraEffect::Id					aId) const;
			int32_t		FilterDamageInput(
							DirectEffect::DamageType		aDamageType,
							int32_t							aDamage) const;
			void		RemoveAura(
							uint32_t						aAuraId);

			// ComponentBase implementation
			void		ToStream(
							IWriter*						aStream) const override;
			bool		FromStream(
							IReader*						aStream) override;
			void		OnLoadedFromPersistence(
							const Manifest*					aManifest) override;

			// Public data
			std::vector<std::unique_ptr<Entry>>					m_entries;
			bool												m_dirty = false;
		};
	}

}