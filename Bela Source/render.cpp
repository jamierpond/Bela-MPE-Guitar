/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io    

\example Trill/bar-sound

Trill Bar Multitouch Theremin     
=============================

This example shows how to communicate with the Trill Bar sensor using
the Trill library.

Each touch on the sensor controls the pitch and volume of an oscillator.

The Trill sensor is scanned on an auxiliary task running parallel to the audio thread
and the number of active touches, their position and size stored as global variables.

Position and size for each touch are then mapped to frequency and amplitude of their
corresponding oscillator. Changes in frequency and amplitude are smoothed using
low pass filters to avoid artifacts.
*/

#include <Bela.h>
#include <cmath>
#include <cstring> 
#include <vector>
#include <algorithm>
#include <libraries/Midi/Midi.h>
#include <stdlib.h>
#include <libraries/Trill/Trill.h>
#include <libraries/OnePole/OnePole.h>
#include <libraries/Oscillator/Oscillator.h>


#include "Utilities.h"
#include "PersistenceInterpretor.h"
#include "MidiParsing.h"

/* This determines how much of the X direction to use. At 1.0, the
 * whole width of the Trill sensor will be used. At 0.5, half the sensor
 * width will used. 
 */
float totalXRange = 1.0;    

/* This determines the offset. at 0.1, the start of the first string
 * will be drawn 20% away from the 'edge' of the 'neck'. At 0.0 
 * the width of the 'neck' will not be affected.
 */
float rangeOffset = 0.0;

const int NUM_TOUCH   = 5;   // Number of touches on Trill sensor
const int NUM_STRINGS = 6; // Number of strings for the 'guitar'.
const int NUM_FRETS   = 6;  // How many frets are being used?   
const int MAX_NUM_MIDI_CHANNELS = 16; // How many MIDI channels will be supported?

std::vector<float> stringCentres(NUM_STRINGS);
#define LOW_E_MIDI 40 

std::vector<bool> activeMidiChannels(MAX_NUM_MIDI_CHANNELS);
std::vector<bool> midiChannelToMidiNoteMapping(MAX_NUM_MIDI_CHANNELS);

std::vector<float> stringBoundaries(NUM_STRINGS); // Which string has been touched by the user?
float stringBoundarySize;


struct TouchSensorData
{
	TouchSensorData() {};
	Trill trill;
	std::vector<float> touchLocation{NUM_TOUCH};
	std::vector<float> touchSize{NUM_TOUCH};
	int numTouches = 0;
};

std::vector<TouchSensorData> sensors{NUM_FRETS};

std::vector<NoteEvent> gNoteEventData(16); // 16 = MAX_NUM_MIDI_CHANNELS. 

PersistenceInterpretor activityMatrix(NUM_FRETS, NUM_STRINGS, gNoteEventData);

// Sleep time for auxiliary task in microseconds
unsigned int gTaskSleepTime = 12000; // microseconds

/*
 * Function to be run on an auxiliary task that reads data from the Trill sensor.
 * Here, a loop is defined so that the task runs recurrently for as long as the
 * audio thread is running.
 */
 
void writeMidiFromNoteEvents()
{
	// This is where we iterate though the NoteEvents and execute them!
	for(int i = 0; i < gNoteEventData.size(); i++)
	{
		if(gNoteEventData[i].isActive)
		{
			if(gNoteEventData[i].isDueNoteOffset)
			{
				//printNoteEvent(gNoteEventData[i]);
				writeNote(1, i, gNoteEventData[i].midiNote);
				gNoteEventData[i].markAsComplete();
			}
			
			if(gNoteEventData[i].isDueNoteOnset)
			{
				//printNoteEvent(gNoteEventData[i]);
				gNoteEventData[i].isDueNoteOnset = false;
				writeNote(0, i, gNoteEventData[i].midiNote, gNoteEventData[i].velocity);
			}
				
			writeAftertouch(i, gNoteEventData[i].pressure);
			//midi.writePitchBend((midi_byte_t)i, gNoteEventData[i].pitchBend);
		}
	}
}
 
void loop(void*)
{
	while(!Bela_stopRequested())
	{
		for(int fret = 0; fret < NUM_FRETS; fret++)
		{
			// Read locations from Trill sensor
			sensors[fret].trill.readI2C();
			sensors[fret].numTouches = sensors[fret].trill.getNumTouches();
			for(unsigned int i = 0; i < sensors[fret].numTouches; i++) {
				sensors[fret].touchLocation[i] = sensors[fret].trill.touchLocation(i);
				sensors[fret].touchSize[i]     = sensors[fret].trill.touchSize(i);
			}
		}
		
		writeMidiFromNoteEvents();
		
		usleep(gTaskSleepTime);
	}
}

bool setup(BelaContext *context, void *userData)
{
	for(int fret = 0; fret < NUM_FRETS; fret++)
	{
		// Setup a Trill Bar sensor on i2c bus 1, using the default mode and address
		if(sensors[fret].trill.setup(1, Trill::BAR, 32 + fret) != 0) {
			fprintf(stderr, "Unable to initialise Trill Bar\n");
			return false;
		}
		sensors[fret].trill.printDetails();
	}
		
	// Calculate the boundaries between each string. 
	for(int i = 0; i < NUM_STRINGS; i++)
	{
		stringBoundaries[i] = (totalXRange/static_cast<float>(NUM_STRINGS)) * static_cast<float>(i) + rangeOffset;
		stringCentres[i] = stringBoundaries[i] + stringBoundaries[0] * 0.5;
		printFloat(stringBoundaries[i]);
	}
	
	stringBoundarySize = stringBoundaries[0];

	// Set and schedule auxiliary task for reading sensor data from the I2C bus
	Bela_runAuxiliaryTask(loop);
	
	midi.readFrom(gMidiPort0);
	midi.writeTo(gMidiPort0);
	midi.enableParser(true);
	//midi.getParser()->setCallback(midiMessageCallback, (void*) gMidiPort0);
	midi.getParser()->setSysexCallback(sysexCallback, (void*) gMidiPort0);
	//gSamplingPeriod = 1 / context->audioSampleRate;
	
	return true;
}

int determineActiveString(float touchPosition)
{
	int activeString = -1;
	if(touchPosition < stringBoundaries[0])
		activeString = NUM_STRINGS - 1;
	else
	{
		for(int i = 1; i <= NUM_STRINGS; i++)
		{
		 	if (touchPosition > stringBoundaries[stringBoundaries.size() - i])
		 	{
		 		activeString = i - 1;
		 		break;
		 	}
		}
	}
	return activeString;
}

void printNoteEventData(int numElements)
{
	rt_printf("==================================================== \n");
	for(int i = 0; i < numElements; i++)
	{
		rt_printf("%i. Note: %i, Active: %i, dueOnset: %i, dueOffset %i \n",
			i, gNoteEventData[i].midiNote, (bool)gNoteEventData[i].isActive,
			(bool)gNoteEventData[i].isDueNoteOnset, (bool)gNoteEventData[i].isDueNoteOffset);
	}
}

void printNoteEvent(NoteEvent& n)
{
	rt_printf("Note: %i, Channel %i, Velocity %i \n", 
		n.midiNote, n.midiChannel, n.velocity);
}

void printBreakLine()
{
	rt_printf("======================================\n");
}

void render(BelaContext *context, void *userData)
{
	// Find all active touches and copy them into the touch interpreter (activityMatrix). 
	for (int fret = 0; fret < NUM_FRETS; fret++)
	{
		for(int i = 0; i < sensors[fret].numTouches; i++)
		{
			//rt_printf("location %f \n", sensors[fret].touchLocation[i]);
			int activeString = determineActiveString(sensors[fret].touchLocation[i]);

			// This signifies that there is a valid note played. 
			int velocity = (int)map(sensors[fret].touchSize[i], 0, 1, 0, 127);
			if(velocity >= 127) velocity = 127;
		    activityMatrix.updateMatrix(fret, activeString, velocity, sensors[fret].touchLocation[i]);
		}
	}
	
	// This is where we interpret the raw touch data into note events wrt the previous touch matrix.
	// This updates the active note list, which will mark flags like 'Play a note on!', or "Keep this note, on but update the 
	// pitch bend and AT!". Will also turn notes off if required.
	activityMatrix.interpretNoteEventData();
	activityMatrix.setCurrentAsPrevious();
	activityMatrix.resetCurrentActiveLocations();
}

void cleanup(BelaContext *context, void *userData)
{

}

