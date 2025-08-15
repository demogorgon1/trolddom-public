#pragma once

#include "RealmModifier.h"

namespace tpublic
{

	class IReader;
	class IWriter;

	class RealmModifierList
	{
	public:
		static const uint32_t VERSION = 1;

		struct Entry
		{
			void		ToStream(
							IWriter*					aWriter) const;
			bool		FromStream(
							IReader*					aReader);

			// Public data
			RealmModifier::Id	m_id = RealmModifier::INVALID_ID;			
			float				m_float = 0.0f;
			bool				m_bool = false;
		};

		void			ToStream(
							IWriter*					aWriter) const;
		bool			FromStream(
							IReader*					aReader);
		void			SetDefined(
							bool						aDefined);
		Entry*			Add(
							const Entry&				aEntry);
		void			Set(
							RealmModifier::Id			aId,
							const char*					aString);
		const Entry*	Get(
							RealmModifier::Id			aId) const;
		Entry*			Get(
							RealmModifier::Id			aId);
		Entry*			GetAutoDefault(
							RealmModifier::Id			aId);
		bool			GetFlag(
							RealmModifier::Id			aId,
							bool						aDefault) const;
		float			GetMultiplier(
							RealmModifier::Id			aId,
							float						aDefault) const;
		void			ToggleFlag(
							RealmModifier::Id			aId);
		void			PruneDefaults();
		void			GetAsStringArray(
							std::vector<std::string>&	aOut) const;
		void			SetFromConfig(
							const char*					aRealmModifierString,
							const char*					aValue);
		
		// Data access
		bool			IsDefined() const { return m_defined; }

	private:

		std::vector<Entry>		m_entries;
		bool					m_defined = false;

		void			_SetString(
							Entry*						aEntry,
							const char*					aString) const;
		bool			_IsDefault(
							const Entry*				aEntry) const;
	};

}