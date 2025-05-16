#include "Pcheader.h"

#include <tpublic/Base64.h>
#include <tpublic/ComponentSchema.h>
#include <tpublic/EntityState.h>
#include <tpublic/Helpers.h>
#include <tpublic/Vec2.h>
#include <tpublic/VectorIO.h>

namespace tpublic
{

	namespace
	{

		struct DeprecatedFieldList
		{
			struct Entry
			{
				Entry()
				{

				}

				~Entry()
				{
					assert(m_data != NULL);
					assert(m_deleteCallback);
					m_deleteCallback(m_data);
				}

				// Public data
				uint32_t								m_id = 0;
				uint8_t*								m_data = 0;
				ComponentSchema::CustomDeleteCallback	m_deleteCallback;
			};

			Entry*
			GetEntry(
				uint32_t	aId)
			{
				for(std::unique_ptr<Entry>& t : m_entries)
				{
					if(t->m_id == aId)
						return t.get();
				}

				Entry* entry = new Entry();
				entry->m_id = aId;
				m_entries.push_back(std::unique_ptr<Entry>(entry));
				return entry;
			}

			Entry*
			GetExistingEntry(
				uint32_t	aId)
			{
				for(std::unique_ptr<Entry>& t : m_entries)
				{
					if(t->m_id == aId)
						return t.get();
				}

				return NULL;
			}

			// Public data
			std::vector<std::unique_ptr<Entry>>			m_entries;
		};

	}

	//--------------------------------------------------------------------------------------

	ComponentSchema::Field*	
	ComponentSchema::Define(
		Type				aType,
		uint32_t			aId,
		const char*			aName,
		uint32_t			aOffset)
	{
		assert(aId <= MAX_FIELD_ID);

		Field t;
		t.m_type = aType;
		t.m_id = aId;
		t.m_name = aName;
		t.m_offset = aOffset;
		m_fields.push_back(t);
		return &m_fields[m_fields.size() - 1];
	}

	void			
	ComponentSchema::InitUpgradeChains()
	{
		for(Field& field : m_fields)
		{
			field.m_upgradeChain.clear();

			uint32_t initFromDeprecatedId = field.m_initFromDeprecatedId;

			while(initFromDeprecatedId != UINT32_MAX)
			{
				field.m_upgradeChain.push_back(initFromDeprecatedId);
				const Field* nextField = _GetFieldById(initFromDeprecatedId);
				assert(nextField != NULL);
				initFromDeprecatedId = nextField->m_initFromDeprecatedId;
			}
		}
	}

	void			
	ComponentSchema::ReadSource(
		const SourceNode*	aSource,
		void*				aObject) const
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_tag == "modifier")
			{
				std::unordered_map<std::string, SourceModifierCallback>::const_iterator i = m_sourceModifierCallbacks.find(aChild->m_name.c_str());
				TP_VERIFY(i != m_sourceModifierCallbacks.cend(), aChild->m_debugInfo, "'%s' is not a valid component modifier.", aChild->m_name.c_str());
				i->second(aObject, aChild);
			}
			else
			{
				const char* fieldName = aChild->m_name.c_str();

				if(!aChild->m_tag.empty())
					fieldName = aChild->m_tag.c_str();

				const Field* field = _GetFieldByName(fieldName);
				TP_VERIFY(field != NULL, aChild->m_debugInfo, "'%s' is not a valid component field.", fieldName);
				TP_VERIFY(field->m_offset != UINT32_MAX, aChild->m_debugInfo, "'%s' is a deprecated component field.", fieldName);

				if(field->m_customReadSource)
				{
					void* t = (void*)&(((const uint8_t*)aObject)[field->m_offset]);
					field->m_customReadSource(aChild, t);
				}
				else
				{
					switch(field->m_type)
					{
					case TYPE_VEC2:
						{
							TP_VERIFY(aChild->m_children.size() == 2 && aChild->m_type == SourceNode::TYPE_ARRAY, aChild->m_debugInfo, "Syntax error.");
							Vec2* t = (Vec2*)&(((const uint8_t*)aObject)[field->m_offset]);
							t->m_x = aChild->m_children[0]->GetInt32();
							t->m_y = aChild->m_children[1]->GetInt32();
						}
						break;

					case TYPE_STRING:
						{
							std::string* t = (std::string*)&(((const uint8_t*)aObject)[field->m_offset]);
							*t = aChild->GetString();					
						}
						break;

					case TYPE_BOOL:
						{
							bool* t = (bool*)&(((const uint8_t*)aObject)[field->m_offset]);
							*t = aChild->GetBool();					
						}
						break;

					case TYPE_INT32:
						{
							int32_t* t = (int32_t*)&(((const uint8_t*)aObject)[field->m_offset]);
							*t = aChild->GetInt32();					
						}
						break;

					case TYPE_INT64:
						{
							int64_t* t = (int64_t*)&(((const uint8_t*)aObject)[field->m_offset]);
							*t = aChild->GetInt64();					
						}
						break;

					case TYPE_UINT32:
						{
							uint32_t* t = (uint32_t*)&(((const uint8_t*)aObject)[field->m_offset]);
				
							if (field->m_dataType != DataType::INVALID_ID)
								*t = aChild->GetId(field->m_dataType);
							else if (field->m_flags & FLAG_ENTITY_STATE)
								*t = EntityState::StringToId(aChild->GetIdentifier());
							else
								*t = aChild->GetUInt32();
						}
						break;

					case TYPE_UINT64:
						{
							uint64_t* t = (uint64_t*)&(((const uint8_t*)aObject)[field->m_offset]);
							*t = aChild->GetUInt64();					
						}
						break;

					case TYPE_FLOAT:
						{
							float* t = (float*)&(((const uint8_t*)aObject)[field->m_offset]);
							*t = aChild->GetFloat();					
						}
						break;

					case TYPE_UINT32_ARRAY:
						{
							TP_VERIFY(aChild->m_type == SourceNode::TYPE_ARRAY, aChild->m_debugInfo, "Syntax error.");
							std::vector<uint32_t>* t = (std::vector<uint32_t>*)&(((const uint8_t*)aObject)[field->m_offset]);
							aChild->ForEachChild([field, t](
								const SourceNode* aElement)
							{
								uint32_t value;

								if (field->m_dataType != DataType::INVALID_ID)
									value = aElement->GetId(field->m_dataType);
								else if (field->m_flags & FLAG_ENTITY_STATE)
									value = EntityState::StringToId(aElement->GetIdentifier());
								else
									value = aElement->GetUInt32();

								t->push_back(value);
							});
						}
						break;

					default:
						break;
					}
				}
			}
		});

		if (m_onReadCallback)
			m_onReadCallback(aObject, READ_TYPE_SOURCE, NULL);
	}
	
	void			
	ComponentSchema::WriteNetwork(
		IWriter*			aWriter,
		const void*			aObject) const
	{
		for(const Field& field : m_fields)
		{
			bool deprecated = field.m_offset == UINT32_MAX;

			if (!deprecated && (field.m_flags & FLAG_NO_NETWORK) == 0)
				_WriteValue(aWriter, aObject, &field);
		}
	}
	
	bool			
	ComponentSchema::ReadNetwork(
		IReader*			aReader,
		void*				aObject) const
	{
		for(const Field& field : m_fields)
		{
			bool deprecated = field.m_offset == UINT32_MAX;

			if(!deprecated)
			{
				if (!_ReadValue(aReader, aObject, &field))
					return false;
			}
		}

		if (m_onReadCallback)
			m_onReadCallback(aObject, READ_TYPE_NETWORK, NULL);

		return true;
	}

	void			
	ComponentSchema::WriteStorage(
		IWriter*			aWriter,
		const void*			aObject) const
	{		
		for(const Field& field : m_fields)
		{
			bool deprecated = field.m_offset == UINT32_MAX;

			if(!deprecated && (field.m_flags & FLAG_NO_STORAGE) == 0)
			{
				aWriter->WriteUInt(field.m_id + 1); // Ids can't be zero (end-of-stream)

				_WriteValue(aWriter, aObject, &field);
			}
		}

		// Mark end
		aWriter->WriteUInt<uint32_t>(0);
	}
	
	bool			
	ComponentSchema::ReadStorage(
		const Manifest*		aManifest,
		IReader*			aReader,
		void*				aObject) const
	{
		DeprecatedFieldList deprecatedFieldList;
		bool loaded[MAX_FIELD_ID] = { 0 };
		
		// Load non-deprecated fields into the object and deprecated ones into temporary storage
		for(;;)
		{
			uint32_t idPlusOne;
			if(!aReader->ReadUInt(idPlusOne))
				return false;

			if(idPlusOne == 0)
				break;

			uint32_t id = idPlusOne - 1;

			const Field* field = _GetFieldById(id);
			if(field == NULL)
				return false;

			bool deprecated = field->m_offset == UINT32_MAX;

			if(!deprecated)
			{
				if (!_ReadValue(aReader, aObject, field))
					return false;

				assert(id <= MAX_FIELD_ID);
				loaded[id] = true;
			}
			else
			{
				DeprecatedFieldList::Entry* deprecatedFieldEntry = deprecatedFieldList.GetEntry(id);
				assert(deprecatedFieldEntry != NULL);
				
				if(deprecatedFieldEntry->m_data == NULL)
				{
					deprecatedFieldEntry->m_data = new uint8_t[_GetFieldSize(field)];
					
					if(field->m_type == TYPE_CUSTOM)
					{
						assert(field->m_customNewCallback && field->m_customDeleteCallback);
						deprecatedFieldEntry->m_deleteCallback = field->m_customDeleteCallback;
						field->m_customNewCallback(deprecatedFieldEntry->m_data);
					}
				}

				if (!_ReadValue(aReader, deprecatedFieldEntry->m_data, field))
					return false;
			}
		}

		// Check if we got all non-deprecated fields. If not, we'll try to convert from deprecated.
		for(const Field& field : m_fields)
		{
			assert(field.m_id <= MAX_FIELD_ID);
			if(!loaded[field.m_id] && field.m_upgradeChain.size() > 0)
			{
				// We could have a chain of missing deprecated fields we'll need to initialize from each other. This might get a little wonky.
				int32_t firstExistingIndex = -1;

				DeprecatedFieldList::Entry* prevDeprecatedFieldEntry = NULL;

				for(size_t i = 0; i < field.m_upgradeChain.size(); i++)
				{
					uint32_t upgradeChainFieldId = field.m_upgradeChain[i];

					prevDeprecatedFieldEntry = deprecatedFieldList.GetExistingEntry(upgradeChainFieldId);

					if(prevDeprecatedFieldEntry != NULL)
					{
						firstExistingIndex = (int32_t)i;
						break;
					}
				}

				if(firstExistingIndex != -1)
				{
					assert(prevDeprecatedFieldEntry != NULL);

					// Upgrade in reverse order
					for(int32_t i = firstExistingIndex - 1; i >= 0; i--)
					{
						uint32_t upgradeChainFieldId = field.m_upgradeChain[i];

						DeprecatedFieldList::Entry* deprecatedFieldEntry = deprecatedFieldList.GetEntry(upgradeChainFieldId);
						assert(deprecatedFieldEntry != NULL);
						assert(deprecatedFieldEntry->m_data == NULL);

						const Field* deprecatedField = _GetFieldById(upgradeChainFieldId);
						assert(deprecatedField != NULL);

						if (deprecatedField->m_type == TYPE_CUSTOM)
						{
							assert(deprecatedField->m_customNewCallback && deprecatedField->m_customDeleteCallback);
							deprecatedFieldEntry->m_deleteCallback = deprecatedField->m_customDeleteCallback;
							deprecatedField->m_customNewCallback(deprecatedFieldEntry->m_data);
						}

						assert(deprecatedField->m_initFromDeprecatedCallback);
						deprecatedField->m_initFromDeprecatedCallback(deprecatedFieldEntry->m_data, prevDeprecatedFieldEntry->m_data);

						prevDeprecatedFieldEntry = deprecatedFieldEntry;
					}

					// Now we can init the actual field
					assert(prevDeprecatedFieldEntry != NULL);
					assert(field.m_initFromDeprecatedCallback);					
					field.m_initFromDeprecatedCallback(&(((uint8_t*)aObject)[field.m_offset]), prevDeprecatedFieldEntry->m_data); 
				}
			}
		}

		if(m_onReadCallback)
			m_onReadCallback(aObject, READ_TYPE_STORAGE, aManifest);

		return true;
	}

	void			
	ComponentSchema::ExportText(
		const void*			aObject,
		ExportTextCallback	aExportTextCallback) const
	{
		for (const Field& field : m_fields)
		{
			bool deprecated = field.m_offset == UINT32_MAX;
			if (!deprecated && (field.m_flags & FLAG_NO_STORAGE) == 0)
			{
				std::string value = _ValueAsString(aObject, &field);

				char buffer[1024];
				if(field.m_name != NULL)						
					TP_STRING_FORMAT(buffer, sizeof(buffer), "%s:%s", field.m_name, value.c_str());
				else
					TP_STRING_FORMAT(buffer, sizeof(buffer), "%u:%s", field.m_id, value.c_str());

				aExportTextCallback(buffer);
			}
		}
	}

	void			
	ComponentSchema::ImportText(
		const char*			aText,
		void*				aObject) const
	{
		enum ParseState
		{
			PARSE_STATE_INIT,
			PARSE_STATE_FIELD_ID,
			PARSE_STATE_FIELD_NAME,
		};

		ParseState parseState = PARSE_STATE_INIT;
		const char* p = aText;

		std::vector<char> buffer;
		const Field* field = NULL;

		while(*p != '\0')
		{
			char c = *p;

			switch(parseState)
			{
			case PARSE_STATE_INIT:
				buffer.push_back(c);
				if (c >= '0' && c <= '9')
					parseState = PARSE_STATE_FIELD_ID;
				else 
					parseState = PARSE_STATE_FIELD_NAME;
				break;

			case PARSE_STATE_FIELD_ID:
			case PARSE_STATE_FIELD_NAME:
				if(c == ':')
				{
					buffer.push_back('\0');
					if(parseState == PARSE_STATE_FIELD_ID)
						field = _GetFieldById(strtoul(&buffer[0], NULL, 10));
					else 
						field = _GetFieldByName(&buffer[0]);
					p++;
					break;
				}
				else 
				{
					buffer.push_back(c);
				}
				break;
			}
			p++;
		}

		TP_CHECK(*p != '\0', "Missing component field value.");
		TP_CHECK(field != NULL, "Missing component field.");

		switch(field->m_type)
		{
		case TYPE_VEC2:
			{				
				Vec2* t = (Vec2*)&(((const uint8_t*)aObject)[field->m_offset]);
				int32_t result = sscanf(p, "%d,%d", &t->m_x, &t->m_y);
				TP_CHECK(result == 2, "Invalid value.");
			}
			break;

		case TYPE_STRING:
			{
				std::string* t = (std::string*)&(((const uint8_t*)aObject)[field->m_offset]);
				*t = p;
			}
			break;

		case TYPE_BOOL:
			{
				bool* t = (bool*)&(((const uint8_t*)aObject)[field->m_offset]);
				*t = strcmp(p, "true") == 0;
			}
			break;

		case TYPE_INT32:
			{
				int32_t* t = (int32_t*)&(((const uint8_t*)aObject)[field->m_offset]);
				int32_t result = sscanf(p, "%d", t);
				TP_CHECK(result == 1, "Invalid value.");
			}
			break;

		case TYPE_INT64:
			{
				int64_t* t = (int64_t*)&(((const uint8_t*)aObject)[field->m_offset]);
				int32_t result = sscanf(p, "%zd", t);
				TP_CHECK(result == 1, "Invalid value.");
			}
			break;

		case TYPE_UINT32:
			{
				uint32_t* t = (uint32_t*)&(((const uint8_t*)aObject)[field->m_offset]);
				int32_t result = sscanf(p, "%u", t);
				TP_CHECK(result == 1, "Invalid value.");
			}
			break;

		case TYPE_UINT64:
			{
				uint64_t* t = (uint64_t*)&(((const uint8_t*)aObject)[field->m_offset]);
				int32_t result = sscanf(p, "%zu", t);
				TP_CHECK(result == 1, "Invalid value.");
		}
			break;

		case TYPE_FLOAT:
			{
				float* t = (float*)&(((const uint8_t*)aObject)[field->m_offset]);
				int32_t result = sscanf(p, "%f", t);
				TP_CHECK(result == 1, "Invalid value.");
			}
			break;

		case TYPE_UINT32_ARRAY:
			{
				std::vector<uint32_t>* t = (std::vector<uint32_t>*)&(((const uint8_t*)aObject)[field->m_offset]);
				t->clear();
				std::stringstream tokenizer(p);
				std::string token;
				while (std::getline(tokenizer, token, ','))
				{
					Helpers::TrimString(token);
					uint32_t v;
					int32_t result = sscanf(p, "%u", &v);
					TP_CHECK(result == 1, "Invalid value.");
					t->push_back(v);
				}
			}
			break;

		default:
			break;
		}
	}

	std::string
	ComponentSchema::AsDebugString(
		const void*			aObject) const
	{
		std::string t;

		for(const Field& field : m_fields)
		{
			char buffer[1024];
			TP_STRING_FORMAT(buffer, sizeof(buffer), "%u(%s)=%s ", field.m_id, field.m_name != NULL ? field.m_name : "", _ValueAsString(aObject, &field).c_str());
			t.append(buffer);
		}

		return t;
	}

	void			
	ComponentSchema::Validate() const
	{
		// Make sure no fields overlap
		for(size_t i = 0; i < m_fields.size(); i++)
		{
			const Field& field1 = m_fields[i];

			for(size_t j = i + 1; j < m_fields.size(); j++)
			{
				const Field& field2 = m_fields[j];

				size_t fieldSize1 = _GetFieldSize(&field1);
				size_t fieldSize2 = _GetFieldSize(&field2);

				size_t rangeMin = field1.m_offset;
				size_t rangeMax = field1.m_offset + fieldSize1;
				if(field2.m_offset < rangeMin)
					rangeMin = field2.m_offset;
				if(field2.m_offset + fieldSize2 > rangeMax)
					rangeMax = field2.m_offset + fieldSize2;

				assert(rangeMax - rangeMin >= fieldSize1 + fieldSize2);
			}
		}
	}

	//--------------------------------------------------------------------------------------

	uint32_t		
	ComponentSchema::_GetFieldSize(
		const Field*		aField) const
	{
		switch(aField->m_type)
		{
		case TYPE_NONE:				return 0;
		case TYPE_VEC2:				return sizeof(Vec2);
		case TYPE_STRING:			return sizeof(std::string);
		case TYPE_BOOL:				return sizeof(bool);
		case TYPE_INT32:			return sizeof(int32_t);
		case TYPE_INT64:			return sizeof(int64_t);
		case TYPE_UINT32:			return sizeof(uint32_t);
		case TYPE_UINT64:			return sizeof(uint64_t);
		case TYPE_FLOAT:			return sizeof(float);
		case TYPE_UINT32_ARRAY:		return sizeof(std::vector<uint32_t>);
		case TYPE_CUSTOM:			return aField->m_customSize;
		default:					assert(false);
		}
		return 0;
	}
	
	const ComponentSchema::Field*
	ComponentSchema::_GetFieldByName(
		const char*			aName) const
	{
		for(const Field& field : m_fields)
		{
			if(field.m_name != NULL && strcmp(field.m_name, aName) == 0)
				return &field;
		}
		return NULL;
	}

	const ComponentSchema::Field*
	ComponentSchema::_GetFieldById(
		uint32_t			aId) const
	{
		for(const Field& field : m_fields)
		{
			if(field.m_id == aId)
				return &field;
		}
		return NULL;
	}

	ComponentSchema::Field*
	ComponentSchema::_GetFieldById(
		uint32_t			aId) 
	{
		for (Field& field : m_fields)
		{
			if (field.m_id == aId)
				return &field;
		}
		return NULL;
	}

	void			
	ComponentSchema::_WriteValue(
		IWriter*			aWriter,
		const void*			aObject,
		const Field*		aField) const
	{
		switch(aField->m_type)
		{
		case TYPE_VEC2:
			{				
				const Vec2* t = (const Vec2*)&(((const uint8_t*)aObject)[aField->m_offset]);
				t->ToStream(aWriter);
			}
			break;

		case TYPE_STRING:
			{				
				const std::string* t = (const std::string*)&(((const uint8_t*)aObject)[aField->m_offset]);
				aWriter->WriteString(*t);
			}
			break;

		case TYPE_BOOL:
			{
				const bool* t = (const bool*)&(((const uint8_t*)aObject)[aField->m_offset]);
				aWriter->WriteBool(*t);
			}
			break;

		case TYPE_INT32:
			{
				const int32_t* t = (const int32_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				aWriter->WriteInt(*t);
			}
			break;

		case TYPE_INT64:
			{
				const int64_t* t = (const int64_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				aWriter->WriteInt(*t);
			}
			break;

		case TYPE_UINT32:
			{
				const uint32_t* t = (const uint32_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				aWriter->WriteUInt(*t);
			}
			break;

		case TYPE_UINT64:
			{
				const uint64_t* t = (const uint64_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				aWriter->WriteUInt(*t);
			}
			break;

		case TYPE_FLOAT:
			{
				const float* t = (const float*)&(((const uint8_t*)aObject)[aField->m_offset]);
				aWriter->WriteFloat(*t);
			}
			break;

		case TYPE_UINT32_ARRAY:
			{
				const std::vector<uint32_t>* t = (const std::vector<uint32_t>*)&(((const uint8_t*)aObject)[aField->m_offset]);
				aWriter->WriteUInts(*t);
			}
			break;

		case TYPE_CUSTOM:
			{
				const void* t = (const void*)&(((const uint8_t*)aObject)[aField->m_offset]);
				assert(aField->m_customWrite);
				aField->m_customWrite(aWriter, t);
			}
			break;

		default:
			break;
		}
	}

	bool
	ComponentSchema::_ReadValue(
		IReader*			aReader,
		void*				aObject,
		const Field*		aField) const
	{
		uint32_t offset = aField->m_offset;

		if(offset == UINT32_MAX)
		{
			// This is a deprecated field and we're not loading it into the actual object
			offset = 0;
		}

		switch(aField->m_type)
		{
		case TYPE_VEC2:
			{				
				Vec2* t = (Vec2*)&(((const uint8_t*)aObject)[offset]);
				if(!t->FromStream(aReader))
					return false;
			}
			break;

		case TYPE_STRING:
			{				
				std::string* t = (std::string*)&(((const uint8_t*)aObject)[offset]);
				if(!aReader->ReadString(*t))
					return false;
			}
			break;

		case TYPE_BOOL:
			{
				bool* t = (bool*)&(((const uint8_t*)aObject)[offset]);
				if(!aReader->ReadBool(*t))
					return false;
			}
			break;

		case TYPE_INT32:
			{
				int32_t* t = (int32_t*)&(((const uint8_t*)aObject)[offset]);
				if(!aReader->ReadInt(*t))
					return false;
			}
			break;

		case TYPE_INT64:
			{
				int64_t* t = (int64_t*)&(((const uint8_t*)aObject)[offset]);
				if(!aReader->ReadInt(*t))
					return false;
			}
			break;

		case TYPE_UINT32:
			{
				uint32_t* t = (uint32_t*)&(((const uint8_t*)aObject)[offset]);
				if(!aReader->ReadUInt(*t))
					return false;
			}
			break;

		case TYPE_UINT64:
			{
				uint64_t* t = (uint64_t*)&(((const uint8_t*)aObject)[offset]);
				if(!aReader->ReadUInt(*t))
					return false;
			}
			break;

		case TYPE_FLOAT:
			{
				float* t = (float*)&(((const uint8_t*)aObject)[offset]);
				if(!aReader->ReadFloat(*t))
					return false;
			}
			break;

		case TYPE_UINT32_ARRAY:
			{
				std::vector<uint32_t>* t = (std::vector<uint32_t>*)&(((const uint8_t*)aObject)[offset]);
				if(!aReader->ReadUInts(*t))
					return false;
			}
			break;

		case TYPE_CUSTOM:
			{
				void* t = (void*)&(((const uint8_t*)aObject)[offset]);
				assert(aField->m_customRead);
				if(!aField->m_customRead(aReader, t))
					return false;
			}
			break;

		default:
			break;
		}

		return true;
	}

	std::string		
	ComponentSchema::_ValueAsString(
		const void*		aObject,
		const Field*	aField) const
	{
		char buffer[1024];

		switch(aField->m_type)
		{
		case TYPE_VEC2:			
			{
				const Vec2* t = (const Vec2*)&(((const uint8_t*)aObject)[aField->m_offset]);
				TP_STRING_FORMAT(buffer, sizeof(buffer), "%d,%d", t->m_x, t->m_y);
			}
			break;

		case TYPE_STRING:
			{
				const std::string* t = (const std::string*)&(((const uint8_t*)aObject)[aField->m_offset]);
				TP_STRING_FORMAT(buffer, sizeof(buffer), "%s", t->c_str());
			}
			break;

		case TYPE_BOOL:
			{
				const bool* t = (const bool*)&(((const uint8_t*)aObject)[aField->m_offset]);
				TP_STRING_FORMAT(buffer, sizeof(buffer), "%s", *t ? "true" : "false");
			}
			break;

		case TYPE_INT32:
			{
				const int32_t* t = (const int32_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				TP_STRING_FORMAT(buffer, sizeof(buffer), "%d", *t);
			}
			break;

		case TYPE_INT64:
			{
				const int64_t* t = (const int64_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				TP_STRING_FORMAT(buffer, sizeof(buffer), "%zd", *t);
			}
			break;

		case TYPE_UINT32:
			{
				const uint32_t* t = (const uint32_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				TP_STRING_FORMAT(buffer, sizeof(buffer), "%u", *t);
			}
			break;

		case TYPE_UINT64:
			{
				const uint64_t* t = (const uint64_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				TP_STRING_FORMAT(buffer, sizeof(buffer), "%zu", *t);
			}
			break;

		case TYPE_FLOAT:
			{
				const float* t = (const float*)&(((const uint8_t*)aObject)[aField->m_offset]);
				TP_STRING_FORMAT(buffer, sizeof(buffer), "%f", *t);
			}
			break;

		case TYPE_UINT32_ARRAY:
			{
				buffer[0] = '\0';
				const std::vector<uint32_t>* t = (const std::vector<uint32_t>*)&(((const uint8_t*)aObject)[aField->m_offset]);
				for(size_t i = 0; i < t->size(); i++)
				{
					if(i > 0)
					{
						Helpers::StringAppend(buffer, sizeof(buffer), ",");
					}
					else
					{
						char tmp[1024];
						TP_STRING_FORMAT(tmp, sizeof(tmp), "%u", t->at(i));
						Helpers::StringAppend(buffer, sizeof(buffer), tmp);
					}
				}
			}
			break;

		case TYPE_CUSTOM:
			{
				const void* t = (const void*)&(((const uint8_t*)aObject)[aField->m_offset]);

				std::vector<uint8_t> serialized;
				VectorIO::Writer writer(serialized);
				aField->m_customWrite(&writer, t);
				std::string base64;
				Base64::Encode(&serialized[0], serialized.size(), base64);
				return base64; // Return base64 directly
			}
			break;

		default:	
			buffer[0] = '\0';
			break;
		}

		return buffer;
	}

}