#pragma once

#include "../DataBase.h"
#include "../Resource.h"

namespace tpublic
{

	namespace Data
	{

		struct AbilityModifier
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ABILITY_MODIFIER;
			static const bool TAGGED = false;

			struct ModifyResourceCost
			{
				ModifyResourceCost()
				{

				}

				ModifyResourceCost(
					const SourceNode*	aSource)
				{	
					m_resourceId = Resource::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_resourceId != Resource::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid resource.", aSource->m_name.c_str());
					m_value = aSource->GetInt32();
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_resourceId);
					aWriter->WriteInt(m_value);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if (!aReader->ReadPOD(m_resourceId))
						return false;
					if (!aReader->ReadInt(m_value))
						return false;
					return true;
				}

				// Public data
				Resource::Id			m_resourceId = Resource::INVALID_ID;
				int32_t					m_value = 0;
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
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "string")
							m_string = aChild->GetString();
						else if (aChild->m_name == "ability")
							m_abilityId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aChild->GetIdentifier());
						else if (aChild->m_tag == "modify_resource_cost")
							m_modifyResourceCost = ModifyResourceCost(aChild);
						else if (aChild->m_name == "modify_aura_update_count")
							m_modifyAuraUpdateCount = aChild->GetInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteString(m_string);
				aWriter->WriteUInt(m_abilityId);
				aWriter->WriteOptionalObject(m_modifyResourceCost);
				aWriter->WriteInt(m_modifyAuraUpdateCount);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadString(m_string))
					return false;
				if(!aReader->ReadUInt(m_abilityId))
					return false;
				if(!aReader->ReadOptionalObject(m_modifyResourceCost))
					return false;
				if (!aReader->ReadInt(m_modifyAuraUpdateCount))
					return false;
				return true;
			}

			// Public data
			std::string								m_string;
			uint32_t								m_abilityId = 0;
			std::optional<ModifyResourceCost>		m_modifyResourceCost;
			int32_t									m_modifyAuraUpdateCount = 0;
		};

	}

}