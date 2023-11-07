set audio.driver coreaudio
set midi.driver coremidi
unload 1
unload 2
unload 3
unload 4
unload 5
set synth.default-soundfont /Users/m4/src/GeneralUser/GeneralUser.sf2
set synth.midi-channels 128
set synth.verbose 0
reset
reset
load /Users/m4/src/GeneralUser/GeneralUser.sf2
set synth.reverb.active 1
set synth.reverb.room-size 0.61
set synth.reverb.damp 0.23
set synth.reverb.width 0.76
set synth.reverb.level 0.57
set synth.chorus.active 1
set synth.chorus.nr 3
set synth.chorus.level 1.2
set synth.chorus.speed 0.3
set synth.chorus.depth 8
prog 00 000
echo "Beethoven - Symphony No. 2 "
echo "Movement 3 in C - Op. 36 "
echo "Dem Fursten von Lichnowsky gewidmet. "
echo "Dedicated to Prince Von Lichnowsky. "
echo "voice 1"
cc 0 0 0
cc 0 32 0
select 0 1 0 73
pitch_bend 0 8192
cc 0 11 80
cc 0 7 91
cc 0 10 64
cc 0 5 0
cc 0 65 0
cc 0 68 0
cc 0 64 0
cc 0 91 54
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "bars 12"
echo "* tempo 100,2d"
echo "page 40"
echo "measure 1 - $$ Page 40, Top, 1st"
echo "voice 2"
cc 1 0 0
cc 1 32 0
select 1 1 0 73
pitch_bend 1 8092
cc 1 11 80
cc 1 7 91
cc 1 10 64
cc 1 5 0
cc 1 65 0
cc 1 68 0
cc 1 64 0
cc 1 91 54
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "voice 3"
cc 2 0 0
cc 2 32 0
select 2 1 0 68
pitch_bend 2 8192
cc 2 11 80
cc 2 7 91
cc 2 10 64
cc 2 5 0
cc 2 65 0
cc 2 68 0
cc 2 64 0
cc 2 91 54
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "voice 4"
cc 3 0 0
cc 3 32 0
select 3 1 0 68
pitch_bend 3 8092
cc 3 11 80
cc 3 7 91
cc 3 10 64
cc 3 5 0
cc 3 65 0
cc 3 68 0
cc 3 64 0
cc 3 91 54
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "voice 5"
cc 4 0 0
cc 4 32 0
select 4 1 0 71
pitch_bend 4 8192
cc 4 11 80
cc 4 7 91
cc 4 10 64
cc 4 5 0
cc 4 65 0
cc 4 68 0
cc 4 64 0
cc 4 91 54
echo "meter 3 2 48 8"
echo "key F 'major'"
echo "voice 6"
cc 5 0 0
cc 5 32 0
select 5 1 0 71
pitch_bend 5 8092
cc 5 11 80
cc 5 7 91
cc 5 10 64
cc 5 5 0
cc 5 65 0
cc 5 68 0
cc 5 64 0
cc 5 91 54
echo "meter 3 2 48 8"
echo "key F 'major'"
echo "voice 7"
cc 6 0 0
cc 6 32 0
select 6 1 0 70
pitch_bend 6 8192
cc 6 11 80
cc 6 7 100
cc 6 10 64
cc 6 5 0
cc 6 65 0
cc 6 68 0
cc 6 64 0
cc 6 91 67
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "voice 8"
cc 7 0 0
cc 7 32 0
select 7 1 0 70
pitch_bend 7 8092
cc 7 11 80
cc 7 7 100
cc 7 10 64
cc 7 5 0
cc 7 65 0
cc 7 68 0
cc 7 64 0
cc 7 91 67
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "voice 9"
cc 8 0 0
cc 8 32 0
select 8 1 0 60
pitch_bend 8 8192
cc 8 11 80
cc 8 7 91
cc 8 10 64
cc 8 5 0
cc 8 65 0
cc 8 68 0
cc 8 64 0
cc 8 91 0
echo "meter 3 2 48 8"
echo "key C 'major'"
echo "voice 10"
cc 10 0 0
cc 10 32 0
select 10 1 0 60
pitch_bend 10 8092
cc 10 11 80
cc 10 7 91
cc 10 10 64
cc 10 5 0
cc 10 65 0
cc 10 68 0
cc 10 64 0
cc 10 91 0
echo "meter 3 2 48 8"
echo "key C 'major'"
echo "voice 11"
cc 11 0 0
cc 11 32 0
select 11 1 0 56
pitch_bend 11 8192
cc 11 11 80
cc 11 7 91
cc 11 10 64
cc 11 5 0
cc 11 65 0
cc 11 68 0
cc 11 64 0
cc 11 91 0
echo "meter 3 2 48 8"
echo "key C 'major'"
echo "voice 12"
cc 12 0 0
cc 12 32 0
select 12 1 0 56
pitch_bend 12 8092
cc 12 11 80
cc 12 7 91
cc 12 10 64
cc 12 5 0
cc 12 65 0
cc 12 68 0
cc 12 64 0
cc 12 91 0
echo "meter 3 2 48 8"
echo "key C 'major'"
echo "voice 13"
cc 13 0 0
cc 13 32 0
select 13 1 0 47
pitch_bend 13 8192
cc 13 11 80
cc 13 7 127
cc 13 10 64
cc 13 5 0
cc 13 65 0
cc 13 68 0
cc 13 64 0
cc 13 91 89
echo "meter 3 2 48 8"
echo "key C 'major'"
echo "voice 14"
cc 14 0 0
cc 14 32 0
select 14 1 0 40
pitch_bend 14 8192
cc 14 11 80
cc 14 7 91
cc 14 10 64
cc 14 5 0
cc 14 65 0
cc 14 68 0
cc 14 64 0
cc 14 91 32
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "voice 15"
cc 15 0 0
cc 15 32 0
select 15 1 0 40
pitch_bend 15 8092
cc 15 11 80
cc 15 7 91
cc 15 10 64
cc 15 5 0
cc 15 65 0
cc 15 68 0
cc 15 64 0
cc 15 91 32
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "voice 16"
cc 16 0 0
cc 16 32 0
select 16 1 0 40
pitch_bend 16 8292
cc 16 11 80
cc 16 7 91
cc 16 10 64
cc 16 5 0
cc 16 65 0
cc 16 68 0
cc 16 64 0
cc 16 91 32
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "measure 2 - $$ Page 40, Top, 2nd"
echo "voice 17"
cc 17 0 0
cc 17 32 0
select 17 1 0 40
pitch_bend 17 8192
cc 17 11 80
cc 17 7 91
cc 17 10 64
cc 17 5 0
cc 17 65 0
cc 17 68 0
cc 17 64 0
cc 17 91 32
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "measure 1 - $$ Page 40, Top, 1st"
echo "voice 18"
cc 18 0 0
cc 18 32 0
select 18 1 0 40
pitch_bend 18 8092
cc 18 11 80
cc 18 7 91
cc 18 10 64
cc 18 5 0
cc 18 65 0
cc 18 68 0
cc 18 64 0
cc 18 91 32
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "voice 19"
cc 19 0 0
cc 19 32 0
select 19 1 0 40
pitch_bend 19 8292
cc 19 11 80
cc 19 7 91
cc 19 10 64
cc 19 5 0
cc 19 65 0
cc 19 68 0
cc 19 64 0
cc 19 91 32
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "measure 2 - $$ Page 40, Top, 2nd"
echo "voice 20"
cc 20 0 0
cc 20 32 0
select 20 1 0 45
pitch_bend 20 8192
cc 20 11 80
cc 20 7 91
cc 20 10 64
cc 20 5 0
cc 20 65 0
cc 20 68 0
cc 20 64 0
cc 20 91 35
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "measure 1 - $$ Page 40, Top, 1st"
echo "voice 21"
cc 21 0 0
cc 21 32 0
select 21 1 0 45
pitch_bend 21 8092
cc 21 11 80
cc 21 7 91
cc 21 10 64
cc 21 5 0
cc 21 65 0
cc 21 68 0
cc 21 64 0
cc 21 91 35
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "voice 22"
cc 22 0 0
cc 22 32 0
select 22 1 0 45
pitch_bend 22 8192
cc 22 11 80
cc 22 7 91
cc 22 10 64
cc 22 5 0
cc 22 65 0
cc 22 68 0
cc 22 64 0
cc 22 91 35
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "voice 23"
cc 23 0 0
cc 23 32 0
select 23 1 0 43
pitch_bend 23 8192
cc 23 11 80
cc 23 7 91
cc 23 10 64
cc 23 5 0
cc 23 65 0
cc 23 68 0
cc 23 64 0
cc 23 91 55
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "voice 24"
cc 24 0 0
cc 24 32 0
select 24 1 0 0
pitch_bend 24 8192
cc 24 11 100
cc 24 7 100
cc 24 10 64
cc 24 5 0
cc 24 65 0
cc 24 68 0
cc 24 64 0
cc 24 91 0
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "measure 2 - $$ Page 40, Top, 2nd"
echo "voice 25"
cc 25 0 0
cc 25 32 0
select 25 1 0 0
pitch_bend 25 8192
cc 25 11 100
cc 25 7 100
cc 25 10 64
cc 25 5 0
cc 25 65 0
cc 25 68 0
cc 25 64 0
cc 25 91 0
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "measure 1 - $$ Page 40, Top, 1st"
echo "measure 2 - $$ Page 40, Top, 2nd"
echo "voice 26"
cc 26 0 0
cc 26 32 0
select 26 1 0 45
pitch_bend 26 8192
cc 26 11 100
cc 26 7 91
cc 26 10 64
cc 26 5 0
cc 26 65 0
cc 26 68 0
cc 26 64 0
cc 26 91 55
echo "meter 3 2 48 8"
echo "key D 'major'"
echo "measure 1 - $$ Page 40, Top, 1st"
echo "measure 2 - $$ Page 40, Top, 2nd"
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 74 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 0 10 64
cc 0 7 91
echo "bars 12"
cc 1 11 80
cc 1 10 64
cc 1 7 91
cc 2 11 80
cc 2 10 64
cc 2 7 91
cc 3 11 80
cc 3 10 64
cc 3 7 91
cc 4 11 80
cc 4 10 64
cc 4 7 91
cc 5 11 80
cc 5 10 64
cc 5 7 91
cc 6 11 80
cc 6 10 64
cc 6 7 100
cc 7 11 80
cc 7 10 64
cc 7 7 100
cc 8 11 80
cc 8 10 64
cc 8 7 91
cc 10 11 80
cc 10 10 64
cc 10 7 91
cc 11 11 80
cc 11 10 64
cc 11 7 91
cc 12 11 80
cc 12 10 64
cc 12 7 91
cc 13 11 80
cc 13 10 64
cc 13 7 127
cc 14 11 80
cc 14 10 64
cc 14 7 91
cc 15 11 80
cc 15 10 64
cc 15 7 91
cc 17 11 80
cc 17 10 64
cc 17 7 91
cc 18 11 80
cc 18 10 64
cc 18 7 91
cc 20 11 80
cc 20 10 64
cc 20 7 91
cc 22 11 80
cc 22 10 64
cc 22 7 91
cc 23 11 80
cc 23 10 64
cc 23 7 91
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 3 - $$ Page 40, Top, 3rd"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 74 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 4 - $$ Page 40, Top, 4th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 76 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 5 - $$ Page 40, Top, 5th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 76 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 6 - $$ Page 40, Top, 6th"
noteoff 14 83 0
noteon 8 66 120
noteon 10 59 120
cc 8 11 50
cc 10 11 50
sleep 200.0
noteoff 8 66 0
noteoff 10 59 0
noteon 8 68 120
noteon 10 64 120
sleep 200.0
noteoff 8 68 0
noteoff 10 64 0
noteon 8 69 120
noteon 10 66 120
sleep 200.0
echo "measure 7 - $$ Page 40, Top, 7th"
noteoff 8 69 0
noteoff 10 66 0
noteon 14 76 120
noteon 17 69 120
cc 17 11 50
sleep 200.0
noteoff 14 76 0
noteoff 17 69 0
noteon 14 78 120
noteon 17 74 120
sleep 200.0
noteoff 14 78 0
noteoff 17 74 0
noteon 14 79 120
noteon 17 76 120
sleep 200.0
echo "measure 8 - $$ Page 40, Top, 8th"
noteoff 14 79 0
noteoff 17 76 0
noteon 2 78 120
noteon 3 74 120
noteon 8 68 120
noteon 10 64 120
cc 2 11 50
cc 3 11 50
sleep 200.0
noteoff 2 78 0
noteoff 3 74 0
noteoff 8 68 0
noteoff 10 64 0
noteon 2 76 120
noteon 3 69 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 66 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
echo "measure 9 - $$ Page 40, Top, 9th"
noteoff 2 74 0
noteoff 3 66 0
noteoff 8 64 0
noteoff 10 56 0
noteon 0 86 120
noteon 1 78 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 90
cc 1 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 78 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 1 79 120
noteon 4 76 120
noteon 5 67 120
noteon 6 57 120
noteon 7 45 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 0 85 0
noteoff 1 79 0
noteoff 4 76 0
noteoff 5 67 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 0 86 120
noteon 1 78 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
echo "measure 10 - $$ Page 40, Top, 10th"
noteoff 0 86 0
noteoff 1 78 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 12 11 80
cc 13 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 11 - $$ Page 40, Top, 11th"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 12 - $$ Page 40, Top, 12th (last)"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 56 120
noteon 7 56 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 64 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 56 120
noteon 22 56 120
noteon 23 44 120
cc 14 11 80
sleep 200.0
noteoff 6 56 0
noteoff 7 56 0
noteoff 13 50 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 44 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 13 - $$ Page 40, Bottom, 1st"
noteoff 0 76 0
echo "page 40"
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 64 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 83 120
cc 14 11 50
sleep 200.0
noteoff 14 83 0
noteon 14 85 120
sleep 200.0
noteoff 14 85 0
noteon 14 86 120
sleep 200.0
echo "bars 12"
echo "measure 14 - $$ Page 40, Bottom, 2nd"
noteoff 14 86 0
noteon 2 73 120
noteon 3 69 120
noteon 17 61 120
noteon 20 57 120
cc 2 11 50
cc 3 11 50
cc 17 11 50
cc 20 11 50
sleep 200.0
noteoff 2 73 0
noteoff 3 69 0
noteoff 17 61 0
noteoff 20 57 0
noteon 2 74 120
noteon 3 71 120
noteon 17 62 120
noteon 20 59 120
sleep 200.0
noteoff 2 74 0
noteoff 3 71 0
noteoff 17 62 0
noteoff 20 59 0
noteon 2 76 120
noteon 3 73 120
noteon 17 64 120
noteon 20 61 120
sleep 200.0
echo "measure 15 - $$ Page 40, Bottom, 3rd"
noteoff 2 76 0
noteoff 3 73 0
noteoff 17 64 0
noteoff 20 61 0
noteon 14 83 120
noteon 17 76 120
sleep 200.0
noteoff 14 83 0
noteoff 17 76 0
noteon 14 85 120
noteon 17 81 120
sleep 200.0
noteoff 14 85 0
noteoff 17 81 0
noteon 14 86 120
noteon 17 83 120
sleep 200.0
echo "measure 16 - $$ Page 40, Bottom, 4th"
noteoff 14 86 0
noteoff 17 83 0
noteon 0 85 120
noteon 2 81 120
noteon 3 73 120
noteon 6 57 120
noteon 8 66 120
cc 0 11 50
cc 6 11 50
cc 8 11 50
sleep 200.0
noteoff 0 85 0
noteoff 2 81 0
noteoff 3 73 0
noteoff 6 57 0
noteoff 8 66 0
noteon 0 83 120
noteon 2 80 120
noteon 3 71 120
noteon 6 52 120
noteon 8 66 120
sleep 200.0
noteoff 0 83 0
noteoff 2 80 0
noteoff 3 71 0
noteoff 6 52 0
noteoff 8 66 0
noteon 0 81 120
noteon 2 81 120
noteon 3 73 120
noteon 6 57 120
noteon 8 66 120
sleep 200.0
echo "measure 1 - $$ Page 40, Top, 1st"
noteoff 0 81 0
noteoff 2 81 0
noteoff 3 73 0
noteoff 6 57 0
noteoff 8 66 0
noteon 0 88 120
noteon 1 85 120
noteon 4 73 120
noteon 5 69 120
noteon 6 57 120
noteon 7 45 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 81 120
noteon 17 73 120
noteon 18 64 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 0 11 90
cc 1 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 8 11 90
cc 10 11 90
cc 11 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 88 0
noteoff 1 85 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 81 0
noteoff 17 73 0
noteoff 18 64 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 88 120
noteon 1 83 120
noteon 4 71 120
noteon 5 68 120
noteon 6 52 120
noteon 7 40 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 80 120
noteon 17 74 120
noteon 18 64 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 0 88 0
noteoff 1 83 0
noteoff 4 71 0
noteoff 5 68 0
noteoff 6 52 0
noteoff 7 40 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 80 0
noteoff 17 74 0
noteoff 18 64 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 0 88 120
echo "bars 12"
echo "* tempo 100,2d"
echo "page 40"
noteon 1 85 120
noteon 4 73 120
noteon 5 69 120
noteon 6 57 120
noteon 7 45 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 81 120
noteon 17 73 120
noteon 18 64 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
echo "measure 2 - $$ Page 40, Top, 2nd"
noteoff 0 88 0
noteoff 1 85 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 81 0
noteoff 17 73 0
noteoff 18 64 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 74 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 13 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 3 - $$ Page 40, Top, 3rd"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 74 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 4 - $$ Page 40, Top, 4th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 76 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 5 - $$ Page 40, Top, 5th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 76 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 6 - $$ Page 40, Top, 6th"
noteoff 14 83 0
noteon 8 66 120
noteon 10 59 120
cc 8 11 50
cc 10 11 50
sleep 200.0
noteoff 8 66 0
noteoff 10 59 0
noteon 8 68 120
noteon 10 64 120
sleep 200.0
noteoff 8 68 0
noteoff 10 64 0
noteon 8 69 120
noteon 10 66 120
sleep 200.0
echo "measure 7 - $$ Page 40, Top, 7th"
noteoff 8 69 0
noteoff 10 66 0
noteon 14 76 120
noteon 17 69 120
cc 17 11 50
sleep 200.0
noteoff 14 76 0
noteoff 17 69 0
noteon 14 78 120
noteon 17 74 120
sleep 200.0
noteoff 14 78 0
noteoff 17 74 0
noteon 14 79 120
noteon 17 76 120
sleep 200.0
echo "measure 8 - $$ Page 40, Top, 8th"
noteoff 14 79 0
noteoff 17 76 0
noteon 2 78 120
noteon 3 74 120
noteon 8 68 120
noteon 10 64 120
cc 2 11 50
cc 3 11 50
sleep 200.0
noteoff 2 78 0
noteoff 3 74 0
noteoff 8 68 0
noteoff 10 64 0
noteon 2 76 120
noteon 3 69 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 66 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
echo "measure 9 - $$ Page 40, Top, 9th"
noteoff 2 74 0
noteoff 3 66 0
noteoff 8 64 0
noteoff 10 56 0
noteon 0 86 120
noteon 1 78 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 90
cc 1 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 78 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 1 79 120
noteon 4 76 120
noteon 5 67 120
noteon 6 57 120
noteon 7 45 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 0 85 0
noteoff 1 79 0
noteoff 4 76 0
noteoff 5 67 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 0 86 120
noteon 1 78 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
echo "measure 10 - $$ Page 40, Top, 10th"
noteoff 0 86 0
noteoff 1 78 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 12 11 80
cc 13 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 11 - $$ Page 40, Top, 11th"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 12 - $$ Page 40, Top, 12th (last)"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 56 120
noteon 7 56 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 64 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 56 120
noteon 22 56 120
noteon 23 44 120
cc 14 11 80
sleep 200.0
noteoff 6 56 0
noteoff 7 56 0
noteoff 13 50 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 44 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 13 - $$ Page 40, Bottom, 1st"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 64 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 83 120
cc 14 11 50
sleep 200.0
noteoff 14 83 0
noteon 14 85 120
sleep 200.0
noteoff 14 85 0
noteon 14 86 120
sleep 200.0
echo "measure 14 - $$ Page 40, Bottom, 2nd"
noteoff 14 86 0
noteon 2 73 120
noteon 3 69 120
noteon 17 61 120
noteon 20 57 120
cc 2 11 50
cc 3 11 50
cc 17 11 50
cc 20 11 50
sleep 200.0
noteoff 2 73 0
noteoff 3 69 0
noteoff 17 61 0
noteoff 20 57 0
noteon 2 74 120
noteon 3 71 120
noteon 17 62 120
noteon 20 59 120
sleep 200.0
noteoff 2 74 0
noteoff 3 71 0
noteoff 17 62 0
noteoff 20 59 0
noteon 2 76 120
noteon 3 73 120
noteon 17 64 120
noteon 20 61 120
sleep 200.0
echo "measure 15 - $$ Page 40, Bottom, 3rd"
noteoff 2 76 0
noteoff 3 73 0
noteoff 17 64 0
noteoff 20 61 0
noteon 14 83 120
noteon 17 76 120
sleep 200.0
noteoff 14 83 0
noteoff 17 76 0
noteon 14 85 120
noteon 17 81 120
sleep 200.0
noteoff 14 85 0
noteoff 17 81 0
noteon 14 86 120
noteon 17 83 120
sleep 200.0
echo "measure 16 - $$ Page 40, Bottom, 4th"
noteoff 14 86 0
noteoff 17 83 0
noteon 0 85 120
noteon 2 81 120
noteon 3 73 120
noteon 6 57 120
noteon 8 66 120
cc 0 11 50
cc 6 11 50
cc 8 11 50
sleep 200.0
noteoff 0 85 0
noteoff 2 81 0
noteoff 3 73 0
noteoff 6 57 0
noteoff 8 66 0
noteon 0 83 120
noteon 2 80 120
noteon 3 71 120
noteon 6 52 120
noteon 8 66 120
sleep 200.0
noteoff 0 83 0
noteoff 2 80 0
noteoff 3 71 0
noteoff 6 52 0
noteoff 8 66 0
noteon 0 81 120
noteon 2 81 120
noteon 3 73 120
noteon 6 57 120
noteon 8 66 120
sleep 200.0
echo "measure 17 - $$ Page 40, Bottom, 5th"
noteoff 0 81 0
noteoff 2 81 0
noteoff 3 73 0
noteoff 6 57 0
noteoff 8 66 0
noteon 0 88 120
noteon 1 85 120
noteon 4 73 120
noteon 5 69 120
noteon 6 57 120
noteon 7 45 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 81 120
noteon 17 73 120
noteon 18 64 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 0 11 90
cc 1 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 8 11 90
cc 10 11 90
cc 11 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 88 0
noteoff 1 85 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 81 0
noteoff 17 73 0
noteoff 18 64 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 88 120
noteon 1 83 120
noteon 4 71 120
noteon 5 68 120
noteon 6 52 120
noteon 7 40 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 80 120
noteon 17 74 120
noteon 18 64 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 0 88 0
noteoff 1 83 0
noteoff 4 71 0
noteoff 5 68 0
noteoff 6 52 0
noteoff 7 40 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 80 0
noteoff 17 74 0
noteoff 18 64 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 0 88 120
noteon 1 85 120
noteon 4 73 120
noteon 5 69 120
noteon 6 57 120
noteon 7 45 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 81 120
noteon 17 73 120
noteon 18 64 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
echo "measure 18 - $$ Page 40, Bottom, 6th"
noteoff 0 88 0
noteoff 1 85 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 81 0
noteoff 17 73 0
noteoff 18 64 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 14 69 120
noteon 17 61 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
cc 14 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 200.0
noteoff 17 61 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 17 62 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
noteoff 17 62 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 17 64 120
noteon 20 61 120
noteon 22 61 120
noteon 23 49 120
sleep 200.0
echo "measure 19 - $$ Page 40, Bottom, 7th"
noteoff 14 69 0
noteoff 17 64 0
noteoff 20 61 0
noteoff 22 61 0
noteoff 23 49 0
noteon 14 77 120
noteon 17 65 120
noteon 20 62 120
noteon 22 62 120
noteon 23 50 120
sleep 200.0
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 62 0
noteoff 22 62 0
noteoff 23 50 0
noteon 14 76 120
sleep 200.0
noteoff 14 76 0
noteon 14 77 120
sleep 200.0
echo "measure 20 - $$ Page 40, Bottom, 8th"
noteoff 14 77 0
noteon 14 65 120
noteon 17 57 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 17 57 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 17 58 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
sleep 200.0
noteoff 17 58 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 17 60 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
echo "measure 21 - $$ Page 40, Bottom, 9th"
noteoff 14 65 0
noteoff 17 60 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 14 74 120
noteon 17 62 120
noteon 20 58 120
noteon 22 58 120
noteon 23 46 120
sleep 200.0
noteoff 14 74 0
noteoff 17 62 0
noteoff 20 58 0
noteoff 22 58 0
noteoff 23 46 0
noteon 14 73 120
sleep 200.0
noteoff 14 73 0
noteon 14 74 120
sleep 200.0
echo "measure 22 - $$ Page 40, Bottom, 10th"
noteoff 14 74 0
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 18 11 50
sleep 200.0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 77 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 400.0
echo "measure 23 - $$ Page 40, Bottom, 11th"
cc 14 11 50
cc 17 11 50
cc 18 11 50
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 400.0
noteoff 14 77 0
noteon 14 74 120
cc 14 68 127
sleep 100.0
noteon 14 70 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 24 - $$ Page 40, Bottom, 12th (last)"
noteoff 14 70 0
noteoff 17 62 0
noteoff 18 58 0
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
cc 14 68 127
sleep 100.0
noteon 14 72 120
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 75 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 72 0
sleep 100.0
noteon 14 74 120
noteoff 14 75 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 72 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 74 0
sleep 100.0
noteon 14 70 120
noteoff 14 72 0
sleep 100.0
echo "measure 25 - $$ Page 41, Top, 1st"
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 70 0
sleep 100.0
noteon 14 67 120
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 65 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 67 0
sleep 100.0
noteon 14 63 120
noteoff 14 65 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 81 120
noteon 6 57 120
noteon 14 62 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 0 68 127
cc 0 11 50
echo "page 41"
cc 6 68 127
cc 6 11 50
noteoff 14 63 0
sleep 100.0
noteon 14 60 120
noteoff 14 62 0
cc 14 68 0
sleep 100.0
echo "measure 26 - $$ Page 41, Top, 2nd"
noteoff 14 60 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 82 120
noteon 2 74 120
noteon 3 70 120
noteon 6 58 120
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 81 0
noteoff 6 57 0
cc 0 68 0
cc 6 68 0
sleep 200.0
noteoff 0 82 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 89 120
noteon 2 74 120
noteon 3 70 120
noteon 6 65 120
noteon 14 77 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 0 11 80
echo "bars 10"
cc 2 11 80
cc 3 11 80
cc 6 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 400.0
cc 0 11 50
cc 2 11 50
echo "measure 27 - $$ Page 41, Top, 3rd"
cc 3 11 50
cc 6 11 50
cc 14 11 50
cc 17 11 50
cc 18 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 400.0
noteoff 0 89 0
noteoff 6 65 0
noteoff 14 77 0
noteon 0 86 120
noteon 6 62 120
noteon 14 74 120
cc 0 68 127
cc 6 68 127
cc 14 68 127
sleep 100.0
noteon 0 82 120
noteon 6 58 120
noteon 14 70 120
noteoff 0 86 0
noteoff 6 62 0
noteoff 14 74 0
cc 0 68 0
cc 6 68 0
cc 14 68 0
sleep 100.0
echo "measure 28 - $$ Page 41, Top, 4th"
noteoff 0 82 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 70 0
noteoff 17 62 0
noteoff 18 58 0
noteon 0 81 120
noteon 2 75 120
noteon 3 72 120
noteon 6 57 120
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
cc 0 68 127
cc 14 68 127
sleep 100.0
noteon 0 84 120
noteon 14 72 120
noteoff 0 81 0
noteoff 14 69 0
sleep 100.0
noteoff 6 57 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 87 120
noteon 14 75 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 84 0
noteoff 14 72 0
sleep 100.0
noteon 0 86 120
noteon 14 74 120
noteoff 0 87 0
noteoff 14 75 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 84 120
noteon 14 72 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 86 0
noteoff 14 74 0
sleep 100.0
noteon 0 82 120
noteon 14 70 120
noteoff 0 84 0
noteoff 14 72 0
sleep 100.0
echo "measure 29 - $$ Page 41, Top, 5th"
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 81 120
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 82 0
noteoff 14 70 0
sleep 100.0
noteon 0 79 120
noteon 14 67 120
noteoff 0 81 0
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 77 120
noteon 6 53 120
noteon 14 65 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 79 0
noteoff 14 67 0
sleep 100.0
noteon 0 75 120
noteon 14 63 120
noteoff 0 77 0
noteoff 14 65 0
sleep 100.0
noteoff 6 53 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 74 120
noteon 6 53 120
noteon 14 62 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 75 0
noteoff 14 63 0
sleep 100.0
noteon 0 72 120
noteon 14 60 120
noteoff 0 74 0
noteoff 14 62 0
cc 0 68 0
sleep 100.0
echo "measure 30 - $$ Page 41, Top, 6th"
noteoff 0 72 0
noteoff 2 75 0
noteoff 3 72 0
noteoff 6 53 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 70 120
noteon 2 74 120
noteon 3 70 120
noteon 6 58 120
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 60 0
cc 14 68 0
sleep 200.0
noteoff 0 70 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 74 120
noteon 17 62 120
noteon 20 58 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 58 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 31 - $$ Page 41, Top, 7th"
noteoff 14 76 0
noteoff 17 65 0
noteoff 20 62 0
noteon 6 57 120
noteon 14 77 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 6 57 0
noteoff 14 77 0
noteoff 22 45 0
noteoff 23 33 0
noteon 14 74 120
noteon 17 62 120
noteon 20 57 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 57 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 32 - $$ Page 41, Top, 8th"
noteoff 14 76 0
noteoff 17 65 0
noteoff 20 62 0
noteon 6 56 120
noteon 14 77 120
noteon 22 44 120
noteon 23 32 120
sleep 200.0
noteoff 6 56 0
noteoff 14 77 0
noteoff 22 44 0
noteoff 23 32 0
noteon 14 74 120
noteon 17 62 120
noteon 20 59 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 59 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
sleep 100.0
echo "measure 33 - $$ Page 41, Top, 9th"
noteoff 17 65 0
noteoff 20 62 0
noteon 6 56 120
noteon 14 77 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
noteon 22 56 120
noteon 23 44 120
cc 21 11 50
cc 21 10 64
cc 21 7 91
noteoff 14 76 0
cc 14 68 0
sleep 200.0
noteoff 6 56 0
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteoff 22 56 0
noteoff 23 44 0
noteon 14 79 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
cc 14 68 127
sleep 100.0
noteon 14 77 120
noteoff 14 79 0
sleep 100.0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteon 14 76 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
noteoff 14 77 0
sleep 100.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 100.0
echo "measure 34 - $$ Page 41, Top, 10th (last)"
noteoff 14 74 0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteon 6 57 120
noteon 14 73 120
noteon 17 64 120
noteon 20 61 120
noteon 21 57 120
noteon 22 57 120
noteon 23 33 120
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 21 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 6 57 0
noteoff 14 73 0
noteoff 17 64 0
noteoff 20 61 0
noteoff 21 57 0
noteoff 22 57 0
noteon 14 74 120
cc 14 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 71 120
noteon 20 49 120
noteon 22 49 120
cc 20 68 127
cc 22 68 127
noteoff 14 73 0
sleep 100.0
noteon 14 69 120
noteoff 14 71 0
cc 14 68 0
sleep 100.0
echo "page 41"
echo "measure 35 - $$ Page 41, Bottom, 1st"
noteoff 14 69 0
noteon 14 77 120
noteon 20 50 120
noteon 22 50 120
noteoff 20 49 0
noteoff 22 49 0
sleep 200.0
noteoff 14 77 0
noteon 14 79 120
noteon 20 53 120
noteon 22 53 120
cc 14 68 127
noteoff 20 50 0
noteoff 22 50 0
sleep 100.0
noteon 14 77 120
noteoff 14 79 0
sleep 100.0
noteon 14 76 120
noteon 20 56 120
noteon 22 56 120
noteoff 14 77 0
noteoff 20 53 0
noteoff 22 53 0
cc 20 68 0
cc 22 68 0
sleep 100.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 100.0
echo "bars 12"
echo "measure 36 - $$ Page 41, Bottom, 2nd"
noteoff 14 74 0
noteoff 20 56 0
noteoff 22 56 0
noteon 14 73 120
noteon 20 57 120
noteon 22 57 120
sleep 100.0
noteoff 20 57 0
noteoff 22 57 0
sleep 100.0
noteoff 14 73 0
noteon 14 74 120
cc 14 68 127
sleep 25.0
cc 14 11 41
sleep 50.0
cc 14 11 42
sleep 25.0
noteon 14 73 120
noteoff 14 74 0
sleep 25.0
cc 14 11 43
sleep 50.0
cc 14 11 44
sleep 25.0
noteon 14 71 120
noteon 17 61 120
noteon 20 49 120
noteon 22 49 120
cc 17 68 127
cc 20 68 127
cc 22 68 127
noteoff 14 73 0
sleep 25.0
cc 14 11 45
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 50.0
cc 14 11 46
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 25.0
noteon 14 69 120
noteoff 14 71 0
cc 14 68 0
sleep 25.0
cc 14 11 47
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 50.0
cc 14 11 48
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 25.0
echo "measure 37 - $$ Page 41, Bottom, 3rd"
noteoff 14 69 0
noteon 14 77 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteoff 17 61 0
noteoff 20 49 0
noteoff 22 49 0
sleep 25.0
cc 14 11 49
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 50.0
cc 14 11 50
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 25.0
noteoff 14 77 0
sleep 25.0
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 50.0
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 25.0
noteon 14 79 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
cc 14 68 127
cc 14 11 52
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 25.0
cc 14 11 53
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 50.0
cc 14 11 54
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 25.0
noteon 14 77 120
noteoff 14 79 0
sleep 25.0
cc 14 11 55
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 50.0
cc 14 11 56
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 25.0
noteon 14 76 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 25.0
cc 14 11 57
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 50.0
cc 14 11 58
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 25.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 25.0
cc 14 11 59
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 50.0
cc 14 11 60
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 25.0
echo "measure 38 - $$ Page 41, Bottom, 4th"
noteoff 14 74 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteon 14 73 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
sleep 25.0
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 7.916
cc 14 11 61
sleep 42.083
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 24.583
cc 14 11 62
sleep 0.416
noteoff 14 73 0
noteoff 17 69 0
noteoff 20 57 0
noteoff 22 57 0
sleep 100.0
noteoff 23 33 0
noteon 14 76 120
noteon 17 57 120
cc 14 68 127
cc 14 11 63
cc 17 68 127
cc 17 11 60
sleep 25.0
cc 14 11 64
cc 17 11 61
sleep 49.583
cc 14 11 65
sleep 0.416
cc 17 11 62
sleep 25.0
noteon 14 74 120
noteoff 14 76 0
sleep 24.583
cc 14 11 66
sleep 0.416
cc 17 11 63
sleep 49.583
cc 14 11 67
sleep 0.416
cc 17 11 64
sleep 25.0
noteon 14 73 120
noteon 17 58 120
noteoff 14 74 0
noteoff 17 57 0
sleep 24.583
cc 14 11 68
cc 17 11 65
sleep 50.0
cc 14 11 69
cc 17 11 66
sleep 25.416
noteon 14 71 120
noteoff 14 73 0
sleep 24.583
cc 14 11 70
cc 17 11 67
sleep 50.0
cc 14 11 71
cc 17 11 68
sleep 25.416
echo "measure 39 - $$ Page 41, Bottom, 5th"
noteon 14 69 120
noteon 17 59 120
noteoff 14 71 0
noteoff 17 58 0
sleep 24.583
cc 14 11 72
cc 17 11 69
sleep 50.0
cc 14 11 73
cc 17 11 70
sleep 25.416
noteon 14 68 120
noteoff 14 69 0
cc 14 68 0
sleep 24.583
cc 17 11 71
sleep 25.416
cc 14 11 74
sleep 24.583
cc 17 11 72
sleep 25.416
noteoff 14 68 0
noteon 14 69 120
noteon 17 60 120
noteoff 17 59 0
sleep 24.583
cc 14 11 75
cc 17 11 73
sleep 25.416
noteoff 14 69 0
sleep 24.583
cc 17 11 74
sleep 25.416
noteon 14 67 120
cc 14 11 76
sleep 24.583
cc 17 11 75
sleep 25.416
noteoff 14 67 0
sleep 24.583
cc 17 11 76
sleep 25.416
noteon 14 66 120
noteon 17 61 120
cc 14 11 77
noteoff 17 60 0
cc 17 68 0
sleep 24.583
cc 14 11 78
cc 17 11 77
sleep 25.416
noteoff 14 66 0
sleep 24.583
cc 17 11 78
sleep 25.416
noteon 14 64 120
cc 14 11 79
sleep 24.583
cc 17 11 79
sleep 25.416
noteoff 14 64 0
sleep 24.583
cc 17 11 80
sleep 25.416
echo "measure 40 - $$ Page 41, Bottom, 6th"
noteoff 17 61 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 13 11 80
cc 14 11 80
cc 15 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 41 - $$ Page 41, Bottom, 7th"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 42 - $$ Page 41, Bottom, 8th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 67 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 67 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 71 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 43 - $$ Page 41, Bottom, 9th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 71 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 44 - $$ Page 41, Bottom, 10th"
noteoff 14 83 0
noteon 8 66 120
noteon 10 59 120
cc 8 11 50
cc 10 11 50
sleep 200.0
noteoff 8 66 0
noteoff 10 59 0
noteon 8 68 120
noteon 10 64 120
sleep 200.0
noteoff 8 68 0
noteoff 10 64 0
noteon 8 69 120
noteon 10 66 120
sleep 200.0
echo "measure 45 - $$ Page 41, Bottom, 11th"
noteoff 8 69 0
noteoff 10 66 0
noteon 14 76 120
noteon 17 69 120
cc 17 11 50
sleep 200.0
noteoff 14 76 0
noteoff 17 69 0
noteon 14 78 120
noteon 17 74 120
sleep 200.0
noteoff 14 78 0
noteoff 17 74 0
noteon 14 79 120
noteon 17 76 120
sleep 200.0
echo "measure 46 - $$ Page 41, Bottom, 12th (last)"
noteoff 14 79 0
noteoff 17 76 0
noteon 2 78 120
noteon 3 74 120
noteon 8 68 120
noteon 10 64 120
cc 2 11 50
cc 3 11 50
sleep 200.0
noteoff 2 78 0
noteoff 3 74 0
noteoff 8 68 0
noteoff 10 64 0
noteon 2 76 120
noteon 3 69 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 66 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
echo "measure 47 - $$ Page 42, Top, 1st"
noteoff 2 74 0
noteoff 3 66 0
noteoff 8 64 0
noteoff 10 56 0
noteon 0 86 120
noteon 1 78 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 90
cc 1 11 90
cc 2 11 90
cc 3 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 1 76 120
noteon 2 79 120
noteon 3 76 120
noteon 4 76 120
noteon 5 67 120
noteon 6 57 120
noteon 7 45 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 0 85 0
noteoff 1 76 0
noteoff 2 79 0
noteoff 3 76 0
noteoff 4 76 0
noteoff 5 67 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 0 86 120
echo "page 42"
noteon 1 78 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
echo "measure 48 - $$ Page 42, Top, 2nd"
noteoff 0 86 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
echo "bars 14"
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 12 11 80
cc 13 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 49 - $$ Page 42, Top, 3rd"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 50 - $$ Page 42, Top, 4th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 51 - $$ Page 42, Top, 5th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 52 - $$ Page 42, Top, 6th"
noteoff 14 83 0
noteon 14 67 120
sleep 200.0
noteoff 14 67 0
noteon 14 69 120
sleep 200.0
noteoff 14 69 0
noteon 14 71 120
sleep 200.0
echo "measure 53 - $$ Page 42, Top, 7th"
noteoff 14 71 0
noteon 17 64 120
cc 17 11 50
sleep 200.0
noteoff 17 64 0
noteon 17 66 120
sleep 200.0
noteoff 17 66 0
noteon 17 67 120
sleep 200.0
echo "measure 54 - $$ Page 42, Top, 8th"
noteoff 17 67 0
noteon 14 67 120
sleep 200.0
noteoff 14 67 0
noteon 14 69 120
sleep 200.0
noteoff 14 69 0
noteon 14 70 120
sleep 200.0
echo "measure 55 - $$ Page 42, Top, 9th"
noteoff 14 70 0
noteon 17 64 120
sleep 200.0
noteoff 17 64 0
noteon 17 66 120
sleep 200.0
noteoff 17 66 0
noteon 17 67 120
sleep 200.0
echo "measure 56 - $$ Page 42, Top, 10th"
noteoff 17 67 0
noteon 14 67 120
sleep 100.0
noteoff 14 67 0
sleep 100.0
noteon 14 69 120
cc 14 11 49
sleep 100.0
noteoff 14 69 0
sleep 100.0
noteon 14 70 120
cc 14 11 48
sleep 100.0
noteoff 14 70 0
sleep 100.0
echo "measure 57 - $$ Page 42, Top, 11th"
noteon 14 67 120
noteon 17 64 120
cc 14 11 47
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 46
cc 17 11 49
sleep 100.0
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 14 11 45
cc 17 11 48
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 58 - $$ Page 42, Top, 12th"
noteon 14 67 120
noteon 17 64 120
cc 14 11 44
cc 17 11 47
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 43
cc 17 11 46
sleep 100.0
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 14 11 42
cc 17 11 45
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 59 - $$ Page 42, Top, 13th"
noteon 14 67 120
noteon 17 64 120
cc 17 11 44
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 41
cc 17 11 43
sleep 49.583
cc 17 11 42
sleep 50.416
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 17 11 41
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 60 - $$ Page 42, Top, 14th (last)"
noteon 2 72 120
noteon 6 60 120
noteon 14 67 120
noteon 17 64 120
noteon 20 60 120
noteon 22 60 120
noteon 23 48 120
cc 2 11 40
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 14 67 0
noteoff 17 64 0
noteoff 20 60 0
noteoff 22 60 0
noteoff 23 48 0
noteon 14 69 120
noteon 17 65 120
sleep 200.0
noteoff 14 69 0
noteoff 17 65 0
noteon 14 70 120
noteon 17 67 120
sleep 200.0
echo "page 42"
echo "measure 61 - $$ Page 42, Bottom, 1st"
noteoff 2 72 0
noteoff 6 60 0
noteoff 14 70 0
noteoff 17 67 0
noteon 2 77 120
noteon 6 65 120
noteon 14 69 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 69 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 2 76 120
noteon 6 64 120
sleep 200.0
noteoff 2 76 0
noteoff 6 64 0
noteon 2 77 120
noteon 6 65 120
sleep 200.0
echo "bars 13"
echo "measure 62 - $$ Page 42, Bottom, 2nd"
noteoff 2 77 0
noteoff 6 65 0
noteon 2 69 120
noteon 6 57 120
noteon 14 64 120
noteon 17 61 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 14 64 0
noteoff 17 61 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 14 65 120
noteon 17 62 120
sleep 200.0
noteoff 6 57 0
noteoff 14 65 0
noteoff 17 62 0
noteon 14 67 120
noteon 17 64 120
sleep 200.0
echo "measure 63 - $$ Page 42, Bottom, 3rd"
noteoff 2 69 0
noteoff 14 67 0
noteoff 17 64 0
noteon 2 74 120
noteon 6 62 120
noteon 14 65 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 65 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
noteoff 23 38 0
noteon 2 73 120
noteon 6 61 120
sleep 200.0
noteoff 2 73 0
noteoff 6 61 0
noteon 2 74 120
noteon 6 62 120
sleep 200.0
echo "measure 64 - $$ Page 42, Bottom, 4th"
noteoff 2 74 0
noteoff 6 62 0
noteon 4 74 120
noteon 5 62 120
noteon 6 52 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 17 11 50
cc 20 68 127
cc 20 11 50
cc 22 68 127
cc 22 11 50
cc 23 11 50
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 40.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 40.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 40.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 40.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
noteon 6 52 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 52 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 40.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 40.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 40.0
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 40.0
cc 6 11 60
cc 17 11 60
cc 20 11 60
cc 22 11 60
sleep 20.0
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 52 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 9.583
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 20.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 20.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 20.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 20.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 20.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 20.0
cc 6 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 10.416
echo "measure 65 - $$ Page 42, Bottom, 5th"
noteoff 7 46 0
noteon 6 46 120
noteon 7 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
cc 7 68 127
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
cc 6 68 0
sleep 200.0
noteon 7 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 7 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
cc 7 11 65
sleep 19.583
cc 7 11 66
sleep 40.0
cc 7 11 67
sleep 40.0
cc 7 11 68
sleep 40.0
cc 7 11 69
sleep 40.0
cc 7 11 70
sleep 20.416
noteon 7 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 7 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 7 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 10.0
cc 7 11 71
sleep 20.0
cc 7 11 72
sleep 20.0
cc 7 11 73
sleep 20.0
cc 7 11 74
sleep 20.0
cc 7 11 75
sleep 20.0
cc 7 11 76
sleep 20.0
cc 7 11 77
sleep 20.0
cc 7 11 78
sleep 20.0
cc 7 11 79
sleep 20.0
cc 7 11 80
sleep 10.0
echo "measure 66 - $$ Page 42, Bottom, 6th"
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 46 0
noteoff 7 56 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 57 120
noteon 12 69 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 68 127
cc 4 11 80
cc 5 68 127
cc 5 11 80
cc 6 11 80
cc 8 11 80
cc 10 11 80
cc 14 11 80
cc 15 68 127
cc 15 11 80
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 81 0
noteoff 17 69 0
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "measure 67 - $$ Page 42, Bottom, 7th"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 57 0
noteoff 12 69 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
noteoff 4 73 0
noteoff 5 61 0
cc 4 68 0
cc 5 68 0
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 50 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 14 76 120
noteon 17 64 120
sleep 200.0
noteoff 14 76 0
noteoff 17 64 0
noteon 14 74 120
noteon 17 62 120
sleep 200.0
echo "measure 68 - $$ Page 42, Bottom, 8th"
noteoff 14 74 0
noteoff 17 62 0
noteon 2 72 120
noteon 6 60 120
noteon 14 72 120
noteon 17 67 120
noteon 20 64 120
noteon 22 60 120
noteon 23 48 120
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 17 67 0
noteoff 20 64 0
noteoff 22 60 0
noteoff 23 48 0
noteon 17 69 120
noteon 20 65 120
sleep 200.0
noteoff 17 69 0
noteoff 20 65 0
noteon 17 70 120
noteon 20 67 120
sleep 200.0
echo "measure 69 - $$ Page 42, Bottom, 9th"
noteoff 2 72 0
noteoff 6 60 0
noteoff 14 72 0
noteoff 17 70 0
noteoff 20 67 0
noteon 2 77 120
noteon 6 65 120
noteon 14 77 120
noteon 17 69 120
noteon 20 65 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 77 0
noteoff 17 69 0
noteoff 20 65 0
noteoff 22 53 0
noteoff 23 41 0
noteon 2 76 120
noteon 6 64 120
noteon 14 76 120
sleep 200.0
noteoff 2 76 0
noteoff 6 64 0
noteoff 14 76 0
noteon 2 77 120
noteon 6 65 120
noteon 14 77 120
sleep 200.0
echo "measure 70 - $$ Page 42, Bottom, 10th"
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 77 0
noteon 0 81 120
noteon 2 69 120
noteon 6 57 120
noteon 14 69 120
noteon 17 64 120
noteon 20 61 120
noteon 22 57 120
noteon 23 45 120
cc 0 11 50
sleep 200.0
noteoff 17 64 0
noteoff 20 61 0
noteoff 22 57 0
noteoff 23 45 0
noteon 17 65 120
noteon 20 62 120
sleep 200.0
noteoff 17 65 0
noteoff 20 62 0
noteon 17 67 120
noteon 20 64 120
sleep 200.0
echo "measure 71 - $$ Page 42, Bottom, 11th"
noteoff 0 81 0
noteoff 2 69 0
noteoff 6 57 0
noteoff 14 69 0
noteoff 17 67 0
noteoff 20 64 0
noteon 0 86 120
noteon 2 74 120
noteon 6 62 120
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
noteoff 0 86 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 74 0
noteoff 17 65 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 2 73 120
noteon 6 61 120
noteon 14 73 120
sleep 200.0
noteoff 0 85 0
noteoff 2 73 0
noteoff 6 61 0
noteoff 14 73 0
noteon 0 86 120
noteon 2 74 120
noteon 6 62 120
noteon 14 74 120
sleep 200.0
echo "measure 72 - $$ Page 42, Bottom, 12th"
noteoff 0 86 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 74 0
noteon 4 74 120
noteon 5 62 120
noteon 6 52 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 20 68 127
cc 22 68 127
sleep 14.166
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 5.833
cc 6 11 51
sleep 22.5
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 17.5
cc 6 11 52
sleep 11.25
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 28.333
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 0.416
cc 6 11 53
sleep 28.333
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 11.666
cc 6 11 54
sleep 17.083
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 22.916
cc 6 11 55
sleep 5.416
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 14.583
noteon 6 54 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 52 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 14.166
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 5.833
cc 6 11 56
sleep 22.5
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 17.5
cc 6 11 57
sleep 11.25
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 28.333
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 0.416
cc 6 11 58
sleep 28.333
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 11.666
cc 6 11 59
sleep 17.083
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 22.916
cc 6 11 60
sleep 5.416
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 14.583
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 54 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 7.083
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 2.5
cc 6 11 59
sleep 11.666
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 8.333
cc 6 11 58
sleep 5.833
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 14.166
cc 6 11 57
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 14.583
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 5.416
cc 6 11 56
sleep 8.75
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 11.25
cc 6 11 55
sleep 2.916
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 14.583
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 2.5
cc 6 11 54
sleep 11.666
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 8.333
cc 6 11 53
sleep 5.833
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 14.166
cc 6 11 52
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 14.583
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 5.416
cc 6 11 51
sleep 8.75
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 11.25
cc 6 11 50
sleep 2.916
cc 17 11 40
cc 20 11 40
cc 22 11 40
sleep 7.5
echo "measure 73 - $$ Page 42, Bottom, 13th (last)"
noteon 6 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
sleep 200.0
noteon 6 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 6 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
sleep 200.0
noteon 6 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 6 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 200.0
echo "page 43"
echo "measure 74 - $$ Page 43, Top, 1st"
noteoff 4 74 0
noteoff 5 62 0
noteoff 7 46 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 68 127
cc 4 11 80
cc 5 68 127
cc 5 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 14 11 80
cc 15 68 127
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
noteoff 6 56 0
cc 6 68 0
cc 6 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 81 0
noteoff 17 69 0
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "bars 12"
echo "measure 75 - $$ Page 43, Top, 2nd"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 62 120
noteon 22 62 120
noteon 23 50 120
noteoff 4 73 0
noteoff 5 61 0
cc 4 68 0
cc 5 68 0
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 62 0
noteoff 22 62 0
noteoff 23 50 0
noteon 14 76 120
noteon 17 64 120
sleep 200.0
noteoff 14 76 0
noteoff 17 64 0
noteon 14 74 120
noteon 17 62 120
sleep 200.0
echo "measure 76 - $$ Page 43, Top, 3rd"
noteoff 14 74 0
noteoff 17 62 0
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 17 11 50
cc 20 68 127
cc 20 11 50
cc 22 68 127
cc 22 11 50
cc 23 11 50
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 40.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 40.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 40.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 40.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
noteon 6 52 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 50 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 40.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 40.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 40.0
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 40.0
cc 6 11 60
cc 17 11 60
cc 20 11 60
cc 22 11 60
sleep 20.0
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 52 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 9.583
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 20.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 20.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 20.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 20.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 20.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 20.0
cc 6 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 10.416
echo "measure 77 - $$ Page 43, Top, 4th"
noteon 6 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
sleep 200.0
noteon 6 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 6 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
sleep 200.0
noteon 6 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 6 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 6 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 200.0
echo "measure 78 - $$ Page 43, Top, 5th"
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 56 0
noteoff 7 46 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 90
cc 12 11 90
cc 14 11 90
cc 15 68 127
cc 15 11 90
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 6 11 50
cc 7 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
noteoff 15 81 0
noteoff 17 69 0
cc 15 11 80
cc 17 11 50
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "measure 79 - $$ Page 43, Top, 6th"
noteoff 4 73 0
noteoff 5 61 0
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
cc 4 11 50
cc 5 11 50
cc 8 11 50
cc 10 11 50
cc 11 11 80
cc 12 11 80
cc 14 11 80
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
noteon 8 68 120
noteon 10 56 120
noteon 14 74 120
noteon 17 62 120
sleep 200.0
noteoff 8 68 0
noteoff 10 56 0
noteoff 14 74 0
noteoff 17 62 0
noteon 8 64 120
noteon 10 52 120
noteon 14 78 120
noteon 17 66 120
sleep 200.0
echo "measure 80 - $$ Page 43, Top, 7th"
noteoff 8 64 0
noteoff 10 52 0
noteoff 14 78 0
noteoff 17 66 0
noteon 0 85 120
noteon 4 73 120
noteon 6 61 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 81 120
noteon 15 62 120
noteon 17 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
cc 0 11 80
cc 4 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 90
cc 12 11 90
cc 14 68 127
cc 14 11 90
cc 15 11 90
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 7 57 120
noteon 14 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 7 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
noteoff 14 81 0
noteoff 17 69 0
cc 14 11 80
cc 17 11 50
sleep 200.0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 7 57 120
noteon 14 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 14 80 0
noteoff 17 68 0
cc 14 68 0
cc 17 68 0
sleep 200.0
echo "measure 81 - $$ Page 43, Top, 8th"
noteoff 0 85 0
noteoff 4 73 0
noteoff 6 61 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 79 0
noteoff 15 62 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 4 74 120
noteon 6 62 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
cc 0 11 50
cc 4 11 50
cc 6 11 50
cc 8 11 50
cc 10 11 50
cc 11 11 80
cc 12 11 80
sleep 200.0
noteoff 0 86 0
noteoff 4 74 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
noteon 8 68 120
noteon 10 56 120
noteon 14 74 120
noteon 17 62 120
sleep 200.0
noteoff 8 68 0
noteoff 10 56 0
noteoff 14 74 0
noteoff 17 62 0
noteon 8 64 120
noteon 10 52 120
noteon 14 78 120
noteon 17 66 120
sleep 200.0
echo "measure 82 - $$ Page 43, Top, 9th"
noteoff 8 64 0
noteoff 10 52 0
noteoff 14 78 0
noteoff 17 66 0
noteon 0 85 120
noteon 1 81 120
noteon 2 81 120
noteon 3 69 120
noteon 4 73 120
noteon 5 69 120
noteon 6 61 120
noteon 7 57 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 61 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
cc 0 11 90
cc 1 11 90
cc 2 11 90
cc 3 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 8 11 90
cc 10 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 68 127
cc 17 11 90
cc 20 68 127
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 13 45 0
noteoff 22 45 0
noteoff 23 33 0
noteon 13 45 120
noteon 17 64 120
noteon 20 61 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 61 0
noteoff 20 57 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 13 45 120
noteon 17 69 120
noteon 20 64 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 64 0
noteoff 20 61 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
cc 0 68 127
cc 1 68 127
cc 2 68 127
cc 3 68 127
cc 4 68 127
cc 5 68 127
cc 6 68 127
cc 7 68 127
echo "measure 83 - $$ Page 43, Top, 10th"
cc 15 68 127
noteoff 12 57 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 12 57 120
noteon 13 45 120
noteon 17 73 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 69 0
noteoff 20 64 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 8 59 0
noteoff 10 47 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 1 83 120
noteon 2 80 120
noteon 3 68 120
noteon 4 74 120
noteon 5 71 120
noteon 6 62 120
noteon 7 59 120
noteon 8 66 120
noteon 10 59 120
noteon 13 45 120
noteon 15 80 120
noteon 17 74 120
noteon 20 71 120
noteon 22 57 120
noteon 23 45 120
noteoff 0 85 0
noteoff 1 81 0
noteoff 2 81 0
noteoff 3 69 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 61 0
noteoff 7 57 0
noteoff 15 81 0
noteoff 17 73 0
noteoff 20 69 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 8 66 0
noteoff 10 59 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 88 120
noteon 1 85 120
noteon 2 79 120
noteon 3 67 120
noteon 4 76 120
noteon 5 73 120
noteon 6 64 120
noteon 7 61 120
noteon 8 66 120
noteon 10 59 120
noteon 13 45 120
noteon 15 79 120
noteon 17 76 120
noteon 20 73 120
noteon 22 57 120
noteon 23 45 120
noteoff 0 86 0
noteoff 1 83 0
noteoff 2 80 0
noteoff 3 68 0
noteoff 4 74 0
noteoff 5 71 0
noteoff 6 62 0
noteoff 7 59 0
noteoff 15 80 0
noteoff 17 74 0
noteoff 20 71 0
cc 0 68 0
cc 1 68 0
cc 2 68 0
cc 3 68 0
cc 4 68 0
cc 5 68 0
cc 6 68 0
cc 7 68 0
cc 15 68 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
echo "measure 84 - $$ Page 43, Top, 11th [First ending.]"
noteoff 0 88 0
noteoff 1 85 0
noteoff 2 79 0
noteoff 3 67 0
noteoff 4 76 0
noteoff 5 73 0
noteoff 6 64 0
noteoff 7 61 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 76 0
noteoff 20 73 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 90 120
noteon 1 78 120
noteon 2 78 120
noteon 3 66 120
noteon 4 78 120
noteon 5 74 120
noteon 6 66 120
noteon 7 62 120
noteon 8 64 120
noteon 10 56 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 78 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
sleep 200.0
noteoff 0 90 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 66 0
noteoff 4 78 0
noteoff 5 74 0
noteoff 6 66 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 78 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
sleep 200.0
noteon 0 85 120
noteon 1 85 120
noteon 2 76 120
noteon 3 73 120
noteon 4 76 120
noteon 5 73 120
noteon 6 57 120
noteon 8 66 120
noteon 10 59 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 18 11 90
sleep 200.0
echo "measure 17 - $$ Page 40, Bottom, 5th"
noteoff 0 85 0
noteoff 1 85 0
noteoff 2 76 0
noteoff 3 73 0
noteoff 4 76 0
noteoff 5 73 0
noteoff 6 57 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 69 0
noteoff 15 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 1 86 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 74 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 56 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 86 120
noteon 16 69 120
noteon 17 78 120
noteon 18 62 120
noteon 19 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 16 11 90
cc 16 10 64
cc 16 7 91
cc 19 11 90
cc 19 10 64
cc 19 7 91
sleep 200.0
noteoff 0 86 0
noteoff 1 86 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 74 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 62 0
noteoff 15 86 0
noteoff 16 69 0
noteoff 17 78 0
noteoff 18 62 0
noteoff 19 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
sleep 200.0
sleep 200.0
echo "measure 18 - $$ Page 40, Bottom, 6th"
noteon 14 69 120
noteon 17 61 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
cc 14 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 200.0
noteoff 17 61 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 17 62 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
noteoff 17 62 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 17 64 120
noteon 20 61 120
noteon 22 61 120
noteon 23 49 120
sleep 200.0
echo "measure 19 - $$ Page 40, Bottom, 7th"
noteoff 14 69 0
noteoff 17 64 0
noteoff 20 61 0
noteoff 22 61 0
noteoff 23 49 0
noteon 14 77 120
noteon 17 65 120
noteon 20 62 120
noteon 22 62 120
noteon 23 50 120
sleep 200.0
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 62 0
noteoff 22 62 0
noteoff 23 50 0
noteon 14 76 120
sleep 200.0
noteoff 14 76 0
noteon 14 77 120
sleep 200.0
echo "measure 20 - $$ Page 40, Bottom, 8th"
noteoff 14 77 0
noteon 14 65 120
noteon 17 57 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 17 57 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 17 58 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
sleep 200.0
noteoff 17 58 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 17 60 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
echo "measure 21 - $$ Page 40, Bottom, 9th"
noteoff 14 65 0
noteoff 17 60 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 14 74 120
noteon 17 62 120
noteon 20 58 120
noteon 22 58 120
noteon 23 46 120
sleep 200.0
noteoff 14 74 0
noteoff 17 62 0
noteoff 20 58 0
noteoff 22 58 0
noteoff 23 46 0
noteon 14 73 120
sleep 200.0
noteoff 14 73 0
noteon 14 74 120
sleep 200.0
echo "measure 22 - $$ Page 40, Bottom, 10th"
noteoff 14 74 0
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 18 11 50
sleep 200.0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 77 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 400.0
echo "measure 23 - $$ Page 40, Bottom, 11th"
cc 14 11 50
cc 17 11 50
cc 18 11 50
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 400.0
noteoff 14 77 0
noteon 14 74 120
cc 14 68 127
sleep 100.0
noteon 14 70 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 24 - $$ Page 40, Bottom, 12th (last)"
noteoff 14 70 0
noteoff 17 62 0
noteoff 18 58 0
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
cc 14 68 127
sleep 100.0
noteon 14 72 120
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 75 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 72 0
sleep 100.0
noteon 14 74 120
noteoff 14 75 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 72 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 74 0
sleep 100.0
noteon 14 70 120
noteoff 14 72 0
sleep 100.0
echo "measure 25 - $$ Page 41, Top, 1st"
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 70 0
sleep 100.0
noteon 14 67 120
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 65 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 67 0
sleep 100.0
noteon 14 63 120
noteoff 14 65 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 81 120
noteon 6 57 120
noteon 14 62 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 0 68 127
cc 0 11 50
cc 6 68 127
cc 6 11 50
noteoff 14 63 0
sleep 100.0
noteon 14 60 120
noteoff 14 62 0
cc 14 68 0
sleep 100.0
echo "measure 26 - $$ Page 41, Top, 2nd"
noteoff 14 60 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 82 120
noteon 2 74 120
noteon 3 70 120
noteon 6 58 120
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 2 11 50
cc 3 11 50
noteoff 0 81 0
noteoff 6 57 0
cc 0 68 0
cc 6 68 0
sleep 200.0
noteoff 0 82 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 89 120
noteon 2 74 120
noteon 3 70 120
noteon 6 65 120
noteon 14 77 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 0 11 80
cc 2 11 80
cc 3 11 80
cc 6 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 400.0
cc 0 11 50
cc 2 11 50
echo "measure 27 - $$ Page 41, Top, 3rd"
cc 3 11 50
cc 6 11 50
cc 14 11 50
cc 17 11 50
cc 18 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 400.0
noteoff 0 89 0
noteoff 6 65 0
noteoff 14 77 0
noteon 0 86 120
noteon 6 62 120
noteon 14 74 120
cc 0 68 127
cc 6 68 127
cc 14 68 127
sleep 100.0
noteon 0 82 120
noteon 6 58 120
noteon 14 70 120
noteoff 0 86 0
noteoff 6 62 0
noteoff 14 74 0
cc 0 68 0
cc 6 68 0
cc 14 68 0
sleep 100.0
echo "measure 28 - $$ Page 41, Top, 4th"
noteoff 0 82 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 70 0
noteoff 17 62 0
noteoff 18 58 0
noteon 0 81 120
noteon 2 75 120
noteon 3 72 120
noteon 6 57 120
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
cc 0 68 127
cc 14 68 127
sleep 100.0
noteon 0 84 120
noteon 14 72 120
noteoff 0 81 0
noteoff 14 69 0
sleep 100.0
noteoff 6 57 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 87 120
noteon 14 75 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 84 0
noteoff 14 72 0
sleep 100.0
noteon 0 86 120
noteon 14 74 120
noteoff 0 87 0
noteoff 14 75 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 84 120
noteon 14 72 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 86 0
noteoff 14 74 0
sleep 100.0
noteon 0 82 120
noteon 14 70 120
noteoff 0 84 0
noteoff 14 72 0
sleep 100.0
echo "measure 29 - $$ Page 41, Top, 5th"
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 81 120
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 82 0
noteoff 14 70 0
sleep 100.0
noteon 0 79 120
noteon 14 67 120
noteoff 0 81 0
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 77 120
noteon 6 53 120
noteon 14 65 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 79 0
noteoff 14 67 0
sleep 100.0
noteon 0 75 120
noteon 14 63 120
noteoff 0 77 0
noteoff 14 65 0
sleep 100.0
noteoff 6 53 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 74 120
noteon 6 53 120
noteon 14 62 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 75 0
noteoff 14 63 0
sleep 100.0
noteon 0 72 120
noteon 14 60 120
noteoff 0 74 0
noteoff 14 62 0
cc 0 68 0
sleep 100.0
echo "measure 30 - $$ Page 41, Top, 6th"
noteoff 0 72 0
noteoff 2 75 0
noteoff 3 72 0
noteoff 6 53 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 70 120
noteon 2 74 120
noteon 3 70 120
noteon 6 58 120
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 60 0
cc 14 68 0
sleep 200.0
noteoff 0 70 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 74 120
noteon 17 62 120
noteon 20 58 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 58 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 31 - $$ Page 41, Top, 7th"
noteoff 14 76 0
noteoff 17 65 0
noteoff 20 62 0
noteon 6 57 120
noteon 14 77 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 6 57 0
noteoff 14 77 0
noteoff 22 45 0
noteoff 23 33 0
noteon 14 74 120
noteon 17 62 120
noteon 20 57 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 57 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 32 - $$ Page 41, Top, 8th"
noteoff 14 76 0
noteoff 17 65 0
noteoff 20 62 0
noteon 6 56 120
noteon 14 77 120
noteon 22 44 120
noteon 23 32 120
sleep 200.0
noteoff 6 56 0
noteoff 14 77 0
noteoff 22 44 0
noteoff 23 32 0
noteon 14 74 120
noteon 17 62 120
noteon 20 59 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 59 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
sleep 100.0
echo "measure 33 - $$ Page 41, Top, 9th"
noteoff 17 65 0
noteoff 20 62 0
noteon 6 56 120
noteon 14 77 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
noteon 22 56 120
noteon 23 44 120
noteoff 14 76 0
cc 14 68 0
sleep 200.0
noteoff 6 56 0
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteoff 22 56 0
noteoff 23 44 0
noteon 14 79 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
cc 14 68 127
sleep 100.0
noteon 14 77 120
noteoff 14 79 0
sleep 100.0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteon 14 76 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
noteoff 14 77 0
sleep 100.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 100.0
echo "measure 34 - $$ Page 41, Top, 10th (last)"
noteoff 14 74 0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteon 6 57 120
noteon 14 73 120
noteon 17 64 120
noteon 20 61 120
noteon 21 57 120
noteon 22 57 120
noteon 23 33 120
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 21 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 6 57 0
noteoff 14 73 0
noteoff 17 64 0
noteoff 20 61 0
noteoff 21 57 0
noteoff 22 57 0
noteon 14 74 120
cc 14 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 71 120
noteon 20 49 120
noteon 22 49 120
cc 20 68 127
cc 22 68 127
noteoff 14 73 0
sleep 100.0
noteon 14 69 120
noteoff 14 71 0
cc 14 68 0
sleep 100.0
echo "measure 35 - $$ Page 41, Bottom, 1st"
noteoff 14 69 0
noteon 14 77 120
noteon 20 50 120
noteon 22 50 120
noteoff 20 49 0
noteoff 22 49 0
sleep 200.0
noteoff 14 77 0
noteon 14 79 120
noteon 20 53 120
noteon 22 53 120
cc 14 68 127
noteoff 20 50 0
noteoff 22 50 0
sleep 100.0
noteon 14 77 120
noteoff 14 79 0
sleep 100.0
noteon 14 76 120
noteon 20 56 120
noteon 22 56 120
noteoff 14 77 0
noteoff 20 53 0
noteoff 22 53 0
cc 20 68 0
cc 22 68 0
sleep 100.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 100.0
echo "measure 36 - $$ Page 41, Bottom, 2nd"
noteoff 14 74 0
noteoff 20 56 0
noteoff 22 56 0
noteon 14 73 120
noteon 20 57 120
noteon 22 57 120
sleep 100.0
noteoff 20 57 0
noteoff 22 57 0
sleep 100.0
noteoff 14 73 0
noteon 14 74 120
cc 14 68 127
sleep 25.0
cc 14 11 41
sleep 50.0
cc 14 11 42
sleep 25.0
noteon 14 73 120
noteoff 14 74 0
sleep 25.0
cc 14 11 43
sleep 50.0
cc 14 11 44
sleep 25.0
noteon 14 71 120
noteon 17 61 120
noteon 20 49 120
noteon 22 49 120
cc 17 68 127
cc 20 68 127
cc 22 68 127
noteoff 14 73 0
sleep 25.0
cc 14 11 45
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 50.0
cc 14 11 46
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 25.0
noteon 14 69 120
noteoff 14 71 0
cc 14 68 0
sleep 25.0
cc 14 11 47
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 50.0
cc 14 11 48
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 25.0
echo "measure 37 - $$ Page 41, Bottom, 3rd"
noteoff 14 69 0
noteon 14 77 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteoff 17 61 0
noteoff 20 49 0
noteoff 22 49 0
sleep 25.0
cc 14 11 49
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 50.0
cc 14 11 50
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 25.0
noteoff 14 77 0
sleep 25.0
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 50.0
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 25.0
noteon 14 79 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
cc 14 68 127
cc 14 11 52
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 25.0
cc 14 11 53
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 50.0
cc 14 11 54
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 25.0
noteon 14 77 120
noteoff 14 79 0
sleep 25.0
cc 14 11 55
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 50.0
cc 14 11 56
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 25.0
noteon 14 76 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 25.0
cc 14 11 57
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 50.0
cc 14 11 58
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 25.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 25.0
cc 14 11 59
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 50.0
cc 14 11 60
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 25.0
echo "measure 38 - $$ Page 41, Bottom, 4th"
noteoff 14 74 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteon 14 73 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
sleep 25.0
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 7.916
cc 14 11 61
sleep 42.083
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 24.583
cc 14 11 62
sleep 0.416
noteoff 14 73 0
noteoff 17 69 0
noteoff 20 57 0
noteoff 22 57 0
sleep 100.0
noteoff 23 33 0
noteon 14 76 120
noteon 17 57 120
cc 14 68 127
cc 14 11 63
cc 17 68 127
cc 17 11 60
sleep 25.0
cc 14 11 64
cc 17 11 61
sleep 49.583
cc 14 11 65
sleep 0.416
cc 17 11 62
sleep 25.0
noteon 14 74 120
noteoff 14 76 0
sleep 24.583
cc 14 11 66
sleep 0.416
cc 17 11 63
sleep 49.583
cc 14 11 67
sleep 0.416
cc 17 11 64
sleep 25.0
noteon 14 73 120
noteon 17 58 120
noteoff 14 74 0
noteoff 17 57 0
sleep 24.583
cc 14 11 68
cc 17 11 65
sleep 50.0
cc 14 11 69
cc 17 11 66
sleep 25.416
noteon 14 71 120
noteoff 14 73 0
sleep 24.583
cc 14 11 70
cc 17 11 67
sleep 50.0
cc 14 11 71
cc 17 11 68
sleep 25.416
echo "measure 39 - $$ Page 41, Bottom, 5th"
noteon 14 69 120
noteon 17 59 120
noteoff 14 71 0
noteoff 17 58 0
sleep 24.583
cc 14 11 72
cc 17 11 69
sleep 50.0
cc 14 11 73
cc 17 11 70
sleep 25.416
noteon 14 68 120
noteoff 14 69 0
cc 14 68 0
sleep 24.583
cc 17 11 71
sleep 25.416
cc 14 11 74
sleep 24.583
cc 17 11 72
sleep 25.416
noteoff 14 68 0
noteon 14 69 120
noteon 17 60 120
noteoff 17 59 0
sleep 24.583
cc 14 11 75
cc 17 11 73
sleep 25.416
noteoff 14 69 0
sleep 24.583
cc 17 11 74
sleep 25.416
noteon 14 67 120
cc 14 11 76
sleep 24.583
cc 17 11 75
sleep 25.416
noteoff 14 67 0
sleep 24.583
cc 17 11 76
sleep 25.416
noteon 14 66 120
noteon 17 61 120
cc 14 11 77
noteoff 17 60 0
cc 17 68 0
sleep 24.583
cc 14 11 78
cc 17 11 77
sleep 25.416
noteoff 14 66 0
sleep 24.583
cc 17 11 78
sleep 25.416
noteon 14 64 120
cc 14 11 79
sleep 24.583
cc 17 11 79
sleep 25.416
noteoff 14 64 0
sleep 24.583
cc 17 11 80
sleep 25.416
echo "measure 40 - $$ Page 41, Bottom, 6th"
noteoff 17 61 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 12 11 80
cc 13 11 80
cc 14 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 41 - $$ Page 41, Bottom, 7th"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 42 - $$ Page 41, Bottom, 8th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 67 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 67 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 71 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 43 - $$ Page 41, Bottom, 9th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 71 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 44 - $$ Page 41, Bottom, 10th"
noteoff 14 83 0
noteon 8 66 120
noteon 10 59 120
cc 8 11 50
cc 10 11 50
sleep 200.0
noteoff 8 66 0
noteoff 10 59 0
noteon 8 68 120
noteon 10 64 120
sleep 200.0
noteoff 8 68 0
noteoff 10 64 0
noteon 8 69 120
noteon 10 66 120
sleep 200.0
echo "measure 45 - $$ Page 41, Bottom, 11th"
noteoff 8 69 0
noteoff 10 66 0
noteon 14 76 120
noteon 17 69 120
cc 17 11 50
sleep 200.0
noteoff 14 76 0
noteoff 17 69 0
noteon 14 78 120
noteon 17 74 120
sleep 200.0
noteoff 14 78 0
noteoff 17 74 0
noteon 14 79 120
noteon 17 76 120
sleep 200.0
echo "measure 46 - $$ Page 41, Bottom, 12th (last)"
noteoff 14 79 0
noteoff 17 76 0
noteon 2 78 120
noteon 3 74 120
noteon 8 68 120
noteon 10 64 120
cc 2 11 50
cc 3 11 50
sleep 200.0
noteoff 2 78 0
noteoff 3 74 0
noteoff 8 68 0
noteoff 10 64 0
noteon 2 76 120
noteon 3 69 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 66 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
echo "measure 47 - $$ Page 42, Top, 1st"
noteoff 2 74 0
noteoff 3 66 0
noteoff 8 64 0
noteoff 10 56 0
noteon 0 86 120
noteon 1 78 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 90
cc 1 11 90
cc 2 11 90
cc 3 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 1 76 120
noteon 2 79 120
noteon 3 76 120
noteon 4 76 120
noteon 5 67 120
noteon 6 57 120
noteon 7 45 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 0 85 0
noteoff 1 76 0
noteoff 2 79 0
noteoff 3 76 0
noteoff 4 76 0
noteoff 5 67 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 0 86 120
noteon 1 78 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
echo "measure 48 - $$ Page 42, Top, 2nd"
noteoff 0 86 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 12 11 80
cc 13 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 49 - $$ Page 42, Top, 3rd"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 50 - $$ Page 42, Top, 4th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 51 - $$ Page 42, Top, 5th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 52 - $$ Page 42, Top, 6th"
noteoff 14 83 0
noteon 14 67 120
sleep 200.0
noteoff 14 67 0
noteon 14 69 120
sleep 200.0
noteoff 14 69 0
noteon 14 71 120
sleep 200.0
echo "measure 53 - $$ Page 42, Top, 7th"
noteoff 14 71 0
noteon 17 64 120
cc 17 11 50
sleep 200.0
noteoff 17 64 0
noteon 17 66 120
sleep 200.0
noteoff 17 66 0
noteon 17 67 120
sleep 200.0
echo "measure 54 - $$ Page 42, Top, 8th"
noteoff 17 67 0
noteon 14 67 120
sleep 200.0
noteoff 14 67 0
noteon 14 69 120
sleep 200.0
noteoff 14 69 0
noteon 14 70 120
sleep 200.0
echo "measure 55 - $$ Page 42, Top, 9th"
noteoff 14 70 0
noteon 17 64 120
sleep 200.0
noteoff 17 64 0
noteon 17 66 120
sleep 200.0
noteoff 17 66 0
noteon 17 67 120
sleep 200.0
echo "measure 56 - $$ Page 42, Top, 10th"
noteoff 17 67 0
noteon 14 67 120
sleep 100.0
noteoff 14 67 0
sleep 100.0
noteon 14 69 120
cc 14 11 49
sleep 100.0
noteoff 14 69 0
sleep 100.0
noteon 14 70 120
cc 14 11 48
sleep 100.0
noteoff 14 70 0
sleep 100.0
echo "measure 57 - $$ Page 42, Top, 11th"
noteon 14 67 120
noteon 17 64 120
cc 14 11 47
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 46
cc 17 11 49
sleep 100.0
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 14 11 45
cc 17 11 48
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 58 - $$ Page 42, Top, 12th"
noteon 14 67 120
noteon 17 64 120
cc 14 11 44
cc 17 11 47
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 43
cc 17 11 46
sleep 100.0
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 14 11 42
cc 17 11 45
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 59 - $$ Page 42, Top, 13th"
noteon 14 67 120
noteon 17 64 120
cc 17 11 44
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 41
cc 17 11 43
sleep 49.583
cc 17 11 42
sleep 50.416
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 17 11 41
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 60 - $$ Page 42, Top, 14th (last)"
noteon 2 72 120
noteon 6 60 120
noteon 14 67 120
noteon 17 64 120
noteon 20 60 120
noteon 22 60 120
noteon 23 48 120
cc 2 11 40
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 14 67 0
noteoff 17 64 0
noteoff 20 60 0
noteoff 22 60 0
noteoff 23 48 0
noteon 14 69 120
noteon 17 65 120
sleep 200.0
noteoff 14 69 0
noteoff 17 65 0
noteon 14 70 120
noteon 17 67 120
sleep 200.0
echo "measure 61 - $$ Page 42, Bottom, 1st"
noteoff 2 72 0
noteoff 6 60 0
noteoff 14 70 0
noteoff 17 67 0
noteon 2 77 120
noteon 6 65 120
noteon 14 69 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 69 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 2 76 120
noteon 6 64 120
sleep 200.0
noteoff 2 76 0
noteoff 6 64 0
noteon 2 77 120
noteon 6 65 120
sleep 200.0
echo "measure 62 - $$ Page 42, Bottom, 2nd"
noteoff 2 77 0
noteoff 6 65 0
noteon 2 69 120
noteon 6 57 120
noteon 14 64 120
noteon 17 61 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 14 64 0
noteoff 17 61 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 14 65 120
noteon 17 62 120
sleep 200.0
noteoff 6 57 0
noteoff 14 65 0
noteoff 17 62 0
noteon 14 67 120
noteon 17 64 120
sleep 200.0
echo "measure 63 - $$ Page 42, Bottom, 3rd"
noteoff 2 69 0
noteoff 14 67 0
noteoff 17 64 0
noteon 2 74 120
noteon 6 62 120
noteon 14 65 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 65 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
noteoff 23 38 0
noteon 2 73 120
noteon 6 61 120
sleep 200.0
noteoff 2 73 0
noteoff 6 61 0
noteon 2 74 120
noteon 6 62 120
sleep 200.0
echo "measure 64 - $$ Page 42, Bottom, 4th"
noteoff 2 74 0
noteoff 6 62 0
noteon 4 74 120
noteon 5 62 120
noteon 6 52 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 17 11 50
cc 20 68 127
cc 20 11 50
cc 22 68 127
cc 22 11 50
cc 23 11 50
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 40.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 40.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 40.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 40.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
noteon 6 52 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 52 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 40.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 40.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 40.0
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 40.0
cc 6 11 60
cc 17 11 60
cc 20 11 60
cc 22 11 60
sleep 20.0
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 52 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 9.583
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 20.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 20.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 20.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 20.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 20.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 20.0
cc 6 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 10.416
echo "measure 65 - $$ Page 42, Bottom, 5th"
noteoff 7 46 0
noteon 6 46 120
noteon 7 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
cc 7 68 127
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
cc 6 68 0
sleep 200.0
noteon 7 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 7 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
cc 7 11 65
sleep 19.583
cc 7 11 66
sleep 40.0
cc 7 11 67
sleep 40.0
cc 7 11 68
sleep 40.0
cc 7 11 69
sleep 40.0
cc 7 11 70
sleep 20.416
noteon 7 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 7 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 7 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 10.0
cc 7 11 71
sleep 20.0
cc 7 11 72
sleep 20.0
cc 7 11 73
sleep 20.0
cc 7 11 74
sleep 20.0
cc 7 11 75
sleep 20.0
cc 7 11 76
sleep 20.0
cc 7 11 77
sleep 20.0
cc 7 11 78
sleep 20.0
cc 7 11 79
sleep 20.0
cc 7 11 80
sleep 10.0
echo "measure 66 - $$ Page 42, Bottom, 6th"
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 46 0
noteoff 7 56 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 57 120
noteon 12 69 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 68 127
cc 4 11 80
cc 5 68 127
cc 5 11 80
cc 6 11 80
cc 8 11 80
cc 10 11 80
cc 14 11 80
cc 15 68 127
cc 15 11 80
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 81 0
noteoff 17 69 0
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "measure 67 - $$ Page 42, Bottom, 7th"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 57 0
noteoff 12 69 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
noteoff 4 73 0
noteoff 5 61 0
cc 4 68 0
cc 5 68 0
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 50 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 14 76 120
noteon 17 64 120
sleep 200.0
noteoff 14 76 0
noteoff 17 64 0
noteon 14 74 120
noteon 17 62 120
sleep 200.0
echo "measure 68 - $$ Page 42, Bottom, 8th"
noteoff 14 74 0
noteoff 17 62 0
noteon 2 72 120
noteon 6 60 120
noteon 14 72 120
noteon 17 67 120
noteon 20 64 120
noteon 22 60 120
noteon 23 48 120
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 17 67 0
noteoff 20 64 0
noteoff 22 60 0
noteoff 23 48 0
noteon 17 69 120
noteon 20 65 120
sleep 200.0
noteoff 17 69 0
noteoff 20 65 0
noteon 17 70 120
noteon 20 67 120
sleep 200.0
echo "measure 69 - $$ Page 42, Bottom, 9th"
noteoff 2 72 0
noteoff 6 60 0
noteoff 14 72 0
noteoff 17 70 0
noteoff 20 67 0
noteon 2 77 120
noteon 6 65 120
noteon 14 77 120
noteon 17 69 120
noteon 20 65 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 77 0
noteoff 17 69 0
noteoff 20 65 0
noteoff 22 53 0
noteoff 23 41 0
noteon 2 76 120
noteon 6 64 120
noteon 14 76 120
sleep 200.0
noteoff 2 76 0
noteoff 6 64 0
noteoff 14 76 0
noteon 2 77 120
noteon 6 65 120
noteon 14 77 120
sleep 200.0
echo "measure 70 - $$ Page 42, Bottom, 10th"
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 77 0
noteon 0 81 120
noteon 2 69 120
noteon 6 57 120
noteon 14 69 120
noteon 17 64 120
noteon 20 61 120
noteon 22 57 120
noteon 23 45 120
cc 0 11 50
sleep 200.0
noteoff 17 64 0
noteoff 20 61 0
noteoff 22 57 0
noteoff 23 45 0
noteon 17 65 120
noteon 20 62 120
sleep 200.0
noteoff 17 65 0
noteoff 20 62 0
noteon 17 67 120
noteon 20 64 120
sleep 200.0
echo "measure 71 - $$ Page 42, Bottom, 11th"
noteoff 0 81 0
noteoff 2 69 0
noteoff 6 57 0
noteoff 14 69 0
noteoff 17 67 0
noteoff 20 64 0
noteon 0 86 120
noteon 2 74 120
noteon 6 62 120
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
noteoff 0 86 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 74 0
noteoff 17 65 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 2 73 120
noteon 6 61 120
noteon 14 73 120
sleep 200.0
noteoff 0 85 0
noteoff 2 73 0
noteoff 6 61 0
noteoff 14 73 0
noteon 0 86 120
noteon 2 74 120
noteon 6 62 120
noteon 14 74 120
sleep 200.0
echo "measure 72 - $$ Page 42, Bottom, 12th"
noteoff 0 86 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 74 0
noteon 4 74 120
noteon 5 62 120
noteon 6 52 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 20 68 127
cc 22 68 127
sleep 14.166
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 5.833
cc 6 11 51
sleep 22.5
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 17.5
cc 6 11 52
sleep 11.25
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 28.333
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 0.416
cc 6 11 53
sleep 28.333
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 11.666
cc 6 11 54
sleep 17.083
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 22.916
cc 6 11 55
sleep 5.416
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 14.583
noteon 6 54 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 52 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 14.166
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 5.833
cc 6 11 56
sleep 22.5
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 17.5
cc 6 11 57
sleep 11.25
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 28.333
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 0.416
cc 6 11 58
sleep 28.333
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 11.666
cc 6 11 59
sleep 17.083
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 22.916
cc 6 11 60
sleep 5.416
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 14.583
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 54 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 7.083
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 2.5
cc 6 11 59
sleep 11.666
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 8.333
cc 6 11 58
sleep 5.833
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 14.166
cc 6 11 57
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 14.583
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 5.416
cc 6 11 56
sleep 8.75
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 11.25
cc 6 11 55
sleep 2.916
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 14.583
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 2.5
cc 6 11 54
sleep 11.666
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 8.333
cc 6 11 53
sleep 5.833
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 14.166
cc 6 11 52
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 14.583
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 5.416
cc 6 11 51
sleep 8.75
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 11.25
cc 6 11 50
sleep 2.916
cc 17 11 40
cc 20 11 40
cc 22 11 40
sleep 7.5
echo "measure 73 - $$ Page 42, Bottom, 13th (last)"
noteon 6 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
sleep 200.0
noteon 6 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 6 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
sleep 200.0
noteon 6 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 6 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 200.0
echo "measure 74 - $$ Page 43, Top, 1st"
noteoff 4 74 0
noteoff 5 62 0
noteoff 7 46 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 68 127
cc 4 11 80
cc 5 68 127
cc 5 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 14 11 80
cc 15 68 127
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
noteoff 6 56 0
cc 6 68 0
cc 6 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 81 0
noteoff 17 69 0
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "measure 75 - $$ Page 43, Top, 2nd"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 62 120
noteon 22 62 120
noteon 23 50 120
noteoff 4 73 0
noteoff 5 61 0
cc 4 68 0
cc 5 68 0
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 62 0
noteoff 22 62 0
noteoff 23 50 0
noteon 14 76 120
noteon 17 64 120
sleep 200.0
noteoff 14 76 0
noteoff 17 64 0
noteon 14 74 120
noteon 17 62 120
sleep 200.0
echo "measure 76 - $$ Page 43, Top, 3rd"
noteoff 14 74 0
noteoff 17 62 0
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 17 11 50
cc 20 68 127
cc 20 11 50
cc 22 68 127
cc 22 11 50
cc 23 11 50
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 40.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 40.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 40.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 40.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
noteon 6 52 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 50 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 40.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 40.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 40.0
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 40.0
cc 6 11 60
cc 17 11 60
cc 20 11 60
cc 22 11 60
sleep 20.0
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 52 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 9.583
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 20.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 20.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 20.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 20.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 20.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 20.0
cc 6 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 10.416
echo "measure 77 - $$ Page 43, Top, 4th"
noteon 6 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
sleep 200.0
noteon 6 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 6 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
sleep 200.0
noteon 6 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 6 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 6 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 200.0
echo "measure 78 - $$ Page 43, Top, 5th"
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 56 0
noteoff 7 46 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 90
cc 12 11 90
cc 14 11 90
cc 15 68 127
cc 15 11 90
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 6 11 50
cc 7 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
noteoff 15 81 0
noteoff 17 69 0
cc 15 11 80
cc 17 11 50
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "measure 79 - $$ Page 43, Top, 6th"
noteoff 4 73 0
noteoff 5 61 0
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
cc 4 11 50
cc 5 11 50
cc 8 11 50
cc 10 11 50
cc 11 11 80
cc 12 11 80
cc 14 11 80
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
noteon 8 68 120
noteon 10 56 120
noteon 14 74 120
noteon 17 62 120
sleep 200.0
noteoff 8 68 0
noteoff 10 56 0
noteoff 14 74 0
noteoff 17 62 0
noteon 8 64 120
noteon 10 52 120
noteon 14 78 120
noteon 17 66 120
sleep 200.0
echo "measure 80 - $$ Page 43, Top, 7th"
noteoff 8 64 0
noteoff 10 52 0
noteoff 14 78 0
noteoff 17 66 0
noteon 0 85 120
noteon 4 73 120
noteon 6 61 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 81 120
noteon 15 62 120
noteon 17 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
cc 0 11 80
cc 4 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 90
cc 12 11 90
cc 14 68 127
cc 14 11 90
cc 15 11 90
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 7 57 120
noteon 14 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 7 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
noteoff 14 81 0
noteoff 17 69 0
cc 14 11 80
cc 17 11 50
sleep 200.0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 7 57 120
noteon 14 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 14 80 0
noteoff 17 68 0
cc 14 68 0
cc 17 68 0
sleep 200.0
echo "measure 81 - $$ Page 43, Top, 8th"
noteoff 0 85 0
noteoff 4 73 0
noteoff 6 61 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 79 0
noteoff 15 62 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 4 74 120
noteon 6 62 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
cc 0 11 50
cc 4 11 50
cc 6 11 50
cc 8 11 50
cc 10 11 50
cc 11 11 80
cc 12 11 80
sleep 200.0
noteoff 0 86 0
noteoff 4 74 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
noteon 8 68 120
noteon 10 56 120
noteon 14 74 120
noteon 17 62 120
sleep 200.0
noteoff 8 68 0
noteoff 10 56 0
noteoff 14 74 0
noteoff 17 62 0
noteon 8 64 120
noteon 10 52 120
noteon 14 78 120
noteon 17 66 120
sleep 200.0
echo "measure 82 - $$ Page 43, Top, 9th"
noteoff 8 64 0
noteoff 10 52 0
noteoff 14 78 0
noteoff 17 66 0
noteon 0 85 120
noteon 1 81 120
noteon 2 81 120
noteon 3 69 120
noteon 4 73 120
noteon 5 69 120
noteon 6 61 120
noteon 7 57 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 61 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
cc 0 11 90
cc 1 11 90
cc 2 11 90
cc 3 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 8 11 90
cc 10 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 68 127
cc 17 11 90
cc 20 68 127
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 13 45 0
noteoff 22 45 0
noteoff 23 33 0
noteon 13 45 120
noteon 17 64 120
noteon 20 61 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 61 0
noteoff 20 57 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 13 45 120
noteon 17 69 120
noteon 20 64 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 64 0
noteoff 20 61 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
cc 0 68 127
cc 1 68 127
cc 2 68 127
cc 3 68 127
cc 4 68 127
cc 5 68 127
cc 6 68 127
cc 7 68 127
echo "measure 83 - $$ Page 43, Top, 10th"
cc 15 68 127
noteoff 12 57 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 12 57 120
noteon 13 45 120
noteon 17 73 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 69 0
noteoff 20 64 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 8 59 0
noteoff 10 47 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 1 83 120
noteon 2 80 120
noteon 3 68 120
noteon 4 74 120
noteon 5 71 120
noteon 6 62 120
noteon 7 59 120
noteon 8 66 120
noteon 10 59 120
noteon 13 45 120
noteon 15 80 120
noteon 17 74 120
noteon 20 71 120
noteon 22 57 120
noteon 23 45 120
noteoff 0 85 0
noteoff 1 81 0
noteoff 2 81 0
noteoff 3 69 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 61 0
noteoff 7 57 0
noteoff 15 81 0
noteoff 17 73 0
noteoff 20 69 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 8 66 0
noteoff 10 59 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 88 120
noteon 1 85 120
noteon 2 79 120
noteon 3 67 120
noteon 4 76 120
noteon 5 73 120
noteon 6 64 120
noteon 7 61 120
noteon 8 66 120
noteon 10 59 120
noteon 13 45 120
noteon 15 79 120
noteon 17 76 120
noteon 20 73 120
noteon 22 57 120
noteon 23 45 120
noteoff 0 86 0
noteoff 1 83 0
noteoff 2 80 0
noteoff 3 68 0
noteoff 4 74 0
noteoff 5 71 0
noteoff 6 62 0
noteoff 7 59 0
noteoff 15 80 0
noteoff 17 74 0
noteoff 20 71 0
cc 0 68 0
cc 1 68 0
cc 2 68 0
cc 3 68 0
cc 4 68 0
cc 5 68 0
cc 6 68 0
cc 7 68 0
cc 15 68 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
echo "measure 84 - $$ Page 43, Top, 11th [First ending.]"
noteoff 0 88 0
noteoff 1 85 0
noteoff 2 79 0
noteoff 3 67 0
noteoff 4 76 0
noteoff 5 73 0
noteoff 6 64 0
noteoff 7 61 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 76 0
noteoff 20 73 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 90 120
noteon 1 78 120
noteon 2 78 120
noteon 3 66 120
noteon 4 78 120
noteon 5 74 120
noteon 6 66 120
noteon 7 62 120
noteon 8 64 120
noteon 10 56 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 78 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
sleep 200.0
noteoff 0 90 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 66 0
noteoff 4 78 0
noteoff 5 74 0
noteoff 6 66 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 78 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
sleep 200.0
noteon 0 85 120
noteon 1 85 120
noteon 2 76 120
noteon 3 73 120
noteon 4 76 120
noteon 5 73 120
noteon 6 57 120
noteon 8 66 120
noteon 10 59 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 18 11 90
sleep 200.0
echo "measure 85 - $$ Page 43, Top, 12th (last) [Second ending.]"
noteoff 0 85 0
noteoff 1 85 0
noteoff 2 76 0
noteoff 3 73 0
noteoff 4 76 0
noteoff 5 73 0
noteoff 6 57 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 69 0
noteoff 15 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 1 86 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 74 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 56 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 86 120
noteon 16 69 120
noteon 17 78 120
noteon 18 62 120
noteon 19 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 16 11 90
cc 19 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 86 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 74 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 62 0
noteoff 15 86 0
noteoff 16 69 0
noteoff 17 78 0
noteoff 18 62 0
noteoff 19 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
sleep 200.0
sleep 200.0
echo "meter 2 2 48 8"
echo "measure 86 - $$ Page 43, Bottom, 1st"
noteon 0 86 120
noteon 1 86 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 74 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 56 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 86 120
noteon 16 69 120
noteon 17 78 120
noteon 18 62 120
noteon 19 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
noteoff 0 86 0
echo "page 43"
echo "* title TRIO."
noteoff 1 86 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 74 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 62 0
noteoff 15 86 0
noteoff 16 69 0
noteoff 17 78 0
noteoff 18 62 0
noteoff 19 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
sleep 200.0
echo "meter 1 2 48 8"
echo "bars 15"
echo "measure 87 - $$ Page 43, Bottom, 2nd"
noteon 2 74 120
cc 2 68 127
cc 2 11 50
sleep 100.0
noteon 2 76 120
noteoff 2 74 0
cc 2 68 0
sleep 100.0
echo "meter 3 2 48 8"
echo "measure 88 - $$ Page 43, Bottom, 3rd"
noteoff 2 76 0
noteon 2 76 120
noteon 3 74 120
noteon 6 62 120
noteon 7 62 120
cc 2 68 127
cc 3 11 50
cc 6 11 50
cc 7 68 127
cc 7 11 50
sleep 600.0
echo "measure 89 - $$ Page 43, Bottom, 4th"
noteon 2 79 120
noteon 7 59 120
noteoff 2 76 0
noteoff 7 62 0
sleep 600.0
echo "measure 90 - $$ Page 43, Bottom, 5th"
cc 6 68 127
noteoff 3 74 0
noteon 2 81 120
noteon 3 74 120
noteon 7 54 120
cc 3 68 127
noteoff 2 79 0
noteoff 7 59 0
sleep 200.0
noteon 2 79 120
noteon 3 69 120
noteon 6 61 120
noteon 7 52 120
noteoff 2 81 0
noteoff 3 74 0
noteoff 6 62 0
noteoff 7 54 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 69 0
noteoff 6 61 0
noteoff 7 52 0
noteon 2 78 120
noteon 3 69 120
noteon 6 62 120
noteon 7 50 120
sleep 200.0
echo "measure 91 - $$ Page 43, Bottom, 6th"
noteoff 2 78 0
noteoff 3 69 0
noteoff 6 62 0
noteoff 7 50 0
noteon 2 76 120
noteon 3 69 120
noteon 6 61 120
noteon 7 69 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 6 61 0
noteoff 7 69 0
sleep 200.0
noteon 2 78 120
noteon 6 63 120
noteon 7 54 120
sleep 200.0
echo "measure 92 - $$ Page 43, Bottom, 7th"
noteoff 2 78 0
noteoff 6 63 0
noteoff 7 54 0
noteon 2 79 120
noteon 3 71 120
noteon 6 64 120
noteon 7 52 120
cc 2 68 127
cc 3 68 127
cc 6 68 127
cc 7 68 127
sleep 9.583
cc 2 11 51
cc 3 11 51
cc 6 11 51
cc 7 11 51
sleep 20.0
cc 2 11 52
cc 3 11 52
cc 6 11 52
cc 7 11 52
sleep 20.0
cc 2 11 53
cc 3 11 53
cc 6 11 53
cc 7 11 53
sleep 20.0
cc 2 11 54
cc 3 11 54
cc 6 11 54
cc 7 11 54
sleep 20.0
cc 2 11 55
cc 3 11 55
cc 6 11 55
cc 7 11 55
sleep 20.0
cc 2 11 56
cc 3 11 56
cc 6 11 56
cc 7 11 56
sleep 20.0
cc 2 11 57
cc 3 11 57
cc 6 11 57
cc 7 11 57
sleep 20.0
cc 2 11 58
cc 3 11 58
cc 6 11 58
cc 7 11 58
sleep 20.0
cc 2 11 59
cc 3 11 59
cc 6 11 59
cc 7 11 59
sleep 20.0
cc 2 11 60
cc 3 11 60
cc 6 11 60
cc 7 11 60
sleep 20.0
cc 2 11 61
cc 3 11 61
cc 6 11 61
cc 7 11 61
sleep 20.0
cc 2 11 62
cc 3 11 62
cc 6 11 62
cc 7 11 62
sleep 20.0
cc 2 11 63
cc 3 11 63
cc 6 11 63
cc 7 11 63
sleep 20.0
cc 2 11 64
cc 3 11 64
cc 6 11 64
cc 7 11 64
sleep 20.0
cc 2 11 65
cc 3 11 65
cc 6 11 65
cc 7 11 65
sleep 20.0
cc 2 11 66
cc 3 11 66
cc 6 11 66
cc 7 11 66
sleep 20.0
cc 2 11 67
cc 3 11 67
cc 6 11 67
cc 7 11 67
sleep 20.0
cc 2 11 68
cc 3 11 68
cc 6 11 68
cc 7 11 68
sleep 20.0
cc 2 11 69
cc 3 11 69
cc 6 11 69
cc 7 11 69
sleep 20.0
cc 2 11 70
cc 3 11 70
cc 6 11 70
cc 7 11 70
sleep 20.0
cc 2 11 71
cc 3 11 71
cc 6 11 71
cc 7 11 71
sleep 20.416
cc 2 11 72
cc 3 11 72
cc 6 11 72
cc 7 11 72
sleep 20.0
cc 2 11 73
cc 3 11 73
cc 6 11 73
cc 7 11 73
sleep 20.0
cc 2 11 74
cc 3 11 74
cc 6 11 74
cc 7 11 74
sleep 20.0
cc 2 11 75
cc 3 11 75
cc 6 11 75
cc 7 11 75
sleep 20.0
cc 2 11 76
cc 3 11 76
cc 6 11 76
cc 7 11 76
sleep 20.0
cc 2 11 77
cc 3 11 77
cc 6 11 77
cc 7 11 77
sleep 20.0
cc 2 11 78
cc 3 11 78
cc 6 11 78
cc 7 11 78
sleep 20.0
cc 2 11 79
cc 3 11 79
cc 6 11 79
cc 7 11 79
sleep 20.0
cc 2 11 80
cc 3 11 80
cc 6 11 80
cc 7 11 80
sleep 10.0
echo "measure 93 - $$ Page 43, Bottom, 8th"
noteon 2 81 120
noteon 3 73 120
noteon 6 61 120
noteon 7 45 120
noteon 8 66 120
noteon 10 59 120
cc 8 11 80
cc 10 11 80
noteoff 2 79 0
noteoff 3 71 0
noteoff 6 64 0
noteoff 7 52 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
sleep 10.0
cc 2 11 79
cc 3 11 79
cc 6 11 79
cc 7 11 79
cc 8 11 79
cc 10 11 79
sleep 20.0
cc 2 11 78
cc 3 11 78
cc 6 11 78
cc 7 11 78
cc 8 11 78
cc 10 11 78
sleep 20.0
cc 2 11 77
cc 3 11 77
cc 6 11 77
cc 7 11 77
cc 8 11 77
cc 10 11 77
sleep 20.0
cc 2 11 76
cc 3 11 76
cc 6 11 76
cc 7 11 76
cc 8 11 76
cc 10 11 76
sleep 20.0
cc 2 11 75
cc 3 11 75
cc 6 11 75
cc 7 11 75
cc 8 11 75
cc 10 11 75
sleep 20.0
cc 2 11 74
cc 3 11 74
cc 6 11 74
cc 7 11 74
cc 8 11 74
cc 10 11 74
sleep 20.0
cc 2 11 73
cc 3 11 73
cc 6 11 73
cc 7 11 73
cc 8 11 73
cc 10 11 73
sleep 20.0
cc 2 11 72
cc 3 11 72
cc 6 11 72
cc 7 11 72
cc 8 11 72
cc 10 11 72
sleep 20.0
cc 2 11 71
cc 3 11 71
cc 6 11 71
cc 7 11 71
cc 8 11 71
cc 10 11 71
sleep 20.0
cc 2 11 70
cc 3 11 70
cc 6 11 70
cc 7 11 70
cc 8 11 70
cc 10 11 70
sleep 20.0
cc 2 11 69
cc 3 11 69
cc 6 11 69
cc 7 11 69
cc 8 11 69
cc 10 11 69
sleep 20.0
cc 2 11 68
cc 3 11 68
cc 6 11 68
cc 7 11 68
cc 8 11 68
cc 10 11 68
sleep 20.0
cc 2 11 67
cc 3 11 67
cc 6 11 67
cc 7 11 67
cc 8 11 67
cc 10 11 67
sleep 20.0
cc 2 11 66
cc 3 11 66
cc 6 11 66
cc 7 11 66
cc 8 11 66
cc 10 11 66
sleep 20.0
cc 2 11 65
cc 3 11 65
cc 6 11 65
cc 7 11 65
cc 8 11 65
cc 10 11 65
sleep 20.0
cc 2 11 64
cc 3 11 64
cc 6 11 64
cc 7 11 64
cc 8 11 64
cc 10 11 64
sleep 20.0
cc 2 11 63
cc 3 11 63
cc 6 11 63
cc 7 11 63
cc 8 11 63
cc 10 11 63
sleep 20.0
cc 2 11 62
cc 3 11 62
cc 6 11 62
cc 7 11 62
cc 8 11 62
cc 10 11 62
sleep 20.0
cc 2 11 61
cc 3 11 61
cc 6 11 61
cc 7 11 61
cc 8 11 61
cc 10 11 61
sleep 20.0
cc 2 11 60
cc 3 11 60
cc 6 11 60
cc 7 11 60
cc 8 11 60
cc 10 11 60
sleep 10.0
noteoff 6 61 0
noteoff 7 45 0
sleep 10.0
cc 2 11 59
cc 3 11 59
cc 8 11 59
cc 10 11 59
sleep 20.0
cc 2 11 58
cc 3 11 58
cc 8 11 58
cc 10 11 58
sleep 20.0
cc 2 11 57
cc 3 11 57
cc 8 11 57
cc 10 11 57
sleep 20.0
cc 2 11 56
cc 3 11 56
cc 8 11 56
cc 10 11 56
sleep 20.0
cc 2 11 55
cc 3 11 55
cc 8 11 55
cc 10 11 55
sleep 20.0
cc 2 11 54
cc 3 11 54
cc 8 11 54
cc 10 11 54
sleep 20.0
cc 2 11 53
cc 3 11 53
cc 8 11 53
cc 10 11 53
sleep 20.0
cc 2 11 52
cc 3 11 52
cc 8 11 52
cc 10 11 52
sleep 20.0
cc 2 11 51
cc 3 11 51
cc 8 11 51
cc 10 11 51
sleep 20.0
cc 2 11 50
cc 3 11 50
cc 8 11 50
cc 10 11 50
sleep 10.0
echo "measure 94 - $$ Page 43, Bottom, 9th [Ending 1]"
noteoff 2 81 0
noteoff 3 73 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 78 120
noteon 3 74 120
noteon 6 62 120
noteon 7 50 120
noteon 8 68 120
noteon 10 64 120
cc 2 68 127
cc 3 68 127
cc 6 68 127
cc 6 11 50
cc 7 68 127
cc 7 11 50
cc 8 68 127
cc 10 68 127
sleep 200.0
noteon 2 79 120
noteon 3 76 120
noteon 6 59 120
noteon 7 55 120
noteon 8 69 120
noteon 10 66 120
noteoff 2 78 0
noteoff 3 74 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 8 68 0
noteoff 10 64 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteoff 6 59 0
noteoff 7 55 0
noteoff 8 69 0
noteoff 10 66 0
noteon 2 76 120
noteon 3 73 120
noteon 6 43 120
noteon 7 45 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
echo "meter 2 2 48 8"
echo "measure 86 - $$ Page 43, Bottom, 1st"
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 43 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 74 120
noteon 6 54 120
noteon 7 50 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
noteoff 2 74 0
noteoff 3 74 0
noteoff 6 54 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 56 0
sleep 200.0
echo "meter 1 2 48 8"
echo "measure 87 - $$ Page 43, Bottom, 2nd"
noteon 2 74 120
cc 2 68 127
sleep 100.0
noteon 2 76 120
noteoff 2 74 0
cc 2 68 0
sleep 100.0
echo "meter 3 2 48 8"
echo "measure 88 - $$ Page 43, Bottom, 3rd"
noteoff 2 76 0
noteon 2 76 120
noteon 3 74 120
noteon 6 62 120
noteon 7 62 120
cc 2 68 127
cc 7 68 127
sleep 600.0
echo "measure 89 - $$ Page 43, Bottom, 4th"
noteon 2 79 120
noteon 7 59 120
noteoff 2 76 0
noteoff 7 62 0
sleep 600.0
echo "measure 90 - $$ Page 43, Bottom, 5th"
cc 6 68 127
noteoff 3 74 0
noteon 2 81 120
noteon 3 74 120
noteon 7 54 120
cc 3 68 127
noteoff 2 79 0
noteoff 7 59 0
sleep 200.0
noteon 2 79 120
noteon 3 69 120
noteon 6 61 120
noteon 7 52 120
noteoff 2 81 0
noteoff 3 74 0
noteoff 6 62 0
noteoff 7 54 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 69 0
noteoff 6 61 0
noteoff 7 52 0
noteon 2 78 120
noteon 3 69 120
noteon 6 62 120
noteon 7 50 120
sleep 200.0
echo "measure 91 - $$ Page 43, Bottom, 6th"
noteoff 2 78 0
noteoff 3 69 0
noteoff 6 62 0
noteoff 7 50 0
noteon 2 76 120
noteon 3 69 120
noteon 6 61 120
noteon 7 69 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 6 61 0
noteoff 7 69 0
sleep 200.0
noteon 2 78 120
noteon 6 63 120
noteon 7 54 120
sleep 200.0
echo "measure 92 - $$ Page 43, Bottom, 7th"
noteoff 2 78 0
noteoff 6 63 0
noteoff 7 54 0
noteon 2 79 120
noteon 3 71 120
noteon 6 64 120
noteon 7 52 120
cc 2 68 127
cc 3 68 127
cc 6 68 127
cc 7 68 127
sleep 9.583
cc 2 11 51
cc 3 11 51
cc 6 11 51
cc 7 11 51
sleep 20.0
cc 2 11 52
cc 3 11 52
cc 6 11 52
cc 7 11 52
sleep 20.0
cc 2 11 53
cc 3 11 53
cc 6 11 53
cc 7 11 53
sleep 20.0
cc 2 11 54
cc 3 11 54
cc 6 11 54
cc 7 11 54
sleep 20.0
cc 2 11 55
cc 3 11 55
cc 6 11 55
cc 7 11 55
sleep 20.0
cc 2 11 56
cc 3 11 56
cc 6 11 56
cc 7 11 56
sleep 20.0
cc 2 11 57
cc 3 11 57
cc 6 11 57
cc 7 11 57
sleep 20.0
cc 2 11 58
cc 3 11 58
cc 6 11 58
cc 7 11 58
sleep 20.0
cc 2 11 59
cc 3 11 59
cc 6 11 59
cc 7 11 59
sleep 20.0
cc 2 11 60
cc 3 11 60
cc 6 11 60
cc 7 11 60
sleep 20.0
cc 2 11 61
cc 3 11 61
cc 6 11 61
cc 7 11 61
sleep 20.0
cc 2 11 62
cc 3 11 62
cc 6 11 62
cc 7 11 62
sleep 20.0
cc 2 11 63
cc 3 11 63
cc 6 11 63
cc 7 11 63
sleep 20.0
cc 2 11 64
cc 3 11 64
cc 6 11 64
cc 7 11 64
sleep 20.0
cc 2 11 65
cc 3 11 65
cc 6 11 65
cc 7 11 65
sleep 20.0
cc 2 11 66
cc 3 11 66
cc 6 11 66
cc 7 11 66
sleep 20.0
cc 2 11 67
cc 3 11 67
cc 6 11 67
cc 7 11 67
sleep 20.0
cc 2 11 68
cc 3 11 68
cc 6 11 68
cc 7 11 68
sleep 20.0
cc 2 11 69
cc 3 11 69
cc 6 11 69
cc 7 11 69
sleep 20.0
cc 2 11 70
cc 3 11 70
cc 6 11 70
cc 7 11 70
sleep 20.0
cc 2 11 71
cc 3 11 71
cc 6 11 71
cc 7 11 71
sleep 20.416
cc 2 11 72
cc 3 11 72
cc 6 11 72
cc 7 11 72
sleep 20.0
cc 2 11 73
cc 3 11 73
cc 6 11 73
cc 7 11 73
sleep 20.0
cc 2 11 74
cc 3 11 74
cc 6 11 74
cc 7 11 74
sleep 20.0
cc 2 11 75
cc 3 11 75
cc 6 11 75
cc 7 11 75
sleep 20.0
cc 2 11 76
cc 3 11 76
cc 6 11 76
cc 7 11 76
sleep 20.0
cc 2 11 77
cc 3 11 77
cc 6 11 77
cc 7 11 77
sleep 20.0
cc 2 11 78
cc 3 11 78
cc 6 11 78
cc 7 11 78
sleep 20.0
cc 2 11 79
cc 3 11 79
cc 6 11 79
cc 7 11 79
sleep 20.0
cc 2 11 80
cc 3 11 80
cc 6 11 80
cc 7 11 80
sleep 10.0
echo "measure 93 - $$ Page 43, Bottom, 8th"
noteon 2 81 120
noteon 3 73 120
noteon 6 61 120
noteon 7 45 120
noteon 8 66 120
noteon 10 59 120
cc 8 11 80
cc 10 11 80
noteoff 2 79 0
noteoff 3 71 0
noteoff 6 64 0
noteoff 7 52 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
sleep 10.0
cc 2 11 79
cc 3 11 79
cc 6 11 79
cc 7 11 79
cc 8 11 79
cc 10 11 79
sleep 20.0
cc 2 11 78
cc 3 11 78
cc 6 11 78
cc 7 11 78
cc 8 11 78
cc 10 11 78
sleep 20.0
cc 2 11 77
cc 3 11 77
cc 6 11 77
cc 7 11 77
cc 8 11 77
cc 10 11 77
sleep 20.0
cc 2 11 76
cc 3 11 76
cc 6 11 76
cc 7 11 76
cc 8 11 76
cc 10 11 76
sleep 20.0
cc 2 11 75
cc 3 11 75
cc 6 11 75
cc 7 11 75
cc 8 11 75
cc 10 11 75
sleep 20.0
cc 2 11 74
cc 3 11 74
cc 6 11 74
cc 7 11 74
cc 8 11 74
cc 10 11 74
sleep 20.0
cc 2 11 73
cc 3 11 73
cc 6 11 73
cc 7 11 73
cc 8 11 73
cc 10 11 73
sleep 20.0
cc 2 11 72
cc 3 11 72
cc 6 11 72
cc 7 11 72
cc 8 11 72
cc 10 11 72
sleep 20.0
cc 2 11 71
cc 3 11 71
cc 6 11 71
cc 7 11 71
cc 8 11 71
cc 10 11 71
sleep 20.0
cc 2 11 70
cc 3 11 70
cc 6 11 70
cc 7 11 70
cc 8 11 70
cc 10 11 70
sleep 20.0
cc 2 11 69
cc 3 11 69
cc 6 11 69
cc 7 11 69
cc 8 11 69
cc 10 11 69
sleep 20.0
cc 2 11 68
cc 3 11 68
cc 6 11 68
cc 7 11 68
cc 8 11 68
cc 10 11 68
sleep 20.0
cc 2 11 67
cc 3 11 67
cc 6 11 67
cc 7 11 67
cc 8 11 67
cc 10 11 67
sleep 20.0
cc 2 11 66
cc 3 11 66
cc 6 11 66
cc 7 11 66
cc 8 11 66
cc 10 11 66
sleep 20.0
cc 2 11 65
cc 3 11 65
cc 6 11 65
cc 7 11 65
cc 8 11 65
cc 10 11 65
sleep 20.0
cc 2 11 64
cc 3 11 64
cc 6 11 64
cc 7 11 64
cc 8 11 64
cc 10 11 64
sleep 20.0
cc 2 11 63
cc 3 11 63
cc 6 11 63
cc 7 11 63
cc 8 11 63
cc 10 11 63
sleep 20.0
cc 2 11 62
cc 3 11 62
cc 6 11 62
cc 7 11 62
cc 8 11 62
cc 10 11 62
sleep 20.0
cc 2 11 61
cc 3 11 61
cc 6 11 61
cc 7 11 61
cc 8 11 61
cc 10 11 61
sleep 20.0
cc 2 11 60
cc 3 11 60
cc 6 11 60
cc 7 11 60
cc 8 11 60
cc 10 11 60
sleep 10.0
noteoff 6 61 0
noteoff 7 45 0
sleep 10.0
cc 2 11 59
cc 3 11 59
cc 8 11 59
cc 10 11 59
sleep 20.0
cc 2 11 58
cc 3 11 58
cc 8 11 58
cc 10 11 58
sleep 20.0
cc 2 11 57
cc 3 11 57
cc 8 11 57
cc 10 11 57
sleep 20.0
cc 2 11 56
cc 3 11 56
cc 8 11 56
cc 10 11 56
sleep 20.0
cc 2 11 55
cc 3 11 55
cc 8 11 55
cc 10 11 55
sleep 20.0
cc 2 11 54
cc 3 11 54
cc 8 11 54
cc 10 11 54
sleep 20.0
cc 2 11 53
cc 3 11 53
cc 8 11 53
cc 10 11 53
sleep 20.0
cc 2 11 52
cc 3 11 52
cc 8 11 52
cc 10 11 52
sleep 20.0
cc 2 11 51
cc 3 11 51
cc 8 11 51
cc 10 11 51
sleep 20.0
cc 2 11 50
cc 3 11 50
cc 8 11 50
cc 10 11 50
sleep 10.0
echo "measure 94 - $$ Page 43, Bottom, 9th [Ending 1]"
noteoff 2 81 0
noteoff 3 73 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 78 120
noteon 3 74 120
noteon 6 62 120
noteon 7 50 120
noteon 8 68 120
noteon 10 64 120
cc 2 68 127
cc 3 68 127
cc 6 68 127
cc 6 11 50
cc 7 68 127
cc 7 11 50
cc 8 68 127
cc 10 68 127
sleep 200.0
noteon 2 79 120
noteon 3 76 120
noteon 6 59 120
noteon 7 55 120
noteon 8 69 120
noteon 10 66 120
noteoff 2 78 0
noteoff 3 74 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 8 68 0
noteoff 10 64 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteoff 6 59 0
noteoff 7 55 0
noteoff 8 69 0
noteoff 10 66 0
noteon 2 76 120
noteon 3 73 120
noteon 6 43 120
noteon 7 45 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
echo "meter 2 2 48 8"
echo "measure 95 - $$ Page 43, Bottom, 10th [Ending 2]"
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 43 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 74 120
noteon 6 54 120
noteon 7 50 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
noteoff 2 74 0
noteoff 3 74 0
noteoff 6 54 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 56 0
sleep 200.0
echo "meter 3 2 48 8"
echo "* Start of next repeat."
echo "measure 96 - $$ Page 43, Bottom, 11th"
noteon 2 74 120
noteon 3 74 120
noteon 6 54 120
noteon 7 50 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
noteoff 2 74 0
noteoff 3 74 0
noteoff 6 54 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 56 0
sleep 200.0
sleep 200.0
echo "measure 97 - $$ Page 43, Bottom, 12th"
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 23 42 120
cc 14 11 80
cc 17 11 80
cc 20 11 80
cc 23 11 80
sleep 600.0
echo "measure 98 - $$ Page 43, Bottom, 13th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 11 90
cc 17 11 90
cc 20 11 90
cc 22 11 100
cc 23 11 90
sleep 600.0
echo "measure 99 - $$ Page 43, Bottom, 14th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 14 11 80
cc 17 68 127
cc 17 11 80
cc 20 68 127
cc 20 11 80
cc 22 68 127
cc 22 11 90
cc 23 68 127
cc 23 11 80
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
cc 22 68 127
cc 23 68 127
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
cc 22 68 127
cc 23 68 127
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
echo "measure 100 - $$ Page 43, Bottom, 15th (last)"
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
cc 22 68 127
cc 23 68 127
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
cc 22 68 127
cc 23 68 127
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
cc 22 68 127
cc 23 68 127
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
echo "page 44"
echo "measure 101 - $$ Page 44, Top, 1st"
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteon 14 66 120
noteon 17 66 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 23 42 0
noteon 14 70 120
noteon 17 70 120
noteon 20 58 120
noteon 23 34 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteon 14 70 120
noteon 17 70 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteoff 20 58 0
noteoff 23 34 0
noteon 14 73 120
noteon 17 73 120
noteon 20 61 120
noteon 23 37 120
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteon 14 73 120
noteon 17 73 120
sleep 100.0
echo "bars 17"
echo "measure 102 - $$ Page 44, Top, 2nd"
noteoff 14 73 0
noteoff 17 73 0
noteoff 20 61 0
noteoff 23 37 0
noteon 14 78 120
noteon 17 78 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
cc 14 11 90
cc 17 11 90
cc 20 11 90
cc 22 11 100
cc 23 11 90
sleep 100.0
noteoff 14 78 0
noteoff 17 78 0
noteon 14 78 120
noteon 17 78 120
cc 14 11 80
cc 17 11 80
sleep 100.0
noteoff 14 78 0
noteoff 17 78 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 73 120
noteon 17 73 120
noteon 20 61 120
noteon 22 49 120
noteon 23 37 120
cc 20 11 80
cc 22 11 90
cc 23 11 80
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteon 14 73 120
noteon 17 73 120
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteoff 20 61 0
noteoff 22 49 0
noteoff 23 37 0
noteon 14 70 120
noteon 17 70 120
noteon 20 58 120
noteon 22 46 120
noteon 23 34 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteon 14 70 120
noteon 17 70 120
sleep 100.0
echo "measure 103 - $$ Page 44, Top, 3rd"
noteoff 14 70 0
noteoff 17 70 0
noteoff 20 58 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteon 14 66 120
noteon 17 66 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 70 120
noteon 17 70 120
noteon 20 58 120
noteon 22 46 120
noteon 23 34 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteon 14 70 120
noteon 17 70 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteoff 20 58 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 73 120
noteon 17 73 120
noteon 20 61 120
noteon 22 49 120
noteon 23 37 120
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteon 14 73 120
noteon 17 73 120
sleep 100.0
echo "measure 104 - $$ Page 44, Top, 4th"
noteoff 14 73 0
noteoff 17 73 0
noteoff 20 61 0
noteoff 22 49 0
noteoff 23 37 0
noteon 14 78 120
noteon 17 78 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
cc 14 11 90
cc 17 11 90
cc 20 11 90
cc 22 11 100
cc 23 11 90
sleep 100.0
noteoff 14 78 0
noteoff 17 78 0
noteon 14 78 120
noteon 17 78 120
cc 14 11 80
cc 17 11 80
sleep 100.0
noteoff 14 78 0
noteoff 17 78 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 73 120
noteon 17 73 120
noteon 20 61 120
noteon 22 49 120
noteon 23 37 120
cc 20 11 80
cc 22 11 90
cc 23 11 80
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteon 14 73 120
noteon 17 73 120
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteoff 20 61 0
noteoff 22 49 0
noteoff 23 37 0
noteon 14 70 120
noteon 17 70 120
noteon 20 58 120
noteon 22 46 120
noteon 23 34 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteon 14 70 120
noteon 17 70 120
sleep 100.0
echo "measure 105 - $$ Page 44, Top, 5th"
noteoff 14 70 0
noteoff 17 70 0
noteoff 20 58 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
cc 14 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
echo "measure 106 - $$ Page 44, Top, 6th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
echo "measure 107 - $$ Page 44, Top, 7th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 42 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 42 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 42 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 42 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 42 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
echo "measure 108 - $$ Page 44, Top, 8th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 42 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
echo "measure 109 - $$ Page 44, Top, 9th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 22 11 40
cc 23 11 40
sleep 600.0
echo "measure 110 - $$ Page 44, Top, 10th"
sleep 600.0
echo "measure 111 - $$ Page 44, Top, 11th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 0 81 120
noteon 1 81 120
noteon 2 81 120
noteon 3 69 120
noteon 4 81 120
noteon 5 69 120
noteon 6 57 120
noteon 7 45 120
noteon 8 71 120
noteon 10 59 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
cc 2 11 90
cc 3 11 90
cc 6 11 90
cc 7 11 90
cc 8 11 90
cc 10 11 90
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
echo "measure 112 - $$ Page 44, Top, 12th"
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
noteon 14 69 120
noteon 17 57 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
cc 14 68 127
cc 14 11 90
cc 17 68 127
cc 17 11 90
cc 20 68 127
cc 20 11 90
cc 22 68 127
cc 22 11 90
cc 23 68 127
cc 23 11 90
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
cc 13 68 127
sleep 50.0
echo "measure 113 - $$ Page 44, Top, 13th"
noteoff 0 81 0
noteoff 1 81 0
noteoff 2 81 0
noteoff 3 69 0
noteoff 4 81 0
noteoff 5 69 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 8 71 0
noteoff 10 59 0
noteoff 11 69 0
noteoff 12 57 0
noteon 0 78 120
noteon 1 78 120
noteon 2 78 120
noteon 3 74 120
noteon 4 78 120
noteon 5 66 120
noteon 6 47 120
noteon 7 47 120
noteon 8 68 120
noteon 10 64 120
noteon 11 66 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 17 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 50
cc 1 11 50
cc 2 68 127
cc 2 11 50
cc 3 11 50
cc 4 11 50
cc 5 11 50
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 11 11 50
cc 12 11 50
noteoff 13 45 0
noteoff 14 69 0
noteoff 17 57 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
cc 13 68 0
cc 13 11 50
cc 14 68 0
cc 14 11 50
cc 17 68 0
cc 17 11 50
cc 20 68 0
cc 20 11 50
cc 22 68 0
cc 22 11 50
cc 23 68 0
cc 23 11 50
sleep 200.0
noteoff 0 78 0
noteoff 1 78 0
noteoff 4 78 0
noteoff 5 66 0
noteoff 6 47 0
noteoff 7 47 0
noteoff 8 68 0
noteoff 10 64 0
noteoff 11 66 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 62 0
noteoff 17 62 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 62 120
noteon 7 62 120
sleep 200.0
noteoff 6 62 0
noteoff 7 62 0
noteon 6 61 120
noteon 7 61 120
sleep 200.0
echo "measure 114 - $$ Page 44, Top, 14th"
cc 3 68 127
noteoff 6 61 0
noteoff 7 61 0
noteon 2 79 120
noteon 6 59 120
noteon 7 59 120
noteoff 2 78 0
sleep 200.0
noteoff 6 59 0
noteoff 7 59 0
noteon 3 73 120
noteon 6 57 120
noteon 7 57 120
noteoff 3 74 0
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteon 3 71 120
noteon 6 55 120
noteon 7 55 120
noteoff 3 73 0
cc 3 68 0
sleep 200.0
echo "measure 115 - $$ Page 44, Top, 15th"
noteoff 3 71 0
noteoff 6 55 0
noteoff 7 55 0
noteon 2 81 120
noteon 3 69 120
noteon 6 54 120
noteon 7 54 120
noteon 8 71 120
noteon 10 59 120
cc 3 68 127
noteoff 2 79 0
sleep 200.0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 71 0
noteoff 10 59 0
noteon 2 79 120
noteon 3 73 120
noteon 6 52 120
noteon 7 52 120
noteon 8 71 120
noteon 10 59 120
noteoff 2 81 0
noteoff 3 69 0
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 8 71 0
noteoff 10 59 0
noteon 2 78 120
noteon 3 74 120
noteon 6 50 120
noteon 7 50 120
noteon 8 71 120
noteon 10 59 120
noteoff 2 79 0
noteoff 3 73 0
cc 2 68 0
cc 3 68 0
sleep 200.0
echo "measure 116 - $$ Page 44, Top, 16th"
noteoff 2 78 0
noteoff 3 74 0
noteoff 6 50 0
noteoff 7 50 0
noteoff 8 71 0
noteoff 10 59 0
noteon 2 76 120
noteon 3 73 120
noteon 6 57 120
noteon 7 57 120
noteon 8 71 120
noteon 10 59 120
sleep 200.0
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 71 0
noteoff 10 59 0
sleep 200.0
noteon 2 78 120
noteon 3 75 120
noteon 4 69 120
noteon 6 54 120
noteon 7 54 120
sleep 200.0
echo "measure 117 - $$ Page 44, Top, 17th (last)"
noteoff 2 78 0
noteoff 3 75 0
noteoff 4 69 0
noteoff 6 54 0
noteoff 7 54 0
noteon 2 79 120
noteon 3 76 120
noteon 4 71 120
noteon 6 52 120
noteon 7 52 120
cc 2 68 127
cc 4 68 127
sleep 7.083
cc 2 11 51
cc 3 11 51
cc 4 11 51
sleep 2.5
cc 6 11 51
cc 7 11 51
sleep 12.5
cc 2 11 52
cc 3 11 52
cc 4 11 52
sleep 7.5
cc 6 11 52
cc 7 11 52
sleep 7.5
cc 2 11 53
cc 3 11 53
cc 4 11 53
sleep 12.5
cc 6 11 53
cc 7 11 53
sleep 2.5
cc 2 11 54
cc 3 11 54
cc 4 11 54
sleep 15.0
cc 2 11 55
cc 3 11 55
cc 4 11 55
sleep 2.5
cc 6 11 54
cc 7 11 54
sleep 12.5
cc 2 11 56
cc 3 11 56
cc 4 11 56
sleep 7.5
cc 6 11 55
cc 7 11 55
sleep 7.5
cc 2 11 57
cc 3 11 57
cc 4 11 57
sleep 2.916
noteoff 6 52 0
noteoff 7 52 0
sleep 12.083
cc 2 11 58
cc 3 11 58
cc 4 11 58
sleep 15.0
cc 2 11 59
cc 3 11 59
cc 4 11 59
sleep 15.0
cc 2 11 60
cc 3 11 60
cc 4 11 60
sleep 15.0
cc 2 11 61
cc 3 11 61
cc 4 11 61
sleep 15.0
cc 2 11 62
cc 3 11 62
cc 4 11 62
sleep 15.0
cc 2 11 63
cc 3 11 63
cc 4 11 63
sleep 12.916
noteon 6 64 120
noteon 7 64 120
cc 6 11 60
cc 7 11 60
sleep 2.083
cc 2 11 64
cc 3 11 64
cc 4 11 64
sleep 7.5
cc 6 11 61
cc 7 11 61
sleep 7.5
cc 2 11 65
cc 3 11 65
cc 4 11 65
sleep 12.5
cc 6 11 62
cc 7 11 62
sleep 2.5
cc 2 11 66
cc 3 11 66
cc 4 11 66
sleep 15.416
cc 2 11 67
cc 3 11 67
cc 4 11 67
sleep 2.083
cc 6 11 63
cc 7 11 63
sleep 12.916
cc 2 11 68
cc 3 11 68
cc 4 11 68
sleep 7.083
cc 6 11 64
cc 7 11 64
sleep 7.916
cc 2 11 69
cc 3 11 69
cc 4 11 69
sleep 12.083
cc 6 11 65
cc 7 11 65
sleep 2.916
cc 2 11 70
cc 3 11 70
cc 4 11 70
sleep 7.5
noteoff 6 64 0
noteoff 7 64 0
sleep 7.5
cc 2 11 71
cc 3 11 71
cc 4 11 71
sleep 15.0
cc 2 11 72
cc 3 11 72
cc 4 11 72
sleep 15.0
cc 2 11 73
cc 3 11 73
cc 4 11 73
sleep 15.0
cc 2 11 74
cc 3 11 74
cc 4 11 74
sleep 15.0
cc 2 11 75
cc 3 11 75
cc 4 11 75
sleep 15.0
cc 2 11 76
cc 3 11 76
cc 4 11 76
sleep 15.0
cc 2 11 77
cc 3 11 77
cc 4 11 77
sleep 2.5
noteon 6 62 120
noteon 7 62 120
cc 6 11 70
cc 7 11 70
sleep 4.583
cc 6 11 71
cc 7 11 71
sleep 7.916
cc 2 11 78
cc 3 11 78
cc 4 11 78
sleep 2.083
cc 6 11 72
cc 7 11 72
sleep 10.0
cc 6 11 73
cc 7 11 73
sleep 2.916
cc 2 11 79
cc 3 11 79
cc 4 11 79
sleep 7.083
cc 6 11 74
cc 7 11 74
sleep 7.916
cc 2 11 80
cc 3 11 80
cc 4 11 80
sleep 2.083
cc 6 11 75
cc 7 11 75
sleep 10.0
cc 6 11 76
cc 7 11 76
sleep 2.916
cc 2 11 81
cc 3 11 81
cc 4 11 81
sleep 7.083
cc 6 11 77
cc 7 11 77
sleep 7.916
cc 2 11 82
cc 3 11 82
cc 4 11 82
sleep 2.083
cc 6 11 78
cc 7 11 78
sleep 10.0
cc 6 11 79
cc 7 11 79
sleep 2.916
cc 2 11 83
cc 3 11 83
cc 4 11 83
sleep 7.083
cc 6 11 80
cc 7 11 80
sleep 5.416
noteoff 6 62 0
noteoff 7 62 0
sleep 2.5
cc 2 11 84
cc 3 11 84
cc 4 11 84
sleep 15.0
cc 2 11 85
cc 3 11 85
cc 4 11 85
sleep 15.0
cc 2 11 86
cc 3 11 86
cc 4 11 86
sleep 15.0
cc 2 11 87
cc 3 11 87
cc 4 11 87
sleep 15.0
cc 2 11 88
cc 3 11 88
cc 4 11 88
sleep 15.0
cc 2 11 89
cc 3 11 89
cc 4 11 89
sleep 15.0
cc 2 11 90
cc 3 11 90
cc 4 11 90
sleep 7.5
echo "page 44"
echo "measure 118 - $$ Page 44, Bottom, 1st"
cc 3 68 127
noteon 2 81 120
noteon 4 69 120
noteon 6 61 120
noteon 7 61 120
noteon 8 66 120
noteon 10 59 120
cc 6 11 90
cc 7 11 90
cc 8 11 80
cc 10 11 80
noteoff 2 79 0
noteoff 4 71 0
cc 2 68 0
sleep 7.083
cc 4 11 89
sleep 0.416
cc 2 11 89
cc 3 11 89
cc 6 11 89
cc 7 11 89
sleep 2.5
cc 8 11 79
cc 10 11 79
sleep 12.083
cc 4 11 88
sleep 0.416
cc 2 11 88
sleep 0.416
cc 3 11 88
cc 6 11 88
cc 7 11 88
sleep 7.083
cc 8 11 78
cc 10 11 78
sleep 6.666
cc 4 11 87
sleep 0.833
cc 2 11 87
sleep 0.833
cc 3 11 87
cc 6 11 87
cc 7 11 87
sleep 11.666
cc 8 11 77
cc 10 11 77
sleep 1.666
cc 4 11 86
sleep 0.833
cc 2 11 86
sleep 1.25
cc 3 11 86
cc 6 11 86
cc 7 11 86
sleep 12.5
cc 4 11 85
sleep 1.25
cc 2 11 85
sleep 1.666
cc 3 11 85
cc 6 11 85
cc 7 11 85
sleep 0.833
cc 8 11 76
cc 10 11 76
sleep 11.25
cc 4 11 84
sleep 1.25
cc 2 11 84
sleep 2.083
cc 3 11 84
cc 6 11 84
cc 7 11 84
sleep 5.416
cc 8 11 75
cc 10 11 75
sleep 6.25
cc 4 11 83
sleep 1.25
cc 2 11 83
sleep 2.083
cc 3 11 83
cc 6 11 83
cc 7 11 83
sleep 0.416
noteoff 6 61 0
noteoff 7 61 0
sleep 10.0
cc 8 11 74
cc 10 11 74
sleep 0.833
cc 4 11 82
sleep 1.666
cc 2 11 82
sleep 2.5
cc 3 11 82
sleep 10.833
cc 4 11 81
sleep 1.666
cc 2 11 81
sleep 2.5
cc 8 11 73
cc 10 11 73
sleep 0.416
cc 3 11 81
sleep 10.0
cc 4 11 80
sleep 2.083
cc 2 11 80
sleep 3.333
cc 3 11 80
sleep 4.166
cc 8 11 72
cc 10 11 72
sleep 5.416
cc 4 11 79
sleep 2.083
cc 2 11 79
sleep 3.75
cc 3 11 79
sleep 8.75
cc 4 11 78
cc 8 11 71
cc 10 11 71
sleep 2.5
cc 2 11 78
sleep 4.166
cc 3 11 78
sleep 8.333
cc 4 11 77
sleep 2.5
cc 2 11 77
sleep 2.5
cc 8 11 70
cc 10 11 70
sleep 2.083
cc 3 11 77
sleep 7.5
cc 4 11 76
sleep 0.416
noteon 3 74 120
noteon 6 59 120
noteon 7 59 120
cc 6 11 77
cc 7 11 77
noteoff 3 76 0
sleep 2.5
cc 2 11 76
sleep 5.0
cc 3 11 76
cc 6 11 76
cc 7 11 76
sleep 2.5
cc 8 11 69
cc 10 11 69
sleep 4.583
cc 4 11 75
sleep 2.916
cc 2 11 75
sleep 5.416
cc 3 11 75
cc 6 11 75
cc 7 11 75
sleep 6.666
cc 4 11 74
sleep 0.416
cc 8 11 68
cc 10 11 68
sleep 2.5
cc 2 11 74
sleep 5.833
cc 3 11 74
cc 6 11 74
cc 7 11 74
sleep 5.833
cc 4 11 73
sleep 3.333
cc 2 11 73
sleep 2.5
cc 8 11 67
cc 10 11 67
sleep 3.75
cc 3 11 73
cc 6 11 73
cc 7 11 73
sleep 5.416
cc 4 11 72
sleep 3.333
cc 2 11 72
sleep 6.666
cc 3 11 72
cc 6 11 72
cc 7 11 72
sleep 0.833
cc 8 11 66
cc 10 11 66
sleep 3.75
cc 4 11 71
sleep 3.75
cc 2 11 71
sleep 7.083
cc 3 11 71
cc 6 11 71
cc 7 11 71
sleep 4.166
cc 4 11 70
sleep 1.25
cc 8 11 65
cc 10 11 65
sleep 2.5
cc 2 11 70
sleep 7.083
cc 3 11 70
cc 6 11 70
cc 7 11 70
sleep 0.416
noteoff 6 59 0
noteoff 7 59 0
sleep 3.333
cc 4 11 69
sleep 4.166
cc 2 11 69
sleep 2.5
cc 8 11 64
cc 10 11 64
sleep 5.0
cc 3 11 69
sleep 3.333
cc 4 11 68
sleep 4.166
cc 2 11 68
sleep 7.5
cc 8 11 63
cc 10 11 63
sleep 0.416
cc 3 11 68
sleep 2.5
cc 4 11 67
sleep 4.583
cc 2 11 67
sleep 8.333
cc 3 11 67
sleep 2.083
cc 4 11 66
sleep 2.083
cc 8 11 62
cc 10 11 62
sleep 2.5
cc 2 11 66
sleep 8.75
cc 3 11 66
sleep 1.666
cc 4 11 65
sleep 4.583
cc 2 11 65
sleep 2.5
cc 8 11 61
cc 10 11 61
sleep 6.666
cc 3 11 65
sleep 0.833
cc 4 11 64
sleep 5.0
cc 2 11 64
sleep 7.5
cc 8 11 60
cc 10 11 60
sleep 2.083
cc 3 11 64
sleep 0.416
cc 4 11 63
sleep 5.0
cc 2 11 63
sleep 2.5
noteon 3 73 120
noteon 4 67 120
noteon 6 57 120
noteon 7 57 120
cc 6 11 64
cc 7 11 64
noteoff 3 74 0
noteoff 4 69 0
cc 3 68 0
cc 4 68 0
sleep 7.083
cc 3 11 63
cc 6 11 63
cc 7 11 63
sleep 0.416
cc 4 11 62
sleep 2.5
cc 8 11 59
cc 10 11 59
sleep 2.5
cc 2 11 62
sleep 8.75
cc 3 11 62
cc 6 11 62
cc 7 11 62
sleep 1.666
cc 4 11 61
sleep 4.583
cc 2 11 61
sleep 2.5
cc 8 11 58
cc 10 11 58
sleep 5.416
cc 3 11 61
cc 6 11 61
cc 7 11 61
sleep 2.916
cc 4 11 60
sleep 4.166
cc 2 11 60
sleep 7.083
cc 3 11 60
cc 6 11 60
cc 7 11 60
sleep 0.416
cc 8 11 57
cc 10 11 57
sleep 3.75
cc 4 11 59
sleep 3.75
cc 2 11 59
sleep 6.666
cc 3 11 59
cc 6 11 59
cc 7 11 59
sleep 5.0
cc 4 11 58
sleep 0.833
cc 8 11 56
cc 10 11 56
sleep 2.5
cc 2 11 58
sleep 5.833
cc 3 11 58
cc 6 11 58
cc 7 11 58
sleep 6.25
cc 4 11 57
sleep 2.916
cc 2 11 57
sleep 2.5
cc 8 11 55
cc 10 11 55
sleep 2.5
cc 3 11 57
cc 6 11 57
cc 7 11 57
sleep 7.5
cc 4 11 56
noteoff 6 57 0
noteoff 7 57 0
sleep 2.5
cc 2 11 56
sleep 4.583
cc 3 11 56
sleep 2.916
cc 8 11 54
cc 10 11 54
sleep 5.0
cc 4 11 55
sleep 2.5
cc 2 11 55
sleep 3.75
cc 3 11 55
sleep 8.75
cc 8 11 53
cc 10 11 53
sleep 0.416
cc 4 11 54
sleep 2.083
cc 2 11 54
sleep 2.916
cc 3 11 54
sleep 10.416
cc 4 11 53
sleep 1.666
cc 2 11 53
sleep 2.083
cc 3 11 53
sleep 0.416
cc 8 11 52
cc 10 11 52
sleep 11.25
cc 4 11 52
sleep 1.25
cc 2 11 52
sleep 1.666
cc 3 11 52
sleep 5.833
cc 8 11 51
cc 10 11 51
sleep 6.666
cc 4 11 51
sleep 0.833
cc 2 11 51
sleep 0.833
cc 3 11 51
sleep 11.666
cc 8 11 50
cc 10 11 50
sleep 2.083
cc 4 11 50
sleep 0.416
cc 2 11 50
cc 3 11 50
sleep 7.5
echo "bars 16"
echo "measure 119 - $$ Page 44, Bottom, 2nd"
noteoff 2 81 0
noteoff 3 73 0
noteoff 4 67 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 78 120
noteon 3 74 120
noteon 4 66 120
noteon 6 62 120
noteon 7 62 120
noteon 8 68 120
noteon 10 64 120
cc 2 68 127
cc 3 68 127
cc 4 68 127
cc 6 68 127
cc 6 11 50
cc 7 68 127
cc 7 11 50
cc 8 68 127
cc 10 68 127
sleep 200.0
noteon 2 79 120
noteon 3 76 120
noteon 4 71 120
noteon 6 55 120
noteon 7 55 120
noteon 8 69 120
noteon 10 66 120
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 66 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 68 0
noteoff 10 64 0
cc 2 68 0
cc 3 68 0
cc 4 68 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteoff 4 71 0
noteoff 6 55 0
noteoff 7 55 0
noteoff 8 69 0
noteoff 10 66 0
noteon 2 76 120
noteon 3 73 120
noteon 4 67 120
noteon 6 57 120
noteon 7 57 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
echo "measure 120 - $$ Page 44, Bottom, 3rd"
noteoff 2 76 0
noteoff 3 73 0
noteoff 4 67 0
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 74 120
noteon 4 66 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 66 0
noteoff 6 50 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 56 0
sleep 200.0
noteon 14 62 120
cc 14 68 127
sleep 100.0
noteon 14 64 120
noteoff 14 62 0
cc 14 68 0
sleep 100.0
echo "measure 121 - $$ Page 44, Bottom, 4th"
noteoff 14 64 0
noteon 8 68 120
noteon 10 52 120
noteon 14 66 120
noteon 17 62 120
noteon 24 50 120
noteon 25 50 120
noteon 26 38 120
cc 8 68 127
cc 14 68 127
cc 24 11 50
cc 24 10 64
cc 24 7 100
cc 25 11 50
cc 25 10 64
cc 25 7 100
cc 26 11 50
cc 26 10 64
cc 26 7 91
sleep 200.0
noteoff 24 50 0
noteoff 25 50 0
noteoff 26 38 0
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
sleep 200.0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 24 61 120
noteon 25 61 120
noteon 26 49 120
sleep 200.0
echo "measure 122 - $$ Page 44, Bottom, 5th"
cc 17 68 127
noteoff 24 61 0
noteoff 25 61 0
noteoff 26 49 0
noteon 8 69 120
noteon 14 67 120
noteon 24 59 120
noteon 25 59 120
noteon 26 47 120
noteoff 8 68 0
noteoff 14 66 0
cc 8 68 0
cc 14 68 0
sleep 200.0
noteoff 24 59 0
noteoff 25 59 0
noteoff 26 47 0
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
noteoff 17 62 0
sleep 200.0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 17 59 120
noteon 24 55 120
noteon 25 55 120
noteon 26 43 120
noteoff 17 61 0
cc 17 68 0
sleep 200.0
echo "measure 123 - $$ Page 44, Bottom, 6th"
noteoff 8 69 0
noteoff 10 52 0
noteoff 14 67 0
noteoff 17 59 0
noteoff 24 55 0
noteoff 25 55 0
noteoff 26 43 0
noteon 8 71 120
noteon 10 52 120
noteon 14 69 120
noteon 17 57 120
noteon 24 54 120
noteon 25 54 120
noteon 26 42 120
cc 8 68 127
cc 10 68 127
cc 14 68 127
cc 17 68 127
sleep 200.0
noteoff 24 54 0
noteoff 25 54 0
noteoff 26 42 0
noteon 8 69 120
noteon 10 51 120
noteon 14 67 120
noteon 17 59 120
noteon 24 52 120
noteon 25 52 120
noteon 26 40 120
noteoff 8 71 0
noteoff 10 52 0
noteoff 14 69 0
noteoff 17 57 0
cc 8 68 0
cc 10 68 0
sleep 200.0
noteoff 8 69 0
noteoff 10 51 0
noteoff 24 52 0
noteoff 25 52 0
noteoff 26 40 0
noteon 8 68 120
noteon 10 52 120
noteon 14 66 120
noteon 17 62 120
noteon 24 50 120
noteon 25 50 120
noteon 26 38 120
noteoff 14 67 0
noteoff 17 59 0
cc 14 68 0
cc 17 68 0
sleep 200.0
echo "measure 124 - $$ Page 44, Bottom, 7th"
noteoff 8 68 0
noteoff 10 52 0
noteoff 14 66 0
noteoff 17 62 0
noteoff 24 50 0
noteoff 25 50 0
noteoff 26 38 0
noteon 8 66 120
noteon 10 47 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
noteoff 8 66 0
noteoff 10 47 0
noteoff 14 64 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
sleep 200.0
noteon 6 57 120
noteon 7 57 120
noteon 8 68 120
noteon 14 66 120
noteon 17 63 120
noteon 24 54 120
noteon 25 54 120
noteon 26 42 120
cc 14 68 127
cc 17 68 127
sleep 200.0
echo "measure 125 - $$ Page 44, Bottom, 8th"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 68 0
noteoff 24 54 0
noteoff 25 54 0
noteoff 26 42 0
noteon 6 59 120
noteon 7 59 120
noteon 8 69 120
noteon 14 67 120
noteon 17 64 120
noteon 24 52 120
noteon 25 52 120
noteon 26 40 120
cc 6 68 127
cc 7 68 127
cc 8 68 127
noteoff 14 66 0
noteoff 17 63 0
cc 17 68 0
sleep 9.583
cc 6 11 51
cc 7 11 51
cc 8 11 51
cc 14 11 51
cc 17 11 51
cc 24 11 51
cc 25 11 51
cc 26 11 51
sleep 20.0
cc 6 11 52
cc 7 11 52
cc 8 11 52
cc 14 11 52
cc 17 11 52
cc 24 11 52
cc 25 11 52
cc 26 11 52
sleep 20.0
cc 6 11 53
cc 7 11 53
cc 8 11 53
cc 14 11 53
cc 17 11 53
cc 24 11 53
cc 25 11 53
cc 26 11 53
sleep 20.0
cc 6 11 54
cc 7 11 54
cc 8 11 54
cc 14 11 54
cc 17 11 54
cc 24 11 54
cc 25 11 54
cc 26 11 54
sleep 20.0
cc 6 11 55
cc 7 11 55
cc 8 11 55
cc 14 11 55
cc 17 11 55
cc 24 11 55
cc 25 11 55
cc 26 11 55
sleep 20.0
cc 6 11 56
cc 7 11 56
cc 8 11 56
cc 14 11 56
cc 17 11 56
cc 24 11 56
cc 25 11 56
cc 26 11 56
sleep 20.0
cc 6 11 57
cc 7 11 57
cc 8 11 57
cc 14 11 57
cc 17 11 57
cc 24 11 57
cc 25 11 57
cc 26 11 57
sleep 20.0
cc 6 11 58
cc 7 11 58
cc 8 11 58
cc 14 11 58
cc 17 11 58
cc 24 11 58
cc 25 11 58
cc 26 11 58
sleep 20.0
cc 6 11 59
cc 7 11 59
cc 8 11 59
cc 14 11 59
cc 17 11 59
cc 24 11 59
cc 25 11 59
cc 26 11 59
sleep 20.0
cc 6 11 60
cc 7 11 60
cc 8 11 60
cc 14 11 60
cc 17 11 60
cc 24 11 60
cc 25 11 60
cc 26 11 60
sleep 10.416
noteoff 24 52 0
noteoff 25 52 0
noteoff 26 40 0
noteon 24 64 120
noteon 25 64 120
noteon 26 52 120
sleep 9.583
cc 6 11 61
cc 7 11 61
cc 8 11 61
cc 14 11 61
cc 17 11 61
cc 24 11 61
cc 25 11 61
cc 26 11 61
sleep 20.0
cc 6 11 62
cc 7 11 62
cc 8 11 62
cc 14 11 62
cc 17 11 62
cc 24 11 62
cc 25 11 62
cc 26 11 62
sleep 20.0
cc 6 11 63
cc 7 11 63
cc 8 11 63
cc 14 11 63
cc 17 11 63
cc 24 11 63
cc 25 11 63
cc 26 11 63
sleep 20.0
cc 6 11 64
cc 7 11 64
cc 8 11 64
cc 14 11 64
cc 17 11 64
cc 24 11 64
cc 25 11 64
cc 26 11 64
sleep 20.0
cc 6 11 65
cc 7 11 65
cc 8 11 65
cc 14 11 65
cc 17 11 65
cc 24 11 65
cc 25 11 65
cc 26 11 65
sleep 20.0
cc 6 11 66
cc 7 11 66
cc 8 11 66
cc 14 11 66
cc 17 11 66
cc 24 11 66
cc 25 11 66
cc 26 11 66
sleep 20.0
cc 6 11 67
cc 7 11 67
cc 8 11 67
cc 14 11 67
cc 17 11 67
sleep 0.416
cc 24 11 67
cc 25 11 67
cc 26 11 67
sleep 19.583
cc 6 11 68
cc 7 11 68
cc 8 11 68
cc 14 11 68
cc 17 11 68
sleep 0.416
cc 24 11 68
cc 25 11 68
cc 26 11 68
sleep 19.583
cc 6 11 69
cc 7 11 69
cc 8 11 69
cc 14 11 69
cc 17 11 69
sleep 0.416
cc 24 11 69
cc 25 11 69
cc 26 11 69
sleep 19.583
cc 6 11 70
cc 7 11 70
cc 8 11 70
cc 14 11 70
cc 17 11 70
sleep 0.416
cc 24 11 70
cc 25 11 70
cc 26 11 70
sleep 10.0
noteoff 24 64 0
noteoff 25 64 0
noteoff 26 52 0
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
sleep 9.583
cc 6 11 71
cc 7 11 71
cc 8 11 71
cc 14 11 71
cc 17 11 71
sleep 0.416
cc 24 11 71
cc 25 11 71
cc 26 11 71
sleep 20.0
cc 6 11 72
cc 7 11 72
cc 8 11 72
cc 14 11 72
cc 17 11 72
cc 24 11 72
cc 25 11 72
cc 26 11 72
sleep 20.0
cc 6 11 73
cc 7 11 73
cc 8 11 73
cc 14 11 73
cc 17 11 73
cc 24 11 73
cc 25 11 73
cc 26 11 73
sleep 20.0
cc 6 11 74
cc 7 11 74
cc 8 11 74
cc 14 11 74
cc 17 11 74
cc 24 11 74
cc 25 11 74
cc 26 11 74
sleep 20.0
cc 6 11 75
cc 7 11 75
cc 8 11 75
cc 14 11 75
cc 17 11 75
cc 24 11 75
cc 25 11 75
cc 26 11 75
sleep 20.0
cc 6 11 76
cc 7 11 76
cc 8 11 76
cc 14 11 76
cc 17 11 76
cc 24 11 76
cc 25 11 76
cc 26 11 76
sleep 20.0
cc 6 11 77
cc 7 11 77
cc 8 11 77
cc 14 11 77
cc 17 11 77
cc 24 11 77
cc 25 11 77
cc 26 11 77
sleep 20.0
cc 6 11 78
cc 7 11 78
cc 8 11 78
cc 14 11 78
cc 17 11 78
cc 24 11 78
cc 25 11 78
cc 26 11 78
sleep 20.0
cc 6 11 79
cc 7 11 79
cc 8 11 79
cc 14 11 79
cc 17 11 79
cc 24 11 79
cc 25 11 79
cc 26 11 79
sleep 20.0
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 14 11 80
cc 17 11 80
cc 24 11 80
cc 25 11 80
cc 26 11 80
sleep 10.0
echo "measure 126 - $$ Page 44, Bottom, 9th"
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 6 57 120
noteon 7 57 120
noteon 8 71 120
noteon 10 59 120
noteon 14 69 120
noteon 24 61 120
noteon 25 61 120
noteon 26 49 120
cc 10 11 80
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 69 0
noteoff 14 67 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 14 68 0
sleep 200.0
noteoff 17 64 0
noteoff 24 61 0
noteoff 25 61 0
noteoff 26 49 0
noteon 17 62 120
noteon 24 59 120
noteon 25 59 120
noteon 26 47 120
cc 17 11 50
cc 24 11 50
cc 25 11 50
cc 26 11 50
sleep 200.0
noteoff 17 62 0
noteoff 24 59 0
noteoff 25 59 0
noteoff 26 47 0
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 127 - $$ Page 44, Bottom, 10th"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 71 0
noteoff 10 59 0
noteoff 14 69 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 6 57 120
noteon 7 57 120
noteon 8 68 120
noteon 10 64 120
noteon 14 66 120
noteon 17 62 120
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
cc 6 68 127
cc 6 11 50
cc 7 68 127
cc 7 11 50
cc 8 68 127
cc 8 11 50
cc 10 68 127
cc 10 11 50
cc 14 68 127
cc 14 11 50
cc 17 68 127
sleep 200.0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 6 59 120
noteon 7 59 120
noteon 8 69 120
noteon 10 66 120
noteon 14 67 120
noteon 17 64 120
noteon 24 55 120
noteon 25 55 120
noteon 26 43 120
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 68 0
noteoff 10 64 0
noteoff 14 66 0
noteoff 17 62 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
cc 14 68 0
cc 17 68 0
sleep 200.0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 69 0
noteoff 10 66 0
noteoff 14 67 0
noteoff 17 64 0
noteoff 24 55 0
noteoff 25 55 0
noteoff 26 43 0
noteon 2 76 120
noteon 3 73 120
noteon 6 55 120
noteon 7 55 120
noteon 8 66 120
noteon 10 59 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 128 - $$ Page 44, Bottom, 11th"
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 55 0
noteoff 7 55 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 64 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 2 78 120
noteon 3 74 120
noteon 6 54 120
noteon 7 54 120
noteon 8 64 120
noteon 10 56 120
noteon 14 62 120
noteon 17 62 120
noteon 24 50 120
noteon 25 50 120
noteon 26 38 120
cc 2 68 127
cc 3 68 127
sleep 200.0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 14 62 0
noteoff 17 62 0
noteoff 24 50 0
noteoff 25 50 0
noteoff 26 38 0
noteon 2 79 120
noteon 3 76 120
noteoff 2 78 0
noteoff 3 74 0
cc 2 68 0
cc 3 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteon 2 76 120
noteon 3 73 120
noteon 6 55 120
noteon 7 55 120
noteon 8 66 120
noteon 10 59 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 129 - $$ Page 44, Bottom, 12th"
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 55 0
noteoff 7 55 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 64 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 2 74 120
noteon 3 74 120
noteon 6 57 120
noteon 7 57 120
noteon 8 68 120
noteon 10 64 120
noteon 14 66 120
noteon 17 62 120
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
cc 6 68 127
cc 7 68 127
cc 8 68 127
cc 10 68 127
cc 14 68 127
cc 17 68 127
sleep 200.0
noteoff 2 74 0
noteoff 3 74 0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 6 59 120
noteon 7 59 120
noteon 8 69 120
noteon 10 66 120
noteon 14 67 120
noteon 17 64 120
noteon 24 55 120
noteon 25 55 120
noteon 26 43 120
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 68 0
noteoff 10 64 0
noteoff 14 66 0
noteoff 17 62 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
cc 14 68 0
cc 17 68 0
sleep 200.0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 69 0
noteoff 10 66 0
noteoff 14 67 0
noteoff 17 64 0
noteoff 24 55 0
noteoff 25 55 0
noteoff 26 43 0
noteon 2 76 120
noteon 3 73 120
noteon 6 55 120
noteon 7 55 120
noteon 8 66 120
noteon 10 59 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 130 - $$ Page 44, Bottom, 13th"
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 55 0
noteoff 7 55 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 64 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 2 78 120
noteon 3 74 120
noteon 6 54 120
noteon 7 54 120
noteon 8 64 120
noteon 10 56 120
noteon 14 62 120
noteon 17 62 120
noteon 24 50 120
noteon 25 50 120
noteon 26 38 120
sleep 9.583
cc 2 11 51
cc 3 11 51
cc 6 11 51
cc 7 11 51
cc 8 11 51
cc 10 11 51
cc 14 11 51
cc 17 11 51
cc 24 11 51
cc 25 11 51
cc 26 11 51
sleep 20.0
cc 2 11 52
cc 3 11 52
cc 6 11 52
cc 7 11 52
cc 8 11 52
cc 10 11 52
cc 14 11 52
cc 17 11 52
cc 24 11 52
cc 25 11 52
cc 26 11 52
sleep 20.0
cc 2 11 53
cc 3 11 53
cc 6 11 53
cc 7 11 53
cc 8 11 53
cc 10 11 53
cc 14 11 53
cc 17 11 53
cc 24 11 53
cc 25 11 53
cc 26 11 53
sleep 20.0
cc 2 11 54
cc 3 11 54
cc 6 11 54
cc 7 11 54
cc 8 11 54
cc 10 11 54
cc 14 11 54
cc 17 11 54
cc 24 11 54
cc 25 11 54
cc 26 11 54
sleep 20.0
cc 2 11 55
cc 3 11 55
cc 6 11 55
cc 7 11 55
cc 8 11 55
cc 10 11 55
cc 14 11 55
cc 17 11 55
cc 24 11 55
cc 25 11 55
cc 26 11 55
sleep 20.0
cc 2 11 56
cc 3 11 56
cc 6 11 56
cc 7 11 56
cc 8 11 56
cc 10 11 56
cc 14 11 56
cc 17 11 56
cc 24 11 56
cc 25 11 56
cc 26 11 56
sleep 20.0
cc 2 11 57
cc 3 11 57
cc 6 11 57
cc 7 11 57
cc 8 11 57
cc 10 11 57
cc 14 11 57
cc 17 11 57
cc 24 11 57
cc 25 11 57
cc 26 11 57
sleep 20.0
cc 2 11 58
cc 3 11 58
cc 6 11 58
cc 7 11 58
cc 8 11 58
cc 10 11 58
cc 14 11 58
cc 17 11 58
cc 24 11 58
cc 25 11 58
cc 26 11 58
sleep 20.0
cc 2 11 59
cc 3 11 59
cc 6 11 59
cc 7 11 59
cc 8 11 59
cc 10 11 59
cc 14 11 59
cc 17 11 59
cc 24 11 59
cc 25 11 59
cc 26 11 59
sleep 20.0
cc 2 11 60
cc 3 11 60
cc 6 11 60
cc 7 11 60
cc 8 11 60
cc 10 11 60
cc 14 11 60
cc 17 11 60
cc 24 11 60
cc 25 11 60
cc 26 11 60
sleep 10.416
noteoff 2 78 0
noteoff 3 74 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 14 62 0
noteoff 17 62 0
noteoff 24 50 0
noteoff 25 50 0
noteoff 26 38 0
noteon 2 79 120
noteon 3 76 120
sleep 9.583
cc 2 11 61
cc 3 11 61
sleep 20.0
cc 2 11 62
cc 3 11 62
sleep 20.0
cc 2 11 63
cc 3 11 63
sleep 20.0
cc 2 11 64
cc 3 11 64
sleep 20.0
cc 2 11 65
cc 3 11 65
sleep 20.0
cc 2 11 66
cc 3 11 66
sleep 20.416
cc 2 11 67
cc 3 11 67
sleep 20.0
cc 2 11 68
cc 3 11 68
sleep 20.0
cc 2 11 69
cc 3 11 69
sleep 20.0
cc 2 11 70
cc 3 11 70
sleep 10.0
noteoff 2 79 0
noteoff 3 76 0
noteon 2 76 120
noteon 3 73 120
noteon 8 66 120
noteon 10 59 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 10.0
cc 2 11 71
cc 3 11 71
cc 8 11 71
cc 10 11 71
cc 14 11 71
cc 17 11 71
cc 24 11 71
cc 25 11 71
cc 26 11 71
sleep 20.0
cc 2 11 72
cc 3 11 72
cc 8 11 72
cc 10 11 72
cc 14 11 72
cc 17 11 72
cc 24 11 72
cc 25 11 72
cc 26 11 72
sleep 20.0
cc 2 11 73
cc 3 11 73
cc 8 11 73
cc 10 11 73
cc 14 11 73
cc 17 11 73
cc 24 11 73
cc 25 11 73
cc 26 11 73
sleep 20.0
cc 2 11 74
cc 3 11 74
cc 8 11 74
cc 10 11 74
cc 14 11 74
cc 17 11 74
cc 24 11 74
cc 25 11 74
cc 26 11 74
sleep 20.0
cc 2 11 75
cc 3 11 75
cc 8 11 75
cc 10 11 75
cc 14 11 75
cc 17 11 75
cc 24 11 75
cc 25 11 75
cc 26 11 75
sleep 10.0
noteoff 2 76 0
noteoff 3 73 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 64 0
noteoff 17 61 0
sleep 10.0
cc 24 11 76
cc 25 11 76
cc 26 11 76
sleep 20.0
cc 24 11 77
cc 25 11 77
cc 26 11 77
sleep 20.0
cc 24 11 78
cc 25 11 78
cc 26 11 78
sleep 20.0
cc 24 11 79
cc 25 11 79
cc 26 11 79
sleep 20.0
cc 24 11 80
cc 25 11 80
cc 26 11 80
sleep 10.0
echo "measure 131 - $$ Page 44, Bottom, 14th"
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 2 78 120
noteon 3 74 120
noteon 8 68 120
noteon 10 64 120
noteon 14 66 120
noteon 17 62 120
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
cc 2 68 127
cc 2 11 80
cc 3 68 127
cc 3 11 80
cc 8 68 127
cc 8 11 80
cc 10 68 127
cc 10 11 80
cc 14 11 80
cc 17 11 80
sleep 100.0
noteoff 14 66 0
noteoff 17 62 0
sleep 100.0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 2 79 120
noteon 3 76 120
noteon 8 69 120
noteon 10 66 120
noteoff 2 78 0
noteoff 3 74 0
noteoff 8 68 0
noteoff 10 64 0
cc 2 68 0
cc 3 68 0
cc 8 68 0
cc 10 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteoff 8 69 0
noteoff 10 66 0
noteon 2 76 120
noteon 3 73 120
noteon 8 66 120
noteon 10 59 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 132 - $$ Page 44, Bottom, 15th"
noteoff 2 76 0
noteoff 3 73 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 64 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 2 78 120
noteon 3 74 120
noteon 6 57 120
noteon 7 57 120
noteon 8 68 120
noteon 10 64 120
noteon 14 66 120
noteon 17 62 120
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
sleep 100.0
noteoff 14 66 0
noteoff 17 62 0
sleep 100.0
noteoff 2 78 0
noteoff 3 74 0
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 68 0
noteoff 10 64 0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 2 79 120
noteon 3 76 120
noteon 8 69 120
noteon 10 66 120
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteoff 8 69 0
noteoff 10 66 0
noteon 0 88 120
noteon 1 85 120
noteon 2 76 120
noteon 3 73 120
noteon 4 73 120
noteon 5 64 120
noteon 6 45 120
noteon 7 45 120
noteon 8 66 120
noteon 10 59 120
noteon 14 73 120
noteon 17 64 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
cc 2 11 50
cc 3 11 50
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 14 11 50
cc 17 11 50
cc 24 11 50
cc 25 11 50
cc 26 11 50
sleep 200.0
echo "measure 133 - $$ Page 44, Bottom, 16th (last)"
noteoff 0 88 0
noteoff 1 85 0
noteoff 2 76 0
noteoff 3 73 0
noteoff 4 73 0
noteoff 5 64 0
noteoff 6 45 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 73 0
noteoff 17 64 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 0 90 120
noteon 1 86 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 57 120
noteon 7 62 120
noteon 8 68 120
noteon 10 64 120
noteon 14 74 120
noteon 17 66 120
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
cc 0 68 127
cc 1 68 127
cc 2 68 127
cc 3 68 127
cc 4 68 127
cc 5 68 127
cc 6 68 127
cc 7 68 127
cc 8 68 127
cc 10 68 127
cc 14 68 127
cc 17 68 127
sleep 200.0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 0 91 120
noteon 1 88 120
noteon 2 79 120
noteon 3 76 120
noteon 4 76 120
noteon 5 67 120
noteon 6 59 120
noteon 7 43 120
noteon 8 69 120
noteon 10 66 120
noteon 14 76 120
noteon 17 67 120
noteon 24 55 120
noteon 25 55 120
noteon 26 43 120
noteoff 0 90 0
noteoff 1 86 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 57 0
noteoff 7 62 0
noteoff 8 68 0
noteoff 10 64 0
noteoff 14 74 0
noteoff 17 66 0
cc 0 68 0
cc 1 68 0
cc 2 68 0
cc 3 68 0
cc 4 68 0
cc 5 68 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
cc 14 68 0
cc 17 68 0
sleep 200.0
noteoff 0 91 0
noteoff 1 88 0
noteoff 2 79 0
noteoff 3 76 0
noteoff 4 76 0
noteoff 5 67 0
noteoff 6 59 0
noteoff 7 43 0
noteoff 8 69 0
noteoff 10 66 0
noteoff 14 76 0
noteoff 17 67 0
noteoff 24 55 0
noteoff 25 55 0
noteoff 26 43 0
noteon 0 88 120
noteon 1 85 120
noteon 2 76 120
noteon 3 73 120
noteon 4 73 120
noteon 5 64 120
noteon 6 61 120
noteon 7 45 120
noteon 8 66 120
noteon 10 71 120
noteon 14 73 120
noteon 17 64 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 1 - $$ Page 40, Top, 1st"
noteoff 0 88 0
noteoff 1 85 0
noteoff 2 76 0
noteoff 3 73 0
noteoff 4 73 0
noteoff 5 64 0
noteoff 6 61 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 71 0
noteoff 14 73 0
noteoff 17 64 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 0 86 120
noteon 1 86 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 8 64 120
noteon 10 56 120
noteon 14 74 120
noteon 17 66 120
noteon 24 50 120
noteon 25 50 120
noteon 26 38 120
sleep 200.0
noteoff 0 86 0
noteoff 1 86 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 14 74 0
noteoff 17 66 0
noteoff 24 50 0
noteoff 25 50 0
noteoff 26 38 0
sleep 200.0
sleep 200.0
echo "measure 2 - $$ Page 40, Top, 2nd"
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 74 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 12 11 80
cc 13 11 80
cc 14 11 80
cc 15 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 3 - $$ Page 40, Top, 3rd"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 74 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 4 - $$ Page 40, Top, 4th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 76 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 5 - $$ Page 40, Top, 5th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 76 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 6 - $$ Page 40, Top, 6th"
noteoff 14 83 0
noteon 8 66 120
noteon 10 59 120
cc 8 11 50
cc 10 11 50
sleep 200.0
noteoff 8 66 0
noteoff 10 59 0
noteon 8 68 120
noteon 10 64 120
sleep 200.0
noteoff 8 68 0
noteoff 10 64 0
noteon 8 69 120
noteon 10 66 120
sleep 200.0
echo "measure 7 - $$ Page 40, Top, 7th"
noteoff 8 69 0
noteoff 10 66 0
noteon 14 76 120
noteon 17 69 120
cc 17 11 50
sleep 200.0
noteoff 14 76 0
noteoff 17 69 0
noteon 14 78 120
noteon 17 74 120
sleep 200.0
noteoff 14 78 0
noteoff 17 74 0
noteon 14 79 120
noteon 17 76 120
sleep 200.0
echo "measure 8 - $$ Page 40, Top, 8th"
noteoff 14 79 0
noteoff 17 76 0
noteon 2 78 120
noteon 3 74 120
noteon 8 68 120
noteon 10 64 120
cc 2 11 50
cc 3 11 50
sleep 200.0
noteoff 2 78 0
noteoff 3 74 0
noteoff 8 68 0
noteoff 10 64 0
noteon 2 76 120
noteon 3 69 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 66 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
echo "measure 9 - $$ Page 40, Top, 9th"
noteoff 2 74 0
noteoff 3 66 0
noteoff 8 64 0
noteoff 10 56 0
noteon 0 86 120
noteon 1 78 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 90
cc 1 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 78 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 1 79 120
noteon 4 76 120
noteon 5 67 120
noteon 6 57 120
noteon 7 45 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 0 85 0
noteoff 1 79 0
noteoff 4 76 0
noteoff 5 67 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 0 86 120
noteon 1 78 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
echo "measure 10 - $$ Page 40, Top, 10th"
noteoff 0 86 0
noteoff 1 78 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 12 11 80
cc 13 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 11 - $$ Page 40, Top, 11th"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 12 - $$ Page 40, Top, 12th (last)"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 56 120
noteon 7 56 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 64 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 56 120
noteon 22 56 120
noteon 23 44 120
cc 14 11 80
sleep 200.0
noteoff 6 56 0
noteoff 7 56 0
noteoff 13 50 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 44 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 13 - $$ Page 40, Bottom, 1st"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 64 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 83 120
cc 14 11 50
sleep 200.0
noteoff 14 83 0
noteon 14 85 120
sleep 200.0
noteoff 14 85 0
noteon 14 86 120
sleep 200.0
echo "measure 14 - $$ Page 40, Bottom, 2nd"
noteoff 14 86 0
noteon 2 73 120
noteon 3 69 120
noteon 17 61 120
noteon 20 57 120
cc 2 11 50
cc 3 11 50
cc 17 11 50
cc 20 11 50
sleep 200.0
noteoff 2 73 0
noteoff 3 69 0
noteoff 17 61 0
noteoff 20 57 0
noteon 2 74 120
noteon 3 71 120
noteon 17 62 120
noteon 20 59 120
sleep 200.0
noteoff 2 74 0
noteoff 3 71 0
noteoff 17 62 0
noteoff 20 59 0
noteon 2 76 120
noteon 3 73 120
noteon 17 64 120
noteon 20 61 120
sleep 200.0
echo "measure 15 - $$ Page 40, Bottom, 3rd"
noteoff 2 76 0
noteoff 3 73 0
noteoff 17 64 0
noteoff 20 61 0
noteon 14 83 120
noteon 17 76 120
sleep 200.0
noteoff 14 83 0
noteoff 17 76 0
noteon 14 85 120
noteon 17 81 120
sleep 200.0
noteoff 14 85 0
noteoff 17 81 0
noteon 14 86 120
noteon 17 83 120
sleep 200.0
echo "measure 16 - $$ Page 40, Bottom, 4th"
noteoff 14 86 0
noteoff 17 83 0
noteon 0 85 120
noteon 2 81 120
noteon 3 73 120
noteon 6 57 120
noteon 8 66 120
cc 0 11 50
cc 6 11 50
cc 8 11 50
sleep 200.0
noteoff 0 85 0
noteoff 2 81 0
noteoff 3 73 0
noteoff 6 57 0
noteoff 8 66 0
noteon 0 83 120
noteon 2 80 120
noteon 3 71 120
noteon 6 52 120
noteon 8 66 120
sleep 200.0
noteoff 0 83 0
noteoff 2 80 0
noteoff 3 71 0
noteoff 6 52 0
noteoff 8 66 0
noteon 0 81 120
noteon 2 81 120
noteon 3 73 120
noteon 6 57 120
noteon 8 66 120
sleep 200.0
echo "measure 1 - $$ Page 40, Top, 1st"
noteoff 0 81 0
noteoff 2 81 0
noteoff 3 73 0
noteoff 6 57 0
noteoff 8 66 0
noteon 0 88 120
noteon 1 85 120
noteon 4 73 120
noteon 5 69 120
noteon 6 57 120
noteon 7 45 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 81 120
noteon 17 73 120
noteon 18 64 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 0 11 90
cc 1 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 8 11 90
cc 10 11 90
cc 11 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 88 0
noteoff 1 85 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 81 0
noteoff 17 73 0
noteoff 18 64 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 88 120
noteon 1 83 120
noteon 4 71 120
noteon 5 68 120
noteon 6 52 120
noteon 7 40 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 80 120
noteon 17 74 120
noteon 18 64 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 0 88 0
noteoff 1 83 0
noteoff 4 71 0
noteoff 5 68 0
noteoff 6 52 0
noteoff 7 40 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 80 0
noteoff 17 74 0
noteoff 18 64 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 0 88 120
noteon 1 85 120
noteon 4 73 120
noteon 5 69 120
noteon 6 57 120
noteon 7 45 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 81 120
noteon 17 73 120
noteon 18 64 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
echo "measure 2 - $$ Page 40, Top, 2nd"
noteoff 0 88 0
noteoff 1 85 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 81 0
noteoff 17 73 0
noteoff 18 64 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 74 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 13 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 3 - $$ Page 40, Top, 3rd"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 74 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 4 - $$ Page 40, Top, 4th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 76 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 5 - $$ Page 40, Top, 5th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 76 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 6 - $$ Page 40, Top, 6th"
noteoff 14 83 0
noteon 8 66 120
noteon 10 59 120
cc 8 11 50
cc 10 11 50
sleep 200.0
noteoff 8 66 0
noteoff 10 59 0
noteon 8 68 120
noteon 10 64 120
sleep 200.0
noteoff 8 68 0
noteoff 10 64 0
noteon 8 69 120
noteon 10 66 120
sleep 200.0
echo "measure 7 - $$ Page 40, Top, 7th"
noteoff 8 69 0
noteoff 10 66 0
noteon 14 76 120
noteon 17 69 120
cc 17 11 50
sleep 200.0
noteoff 14 76 0
noteoff 17 69 0
noteon 14 78 120
noteon 17 74 120
sleep 200.0
noteoff 14 78 0
noteoff 17 74 0
noteon 14 79 120
noteon 17 76 120
sleep 200.0
echo "measure 8 - $$ Page 40, Top, 8th"
noteoff 14 79 0
noteoff 17 76 0
noteon 2 78 120
noteon 3 74 120
noteon 8 68 120
noteon 10 64 120
cc 2 11 50
cc 3 11 50
sleep 200.0
noteoff 2 78 0
noteoff 3 74 0
noteoff 8 68 0
noteoff 10 64 0
noteon 2 76 120
noteon 3 69 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 66 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
echo "measure 9 - $$ Page 40, Top, 9th"
noteoff 2 74 0
noteoff 3 66 0
noteoff 8 64 0
noteoff 10 56 0
noteon 0 86 120
noteon 1 78 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 90
cc 1 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 78 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 1 79 120
noteon 4 76 120
noteon 5 67 120
noteon 6 57 120
noteon 7 45 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 0 85 0
noteoff 1 79 0
noteoff 4 76 0
noteoff 5 67 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 0 86 120
noteon 1 78 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
echo "measure 10 - $$ Page 40, Top, 10th"
noteoff 0 86 0
noteoff 1 78 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 12 11 80
cc 13 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 11 - $$ Page 40, Top, 11th"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 12 - $$ Page 40, Top, 12th (last)"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 56 120
noteon 7 56 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 64 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 56 120
noteon 22 56 120
noteon 23 44 120
cc 14 11 80
sleep 200.0
noteoff 6 56 0
noteoff 7 56 0
noteoff 13 50 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 44 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 13 - $$ Page 40, Bottom, 1st"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 64 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 83 120
cc 14 11 50
sleep 200.0
noteoff 14 83 0
noteon 14 85 120
sleep 200.0
noteoff 14 85 0
noteon 14 86 120
sleep 200.0
echo "measure 14 - $$ Page 40, Bottom, 2nd"
noteoff 14 86 0
noteon 2 73 120
noteon 3 69 120
noteon 17 61 120
noteon 20 57 120
cc 2 11 50
cc 3 11 50
cc 17 11 50
cc 20 11 50
sleep 200.0
noteoff 2 73 0
noteoff 3 69 0
noteoff 17 61 0
noteoff 20 57 0
noteon 2 74 120
noteon 3 71 120
noteon 17 62 120
noteon 20 59 120
sleep 200.0
noteoff 2 74 0
noteoff 3 71 0
noteoff 17 62 0
noteoff 20 59 0
noteon 2 76 120
noteon 3 73 120
noteon 17 64 120
noteon 20 61 120
sleep 200.0
echo "measure 15 - $$ Page 40, Bottom, 3rd"
noteoff 2 76 0
noteoff 3 73 0
noteoff 17 64 0
noteoff 20 61 0
noteon 14 83 120
noteon 17 76 120
sleep 200.0
noteoff 14 83 0
noteoff 17 76 0
noteon 14 85 120
noteon 17 81 120
sleep 200.0
noteoff 14 85 0
noteoff 17 81 0
noteon 14 86 120
noteon 17 83 120
sleep 200.0
echo "measure 16 - $$ Page 40, Bottom, 4th"
noteoff 14 86 0
noteoff 17 83 0
noteon 0 85 120
noteon 2 81 120
noteon 3 73 120
noteon 6 57 120
noteon 8 66 120
cc 0 11 50
cc 6 11 50
cc 8 11 50
sleep 200.0
noteoff 0 85 0
noteoff 2 81 0
noteoff 3 73 0
noteoff 6 57 0
noteoff 8 66 0
noteon 0 83 120
noteon 2 80 120
noteon 3 71 120
noteon 6 52 120
noteon 8 66 120
sleep 200.0
noteoff 0 83 0
noteoff 2 80 0
noteoff 3 71 0
noteoff 6 52 0
noteoff 8 66 0
noteon 0 81 120
noteon 2 81 120
noteon 3 73 120
noteon 6 57 120
noteon 8 66 120
sleep 200.0
echo "measure 17 - $$ Page 40, Bottom, 5th"
noteoff 0 81 0
noteoff 2 81 0
noteoff 3 73 0
noteoff 6 57 0
noteoff 8 66 0
noteon 0 88 120
noteon 1 85 120
noteon 4 73 120
noteon 5 69 120
noteon 6 57 120
noteon 7 45 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 81 120
noteon 17 73 120
noteon 18 64 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 0 11 90
cc 1 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 8 11 90
cc 10 11 90
cc 11 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 88 0
noteoff 1 85 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 81 0
noteoff 17 73 0
noteoff 18 64 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 88 120
noteon 1 83 120
noteon 4 71 120
noteon 5 68 120
noteon 6 52 120
noteon 7 40 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 80 120
noteon 17 74 120
noteon 18 64 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 0 88 0
noteoff 1 83 0
noteoff 4 71 0
noteoff 5 68 0
noteoff 6 52 0
noteoff 7 40 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 80 0
noteoff 17 74 0
noteoff 18 64 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 0 88 120
noteon 1 85 120
noteon 4 73 120
noteon 5 69 120
noteon 6 57 120
noteon 7 45 120
noteon 8 66 120
noteon 10 66 120
noteon 11 76 120
noteon 13 45 120
noteon 14 81 120
noteon 17 73 120
noteon 18 64 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
echo "measure 18 - $$ Page 40, Bottom, 6th"
noteoff 0 88 0
noteoff 1 85 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 66 0
noteoff 11 76 0
noteoff 13 45 0
noteoff 14 81 0
noteoff 17 73 0
noteoff 18 64 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 14 69 120
noteon 17 61 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
cc 14 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 200.0
noteoff 17 61 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 17 62 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
noteoff 17 62 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 17 64 120
noteon 20 61 120
noteon 22 61 120
noteon 23 49 120
sleep 200.0
echo "measure 19 - $$ Page 40, Bottom, 7th"
noteoff 14 69 0
noteoff 17 64 0
noteoff 20 61 0
noteoff 22 61 0
noteoff 23 49 0
noteon 14 77 120
noteon 17 65 120
noteon 20 62 120
noteon 22 62 120
noteon 23 50 120
sleep 200.0
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 62 0
noteoff 22 62 0
noteoff 23 50 0
noteon 14 76 120
sleep 200.0
noteoff 14 76 0
noteon 14 77 120
sleep 200.0
echo "measure 20 - $$ Page 40, Bottom, 8th"
noteoff 14 77 0
noteon 14 65 120
noteon 17 57 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 17 57 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 17 58 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
sleep 200.0
noteoff 17 58 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 17 60 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
echo "measure 21 - $$ Page 40, Bottom, 9th"
noteoff 14 65 0
noteoff 17 60 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 14 74 120
noteon 17 62 120
noteon 20 58 120
noteon 22 58 120
noteon 23 46 120
sleep 200.0
noteoff 14 74 0
noteoff 17 62 0
noteoff 20 58 0
noteoff 22 58 0
noteoff 23 46 0
noteon 14 73 120
sleep 200.0
noteoff 14 73 0
noteon 14 74 120
sleep 200.0
echo "measure 22 - $$ Page 40, Bottom, 10th"
noteoff 14 74 0
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 18 11 50
sleep 200.0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 77 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 400.0
echo "measure 23 - $$ Page 40, Bottom, 11th"
cc 14 11 50
cc 17 11 50
cc 18 11 50
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 400.0
noteoff 14 77 0
noteon 14 74 120
cc 14 68 127
sleep 100.0
noteon 14 70 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 24 - $$ Page 40, Bottom, 12th (last)"
noteoff 14 70 0
noteoff 17 62 0
noteoff 18 58 0
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
cc 14 68 127
sleep 100.0
noteon 14 72 120
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 75 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 72 0
sleep 100.0
noteon 14 74 120
noteoff 14 75 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 72 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 74 0
sleep 100.0
noteon 14 70 120
noteoff 14 72 0
sleep 100.0
echo "measure 25 - $$ Page 41, Top, 1st"
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 70 0
sleep 100.0
noteon 14 67 120
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 65 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 67 0
sleep 100.0
noteon 14 63 120
noteoff 14 65 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 81 120
noteon 6 57 120
noteon 14 62 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 0 68 127
cc 0 11 50
cc 6 68 127
cc 6 11 50
noteoff 14 63 0
sleep 100.0
noteon 14 60 120
noteoff 14 62 0
cc 14 68 0
sleep 100.0
echo "measure 26 - $$ Page 41, Top, 2nd"
noteoff 14 60 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 82 120
noteon 2 74 120
noteon 3 70 120
noteon 6 58 120
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 81 0
noteoff 6 57 0
cc 0 68 0
cc 6 68 0
sleep 200.0
noteoff 0 82 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 89 120
noteon 2 74 120
noteon 3 70 120
noteon 6 65 120
noteon 14 77 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 0 11 80
cc 2 11 80
cc 3 11 80
cc 6 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 400.0
cc 0 11 50
cc 2 11 50
echo "measure 27 - $$ Page 41, Top, 3rd"
cc 3 11 50
cc 6 11 50
cc 14 11 50
cc 17 11 50
cc 18 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 400.0
noteoff 0 89 0
noteoff 6 65 0
noteoff 14 77 0
noteon 0 86 120
noteon 6 62 120
noteon 14 74 120
cc 0 68 127
cc 6 68 127
cc 14 68 127
sleep 100.0
noteon 0 82 120
noteon 6 58 120
noteon 14 70 120
noteoff 0 86 0
noteoff 6 62 0
noteoff 14 74 0
cc 0 68 0
cc 6 68 0
cc 14 68 0
sleep 100.0
echo "measure 28 - $$ Page 41, Top, 4th"
noteoff 0 82 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 70 0
noteoff 17 62 0
noteoff 18 58 0
noteon 0 81 120
noteon 2 75 120
noteon 3 72 120
noteon 6 57 120
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
cc 0 68 127
cc 14 68 127
sleep 100.0
noteon 0 84 120
noteon 14 72 120
noteoff 0 81 0
noteoff 14 69 0
sleep 100.0
noteoff 6 57 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 87 120
noteon 14 75 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 84 0
noteoff 14 72 0
sleep 100.0
noteon 0 86 120
noteon 14 74 120
noteoff 0 87 0
noteoff 14 75 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 84 120
noteon 14 72 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 86 0
noteoff 14 74 0
sleep 100.0
noteon 0 82 120
noteon 14 70 120
noteoff 0 84 0
noteoff 14 72 0
sleep 100.0
echo "measure 29 - $$ Page 41, Top, 5th"
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 81 120
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 82 0
noteoff 14 70 0
sleep 100.0
noteon 0 79 120
noteon 14 67 120
noteoff 0 81 0
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 77 120
noteon 6 53 120
noteon 14 65 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 79 0
noteoff 14 67 0
sleep 100.0
noteon 0 75 120
noteon 14 63 120
noteoff 0 77 0
noteoff 14 65 0
sleep 100.0
noteoff 6 53 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 74 120
noteon 6 53 120
noteon 14 62 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 75 0
noteoff 14 63 0
sleep 100.0
noteon 0 72 120
noteon 14 60 120
noteoff 0 74 0
noteoff 14 62 0
cc 0 68 0
sleep 100.0
echo "measure 30 - $$ Page 41, Top, 6th"
noteoff 0 72 0
noteoff 2 75 0
noteoff 3 72 0
noteoff 6 53 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 70 120
noteon 2 74 120
noteon 3 70 120
noteon 6 58 120
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 60 0
cc 14 68 0
sleep 200.0
noteoff 0 70 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 74 120
noteon 17 62 120
noteon 20 58 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 58 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 31 - $$ Page 41, Top, 7th"
noteoff 14 76 0
noteoff 17 65 0
noteoff 20 62 0
noteon 6 57 120
noteon 14 77 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 6 57 0
noteoff 14 77 0
noteoff 22 45 0
noteoff 23 33 0
noteon 14 74 120
noteon 17 62 120
noteon 20 57 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 57 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 32 - $$ Page 41, Top, 8th"
noteoff 14 76 0
noteoff 17 65 0
noteoff 20 62 0
noteon 6 56 120
noteon 14 77 120
noteon 22 44 120
noteon 23 32 120
sleep 200.0
noteoff 6 56 0
noteoff 14 77 0
noteoff 22 44 0
noteoff 23 32 0
noteon 14 74 120
noteon 17 62 120
noteon 20 59 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 59 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
sleep 100.0
echo "measure 33 - $$ Page 41, Top, 9th"
noteoff 17 65 0
noteoff 20 62 0
noteon 6 56 120
noteon 14 77 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
noteon 22 56 120
noteon 23 44 120
noteoff 14 76 0
cc 14 68 0
sleep 200.0
noteoff 6 56 0
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteoff 22 56 0
noteoff 23 44 0
noteon 14 79 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
cc 14 68 127
sleep 100.0
noteon 14 77 120
noteoff 14 79 0
sleep 100.0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteon 14 76 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
noteoff 14 77 0
sleep 100.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 100.0
echo "measure 34 - $$ Page 41, Top, 10th (last)"
noteoff 14 74 0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteon 6 57 120
noteon 14 73 120
noteon 17 64 120
noteon 20 61 120
noteon 21 57 120
noteon 22 57 120
noteon 23 33 120
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 21 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 6 57 0
noteoff 14 73 0
noteoff 17 64 0
noteoff 20 61 0
noteoff 21 57 0
noteoff 22 57 0
noteon 14 74 120
cc 14 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 71 120
noteon 20 49 120
noteon 22 49 120
cc 20 68 127
cc 22 68 127
noteoff 14 73 0
sleep 100.0
noteon 14 69 120
noteoff 14 71 0
cc 14 68 0
sleep 100.0
echo "measure 35 - $$ Page 41, Bottom, 1st"
noteoff 14 69 0
noteon 14 77 120
noteon 20 50 120
noteon 22 50 120
noteoff 20 49 0
noteoff 22 49 0
sleep 200.0
noteoff 14 77 0
noteon 14 79 120
noteon 20 53 120
noteon 22 53 120
cc 14 68 127
noteoff 20 50 0
noteoff 22 50 0
sleep 100.0
noteon 14 77 120
noteoff 14 79 0
sleep 100.0
noteon 14 76 120
noteon 20 56 120
noteon 22 56 120
noteoff 14 77 0
noteoff 20 53 0
noteoff 22 53 0
cc 20 68 0
cc 22 68 0
sleep 100.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 100.0
echo "measure 36 - $$ Page 41, Bottom, 2nd"
noteoff 14 74 0
noteoff 20 56 0
noteoff 22 56 0
noteon 14 73 120
noteon 20 57 120
noteon 22 57 120
sleep 100.0
noteoff 20 57 0
noteoff 22 57 0
sleep 100.0
noteoff 14 73 0
noteon 14 74 120
cc 14 68 127
sleep 25.0
cc 14 11 41
sleep 50.0
cc 14 11 42
sleep 25.0
noteon 14 73 120
noteoff 14 74 0
sleep 25.0
cc 14 11 43
sleep 50.0
cc 14 11 44
sleep 25.0
noteon 14 71 120
noteon 17 61 120
noteon 20 49 120
noteon 22 49 120
cc 17 68 127
cc 20 68 127
cc 22 68 127
noteoff 14 73 0
sleep 25.0
cc 14 11 45
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 50.0
cc 14 11 46
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 25.0
noteon 14 69 120
noteoff 14 71 0
cc 14 68 0
sleep 25.0
cc 14 11 47
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 50.0
cc 14 11 48
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 25.0
echo "measure 37 - $$ Page 41, Bottom, 3rd"
noteoff 14 69 0
noteon 14 77 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteoff 17 61 0
noteoff 20 49 0
noteoff 22 49 0
sleep 25.0
cc 14 11 49
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 50.0
cc 14 11 50
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 25.0
noteoff 14 77 0
sleep 25.0
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 50.0
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 25.0
noteon 14 79 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
cc 14 68 127
cc 14 11 52
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 25.0
cc 14 11 53
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 50.0
cc 14 11 54
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 25.0
noteon 14 77 120
noteoff 14 79 0
sleep 25.0
cc 14 11 55
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 50.0
cc 14 11 56
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 25.0
noteon 14 76 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 25.0
cc 14 11 57
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 50.0
cc 14 11 58
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 25.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 25.0
cc 14 11 59
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 50.0
cc 14 11 60
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 25.0
echo "measure 38 - $$ Page 41, Bottom, 4th"
noteoff 14 74 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteon 14 73 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
sleep 25.0
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 7.916
cc 14 11 61
sleep 42.083
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 24.583
cc 14 11 62
sleep 0.416
noteoff 14 73 0
noteoff 17 69 0
noteoff 20 57 0
noteoff 22 57 0
sleep 100.0
noteoff 23 33 0
noteon 14 76 120
noteon 17 57 120
cc 14 68 127
cc 14 11 63
cc 17 68 127
cc 17 11 60
sleep 25.0
cc 14 11 64
cc 17 11 61
sleep 49.583
cc 14 11 65
sleep 0.416
cc 17 11 62
sleep 25.0
noteon 14 74 120
noteoff 14 76 0
sleep 24.583
cc 14 11 66
sleep 0.416
cc 17 11 63
sleep 49.583
cc 14 11 67
sleep 0.416
cc 17 11 64
sleep 25.0
noteon 14 73 120
noteon 17 58 120
noteoff 14 74 0
noteoff 17 57 0
sleep 24.583
cc 14 11 68
cc 17 11 65
sleep 50.0
cc 14 11 69
cc 17 11 66
sleep 25.416
noteon 14 71 120
noteoff 14 73 0
sleep 24.583
cc 14 11 70
cc 17 11 67
sleep 50.0
cc 14 11 71
cc 17 11 68
sleep 25.416
echo "measure 39 - $$ Page 41, Bottom, 5th"
noteon 14 69 120
noteon 17 59 120
noteoff 14 71 0
noteoff 17 58 0
sleep 24.583
cc 14 11 72
cc 17 11 69
sleep 50.0
cc 14 11 73
cc 17 11 70
sleep 25.416
noteon 14 68 120
noteoff 14 69 0
cc 14 68 0
sleep 24.583
cc 17 11 71
sleep 25.416
cc 14 11 74
sleep 24.583
cc 17 11 72
sleep 25.416
noteoff 14 68 0
noteon 14 69 120
noteon 17 60 120
noteoff 17 59 0
sleep 24.583
cc 14 11 75
cc 17 11 73
sleep 25.416
noteoff 14 69 0
sleep 24.583
cc 17 11 74
sleep 25.416
noteon 14 67 120
cc 14 11 76
sleep 24.583
cc 17 11 75
sleep 25.416
noteoff 14 67 0
sleep 24.583
cc 17 11 76
sleep 25.416
noteon 14 66 120
noteon 17 61 120
cc 14 11 77
noteoff 17 60 0
cc 17 68 0
sleep 24.583
cc 14 11 78
cc 17 11 77
sleep 25.416
noteoff 14 66 0
sleep 24.583
cc 17 11 78
sleep 25.416
noteon 14 64 120
cc 14 11 79
sleep 24.583
cc 17 11 79
sleep 25.416
noteoff 14 64 0
sleep 24.583
cc 17 11 80
sleep 25.416
echo "measure 40 - $$ Page 41, Bottom, 6th"
noteoff 17 61 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 13 11 80
cc 14 11 80
cc 15 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 41 - $$ Page 41, Bottom, 7th"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 42 - $$ Page 41, Bottom, 8th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 67 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 67 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 71 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 43 - $$ Page 41, Bottom, 9th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 71 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 44 - $$ Page 41, Bottom, 10th"
noteoff 14 83 0
noteon 8 66 120
noteon 10 59 120
cc 8 11 50
cc 10 11 50
sleep 200.0
noteoff 8 66 0
noteoff 10 59 0
noteon 8 68 120
noteon 10 64 120
sleep 200.0
noteoff 8 68 0
noteoff 10 64 0
noteon 8 69 120
noteon 10 66 120
sleep 200.0
echo "measure 45 - $$ Page 41, Bottom, 11th"
noteoff 8 69 0
noteoff 10 66 0
noteon 14 76 120
noteon 17 69 120
cc 17 11 50
sleep 200.0
noteoff 14 76 0
noteoff 17 69 0
noteon 14 78 120
noteon 17 74 120
sleep 200.0
noteoff 14 78 0
noteoff 17 74 0
noteon 14 79 120
noteon 17 76 120
sleep 200.0
echo "measure 46 - $$ Page 41, Bottom, 12th (last)"
noteoff 14 79 0
noteoff 17 76 0
noteon 2 78 120
noteon 3 74 120
noteon 8 68 120
noteon 10 64 120
cc 2 11 50
cc 3 11 50
sleep 200.0
noteoff 2 78 0
noteoff 3 74 0
noteoff 8 68 0
noteoff 10 64 0
noteon 2 76 120
noteon 3 69 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 66 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
echo "measure 47 - $$ Page 42, Top, 1st"
noteoff 2 74 0
noteoff 3 66 0
noteoff 8 64 0
noteoff 10 56 0
noteon 0 86 120
noteon 1 78 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 90
cc 1 11 90
cc 2 11 90
cc 3 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 1 76 120
noteon 2 79 120
noteon 3 76 120
noteon 4 76 120
noteon 5 67 120
noteon 6 57 120
noteon 7 45 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 0 85 0
noteoff 1 76 0
noteoff 2 79 0
noteoff 3 76 0
noteoff 4 76 0
noteoff 5 67 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 0 86 120
noteon 1 78 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
echo "measure 48 - $$ Page 42, Top, 2nd"
noteoff 0 86 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 12 11 80
cc 13 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 49 - $$ Page 42, Top, 3rd"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 50 - $$ Page 42, Top, 4th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 51 - $$ Page 42, Top, 5th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 52 - $$ Page 42, Top, 6th"
noteoff 14 83 0
noteon 14 67 120
sleep 200.0
noteoff 14 67 0
noteon 14 69 120
sleep 200.0
noteoff 14 69 0
noteon 14 71 120
sleep 200.0
echo "measure 53 - $$ Page 42, Top, 7th"
noteoff 14 71 0
noteon 17 64 120
cc 17 11 50
sleep 200.0
noteoff 17 64 0
noteon 17 66 120
sleep 200.0
noteoff 17 66 0
noteon 17 67 120
sleep 200.0
echo "measure 54 - $$ Page 42, Top, 8th"
noteoff 17 67 0
noteon 14 67 120
sleep 200.0
noteoff 14 67 0
noteon 14 69 120
sleep 200.0
noteoff 14 69 0
noteon 14 70 120
sleep 200.0
echo "measure 55 - $$ Page 42, Top, 9th"
noteoff 14 70 0
noteon 17 64 120
sleep 200.0
noteoff 17 64 0
noteon 17 66 120
sleep 200.0
noteoff 17 66 0
noteon 17 67 120
sleep 200.0
echo "measure 56 - $$ Page 42, Top, 10th"
noteoff 17 67 0
noteon 14 67 120
sleep 100.0
noteoff 14 67 0
sleep 100.0
noteon 14 69 120
cc 14 11 49
sleep 100.0
noteoff 14 69 0
sleep 100.0
noteon 14 70 120
cc 14 11 48
sleep 100.0
noteoff 14 70 0
sleep 100.0
echo "measure 57 - $$ Page 42, Top, 11th"
noteon 14 67 120
noteon 17 64 120
cc 14 11 47
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 46
cc 17 11 49
sleep 100.0
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 14 11 45
cc 17 11 48
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 58 - $$ Page 42, Top, 12th"
noteon 14 67 120
noteon 17 64 120
cc 14 11 44
cc 17 11 47
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 43
cc 17 11 46
sleep 100.0
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 14 11 42
cc 17 11 45
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 59 - $$ Page 42, Top, 13th"
noteon 14 67 120
noteon 17 64 120
cc 17 11 44
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 41
cc 17 11 43
sleep 49.583
cc 17 11 42
sleep 50.416
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 17 11 41
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 60 - $$ Page 42, Top, 14th (last)"
noteon 2 72 120
noteon 6 60 120
noteon 14 67 120
noteon 17 64 120
noteon 20 60 120
noteon 22 60 120
noteon 23 48 120
cc 2 11 40
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 14 67 0
noteoff 17 64 0
noteoff 20 60 0
noteoff 22 60 0
noteoff 23 48 0
noteon 14 69 120
noteon 17 65 120
sleep 200.0
noteoff 14 69 0
noteoff 17 65 0
noteon 14 70 120
noteon 17 67 120
sleep 200.0
echo "measure 61 - $$ Page 42, Bottom, 1st"
noteoff 2 72 0
noteoff 6 60 0
noteoff 14 70 0
noteoff 17 67 0
noteon 2 77 120
noteon 6 65 120
noteon 14 69 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 69 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 2 76 120
noteon 6 64 120
sleep 200.0
noteoff 2 76 0
noteoff 6 64 0
noteon 2 77 120
noteon 6 65 120
sleep 200.0
echo "measure 62 - $$ Page 42, Bottom, 2nd"
noteoff 2 77 0
noteoff 6 65 0
noteon 2 69 120
noteon 6 57 120
noteon 14 64 120
noteon 17 61 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 14 64 0
noteoff 17 61 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 14 65 120
noteon 17 62 120
sleep 200.0
noteoff 6 57 0
noteoff 14 65 0
noteoff 17 62 0
noteon 14 67 120
noteon 17 64 120
sleep 200.0
echo "measure 63 - $$ Page 42, Bottom, 3rd"
noteoff 2 69 0
noteoff 14 67 0
noteoff 17 64 0
noteon 2 74 120
noteon 6 62 120
noteon 14 65 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 65 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
noteoff 23 38 0
noteon 2 73 120
noteon 6 61 120
sleep 200.0
noteoff 2 73 0
noteoff 6 61 0
noteon 2 74 120
noteon 6 62 120
sleep 200.0
echo "measure 64 - $$ Page 42, Bottom, 4th"
noteoff 2 74 0
noteoff 6 62 0
noteon 4 74 120
noteon 5 62 120
noteon 6 52 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 17 11 50
cc 20 68 127
cc 20 11 50
cc 22 68 127
cc 22 11 50
cc 23 11 50
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 40.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 40.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 40.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 40.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
noteon 6 52 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 52 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 40.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 40.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 40.0
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 40.0
cc 6 11 60
cc 17 11 60
cc 20 11 60
cc 22 11 60
sleep 20.0
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 52 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 9.583
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 20.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 20.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 20.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 20.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 20.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 20.0
cc 6 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 10.416
echo "measure 65 - $$ Page 42, Bottom, 5th"
noteoff 7 46 0
noteon 6 46 120
noteon 7 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
cc 7 68 127
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
cc 6 68 0
sleep 200.0
noteon 7 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 7 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
cc 7 11 65
sleep 19.583
cc 7 11 66
sleep 40.0
cc 7 11 67
sleep 40.0
cc 7 11 68
sleep 40.0
cc 7 11 69
sleep 40.0
cc 7 11 70
sleep 20.416
noteon 7 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 7 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 7 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 10.0
cc 7 11 71
sleep 20.0
cc 7 11 72
sleep 20.0
cc 7 11 73
sleep 20.0
cc 7 11 74
sleep 20.0
cc 7 11 75
sleep 20.0
cc 7 11 76
sleep 20.0
cc 7 11 77
sleep 20.0
cc 7 11 78
sleep 20.0
cc 7 11 79
sleep 20.0
cc 7 11 80
sleep 10.0
echo "measure 66 - $$ Page 42, Bottom, 6th"
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 46 0
noteoff 7 56 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 57 120
noteon 12 69 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 68 127
cc 4 11 80
cc 5 68 127
cc 5 11 80
cc 6 11 80
cc 8 11 80
cc 10 11 80
cc 14 11 80
cc 15 68 127
cc 15 11 80
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 81 0
noteoff 17 69 0
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "measure 67 - $$ Page 42, Bottom, 7th"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 57 0
noteoff 12 69 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
noteoff 4 73 0
noteoff 5 61 0
cc 4 68 0
cc 5 68 0
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 50 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 14 76 120
noteon 17 64 120
sleep 200.0
noteoff 14 76 0
noteoff 17 64 0
noteon 14 74 120
noteon 17 62 120
sleep 200.0
echo "measure 68 - $$ Page 42, Bottom, 8th"
noteoff 14 74 0
noteoff 17 62 0
noteon 2 72 120
noteon 6 60 120
noteon 14 72 120
noteon 17 67 120
noteon 20 64 120
noteon 22 60 120
noteon 23 48 120
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 17 67 0
noteoff 20 64 0
noteoff 22 60 0
noteoff 23 48 0
noteon 17 69 120
noteon 20 65 120
sleep 200.0
noteoff 17 69 0
noteoff 20 65 0
noteon 17 70 120
noteon 20 67 120
sleep 200.0
echo "measure 69 - $$ Page 42, Bottom, 9th"
noteoff 2 72 0
noteoff 6 60 0
noteoff 14 72 0
noteoff 17 70 0
noteoff 20 67 0
noteon 2 77 120
noteon 6 65 120
noteon 14 77 120
noteon 17 69 120
noteon 20 65 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 77 0
noteoff 17 69 0
noteoff 20 65 0
noteoff 22 53 0
noteoff 23 41 0
noteon 2 76 120
noteon 6 64 120
noteon 14 76 120
sleep 200.0
noteoff 2 76 0
noteoff 6 64 0
noteoff 14 76 0
noteon 2 77 120
noteon 6 65 120
noteon 14 77 120
sleep 200.0
echo "measure 70 - $$ Page 42, Bottom, 10th"
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 77 0
noteon 0 81 120
noteon 2 69 120
noteon 6 57 120
noteon 14 69 120
noteon 17 64 120
noteon 20 61 120
noteon 22 57 120
noteon 23 45 120
cc 0 11 50
sleep 200.0
noteoff 17 64 0
noteoff 20 61 0
noteoff 22 57 0
noteoff 23 45 0
noteon 17 65 120
noteon 20 62 120
sleep 200.0
noteoff 17 65 0
noteoff 20 62 0
noteon 17 67 120
noteon 20 64 120
sleep 200.0
echo "measure 71 - $$ Page 42, Bottom, 11th"
noteoff 0 81 0
noteoff 2 69 0
noteoff 6 57 0
noteoff 14 69 0
noteoff 17 67 0
noteoff 20 64 0
noteon 0 86 120
noteon 2 74 120
noteon 6 62 120
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
noteoff 0 86 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 74 0
noteoff 17 65 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 2 73 120
noteon 6 61 120
noteon 14 73 120
sleep 200.0
noteoff 0 85 0
noteoff 2 73 0
noteoff 6 61 0
noteoff 14 73 0
noteon 0 86 120
noteon 2 74 120
noteon 6 62 120
noteon 14 74 120
sleep 200.0
echo "measure 72 - $$ Page 42, Bottom, 12th"
noteoff 0 86 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 74 0
noteon 4 74 120
noteon 5 62 120
noteon 6 52 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 20 68 127
cc 22 68 127
sleep 14.166
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 5.833
cc 6 11 51
sleep 22.5
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 17.5
cc 6 11 52
sleep 11.25
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 28.333
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 0.416
cc 6 11 53
sleep 28.333
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 11.666
cc 6 11 54
sleep 17.083
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 22.916
cc 6 11 55
sleep 5.416
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 14.583
noteon 6 54 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 52 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 14.166
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 5.833
cc 6 11 56
sleep 22.5
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 17.5
cc 6 11 57
sleep 11.25
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 28.333
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 0.416
cc 6 11 58
sleep 28.333
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 11.666
cc 6 11 59
sleep 17.083
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 22.916
cc 6 11 60
sleep 5.416
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 14.583
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 54 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 7.083
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 2.5
cc 6 11 59
sleep 11.666
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 8.333
cc 6 11 58
sleep 5.833
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 14.166
cc 6 11 57
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 14.583
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 5.416
cc 6 11 56
sleep 8.75
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 11.25
cc 6 11 55
sleep 2.916
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 14.583
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 2.5
cc 6 11 54
sleep 11.666
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 8.333
cc 6 11 53
sleep 5.833
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 14.166
cc 6 11 52
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 14.583
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 5.416
cc 6 11 51
sleep 8.75
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 11.25
cc 6 11 50
sleep 2.916
cc 17 11 40
cc 20 11 40
cc 22 11 40
sleep 7.5
echo "measure 73 - $$ Page 42, Bottom, 13th (last)"
noteon 6 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
sleep 200.0
noteon 6 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 6 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
sleep 200.0
noteon 6 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 6 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 200.0
echo "measure 74 - $$ Page 43, Top, 1st"
noteoff 4 74 0
noteoff 5 62 0
noteoff 7 46 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 68 127
cc 4 11 80
cc 5 68 127
cc 5 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 14 11 80
cc 15 68 127
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
noteoff 6 56 0
cc 6 68 0
cc 6 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 81 0
noteoff 17 69 0
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "measure 75 - $$ Page 43, Top, 2nd"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 62 120
noteon 22 62 120
noteon 23 50 120
noteoff 4 73 0
noteoff 5 61 0
cc 4 68 0
cc 5 68 0
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 62 0
noteoff 22 62 0
noteoff 23 50 0
noteon 14 76 120
noteon 17 64 120
sleep 200.0
noteoff 14 76 0
noteoff 17 64 0
noteon 14 74 120
noteon 17 62 120
sleep 200.0
echo "measure 76 - $$ Page 43, Top, 3rd"
noteoff 14 74 0
noteoff 17 62 0
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 17 11 50
cc 20 68 127
cc 20 11 50
cc 22 68 127
cc 22 11 50
cc 23 11 50
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 40.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 40.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 40.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 40.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
noteon 6 52 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 50 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 40.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 40.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 40.0
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 40.0
cc 6 11 60
cc 17 11 60
cc 20 11 60
cc 22 11 60
sleep 20.0
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 52 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 9.583
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 20.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 20.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 20.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 20.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 20.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 20.0
cc 6 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 10.416
echo "measure 77 - $$ Page 43, Top, 4th"
noteon 6 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
sleep 200.0
noteon 6 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 6 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
sleep 200.0
noteon 6 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 6 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 6 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 200.0
echo "measure 78 - $$ Page 43, Top, 5th"
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 56 0
noteoff 7 46 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 90
cc 12 11 90
cc 14 11 90
cc 15 68 127
cc 15 11 90
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 6 11 50
cc 7 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
noteoff 15 81 0
noteoff 17 69 0
cc 15 11 80
cc 17 11 50
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "measure 79 - $$ Page 43, Top, 6th"
noteoff 4 73 0
noteoff 5 61 0
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
cc 4 11 50
cc 5 11 50
cc 8 11 50
cc 10 11 50
cc 11 11 80
cc 12 11 80
cc 14 11 80
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
noteon 8 68 120
noteon 10 56 120
noteon 14 74 120
noteon 17 62 120
sleep 200.0
noteoff 8 68 0
noteoff 10 56 0
noteoff 14 74 0
noteoff 17 62 0
noteon 8 64 120
noteon 10 52 120
noteon 14 78 120
noteon 17 66 120
sleep 200.0
echo "measure 80 - $$ Page 43, Top, 7th"
noteoff 8 64 0
noteoff 10 52 0
noteoff 14 78 0
noteoff 17 66 0
noteon 0 85 120
noteon 4 73 120
noteon 6 61 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 81 120
noteon 15 62 120
noteon 17 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
cc 0 11 80
cc 4 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 90
cc 12 11 90
cc 14 68 127
cc 14 11 90
cc 15 11 90
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 7 57 120
noteon 14 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 7 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
noteoff 14 81 0
noteoff 17 69 0
cc 14 11 80
cc 17 11 50
sleep 200.0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 7 57 120
noteon 14 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 14 80 0
noteoff 17 68 0
cc 14 68 0
cc 17 68 0
sleep 200.0
echo "measure 81 - $$ Page 43, Top, 8th"
noteoff 0 85 0
noteoff 4 73 0
noteoff 6 61 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 79 0
noteoff 15 62 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 4 74 120
noteon 6 62 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
cc 0 11 50
cc 4 11 50
cc 6 11 50
cc 8 11 50
cc 10 11 50
cc 11 11 80
cc 12 11 80
sleep 200.0
noteoff 0 86 0
noteoff 4 74 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
noteon 8 68 120
noteon 10 56 120
noteon 14 74 120
noteon 17 62 120
sleep 200.0
noteoff 8 68 0
noteoff 10 56 0
noteoff 14 74 0
noteoff 17 62 0
noteon 8 64 120
noteon 10 52 120
noteon 14 78 120
noteon 17 66 120
sleep 200.0
echo "measure 82 - $$ Page 43, Top, 9th"
noteoff 8 64 0
noteoff 10 52 0
noteoff 14 78 0
noteoff 17 66 0
noteon 0 85 120
noteon 1 81 120
noteon 2 81 120
noteon 3 69 120
noteon 4 73 120
noteon 5 69 120
noteon 6 61 120
noteon 7 57 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 61 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
cc 0 11 90
cc 1 11 90
cc 2 11 90
cc 3 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 8 11 90
cc 10 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 68 127
cc 17 11 90
cc 20 68 127
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 13 45 0
noteoff 22 45 0
noteoff 23 33 0
noteon 13 45 120
noteon 17 64 120
noteon 20 61 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 61 0
noteoff 20 57 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 13 45 120
noteon 17 69 120
noteon 20 64 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 64 0
noteoff 20 61 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
cc 0 68 127
cc 1 68 127
cc 2 68 127
cc 3 68 127
cc 4 68 127
cc 5 68 127
cc 6 68 127
cc 7 68 127
echo "measure 83 - $$ Page 43, Top, 10th"
cc 15 68 127
noteoff 12 57 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 12 57 120
noteon 13 45 120
noteon 17 73 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 69 0
noteoff 20 64 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 8 59 0
noteoff 10 47 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 1 83 120
noteon 2 80 120
noteon 3 68 120
noteon 4 74 120
noteon 5 71 120
noteon 6 62 120
noteon 7 59 120
noteon 8 66 120
noteon 10 59 120
noteon 13 45 120
noteon 15 80 120
noteon 17 74 120
noteon 20 71 120
noteon 22 57 120
noteon 23 45 120
noteoff 0 85 0
noteoff 1 81 0
noteoff 2 81 0
noteoff 3 69 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 61 0
noteoff 7 57 0
noteoff 15 81 0
noteoff 17 73 0
noteoff 20 69 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 8 66 0
noteoff 10 59 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 88 120
noteon 1 85 120
noteon 2 79 120
noteon 3 67 120
noteon 4 76 120
noteon 5 73 120
noteon 6 64 120
noteon 7 61 120
noteon 8 66 120
noteon 10 59 120
noteon 13 45 120
noteon 15 79 120
noteon 17 76 120
noteon 20 73 120
noteon 22 57 120
noteon 23 45 120
noteoff 0 86 0
noteoff 1 83 0
noteoff 2 80 0
noteoff 3 68 0
noteoff 4 74 0
noteoff 5 71 0
noteoff 6 62 0
noteoff 7 59 0
noteoff 15 80 0
noteoff 17 74 0
noteoff 20 71 0
cc 0 68 0
cc 1 68 0
cc 2 68 0
cc 3 68 0
cc 4 68 0
cc 5 68 0
cc 6 68 0
cc 7 68 0
cc 15 68 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
echo "measure 84 - $$ Page 43, Top, 11th [First ending.]"
noteoff 0 88 0
noteoff 1 85 0
noteoff 2 79 0
noteoff 3 67 0
noteoff 4 76 0
noteoff 5 73 0
noteoff 6 64 0
noteoff 7 61 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 76 0
noteoff 20 73 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 90 120
noteon 1 78 120
noteon 2 78 120
noteon 3 66 120
noteon 4 78 120
noteon 5 74 120
noteon 6 66 120
noteon 7 62 120
noteon 8 64 120
noteon 10 56 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 78 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
sleep 200.0
noteoff 0 90 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 66 0
noteoff 4 78 0
noteoff 5 74 0
noteoff 6 66 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 78 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
sleep 200.0
noteon 0 85 120
noteon 1 85 120
noteon 2 76 120
noteon 3 73 120
noteon 4 76 120
noteon 5 73 120
noteon 6 57 120
noteon 8 66 120
noteon 10 59 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 18 11 90
sleep 200.0
echo "measure 17 - $$ Page 40, Bottom, 5th"
noteoff 0 85 0
noteoff 1 85 0
noteoff 2 76 0
noteoff 3 73 0
noteoff 4 76 0
noteoff 5 73 0
noteoff 6 57 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 69 0
noteoff 15 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 1 86 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 74 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 56 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 86 120
noteon 16 69 120
noteon 17 78 120
noteon 18 62 120
noteon 19 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 16 11 90
cc 19 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 86 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 74 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 62 0
noteoff 15 86 0
noteoff 16 69 0
noteoff 17 78 0
noteoff 18 62 0
noteoff 19 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
sleep 200.0
sleep 200.0
echo "measure 18 - $$ Page 40, Bottom, 6th"
noteon 14 69 120
noteon 17 61 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
cc 14 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 200.0
noteoff 17 61 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 17 62 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
noteoff 17 62 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 17 64 120
noteon 20 61 120
noteon 22 61 120
noteon 23 49 120
sleep 200.0
echo "measure 19 - $$ Page 40, Bottom, 7th"
noteoff 14 69 0
noteoff 17 64 0
noteoff 20 61 0
noteoff 22 61 0
noteoff 23 49 0
noteon 14 77 120
noteon 17 65 120
noteon 20 62 120
noteon 22 62 120
noteon 23 50 120
sleep 200.0
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 62 0
noteoff 22 62 0
noteoff 23 50 0
noteon 14 76 120
sleep 200.0
noteoff 14 76 0
noteon 14 77 120
sleep 200.0
echo "measure 20 - $$ Page 40, Bottom, 8th"
noteoff 14 77 0
noteon 14 65 120
noteon 17 57 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 17 57 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 17 58 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
sleep 200.0
noteoff 17 58 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 17 60 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
echo "measure 21 - $$ Page 40, Bottom, 9th"
noteoff 14 65 0
noteoff 17 60 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 14 74 120
noteon 17 62 120
noteon 20 58 120
noteon 22 58 120
noteon 23 46 120
sleep 200.0
noteoff 14 74 0
noteoff 17 62 0
noteoff 20 58 0
noteoff 22 58 0
noteoff 23 46 0
noteon 14 73 120
sleep 200.0
noteoff 14 73 0
noteon 14 74 120
sleep 200.0
echo "measure 22 - $$ Page 40, Bottom, 10th"
noteoff 14 74 0
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 18 11 50
sleep 200.0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 77 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 400.0
echo "measure 23 - $$ Page 40, Bottom, 11th"
cc 14 11 50
cc 17 11 50
cc 18 11 50
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 400.0
noteoff 14 77 0
noteon 14 74 120
cc 14 68 127
sleep 100.0
noteon 14 70 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 24 - $$ Page 40, Bottom, 12th (last)"
noteoff 14 70 0
noteoff 17 62 0
noteoff 18 58 0
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
cc 14 68 127
sleep 100.0
noteon 14 72 120
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 75 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 72 0
sleep 100.0
noteon 14 74 120
noteoff 14 75 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 72 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 74 0
sleep 100.0
noteon 14 70 120
noteoff 14 72 0
sleep 100.0
echo "measure 25 - $$ Page 41, Top, 1st"
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 70 0
sleep 100.0
noteon 14 67 120
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 65 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 67 0
sleep 100.0
noteon 14 63 120
noteoff 14 65 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 81 120
noteon 6 57 120
noteon 14 62 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 0 68 127
cc 0 11 50
cc 6 68 127
cc 6 11 50
noteoff 14 63 0
sleep 100.0
noteon 14 60 120
noteoff 14 62 0
cc 14 68 0
sleep 100.0
echo "measure 26 - $$ Page 41, Top, 2nd"
noteoff 14 60 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 82 120
noteon 2 74 120
noteon 3 70 120
noteon 6 58 120
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 2 11 50
cc 3 11 50
noteoff 0 81 0
noteoff 6 57 0
cc 0 68 0
cc 6 68 0
sleep 200.0
noteoff 0 82 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 89 120
noteon 2 74 120
noteon 3 70 120
noteon 6 65 120
noteon 14 77 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
cc 0 11 80
cc 2 11 80
cc 3 11 80
cc 6 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 400.0
cc 0 11 50
cc 2 11 50
echo "measure 27 - $$ Page 41, Top, 3rd"
cc 3 11 50
cc 6 11 50
cc 14 11 50
cc 17 11 50
cc 18 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 400.0
noteoff 0 89 0
noteoff 6 65 0
noteoff 14 77 0
noteon 0 86 120
noteon 6 62 120
noteon 14 74 120
cc 0 68 127
cc 6 68 127
cc 14 68 127
sleep 100.0
noteon 0 82 120
noteon 6 58 120
noteon 14 70 120
noteoff 0 86 0
noteoff 6 62 0
noteoff 14 74 0
cc 0 68 0
cc 6 68 0
cc 14 68 0
sleep 100.0
echo "measure 28 - $$ Page 41, Top, 4th"
noteoff 0 82 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 70 0
noteoff 17 62 0
noteoff 18 58 0
noteon 0 81 120
noteon 2 75 120
noteon 3 72 120
noteon 6 57 120
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
cc 0 68 127
cc 14 68 127
sleep 100.0
noteon 0 84 120
noteon 14 72 120
noteoff 0 81 0
noteoff 14 69 0
sleep 100.0
noteoff 6 57 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 87 120
noteon 14 75 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 84 0
noteoff 14 72 0
sleep 100.0
noteon 0 86 120
noteon 14 74 120
noteoff 0 87 0
noteoff 14 75 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 84 120
noteon 14 72 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 86 0
noteoff 14 74 0
sleep 100.0
noteon 0 82 120
noteon 14 70 120
noteoff 0 84 0
noteoff 14 72 0
sleep 100.0
echo "measure 29 - $$ Page 41, Top, 5th"
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 81 120
noteon 14 69 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 82 0
noteoff 14 70 0
sleep 100.0
noteon 0 79 120
noteon 14 67 120
noteoff 0 81 0
noteoff 14 69 0
sleep 100.0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 77 120
noteon 6 53 120
noteon 14 65 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 79 0
noteoff 14 67 0
sleep 100.0
noteon 0 75 120
noteon 14 63 120
noteoff 0 77 0
noteoff 14 65 0
sleep 100.0
noteoff 6 53 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 74 120
noteon 6 53 120
noteon 14 62 120
noteon 17 63 120
noteon 18 60 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 0 75 0
noteoff 14 63 0
sleep 100.0
noteon 0 72 120
noteon 14 60 120
noteoff 0 74 0
noteoff 14 62 0
cc 0 68 0
sleep 100.0
echo "measure 30 - $$ Page 41, Top, 6th"
noteoff 0 72 0
noteoff 2 75 0
noteoff 3 72 0
noteoff 6 53 0
noteoff 17 63 0
noteoff 18 60 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 0 70 120
noteon 2 74 120
noteon 3 70 120
noteon 6 58 120
noteon 14 58 120
noteon 17 62 120
noteon 18 58 120
noteon 20 53 120
noteon 22 46 120
noteon 23 34 120
noteoff 14 60 0
cc 14 68 0
sleep 200.0
noteoff 0 70 0
noteoff 2 74 0
noteoff 3 70 0
noteoff 6 58 0
noteoff 14 58 0
noteoff 17 62 0
noteoff 18 58 0
noteoff 20 53 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 74 120
noteon 17 62 120
noteon 20 58 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 58 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 31 - $$ Page 41, Top, 7th"
noteoff 14 76 0
noteoff 17 65 0
noteoff 20 62 0
noteon 6 57 120
noteon 14 77 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 6 57 0
noteoff 14 77 0
noteoff 22 45 0
noteoff 23 33 0
noteon 14 74 120
noteon 17 62 120
noteon 20 57 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 57 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
cc 14 68 0
sleep 100.0
echo "measure 32 - $$ Page 41, Top, 8th"
noteoff 14 76 0
noteoff 17 65 0
noteoff 20 62 0
noteon 6 56 120
noteon 14 77 120
noteon 22 44 120
noteon 23 32 120
sleep 200.0
noteoff 6 56 0
noteoff 14 77 0
noteoff 22 44 0
noteoff 23 32 0
noteon 14 74 120
noteon 17 62 120
noteon 20 59 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteoff 14 73 0
noteoff 17 62 0
noteoff 20 59 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteon 14 76 120
noteoff 14 74 0
sleep 100.0
echo "measure 33 - $$ Page 41, Top, 9th"
noteoff 17 65 0
noteoff 20 62 0
noteon 6 56 120
noteon 14 77 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
noteon 22 56 120
noteon 23 44 120
noteoff 14 76 0
cc 14 68 0
sleep 200.0
noteoff 6 56 0
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteoff 22 56 0
noteoff 23 44 0
noteon 14 79 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
cc 14 68 127
sleep 100.0
noteon 14 77 120
noteoff 14 79 0
sleep 100.0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteon 14 76 120
noteon 17 65 120
noteon 20 59 120
noteon 21 56 120
noteoff 14 77 0
sleep 100.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 100.0
echo "measure 34 - $$ Page 41, Top, 10th (last)"
noteoff 14 74 0
noteoff 17 65 0
noteoff 20 59 0
noteoff 21 56 0
noteon 6 57 120
noteon 14 73 120
noteon 17 64 120
noteon 20 61 120
noteon 21 57 120
noteon 22 57 120
noteon 23 33 120
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 21 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 6 57 0
noteoff 14 73 0
noteoff 17 64 0
noteoff 20 61 0
noteoff 21 57 0
noteoff 22 57 0
noteon 14 74 120
cc 14 68 127
sleep 100.0
noteon 14 73 120
noteoff 14 74 0
sleep 100.0
noteon 14 71 120
noteon 20 49 120
noteon 22 49 120
cc 20 68 127
cc 22 68 127
noteoff 14 73 0
sleep 100.0
noteon 14 69 120
noteoff 14 71 0
cc 14 68 0
sleep 100.0
echo "measure 35 - $$ Page 41, Bottom, 1st"
noteoff 14 69 0
noteon 14 77 120
noteon 20 50 120
noteon 22 50 120
noteoff 20 49 0
noteoff 22 49 0
sleep 200.0
noteoff 14 77 0
noteon 14 79 120
noteon 20 53 120
noteon 22 53 120
cc 14 68 127
noteoff 20 50 0
noteoff 22 50 0
sleep 100.0
noteon 14 77 120
noteoff 14 79 0
sleep 100.0
noteon 14 76 120
noteon 20 56 120
noteon 22 56 120
noteoff 14 77 0
noteoff 20 53 0
noteoff 22 53 0
cc 20 68 0
cc 22 68 0
sleep 100.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 100.0
echo "measure 36 - $$ Page 41, Bottom, 2nd"
noteoff 14 74 0
noteoff 20 56 0
noteoff 22 56 0
noteon 14 73 120
noteon 20 57 120
noteon 22 57 120
sleep 100.0
noteoff 20 57 0
noteoff 22 57 0
sleep 100.0
noteoff 14 73 0
noteon 14 74 120
cc 14 68 127
sleep 25.0
cc 14 11 41
sleep 50.0
cc 14 11 42
sleep 25.0
noteon 14 73 120
noteoff 14 74 0
sleep 25.0
cc 14 11 43
sleep 50.0
cc 14 11 44
sleep 25.0
noteon 14 71 120
noteon 17 61 120
noteon 20 49 120
noteon 22 49 120
cc 17 68 127
cc 20 68 127
cc 22 68 127
noteoff 14 73 0
sleep 25.0
cc 14 11 45
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 50.0
cc 14 11 46
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 25.0
noteon 14 69 120
noteoff 14 71 0
cc 14 68 0
sleep 25.0
cc 14 11 47
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 50.0
cc 14 11 48
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 25.0
echo "measure 37 - $$ Page 41, Bottom, 3rd"
noteoff 14 69 0
noteon 14 77 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteoff 17 61 0
noteoff 20 49 0
noteoff 22 49 0
sleep 25.0
cc 14 11 49
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 50.0
cc 14 11 50
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 25.0
noteoff 14 77 0
sleep 25.0
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 50.0
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 25.0
noteon 14 79 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
cc 14 68 127
cc 14 11 52
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 25.0
cc 14 11 53
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 50.0
cc 14 11 54
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 25.0
noteon 14 77 120
noteoff 14 79 0
sleep 25.0
cc 14 11 55
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 50.0
cc 14 11 56
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 25.0
noteon 14 76 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 14 77 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 25.0
cc 14 11 57
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 50.0
cc 14 11 58
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 25.0
noteon 14 74 120
noteoff 14 76 0
cc 14 68 0
sleep 25.0
cc 14 11 59
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 50.0
cc 14 11 60
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 25.0
echo "measure 38 - $$ Page 41, Bottom, 4th"
noteoff 14 74 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteon 14 73 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
sleep 25.0
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 7.916
cc 14 11 61
sleep 42.083
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 24.583
cc 14 11 62
sleep 0.416
noteoff 14 73 0
noteoff 17 69 0
noteoff 20 57 0
noteoff 22 57 0
sleep 100.0
noteoff 23 33 0
noteon 14 76 120
noteon 17 57 120
cc 14 68 127
cc 14 11 63
cc 17 68 127
cc 17 11 60
sleep 25.0
cc 14 11 64
cc 17 11 61
sleep 49.583
cc 14 11 65
sleep 0.416
cc 17 11 62
sleep 25.0
noteon 14 74 120
noteoff 14 76 0
sleep 24.583
cc 14 11 66
sleep 0.416
cc 17 11 63
sleep 49.583
cc 14 11 67
sleep 0.416
cc 17 11 64
sleep 25.0
noteon 14 73 120
noteon 17 58 120
noteoff 14 74 0
noteoff 17 57 0
sleep 24.583
cc 14 11 68
cc 17 11 65
sleep 50.0
cc 14 11 69
cc 17 11 66
sleep 25.416
noteon 14 71 120
noteoff 14 73 0
sleep 24.583
cc 14 11 70
cc 17 11 67
sleep 50.0
cc 14 11 71
cc 17 11 68
sleep 25.416
echo "measure 39 - $$ Page 41, Bottom, 5th"
noteon 14 69 120
noteon 17 59 120
noteoff 14 71 0
noteoff 17 58 0
sleep 24.583
cc 14 11 72
cc 17 11 69
sleep 50.0
cc 14 11 73
cc 17 11 70
sleep 25.416
noteon 14 68 120
noteoff 14 69 0
cc 14 68 0
sleep 24.583
cc 17 11 71
sleep 25.416
cc 14 11 74
sleep 24.583
cc 17 11 72
sleep 25.416
noteoff 14 68 0
noteon 14 69 120
noteon 17 60 120
noteoff 17 59 0
sleep 24.583
cc 14 11 75
cc 17 11 73
sleep 25.416
noteoff 14 69 0
sleep 24.583
cc 17 11 74
sleep 25.416
noteon 14 67 120
cc 14 11 76
sleep 24.583
cc 17 11 75
sleep 25.416
noteoff 14 67 0
sleep 24.583
cc 17 11 76
sleep 25.416
noteon 14 66 120
noteon 17 61 120
cc 14 11 77
noteoff 17 60 0
cc 17 68 0
sleep 24.583
cc 14 11 78
cc 17 11 77
sleep 25.416
noteoff 14 66 0
sleep 24.583
cc 17 11 78
sleep 25.416
noteon 14 64 120
cc 14 11 79
sleep 24.583
cc 17 11 79
sleep 25.416
noteoff 14 64 0
sleep 24.583
cc 17 11 80
sleep 25.416
echo "measure 40 - $$ Page 41, Bottom, 6th"
noteoff 17 61 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 12 11 80
cc 13 11 80
cc 14 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 41 - $$ Page 41, Bottom, 7th"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 42 - $$ Page 41, Bottom, 8th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 67 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 67 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 71 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 43 - $$ Page 41, Bottom, 9th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 71 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 44 - $$ Page 41, Bottom, 10th"
noteoff 14 83 0
noteon 8 66 120
noteon 10 59 120
cc 8 11 50
cc 10 11 50
sleep 200.0
noteoff 8 66 0
noteoff 10 59 0
noteon 8 68 120
noteon 10 64 120
sleep 200.0
noteoff 8 68 0
noteoff 10 64 0
noteon 8 69 120
noteon 10 66 120
sleep 200.0
echo "measure 45 - $$ Page 41, Bottom, 11th"
noteoff 8 69 0
noteoff 10 66 0
noteon 14 76 120
noteon 17 69 120
cc 17 11 50
sleep 200.0
noteoff 14 76 0
noteoff 17 69 0
noteon 14 78 120
noteon 17 74 120
sleep 200.0
noteoff 14 78 0
noteoff 17 74 0
noteon 14 79 120
noteon 17 76 120
sleep 200.0
echo "measure 46 - $$ Page 41, Bottom, 12th (last)"
noteoff 14 79 0
noteoff 17 76 0
noteon 2 78 120
noteon 3 74 120
noteon 8 68 120
noteon 10 64 120
cc 2 11 50
cc 3 11 50
sleep 200.0
noteoff 2 78 0
noteoff 3 74 0
noteoff 8 68 0
noteoff 10 64 0
noteon 2 76 120
noteon 3 69 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 66 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
echo "measure 47 - $$ Page 42, Top, 1st"
noteoff 2 74 0
noteoff 3 66 0
noteoff 8 64 0
noteoff 10 56 0
noteon 0 86 120
noteon 1 78 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 90
cc 1 11 90
cc 2 11 90
cc 3 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 11 90
cc 18 11 90
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 1 76 120
noteon 2 79 120
noteon 3 76 120
noteon 4 76 120
noteon 5 67 120
noteon 6 57 120
noteon 7 45 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
sleep 200.0
noteoff 0 85 0
noteoff 1 76 0
noteoff 2 79 0
noteoff 3 76 0
noteoff 4 76 0
noteoff 5 67 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 0 86 120
noteon 1 78 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 86 120
noteon 17 78 120
noteon 18 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
echo "measure 48 - $$ Page 42, Top, 2nd"
noteoff 0 86 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 86 0
noteoff 17 78 0
noteoff 18 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 74 120
noteon 1 74 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 62 120
noteon 17 62 120
noteon 18 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 80
cc 1 11 80
cc 2 11 80
cc 3 11 80
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 80
cc 12 11 80
cc 13 11 80
cc 14 11 80
cc 17 11 80
cc 18 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 50 0
noteoff 7 50 0
noteoff 13 50 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 52 120
noteon 7 52 120
noteon 20 64 120
noteon 22 52 120
noteon 23 40 120
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 20 64 0
noteoff 22 52 0
noteoff 23 40 0
noteon 6 54 120
noteon 7 54 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
sleep 200.0
echo "measure 49 - $$ Page 42, Top, 3rd"
noteoff 0 74 0
noteoff 1 74 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 62 0
noteoff 15 62 0
noteoff 17 62 0
noteoff 18 62 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 78 120
cc 14 11 50
sleep 200.0
noteoff 14 78 0
noteon 14 79 120
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
echo "measure 50 - $$ Page 42, Top, 4th"
noteoff 14 81 0
noteon 0 76 120
noteon 1 74 120
noteon 2 76 120
noteon 3 74 120
noteon 4 74 120
noteon 5 64 120
noteon 6 55 120
noteon 7 55 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 55 120
noteon 15 64 120
noteon 17 62 120
noteon 18 59 120
noteon 20 55 120
noteon 22 55 120
noteon 23 43 120
cc 14 11 80
sleep 200.0
noteoff 6 55 0
noteoff 7 55 0
noteoff 13 50 0
noteoff 20 55 0
noteoff 22 55 0
noteoff 23 43 0
noteon 6 57 120
noteon 7 57 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 59 120
noteon 7 59 120
noteon 20 59 120
noteon 22 59 120
noteon 23 47 120
sleep 200.0
echo "measure 51 - $$ Page 42, Top, 5th"
noteoff 0 76 0
noteoff 1 74 0
noteoff 2 76 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 64 0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 14 55 0
noteoff 15 64 0
noteoff 17 62 0
noteoff 18 59 0
noteoff 20 59 0
noteoff 22 59 0
noteoff 23 47 0
noteon 14 79 120
cc 14 11 50
sleep 200.0
noteoff 14 79 0
noteon 14 81 120
sleep 200.0
noteoff 14 81 0
noteon 14 83 120
sleep 200.0
echo "measure 52 - $$ Page 42, Top, 6th"
noteoff 14 83 0
noteon 14 67 120
sleep 200.0
noteoff 14 67 0
noteon 14 69 120
sleep 200.0
noteoff 14 69 0
noteon 14 71 120
sleep 200.0
echo "measure 53 - $$ Page 42, Top, 7th"
noteoff 14 71 0
noteon 17 64 120
cc 17 11 50
sleep 200.0
noteoff 17 64 0
noteon 17 66 120
sleep 200.0
noteoff 17 66 0
noteon 17 67 120
sleep 200.0
echo "measure 54 - $$ Page 42, Top, 8th"
noteoff 17 67 0
noteon 14 67 120
sleep 200.0
noteoff 14 67 0
noteon 14 69 120
sleep 200.0
noteoff 14 69 0
noteon 14 70 120
sleep 200.0
echo "measure 55 - $$ Page 42, Top, 9th"
noteoff 14 70 0
noteon 17 64 120
sleep 200.0
noteoff 17 64 0
noteon 17 66 120
sleep 200.0
noteoff 17 66 0
noteon 17 67 120
sleep 200.0
echo "measure 56 - $$ Page 42, Top, 10th"
noteoff 17 67 0
noteon 14 67 120
sleep 100.0
noteoff 14 67 0
sleep 100.0
noteon 14 69 120
cc 14 11 49
sleep 100.0
noteoff 14 69 0
sleep 100.0
noteon 14 70 120
cc 14 11 48
sleep 100.0
noteoff 14 70 0
sleep 100.0
echo "measure 57 - $$ Page 42, Top, 11th"
noteon 14 67 120
noteon 17 64 120
cc 14 11 47
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 46
cc 17 11 49
sleep 100.0
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 14 11 45
cc 17 11 48
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 58 - $$ Page 42, Top, 12th"
noteon 14 67 120
noteon 17 64 120
cc 14 11 44
cc 17 11 47
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 43
cc 17 11 46
sleep 100.0
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 14 11 42
cc 17 11 45
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 59 - $$ Page 42, Top, 13th"
noteon 14 67 120
noteon 17 64 120
cc 17 11 44
sleep 100.0
noteoff 14 67 0
noteoff 17 64 0
sleep 100.0
noteon 14 69 120
noteon 17 65 120
cc 14 11 41
cc 17 11 43
sleep 49.583
cc 17 11 42
sleep 50.416
noteoff 14 69 0
noteoff 17 65 0
sleep 100.0
noteon 14 70 120
noteon 17 67 120
cc 17 11 41
sleep 100.0
noteoff 14 70 0
noteoff 17 67 0
sleep 100.0
echo "measure 60 - $$ Page 42, Top, 14th (last)"
noteon 2 72 120
noteon 6 60 120
noteon 14 67 120
noteon 17 64 120
noteon 20 60 120
noteon 22 60 120
noteon 23 48 120
cc 2 11 40
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 14 67 0
noteoff 17 64 0
noteoff 20 60 0
noteoff 22 60 0
noteoff 23 48 0
noteon 14 69 120
noteon 17 65 120
sleep 200.0
noteoff 14 69 0
noteoff 17 65 0
noteon 14 70 120
noteon 17 67 120
sleep 200.0
echo "measure 61 - $$ Page 42, Bottom, 1st"
noteoff 2 72 0
noteoff 6 60 0
noteoff 14 70 0
noteoff 17 67 0
noteon 2 77 120
noteon 6 65 120
noteon 14 69 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 69 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 2 76 120
noteon 6 64 120
sleep 200.0
noteoff 2 76 0
noteoff 6 64 0
noteon 2 77 120
noteon 6 65 120
sleep 200.0
echo "measure 62 - $$ Page 42, Bottom, 2nd"
noteoff 2 77 0
noteoff 6 65 0
noteon 2 69 120
noteon 6 57 120
noteon 14 64 120
noteon 17 61 120
noteon 20 57 120
noteon 22 57 120
noteon 23 45 120
sleep 200.0
noteoff 14 64 0
noteoff 17 61 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 45 0
noteon 14 65 120
noteon 17 62 120
sleep 200.0
noteoff 6 57 0
noteoff 14 65 0
noteoff 17 62 0
noteon 14 67 120
noteon 17 64 120
sleep 200.0
echo "measure 63 - $$ Page 42, Bottom, 3rd"
noteoff 2 69 0
noteoff 14 67 0
noteoff 17 64 0
noteon 2 74 120
noteon 6 62 120
noteon 14 65 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 65 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
noteoff 23 38 0
noteon 2 73 120
noteon 6 61 120
sleep 200.0
noteoff 2 73 0
noteoff 6 61 0
noteon 2 74 120
noteon 6 62 120
sleep 200.0
echo "measure 64 - $$ Page 42, Bottom, 4th"
noteoff 2 74 0
noteoff 6 62 0
noteon 4 74 120
noteon 5 62 120
noteon 6 52 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 17 11 50
cc 20 68 127
cc 20 11 50
cc 22 68 127
cc 22 11 50
cc 23 11 50
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 40.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 40.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 40.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 40.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
noteon 6 52 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 52 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 40.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 40.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 40.0
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 40.0
cc 6 11 60
cc 17 11 60
cc 20 11 60
cc 22 11 60
sleep 20.0
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 52 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 9.583
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 20.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 20.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 20.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 20.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 20.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 20.0
cc 6 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 10.416
echo "measure 65 - $$ Page 42, Bottom, 5th"
noteoff 7 46 0
noteon 6 46 120
noteon 7 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
cc 7 68 127
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
cc 6 68 0
sleep 200.0
noteon 7 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 7 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
cc 7 11 65
sleep 19.583
cc 7 11 66
sleep 40.0
cc 7 11 67
sleep 40.0
cc 7 11 68
sleep 40.0
cc 7 11 69
sleep 40.0
cc 7 11 70
sleep 20.416
noteon 7 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 7 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 7 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 10.0
cc 7 11 71
sleep 20.0
cc 7 11 72
sleep 20.0
cc 7 11 73
sleep 20.0
cc 7 11 74
sleep 20.0
cc 7 11 75
sleep 20.0
cc 7 11 76
sleep 20.0
cc 7 11 77
sleep 20.0
cc 7 11 78
sleep 20.0
cc 7 11 79
sleep 20.0
cc 7 11 80
sleep 10.0
echo "measure 66 - $$ Page 42, Bottom, 6th"
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 46 0
noteoff 7 56 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 57 120
noteon 12 69 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 68 127
cc 4 11 80
cc 5 68 127
cc 5 11 80
cc 6 11 80
cc 8 11 80
cc 10 11 80
cc 14 11 80
cc 15 68 127
cc 15 11 80
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 81 0
noteoff 17 69 0
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "measure 67 - $$ Page 42, Bottom, 7th"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 57 0
noteoff 12 69 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
noteoff 4 73 0
noteoff 5 61 0
cc 4 68 0
cc 5 68 0
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 50 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 14 76 120
noteon 17 64 120
sleep 200.0
noteoff 14 76 0
noteoff 17 64 0
noteon 14 74 120
noteon 17 62 120
sleep 200.0
echo "measure 68 - $$ Page 42, Bottom, 8th"
noteoff 14 74 0
noteoff 17 62 0
noteon 2 72 120
noteon 6 60 120
noteon 14 72 120
noteon 17 67 120
noteon 20 64 120
noteon 22 60 120
noteon 23 48 120
cc 6 11 40
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 22 11 40
cc 23 11 40
sleep 200.0
noteoff 17 67 0
noteoff 20 64 0
noteoff 22 60 0
noteoff 23 48 0
noteon 17 69 120
noteon 20 65 120
sleep 200.0
noteoff 17 69 0
noteoff 20 65 0
noteon 17 70 120
noteon 20 67 120
sleep 200.0
echo "measure 69 - $$ Page 42, Bottom, 9th"
noteoff 2 72 0
noteoff 6 60 0
noteoff 14 72 0
noteoff 17 70 0
noteoff 20 67 0
noteon 2 77 120
noteon 6 65 120
noteon 14 77 120
noteon 17 69 120
noteon 20 65 120
noteon 22 53 120
noteon 23 41 120
sleep 200.0
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 77 0
noteoff 17 69 0
noteoff 20 65 0
noteoff 22 53 0
noteoff 23 41 0
noteon 2 76 120
noteon 6 64 120
noteon 14 76 120
sleep 200.0
noteoff 2 76 0
noteoff 6 64 0
noteoff 14 76 0
noteon 2 77 120
noteon 6 65 120
noteon 14 77 120
sleep 200.0
echo "measure 70 - $$ Page 42, Bottom, 10th"
noteoff 2 77 0
noteoff 6 65 0
noteoff 14 77 0
noteon 0 81 120
noteon 2 69 120
noteon 6 57 120
noteon 14 69 120
noteon 17 64 120
noteon 20 61 120
noteon 22 57 120
noteon 23 45 120
cc 0 11 50
sleep 200.0
noteoff 17 64 0
noteoff 20 61 0
noteoff 22 57 0
noteoff 23 45 0
noteon 17 65 120
noteon 20 62 120
sleep 200.0
noteoff 17 65 0
noteoff 20 62 0
noteon 17 67 120
noteon 20 64 120
sleep 200.0
echo "measure 71 - $$ Page 42, Bottom, 11th"
noteoff 0 81 0
noteoff 2 69 0
noteoff 6 57 0
noteoff 14 69 0
noteoff 17 67 0
noteoff 20 64 0
noteon 0 86 120
noteon 2 74 120
noteon 6 62 120
noteon 14 74 120
noteon 17 65 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
noteoff 0 86 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 74 0
noteoff 17 65 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 0 85 120
noteon 2 73 120
noteon 6 61 120
noteon 14 73 120
sleep 200.0
noteoff 0 85 0
noteoff 2 73 0
noteoff 6 61 0
noteoff 14 73 0
noteon 0 86 120
noteon 2 74 120
noteon 6 62 120
noteon 14 74 120
sleep 200.0
echo "measure 72 - $$ Page 42, Bottom, 12th"
noteoff 0 86 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 14 74 0
noteon 4 74 120
noteon 5 62 120
noteon 6 52 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 20 68 127
cc 22 68 127
sleep 14.166
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 5.833
cc 6 11 51
sleep 22.5
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 17.5
cc 6 11 52
sleep 11.25
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 28.333
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 0.416
cc 6 11 53
sleep 28.333
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 11.666
cc 6 11 54
sleep 17.083
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 22.916
cc 6 11 55
sleep 5.416
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 14.583
noteon 6 54 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 52 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 14.166
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 5.833
cc 6 11 56
sleep 22.5
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 17.5
cc 6 11 57
sleep 11.25
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 28.333
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 0.416
cc 6 11 58
sleep 28.333
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 11.666
cc 6 11 59
sleep 17.083
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 22.916
cc 6 11 60
sleep 5.416
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 14.583
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 54 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 7.083
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 2.5
cc 6 11 59
sleep 11.666
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 8.333
cc 6 11 58
sleep 5.833
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 14.166
cc 6 11 57
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 14.583
cc 17 11 49
cc 20 11 49
cc 22 11 49
sleep 5.416
cc 6 11 56
sleep 8.75
cc 17 11 48
cc 20 11 48
cc 22 11 48
sleep 11.25
cc 6 11 55
sleep 2.916
cc 17 11 47
cc 20 11 47
cc 22 11 47
sleep 14.583
cc 17 11 46
cc 20 11 46
cc 22 11 46
sleep 2.5
cc 6 11 54
sleep 11.666
cc 17 11 45
cc 20 11 45
cc 22 11 45
sleep 8.333
cc 6 11 53
sleep 5.833
cc 17 11 44
cc 20 11 44
cc 22 11 44
sleep 14.166
cc 6 11 52
cc 17 11 43
cc 20 11 43
cc 22 11 43
sleep 14.583
cc 17 11 42
cc 20 11 42
cc 22 11 42
sleep 5.416
cc 6 11 51
sleep 8.75
cc 17 11 41
cc 20 11 41
cc 22 11 41
sleep 11.25
cc 6 11 50
sleep 2.916
cc 17 11 40
cc 20 11 40
cc 22 11 40
sleep 7.5
echo "measure 73 - $$ Page 42, Bottom, 13th (last)"
noteon 6 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
sleep 200.0
noteon 6 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 6 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
sleep 200.0
noteon 6 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 6 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 200.0
echo "measure 74 - $$ Page 43, Top, 1st"
noteoff 4 74 0
noteoff 5 62 0
noteoff 7 46 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 68 127
cc 4 11 80
cc 5 68 127
cc 5 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 14 11 80
cc 15 68 127
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
noteoff 6 56 0
cc 6 68 0
cc 6 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 81 0
noteoff 17 69 0
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "measure 75 - $$ Page 43, Top, 2nd"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 62 120
noteon 22 62 120
noteon 23 50 120
noteoff 4 73 0
noteoff 5 61 0
cc 4 68 0
cc 5 68 0
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 62 0
noteoff 22 62 0
noteoff 23 50 0
noteon 14 76 120
noteon 17 64 120
sleep 200.0
noteoff 14 76 0
noteoff 17 64 0
noteon 14 74 120
noteon 17 62 120
sleep 200.0
echo "measure 76 - $$ Page 43, Top, 3rd"
noteoff 14 74 0
noteoff 17 62 0
noteon 4 74 120
noteon 5 62 120
noteon 6 50 120
noteon 7 46 120
noteon 8 64 120
noteon 10 52 120
noteon 17 62 120
noteon 20 50 120
noteon 22 50 120
noteon 23 34 120
cc 4 11 50
cc 5 11 50
cc 6 68 127
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 17 68 127
cc 17 11 50
cc 20 68 127
cc 20 11 50
cc 22 68 127
cc 22 11 50
cc 23 11 50
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 40.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 40.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 40.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 40.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
noteon 6 52 120
noteon 17 64 120
noteon 20 52 120
noteon 22 52 120
noteoff 6 50 0
noteoff 17 62 0
noteoff 20 50 0
noteoff 22 50 0
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 40.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 40.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 40.0
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 40.0
cc 6 11 60
cc 17 11 60
cc 20 11 60
cc 22 11 60
sleep 20.0
noteon 6 53 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteoff 6 52 0
noteoff 17 64 0
noteoff 20 52 0
noteoff 22 52 0
sleep 9.583
cc 6 11 59
cc 17 11 59
cc 20 11 59
cc 22 11 59
sleep 20.0
cc 6 11 58
cc 17 11 58
cc 20 11 58
cc 22 11 58
sleep 20.0
cc 6 11 57
cc 17 11 57
cc 20 11 57
cc 22 11 57
sleep 20.0
cc 6 11 56
cc 17 11 56
cc 20 11 56
cc 22 11 56
sleep 20.0
cc 6 11 55
cc 17 11 55
cc 20 11 55
cc 22 11 55
sleep 20.0
cc 6 11 54
cc 17 11 54
cc 20 11 54
cc 22 11 54
sleep 20.0
cc 6 11 53
cc 17 11 53
cc 20 11 53
cc 22 11 53
sleep 20.0
cc 6 11 52
cc 17 11 52
cc 20 11 52
cc 22 11 52
sleep 20.0
cc 6 11 51
cc 17 11 51
cc 20 11 51
cc 22 11 51
sleep 20.0
cc 6 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
sleep 10.416
echo "measure 77 - $$ Page 43, Top, 4th"
noteon 6 54 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteoff 6 53 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
sleep 200.0
noteon 6 55 120
noteon 17 67 120
noteon 20 55 120
noteon 22 55 120
noteoff 6 54 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
sleep 200.0
noteon 6 56 120
noteon 17 68 120
noteon 20 56 120
noteon 22 56 120
noteoff 6 55 0
noteoff 17 67 0
noteoff 20 55 0
noteoff 22 55 0
cc 6 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
sleep 200.0
echo "measure 78 - $$ Page 43, Top, 5th"
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 56 0
noteoff 7 46 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 17 68 0
noteoff 20 56 0
noteoff 22 56 0
noteoff 23 34 0
noteon 4 73 120
noteon 5 61 120
noteon 6 57 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 69 120
noteon 20 57 120
noteon 22 57 120
noteon 23 33 120
cc 4 11 80
cc 5 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 90
cc 12 11 90
cc 14 11 90
cc 15 68 127
cc 15 11 90
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 6 57 0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 57 0
noteoff 23 33 0
noteon 6 57 120
noteon 7 57 120
noteon 15 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 6 11 50
cc 7 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
noteoff 15 81 0
noteoff 17 69 0
cc 15 11 80
cc 17 11 50
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 6 57 120
noteon 7 57 120
noteon 15 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 15 80 0
noteoff 17 68 0
cc 15 68 0
cc 17 68 0
sleep 200.0
echo "measure 79 - $$ Page 43, Top, 6th"
noteoff 4 73 0
noteoff 5 61 0
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 4 74 120
noteon 5 62 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
cc 4 11 50
cc 5 11 50
cc 8 11 50
cc 10 11 50
cc 11 11 80
cc 12 11 80
cc 14 11 80
sleep 200.0
noteoff 4 74 0
noteoff 5 62 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
noteon 8 68 120
noteon 10 56 120
noteon 14 74 120
noteon 17 62 120
sleep 200.0
noteoff 8 68 0
noteoff 10 56 0
noteoff 14 74 0
noteoff 17 62 0
noteon 8 64 120
noteon 10 52 120
noteon 14 78 120
noteon 17 66 120
sleep 200.0
echo "measure 80 - $$ Page 43, Top, 7th"
noteoff 8 64 0
noteoff 10 52 0
noteoff 14 78 0
noteoff 17 66 0
noteon 0 85 120
noteon 4 73 120
noteon 6 61 120
noteon 7 45 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 81 120
noteon 15 62 120
noteon 17 69 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
cc 0 11 80
cc 4 11 80
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 10 11 80
cc 11 11 90
cc 12 11 90
cc 14 68 127
cc 14 11 90
cc 15 11 90
cc 17 68 127
cc 17 11 80
cc 20 11 80
cc 22 11 80
cc 23 11 80
sleep 200.0
noteoff 7 45 0
noteoff 13 45 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
noteon 7 57 120
noteon 14 80 120
noteon 17 68 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 7 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
noteoff 14 81 0
noteoff 17 69 0
cc 14 11 80
cc 17 11 50
sleep 200.0
noteoff 7 57 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 7 57 120
noteon 14 79 120
noteon 17 67 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 14 80 0
noteoff 17 68 0
cc 14 68 0
cc 17 68 0
sleep 200.0
echo "measure 81 - $$ Page 43, Top, 8th"
noteoff 0 85 0
noteoff 4 73 0
noteoff 6 61 0
noteoff 7 57 0
noteoff 8 59 0
noteoff 10 47 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 14 79 0
noteoff 15 62 0
noteoff 17 67 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 4 74 120
noteon 6 62 120
noteon 7 50 120
noteon 8 64 120
noteon 10 52 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 66 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
cc 0 11 50
cc 4 11 50
cc 6 11 50
cc 8 11 50
cc 10 11 50
cc 11 11 80
cc 12 11 80
sleep 200.0
noteoff 0 86 0
noteoff 4 74 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 52 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 66 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
noteon 8 68 120
noteon 10 56 120
noteon 14 74 120
noteon 17 62 120
sleep 200.0
noteoff 8 68 0
noteoff 10 56 0
noteoff 14 74 0
noteoff 17 62 0
noteon 8 64 120
noteon 10 52 120
noteon 14 78 120
noteon 17 66 120
sleep 200.0
echo "measure 82 - $$ Page 43, Top, 9th"
noteoff 8 64 0
noteoff 10 52 0
noteoff 14 78 0
noteoff 17 66 0
noteon 0 85 120
noteon 1 81 120
noteon 2 81 120
noteon 3 69 120
noteon 4 73 120
noteon 5 69 120
noteon 6 61 120
noteon 7 57 120
noteon 8 59 120
noteon 10 47 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 81 120
noteon 17 61 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
cc 0 11 90
cc 1 11 90
cc 2 11 90
cc 3 11 90
cc 4 11 90
cc 5 11 90
cc 6 11 90
cc 7 11 90
cc 8 11 90
cc 10 11 90
cc 11 11 90
cc 12 11 90
cc 13 11 90
cc 14 11 90
cc 17 68 127
cc 17 11 90
cc 20 68 127
cc 20 11 90
cc 22 11 90
cc 23 11 90
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 13 45 0
noteoff 22 45 0
noteoff 23 33 0
noteon 13 45 120
noteon 17 64 120
noteon 20 61 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 61 0
noteoff 20 57 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 13 45 120
noteon 17 69 120
noteon 20 64 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 64 0
noteoff 20 61 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
cc 0 68 127
cc 1 68 127
cc 2 68 127
cc 3 68 127
cc 4 68 127
cc 5 68 127
cc 6 68 127
cc 7 68 127
echo "measure 83 - $$ Page 43, Top, 10th"
cc 15 68 127
noteoff 12 57 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 12 57 120
noteon 13 45 120
noteon 17 73 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
noteoff 17 69 0
noteoff 20 64 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 8 59 0
noteoff 10 47 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 1 83 120
noteon 2 80 120
noteon 3 68 120
noteon 4 74 120
noteon 5 71 120
noteon 6 62 120
noteon 7 59 120
noteon 8 66 120
noteon 10 59 120
noteon 13 45 120
noteon 15 80 120
noteon 17 74 120
noteon 20 71 120
noteon 22 57 120
noteon 23 45 120
noteoff 0 85 0
noteoff 1 81 0
noteoff 2 81 0
noteoff 3 69 0
noteoff 4 73 0
noteoff 5 69 0
noteoff 6 61 0
noteoff 7 57 0
noteoff 15 81 0
noteoff 17 73 0
noteoff 20 69 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
noteoff 8 66 0
noteoff 10 59 0
noteoff 13 45 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 88 120
noteon 1 85 120
noteon 2 79 120
noteon 3 67 120
noteon 4 76 120
noteon 5 73 120
noteon 6 64 120
noteon 7 61 120
noteon 8 66 120
noteon 10 59 120
noteon 13 45 120
noteon 15 79 120
noteon 17 76 120
noteon 20 73 120
noteon 22 57 120
noteon 23 45 120
noteoff 0 86 0
noteoff 1 83 0
noteoff 2 80 0
noteoff 3 68 0
noteoff 4 74 0
noteoff 5 71 0
noteoff 6 62 0
noteoff 7 59 0
noteoff 15 80 0
noteoff 17 74 0
noteoff 20 71 0
cc 0 68 0
cc 1 68 0
cc 2 68 0
cc 3 68 0
cc 4 68 0
cc 5 68 0
cc 6 68 0
cc 7 68 0
cc 15 68 0
cc 17 68 0
cc 20 68 0
sleep 100.0
noteoff 13 45 0
noteon 13 45 120
sleep 100.0
echo "measure 84 - $$ Page 43, Top, 11th [First ending.]"
noteoff 0 88 0
noteoff 1 85 0
noteoff 2 79 0
noteoff 3 67 0
noteoff 4 76 0
noteoff 5 73 0
noteoff 6 64 0
noteoff 7 61 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 69 0
noteoff 15 79 0
noteoff 17 76 0
noteoff 20 73 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 90 120
noteon 1 78 120
noteon 2 78 120
noteon 3 66 120
noteon 4 78 120
noteon 5 74 120
noteon 6 66 120
noteon 7 62 120
noteon 8 64 120
noteon 10 56 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 78 120
noteon 17 78 120
noteon 20 74 120
noteon 22 62 120
noteon 23 50 120
sleep 200.0
noteoff 0 90 0
noteoff 1 78 0
noteoff 2 78 0
noteoff 3 66 0
noteoff 4 78 0
noteoff 5 74 0
noteoff 6 66 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 78 0
noteoff 17 78 0
noteoff 20 74 0
noteoff 22 62 0
noteoff 23 50 0
sleep 200.0
noteon 0 85 120
noteon 1 85 120
noteon 2 76 120
noteon 3 73 120
noteon 4 76 120
noteon 5 73 120
noteon 6 57 120
noteon 8 66 120
noteon 10 59 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
noteon 14 69 120
noteon 15 85 120
noteon 17 79 120
noteon 18 69 120
noteon 20 69 120
noteon 22 57 120
noteon 23 45 120
cc 18 11 90
sleep 200.0
echo "measure 85 - $$ Page 43, Top, 12th (last) [Second ending.]"
noteoff 0 85 0
noteoff 1 85 0
noteoff 2 76 0
noteoff 3 73 0
noteoff 4 76 0
noteoff 5 73 0
noteoff 6 57 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 11 69 0
noteoff 12 57 0
noteoff 13 45 0
noteoff 14 69 0
noteoff 15 85 0
noteoff 17 79 0
noteoff 18 69 0
noteoff 20 69 0
noteoff 22 57 0
noteoff 23 45 0
noteon 0 86 120
noteon 1 86 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 74 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 56 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 86 120
noteon 16 69 120
noteon 17 78 120
noteon 18 62 120
noteon 19 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 16 11 90
cc 19 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 86 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 74 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 62 0
noteoff 15 86 0
noteoff 16 69 0
noteoff 17 78 0
noteoff 18 62 0
noteoff 19 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
sleep 200.0
sleep 200.0
echo "meter 2 2 48 8"
echo "measure 86 - $$ Page 43, Bottom, 1st"
noteon 0 86 120
noteon 1 86 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 74 120
noteon 6 62 120
noteon 7 62 120
noteon 8 64 120
noteon 10 56 120
noteon 11 74 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 15 86 120
noteon 16 69 120
noteon 17 78 120
noteon 18 62 120
noteon 19 69 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
sleep 200.0
noteoff 0 86 0
noteoff 1 86 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 74 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 11 74 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 62 0
noteoff 15 86 0
noteoff 16 69 0
noteoff 17 78 0
noteoff 18 62 0
noteoff 19 69 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
sleep 200.0
echo "meter 1 2 48 8"
echo "measure 87 - $$ Page 43, Bottom, 2nd"
noteon 2 74 120
cc 2 68 127
cc 2 11 50
sleep 100.0
noteon 2 76 120
noteoff 2 74 0
cc 2 68 0
sleep 100.0
echo "meter 3 2 48 8"
echo "measure 88 - $$ Page 43, Bottom, 3rd"
noteoff 2 76 0
noteon 2 76 120
noteon 3 74 120
noteon 6 62 120
noteon 7 62 120
cc 2 68 127
cc 3 11 50
cc 6 11 50
cc 7 68 127
cc 7 11 50
sleep 600.0
echo "measure 89 - $$ Page 43, Bottom, 4th"
noteon 2 79 120
noteon 7 59 120
noteoff 2 76 0
noteoff 7 62 0
sleep 600.0
echo "measure 90 - $$ Page 43, Bottom, 5th"
cc 6 68 127
noteoff 3 74 0
noteon 2 81 120
noteon 3 74 120
noteon 7 54 120
cc 3 68 127
noteoff 2 79 0
noteoff 7 59 0
sleep 200.0
noteon 2 79 120
noteon 3 69 120
noteon 6 61 120
noteon 7 52 120
noteoff 2 81 0
noteoff 3 74 0
noteoff 6 62 0
noteoff 7 54 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 69 0
noteoff 6 61 0
noteoff 7 52 0
noteon 2 78 120
noteon 3 69 120
noteon 6 62 120
noteon 7 50 120
sleep 200.0
echo "measure 91 - $$ Page 43, Bottom, 6th"
noteoff 2 78 0
noteoff 3 69 0
noteoff 6 62 0
noteoff 7 50 0
noteon 2 76 120
noteon 3 69 120
noteon 6 61 120
noteon 7 69 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 6 61 0
noteoff 7 69 0
sleep 200.0
noteon 2 78 120
noteon 6 63 120
noteon 7 54 120
sleep 200.0
echo "measure 92 - $$ Page 43, Bottom, 7th"
noteoff 2 78 0
noteoff 6 63 0
noteoff 7 54 0
noteon 2 79 120
noteon 3 71 120
noteon 6 64 120
noteon 7 52 120
cc 2 68 127
cc 3 68 127
cc 6 68 127
cc 7 68 127
sleep 9.583
cc 2 11 51
cc 3 11 51
cc 6 11 51
cc 7 11 51
sleep 20.0
cc 2 11 52
cc 3 11 52
cc 6 11 52
cc 7 11 52
sleep 20.0
cc 2 11 53
cc 3 11 53
cc 6 11 53
cc 7 11 53
sleep 20.0
cc 2 11 54
cc 3 11 54
cc 6 11 54
cc 7 11 54
sleep 20.0
cc 2 11 55
cc 3 11 55
cc 6 11 55
cc 7 11 55
sleep 20.0
cc 2 11 56
cc 3 11 56
cc 6 11 56
cc 7 11 56
sleep 20.0
cc 2 11 57
cc 3 11 57
cc 6 11 57
cc 7 11 57
sleep 20.0
cc 2 11 58
cc 3 11 58
cc 6 11 58
cc 7 11 58
sleep 20.0
cc 2 11 59
cc 3 11 59
cc 6 11 59
cc 7 11 59
sleep 20.0
cc 2 11 60
cc 3 11 60
cc 6 11 60
cc 7 11 60
sleep 20.0
cc 2 11 61
cc 3 11 61
cc 6 11 61
cc 7 11 61
sleep 20.0
cc 2 11 62
cc 3 11 62
cc 6 11 62
cc 7 11 62
sleep 20.0
cc 2 11 63
cc 3 11 63
cc 6 11 63
cc 7 11 63
sleep 20.0
cc 2 11 64
cc 3 11 64
cc 6 11 64
cc 7 11 64
sleep 20.0
cc 2 11 65
cc 3 11 65
cc 6 11 65
cc 7 11 65
sleep 20.0
cc 2 11 66
cc 3 11 66
cc 6 11 66
cc 7 11 66
sleep 20.0
cc 2 11 67
cc 3 11 67
cc 6 11 67
cc 7 11 67
sleep 20.0
cc 2 11 68
cc 3 11 68
cc 6 11 68
cc 7 11 68
sleep 20.0
cc 2 11 69
cc 3 11 69
cc 6 11 69
cc 7 11 69
sleep 20.0
cc 2 11 70
cc 3 11 70
cc 6 11 70
cc 7 11 70
sleep 20.0
cc 2 11 71
cc 3 11 71
cc 6 11 71
cc 7 11 71
sleep 20.416
cc 2 11 72
cc 3 11 72
cc 6 11 72
cc 7 11 72
sleep 20.0
cc 2 11 73
cc 3 11 73
cc 6 11 73
cc 7 11 73
sleep 20.0
cc 2 11 74
cc 3 11 74
cc 6 11 74
cc 7 11 74
sleep 20.0
cc 2 11 75
cc 3 11 75
cc 6 11 75
cc 7 11 75
sleep 20.0
cc 2 11 76
cc 3 11 76
cc 6 11 76
cc 7 11 76
sleep 20.0
cc 2 11 77
cc 3 11 77
cc 6 11 77
cc 7 11 77
sleep 20.0
cc 2 11 78
cc 3 11 78
cc 6 11 78
cc 7 11 78
sleep 20.0
cc 2 11 79
cc 3 11 79
cc 6 11 79
cc 7 11 79
sleep 20.0
cc 2 11 80
cc 3 11 80
cc 6 11 80
cc 7 11 80
sleep 10.0
echo "measure 93 - $$ Page 43, Bottom, 8th"
noteon 2 81 120
noteon 3 73 120
noteon 6 61 120
noteon 7 45 120
noteon 8 66 120
noteon 10 59 120
cc 8 11 80
cc 10 11 80
noteoff 2 79 0
noteoff 3 71 0
noteoff 6 64 0
noteoff 7 52 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
sleep 10.0
cc 2 11 79
cc 3 11 79
cc 6 11 79
cc 7 11 79
cc 8 11 79
cc 10 11 79
sleep 20.0
cc 2 11 78
cc 3 11 78
cc 6 11 78
cc 7 11 78
cc 8 11 78
cc 10 11 78
sleep 20.0
cc 2 11 77
cc 3 11 77
cc 6 11 77
cc 7 11 77
cc 8 11 77
cc 10 11 77
sleep 20.0
cc 2 11 76
cc 3 11 76
cc 6 11 76
cc 7 11 76
cc 8 11 76
cc 10 11 76
sleep 20.0
cc 2 11 75
cc 3 11 75
cc 6 11 75
cc 7 11 75
cc 8 11 75
cc 10 11 75
sleep 20.0
cc 2 11 74
cc 3 11 74
cc 6 11 74
cc 7 11 74
cc 8 11 74
cc 10 11 74
sleep 20.0
cc 2 11 73
cc 3 11 73
cc 6 11 73
cc 7 11 73
cc 8 11 73
cc 10 11 73
sleep 20.0
cc 2 11 72
cc 3 11 72
cc 6 11 72
cc 7 11 72
cc 8 11 72
cc 10 11 72
sleep 20.0
cc 2 11 71
cc 3 11 71
cc 6 11 71
cc 7 11 71
cc 8 11 71
cc 10 11 71
sleep 20.0
cc 2 11 70
cc 3 11 70
cc 6 11 70
cc 7 11 70
cc 8 11 70
cc 10 11 70
sleep 20.0
cc 2 11 69
cc 3 11 69
cc 6 11 69
cc 7 11 69
cc 8 11 69
cc 10 11 69
sleep 20.0
cc 2 11 68
cc 3 11 68
cc 6 11 68
cc 7 11 68
cc 8 11 68
cc 10 11 68
sleep 20.0
cc 2 11 67
cc 3 11 67
cc 6 11 67
cc 7 11 67
cc 8 11 67
cc 10 11 67
sleep 20.0
cc 2 11 66
cc 3 11 66
cc 6 11 66
cc 7 11 66
cc 8 11 66
cc 10 11 66
sleep 20.0
cc 2 11 65
cc 3 11 65
cc 6 11 65
cc 7 11 65
cc 8 11 65
cc 10 11 65
sleep 20.0
cc 2 11 64
cc 3 11 64
cc 6 11 64
cc 7 11 64
cc 8 11 64
cc 10 11 64
sleep 20.0
cc 2 11 63
cc 3 11 63
cc 6 11 63
cc 7 11 63
cc 8 11 63
cc 10 11 63
sleep 20.0
cc 2 11 62
cc 3 11 62
cc 6 11 62
cc 7 11 62
cc 8 11 62
cc 10 11 62
sleep 20.0
cc 2 11 61
cc 3 11 61
cc 6 11 61
cc 7 11 61
cc 8 11 61
cc 10 11 61
sleep 20.0
cc 2 11 60
cc 3 11 60
cc 6 11 60
cc 7 11 60
cc 8 11 60
cc 10 11 60
sleep 10.0
noteoff 6 61 0
noteoff 7 45 0
sleep 10.0
cc 2 11 59
cc 3 11 59
cc 8 11 59
cc 10 11 59
sleep 20.0
cc 2 11 58
cc 3 11 58
cc 8 11 58
cc 10 11 58
sleep 20.0
cc 2 11 57
cc 3 11 57
cc 8 11 57
cc 10 11 57
sleep 20.0
cc 2 11 56
cc 3 11 56
cc 8 11 56
cc 10 11 56
sleep 20.0
cc 2 11 55
cc 3 11 55
cc 8 11 55
cc 10 11 55
sleep 20.0
cc 2 11 54
cc 3 11 54
cc 8 11 54
cc 10 11 54
sleep 20.0
cc 2 11 53
cc 3 11 53
cc 8 11 53
cc 10 11 53
sleep 20.0
cc 2 11 52
cc 3 11 52
cc 8 11 52
cc 10 11 52
sleep 20.0
cc 2 11 51
cc 3 11 51
cc 8 11 51
cc 10 11 51
sleep 20.0
cc 2 11 50
cc 3 11 50
cc 8 11 50
cc 10 11 50
sleep 10.0
echo "measure 94 - $$ Page 43, Bottom, 9th [Ending 1]"
noteoff 2 81 0
noteoff 3 73 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 78 120
noteon 3 74 120
noteon 6 62 120
noteon 7 50 120
noteon 8 68 120
noteon 10 64 120
cc 2 68 127
cc 3 68 127
cc 6 68 127
cc 6 11 50
cc 7 68 127
cc 7 11 50
cc 8 68 127
cc 10 68 127
sleep 200.0
noteon 2 79 120
noteon 3 76 120
noteon 6 59 120
noteon 7 55 120
noteon 8 69 120
noteon 10 66 120
noteoff 2 78 0
noteoff 3 74 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 8 68 0
noteoff 10 64 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteoff 6 59 0
noteoff 7 55 0
noteoff 8 69 0
noteoff 10 66 0
noteon 2 76 120
noteon 3 73 120
noteon 6 43 120
noteon 7 45 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
echo "meter 2 2 48 8"
echo "measure 86 - $$ Page 43, Bottom, 1st"
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 43 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 74 120
noteon 6 54 120
noteon 7 50 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
noteoff 2 74 0
noteoff 3 74 0
noteoff 6 54 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 56 0
sleep 200.0
echo "meter 1 2 48 8"
echo "measure 87 - $$ Page 43, Bottom, 2nd"
noteon 2 74 120
cc 2 68 127
sleep 100.0
noteon 2 76 120
noteoff 2 74 0
cc 2 68 0
sleep 100.0
echo "meter 3 2 48 8"
echo "measure 88 - $$ Page 43, Bottom, 3rd"
noteoff 2 76 0
noteon 2 76 120
noteon 3 74 120
noteon 6 62 120
noteon 7 62 120
cc 2 68 127
cc 7 68 127
sleep 600.0
echo "measure 89 - $$ Page 43, Bottom, 4th"
noteon 2 79 120
noteon 7 59 120
noteoff 2 76 0
noteoff 7 62 0
sleep 600.0
echo "measure 90 - $$ Page 43, Bottom, 5th"
cc 6 68 127
noteoff 3 74 0
noteon 2 81 120
noteon 3 74 120
noteon 7 54 120
cc 3 68 127
noteoff 2 79 0
noteoff 7 59 0
sleep 200.0
noteon 2 79 120
noteon 3 69 120
noteon 6 61 120
noteon 7 52 120
noteoff 2 81 0
noteoff 3 74 0
noteoff 6 62 0
noteoff 7 54 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 69 0
noteoff 6 61 0
noteoff 7 52 0
noteon 2 78 120
noteon 3 69 120
noteon 6 62 120
noteon 7 50 120
sleep 200.0
echo "measure 91 - $$ Page 43, Bottom, 6th"
noteoff 2 78 0
noteoff 3 69 0
noteoff 6 62 0
noteoff 7 50 0
noteon 2 76 120
noteon 3 69 120
noteon 6 61 120
noteon 7 69 120
sleep 200.0
noteoff 2 76 0
noteoff 3 69 0
noteoff 6 61 0
noteoff 7 69 0
sleep 200.0
noteon 2 78 120
noteon 6 63 120
noteon 7 54 120
sleep 200.0
echo "measure 92 - $$ Page 43, Bottom, 7th"
noteoff 2 78 0
noteoff 6 63 0
noteoff 7 54 0
noteon 2 79 120
noteon 3 71 120
noteon 6 64 120
noteon 7 52 120
cc 2 68 127
cc 3 68 127
cc 6 68 127
cc 7 68 127
sleep 9.583
cc 2 11 51
cc 3 11 51
cc 6 11 51
cc 7 11 51
sleep 20.0
cc 2 11 52
cc 3 11 52
cc 6 11 52
cc 7 11 52
sleep 20.0
cc 2 11 53
cc 3 11 53
cc 6 11 53
cc 7 11 53
sleep 20.0
cc 2 11 54
cc 3 11 54
cc 6 11 54
cc 7 11 54
sleep 20.0
cc 2 11 55
cc 3 11 55
cc 6 11 55
cc 7 11 55
sleep 20.0
cc 2 11 56
cc 3 11 56
cc 6 11 56
cc 7 11 56
sleep 20.0
cc 2 11 57
cc 3 11 57
cc 6 11 57
cc 7 11 57
sleep 20.0
cc 2 11 58
cc 3 11 58
cc 6 11 58
cc 7 11 58
sleep 20.0
cc 2 11 59
cc 3 11 59
cc 6 11 59
cc 7 11 59
sleep 20.0
cc 2 11 60
cc 3 11 60
cc 6 11 60
cc 7 11 60
sleep 20.0
cc 2 11 61
cc 3 11 61
cc 6 11 61
cc 7 11 61
sleep 20.0
cc 2 11 62
cc 3 11 62
cc 6 11 62
cc 7 11 62
sleep 20.0
cc 2 11 63
cc 3 11 63
cc 6 11 63
cc 7 11 63
sleep 20.0
cc 2 11 64
cc 3 11 64
cc 6 11 64
cc 7 11 64
sleep 20.0
cc 2 11 65
cc 3 11 65
cc 6 11 65
cc 7 11 65
sleep 20.0
cc 2 11 66
cc 3 11 66
cc 6 11 66
cc 7 11 66
sleep 20.0
cc 2 11 67
cc 3 11 67
cc 6 11 67
cc 7 11 67
sleep 20.0
cc 2 11 68
cc 3 11 68
cc 6 11 68
cc 7 11 68
sleep 20.0
cc 2 11 69
cc 3 11 69
cc 6 11 69
cc 7 11 69
sleep 20.0
cc 2 11 70
cc 3 11 70
cc 6 11 70
cc 7 11 70
sleep 20.0
cc 2 11 71
cc 3 11 71
cc 6 11 71
cc 7 11 71
sleep 20.416
cc 2 11 72
cc 3 11 72
cc 6 11 72
cc 7 11 72
sleep 20.0
cc 2 11 73
cc 3 11 73
cc 6 11 73
cc 7 11 73
sleep 20.0
cc 2 11 74
cc 3 11 74
cc 6 11 74
cc 7 11 74
sleep 20.0
cc 2 11 75
cc 3 11 75
cc 6 11 75
cc 7 11 75
sleep 20.0
cc 2 11 76
cc 3 11 76
cc 6 11 76
cc 7 11 76
sleep 20.0
cc 2 11 77
cc 3 11 77
cc 6 11 77
cc 7 11 77
sleep 20.0
cc 2 11 78
cc 3 11 78
cc 6 11 78
cc 7 11 78
sleep 20.0
cc 2 11 79
cc 3 11 79
cc 6 11 79
cc 7 11 79
sleep 20.0
cc 2 11 80
cc 3 11 80
cc 6 11 80
cc 7 11 80
sleep 10.0
echo "measure 93 - $$ Page 43, Bottom, 8th"
noteon 2 81 120
noteon 3 73 120
noteon 6 61 120
noteon 7 45 120
noteon 8 66 120
noteon 10 59 120
cc 8 11 80
cc 10 11 80
noteoff 2 79 0
noteoff 3 71 0
noteoff 6 64 0
noteoff 7 52 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
sleep 10.0
cc 2 11 79
cc 3 11 79
cc 6 11 79
cc 7 11 79
cc 8 11 79
cc 10 11 79
sleep 20.0
cc 2 11 78
cc 3 11 78
cc 6 11 78
cc 7 11 78
cc 8 11 78
cc 10 11 78
sleep 20.0
cc 2 11 77
cc 3 11 77
cc 6 11 77
cc 7 11 77
cc 8 11 77
cc 10 11 77
sleep 20.0
cc 2 11 76
cc 3 11 76
cc 6 11 76
cc 7 11 76
cc 8 11 76
cc 10 11 76
sleep 20.0
cc 2 11 75
cc 3 11 75
cc 6 11 75
cc 7 11 75
cc 8 11 75
cc 10 11 75
sleep 20.0
cc 2 11 74
cc 3 11 74
cc 6 11 74
cc 7 11 74
cc 8 11 74
cc 10 11 74
sleep 20.0
cc 2 11 73
cc 3 11 73
cc 6 11 73
cc 7 11 73
cc 8 11 73
cc 10 11 73
sleep 20.0
cc 2 11 72
cc 3 11 72
cc 6 11 72
cc 7 11 72
cc 8 11 72
cc 10 11 72
sleep 20.0
cc 2 11 71
cc 3 11 71
cc 6 11 71
cc 7 11 71
cc 8 11 71
cc 10 11 71
sleep 20.0
cc 2 11 70
cc 3 11 70
cc 6 11 70
cc 7 11 70
cc 8 11 70
cc 10 11 70
sleep 20.0
cc 2 11 69
cc 3 11 69
cc 6 11 69
cc 7 11 69
cc 8 11 69
cc 10 11 69
sleep 20.0
cc 2 11 68
cc 3 11 68
cc 6 11 68
cc 7 11 68
cc 8 11 68
cc 10 11 68
sleep 20.0
cc 2 11 67
cc 3 11 67
cc 6 11 67
cc 7 11 67
cc 8 11 67
cc 10 11 67
sleep 20.0
cc 2 11 66
cc 3 11 66
cc 6 11 66
cc 7 11 66
cc 8 11 66
cc 10 11 66
sleep 20.0
cc 2 11 65
cc 3 11 65
cc 6 11 65
cc 7 11 65
cc 8 11 65
cc 10 11 65
sleep 20.0
cc 2 11 64
cc 3 11 64
cc 6 11 64
cc 7 11 64
cc 8 11 64
cc 10 11 64
sleep 20.0
cc 2 11 63
cc 3 11 63
cc 6 11 63
cc 7 11 63
cc 8 11 63
cc 10 11 63
sleep 20.0
cc 2 11 62
cc 3 11 62
cc 6 11 62
cc 7 11 62
cc 8 11 62
cc 10 11 62
sleep 20.0
cc 2 11 61
cc 3 11 61
cc 6 11 61
cc 7 11 61
cc 8 11 61
cc 10 11 61
sleep 20.0
cc 2 11 60
cc 3 11 60
cc 6 11 60
cc 7 11 60
cc 8 11 60
cc 10 11 60
sleep 10.0
noteoff 6 61 0
noteoff 7 45 0
sleep 10.0
cc 2 11 59
cc 3 11 59
cc 8 11 59
cc 10 11 59
sleep 20.0
cc 2 11 58
cc 3 11 58
cc 8 11 58
cc 10 11 58
sleep 20.0
cc 2 11 57
cc 3 11 57
cc 8 11 57
cc 10 11 57
sleep 20.0
cc 2 11 56
cc 3 11 56
cc 8 11 56
cc 10 11 56
sleep 20.0
cc 2 11 55
cc 3 11 55
cc 8 11 55
cc 10 11 55
sleep 20.0
cc 2 11 54
cc 3 11 54
cc 8 11 54
cc 10 11 54
sleep 20.0
cc 2 11 53
cc 3 11 53
cc 8 11 53
cc 10 11 53
sleep 20.0
cc 2 11 52
cc 3 11 52
cc 8 11 52
cc 10 11 52
sleep 20.0
cc 2 11 51
cc 3 11 51
cc 8 11 51
cc 10 11 51
sleep 20.0
cc 2 11 50
cc 3 11 50
cc 8 11 50
cc 10 11 50
sleep 10.0
echo "measure 94 - $$ Page 43, Bottom, 9th [Ending 1]"
noteoff 2 81 0
noteoff 3 73 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 78 120
noteon 3 74 120
noteon 6 62 120
noteon 7 50 120
noteon 8 68 120
noteon 10 64 120
cc 2 68 127
cc 3 68 127
cc 6 68 127
cc 6 11 50
cc 7 68 127
cc 7 11 50
cc 8 68 127
cc 10 68 127
sleep 200.0
noteon 2 79 120
noteon 3 76 120
noteon 6 59 120
noteon 7 55 120
noteon 8 69 120
noteon 10 66 120
noteoff 2 78 0
noteoff 3 74 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 8 68 0
noteoff 10 64 0
cc 2 68 0
cc 3 68 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteoff 6 59 0
noteoff 7 55 0
noteoff 8 69 0
noteoff 10 66 0
noteon 2 76 120
noteon 3 73 120
noteon 6 43 120
noteon 7 45 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
echo "meter 2 2 48 8"
echo "measure 95 - $$ Page 43, Bottom, 10th [Ending 2]"
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 43 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 74 120
noteon 6 54 120
noteon 7 50 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
noteoff 2 74 0
noteoff 3 74 0
noteoff 6 54 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 56 0
sleep 200.0
echo "meter 3 2 48 8"
echo "measure 96 - $$ Page 43, Bottom, 11th"
noteon 2 74 120
noteon 3 74 120
noteon 6 54 120
noteon 7 50 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
noteoff 2 74 0
noteoff 3 74 0
noteoff 6 54 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 56 0
sleep 200.0
sleep 200.0
echo "measure 97 - $$ Page 43, Bottom, 12th"
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 23 42 120
cc 14 11 80
cc 17 11 80
cc 20 11 80
cc 23 11 80
sleep 600.0
echo "measure 98 - $$ Page 43, Bottom, 13th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 11 90
cc 17 11 90
cc 20 11 90
cc 22 11 100
cc 23 11 90
sleep 600.0
echo "measure 99 - $$ Page 43, Bottom, 14th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 14 11 80
cc 17 68 127
cc 17 11 80
cc 20 68 127
cc 20 11 80
cc 22 68 127
cc 22 11 90
cc 23 68 127
cc 23 11 80
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
cc 22 68 127
cc 23 68 127
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
cc 22 68 127
cc 23 68 127
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
echo "measure 100 - $$ Page 43, Bottom, 15th (last)"
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
cc 22 68 127
cc 23 68 127
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
cc 22 68 127
cc 23 68 127
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
cc 14 68 127
cc 17 68 127
cc 20 68 127
cc 22 68 127
cc 23 68 127
sleep 100.0
noteon 14 65 120
noteon 17 65 120
noteon 20 53 120
noteon 22 53 120
noteon 23 41 120
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
cc 14 68 0
cc 17 68 0
cc 20 68 0
cc 22 68 0
cc 23 68 0
sleep 100.0
echo "measure 101 - $$ Page 44, Top, 1st"
noteoff 14 65 0
noteoff 17 65 0
noteoff 20 53 0
noteoff 22 53 0
noteoff 23 41 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteon 14 66 120
noteon 17 66 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 23 42 0
noteon 14 70 120
noteon 17 70 120
noteon 20 58 120
noteon 23 34 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteon 14 70 120
noteon 17 70 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteoff 20 58 0
noteoff 23 34 0
noteon 14 73 120
noteon 17 73 120
noteon 20 61 120
noteon 23 37 120
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteon 14 73 120
noteon 17 73 120
sleep 100.0
echo "measure 102 - $$ Page 44, Top, 2nd"
noteoff 14 73 0
noteoff 17 73 0
noteoff 20 61 0
noteoff 23 37 0
noteon 14 78 120
noteon 17 78 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
cc 14 11 90
cc 17 11 90
cc 20 11 90
cc 22 11 100
cc 23 11 90
sleep 100.0
noteoff 14 78 0
noteoff 17 78 0
noteon 14 78 120
noteon 17 78 120
cc 14 11 80
cc 17 11 80
sleep 100.0
noteoff 14 78 0
noteoff 17 78 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 73 120
noteon 17 73 120
noteon 20 61 120
noteon 22 49 120
noteon 23 37 120
cc 20 11 80
cc 22 11 90
cc 23 11 80
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteon 14 73 120
noteon 17 73 120
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteoff 20 61 0
noteoff 22 49 0
noteoff 23 37 0
noteon 14 70 120
noteon 17 70 120
noteon 20 58 120
noteon 22 46 120
noteon 23 34 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteon 14 70 120
noteon 17 70 120
sleep 100.0
echo "measure 103 - $$ Page 44, Top, 3rd"
noteoff 14 70 0
noteoff 17 70 0
noteoff 20 58 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteon 14 66 120
noteon 17 66 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 70 120
noteon 17 70 120
noteon 20 58 120
noteon 22 46 120
noteon 23 34 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteon 14 70 120
noteon 17 70 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteoff 20 58 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 73 120
noteon 17 73 120
noteon 20 61 120
noteon 22 49 120
noteon 23 37 120
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteon 14 73 120
noteon 17 73 120
sleep 100.0
echo "measure 104 - $$ Page 44, Top, 4th"
noteoff 14 73 0
noteoff 17 73 0
noteoff 20 61 0
noteoff 22 49 0
noteoff 23 37 0
noteon 14 78 120
noteon 17 78 120
noteon 20 66 120
noteon 22 54 120
noteon 23 42 120
cc 14 11 90
cc 17 11 90
cc 20 11 90
cc 22 11 100
cc 23 11 90
sleep 100.0
noteoff 14 78 0
noteoff 17 78 0
noteon 14 78 120
noteon 17 78 120
cc 14 11 80
cc 17 11 80
sleep 100.0
noteoff 14 78 0
noteoff 17 78 0
noteoff 20 66 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 73 120
noteon 17 73 120
noteon 20 61 120
noteon 22 49 120
noteon 23 37 120
cc 20 11 80
cc 22 11 90
cc 23 11 80
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteon 14 73 120
noteon 17 73 120
sleep 100.0
noteoff 14 73 0
noteoff 17 73 0
noteoff 20 61 0
noteoff 22 49 0
noteoff 23 37 0
noteon 14 70 120
noteon 17 70 120
noteon 20 58 120
noteon 22 46 120
noteon 23 34 120
sleep 100.0
noteoff 14 70 0
noteoff 17 70 0
noteon 14 70 120
noteon 17 70 120
sleep 100.0
echo "measure 105 - $$ Page 44, Top, 5th"
noteoff 14 70 0
noteoff 17 70 0
noteoff 20 58 0
noteoff 22 46 0
noteoff 23 34 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
cc 14 11 50
cc 17 11 50
cc 20 11 50
cc 22 11 50
cc 23 11 50
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
echo "measure 106 - $$ Page 44, Top, 6th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 54 120
noteon 23 42 120
sleep 100.0
echo "measure 107 - $$ Page 44, Top, 7th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 54 0
noteoff 23 42 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 42 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 42 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 42 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 42 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 42 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
echo "measure 108 - $$ Page 44, Top, 8th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 42 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
sleep 100.0
echo "measure 109 - $$ Page 44, Top, 9th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 14 66 120
noteon 17 66 120
noteon 20 54 120
noteon 22 42 120
noteon 23 30 120
cc 14 11 40
cc 17 11 40
cc 20 11 40
cc 22 11 40
cc 23 11 40
sleep 600.0
echo "measure 110 - $$ Page 44, Top, 10th"
sleep 600.0
echo "measure 111 - $$ Page 44, Top, 11th"
noteoff 14 66 0
noteoff 17 66 0
noteoff 20 54 0
noteoff 22 42 0
noteoff 23 30 0
noteon 0 81 120
noteon 1 81 120
noteon 2 81 120
noteon 3 69 120
noteon 4 81 120
noteon 5 69 120
noteon 6 57 120
noteon 7 45 120
noteon 8 71 120
noteon 10 59 120
noteon 11 69 120
noteon 12 57 120
noteon 13 45 120
cc 2 11 90
cc 3 11 90
cc 6 11 90
cc 7 11 90
cc 8 11 90
cc 10 11 90
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
echo "measure 112 - $$ Page 44, Top, 12th"
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
noteon 14 69 120
noteon 17 57 120
noteon 20 57 120
noteon 22 45 120
noteon 23 33 120
cc 14 68 127
cc 14 11 90
cc 17 68 127
cc 17 11 90
cc 20 68 127
cc 20 11 90
cc 22 68 127
cc 22 11 90
cc 23 68 127
cc 23 11 90
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
sleep 50.0
noteoff 13 45 0
noteon 13 45 120
cc 13 68 127
sleep 50.0
echo "measure 113 - $$ Page 44, Top, 13th"
noteoff 0 81 0
noteoff 1 81 0
noteoff 2 81 0
noteoff 3 69 0
noteoff 4 81 0
noteoff 5 69 0
noteoff 6 57 0
noteoff 7 45 0
noteoff 8 71 0
noteoff 10 59 0
noteoff 11 69 0
noteoff 12 57 0
noteon 0 78 120
noteon 1 78 120
noteon 2 78 120
noteon 3 74 120
noteon 4 78 120
noteon 5 66 120
noteon 6 47 120
noteon 7 47 120
noteon 8 68 120
noteon 10 64 120
noteon 11 66 120
noteon 12 62 120
noteon 13 50 120
noteon 14 62 120
noteon 17 62 120
noteon 20 62 120
noteon 22 50 120
noteon 23 38 120
cc 0 11 50
cc 1 11 50
cc 2 68 127
cc 2 11 50
cc 3 11 50
cc 4 11 50
cc 5 11 50
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 11 11 50
cc 12 11 50
noteoff 13 45 0
noteoff 14 69 0
noteoff 17 57 0
noteoff 20 57 0
noteoff 22 45 0
noteoff 23 33 0
cc 13 68 0
cc 13 11 50
cc 14 68 0
cc 14 11 50
cc 17 68 0
cc 17 11 50
cc 20 68 0
cc 20 11 50
cc 22 68 0
cc 22 11 50
cc 23 68 0
cc 23 11 50
sleep 200.0
noteoff 0 78 0
noteoff 1 78 0
noteoff 4 78 0
noteoff 5 66 0
noteoff 6 47 0
noteoff 7 47 0
noteoff 8 68 0
noteoff 10 64 0
noteoff 11 66 0
noteoff 12 62 0
noteoff 13 50 0
noteoff 14 62 0
noteoff 17 62 0
noteoff 20 62 0
noteoff 22 50 0
noteoff 23 38 0
noteon 6 62 120
noteon 7 62 120
sleep 200.0
noteoff 6 62 0
noteoff 7 62 0
noteon 6 61 120
noteon 7 61 120
sleep 200.0
echo "measure 114 - $$ Page 44, Top, 14th"
cc 3 68 127
noteoff 6 61 0
noteoff 7 61 0
noteon 2 79 120
noteon 6 59 120
noteon 7 59 120
noteoff 2 78 0
sleep 200.0
noteoff 6 59 0
noteoff 7 59 0
noteon 3 73 120
noteon 6 57 120
noteon 7 57 120
noteoff 3 74 0
sleep 200.0
noteoff 6 57 0
noteoff 7 57 0
noteon 3 71 120
noteon 6 55 120
noteon 7 55 120
noteoff 3 73 0
cc 3 68 0
sleep 200.0
echo "measure 115 - $$ Page 44, Top, 15th"
noteoff 3 71 0
noteoff 6 55 0
noteoff 7 55 0
noteon 2 81 120
noteon 3 69 120
noteon 6 54 120
noteon 7 54 120
noteon 8 71 120
noteon 10 59 120
cc 3 68 127
noteoff 2 79 0
sleep 200.0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 71 0
noteoff 10 59 0
noteon 2 79 120
noteon 3 73 120
noteon 6 52 120
noteon 7 52 120
noteon 8 71 120
noteon 10 59 120
noteoff 2 81 0
noteoff 3 69 0
sleep 200.0
noteoff 6 52 0
noteoff 7 52 0
noteoff 8 71 0
noteoff 10 59 0
noteon 2 78 120
noteon 3 74 120
noteon 6 50 120
noteon 7 50 120
noteon 8 71 120
noteon 10 59 120
noteoff 2 79 0
noteoff 3 73 0
cc 2 68 0
cc 3 68 0
sleep 200.0
echo "measure 116 - $$ Page 44, Top, 16th"
noteoff 2 78 0
noteoff 3 74 0
noteoff 6 50 0
noteoff 7 50 0
noteoff 8 71 0
noteoff 10 59 0
noteon 2 76 120
noteon 3 73 120
noteon 6 57 120
noteon 7 57 120
noteon 8 71 120
noteon 10 59 120
sleep 200.0
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 71 0
noteoff 10 59 0
sleep 200.0
noteon 2 78 120
noteon 3 75 120
noteon 4 69 120
noteon 6 54 120
noteon 7 54 120
sleep 200.0
echo "measure 117 - $$ Page 44, Top, 17th (last)"
noteoff 2 78 0
noteoff 3 75 0
noteoff 4 69 0
noteoff 6 54 0
noteoff 7 54 0
noteon 2 79 120
noteon 3 76 120
noteon 4 71 120
noteon 6 52 120
noteon 7 52 120
cc 2 68 127
cc 4 68 127
sleep 7.083
cc 2 11 51
cc 3 11 51
cc 4 11 51
sleep 2.5
cc 6 11 51
cc 7 11 51
sleep 12.5
cc 2 11 52
cc 3 11 52
cc 4 11 52
sleep 7.5
cc 6 11 52
cc 7 11 52
sleep 7.5
cc 2 11 53
cc 3 11 53
cc 4 11 53
sleep 12.5
cc 6 11 53
cc 7 11 53
sleep 2.5
cc 2 11 54
cc 3 11 54
cc 4 11 54
sleep 15.0
cc 2 11 55
cc 3 11 55
cc 4 11 55
sleep 2.5
cc 6 11 54
cc 7 11 54
sleep 12.5
cc 2 11 56
cc 3 11 56
cc 4 11 56
sleep 7.5
cc 6 11 55
cc 7 11 55
sleep 7.5
cc 2 11 57
cc 3 11 57
cc 4 11 57
sleep 2.916
noteoff 6 52 0
noteoff 7 52 0
sleep 12.083
cc 2 11 58
cc 3 11 58
cc 4 11 58
sleep 15.0
cc 2 11 59
cc 3 11 59
cc 4 11 59
sleep 15.0
cc 2 11 60
cc 3 11 60
cc 4 11 60
sleep 15.0
cc 2 11 61
cc 3 11 61
cc 4 11 61
sleep 15.0
cc 2 11 62
cc 3 11 62
cc 4 11 62
sleep 15.0
cc 2 11 63
cc 3 11 63
cc 4 11 63
sleep 12.916
noteon 6 64 120
noteon 7 64 120
cc 6 11 60
cc 7 11 60
sleep 2.083
cc 2 11 64
cc 3 11 64
cc 4 11 64
sleep 7.5
cc 6 11 61
cc 7 11 61
sleep 7.5
cc 2 11 65
cc 3 11 65
cc 4 11 65
sleep 12.5
cc 6 11 62
cc 7 11 62
sleep 2.5
cc 2 11 66
cc 3 11 66
cc 4 11 66
sleep 15.416
cc 2 11 67
cc 3 11 67
cc 4 11 67
sleep 2.083
cc 6 11 63
cc 7 11 63
sleep 12.916
cc 2 11 68
cc 3 11 68
cc 4 11 68
sleep 7.083
cc 6 11 64
cc 7 11 64
sleep 7.916
cc 2 11 69
cc 3 11 69
cc 4 11 69
sleep 12.083
cc 6 11 65
cc 7 11 65
sleep 2.916
cc 2 11 70
cc 3 11 70
cc 4 11 70
sleep 7.5
noteoff 6 64 0
noteoff 7 64 0
sleep 7.5
cc 2 11 71
cc 3 11 71
cc 4 11 71
sleep 15.0
cc 2 11 72
cc 3 11 72
cc 4 11 72
sleep 15.0
cc 2 11 73
cc 3 11 73
cc 4 11 73
sleep 15.0
cc 2 11 74
cc 3 11 74
cc 4 11 74
sleep 15.0
cc 2 11 75
cc 3 11 75
cc 4 11 75
sleep 15.0
cc 2 11 76
cc 3 11 76
cc 4 11 76
sleep 15.0
cc 2 11 77
cc 3 11 77
cc 4 11 77
sleep 2.5
noteon 6 62 120
noteon 7 62 120
cc 6 11 70
cc 7 11 70
sleep 4.583
cc 6 11 71
cc 7 11 71
sleep 7.916
cc 2 11 78
cc 3 11 78
cc 4 11 78
sleep 2.083
cc 6 11 72
cc 7 11 72
sleep 10.0
cc 6 11 73
cc 7 11 73
sleep 2.916
cc 2 11 79
cc 3 11 79
cc 4 11 79
sleep 7.083
cc 6 11 74
cc 7 11 74
sleep 7.916
cc 2 11 80
cc 3 11 80
cc 4 11 80
sleep 2.083
cc 6 11 75
cc 7 11 75
sleep 10.0
cc 6 11 76
cc 7 11 76
sleep 2.916
cc 2 11 81
cc 3 11 81
cc 4 11 81
sleep 7.083
cc 6 11 77
cc 7 11 77
sleep 7.916
cc 2 11 82
cc 3 11 82
cc 4 11 82
sleep 2.083
cc 6 11 78
cc 7 11 78
sleep 10.0
cc 6 11 79
cc 7 11 79
sleep 2.916
cc 2 11 83
cc 3 11 83
cc 4 11 83
sleep 7.083
cc 6 11 80
cc 7 11 80
sleep 5.416
noteoff 6 62 0
noteoff 7 62 0
sleep 2.5
cc 2 11 84
cc 3 11 84
cc 4 11 84
sleep 15.0
cc 2 11 85
cc 3 11 85
cc 4 11 85
sleep 15.0
cc 2 11 86
cc 3 11 86
cc 4 11 86
sleep 15.0
cc 2 11 87
cc 3 11 87
cc 4 11 87
sleep 15.0
cc 2 11 88
cc 3 11 88
cc 4 11 88
sleep 15.0
cc 2 11 89
cc 3 11 89
cc 4 11 89
sleep 15.0
cc 2 11 90
cc 3 11 90
cc 4 11 90
sleep 7.5
echo "measure 118 - $$ Page 44, Bottom, 1st"
cc 3 68 127
noteon 2 81 120
noteon 4 69 120
noteon 6 61 120
noteon 7 61 120
noteon 8 66 120
noteon 10 59 120
cc 6 11 90
cc 7 11 90
cc 8 11 80
cc 10 11 80
noteoff 2 79 0
noteoff 4 71 0
cc 2 68 0
sleep 7.083
cc 4 11 89
sleep 0.416
cc 2 11 89
cc 3 11 89
cc 6 11 89
cc 7 11 89
sleep 2.5
cc 8 11 79
cc 10 11 79
sleep 12.083
cc 4 11 88
sleep 0.416
cc 2 11 88
sleep 0.416
cc 3 11 88
cc 6 11 88
cc 7 11 88
sleep 7.083
cc 8 11 78
cc 10 11 78
sleep 6.666
cc 4 11 87
sleep 0.833
cc 2 11 87
sleep 0.833
cc 3 11 87
cc 6 11 87
cc 7 11 87
sleep 11.666
cc 8 11 77
cc 10 11 77
sleep 1.666
cc 4 11 86
sleep 0.833
cc 2 11 86
sleep 1.25
cc 3 11 86
cc 6 11 86
cc 7 11 86
sleep 12.5
cc 4 11 85
sleep 1.25
cc 2 11 85
sleep 1.666
cc 3 11 85
cc 6 11 85
cc 7 11 85
sleep 0.833
cc 8 11 76
cc 10 11 76
sleep 11.25
cc 4 11 84
sleep 1.25
cc 2 11 84
sleep 2.083
cc 3 11 84
cc 6 11 84
cc 7 11 84
sleep 5.416
cc 8 11 75
cc 10 11 75
sleep 6.25
cc 4 11 83
sleep 1.25
cc 2 11 83
sleep 2.083
cc 3 11 83
cc 6 11 83
cc 7 11 83
sleep 0.416
noteoff 6 61 0
noteoff 7 61 0
sleep 10.0
cc 8 11 74
cc 10 11 74
sleep 0.833
cc 4 11 82
sleep 1.666
cc 2 11 82
sleep 2.5
cc 3 11 82
sleep 10.833
cc 4 11 81
sleep 1.666
cc 2 11 81
sleep 2.5
cc 8 11 73
cc 10 11 73
sleep 0.416
cc 3 11 81
sleep 10.0
cc 4 11 80
sleep 2.083
cc 2 11 80
sleep 3.333
cc 3 11 80
sleep 4.166
cc 8 11 72
cc 10 11 72
sleep 5.416
cc 4 11 79
sleep 2.083
cc 2 11 79
sleep 3.75
cc 3 11 79
sleep 8.75
cc 4 11 78
cc 8 11 71
cc 10 11 71
sleep 2.5
cc 2 11 78
sleep 4.166
cc 3 11 78
sleep 8.333
cc 4 11 77
sleep 2.5
cc 2 11 77
sleep 2.5
cc 8 11 70
cc 10 11 70
sleep 2.083
cc 3 11 77
sleep 7.5
cc 4 11 76
sleep 0.416
noteon 3 74 120
noteon 6 59 120
noteon 7 59 120
cc 6 11 77
cc 7 11 77
noteoff 3 76 0
sleep 2.5
cc 2 11 76
sleep 5.0
cc 3 11 76
cc 6 11 76
cc 7 11 76
sleep 2.5
cc 8 11 69
cc 10 11 69
sleep 4.583
cc 4 11 75
sleep 2.916
cc 2 11 75
sleep 5.416
cc 3 11 75
cc 6 11 75
cc 7 11 75
sleep 6.666
cc 4 11 74
sleep 0.416
cc 8 11 68
cc 10 11 68
sleep 2.5
cc 2 11 74
sleep 5.833
cc 3 11 74
cc 6 11 74
cc 7 11 74
sleep 5.833
cc 4 11 73
sleep 3.333
cc 2 11 73
sleep 2.5
cc 8 11 67
cc 10 11 67
sleep 3.75
cc 3 11 73
cc 6 11 73
cc 7 11 73
sleep 5.416
cc 4 11 72
sleep 3.333
cc 2 11 72
sleep 6.666
cc 3 11 72
cc 6 11 72
cc 7 11 72
sleep 0.833
cc 8 11 66
cc 10 11 66
sleep 3.75
cc 4 11 71
sleep 3.75
cc 2 11 71
sleep 7.083
cc 3 11 71
cc 6 11 71
cc 7 11 71
sleep 4.166
cc 4 11 70
sleep 1.25
cc 8 11 65
cc 10 11 65
sleep 2.5
cc 2 11 70
sleep 7.083
cc 3 11 70
cc 6 11 70
cc 7 11 70
sleep 0.416
noteoff 6 59 0
noteoff 7 59 0
sleep 3.333
cc 4 11 69
sleep 4.166
cc 2 11 69
sleep 2.5
cc 8 11 64
cc 10 11 64
sleep 5.0
cc 3 11 69
sleep 3.333
cc 4 11 68
sleep 4.166
cc 2 11 68
sleep 7.5
cc 8 11 63
cc 10 11 63
sleep 0.416
cc 3 11 68
sleep 2.5
cc 4 11 67
sleep 4.583
cc 2 11 67
sleep 8.333
cc 3 11 67
sleep 2.083
cc 4 11 66
sleep 2.083
cc 8 11 62
cc 10 11 62
sleep 2.5
cc 2 11 66
sleep 8.75
cc 3 11 66
sleep 1.666
cc 4 11 65
sleep 4.583
cc 2 11 65
sleep 2.5
cc 8 11 61
cc 10 11 61
sleep 6.666
cc 3 11 65
sleep 0.833
cc 4 11 64
sleep 5.0
cc 2 11 64
sleep 7.5
cc 8 11 60
cc 10 11 60
sleep 2.083
cc 3 11 64
sleep 0.416
cc 4 11 63
sleep 5.0
cc 2 11 63
sleep 2.5
noteon 3 73 120
noteon 4 67 120
noteon 6 57 120
noteon 7 57 120
cc 6 11 64
cc 7 11 64
noteoff 3 74 0
noteoff 4 69 0
cc 3 68 0
cc 4 68 0
sleep 7.083
cc 3 11 63
cc 6 11 63
cc 7 11 63
sleep 0.416
cc 4 11 62
sleep 2.5
cc 8 11 59
cc 10 11 59
sleep 2.5
cc 2 11 62
sleep 8.75
cc 3 11 62
cc 6 11 62
cc 7 11 62
sleep 1.666
cc 4 11 61
sleep 4.583
cc 2 11 61
sleep 2.5
cc 8 11 58
cc 10 11 58
sleep 5.416
cc 3 11 61
cc 6 11 61
cc 7 11 61
sleep 2.916
cc 4 11 60
sleep 4.166
cc 2 11 60
sleep 7.083
cc 3 11 60
cc 6 11 60
cc 7 11 60
sleep 0.416
cc 8 11 57
cc 10 11 57
sleep 3.75
cc 4 11 59
sleep 3.75
cc 2 11 59
sleep 6.666
cc 3 11 59
cc 6 11 59
cc 7 11 59
sleep 5.0
cc 4 11 58
sleep 0.833
cc 8 11 56
cc 10 11 56
sleep 2.5
cc 2 11 58
sleep 5.833
cc 3 11 58
cc 6 11 58
cc 7 11 58
sleep 6.25
cc 4 11 57
sleep 2.916
cc 2 11 57
sleep 2.5
cc 8 11 55
cc 10 11 55
sleep 2.5
cc 3 11 57
cc 6 11 57
cc 7 11 57
sleep 7.5
cc 4 11 56
noteoff 6 57 0
noteoff 7 57 0
sleep 2.5
cc 2 11 56
sleep 4.583
cc 3 11 56
sleep 2.916
cc 8 11 54
cc 10 11 54
sleep 5.0
cc 4 11 55
sleep 2.5
cc 2 11 55
sleep 3.75
cc 3 11 55
sleep 8.75
cc 8 11 53
cc 10 11 53
sleep 0.416
cc 4 11 54
sleep 2.083
cc 2 11 54
sleep 2.916
cc 3 11 54
sleep 10.416
cc 4 11 53
sleep 1.666
cc 2 11 53
sleep 2.083
cc 3 11 53
sleep 0.416
cc 8 11 52
cc 10 11 52
sleep 11.25
cc 4 11 52
sleep 1.25
cc 2 11 52
sleep 1.666
cc 3 11 52
sleep 5.833
cc 8 11 51
cc 10 11 51
sleep 6.666
cc 4 11 51
sleep 0.833
cc 2 11 51
sleep 0.833
cc 3 11 51
sleep 11.666
cc 8 11 50
cc 10 11 50
sleep 2.083
cc 4 11 50
sleep 0.416
cc 2 11 50
cc 3 11 50
sleep 7.5
echo "measure 119 - $$ Page 44, Bottom, 2nd"
noteoff 2 81 0
noteoff 3 73 0
noteoff 4 67 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 78 120
noteon 3 74 120
noteon 4 66 120
noteon 6 62 120
noteon 7 62 120
noteon 8 68 120
noteon 10 64 120
cc 2 68 127
cc 3 68 127
cc 4 68 127
cc 6 68 127
cc 6 11 50
cc 7 68 127
cc 7 11 50
cc 8 68 127
cc 10 68 127
sleep 200.0
noteon 2 79 120
noteon 3 76 120
noteon 4 71 120
noteon 6 55 120
noteon 7 55 120
noteon 8 69 120
noteon 10 66 120
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 66 0
noteoff 6 62 0
noteoff 7 62 0
noteoff 8 68 0
noteoff 10 64 0
cc 2 68 0
cc 3 68 0
cc 4 68 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteoff 4 71 0
noteoff 6 55 0
noteoff 7 55 0
noteoff 8 69 0
noteoff 10 66 0
noteon 2 76 120
noteon 3 73 120
noteon 4 67 120
noteon 6 57 120
noteon 7 57 120
noteon 8 66 120
noteon 10 59 120
sleep 200.0
echo "measure 120 - $$ Page 44, Bottom, 3rd"
noteoff 2 76 0
noteoff 3 73 0
noteoff 4 67 0
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 66 0
noteoff 10 59 0
noteon 2 74 120
noteon 3 74 120
noteon 4 66 120
noteon 6 50 120
noteon 7 50 120
noteon 8 64 120
noteon 10 56 120
sleep 200.0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 66 0
noteoff 6 50 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 56 0
sleep 200.0
noteon 14 62 120
cc 14 68 127
sleep 100.0
noteon 14 64 120
noteoff 14 62 0
cc 14 68 0
sleep 100.0
echo "measure 121 - $$ Page 44, Bottom, 4th"
noteoff 14 64 0
noteon 8 68 120
noteon 10 52 120
noteon 14 66 120
noteon 17 62 120
noteon 24 50 120
noteon 25 50 120
noteon 26 38 120
cc 8 68 127
cc 14 68 127
sleep 200.0
noteoff 24 50 0
noteoff 25 50 0
noteoff 26 38 0
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
sleep 200.0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 24 61 120
noteon 25 61 120
noteon 26 49 120
sleep 200.0
echo "measure 122 - $$ Page 44, Bottom, 5th"
cc 17 68 127
noteoff 24 61 0
noteoff 25 61 0
noteoff 26 49 0
noteon 8 69 120
noteon 14 67 120
noteon 24 59 120
noteon 25 59 120
noteon 26 47 120
noteoff 8 68 0
noteoff 14 66 0
cc 8 68 0
cc 14 68 0
sleep 200.0
noteoff 24 59 0
noteoff 25 59 0
noteoff 26 47 0
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
noteoff 17 62 0
sleep 200.0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 17 59 120
noteon 24 55 120
noteon 25 55 120
noteon 26 43 120
noteoff 17 61 0
cc 17 68 0
sleep 200.0
echo "measure 123 - $$ Page 44, Bottom, 6th"
noteoff 8 69 0
noteoff 10 52 0
noteoff 14 67 0
noteoff 17 59 0
noteoff 24 55 0
noteoff 25 55 0
noteoff 26 43 0
noteon 8 71 120
noteon 10 52 120
noteon 14 69 120
noteon 17 57 120
noteon 24 54 120
noteon 25 54 120
noteon 26 42 120
cc 8 68 127
cc 10 68 127
cc 14 68 127
cc 17 68 127
sleep 200.0
noteoff 24 54 0
noteoff 25 54 0
noteoff 26 42 0
noteon 8 69 120
noteon 10 51 120
noteon 14 67 120
noteon 17 59 120
noteon 24 52 120
noteon 25 52 120
noteon 26 40 120
noteoff 8 71 0
noteoff 10 52 0
noteoff 14 69 0
noteoff 17 57 0
cc 8 68 0
cc 10 68 0
sleep 200.0
noteoff 8 69 0
noteoff 10 51 0
noteoff 24 52 0
noteoff 25 52 0
noteoff 26 40 0
noteon 8 68 120
noteon 10 52 120
noteon 14 66 120
noteon 17 62 120
noteon 24 50 120
noteon 25 50 120
noteon 26 38 120
noteoff 14 67 0
noteoff 17 59 0
cc 14 68 0
cc 17 68 0
sleep 200.0
echo "measure 124 - $$ Page 44, Bottom, 7th"
noteoff 8 68 0
noteoff 10 52 0
noteoff 14 66 0
noteoff 17 62 0
noteoff 24 50 0
noteoff 25 50 0
noteoff 26 38 0
noteon 8 66 120
noteon 10 47 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
noteoff 8 66 0
noteoff 10 47 0
noteoff 14 64 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
sleep 200.0
noteon 6 57 120
noteon 7 57 120
noteon 8 68 120
noteon 14 66 120
noteon 17 63 120
noteon 24 54 120
noteon 25 54 120
noteon 26 42 120
cc 14 68 127
cc 17 68 127
sleep 200.0
echo "measure 125 - $$ Page 44, Bottom, 8th"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 68 0
noteoff 24 54 0
noteoff 25 54 0
noteoff 26 42 0
noteon 6 59 120
noteon 7 59 120
noteon 8 69 120
noteon 14 67 120
noteon 17 64 120
noteon 24 52 120
noteon 25 52 120
noteon 26 40 120
cc 6 68 127
cc 7 68 127
cc 8 68 127
noteoff 14 66 0
noteoff 17 63 0
cc 17 68 0
sleep 9.583
cc 6 11 51
cc 7 11 51
cc 8 11 51
cc 14 11 51
cc 17 11 51
cc 24 11 51
cc 25 11 51
cc 26 11 51
sleep 20.0
cc 6 11 52
cc 7 11 52
cc 8 11 52
cc 14 11 52
cc 17 11 52
cc 24 11 52
cc 25 11 52
cc 26 11 52
sleep 20.0
cc 6 11 53
cc 7 11 53
cc 8 11 53
cc 14 11 53
cc 17 11 53
cc 24 11 53
cc 25 11 53
cc 26 11 53
sleep 20.0
cc 6 11 54
cc 7 11 54
cc 8 11 54
cc 14 11 54
cc 17 11 54
cc 24 11 54
cc 25 11 54
cc 26 11 54
sleep 20.0
cc 6 11 55
cc 7 11 55
cc 8 11 55
cc 14 11 55
cc 17 11 55
cc 24 11 55
cc 25 11 55
cc 26 11 55
sleep 20.0
cc 6 11 56
cc 7 11 56
cc 8 11 56
cc 14 11 56
cc 17 11 56
cc 24 11 56
cc 25 11 56
cc 26 11 56
sleep 20.0
cc 6 11 57
cc 7 11 57
cc 8 11 57
cc 14 11 57
cc 17 11 57
cc 24 11 57
cc 25 11 57
cc 26 11 57
sleep 20.0
cc 6 11 58
cc 7 11 58
cc 8 11 58
cc 14 11 58
cc 17 11 58
cc 24 11 58
cc 25 11 58
cc 26 11 58
sleep 20.0
cc 6 11 59
cc 7 11 59
cc 8 11 59
cc 14 11 59
cc 17 11 59
cc 24 11 59
cc 25 11 59
cc 26 11 59
sleep 20.0
cc 6 11 60
cc 7 11 60
cc 8 11 60
cc 14 11 60
cc 17 11 60
cc 24 11 60
cc 25 11 60
cc 26 11 60
sleep 10.416
noteoff 24 52 0
noteoff 25 52 0
noteoff 26 40 0
noteon 24 64 120
noteon 25 64 120
noteon 26 52 120
sleep 9.583
cc 6 11 61
cc 7 11 61
cc 8 11 61
cc 14 11 61
cc 17 11 61
cc 24 11 61
cc 25 11 61
cc 26 11 61
sleep 20.0
cc 6 11 62
cc 7 11 62
cc 8 11 62
cc 14 11 62
cc 17 11 62
cc 24 11 62
cc 25 11 62
cc 26 11 62
sleep 20.0
cc 6 11 63
cc 7 11 63
cc 8 11 63
cc 14 11 63
cc 17 11 63
cc 24 11 63
cc 25 11 63
cc 26 11 63
sleep 20.0
cc 6 11 64
cc 7 11 64
cc 8 11 64
cc 14 11 64
cc 17 11 64
cc 24 11 64
cc 25 11 64
cc 26 11 64
sleep 20.0
cc 6 11 65
cc 7 11 65
cc 8 11 65
cc 14 11 65
cc 17 11 65
cc 24 11 65
cc 25 11 65
cc 26 11 65
sleep 20.0
cc 6 11 66
cc 7 11 66
cc 8 11 66
cc 14 11 66
cc 17 11 66
cc 24 11 66
cc 25 11 66
cc 26 11 66
sleep 20.0
cc 6 11 67
cc 7 11 67
cc 8 11 67
cc 14 11 67
cc 17 11 67
sleep 0.416
cc 24 11 67
cc 25 11 67
cc 26 11 67
sleep 19.583
cc 6 11 68
cc 7 11 68
cc 8 11 68
cc 14 11 68
cc 17 11 68
sleep 0.416
cc 24 11 68
cc 25 11 68
cc 26 11 68
sleep 19.583
cc 6 11 69
cc 7 11 69
cc 8 11 69
cc 14 11 69
cc 17 11 69
sleep 0.416
cc 24 11 69
cc 25 11 69
cc 26 11 69
sleep 19.583
cc 6 11 70
cc 7 11 70
cc 8 11 70
cc 14 11 70
cc 17 11 70
sleep 0.416
cc 24 11 70
cc 25 11 70
cc 26 11 70
sleep 10.0
noteoff 24 64 0
noteoff 25 64 0
noteoff 26 52 0
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
sleep 9.583
cc 6 11 71
cc 7 11 71
cc 8 11 71
cc 14 11 71
cc 17 11 71
sleep 0.416
cc 24 11 71
cc 25 11 71
cc 26 11 71
sleep 20.0
cc 6 11 72
cc 7 11 72
cc 8 11 72
cc 14 11 72
cc 17 11 72
cc 24 11 72
cc 25 11 72
cc 26 11 72
sleep 20.0
cc 6 11 73
cc 7 11 73
cc 8 11 73
cc 14 11 73
cc 17 11 73
cc 24 11 73
cc 25 11 73
cc 26 11 73
sleep 20.0
cc 6 11 74
cc 7 11 74
cc 8 11 74
cc 14 11 74
cc 17 11 74
cc 24 11 74
cc 25 11 74
cc 26 11 74
sleep 20.0
cc 6 11 75
cc 7 11 75
cc 8 11 75
cc 14 11 75
cc 17 11 75
cc 24 11 75
cc 25 11 75
cc 26 11 75
sleep 20.0
cc 6 11 76
cc 7 11 76
cc 8 11 76
cc 14 11 76
cc 17 11 76
cc 24 11 76
cc 25 11 76
cc 26 11 76
sleep 20.0
cc 6 11 77
cc 7 11 77
cc 8 11 77
cc 14 11 77
cc 17 11 77
cc 24 11 77
cc 25 11 77
cc 26 11 77
sleep 20.0
cc 6 11 78
cc 7 11 78
cc 8 11 78
cc 14 11 78
cc 17 11 78
cc 24 11 78
cc 25 11 78
cc 26 11 78
sleep 20.0
cc 6 11 79
cc 7 11 79
cc 8 11 79
cc 14 11 79
cc 17 11 79
cc 24 11 79
cc 25 11 79
cc 26 11 79
sleep 20.0
cc 6 11 80
cc 7 11 80
cc 8 11 80
cc 14 11 80
cc 17 11 80
cc 24 11 80
cc 25 11 80
cc 26 11 80
sleep 10.0
echo "measure 126 - $$ Page 44, Bottom, 9th"
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 6 57 120
noteon 7 57 120
noteon 8 71 120
noteon 10 59 120
noteon 14 69 120
noteon 24 61 120
noteon 25 61 120
noteon 26 49 120
cc 10 11 80
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 69 0
noteoff 14 67 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 14 68 0
sleep 200.0
noteoff 17 64 0
noteoff 24 61 0
noteoff 25 61 0
noteoff 26 49 0
noteon 17 62 120
noteon 24 59 120
noteon 25 59 120
noteon 26 47 120
cc 17 11 50
cc 24 11 50
cc 25 11 50
cc 26 11 50
sleep 200.0
noteoff 17 62 0
noteoff 24 59 0
noteoff 25 59 0
noteoff 26 47 0
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 127 - $$ Page 44, Bottom, 10th"
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 71 0
noteoff 10 59 0
noteoff 14 69 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 6 57 120
noteon 7 57 120
noteon 8 68 120
noteon 10 64 120
noteon 14 66 120
noteon 17 62 120
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
cc 6 68 127
cc 6 11 50
cc 7 68 127
cc 7 11 50
cc 8 68 127
cc 8 11 50
cc 10 68 127
cc 10 11 50
cc 14 68 127
cc 14 11 50
cc 17 68 127
sleep 200.0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 6 59 120
noteon 7 59 120
noteon 8 69 120
noteon 10 66 120
noteon 14 67 120
noteon 17 64 120
noteon 24 55 120
noteon 25 55 120
noteon 26 43 120
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 68 0
noteoff 10 64 0
noteoff 14 66 0
noteoff 17 62 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
cc 14 68 0
cc 17 68 0
sleep 200.0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 69 0
noteoff 10 66 0
noteoff 14 67 0
noteoff 17 64 0
noteoff 24 55 0
noteoff 25 55 0
noteoff 26 43 0
noteon 2 76 120
noteon 3 73 120
noteon 6 55 120
noteon 7 55 120
noteon 8 66 120
noteon 10 59 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 128 - $$ Page 44, Bottom, 11th"
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 55 0
noteoff 7 55 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 64 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 2 78 120
noteon 3 74 120
noteon 6 54 120
noteon 7 54 120
noteon 8 64 120
noteon 10 56 120
noteon 14 62 120
noteon 17 62 120
noteon 24 50 120
noteon 25 50 120
noteon 26 38 120
cc 2 68 127
cc 3 68 127
sleep 200.0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 14 62 0
noteoff 17 62 0
noteoff 24 50 0
noteoff 25 50 0
noteoff 26 38 0
noteon 2 79 120
noteon 3 76 120
noteoff 2 78 0
noteoff 3 74 0
cc 2 68 0
cc 3 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteon 2 76 120
noteon 3 73 120
noteon 6 55 120
noteon 7 55 120
noteon 8 66 120
noteon 10 59 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 129 - $$ Page 44, Bottom, 12th"
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 55 0
noteoff 7 55 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 64 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 2 74 120
noteon 3 74 120
noteon 6 57 120
noteon 7 57 120
noteon 8 68 120
noteon 10 64 120
noteon 14 66 120
noteon 17 62 120
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
cc 6 68 127
cc 7 68 127
cc 8 68 127
cc 10 68 127
cc 14 68 127
cc 17 68 127
sleep 200.0
noteoff 2 74 0
noteoff 3 74 0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 6 59 120
noteon 7 59 120
noteon 8 69 120
noteon 10 66 120
noteon 14 67 120
noteon 17 64 120
noteon 24 55 120
noteon 25 55 120
noteon 26 43 120
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 68 0
noteoff 10 64 0
noteoff 14 66 0
noteoff 17 62 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
cc 14 68 0
cc 17 68 0
sleep 200.0
noteoff 6 59 0
noteoff 7 59 0
noteoff 8 69 0
noteoff 10 66 0
noteoff 14 67 0
noteoff 17 64 0
noteoff 24 55 0
noteoff 25 55 0
noteoff 26 43 0
noteon 2 76 120
noteon 3 73 120
noteon 6 55 120
noteon 7 55 120
noteon 8 66 120
noteon 10 59 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 130 - $$ Page 44, Bottom, 13th"
noteoff 2 76 0
noteoff 3 73 0
noteoff 6 55 0
noteoff 7 55 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 64 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 2 78 120
noteon 3 74 120
noteon 6 54 120
noteon 7 54 120
noteon 8 64 120
noteon 10 56 120
noteon 14 62 120
noteon 17 62 120
noteon 24 50 120
noteon 25 50 120
noteon 26 38 120
sleep 9.583
cc 2 11 51
cc 3 11 51
cc 6 11 51
cc 7 11 51
cc 8 11 51
cc 10 11 51
cc 14 11 51
cc 17 11 51
cc 24 11 51
cc 25 11 51
cc 26 11 51
sleep 20.0
cc 2 11 52
cc 3 11 52
cc 6 11 52
cc 7 11 52
cc 8 11 52
cc 10 11 52
cc 14 11 52
cc 17 11 52
cc 24 11 52
cc 25 11 52
cc 26 11 52
sleep 20.0
cc 2 11 53
cc 3 11 53
cc 6 11 53
cc 7 11 53
cc 8 11 53
cc 10 11 53
cc 14 11 53
cc 17 11 53
cc 24 11 53
cc 25 11 53
cc 26 11 53
sleep 20.0
cc 2 11 54
cc 3 11 54
cc 6 11 54
cc 7 11 54
cc 8 11 54
cc 10 11 54
cc 14 11 54
cc 17 11 54
cc 24 11 54
cc 25 11 54
cc 26 11 54
sleep 20.0
cc 2 11 55
cc 3 11 55
cc 6 11 55
cc 7 11 55
cc 8 11 55
cc 10 11 55
cc 14 11 55
cc 17 11 55
cc 24 11 55
cc 25 11 55
cc 26 11 55
sleep 20.0
cc 2 11 56
cc 3 11 56
cc 6 11 56
cc 7 11 56
cc 8 11 56
cc 10 11 56
cc 14 11 56
cc 17 11 56
cc 24 11 56
cc 25 11 56
cc 26 11 56
sleep 20.0
cc 2 11 57
cc 3 11 57
cc 6 11 57
cc 7 11 57
cc 8 11 57
cc 10 11 57
cc 14 11 57
cc 17 11 57
cc 24 11 57
cc 25 11 57
cc 26 11 57
sleep 20.0
cc 2 11 58
cc 3 11 58
cc 6 11 58
cc 7 11 58
cc 8 11 58
cc 10 11 58
cc 14 11 58
cc 17 11 58
cc 24 11 58
cc 25 11 58
cc 26 11 58
sleep 20.0
cc 2 11 59
cc 3 11 59
cc 6 11 59
cc 7 11 59
cc 8 11 59
cc 10 11 59
cc 14 11 59
cc 17 11 59
cc 24 11 59
cc 25 11 59
cc 26 11 59
sleep 20.0
cc 2 11 60
cc 3 11 60
cc 6 11 60
cc 7 11 60
cc 8 11 60
cc 10 11 60
cc 14 11 60
cc 17 11 60
cc 24 11 60
cc 25 11 60
cc 26 11 60
sleep 10.416
noteoff 2 78 0
noteoff 3 74 0
noteoff 6 54 0
noteoff 7 54 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 14 62 0
noteoff 17 62 0
noteoff 24 50 0
noteoff 25 50 0
noteoff 26 38 0
noteon 2 79 120
noteon 3 76 120
sleep 9.583
cc 2 11 61
cc 3 11 61
sleep 20.0
cc 2 11 62
cc 3 11 62
sleep 20.0
cc 2 11 63
cc 3 11 63
sleep 20.0
cc 2 11 64
cc 3 11 64
sleep 20.0
cc 2 11 65
cc 3 11 65
sleep 20.0
cc 2 11 66
cc 3 11 66
sleep 20.416
cc 2 11 67
cc 3 11 67
sleep 20.0
cc 2 11 68
cc 3 11 68
sleep 20.0
cc 2 11 69
cc 3 11 69
sleep 20.0
cc 2 11 70
cc 3 11 70
sleep 10.0
noteoff 2 79 0
noteoff 3 76 0
noteon 2 76 120
noteon 3 73 120
noteon 8 66 120
noteon 10 59 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 10.0
cc 2 11 71
cc 3 11 71
cc 8 11 71
cc 10 11 71
cc 14 11 71
cc 17 11 71
cc 24 11 71
cc 25 11 71
cc 26 11 71
sleep 20.0
cc 2 11 72
cc 3 11 72
cc 8 11 72
cc 10 11 72
cc 14 11 72
cc 17 11 72
cc 24 11 72
cc 25 11 72
cc 26 11 72
sleep 20.0
cc 2 11 73
cc 3 11 73
cc 8 11 73
cc 10 11 73
cc 14 11 73
cc 17 11 73
cc 24 11 73
cc 25 11 73
cc 26 11 73
sleep 20.0
cc 2 11 74
cc 3 11 74
cc 8 11 74
cc 10 11 74
cc 14 11 74
cc 17 11 74
cc 24 11 74
cc 25 11 74
cc 26 11 74
sleep 20.0
cc 2 11 75
cc 3 11 75
cc 8 11 75
cc 10 11 75
cc 14 11 75
cc 17 11 75
cc 24 11 75
cc 25 11 75
cc 26 11 75
sleep 10.0
noteoff 2 76 0
noteoff 3 73 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 64 0
noteoff 17 61 0
sleep 10.0
cc 24 11 76
cc 25 11 76
cc 26 11 76
sleep 20.0
cc 24 11 77
cc 25 11 77
cc 26 11 77
sleep 20.0
cc 24 11 78
cc 25 11 78
cc 26 11 78
sleep 20.0
cc 24 11 79
cc 25 11 79
cc 26 11 79
sleep 20.0
cc 24 11 80
cc 25 11 80
cc 26 11 80
sleep 10.0
echo "measure 131 - $$ Page 44, Bottom, 14th"
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 2 78 120
noteon 3 74 120
noteon 8 68 120
noteon 10 64 120
noteon 14 66 120
noteon 17 62 120
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
cc 2 68 127
cc 2 11 80
cc 3 68 127
cc 3 11 80
cc 8 68 127
cc 8 11 80
cc 10 68 127
cc 10 11 80
cc 14 11 80
cc 17 11 80
sleep 100.0
noteoff 14 66 0
noteoff 17 62 0
sleep 100.0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 2 79 120
noteon 3 76 120
noteon 8 69 120
noteon 10 66 120
noteoff 2 78 0
noteoff 3 74 0
noteoff 8 68 0
noteoff 10 64 0
cc 2 68 0
cc 3 68 0
cc 8 68 0
cc 10 68 0
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteoff 8 69 0
noteoff 10 66 0
noteon 2 76 120
noteon 3 73 120
noteon 8 66 120
noteon 10 59 120
noteon 14 64 120
noteon 17 61 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 132 - $$ Page 44, Bottom, 15th"
noteoff 2 76 0
noteoff 3 73 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 64 0
noteoff 17 61 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 2 78 120
noteon 3 74 120
noteon 6 57 120
noteon 7 57 120
noteon 8 68 120
noteon 10 64 120
noteon 14 66 120
noteon 17 62 120
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
sleep 100.0
noteoff 14 66 0
noteoff 17 62 0
sleep 100.0
noteoff 2 78 0
noteoff 3 74 0
noteoff 6 57 0
noteoff 7 57 0
noteoff 8 68 0
noteoff 10 64 0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 2 79 120
noteon 3 76 120
noteon 8 69 120
noteon 10 66 120
sleep 200.0
noteoff 2 79 0
noteoff 3 76 0
noteoff 8 69 0
noteoff 10 66 0
noteon 0 88 120
noteon 1 85 120
noteon 2 76 120
noteon 3 73 120
noteon 4 73 120
noteon 5 64 120
noteon 6 45 120
noteon 7 45 120
noteon 8 66 120
noteon 10 59 120
noteon 14 73 120
noteon 17 64 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
cc 2 11 50
cc 3 11 50
cc 6 11 50
cc 7 11 50
cc 8 11 50
cc 10 11 50
cc 14 11 50
cc 17 11 50
cc 24 11 50
cc 25 11 50
cc 26 11 50
sleep 200.0
echo "measure 133 - $$ Page 44, Bottom, 16th (last)"
noteoff 0 88 0
noteoff 1 85 0
noteoff 2 76 0
noteoff 3 73 0
noteoff 4 73 0
noteoff 5 64 0
noteoff 6 45 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 59 0
noteoff 14 73 0
noteoff 17 64 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 0 90 120
noteon 1 86 120
noteon 2 78 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 57 120
noteon 7 62 120
noteon 8 68 120
noteon 10 64 120
noteon 14 74 120
noteon 17 66 120
noteon 24 62 120
noteon 25 62 120
noteon 26 50 120
cc 0 68 127
cc 1 68 127
cc 2 68 127
cc 3 68 127
cc 4 68 127
cc 5 68 127
cc 6 68 127
cc 7 68 127
cc 8 68 127
cc 10 68 127
cc 14 68 127
cc 17 68 127
sleep 200.0
noteoff 24 62 0
noteoff 25 62 0
noteoff 26 50 0
noteon 0 91 120
noteon 1 88 120
noteon 2 79 120
noteon 3 76 120
noteon 4 76 120
noteon 5 67 120
noteon 6 59 120
noteon 7 43 120
noteon 8 69 120
noteon 10 66 120
noteon 14 76 120
noteon 17 67 120
noteon 24 55 120
noteon 25 55 120
noteon 26 43 120
noteoff 0 90 0
noteoff 1 86 0
noteoff 2 78 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 57 0
noteoff 7 62 0
noteoff 8 68 0
noteoff 10 64 0
noteoff 14 74 0
noteoff 17 66 0
cc 0 68 0
cc 1 68 0
cc 2 68 0
cc 3 68 0
cc 4 68 0
cc 5 68 0
cc 6 68 0
cc 7 68 0
cc 8 68 0
cc 10 68 0
cc 14 68 0
cc 17 68 0
sleep 200.0
noteoff 0 91 0
noteoff 1 88 0
noteoff 2 79 0
noteoff 3 76 0
noteoff 4 76 0
noteoff 5 67 0
noteoff 6 59 0
noteoff 7 43 0
noteoff 8 69 0
noteoff 10 66 0
noteoff 14 76 0
noteoff 17 67 0
noteoff 24 55 0
noteoff 25 55 0
noteoff 26 43 0
noteon 0 88 120
noteon 1 85 120
noteon 2 76 120
noteon 3 73 120
noteon 4 73 120
noteon 5 64 120
noteon 6 61 120
noteon 7 45 120
noteon 8 66 120
noteon 10 71 120
noteon 14 73 120
noteon 17 64 120
noteon 24 57 120
noteon 25 57 120
noteon 26 45 120
sleep 200.0
echo "measure 134 - $$ End of song."
noteoff 0 88 0
noteoff 1 85 0
noteoff 2 76 0
noteoff 3 73 0
noteoff 4 73 0
noteoff 5 64 0
noteoff 6 61 0
noteoff 7 45 0
noteoff 8 66 0
noteoff 10 71 0
noteoff 14 73 0
noteoff 17 64 0
noteoff 24 57 0
noteoff 25 57 0
noteoff 26 45 0
noteon 0 86 120
noteon 1 86 120
noteon 2 74 120
noteon 3 74 120
noteon 4 74 120
noteon 5 66 120
noteon 6 62 120
noteon 7 50 120
noteon 8 64 120
noteon 10 56 120
noteon 14 74 120
noteon 17 66 120
noteon 24 50 120
noteon 25 50 120
noteon 26 38 120
sleep 200.0
noteoff 0 86 0
noteoff 1 86 0
noteoff 2 74 0
noteoff 3 74 0
noteoff 4 74 0
noteoff 5 66 0
noteoff 6 62 0
noteoff 7 50 0
noteoff 8 64 0
noteoff 10 56 0
noteoff 14 74 0
noteoff 17 66 0
noteoff 24 50 0
noteoff 25 50 0
noteoff 26 38 0
sleep 200.0
sleep 400.0
cc 26 121 0
quit
