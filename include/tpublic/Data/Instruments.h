#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		// Instruments for generated music, like a SoundFont: samples (each a sound) and presets made of zones. Values are
		// as stored in SoundFont files (2.01 sections 8.1 and 8.2), so these are usually made from one by a tool.
		//
		// sample <name>: { sound: <sound> root_key: <key> fine_tune: <cents> loop: [ <start> <end> ] looped: <bool> }
		// preset <name>: { bank: <n> program: <n> zone: { keys: [ <low> <high> ] velocities: [ <low> <high> ] sample: <name>
		//   set: [ <generator> <value> ... ] add: [ <generator> <value> ... ]
		//   instrument_modulators: [ <source> <destination> <amount> <amount source> <transform> ... ] preset_modulators: [ ... ] } }
		//
		// "set" generators replace the defaults (instrument level), "add" generators are added (preset level).
		// Instrument modulators replace the defaults, preset modulators are added.
		struct Instruments
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_INSTRUMENTS;
			static const bool TAGGED = false;

			struct Sample
			{
				void
				ToStream(
					IWriter*				aWriter) const
				{
					aWriter->WriteUInt(m_soundId);
					aWriter->WriteInt(m_rootKey);
					aWriter->WriteInt(m_fineTune);
					aWriter->WriteUInt(m_loopStart);
					aWriter->WriteUInt(m_loopEnd);
					aWriter->WriteBool(m_looped);
				}

				bool
				FromStream(
					IReader*				aReader)
				{
					if(!aReader->ReadUInt(m_soundId))
						return false;
					if(!aReader->ReadInt(m_rootKey))
						return false;
					if(!aReader->ReadInt(m_fineTune))
						return false;
					if(!aReader->ReadUInt(m_loopStart))
						return false;
					if(!aReader->ReadUInt(m_loopEnd))
						return false;
					if(!aReader->ReadBool(m_looped))
						return false;
					return true;
				}

				// Public data
				std::string				m_name;					// Not serialized
				uint32_t				m_soundId = 0;
				int32_t					m_rootKey = 60;
				int32_t					m_fineTune = 0;			// Cents
				uint32_t				m_loopStart = 0;		// Frames (48 kHz)
				uint32_t				m_loopEnd = 0;			// First frame after the loop
				bool					m_looped = false;		// Played looped by some zone
			};

			struct Generator
			{
				void
				ToStream(
					IWriter*				aWriter) const
				{
					aWriter->WriteUInt(m_id);
					aWriter->WriteInt(m_value);
				}

				bool
				FromStream(
					IReader*				aReader)
				{
					if(!aReader->ReadUInt(m_id))
						return false;
					if(!aReader->ReadInt(m_value))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_id = 0;
				int32_t					m_value = 0;
			};

			struct Modulator
			{
				void
				ToStream(
					IWriter*				aWriter) const
				{
					aWriter->WriteUInt(m_source);
					aWriter->WriteUInt(m_destination);
					aWriter->WriteInt(m_amount);
					aWriter->WriteUInt(m_amountSource);
					aWriter->WriteUInt(m_transform);
				}

				bool
				FromStream(
					IReader*				aReader)
				{
					if(!aReader->ReadUInt(m_source))
						return false;
					if(!aReader->ReadUInt(m_destination))
						return false;
					if(!aReader->ReadInt(m_amount))
						return false;
					if(!aReader->ReadUInt(m_amountSource))
						return false;
					if(!aReader->ReadUInt(m_transform))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_source = 0;
				uint32_t				m_destination = 0;
				int32_t					m_amount = 0;
				uint32_t				m_amountSource = 0;
				uint32_t				m_transform = 0;
			};

			struct Zone
			{
				Zone()
				{

				}

				Zone(
					const SourceNode*		aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "keys")
							_GetRange(aChild, m_keyLow, m_keyHigh);
						else if(aChild->m_name == "velocities")
							_GetRange(aChild, m_velocityLow, m_velocityHigh);
						else if(aChild->m_name == "sample")
							m_sampleName = aChild->GetIdentifier();
						else if(aChild->m_name == "set")
							_GetGenerators(aChild, m_set);
						else if(aChild->m_name == "add")
							_GetGenerators(aChild, m_add);
						else if(aChild->m_name == "instrument_modulators")
							_GetModulators(aChild, m_instrumentModulators);
						else if(aChild->m_name == "preset_modulators")
							_GetModulators(aChild, m_presetModulators);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});

					TP_VERIFY(!m_sampleName.empty(), aSource->m_debugInfo, "Zone has no sample.");
				}

				void
				ToStream(
					IWriter*				aWriter) const
				{
					aWriter->WriteUInt(m_keyLow);
					aWriter->WriteUInt(m_keyHigh);
					aWriter->WriteUInt(m_velocityLow);
					aWriter->WriteUInt(m_velocityHigh);
					aWriter->WriteUInt(m_sampleIndex);
					aWriter->WriteObjects(m_set);
					aWriter->WriteObjects(m_add);
					aWriter->WriteObjects(m_instrumentModulators);
					aWriter->WriteObjects(m_presetModulators);
				}

				bool
				FromStream(
					IReader*				aReader)
				{
					if(!aReader->ReadUInt(m_keyLow))
						return false;
					if(!aReader->ReadUInt(m_keyHigh))
						return false;
					if(!aReader->ReadUInt(m_velocityLow))
						return false;
					if(!aReader->ReadUInt(m_velocityHigh))
						return false;
					if(!aReader->ReadUInt(m_sampleIndex))
						return false;
					if(!aReader->ReadObjects(m_set))
						return false;
					if(!aReader->ReadObjects(m_add))
						return false;
					if(!aReader->ReadObjects(m_instrumentModulators))
						return false;
					if(!aReader->ReadObjects(m_presetModulators))
						return false;
					return true;
				}

				// Public data
				uint32_t					m_keyLow = 0;
				uint32_t					m_keyHigh = 127;
				uint32_t					m_velocityLow = 0;
				uint32_t					m_velocityHigh = 127;
				std::string					m_sampleName;		// Not serialized
				DataErrorHandling::DebugInfo m_debugInfo;		// Not serialized
				uint32_t					m_sampleIndex = 0;
				std::vector<Generator>		m_set;
				std::vector<Generator>		m_add;
				std::vector<Modulator>		m_instrumentModulators;
				std::vector<Modulator>		m_presetModulators;

			private:

				static void
				_GetRange(
					const SourceNode*		aSource,
					uint32_t&				aOutLow,
					uint32_t&				aOutHigh)
				{
					const SourceNode* array = aSource->GetArray();
					TP_VERIFY(array->m_children.size() == 2, aSource->m_debugInfo, "Expected [ <low> <high> ].");
					aOutLow = array->m_children[0]->GetUInt32();
					aOutHigh = array->m_children[1]->GetUInt32();
					TP_VERIFY(aOutLow <= aOutHigh && aOutHigh <= 127, aSource->m_debugInfo, "Invalid range.");
				}

				static void
				_GetGenerators(
					const SourceNode*		aSource,
					std::vector<Generator>&	aOut)
				{
					const SourceNode* array = aSource->GetArray();
					TP_VERIFY(array->m_children.size() % 2 == 0, aSource->m_debugInfo, "Expected [ <generator> <value> ... ].");
					for(size_t i = 0; i < array->m_children.size(); i += 2)
					{
						Generator t;
						t.m_id = array->m_children[i]->GetUInt32();
						t.m_value = array->m_children[i + 1]->GetInt32();
						aOut.push_back(t);
					}
				}

				static void
				_GetModulators(
					const SourceNode*		aSource,
					std::vector<Modulator>&	aOut)
				{
					const SourceNode* array = aSource->GetArray();
					TP_VERIFY(array->m_children.size() % 5 == 0, aSource->m_debugInfo, "Expected [ <source> <destination> <amount> <amount source> <transform> ... ].");
					for(size_t i = 0; i < array->m_children.size(); i += 5)
					{
						Modulator t;
						t.m_source = array->m_children[i]->GetUInt32();
						t.m_destination = array->m_children[i + 1]->GetUInt32();
						t.m_amount = array->m_children[i + 2]->GetInt32();
						t.m_amountSource = array->m_children[i + 3]->GetUInt32();
						t.m_transform = array->m_children[i + 4]->GetUInt32();
						aOut.push_back(t);
					}
				}
			};

			struct Preset
			{
				void
				ToStream(
					IWriter*				aWriter) const
				{
					aWriter->WriteString(m_name);
					aWriter->WriteUInt(m_bank);
					aWriter->WriteUInt(m_program);
					aWriter->WriteObjects(m_zones);
				}

				bool
				FromStream(
					IReader*				aReader)
				{
					if(!aReader->ReadString(m_name))
						return false;
					if(!aReader->ReadUInt(m_bank))
						return false;
					if(!aReader->ReadUInt(m_program))
						return false;
					if(!aReader->ReadObjects(m_zones))
						return false;
					return true;
				}

				// Public data
				std::string				m_name;
				uint32_t				m_bank = 0;
				uint32_t				m_program = 0;
				std::vector<Zone>		m_zones;
			};

			void
			Verify() const
			{
				VerifyBase();

				for(const Preset& preset : m_presets)
				{
					for(const Preset& other : m_presets)
						TP_VERIFY(&preset == &other || preset.m_bank != other.m_bank || preset.m_program != other.m_program, m_debugInfo, "'%s' has two presets with bank %u and program %u.", m_name.c_str(), preset.m_bank, preset.m_program);
				}
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
						if(aChild->m_tag == "sample")
							_ReadSample(aChild);
						else if(aChild->m_tag == "preset")
							_ReadPreset(aChild);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});

				// Zones refer to samples by name, which can come later
				std::unordered_map<std::string, uint32_t> sampleTable;
				for(uint32_t i = 0; i < (uint32_t)m_samples.size(); i++)
				{
					TP_VERIFY(!sampleTable.contains(m_samples[i].m_name), aSource->m_debugInfo, "Sample '%s' defined more than once.", m_samples[i].m_name.c_str());
					sampleTable[m_samples[i].m_name] = i;
				}

				for(Preset& preset : m_presets)
				{
					for(Zone& zone : preset.m_zones)
					{
						std::unordered_map<std::string, uint32_t>::const_iterator i = sampleTable.find(zone.m_sampleName);
						TP_VERIFY(i != sampleTable.cend(), zone.m_debugInfo, "'%s' is not a sample.", zone.m_sampleName.c_str());
						zone.m_sampleIndex = i->second;
					}
				}
			}

			void
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteObjects(m_samples);
				aWriter->WriteObjects(m_presets);
			}

			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadObjects(m_samples))
					return false;
				if(!aReader->ReadObjects(m_presets))
					return false;
				return true;
			}

			// Public data
			std::vector<Sample>			m_samples;
			std::vector<Preset>			m_presets;

		private:

			void
			_ReadSample(
				const SourceNode*		aSource)
			{
				Sample t;
				t.m_name = aSource->m_name;

				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "sound")
					{
						t.m_soundId = aChild->GetId(DataType::ID_SOUND);
					}
					else if(aChild->m_name == "root_key")
					{
						t.m_rootKey = aChild->GetInt32();
						TP_VERIFY(t.m_rootKey >= 0 && t.m_rootKey <= 127, aChild->m_debugInfo, "Invalid root key.");
					}
					else if(aChild->m_name == "fine_tune")
					{
						t.m_fineTune = aChild->GetInt32();
					}
					else if(aChild->m_name == "loop")
					{
						const SourceNode* array = aChild->GetArray();
						TP_VERIFY(array->m_children.size() == 2, aChild->m_debugInfo, "Expected [ <start> <end> ].");
						t.m_loopStart = array->m_children[0]->GetUInt32();
						t.m_loopEnd = array->m_children[1]->GetUInt32();
						TP_VERIFY(t.m_loopStart < t.m_loopEnd, aChild->m_debugInfo, "Invalid loop.");
					}
					else if(aChild->m_name == "looped")
					{
						t.m_looped = aChild->GetBool();
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});

				TP_VERIFY(t.m_soundId != 0, aSource->m_debugInfo, "Sample '%s' has no sound.", t.m_name.c_str());
				m_samples.push_back(t);
			}

			void
			_ReadPreset(
				const SourceNode*		aSource)
			{
				Preset t;
				t.m_name = aSource->m_name;

				bool hasProgram = false;
				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "bank")
					{
						t.m_bank = aChild->GetUInt32();
					}
					else if(aChild->m_name == "program")
					{
						t.m_program = aChild->GetUInt32();
						hasProgram = true;
					}
					else if(aChild->m_name == "zone")
					{
						Zone zone(aChild);
						zone.m_debugInfo = aChild->m_debugInfo;
						t.m_zones.push_back(zone);
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});

				TP_VERIFY(hasProgram && t.m_bank <= 128 && t.m_program <= 127, aSource->m_debugInfo, "Preset '%s' needs a bank (0-128) and a program (0-127).", t.m_name.c_str());
				m_presets.push_back(t);
			}
		};

	}

}
