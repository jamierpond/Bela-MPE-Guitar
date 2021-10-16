/***** ActivityMatrix.h *****/
/*
	Creates two width*height binary matricies
	in order to compare the current touch state
	with the previous touch state. 
	i.e. if this is previousActiveLocations:
	[0] [0] [1] [0]
	[0] [0] [0] [0]
	
	and this is currentActiveLocations:
	[0] [0] [0] [0]
	[1] [0] [0] [0]
	
	We know there needs to register a NOTEOFF on [0,2]
	and send a NOTEON on [1,0].
*/

#pragma once
#include "NoteEvent.h"
#include "TouchMatrix.h"
#include "PrintHelpers.h"
// #include "MidiParsing.h"

std::vector<int> standardTuningProfile = {0, 5, 10, 15, 19, 24};
int LOW_E_MIDI = 40;
int getMidiNote(int fret, int str)
{
	return fret + standardTuningProfile[str] + LOW_E_MIDI;
}

inline int cantorPairing(int x, int y)
{
	return (int)((x + y) * (x + y +1)/2 + y);
}

class PersistenceInterpretor
{
public:

	/* We want to create a frets*strings matrix.
	 * so we can access each note by [fret][string].
	 */
	PersistenceInterpretor(int _numFrets, int _numStrings, std::vector<NoteEvent>& _noteEventData)
	: mNumFrets(_numFrets), mNumStrings(_numStrings), 
	currentActiveLocations(mNumFrets, mNumStrings),
	previousActiveLocations(mNumFrets, mNumStrings),
	pNoteEventData(_noteEventData)
	{
		
	}
	
	void resetCurrentActiveLocations()
	{
		currentActiveLocations.reset();
	}
	
	void setCurrentAsPrevious()
	{
		previousActiveLocations.copy(currentActiveLocations);
	}
	
	bool noteOnMatchingFunction(int fret, int str)
	{
		auto prevValue    = previousActiveLocations.getValue(fret, str).isActive;
		auto currentValue = currentActiveLocations.getValue(fret, str).isActive;
		bool match        = (currentValue && !prevValue);
		// if(match)
			// rt_printf("fret: %i, string: %i, P: %i, C: %i \n", fret, str, prevValue, currentValue);
		return match;
	}
	
	bool noteOffMatchingFunction(int fret, int str)
	{
		return (!currentActiveLocations.getValue(fret, str).isActive && previousActiveLocations.getValue(fret, str).isActive);
	}
	 
 	void updateMatrix(int fret, int str, int velocity, float location)
	{
		currentActiveLocations.setValue(fret, str, true, velocity, location);
	}
	 
	void noteOnEventMatch(int fret, int str)
	{
		//rt_printf("note on \n");
		
		// static int count = 0;
		// rt_printf("apparently note on %i \n", count++);
		int currentVelocity = currentActiveLocations.getValue(fret, str).velocity;
		float touchLocation = currentActiveLocations.getValue(fret, str).touchLocation;
		bool hasFoundValidNote = false;
		int i = 0;
		while(!hasFoundValidNote)
		{
			if(!pNoteEventData[i].isActive)
			{
				hasFoundValidNote = true;
				pNoteEventData[i] = NoteEvent(getMidiNote(fret, str), 0, currentVelocity, touchLocation, cantorPairing(fret, str));
			}
			
			if(i++ >= 15) break; // If out of range, stop, something has gone wrong.
		}
	}
	
	void updateCCParameters(int fret, int str)
	{
		for(int i = 0; i < 15; i++)
		{
			if(pNoteEventData[i].fretStringHash == cantorPairing(fret, str) && pNoteEventData[i].isActive)
			{
				pNoteEventData[i].pressure = currentActiveLocations.getValue(fret, str).velocity;
				
				float locationDifference = pNoteEventData[i].initialTouchLocation 
												- currentActiveLocations.getValue(fret, str).touchLocation;
				
				float locationDifferenceMax = 1.f/mNumStrings; 
				
				int PITCHBEND_MAX_VALUE = 16383;
				
				locationDifference /= locationDifferenceMax;
				locationDifference  = powf(locationDifference, 3);
				locationDifference *= PITCHBEND_MAX_VALUE * 2;
				locationDifference -= PITCHBEND_MAX_VALUE/2;
				
				pNoteEventData[i].pitchBend = (int)locationDifference;
			}
		}
	}
	
	void noteOffEventMatch(int fret,int str)
	{
		bool hasFoundTheRightNoteToTurnOff = false;
		int i = 0;
		while(!hasFoundTheRightNoteToTurnOff)
		{
			if(pNoteEventData[i].fretStringHash == cantorPairing(fret, str) && pNoteEventData[i].isActive)
			{
				pNoteEventData[i].isDueNoteOffset = true;
				hasFoundTheRightNoteToTurnOff = true;
			}
			if(i++ > 15) break; // If out of range, stop, something has gone wrong.
		}
	}
	 
	void interpretNoteEventData()
	{
		for(int fret = 0; fret < mNumFrets; fret++)
		{
			for(int str = 0; str < mNumStrings; str++)
			{
				if(noteOnMatchingFunction(fret, str))
				{
					noteOnEventMatch(fret, str);
				}
				
				if(noteOffMatchingFunction(fret, str))
				{
					//rt_printf("note off! \n");
					noteOffEventMatch(fret, str);
				}
					
				updateCCParameters(fret, str);
			}
		}
	}
     
	int mNumFrets, mNumStrings;
	TouchMatrix currentActiveLocations, previousActiveLocations;

	std::vector<NoteEvent>& pNoteEventData;
};

