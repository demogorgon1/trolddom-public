#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct Sprite
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_SPRITE;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			// FIXME: this doesn't need to exist on server

			struct Animation
			{
				void
				FromSource(
					const Parser::Node*	aSource)
				{
					aSource->ForEachChild([&](
						const Parser::Node*	aChild)
					{
						if (aChild->m_name == "states")
						{
							aChild->GetArray()->ForEachChild([&](
								const Parser::Node* aState)
							{
								EntityState::Id entityState = EntityState::StringToId(aState->GetIdentifier());
								TP_VERIFY(entityState != EntityState::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid entity state.", aState->GetIdentifier());
								m_entityStates.push_back(entityState);
							});
						}
						else if (aChild->m_name == "frames")
						{
							aChild->GetIdArray(DataType::ID_SPRITE, m_frameSpriteIds);
						}
						else if(aChild->m_name == "frame_delay")
						{
							m_frameDelay = aChild->GetUInt32();
						}
						else if (aChild->m_name == "random_start_frame")
						{
							m_randomStartFrame = aChild->GetBool();
						}
						else if (aChild->m_name == "z_offset")
						{
							m_zOffset = aChild->GetInt32();
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
						}
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePODs(m_entityStates);
					aWriter->WriteUInts(m_frameSpriteIds);
					aWriter->WriteUInt(m_frameDelay);
					aWriter->WritePOD(m_randomStartFrame);
					aWriter->WriteInt(m_zOffset);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if (!aReader->ReadPODs(m_entityStates))
						return false;
					if(!aReader->ReadUInts(m_frameSpriteIds))
						return false;
					if(!aReader->ReadUInt(m_frameDelay))
						return false;
					if(!aReader->ReadPOD(m_randomStartFrame))
						return false;
					if (!aReader->ReadInt(m_zOffset))
						return false;
					return true;
				}

				// Public data
				std::vector<EntityState::Id>	m_entityStates;
				std::vector<uint32_t>			m_frameSpriteIds;
				uint32_t						m_frameDelay = 0;
				bool							m_randomStartFrame = false;
				int32_t							m_zOffset = 0;
			};

			enum Field
			{
				FIELD_ANIMATIONS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectPointers<Animation>(FIELD_ANIMATIONS, "animations", offsetof(Sprite, m_animations));
				
				aSchema->OnRead<Sprite>([](
					Sprite*						aSprite,
					ComponentSchema::ReadType	/*aReadType*/,
					const Manifest*				/*aManifest*/)
				{
					aSprite->UpdateStateAnimations();
				});
			}

			// ComponentBase implementation
			void
			UpdateStateAnimations()
			{
				memset(m_stateAnimations, 0, sizeof(m_stateAnimations));
				for (const std::unique_ptr<Animation>& animation : m_animations)
				{
					for (EntityState::Id entityState : animation->m_entityStates)
						m_stateAnimations[entityState] = animation.get();
				}
			}

			void
			AddAnimation(
				const std::vector<EntityState::Id>&	aStates,
				const std::vector<uint32_t>&		aFrames,
				uint32_t							aFrameDelay,
				int32_t								aZOffset)
			{
				std::unique_ptr<Animation> t = std::make_unique<Animation>();
				t->m_entityStates = aStates;
				t->m_frameSpriteIds = aFrames;
				t->m_frameDelay = aFrameDelay;
				t->m_zOffset = aZOffset;
				m_animations.push_back(std::move(t));
			}

			// Public data
			std::vector<std::unique_ptr<Animation>>	m_animations;
			const Animation*						m_stateAnimations[EntityState::NUM_IDS] = { 0 };
		};
	}

}