/***** NoteEvent *****/
     
enum NoteEventType {
	INVALID = -1,
	NOTEON = 0,
	NOTEOFF,
	SUSTAIN,
};
     
struct NoteEvent
{
	NoteEvent(int _midiNote, int _midiChannel, int _velocity, float _touchLocation, int _fretStringHash) : 
	midiNote(_midiNote), midiChannel(_midiChannel), velocity(_velocity), 
	initialTouchLocation(_touchLocation), fretStringHash(_fretStringHash)
	{
		isActive = true;
		isDueNoteOnset = true;
	}
	
	void setNewEvent(int _midiNote, int _midiChannel, int _velocity, float _touchLocation)
	{
		midiNote = _midiNote;
		midiChannel = _midiChannel;
		velocity = _velocity;
		initialTouchLocation = _touchLocation;
		
		isActive = true;
		isDueNoteOnset =  true;
	}
	
	void markNoteOnsetComplete()
	{
		isDueNoteOnset = false;
	}
	
	void markAsComplete()
	{
		isActive = false;
	}
	
	NoteEvent(){}
	
	int midiNote = -1;
	int midiChannel = -1;
	int velocity = -1;
	int pressure = -1;
	int pitchBend = -1;
	float initialTouchLocation = -1;
	int fretStringHash = -1;
	bool isActive = false;
	bool isDueNoteOnset  = false;
	bool isDueNoteOffset = false;
};













