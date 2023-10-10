#include "Pcheader.h"

#include <tpublic/ComponentSchema.h>
#include <tpublic/Vec2.h>

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

			while(initFromDeprecatedId != 0)
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
		const Parser::Node*	aSource,
		void*				aObject) const
	{
		aSource->ForEachChild([&](
			const Parser::Node* aChild)
		{
			const char* fieldName = aChild->m_name.c_str();

			const Field* field = _GetFieldByName(fieldName);
			TP_VERIFY(field != NULL, aChild->m_debugInfo, "'%s' is not a valid component field.", fieldName);
			TP_VERIFY(field->m_offset != UINT32_MAX, aChild->m_debugInfo, "'%s' is a deprecated component field.", fieldName);

			switch(field->m_type)
			{
			case TYPE_VEC2:
				{
					TP_VERIFY(aChild->m_children.size() == 2 && aChild->m_type == Parser::Node::TYPE_ARRAY, aChild->m_debugInfo, "Syntax error.");
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
						*t = aSource->m_sourceContext->m_persistentIdTable->GetId(field->m_dataType, aChild->GetIdentifier());
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

			case TYPE_UINT32_ARRAY:
				{
					TP_VERIFY(aChild->m_type == Parser::Node::TYPE_ARRAY, aChild->m_debugInfo, "Syntax error.");
					std::vector<uint32_t>* t = (std::vector<uint32_t>*)&(((const uint8_t*)aObject)[field->m_offset]);
					aChild->ForEachChild([t](
						const Parser::Node* aElement)
					{
						t->push_back(aElement->GetUInt32());
					});
				}
				break;

			case TYPE_CUSTOM:
				{
					void* t = (void*) & (((const uint8_t*)aObject)[field->m_offset]);
					assert(field->m_customReadSource);
					field->m_customReadSource(aChild, t);
				}
				break;

			default:
				break;
			}
		});
	}
	
	void			
	ComponentSchema::WriteNetwork(
		IWriter*			aWriter,
		const void*			aObject) const
	{
		for(const Field& field : m_fields)
		{
			bool deprecated = field.m_offset == UINT32_MAX;

			if(!deprecated)
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

			if(!deprecated)
			{
				aWriter->WriteUInt(field.m_id + 1); // Ids can't be zero (end-of-stream)

				_WriteValue(aWriter, aObject, &field);
			}
		}

		// Mark end
		aWriter->WriteUInt(0);
	}
	
	bool			
	ComponentSchema::ReadStorage(
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

		return true;
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
				Vec2* t = (Vec2*)&(((const uint8_t*)aObject)[aField->m_offset]);
				if(!t->FromStream(aReader))
					return false;
			}
			break;

		case TYPE_STRING:
			{				
				std::string* t = (std::string*)&(((const uint8_t*)aObject)[aField->m_offset]);
				if(!aReader->ReadString(*t))
					return false;
			}
			break;

		case TYPE_BOOL:
			{
				bool* t = (bool*)&(((const uint8_t*)aObject)[aField->m_offset]);
				if(!aReader->ReadBool(*t))
					return false;
			}
			break;

		case TYPE_INT32:
			{
				int32_t* t = (int32_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				if(!aReader->ReadInt(*t))
					return false;
			}
			break;

		case TYPE_INT64:
			{
				int64_t* t = (int64_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				if(!aReader->ReadInt(*t))
					return false;
			}
			break;

		case TYPE_UINT32:
			{
				uint32_t* t = (uint32_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				if(!aReader->ReadUInt(*t))
					return false;
			}
			break;

		case TYPE_UINT64:
			{
				uint64_t* t = (uint64_t*)&(((const uint8_t*)aObject)[aField->m_offset]);
				if(!aReader->ReadUInt(*t))
					return false;
			}
			break;

		case TYPE_UINT32_ARRAY:
			{
				std::vector<uint32_t>* t = (std::vector<uint32_t>*)&(((const uint8_t*)aObject)[aField->m_offset]);
				if(!aReader->ReadUInts(*t))
					return false;
			}
			break;

		case TYPE_CUSTOM:
			{
				void* t = (void*)&(((const uint8_t*)aObject)[aField->m_offset]);
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

}