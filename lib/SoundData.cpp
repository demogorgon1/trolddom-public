#include "Pcheader.h"

#include <tpublic/IReader.h>
#include <tpublic/IWriter.h>
#include <tpublic/SoundData.h>

namespace tpublic
{

	void		
	SoundData::Entry::ToStream(
		IWriter*					aWriter) const
	{
		aWriter->WriteUInt(m_data.size());
		if(m_data.size() > 0)
			aWriter->Write(&m_data[0], m_data.size());
	}
	
	bool		
	SoundData::Entry::FromStream(
		IReader*					aReader)
	{
		size_t size;
		if(!aReader->ReadUInt(size))
			return false;
		if(size > 10 * 1024 * 1024)
			return false;
		m_data.resize(size);
		if(!aReader->Read(&m_data[0], size) != size)
			return false;
		return true;
	}

	//-----------------------------------------------------------------------

	void			
	SoundData::ToStream(
		IWriter*					aWriter) const
	{
		aWriter->WriteUInt(m_table.size());
		for(Table::const_iterator i = m_table.cbegin(); i != m_table.cend(); i++)
		{
			aWriter->WriteUInt(i->first);
			i->second->ToStream(aWriter);
		}
	}
	
	bool			
	SoundData::FromStream(
		IReader*					aReader)
	{
		size_t count;
		if(!aReader->ReadUInt(count))
			return false;
		for(size_t i = 0; i < count; i++)
		{
			uint32_t soundId;
			if(!aReader->ReadUInt(soundId))
				return false;

			std::unique_ptr<Entry> t = std::make_unique<Entry>();
			if(!t->FromStream(aReader))
				return false;

			m_table[soundId] = std::move(t);
		}
		return true;
	}
	
	void			
	SoundData::AddEntry(
		uint32_t					aSoundId,
		std::vector<uint8_t>&		aData)
	{
		TP_CHECK(!m_table.contains(aSoundId), "Sound id collision: %u", aSoundId);
		std::unique_ptr<Entry> t = std::make_unique<Entry>();
		t->m_data = std::move(aData);
		m_table[aSoundId] = std::move(t);
	}
	
	const SoundData::Entry* 
	SoundData::GetEntry(
		uint32_t					aSoundId) const
	{
		Table::const_iterator i = m_table.find(aSoundId);
		if(i == m_table.cend())
			return NULL;
		return i->second.get();
	}

}
