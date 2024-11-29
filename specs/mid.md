# .mid File Specifications
Sequence file

## Terms

```
FOURCC      -   four-character code identifying type of data
BYTE        -   data structure of 8 bits
WORD        -   data structure of 16 bits
DWORD       -   data structure of 32 bits
VLV         -   data structure of variable bits (8, 16, 24, 32)
LIST(...)   -   variable-sized list of '...' data
```

## .mid File Structure

.mid files are big-endian and have no alignment

### Header

```
FOURCC          "MThd"
DWORD           size of header (always '6')
WORD            format
WORD            amount tracks
WORD            division
```

### Track

```
FOURCC          "MTrk"
DWORD           size of following data
LIST(
    VLV         delta time
    LIST(
        BYTE    event data
    )
)
```

## Info on Header Data

Format determines format of the file
* '0' is single track
* '1' is several vertically dimensional tracks (played simultaneously)
* '2' is several horizontally dimensional tracks (played sequentially)

Division determines default unit of delta time
* negative value is ticks per SMPTE frame, SMPTE frames per second
    * '0b1ffffffftttttttt'
    * '1' is negative bit
    * 'fffffff' is SMPTE frames per second
        * can be '24', '25', '29' (or drop 30), '30'
    * 'tttttttt' is ticks per SMPTE frame
* positive value is pulses per quarter-note
    * '0b0ttttttttttttttt'
    * '0' is negative bit
    * 'ttttttttttttttt' is pulses per quarter-note

## Info on Event Data

All time values are in microseconds
* microseconds = delta time \* ((tempo / division) / 1000)

All event values are in range 0 to 127 unless otherwise specified

Length of event data is context sensitive
* '0x8X NN VV' is note off event
    * 'X' is channel
    * 'NN' is note
    * 'VV' is release velocity
* '0x9X NN VV' is note on event
    * 'X' is channel
    * 'NN' is note
    * 'VV' is velocity
* '0xAX NN VV' is key pressure event
    * 'X' is channel
    * 'NN' is note
    * 'VV' is pressure
* '0xBX CC VV' is controller event
    * 'X' is channel
    * 'CC' is continuous controller
    * 'VV' is controller value
        * ranged values are lowest (0) to highest (127)
        * time values are lowest (0) to unchanged (64) to highest (127)
        * switch values are off (0 to 63) to on (64 to 127)
* '0xCX VV' is programme change event
    * 'X' is channel
    * 'VV' is programme
* '0xDX VV' is channel pressure event
    * 'X' is channel
    * 'VV' is pressure
* '0xEX CC FF' is pitch bend event
    * 'X' is channel
    * 'CC' is coarse pitch
        * '0' is lowest change (default 2 semitones lower)
        * '64' is no change
        * '127' is highest change (default 2 semitones higher)
    * 'FF' is fine pitch
        * '0' is no change
        * '127' is highest change
* '0xF0 [LL...] MM... VV... 0xF7' is system exclusive event
    * 'LL...' is size of following data
        * Exclusive to file
    * 'MM...' is manufacturer ID
    * 'VV...' is data
    * '0xF7' is end of exclusive event
* '0xF1 VV' is time code event
    * 'VV' is quarter frame value
        * '0bccccvvvv'
        * 'cccc' is type
            * '0' is low nibble of frames
            * '1' is high nibble of frames
            * '2' is low nibble of seconds
            * '3' is high nibble of seconds
            * '4' is low nibble of minutes
            * '5' is high nibble of minutes
            * '6' is low nibble of hours
            * '7' is hours and frame rate
        * 'vvvv' is value
            * '0bvvvv' for frames, seconds, minutes, hours
            * '0b0ppv' for hours and rate
                * '0' is reserved
                * 'pp' is frame rate
                    * '0' is 24 frames per second
                    * '1' is 25 frames per second
                    * '2' is 29.97 (drop 30) frames per second
                    * '3' is 30 frames per second
                * 'v' is high bit of hours
* '0xF2 VVVV' is song position pointer event
    * 'VVVV' is song position
        * '0b0vvvvvvv0vvvvvvv'
        * Forms a 14-bit value (0 to 16383)
* '0xF3 VV' is song select event
    * 'VV' is song id
* '0xF6' is tune request event
* '0xF7 [LL...] MM... VV... [0xF7]' is system exclusive packet event
    * 'LL...' is size of following data
        * Exclusive to file
    * 'MM...' is manufacturer ID
    * 'VV...' is data
        * Includes any data, including real-time events
    * '0xF7' is end of exclusive event
        * Can only appear in final packet
* '0xF8' is time clock event
* '0xFA' is start sequence event
* '0xFB' is continue sequence event
* '0xFC' is stop sequence event
* '0xFE' is active sensing event
* '0xFF' is reset event
* '0xFF TT LL... VV...' is meta event
    * 'TT' is type
        * '0' is song id, always size '2'
            * Must occur at beginning of track
            * Data is 16-bit number
        * '1' is text
        * '2' is copyright
            * Must occur at beginning of first track
        * '3' is track name
            * Should occur at beginning of track
        * '4' is instrument name
        * '5' is lyric
        * '6' is marker
        * '7' is cue
        * '8' is patch name
        * '9' is device name
        * '10' to '15' is unknown text
        * '32' is midi channel prefix, always size '1'
        * '33' is midi port, always size '1'
        * '47' is end of track, always size '0'
            * Must occur at end of every track
        * '81' is tempo, always size '3'
            * Should occur in first track (format 1) or every track (format 2)
            * Data is 24-bit number
            * Undefined effect on MTC files
        * '84' is SMPTE offset, always size '5'
            * Must occur at beginning of track
            * Data is hours, minutes, seconds, frames in SMTPE format and subframes
        * '88' time signature, always size '4'
            * Should occur in first track (format 1) or every track (format 2)
            * Data is numerator, 2 ^ denominator, MIDI clocks per tick, notated 32nd notes per quarter-note
        * '89' is key signature, always size '2'
            * Data is number flats(-)/sharps(+), scale(major is '0', minor is '1')
        * '127' is sequencer exclusive
    * 'LL...' is size of following data
    * 'VV...' is data

### Continuous Controllers

* '0' is bank select (coarse)
* '1' is modulation wheel (coarse)
* '2' is breath controller (coarse)
* '4' is foot controller (coarse)
* '5' is portamento time (coarse)
* '6' is rpn data entry (coarse)
* '7' is channel volume, formerly main volume (coarse)
* '8' is balance (coarse)
* '10' is pan (coarse)
* '11' is expression (coarse)
* '12' is effect control 1 (coarse)
* '13' is effect control 2 (coarse)
* '16' is general purpose controller 1 (coarse)
* '17' is general purpose controller 2 (coarse)
* '18' is general purpose controller 3 (coarse)
* '19' is general purpose controller 4 (coarse)
* '32' is bank select (fine)
* '33' is modulation wheel (fine)
* '34' is breath controller (fine)
* '36' is foot controller (fine)
* '37' is portamento time (fine)
* '38' is rpn data entry (fine)
* '39' is channel volume (fine)
* '40' is balance (fine)
* '42' is pan (fine)
* '43' is expression (fine)
* '44' is effect control 1 (fine)
* '45' is effect control 2 (fine)
* '64' is hold pedal (damper, sustain)
* '65' is portamento pedal
* '66' is sostenuto pedal
* '67' is soft pedal
* '68' is legato pedal
* '69' is hold pedal 2
* '70' is sound controller 1 (default is sound variation)
* '71' is sound controller 2 (default is timbre, harmonic intensity, filter resonance)
* '72' is sound controller 3 (default is release time)
* '73' is sound controller 4 (default is attack time)
* '74' is sound controller 5 (default is brightness, cutoff frequency)
* '75' is sound controller 6 (default is undefined, decay time)
* '76' is sound controller 7 (default is undefined, vibrato rate)
* '77' is sound controller 8 (default is undefined, vibrato depth)
* '78' is sound controller 9 (default is undefined, vibrato delay)
* '79' is sound controller 10 (default is undefined)
* '80' is general purpose controller 5
* '81' is general purpose controller 6
* '82' is general purpose controller 7
* '83' is general purpose controller 8
* '84' is portamento control
* '88' is high resolution velocity prefix
* '91' is effect 1 depth (default is reverb send level, formerly external effect depth)
* '92' is effect 2 depth (formerly tremolo depth)
* '93' is effect 3 depth (default is chorus send level, formerly chorus depth)
* '94' is effect 4 depth (formerly celeste depth)
* '95' is effect 5 depth (formerly phaser level)
* '96' is data increment
* '97' is data decrement
* '98' is non-registered parameter (coarse)
* '99' is non-registered parameter (fine)
* '100' is registered parameter (coarse)
* '101' is registered parameter (fine)
* '120' is all sound off
* '121' is all controllers off
* '122' is local control
* '123' is all notes off
* '124' is omni mode off
* '125' is omni mode on
* '126' is mono operation and all notes off
* '127' is poly operation and all notes off

### Registered Parameter Numbers

* '0xBX 0x64 0x00 ... 0xBX 0x65 0x00' is pitch bend sensitivity
* '0xBX 0x64 0x00 ... 0xBX 0x65 0x01' is channel fine tuning
* '0xBX 0x64 0x00 ... 0xBX 0x65 0x02' is channel coarse tuning
* '0xBX 0x64 0x00 ... 0xBX 0x65 0x03' is select tuning program
* '0xBX 0x64 0x00 ... 0xBX 0x65 0x04' is select tuning bank
* '0xBX 0x64 0x00 ... 0xBX 0x65 0x05' is modulation depth range
* '0xBX 0x64 0x7F ... 0xBX 0x65 0x7F' is cancel RPN

### System Exclusive Manufacturer ID's

* '0x00' is extended
    * '0x00 0x00 0x01' is Time/Warner Interactive
    * '0x00 0x00 0x02' is Advanced Gravis Comp. Tech Ltd.
    * '0x00 0x00 0x03' is Media Vision
    * '0x00 0x00 0x04' is Dornes Research Group
    * '0x00 0x00 0x05' is K-Muse
    * '0x00 0x00 0x06' is Stypher
    * '0x00 0x00 0x07' is Digital Music Corp.
    * '0x00 0x00 0x08' is IOTA Systems
    * '0x00 0x00 0x09' is New England Digital
    * '0x00 0x00 0x0A' is Artisyn
    * '0x00 0x00 0x0B' is IVL Technologies Ltd.
    * '0x00 0x00 0x0C' is Southern Music Systems
    * '0x00 0x00 0x0D' is Lake Butler Sound Company
    * '0x00 0x00 0x0E' is Alesis Studio Electronics
    * '0x00 0x00 0x0F' is Sound Creation
    * '0x00 0x00 0x01' is Time/Warner Interactive
    * '0x00 0x00 0x10' is DOD Electronics Corp.
    * '0x00 0x00 0x11' is Studer-Editech
    * '0x00 0x00 0x12' is Sonus
    * '0x00 0x00 0x13' is Temporal Acuity Products
    * '0x00 0x00 0x14' is Perfect Fretworks
    * '0x00 0x00 0x15' is KAT Inc.
    * '0x00 0x00 0x16' is Opcode Systems
    * '0x00 0x00 0x17' is Rane Corporation
    * '0x00 0x00 0x18' is Anadi Electronique
    * '0x00 0x00 0x19' is KMX
    * '0x00 0x00 0x1A' is Allen & Heath Brenell
    * '0x00 0x00 0x1B' is Peavey Electronics
    * '0x00 0x00 0x1C' is 360 Systems
    * '0x00 0x00 0x1D' is Spectrum Design and Development
    * '0x00 0x00 0x1E' is Marquis Music
    * '0x00 0x00 0x1F' is Zeta Systems
    * '0x00 0x00 0x20' is Axxes (Brian Parsonett)
    * '0x00 0x00 0x21' is Orban
    * '0x00 0x00 0x22' is Indian Valley Mfg.
    * '0x00 0x00 0x23' is Triton
    * '0x00 0x00 0x24' is KTI
    * '0x00 0x00 0x25' is Breakaway Technologies
    * '0x00 0x00 0x26' is Leprecon / CAE Inc.
    * '0x00 0x00 0x27' is Harrison Systems Inc.
    * '0x00 0x00 0x28' is Future Lab/Mark Kuo
    * '0x00 0x00 0x29' is Rocktron Corporation
    * '0x00 0x00 0x2A' is PianoDisc
    * '0x00 0x00 0x2B' is Cannon Research Group
    * '0x00 0x00 0x2C' is Reserved
    * '0x00 0x00 0x2D' is Rodgers Instrument LLC
    * '0x00 0x00 0x2E' is Blue Sky Logic
    * '0x00 0x00 0x2F' is Encore Electronics
    * '0x00 0x00 0x30' is Uptown
    * '0x00 0x00 0x31' is Voce
    * '0x00 0x00 0x32' is CTI Audio, Inc. (Musically Intel. Devs.)
    * '0x00 0x00 0x33' is S3 Incorporated
    * '0x00 0x00 0x34' is Broderbund / Red Orb
    * '0x00 0x00 0x35' is Allen Organ Co.
    * '0x00 0x00 0x36' is Reserved
    * '0x00 0x00 0x37' is Music Quest
    * '0x00 0x00 0x38' is Aphex
    * '0x00 0x00 0x39' is Gallien Krueger
    * '0x00 0x00 0x3A' is IBM
    * '0x00 0x00 0x3B' is Mark Of The Unicorn
    * '0x00 0x00 0x3C' is Hotz Corporation
    * '0x00 0x00 0x3D' is ETA Lighting
    * '0x00 0x00 0x3E' is NSI Corporation
    * '0x00 0x00 0x3F' is Ad Lib, Inc.
    * '0x00 0x00 0x40' is Richmond Sound Design
    * '0x00 0x00 0x41' is Microsoft
    * '0x00 0x00 0x42' is Mindscape (Software Toolworks)
    * '0x00 0x00 0x43' is Russ Jones Marketing / Niche
    * '0x00 0x00 0x44' is Intone
    * '0x00 0x00 0x45' is Advanced Remote Technologies
    * '0x00 0x00 0x46' is White Instruments
    * '0x00 0x00 0x47' is GT Electronics/Groove Tubes
    * '0x00 0x00 0x48' is Pacific Research & Engineering
    * '0x00 0x00 0x49' is Timeline Vista, Inc.
    * '0x00 0x00 0x4A' is Mesa Boogie Ltd.
    * '0x00 0x00 0x4B' is FSLI
    * '0x00 0x00 0x4C' is Sequoia Development Group
    * '0x00 0x00 0x4D' is Studio Electronics
    * '0x00 0x00 0x4E' is Euphonix, Inc
    * '0x00 0x00 0x4F' is InterMIDI, Inc.
    * '0x00 0x00 0x50' is MIDI Solutions Inc.
    * '0x00 0x00 0x51' is 3DO Company
    * '0x00 0x00 0x52' is Lightwave Research / High End Systems
    * '0x00 0x00 0x53' is Micro-W Corporation
    * '0x00 0x00 0x54' is Spectral Synthesis, Inc.
    * '0x00 0x00 0x55' is Lone Wolf
    * '0x00 0x00 0x56' is Studio Technologies Inc.
    * '0x00 0x00 0x57' is Peterson Electro-Musical Product, Inc.
    * '0x00 0x00 0x58' is Atari Corporation
    * '0x00 0x00 0x59' is Marion Systems Corporation
    * '0x00 0x00 0x5A' is Design Event
    * '0x00 0x00 0x5B' is Winjammer Software Ltd.
    * '0x00 0x00 0x5C' is AT&T Bell Laboratories
    * '0x00 0x00 0x5D' is Reserved
    * '0x00 0x00 0x5E' is Symetrix
    * '0x00 0x00 0x5F' is MIDI the World
    * '0x00 0x00 0x60' is Spatializer
    * '0x00 0x00 0x61' is Micros 'N MIDI
    * '0x00 0x00 0x62' is Accordians International
    * '0x00 0x00 0x63' is EuPhonics (now 3Com)
    * '0x00 0x00 0x64' is Musonix
    * '0x00 0x00 0x65' is Turtle Beach Systems (Voyetra)
    * '0x00 0x00 0x66' is Loud Technologies / Mackie
    * '0x00 0x00 0x67' is Compuserve
    * '0x00 0x00 0x68' is BEC Technologies
    * '0x00 0x00 0x69' is QRS Music Inc
    * '0x00 0x00 0x6A' is P.G. Music
    * '0x00 0x00 0x6B' is Sierra Semiconductor
    * '0x00 0x00 0x6C' is EpiGraf
    * '0x00 0x00 0x6D' is Electronics Diversified Inc
    * '0x00 0x00 0x6E' is Tune 1000
    * '0x00 0x00 0x6F' is Advanced Micro Devices
    * '0x00 0x00 0x70' is Mediamation
    * '0x00 0x00 0x71' is Sabine Musical Mfg. Co. Inc.
    * '0x00 0x00 0x72' is Woog Labs
    * '0x00 0x00 0x73' is Micropolis Corp
    * '0x00 0x00 0x74' is Ta Horng Musical Instrument
    * '0x00 0x00 0x75' is e-Tek Labs (Forte Tech)
    * '0x00 0x00 0x76' is Electro-Voice
    * '0x00 0x00 0x77' is Midisoft Corporation
    * '0x00 0x00 0x78' is QSound Labs
    * '0x00 0x00 0x79' is Westrex
    * '0x00 0x00 0x7A' is Nvidia
    * '0x00 0x00 0x7B' is ESS Technology
    * '0x00 0x00 0x7C' is Media Trix Peripherals
    * '0x00 0x00 0x7D' is Brooktree Corp
    * '0x00 0x00 0x7E' is Otari Corp
    * '0x00 0x00 0x7F' is Key Electronics, Inc.
    * '0x00 0x01 0x00' is Shure Incorporated
    * '0x00 0x01 0x01' is AuraSound
    * '0x00 0x01 0x02' is Crystal Semiconductor
    * '0x00 0x01 0x03' is Conexant (Rockwell)
    * '0x00 0x01 0x04' is Silicon Graphics
    * '0x00 0x01 0x05' is M-Audio (Midiman)
    * '0x00 0x01 0x06' is PreSonus
    * '0x00 0x01 0x08' is Topaz Enterprises
    * '0x00 0x01 0x09' is Cast Lighting
    * '0x00 0x01 0x0A' is Microsoft Consumer Division
    * '0x00 0x01 0x0B' is Sonic Foundry
    * '0x00 0x01 0x0C' is Line 6 (Fast Forward) (Yamaha)
    * '0x00 0x01 0x0D' is Beatnik Inc
    * '0x00 0x01 0x0E' is Van Koevering Company
    * '0x00 0x01 0x0F' is Altech Systems
    * '0x00 0x01 0x10' is S & S Research
    * '0x00 0x01 0x11' is VLSI Technology
    * '0x00 0x01 0x12' is Chromatic Research
    * '0x00 0x01 0x13' is Sapphire
    * '0x00 0x01 0x14' is IDRC
    * '0x00 0x01 0x15' is Justonic Tuning
    * '0x00 0x01 0x16' is TorComp Research Inc.
    * '0x00 0x01 0x17' is Newtek Inc.
    * '0x00 0x01 0x18' is Sound Sculpture
    * '0x00 0x01 0x19' is Walker Technical
    * '0x00 0x01 0x1A' is Digital Harmony (PAVO)
    * '0x00 0x01 0x1B' is InVision Interactive
    * '0x00 0x01 0x1C' is T-Square Design
    * '0x00 0x01 0x1D' is Nemesys Music Technology
    * '0x00 0x01 0x1E' is DBX Professional (Harman Intl)
    * '0x00 0x01 0x1F' is Syndyne Corporation
    * '0x00 0x01 0x20' is Bitheadz
    * '0x00 0x01 0x21' is BandLab Technologies
    * '0x00 0x01 0x22' is Analog Devices
    * '0x00 0x01 0x23' is National Semiconductor
    * '0x00 0x01 0x24' is Boom Theory / Adinolfi Alternative Percussion
    * '0x00 0x01 0x25' is Virtual DSP Corporation
    * '0x00 0x01 0x26' is Antares Systems
    * '0x00 0x01 0x27' is Angel Software
    * '0x00 0x01 0x28' is St Louis Music
    * '0x00 0x01 0x29' is Passport Music Software LLC (Gvox)
    * '0x00 0x01 0x2A' is Ashley Audio Inc.
    * '0x00 0x01 0x2B' is Vari-Lite Inc.
    * '0x00 0x01 0x2C' is Summit Audio Inc.
    * '0x00 0x01 0x2D' is Aureal Semiconductor Inc.
    * '0x00 0x01 0x2E' is SeaSound LLC
    * '0x00 0x01 0x2F' is U.S. Robotics
    * '0x00 0x01 0x30' is Aurisis Research
    * '0x00 0x01 0x31' is Nearfield Research
    * '0x00 0x01 0x32' is FM7 Inc
    * '0x00 0x01 0x33' is Swivel Systems
    * '0x00 0x01 0x34' is Hyperactive Audio Systems
    * '0x00 0x01 0x35' is MidiLite (Castle Studios Productions)
    * '0x00 0x01 0x36' is Radikal Technologies
    * '0x00 0x01 0x37' is Roger Linn Design
    * '0x00 0x01 0x38' is TC-Helicon Vocal Technologies
    * '0x00 0x01 0x39' is Event Electronics
    * '0x00 0x01 0x3A' is Sonic Network Inc
    * '0x00 0x01 0x3B' is Realtime Music Solutions
    * '0x00 0x01 0x3C' is Apogee Digital
    * '0x00 0x01 0x3D' is Classical Organs, Inc.
    * '0x00 0x01 0x3E' is Microtools Inc.
    * '0x00 0x01 0x3F' is Numark Industries
    * '0x00 0x01 0x40' is Frontier Design Group, LLC
    * '0x00 0x01 0x41' is Recordare LLC
    * '0x00 0x01 0x42' is Starr Labs
    * '0x00 0x01 0x43' is Voyager Sound Inc.
    * '0x00 0x01 0x44' is Manifold Labs
    * '0x00 0x01 0x45' is Aviom Inc.
    * '0x00 0x01 0x46' is Mixmeister Technology
    * '0x00 0x01 0x47' is Notation Software
    * '0x00 0x01 0x48' is Mercurial Communications
    * '0x00 0x01 0x49' is Wave Arts
    * '0x00 0x01 0x4A' is Logic Sequencing Devices
    * '0x00 0x01 0x4B' is Axess Electronics
    * '0x00 0x01 0x4C' is Muse Research
    * '0x00 0x01 0x4D' is Open Labs
    * '0x00 0x01 0x4E' is Guillemot Corp
    * '0x00 0x01 0x4F' is Samson Technologies
    * '0x00 0x01 0x50' is Electronic Theatre Controls
    * '0x00 0x01 0x51' is Blackberry (RIM)
    * '0x00 0x01 0x52' is Mobileer
    * '0x00 0x01 0x53' is Synthogy
    * '0x00 0x01 0x54' is Lynx Studio Technology Inc.
    * '0x00 0x01 0x55' is Damage Control Engineering LLC
    * '0x00 0x01 0x56' is Yost Engineering, Inc.
    * '0x00 0x01 0x57' is Brooks & Forsman Designs LLC / DrumLite
    * '0x00 0x01 0x58' is Infinite Response
    * '0x00 0x01 0x59' is Garritan Corp
    * '0x00 0x01 0x5A' is Plogue Art et Technologie, Inc
    * '0x00 0x01 0x5B' is RJM Music Technology
    * '0x00 0x01 0x5C' is Custom Solutions Software
    * '0x00 0x01 0x5D' is Sonarcana LLC / Highly Liquid
    * '0x00 0x01 0x5E' is Centrance
    * '0x00 0x01 0x5F' is Kesumo LLC
    * '0x00 0x01 0x60' is Stanton (Gibson Brands)
    * '0x00 0x01 0x61' is Livid Instruments
    * '0x00 0x01 0x62' is First Act / 745 Media
    * '0x00 0x01 0x63' is Pygraphics, Inc.
    * '0x00 0x01 0x64' is Panadigm Innovations Ltd
    * '0x00 0x01 0x65' is Avedis Zildjian Co
    * '0x00 0x01 0x66' is Auvital Music Corp
    * '0x00 0x01 0x67' is You Rock Guitar (was: Inspired Instruments)
    * '0x00 0x01 0x68' is Chris Grigg Designs
    * '0x00 0x01 0x69' is Slate Digital LLC
    * '0x00 0x01 0x6A' is Mixware
    * '0x00 0x01 0x6B' is Social Entropy
    * '0x00 0x01 0x6C' is Source Audio LLC
    * '0x00 0x01 0x6D' is Ernie Ball / Music Man
    * '0x00 0x01 0x6E' is Fishman
    * '0x00 0x01 0x6F' is Custom Audio Electronics
    * '0x00 0x01 0x70' is American Audio/DJ
    * '0x00 0x01 0x71' is Mega Control Systems
    * '0x00 0x01 0x72' is Kilpatrick Audio
    * '0x00 0x01 0x73' is iConnectivity
    * '0x00 0x01 0x74' is Fractal Audio
    * '0x00 0x01 0x75' is NetLogic Microsystems
    * '0x00 0x01 0x76' is Music Computing
    * '0x00 0x01 0x77' is Nektar Technology Inc
    * '0x00 0x01 0x78' is Zenph Sound Innovations
    * '0x00 0x01 0x79' is DJTechTools.com
    * '0x00 0x01 0x7A' is Rezonance Labs
    * '0x00 0x01 0x7B' is Decibel Eleven
    * '0x00 0x01 0x7C' is CNMAT
    * '0x00 0x01 0x7D' is Media Overkill
    * '0x00 0x01 0x7E' is Confusion Studios
    * '0x00 0x01 0x7F' is moForte Inc
    * '0x00 0x02 0x00' is Miselu Inc
    * '0x00 0x02 0x01' is Amelia's Compass LLC
    * '0x00 0x02 0x02' is Zivix LLC
    * '0x00 0x02 0x03' is Artiphon
    * '0x00 0x02 0x04' is Synclavier Digital
    * '0x00 0x02 0x05' is Light & Sound Control Devices LLC
    * '0x00 0x02 0x06' is Retronyms Inc
    * '0x00 0x02 0x07' is JS Technologies
    * '0x00 0x02 0x08' is Quicco Sound
    * '0x00 0x02 0x09' is A-Designs Audio
    * '0x00 0x02 0x0A' is McCarthy Music Corp
    * '0x00 0x02 0x0B' is Denon DJ
    * '0x00 0x02 0x0C' is Keith Robert Murray
    * '0x00 0x02 0x0D' is Google
    * '0x00 0x02 0x0E' is ISP Technologies
    * '0x00 0x02 0x0F' is Abstrakt Instruments LLC
    * '0x00 0x02 0x10' is Meris LLC
    * '0x00 0x02 0x11' is Sensorpoint LLC
    * '0x00 0x02 0x12' is Hi-Z Labs
    * '0x00 0x02 0x13' is Imitone
    * '0x00 0x02 0x14' is Intellijel Designs Inc.
    * '0x00 0x02 0x15' is Dasz Instruments Inc.
    * '0x00 0x02 0x16' is Remidi
    * '0x00 0x02 0x17' is Disaster Area Designs LLC
    * '0x00 0x02 0x18' is Universal Audio
    * '0x00 0x02 0x19' is Carter Duncan Corp
    * '0x00 0x02 0x1A' is Essential Technology
    * '0x00 0x02 0x1B' is Cantux Research LLC
    * '0x00 0x02 0x1C' is Hummel Technologies
    * '0x00 0x02 0x1D' is Sensel Inc
    * '0x00 0x02 0x1E' is DBML Group
    * '0x00 0x02 0x1F' is Madrona Labs
    * '0x00 0x02 0x20' is Mesa Boogie
    * '0x00 0x02 0x21' is Effigy Labs
    * '0x00 0x02 0x22' is Amenote
    * '0x00 0x02 0x23' is Red Panda LLC
    * '0x00 0x02 0x24' is OnSong LLC
    * '0x00 0x02 0x25' is Jamboxx Inc.
    * '0x00 0x02 0x26' is Electro-Harmonix 
    * '0x00 0x02 0x27' is RnD64 Inc
    * '0x00 0x02 0x28' is Neunaber Technology LLC 
    * '0x00 0x02 0x29' is Kaom Inc.
    * '0x00 0x02 0x2A' is Hallowell EMC
    * '0x00 0x02 0x2B' is Sound Devices, LLC
    * '0x00 0x02 0x2C' is Spectrasonics, Inc
    * '0x00 0x02 0x2D' is Second Sound, LLC
    * '0x00 0x02 0x2E' is 8eo (Horn)
    * '0x00 0x02 0x2F' is VIDVOX LLC
    * '0x00 0x02 0x30' is Matthews Effects
    * '0x00 0x02 0x31' is Bright Blue Beetle
    * '0x00 0x02 0x32' is Audio Impressions
    * '0x00 0x02 0x33' is Looperlative
    * '0x00 0x02 0x34' is Steinway
    * '0x00 0x02 0x35' is Ingenious Arts and Technologies LLC
    * '0x00 0x02 0x36' is DCA Audio
    * '0x00 0x02 0x37' is Buchla USA
    * '0x00 0x02 0x38' is Sinicon
    * '0x00 0x02 0x39' is Isla Instruments
    * '0x00 0x02 0x3A' is Soundiron LLC
    * '0x00 0x02 0x3B' is Sonoclast, LLC
    * '0x00 0x02 0x3C' is Copper and Cedar
    * '0x00 0x02 0x3D' is Whirled Notes
    * '0x00 0x02 0x3E' is Cejetvole, LLC
    * '0x00 0x02 0x3F' is DAWn Audio LLC
    * '0x00 0x02 0x40' is Space Brain Circuits
    * '0x00 0x02 0x41' is Caedence 
    * '0x00 0x02 0x42' is HCN Designs, LLC (The MIDI Maker)
    * '0x00 0x02 0x43' is PTZOptics
    * '0x00 0x02 0x44' is Noise Engineering
    * '0x00 0x02 0x45' is Synthesia LLC
    * '0x00 0x02 0x46' is Jeff Whitehead Lutherie LLC
    * '0x00 0x02 0x47' is Wampler Pedals Inc.
    * '0x00 0x02 0x48' is Tapis Magique
    * '0x00 0x02 0x49' is Leaf Secrets
    * '0x00 0x02 0x4A' is Groove Synthesis
    * '0x00 0x20 0x00' is Dream SAS
    * '0x00 0x20 0x01' is Strand Lighting
    * '0x00 0x20 0x02' is Amek Div of Harman Industries
    * '0x00 0x20 0x03' is Casa Di Risparmio Di Loreto
    * '0x00 0x20 0x04' is Böhm electronic GmbH
    * '0x00 0x20 0x05' is Syntec Digital Audio
    * '0x00 0x20 0x06' is Trident Audio Developments
    * '0x00 0x20 0x07' is Real World Studio
    * '0x00 0x20 0x08' is Evolution Synthesis, Ltd
    * '0x00 0x20 0x09' is Yes Technology
    * '0x00 0x20 0x0A' is Audiomatica
    * '0x00 0x20 0x0B' is Bontempi SpA (Sigma)
    * '0x00 0x20 0x0C' is F.B.T. Elettronica SpA
    * '0x00 0x20 0x0D' is MidiTemp GmbH
    * '0x00 0x20 0x0E' is LA Audio (Larking Audio)
    * '0x00 0x20 0x0F' is Zero 88 Lighting Limited
    * '0x00 0x20 0x10' is Micon Audio Electronics GmbH
    * '0x00 0x20 0x11' is Forefront Technology
    * '0x00 0x20 0x12' is Studio Audio and Video Ltd.
    * '0x00 0x20 0x13' is Kenton Electronics
    * '0x00 0x20 0x14' is Celco/ Electrosonic
    * '0x00 0x20 0x15' is ADB
    * '0x00 0x20 0x16' is Marshall Products Limited
    * '0x00 0x20 0x17' is DDA
    * '0x00 0x20 0x18' is BSS Audio Ltd.
    * '0x00 0x20 0x19' is MA Lighting Technology
    * '0x00 0x20 0x1A' is Fatar SRL c/o Music Industries
    * '0x00 0x20 0x1B' is QSC Audio Products Inc.
    * '0x00 0x20 0x1C' is Artisan Clasic Organ Inc.
    * '0x00 0x20 0x1D' is Orla Spa
    * '0x00 0x20 0x1E' is Pinnacle Audio (Klark Teknik PLC)
    * '0x00 0x20 0x1F' is TC Electronics
    * '0x00 0x20 0x20' is Doepfer Musikelektronik GmbH
    * '0x00 0x20 0x21' is Creative ATC / E-mu
    * '0x00 0x20 0x22' is Seyddo/Minami
    * '0x00 0x20 0x23' is LG Electronics (Goldstar)
    * '0x00 0x20 0x24' is Midisoft sas di M.Cima & C
    * '0x00 0x20 0x25' is Samick Musical Inst. Co. Ltd.
    * '0x00 0x20 0x26' is Penny and Giles (Bowthorpe PLC)
    * '0x00 0x20 0x27' is Acorn Computer
    * '0x00 0x20 0x28' is LSC Electronics Pty. Ltd.
    * '0x00 0x20 0x29' is Focusrite/Novation
    * '0x00 0x20 0x2A' is Samkyung Mechatronics
    * '0x00 0x20 0x2B' is Medeli Electronics Co.
    * '0x00 0x20 0x2C' is Charlie Lab SRL
    * '0x00 0x20 0x2D' is Blue Chip Music Technology
    * '0x00 0x20 0x2E' is BEE OH Corp
    * '0x00 0x20 0x2F' is LG Semicon America
    * '0x00 0x20 0x30' is TESI
    * '0x00 0x20 0x31' is EMAGIC
    * '0x00 0x20 0x32' is Behringer GmbH
    * '0x00 0x20 0x33' is Access Music Electronics
    * '0x00 0x20 0x34' is Synoptic
    * '0x00 0x20 0x35' is Hanmesoft
    * '0x00 0x20 0x36' is Terratec Electronic GmbH
    * '0x00 0x20 0x37' is Proel SpA
    * '0x00 0x20 0x38' is IBK MIDI
    * '0x00 0x20 0x39' is IRCAM
    * '0x00 0x20 0x3A' is Propellerhead Software
    * '0x00 0x20 0x3B' is Red Sound Systems Ltd
    * '0x00 0x20 0x3C' is Elektron ESI AB
    * '0x00 0x20 0x3D' is Sintefex Audio
    * '0x00 0x20 0x3E' is MAM (Music and More)
    * '0x00 0x20 0x3F' is Amsaro GmbH
    * '0x00 0x20 0x40' is CDS Advanced Technology BV (Lanbox)
    * '0x00 0x20 0x41' is Mode Machines (Touched By Sound GmbH)
    * '0x00 0x20 0x42' is DSP Arts
    * '0x00 0x20 0x43' is Phil Rees Music Tech
    * '0x00 0x20 0x44' is Stamer Musikanlagen GmbH
    * '0x00 0x20 0x45' is Musical Muntaner S.A. dba Soundart
    * '0x00 0x20 0x46' is C-Mexx Software
    * '0x00 0x20 0x47' is Klavis Technologies
    * '0x00 0x20 0x48' is Noteheads AB
    * '0x00 0x20 0x49' is Algorithmix
    * '0x00 0x20 0x4A' is Skrydstrup R&D
    * '0x00 0x20 0x4B' is Professional Audio Company
    * '0x00 0x20 0x4C' is NewWave Labs (MadWaves)
    * '0x00 0x20 0x4D' is Vermona
    * '0x00 0x20 0x4E' is Nokia
    * '0x00 0x20 0x4F' is Wave Idea
    * '0x00 0x20 0x50' is Hartmann GmbH
    * '0x00 0x20 0x51' is Lion's Tracs
    * '0x00 0x20 0x52' is Analogue Systems
    * '0x00 0x20 0x53' is Focal-JMlab
    * '0x00 0x20 0x54' is Ringway Electronics (Chang-Zhou) Co Ltd
    * '0x00 0x20 0x55' is Faith Technologies (Digiplug)
    * '0x00 0x20 0x56' is Showworks
    * '0x00 0x20 0x57' is Manikin Electronic
    * '0x00 0x20 0x58' is 1 Come Tech
    * '0x00 0x20 0x59' is Phonic Corp
    * '0x00 0x20 0x5A' is Dolby Australia (Lake)
    * '0x00 0x20 0x5B' is Silansys Technologies
    * '0x00 0x20 0x5C' is Winbond Electronics
    * '0x00 0x20 0x5D' is Cinetix Medien und Interface GmbH
    * '0x00 0x20 0x5E' is A&G Soluzioni Digitali
    * '0x00 0x20 0x5F' is Sequentix GmbH
    * '0x00 0x20 0x60' is Oram Pro Audio
    * '0x00 0x20 0x61' is Be4 Ltd
    * '0x00 0x20 0x62' is Infection Music
    * '0x00 0x20 0x63' is Central Music Co. (CME)
    * '0x00 0x20 0x64' is genoQs Machines GmbH
    * '0x00 0x20 0x65' is Medialon
    * '0x00 0x20 0x66' is Waves Audio Ltd
    * '0x00 0x20 0x67' is Jerash Labs
    * '0x00 0x20 0x68' is Da Fact
    * '0x00 0x20 0x69' is Elby Designs
    * '0x00 0x20 0x6A' is Spectral Audio
    * '0x00 0x20 0x6B' is Arturia
    * '0x00 0x20 0x6C' is Vixid
    * '0x00 0x20 0x6D' is C-Thru Music
    * '0x00 0x20 0x6E' is Ya Horng Electronic Co LTD
    * '0x00 0x20 0x6F' is SM Pro Audio
    * '0x00 0x20 0x70' is OTO Machines
    * '0x00 0x20 0x71' is ELZAB S.A. (G LAB)
    * '0x00 0x20 0x72' is Blackstar Amplification Ltd
    * '0x00 0x20 0x73' is M3i Technologies GmbH
    * '0x00 0x20 0x74' is Gemalto (from Xiring)
    * '0x00 0x20 0x75' is Prostage SL
    * '0x00 0x20 0x76' is Teenage Engineering
    * '0x00 0x20 0x77' is Tobias Erichsen Consulting
    * '0x00 0x20 0x78' is Nixer Ltd
    * '0x00 0x20 0x79' is Hanpin Electron Co Ltd
    * '0x00 0x20 0x7A' is "MIDI-hardware" R.Sowa
    * '0x00 0x20 0x7B' is Beyond Music Industrial Ltd
    * '0x00 0x20 0x7C' is Kiss Box B.V.
    * '0x00 0x20 0x7D' is Misa Digital Technologies Ltd
    * '0x00 0x20 0x7E' is AI Musics Technology Inc
    * '0x00 0x20 0x7F' is Serato Inc LP
    * '0x00 0x21 0x00' is Limex
    * '0x00 0x21 0x01' is Kyodday (Tokai)
    * '0x00 0x21 0x02' is Mutable Instruments
    * '0x00 0x21 0x03' is PreSonus Software Ltd
    * '0x00 0x21 0x04' is Ingenico (was Xiring)
    * '0x00 0x21 0x05' is Fairlight Instruments Pty Ltd
    * '0x00 0x21 0x06' is Musicom Lab
    * '0x00 0x21 0x07' is Modal Electronics (Modulus/VacoLoco)
    * '0x00 0x21 0x08' is RWA (Hong Kong) Limited
    * '0x00 0x21 0x09' is Native Instruments
    * '0x00 0x21 0x0A' is Naonext
    * '0x00 0x21 0x0B' is MFB
    * '0x00 0x21 0x0C' is Teknel Research
    * '0x00 0x21 0x0D' is Ploytec GmbH
    * '0x00 0x21 0x0E' is Surfin Kangaroo Studio
    * '0x00 0x21 0x0F' is Philips Electronics HK Ltd
    * '0x00 0x21 0x10' is ROLI Ltd
    * '0x00 0x21 0x11' is Panda-Audio Ltd
    * '0x00 0x21 0x12' is BauM Software
    * '0x00 0x21 0x13' is Machinewerks Ltd.
    * '0x00 0x21 0x14' is Xiamen Elane Electronics
    * '0x00 0x21 0x15' is Marshall Amplification PLC
    * '0x00 0x21 0x16' is Kiwitechnics Ltd
    * '0x00 0x21 0x17' is Rob Papen
    * '0x00 0x21 0x18' is Spicetone OU
    * '0x00 0x21 0x19' is V3Sound
    * '0x00 0x21 0x1A' is IK Multimedia
    * '0x00 0x21 0x1B' is Novalia Ltd
    * '0x00 0x21 0x1C' is Modor Music
    * '0x00 0x21 0x1D' is Ableton
    * '0x00 0x21 0x1E' is Dtronics
    * '0x00 0x21 0x1F' is ZAQ Audio
    * '0x00 0x21 0x20' is Muabaobao Education Technology Co Ltd
    * '0x00 0x21 0x21' is Flux Effects
    * '0x00 0x21 0x22' is Audiothingies (MCDA)
    * '0x00 0x21 0x23' is Retrokits
    * '0x00 0x21 0x24' is Morningstar FX Pte Ltd
    * '0x00 0x21 0x25' is Changsha Hotone Audio Co Ltd
    * '0x00 0x21 0x26' is Expressive E
    * '0x00 0x21 0x27' is Expert Sleepers Ltd
    * '0x00 0x21 0x28' is Timecode-Vision Technology
    * '0x00 0x21 0x29' is Hornberg Research GbR
    * '0x00 0x21 0x2A' is Sonic Potions
    * '0x00 0x21 0x2B' is Audiofront
    * '0x00 0x21 0x2C' is Fred's Lab
    * '0x00 0x21 0x2D' is Audio Modeling
    * '0x00 0x21 0x2E' is C. Bechstein Digital GmbH
    * '0x00 0x21 0x2F' is Motas Electronics Ltd
    * '0x00 0x21 0x30' is Elk Audio
    * '0x00 0x21 0x31' is Sonic Academy Ltd
    * '0x00 0x21 0x32' is Bome Software
    * '0x00 0x21 0x33' is AODYO SAS
    * '0x00 0x21 0x34' is Pianoforce S.R.O
    * '0x00 0x21 0x35' is Dreadbox P.C.
    * '0x00 0x21 0x36' is TouchKeys Instruments Ltd
    * '0x00 0x21 0x37' is The Gigrig Ltd
    * '0x00 0x21 0x38' is ALM Co
    * '0x00 0x21 0x39' is CH Sound Design
    * '0x00 0x21 0x3A' is Beat Bars
    * '0x00 0x21 0x3B' is Blokas
    * '0x00 0x21 0x3C' is GEWA Music GmbH
    * '0x00 0x21 0x3D' is dadamachines
    * '0x00 0x21 0x3E' is Augmented Instruments Ltd (Bela)
    * '0x00 0x21 0x3F' is Supercritical Ltd
    * '0x00 0x21 0x40' is Genki Instruments
    * '0x00 0x21 0x41' is Marienberg Devices Germany
    * '0x00 0x21 0x42' is Supperware Ltd
    * '0x00 0x21 0x43' is Imoxplus BVBA 
    * '0x00 0x21 0x44' is Swapp Technologies SRL
    * '0x00 0x21 0x45' is Electra One S.R.O.
    * '0x00 0x21 0x46' is Digital Clef Limited
    * '0x00 0x21 0x47' is Paul Whittington Group Ltd
    * '0x00 0x21 0x48' is Music Hackspace
    * '0x00 0x21 0x49' is Bitwig GMBH
    * '0x00 0x21 0x4A' is Enhancia
    * '0x00 0x21 0x4B' is KV 331
    * '0x00 0x21 0x4C' is Tehnicadelarte
    * '0x00 0x21 0x4D' is Endlesss Studio
    * '0x00 0x21 0x4E' is Dongguan MIDIPLUS Co., LTD
    * '0x00 0x21 0x4F' is Gracely Pty Ltd.
    * '0x00 0x21 0x50' is Embodme
    * '0x00 0x21 0x51' is MuseScore
    * '0x00 0x21 0x52' is EPFL (E-Lab)
    * '0x00 0x21 0x53' is Orb3 Ltd.
    * '0x00 0x21 0x54' is Pitch Innovations
    * '0x00 0x21 0x55' is Playces 
    * '0x00 0x21 0x56' is UDO Audio LTD
    * '0x00 0x21 0x57' is RSS Sound Design
    * '0x00 0x21 0x58' is Nonlinear Labs GmbH
    * '0x00 0x21 0x59' is Robkoo Information & Technologies Co., Ltd.
    * '0x00 0x21 0x5A' is Cari Electronic
    * '0x00 0x21 0x5B' is Oxi Electronic Instruments SL
    * '0x00 0x21 0x5C' is XMPT
    * '0x00 0x21 0x5D' is SHANGHAI HUAXIN MUSICAL INSTRUMENT 
    * '0x00 0x21 0x5E' is Shenzhen Huashi Technology Co., Ltd
    * '0x00 0x21 0x60' is Guangzhou Rantion Technology Co., Ltd. 
    * '0x00 0x21 0x61' is Ryme Music
    * '0x00 0x21 0x62' is GS Music
    * '0x00 0x21 0x63' is Shenzhen Flamma Innovation Co., Ltd
    * '0x00 0x21 0x64' is Shenzhen Mooer Audio Co.,LTD. 
    * '0x00 0x21 0x65' is Raw Material Software Limited (JUCE)
    * '0x00 0x21 0x66' is Birdkids
    * '0x00 0x21 0x67' is Beijing QianYinHuLian Tech. Co
    * '0x00 0x21 0x68' is Nimikry Music OG
    * '0x00 0x21 0x69' is Newzik
    * '0x00 0x21 0x6A' is Hamburg Wave
    * '0x00 0x21 0x6B' is Grimm Audio
    * '0x00 0x21 0x6C' is Arcana Instruments LTD.
    * '0x00 0x21 0x6D' is GameChanger Audio
    * '0x00 0x21 0x6E' is OakTone
    * '0x00 0x21 0x6F' is The Digi-Gurdy: A MIDI Hurdy Gurdy
    * '0x00 0x21 0x70' is MusiKraken
    * '0x00 0x21 0x71' is PhotoSynth > InterFACE
    * '0x00 0x21 0x72' is Instruments of Things
    * '0x00 0x40 0x00' is Crimson Technology Inc.
    * '0x00 0x40 0x01' is Softbank Mobile Corp
    * '0x00 0x40 0x03' is D&M Holdings Inc.
    * '0x00 0x40 0x04' is Xing Inc.
    * '0x00 0x40 0x05' is AlphaTheta Corporation
    * '0x00 0x40 0x06' is Pioneer Corporation
    * '0x00 0x40 0x07' is Slik Corporation
    * '0x00 0x48 0x00' is Sigboost Co., Ltd.
    * '0x00 0x48 0x01' is Lost Technology
    * '0x00 0x48 0x02' is Uchiwa Fujin
    * '0x00 0x48 0x03' is Tsukuba Science Co., Ltd.
    * '0x00 0x48 0x04' is Sonicware Co., Ltd.
    * '0x00 0x48 0x05' is Poppy only workshop
* '0x01' is Sequential Circuits
* '0x02' is IDP
* '0x03' is Voyetra Turtle Beach, Inc.
* '0x04' is Moog Music
* '0x05' is Passport Designs
* '0x06' is Lexicon Inc.
* '0x07' is Kurzweil / Young Chang
* '0x08' is Fender
* '0x09' is MIDI9
* '0x0A' is AKG Acoustics
* '0x0B' is Voyce Music
* '0x0C' is WaveFrame (Timeline)
* '0x0D' is ADA Signal Processors, Inc.
* '0x0E' is Garfield Electronics
* '0x0F' is Ensoniq
* '0x10' is Oberheim / Gibson Labs
* '0x11' is Apple
* '0x12' is Grey Matter Response
* '0x13' is Digidesign Inc.
* '0x14' is Palmtree Instruments
* '0x15' is JLCooper Electronics
* '0x16' is Lowrey Organ Company
* '0x17' is Adams-Smith
* '0x18' is E-mu
* '0x19' is Harmony Systems
* '0x1A' is ART
* '0x1B' is Baldwin
* '0x1C' is Eventide
* '0x1D' is Inventronics
* '0x1E' is Key Concepts
* '0x1F' is Clarity
* '0x20' is Passac
* '0x21' is Proel Labs (SIEL)
* '0x22' is Synthaxe (UK)
* '0x23' is Stepp
* '0x24' is Hohner
* '0x25' is Twister
* '0x26' is Ketron s.r.l.
* '0x27' is Jellinghaus MS
* '0x28' is Southworth Music Systems
* '0x29' is PPG (Germany)
* '0x2A' is JEN
* '0x2B' is Solid State Logic Organ Systems
* '0x2C' is Audio Veritrieb-P. Struven
* '0x2D' is Neve
* '0x2E' is Soundtracs Ltd.
* '0x2F' is Elka
* '0x30' is Dynacord
* '0x31' is Viscount International Spa (Intercontinental Electronics)
* '0x32' is Drawmer
* '0x33' is Clavia Digital Instruments
* '0x34' is Audio Architecture
* '0x35' is Generalmusic Corp SpA
* '0x36' is Cheetah Marketing
* '0x37' is C.T.M.
* '0x38' is Simmons UK
* '0x39' is Soundcraft Electronics
* '0x3A' is Steinberg Media Technologies GmbH
* '0x3B' is Wersi Gmbh
* '0x3C' is AVAB Niethammer AB
* '0x3D' is Digigram
* '0x3E' is Waldorf Electronics GmbH
* '0x3F' is Quasimidi
* '0x40' is Kawai Musical Instruments MFG. CO. Ltd
* '0x41' is Roland Corporation
* '0x42' is Korg Inc.
* '0x43' is Yamaha Corporation
* '0x44' is Casio Computer Co. Ltd
* '0x46' is Kamiya Studio Co. Ltd
* '0x47' is Akai Electric Co. Ltd.
* '0x48' is Victor Company of Japan, Ltd.
* '0x4B' is Fujitsu Limited
* '0x4C' is Sony Corporation
* '0x4E' is Teac Corporation
* '0x50' is Matsushita Electric Industrial Co. , Ltd
* '0x51' is Fostex Corporation
* '0x52' is Zoom Corporation
* '0x54' is Matsushita Communication Industrial Co., Ltd.
* '0x55' is Suzuki Musical Instruments MFG. Co., Ltd.
* '0x56' is Fuji Sound Corporation Ltd.
* '0x57' is Acoustic Technical Laboratory, Inc.
* '0x59' is Faith, Inc.
* '0x5A' is Internet Corporation
* '0x5C' is Seekers Co. Ltd.
* '0x5F' is SD Card Association
* '0x7D' is educational
* '0x7E' is universal (non-real time)
* '0x7F' is universal (real time)

### System Exclusive Universal Messages (non-real)

* '0x00' is unused
* '0x01' is sample dump header
* '0x02' is sample data packet
* '0x03' is sample dump request
* '0x04 VV' is MIDI time code
    * '0x00' is special
    * '0x01' is punch-in points
    * '0x02' is punch-out points
    * '0x03' is delete punch-in point
    * '0x04' is delete punch-out point
    * '0x05' is event start point
    * '0x06' is event stop point
    * '0x07' is event start points with additional info
    * '0x08' is event stop points with additional info
    * '0x09' is delete event start point
    * '0x0A' is delete event stop point
    * '0x0B' is cue points
    * '0x0C' is cue points with additional info
    * '0x0D' is delete cue point
    * '0x0E' is event name in additional info
* '0x05 VV' is sample dump extensions
    * '0x01' is multiple loop points
    * '0x02' is loop points request
* '0x06 VV' is general information
    * '0x01' is identity request
    * '0x02' is identity reply
* '0x07 VV' is file dump
    * '0x01' is header
    * '0x02' is data packet
    * '0x03' is request
* '0x08 VV' is MIDI tuning standard
    * '0x00' is bulk dump request
    * '0x01' is bulk dump reply
* '0x09 VV' is general MIDI
    * '0x01' is general MIDI system on
    * '0x02' is general MIDI system off
* '0x7B' is end of file
* '0x7C' is wait
* '0x7D' is cancel
* '0x7E' is negative acknowledgment (NAK)
* '0x7F' is acknowledgment (ACK)

### System Exclusive Universal Messages (real)

* '0x00' is unused
* '0x01 VV' is MIDI time code
    * '0x01' is full message
    * '0x02' is user bits
* '0x02 VV' is MIDI show control
    * '0x00' is msc extensions
    * '0x01' to '0x7F' is ...
* '0x03 VV' is notation information
    * '0x01' is bar number
    * '0x02' is time signature (immediate)
    * '0x42' is time signature (delayed)
* '0x04 VV' is device control
    * '0x01' is master volume
    * '0x02' is master balance
* '0x05 VV' is real time mtc cueing
    * '0x00' is special
    * '0x01' is punch-in points
    * '0x02' is punch-out points
    * '0x05' is event start point
    * '0x06' is event stop point
    * '0x07' is event start points with additional info
    * '0x08' is event stop points with additional info
    * '0x0B' is cue points
    * '0x0C' is cue points with additional info
    * '0x0E' is event name in additional info
* '0x06 VV' is MIDI machine control commands
    * ...
* '0x07 VV' is MIDI machine control responses
    * ...
* '0x08 VV' is MIDI tuning standard
    * '0x02' is note change
