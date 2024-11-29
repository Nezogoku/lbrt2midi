# .lrt File Specifications
Patapon sequence file

## Terms

```
FOURCC      -   four-character code identifying type of data
BYTE        -   data structure of 8 bits
WORD        -   data structure of 16 bits
DWORD       -   data structure of 32 bits
LIST(...)   -   variable-sized list of '...' data
```

## .lrt File Structure

.lrt files are little-endian and appear to have WORD alignment

### Header

```
FOURCC          "LBRT"
DWORD           address to event data
DWORD           MIDI clock ticks per click
DWORD           pulses per quarter-note
```

### Sub Header

```
DWORD           amount tracks
LIST(
    DWORD       track ID (?)
    DWORD       unknown (usually '256')
    DWORD       amount events
    DWORD       amount quarter events
    LIST(
        DWORD   address to quarter event
    )
)
```

### Event Data

```
LIST(
    DWORD       event ID
    DWORD       delta time
    DWORD       time value
    DWORD       value 1
    WORD        value 2
    WORD        value 3
    WORD        note on velocity
    WORD        note on value
    BYTE        channel
    BYTE        status
    BYTE        note off velocity
    BYTE        note off value
)
```

## Info on Event Data

Event ID's with a value of '0' mark the beginning and end of a track

All time values are in microseconds

Time value is context sensitive
* Current tempo for running status
* Duration for note event
* Tempo for tempo event (BPM = 60000000 / time value)
    
Value 1 is context sensitive
* Pitch for note event
* Value for controller event
    * '0x14' is loop start for loop event
    * '0x1E' is loop stop for loop event

Value 3 is context sensitive
* Instrument for note event
* Controller for controller event
    * '0x63' is loop event
* Meta Type for meta event
    * '0x2F' is end of track event
    * '0x51' is tempo event

Status determines context
* '0x00' is running event
* '0x9X' is note event
* '0xBX' is controller event
* '0xFF' is meta event