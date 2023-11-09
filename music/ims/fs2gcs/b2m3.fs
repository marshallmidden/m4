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
echo "Header 2 120"
echo "meter 3 2 24 8"
echo "key D 'major'"
echo "Title '2 Flauti'"
select 0 1 0 73
sleep 24.999
select 0 1 0 73
sleep 20.833
pitch_bend 0 8192
sleep 1454.165
echo "tempo_s=315 tempo_l=0.25"
sleep 1.587
noteon 0 74 95
sleep 188.888
echo "tempo_s=298 tempo_l=0.25"
sleep 402.684
echo "tempo_s=315 tempo_l=0.25"
sleep 1.587
noteoff 0 74 0
sleep 61.904
quit
