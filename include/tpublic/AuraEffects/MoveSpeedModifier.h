#pragma once

#include "../AuraEffectBase.h"
#include "../MoveSpeed.h"

namespace tpublic
{

	namespace AuraEffects
	{

		struct MoveSpeedModifier
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_MOVE_SPEED_MODIFIER;

			MoveSpeedModifier()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~MoveSpeedModifier()
			{

			}

			// AuraEffectBase implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if(aChild->m_name == "move_speed")
						{
							m_moveSpeed = MoveSpeed::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_moveSpeed != MoveSpeed::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid move speed.", aChild->GetIdentifier());
						}
						else 
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WritePOD(m_moveSpeed);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadPOD(m_moveSpeed))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				MoveSpeedModifier* t = new MoveSpeedModifier();
				t->CopyBase(this);
				t->m_moveSpeed = m_moveSpeed;
				return t;
			}

			MoveSpeed::Id	
			GetMoveSpeedModifier() const override
			{ 
				return m_moveSpeed; 
			}

			// Public data
			MoveSpeed::Id			m_moveSpeed = MoveSpeed::INVALID_ID;
		};

	}

}