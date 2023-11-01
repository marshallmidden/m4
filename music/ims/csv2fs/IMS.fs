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
noteon 0 74 120
noteon 1 74 120
cc 0 11 80
cc 0 10 64
cc 0 7 91
cc 1 11 80
cc 1 10 64
cc 1 7 91
sleep 600.0
noteoff 0 74 0
noteoff 1 74 0
sleep 600.0
noteon 0 76 120
noteon 1 74 120
sleep 600.0
noteoff 0 76 0
noteoff 1 74 0
sleep 600.0
sleep 600.0
sleep 600.0
sleep 600.0
noteon 0 86 120
noteon 1 78 120
cc 0 11 90
cc 1 11 90
sleep 200.0
noteoff 0 86 0
noteoff 1 78 0
noteon 0 85 120
noteon 1 79 120
sleep 200.0
noteoff 0 85 0
noteoff 1 79 0
noteon 0 86 120
noteon 1 78 120
sleep 200.0
noteoff 0 86 0
noteoff 1 78 0
noteon 0 74 120
noteon 1 74 120
cc 0 11 80
cc 1 11 80
sleep 600.0
noteoff 0 74 0
noteoff 1 74 0
sleep 200.0
quit
