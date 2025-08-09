#include "Pcheader.h"

#include <tpublic/DataErrorHandling.h>
#include <tpublic/Helpers.h>
#include <tpublic/IReader.h>
#include <tpublic/IWriter.h>
#include <tpublic/RealmModifierList.h>

namespace tpublic
{

	void		
	RealmModifierList::Entry::ToStream(
		IWriter*			aWriter) const
	{
		aWriter->WritePOD(m_id);

		const RealmModifier::Info* info = RealmModifier::GetInfo(m_id);
		assert(info != NULL);		
		switch(info->m_type)
		{
		case RealmModifier::Info::TYPE_FLAG:	
			aWriter->WriteBool(m_bool); 
			break;
		
		case RealmModifier::Info::TYPE_MULTIPLIER:	
			aWriter->WriteFloat(m_float); 
			break;

		default:
			assert(false);
		}
	}
	
	bool		
	RealmModifierList::Entry::FromStream(
		IReader*			aReader)
	{
		if(!aReader->ReadPOD(m_id))
			return false;

		const RealmModifier::Info* info = RealmModifier::GetInfo(m_id);
		if(info == NULL)
			return false;

		switch (info->m_type)
		{
		case RealmModifier::Info::TYPE_FLAG:
			if(!aReader->ReadBool(m_bool))
				return false;
			break;

		case RealmModifier::Info::TYPE_MULTIPLIER:
			if (!aReader->ReadFloat(m_float))
				return false;
			break;

		default:
			return false;
		}

		return true;
	}

	//--------------------------------------------------------------------------------------------------------

	void			
	RealmModifierList::ToStream(
		IWriter*			aWriter) const
	{	
		aWriter->WriteUInt<uint32_t>(VERSION);
		aWriter->WriteObjects(m_entries);
		aWriter->WriteBool(m_defined);
	}
	
	bool			
	RealmModifierList::FromStream(
		IReader*			aReader)
	{
		uint32_t version;
		if(!aReader->ReadUInt(version))
			return false;

		if(!aReader->ReadObjects(m_entries))
			return false;
		if(!aReader->ReadBool(m_defined))
			return false;

		return true;
	}

	void			
	RealmModifierList::SetDefined(
		bool				aDefined)
	{
		m_defined = aDefined;
	}

	RealmModifierList::Entry*
	RealmModifierList::Add(
		const Entry&		aEntry)
	{
		for(Entry& t : m_entries)
		{
			if(t.m_id == aEntry.m_id)
			{
				t = aEntry;
				return &t;
			}
		}
		m_entries.push_back(aEntry);
		return &m_entries[m_entries.size() - 1];
	}
	
	void			
	RealmModifierList::Set(
		RealmModifier::Id	aId,
		const char*			aString)
	{
		Entry t;
		t.m_id = aId;
		_SetString(&t, aString);
		Add(t);
	}

	const RealmModifierList::Entry*
	RealmModifierList::Get(
		RealmModifier::Id	aId) const
	{
		for (const Entry& t : m_entries)
		{
			if (t.m_id == aId)
				return &t;
		}
		return NULL;
	}
	
	RealmModifierList::Entry* 
	RealmModifierList::Get(
		RealmModifier::Id	aId)
	{
		for (Entry& t : m_entries)
		{
			if (t.m_id == aId)
				return &t;
		}
		return NULL;
	}

	RealmModifierList::Entry* 
	RealmModifierList::GetAutoDefault(
		RealmModifier::Id	aId)
	{
		{
			Entry* t = Get(aId);
			if (t != NULL)
				return t;
		}
		
		{
			const RealmModifier::Info* info = RealmModifier::GetInfo(aId);

			Entry t;
			t.m_id = aId;
			_SetString(&t, info->m_default);
			return Add(t);
		}
	}

	bool			
	RealmModifierList::GetFlag(
		RealmModifier::Id	aId,
		bool				aDefault) const
	{
		const RealmModifier::Info* info = RealmModifier::GetInfo(aId);
		TP_CHECK(info->m_type == RealmModifier::Info::TYPE_FLAG, "Not a realm modifier flag: %s", info->m_string);			
		const Entry* t = Get(aId);
		if(t == NULL)
			return aDefault;
		return t->m_bool;
	}
	
	float			
	RealmModifierList::GetMultiplier(
		RealmModifier::Id	aId,
		float				aDefault) const
	{
		const RealmModifier::Info* info = RealmModifier::GetInfo(aId);
		TP_CHECK(info->m_type == RealmModifier::Info::TYPE_MULTIPLIER, "Not a realm modifier multiplier: %s", info->m_string);
		const Entry* t = Get(aId);
		if (t == NULL)
			return aDefault;
		return t->m_float;
	}

	void			
	RealmModifierList::ToggleFlag(
		RealmModifier::Id	aId)
	{
		Entry* t = GetAutoDefault(aId);
		
		t->m_bool = !t->m_bool;
	}

	void			
	RealmModifierList::PruneDefaults()
	{
		for(size_t i = 0; i < m_entries.size(); i++)
		{
			if(_IsDefault(&m_entries[i]))
			{
				Helpers::RemoveCyclicFromVector(m_entries, i);
				i--;
			}
		}
	}

	void			
	RealmModifierList::GetAsStringArray(
		std::vector<std::string>&	aOut) const
	{
		for (const Entry& t : m_entries)
		{
			if (!_IsDefault(&t))
			{
				const RealmModifier::Info* info = RealmModifier::GetInfo(t.m_id);

				std::string s;

				switch (info->m_type)
				{
				case RealmModifier::Info::TYPE_FLAG:
					if(t.m_bool)
						s = info->m_displayName;
					break;

				case RealmModifier::Info::TYPE_MULTIPLIER:
					s = Helpers::Format("%s: %.0f%%", info->m_displayName, t.m_float * 100.0f);
					break;

				default:
					break;
				}

				if(!s.empty())
					aOut.push_back(std::move(s));
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------

	void			
	RealmModifierList::_SetString(
		Entry*				aEntry,
		const char*			aString) const
	{
		const RealmModifier::Info* info = RealmModifier::GetInfo(aEntry->m_id);
		switch(info->m_type)
		{
		case RealmModifier::Info::TYPE_FLAG:	
			{
				std::string_view t(aString);
				aEntry->m_bool = t == "true" || t == "1" || t == "yes" || t == "on";
			}
			break;

		case RealmModifier::Info::TYPE_MULTIPLIER:
			if(aString[0] == '\0')
			{
				aEntry->m_float = 1.0f;
			}
			else
			{
				size_t length = strlen(aString);

				if(aString[length - 1] == '%')
				{
					float percent;
					int result = sscanf(aString, "%f%%", &percent);
					if(result != 1)
						aEntry->m_float = 1.0f;
					else
						aEntry->m_float = percent / 100.0f;
				}
				else
				{
					float value;
					int result = sscanf(aString, "%f", &value);
					if (result != 1)
						aEntry->m_float = 1.0f;
					else
						aEntry->m_float = value;
				}
			}
			break;

		default:
			break;
		}
	}

	bool			
	RealmModifierList::_IsDefault(
		const Entry*		aEntry) const
	{
		Entry t;
		t.m_id = aEntry->m_id;
		_SetString(&t, RealmModifier::GetInfo(aEntry->m_id)->m_default);
		return aEntry->m_bool == t.m_bool 
			&& aEntry->m_float == t.m_float;
	}


}