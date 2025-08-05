#include "Pcheader.h"

#include <tpublic/DataErrorHandling.h>
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

	void			
	RealmModifierList::Add(
		const Entry&		aEntry)
	{
		for(Entry& t : m_entries)
		{
			if(t.m_id == aEntry.m_id)
			{
				t = aEntry;
				return;
			}
		}
		m_entries.push_back(aEntry);
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
	
	bool			
	RealmModifierList::GetFlag(
		RealmModifier::Id	aId,
		bool				aDefault) const
	{
		const RealmModifier::Info* info = RealmModifier::GetInfo(aId);
		TP_CHECK(info->m_type != RealmModifier::Info::TYPE_FLAG, "Not a realm modifier flag: %s", info->m_string);			
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
		TP_CHECK(info->m_type != RealmModifier::Info::TYPE_MULTIPLIER, "Not a realm modifier multiplier: %s", info->m_string);
		const Entry* t = Get(aId);
		if (t == NULL)
			return aDefault;
		return t->m_float;
	}

}