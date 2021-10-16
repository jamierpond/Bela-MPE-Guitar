/***** MidiParsing.h *****/

Midi midi;

const char* gMidiPort0 = "hw:0,0,0";

int MIDI_CHANNEL_ZERO_NOTE_ON = 144;
int MIDI_CHANNEL_ZERO_NOTE_OFF = 128;
int MIDI_CHANNEL_ZERO_AFTERTOUCH = 208;
int MIDI_CHANNEL_ZERO_PITCHBEND = 224;

void writeNote(int event, int channel, midi_byte_t note, midi_byte_t velocity = 0)
{
	int eventType = (event == 0) ? MIDI_CHANNEL_ZERO_NOTE_ON : MIDI_CHANNEL_ZERO_NOTE_OFF;
	midi_byte_t statusByte = midi_byte_t(eventType + channel); // control change on channel 0
	midi_byte_t bytes[3] = {statusByte, note, velocity};
	midi.writeOutput(bytes, 3); // send a control change message
}

void writeAftertouch(int channel, midi_byte_t value)
{
	
	midi_byte_t statusByte = midi_byte_t(MIDI_CHANNEL_ZERO_AFTERTOUCH + channel); // control change on channel 0
	midi_byte_t bytes[2] = {statusByte, value};
	midi.writeOutput(bytes, 2); // send a control change message
	
	// rt_printf("Channel %i, Value %i \n", channel, (int)value);
}

void writePitchBend(int channel, midi_byte_t value)
{
	
	midi_byte_t statusByte = midi_byte_t(MIDI_CHANNEL_ZERO_PITCHBEND + channel); // control change on channel 0
	midi_byte_t bytes[2] = {statusByte, value};
	midi.writeOutput(bytes, 2); // send a control change message
	
	// rt_printf("Channel %i, Value %i \n", channel, (int)value);
}

float midiToFrequency(float midiNote)
{
	return 440.f * powf(2, (midiNote - 69.f)/12.f);
}

void sysexCallback(midi_byte_t byte, void* arg)
{
	printf("Sysex byte");
	if(arg != NULL){
		 printf(" from midi port %s", (const char*) arg);
	}
	printf(": %d\n", byte);
}





