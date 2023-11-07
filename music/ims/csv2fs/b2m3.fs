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
echo "Title 'Symphony n^0 2 in D major'"
echo "Title 'III - Scherzo : Allegro'"
echo "L. Van Beethoven (1770-1827)"
echo "Tempo sup'erieur `a 250 : pr'evoir correction avant publication\012"
echo "Partition compl`ete - dur'ee : 00:03:52\012"
echo "D'ebut du travail : 26 septembre 2000\012"
echo "Fin de la version 0 : 28 septembre 2000\012"
echo "Version 1.0 : 23 novembre 2000\012"
echo "Version 1.01 : 17 janvier 2001 : r'evision avant publcation\012"
echo "Dur'ee d''edition et d'audition : 6 h\012"
echo "-----\012"
echo "Full score - length : 00:03:52\012"
echo "Beginning of writing : 26 september 2000\012"
echo "End of version 0 : 28 september 2000\012"
echo "Version 1.0 : 23 november 2000\012"
echo "Version 1.01 : 17 january 2001 : revision before publcation\012"
echo "Editing and audition time :6 h\012"
echo "meter 3 2 24 8"
echo "key D 'major'"
echo "Title '2 Flauti'"
select 0 1 0 73
echo "Title '2 Oboi'"
select 1 1 0 68
echo "Title '2 Clarinetti in A'"
select 2 1 0 71
echo "Title '2 Fagotti'"
select 3 1 0 70
echo "Title 'Corno I in D'"
select 4 1 0 60
echo "Title 'Corno II in D'"
select 5 1 0 60
echo "Title '2 Trombe in D'"
select 6 1 0 56
echo "Title 'Timpani'"
select 15 1 0 47
echo "Title 'Violine I'"
select 10 1 0 48
echo "Title 'Violine II'"
select 11 1 0 48
echo "Title 'Viola'"
select 12 1 0 48
echo "Title 'Violoncelle'"
select 13 1 0 48
echo "Title 'Contrabasse'"
select 14 1 0 48
echo "Title 'Sequ. by J.F.Lucarelli'"
echo "Title 'j-f.lucarelli@infonie.be  --  http://perso.infonie.fr/espace-midi'"
echo "Title 'Optimized for AWE 64  + 8 MB memory'"
echo "Title '(c) January 2001'"
echo "Title 'Beethoven - Symphony no 2 in D - Scherzo : Allegro'"
sleep 10.415
select 10 1 0 48
sleep 2.083
select 0 1 0 73
sleep 2.083
select 1 1 0 68
select 4 1 0 60
select 11 1 0 48
sleep 2.083
select 2 1 0 71
select 6 1 0 56
sleep 4.166
pitch_bend 10 8292
sleep 2.083
pitch_bend 0 8192
select 5 1 0 60
select 12 1 0 48
sleep 2.083
pitch_bend 1 8192
pitch_bend 4 8292
pitch_bend 11 7992
sleep 2.083
pitch_bend 2 8192
pitch_bend 6 8192
sleep 2.083
select 3 1 0 70
sleep 2.083
select 13 1 0 48
sleep 2.083
pitch_bend 5 8092
select 15 1 0 47
pitch_bend 12 8492
sleep 2.083
select 14 1 0 48
sleep 4.166
pitch_bend 3 8192
sleep 2.083
pitch_bend 13 7892
sleep 2.083
pitch_bend 15 8192
sleep 2.083
pitch_bend 14 8492
sleep 704.160
noteon 10 62 102
sleep 1.587
noteon 0 74 95
sleep 1.587
noteon 1 74 94
noteon 4 62 94
noteon 11 62 102
sleep 1.587
noteon 2 74 95
noteon 2 62 95
noteon 6 74 102
noteon 6 62 102
sleep 4.761
noteon 5 50 94
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 55.358
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 21.808
noteon 10 78 102
sleep 1.587
noteoff 0 74 0
sleep 1.587
noteoff 1 74 0
noteoff 4 62 0
sleep 1.587
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 64 102
noteon 10 55 102
sleep 1.587
noteon 0 76 95
noteon 0 74 95
sleep 1.587
noteon 1 76 94
noteon 1 74 94
noteon 4 62 94
noteon 11 59 102
noteon 11 62 102
sleep 1.587
noteon 2 76 95
noteon 2 64 95
noteon 6 74 102
noteon 6 62 102
sleep 4.761
noteon 5 50 94
noteon 12 55 102
sleep 4.761
noteon 3 55 100
sleep 1.587
noteon 13 55 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 43 106
sleep 85.706
noteoff 12 55 0
sleep 4.761
noteoff 3 55 0
sleep 1.587
noteoff 13 55 0
sleep 3.174
noteoff 14 43 0
sleep 86.248
noteon 12 57 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 57 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 59 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 59 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 55.358
noteoff 10 55 0
noteoff 10 64 0
sleep 3.354
noteoff 11 62 0
noteoff 11 59 0
sleep 21.808
noteon 10 79 102
sleep 1.587
noteoff 0 74 0
noteoff 0 76 0
sleep 1.587
noteoff 1 74 0
noteoff 1 76 0
noteoff 4 62 0
sleep 1.587
noteoff 2 64 0
noteoff 2 76 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 79 0
sleep 109.523
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 122.775
noteon 10 83 102
sleep 90.747
noteoff 10 83 0
sleep 125.949
noteon 4 64 94
sleep 6.349
noteon 5 57 94
sleep 88.888
noteoff 4 64 0
sleep 6.349
noteoff 5 57 0
sleep 89.272
noteon 4 66 94
sleep 7.117
noteon 5 62 94
sleep 99.644
noteoff 4 66 0
sleep 7.117
noteoff 5 62 0
sleep 99.644
noteon 4 67 94
sleep 7.117
noteon 5 64 94
sleep 99.644
noteoff 4 67 0
sleep 7.117
noteoff 5 64 0
sleep 96.085
noteon 10 76 102
sleep 3.174
noteon 11 69 102
sleep 77.777
noteoff 10 76 0
sleep 3.174
noteoff 11 69 0
sleep 106.349
noteon 10 78 102
sleep 3.558
noteon 11 74 102
sleep 87.188
noteoff 10 78 0
sleep 3.558
noteoff 11 74 0
sleep 119.217
noteon 10 79 102
sleep 3.558
noteon 11 76 102
sleep 87.188
noteoff 10 79 0
sleep 3.558
noteoff 11 76 0
sleep 122.391
noteon 1 74 94
noteon 4 66 94
noteon 1 78 94
sleep 6.349
noteon 5 62 94
sleep 88.888
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
sleep 6.349
noteoff 5 62 0
sleep 89.272
noteon 1 76 94
noteon 1 69 94
noteon 4 64 94
sleep 7.117
noteon 5 57 94
sleep 99.644
noteoff 1 69 0
noteoff 1 76 0
noteoff 4 64 0
sleep 7.117
noteoff 5 57 0
sleep 99.644
noteon 1 66 94
noteon 1 74 94
noteon 4 62 94
sleep 7.117
noteon 5 54 94
sleep 99.644
noteoff 1 74 0
noteoff 1 66 0
noteoff 4 62 0
sleep 7.117
noteoff 5 54 0
sleep 96.085
noteon 10 86 102
sleep 1.587
noteon 0 78 101
noteon 0 86 101
sleep 1.587
noteon 11 78 102
noteon 11 69 102
sleep 1.587
noteon 2 66 101
noteon 2 74 101
noteon 6 74 102
noteon 6 62 102
sleep 4.761
noteon 12 62 102
sleep 4.761
noteon 3 62 100
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 112
sleep 1.587
noteon 14 38 106
sleep 76.19
noteoff 10 86 0
sleep 1.587
noteoff 0 86 0
noteoff 0 78 0
sleep 1.587
noteoff 11 69 0
noteoff 11 78 0
sleep 1.587
noteoff 2 74 0
noteoff 2 66 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
noteoff 3 62 0
sleep 1.587
noteoff 13 50 0
sleep 1.587
noteoff 15 50 0
sleep 1.587
noteoff 14 38 0
sleep 76.19
noteon 10 85 102
sleep 1.779
noteon 0 79 101
noteon 0 85 101
sleep 1.779
noteon 11 79 102
noteon 11 69 102
sleep 1.779
noteon 2 76 101
noteon 2 67 101
noteon 6 57 102
noteon 6 69 102
sleep 5.338
noteon 12 57 102
sleep 5.338
noteon 3 57 100
noteon 3 45 100
sleep 1.779
noteon 13 45 104
sleep 1.779
noteon 15 45 112
sleep 1.779
noteon 14 33 106
sleep 85.409
noteoff 10 85 0
sleep 1.779
noteoff 0 85 0
noteoff 0 79 0
sleep 1.779
noteoff 11 69 0
noteoff 11 79 0
sleep 1.779
noteoff 2 67 0
noteoff 2 76 0
noteoff 6 69 0
noteoff 6 57 0
sleep 5.338
noteoff 12 57 0
sleep 5.338
noteoff 3 45 0
noteoff 3 57 0
sleep 1.779
noteoff 13 45 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 33 0
sleep 85.409
noteon 10 86 102
sleep 1.779
noteon 0 86 101
noteon 0 78 101
sleep 1.779
noteon 11 78 102
noteon 11 69 102
sleep 1.779
noteon 2 66 101
noteon 2 74 101
noteon 6 62 102
noteon 6 74 102
sleep 5.338
noteon 12 62 102
sleep 5.338
noteon 3 62 100
noteon 3 50 100
sleep 1.779
noteon 13 50 104
sleep 1.779
noteon 15 50 112
sleep 1.779
noteon 14 38 106
sleep 85.409
noteoff 10 86 0
sleep 1.779
noteoff 0 78 0
noteoff 0 86 0
sleep 1.779
noteoff 11 69 0
noteoff 11 78 0
sleep 1.779
noteoff 2 74 0
noteoff 2 66 0
noteoff 6 74 0
noteoff 6 62 0
sleep 5.338
noteoff 12 62 0
sleep 5.338
noteoff 3 50 0
noteoff 3 62 0
sleep 1.779
noteoff 13 50 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 38 0
sleep 85.409
noteon 10 62 102
sleep 1.587
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 4 62 94
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
noteon 6 62 102
noteon 6 74 102
sleep 4.761
noteon 5 50 94
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 55.358
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 21.808
noteon 10 78 102
sleep 1.587
noteoff 0 74 0
sleep 1.587
noteoff 1 74 0
noteoff 4 62 0
sleep 1.587
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 64 102
sleep 1.587
noteon 0 74 101
noteon 0 76 101
sleep 1.587
noteon 1 76 100
noteon 1 74 100
noteon 4 62 94
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 2 64 101
noteon 6 62 102
noteon 6 74 102
sleep 4.761
noteon 5 50 94
noteon 12 56 102
sleep 4.761
noteon 3 56 100
sleep 1.587
noteon 13 56 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 44 106
sleep 85.706
noteoff 12 56 0
sleep 4.761
noteoff 3 56 0
sleep 1.587
noteoff 13 56 0
sleep 3.174
noteoff 14 44 0
sleep 86.248
noteon 12 57 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 57 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 59 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 59 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 55.358
noteoff 10 64 0
sleep 3.354
noteoff 11 62 0
sleep 21.808
noteon 10 83 102
sleep 1.587
noteoff 0 76 0
noteoff 0 74 0
sleep 1.587
noteoff 1 74 0
noteoff 1 76 0
noteoff 4 62 0
sleep 1.587
noteoff 2 64 0
noteoff 2 74 0
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 83 0
sleep 109.523
noteon 10 85 102
sleep 90.747
noteoff 10 85 0
sleep 122.775
noteon 10 86 102
sleep 90.747
noteoff 10 86 0
sleep 125.949
noteon 1 73 100
noteon 1 69 100
noteon 11 61 102
sleep 6.349
noteon 12 57 102
sleep 74.603
noteoff 11 61 0
sleep 6.349
noteoff 12 57 0
sleep 7.936
noteoff 1 69 0
noteoff 1 73 0
sleep 95.621
noteon 1 71 100
noteon 1 74 100
noteon 11 62 102
sleep 7.117
noteon 12 59 102
sleep 83.629
noteoff 11 62 0
sleep 7.117
noteoff 12 59 0
sleep 8.896
noteoff 1 74 0
noteoff 1 71 0
sleep 106.761
noteon 1 76 100
noteon 1 73 100
noteon 11 64 102
sleep 7.117
noteon 12 61 102
sleep 83.629
noteoff 11 64 0
sleep 7.117
noteoff 12 61 0
sleep 8.896
noteoff 1 73 0
noteoff 1 76 0
sleep 103.202
noteon 10 83 102
sleep 3.174
noteon 11 76 102
sleep 77.777
noteoff 10 83 0
sleep 3.174
noteoff 11 76 0
sleep 106.349
noteon 10 85 102
sleep 3.558
noteon 11 81 102
sleep 87.188
noteoff 10 85 0
sleep 3.558
noteoff 11 81 0
sleep 119.217
noteon 10 86 102
sleep 3.558
noteon 11 83 102
sleep 87.188
noteoff 10 86 0
sleep 3.558
noteoff 11 83 0
sleep 120.804
noteon 0 85 101
sleep 1.587
noteon 1 73 100
noteon 1 81 100
noteon 4 64 94
sleep 11.111
noteon 3 57 100
sleep 82.539
noteoff 0 85 0
sleep 1.587
noteoff 1 81 0
noteoff 1 73 0
noteoff 4 64 0
sleep 11.111
noteoff 3 57 0
sleep 82.731
noteon 0 83 101
sleep 1.779
noteon 1 80 100
noteon 1 71 100
noteon 4 64 94
sleep 12.455
noteon 3 52 100
sleep 92.526
noteoff 0 83 0
sleep 1.779
noteoff 1 71 0
noteoff 1 80 0
noteoff 4 64 0
sleep 12.455
noteoff 3 52 0
sleep 92.526
noteon 0 81 101
sleep 1.779
noteon 1 81 100
noteon 1 73 100
noteon 4 64 94
sleep 12.455
noteon 3 57 100
sleep 92.526
noteoff 0 81 0
sleep 1.779
noteoff 1 73 0
noteoff 1 81 0
noteoff 4 64 0
sleep 12.455
noteoff 3 57 0
sleep 90.747
noteon 10 81 102
sleep 1.587
noteon 0 88 101
noteon 0 85 101
sleep 1.587
noteon 4 64 94
noteon 11 73 102
noteon 11 64 102
sleep 1.587
noteon 2 73 101
noteon 6 76 102
noteon 2 69 101
sleep 4.761
noteon 5 64 94
sleep 4.761
noteon 3 45 100
noteon 3 57 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 112
sleep 1.587
noteon 14 45 106
sleep 76.19
noteoff 10 81 0
sleep 1.587
noteoff 0 85 0
noteoff 0 88 0
sleep 1.587
noteoff 4 64 0
noteoff 11 64 0
noteoff 11 73 0
sleep 1.587
noteoff 2 69 0
noteoff 2 73 0
noteoff 6 76 0
sleep 4.761
noteoff 5 64 0
sleep 4.761
noteoff 3 57 0
noteoff 3 45 0
sleep 1.587
noteoff 13 57 0
sleep 1.587
noteoff 15 45 0
sleep 1.587
noteoff 14 45 0
sleep 76.19
noteon 10 80 102
sleep 1.779
noteon 0 88 101
noteon 0 83 101
sleep 1.779
noteon 4 64 94
noteon 11 74 102
noteon 11 64 102
sleep 1.779
noteon 2 71 101
noteon 2 68 101
noteon 6 76 102
sleep 5.338
noteon 5 64 94
sleep 5.338
noteon 3 40 100
noteon 3 52 100
sleep 1.779
noteon 13 52 104
sleep 1.779
noteon 15 45 112
sleep 1.779
noteon 14 40 106
sleep 85.409
noteoff 10 80 0
sleep 1.779
noteoff 0 83 0
noteoff 0 88 0
sleep 1.779
noteoff 4 64 0
noteoff 11 64 0
noteoff 11 74 0
sleep 1.779
noteoff 2 68 0
noteoff 2 71 0
noteoff 6 76 0
sleep 5.338
noteoff 5 64 0
sleep 5.338
noteoff 3 52 0
noteoff 3 40 0
sleep 1.779
noteoff 13 52 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 40 0
sleep 85.409
noteon 10 81 102
sleep 1.779
noteon 0 88 101
noteon 0 85 101
sleep 1.779
noteon 4 64 94
noteon 11 64 102
noteon 11 73 102
sleep 1.779
noteon 2 73 101
noteon 2 69 101
noteon 6 76 102
sleep 5.338
noteon 5 64 94
sleep 5.338
noteon 3 45 100
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteon 15 45 112
sleep 1.779
noteon 14 45 106
sleep 85.409
noteoff 10 81 0
sleep 1.779
noteoff 0 85 0
noteoff 0 88 0
sleep 1.779
noteoff 4 64 0
noteoff 11 73 0
noteoff 11 64 0
sleep 1.779
noteoff 2 69 0
noteoff 2 73 0
noteoff 6 76 0
sleep 5.338
noteoff 5 64 0
sleep 5.338
noteoff 3 57 0
noteoff 3 45 0
sleep 1.779
noteoff 13 57 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 45 0
sleep 85.409
noteon 10 62 102
sleep 1.587
noteon 0 74 95
sleep 1.587
noteon 1 74 94
noteon 4 62 94
noteon 11 62 102
sleep 1.587
noteon 2 74 95
noteon 2 62 95
noteon 6 74 102
noteon 6 62 102
sleep 4.761
noteon 5 50 94
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 55.358
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 21.808
noteon 10 78 102
sleep 1.587
noteoff 0 74 0
sleep 1.587
noteoff 1 74 0
noteoff 4 62 0
sleep 1.587
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 64 102
noteon 10 55 102
sleep 1.587
noteon 0 76 95
noteon 0 74 95
sleep 1.587
noteon 1 74 94
noteon 1 76 94
noteon 4 62 94
noteon 11 59 102
noteon 11 62 102
sleep 1.587
noteon 2 76 95
noteon 2 64 95
noteon 6 74 102
noteon 6 62 102
sleep 4.761
noteon 5 50 94
noteon 12 55 102
sleep 4.761
noteon 3 55 100
sleep 1.587
noteon 13 55 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 43 106
sleep 85.706
noteoff 12 55 0
sleep 4.761
noteoff 3 55 0
sleep 1.587
noteoff 13 55 0
sleep 3.174
noteoff 14 43 0
sleep 86.248
noteon 12 57 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 57 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 59 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 59 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 55.358
noteoff 10 55 0
noteoff 10 64 0
sleep 3.354
noteoff 11 62 0
noteoff 11 59 0
sleep 21.808
noteon 10 79 102
sleep 1.587
noteoff 0 74 0
noteoff 0 76 0
sleep 1.587
noteoff 1 76 0
noteoff 1 74 0
noteoff 4 62 0
sleep 1.587
noteoff 2 64 0
noteoff 2 76 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 79 0
sleep 109.523
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 122.775
noteon 10 83 102
sleep 90.747
noteoff 10 83 0
sleep 125.949
noteon 4 64 94
sleep 6.349
noteon 5 57 94
sleep 88.888
noteoff 4 64 0
sleep 6.349
noteoff 5 57 0
sleep 89.272
noteon 4 66 94
sleep 7.117
noteon 5 62 94
sleep 99.644
noteoff 4 66 0
sleep 7.117
noteoff 5 62 0
sleep 99.644
noteon 4 67 94
sleep 7.117
noteon 5 64 94
sleep 99.644
noteoff 4 67 0
sleep 7.117
noteoff 5 64 0
sleep 96.085
noteon 10 76 102
sleep 3.174
noteon 11 69 102
sleep 77.777
noteoff 10 76 0
sleep 3.174
noteoff 11 69 0
sleep 106.349
noteon 10 78 102
sleep 3.558
noteon 11 74 102
sleep 87.188
noteoff 10 78 0
sleep 3.558
noteoff 11 74 0
sleep 119.217
noteon 10 79 102
sleep 3.558
noteon 11 76 102
sleep 87.188
noteoff 10 79 0
sleep 3.558
noteoff 11 76 0
sleep 122.391
noteon 1 74 94
noteon 4 66 94
noteon 1 78 94
sleep 6.349
noteon 5 62 94
sleep 88.888
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
sleep 6.349
noteoff 5 62 0
sleep 89.272
noteon 1 76 94
noteon 1 69 94
noteon 4 64 94
sleep 7.117
noteon 5 57 94
sleep 99.644
noteoff 1 69 0
noteoff 1 76 0
noteoff 4 64 0
sleep 7.117
noteoff 5 57 0
sleep 99.644
noteon 1 66 94
noteon 1 74 94
noteon 4 62 94
sleep 7.117
noteon 5 54 94
sleep 99.644
noteoff 1 74 0
noteoff 1 66 0
noteoff 4 62 0
sleep 7.117
noteoff 5 54 0
sleep 96.085
noteon 10 86 102
sleep 1.587
noteon 0 78 101
noteon 0 86 101
sleep 1.587
noteon 11 69 102
noteon 11 78 102
sleep 1.587
noteon 2 74 101
noteon 2 66 101
noteon 6 62 102
noteon 6 74 102
sleep 4.761
noteon 12 62 102
sleep 4.761
noteon 3 62 100
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 112
sleep 1.587
noteon 14 38 106
sleep 76.19
noteoff 10 86 0
sleep 1.587
noteoff 0 86 0
noteoff 0 78 0
sleep 1.587
noteoff 11 78 0
noteoff 11 69 0
sleep 1.587
noteoff 2 66 0
noteoff 2 74 0
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
noteoff 3 62 0
sleep 1.587
noteoff 13 50 0
sleep 1.587
noteoff 15 50 0
sleep 1.587
noteoff 14 38 0
sleep 76.19
noteon 10 85 102
sleep 1.779
noteon 0 85 101
noteon 0 79 101
sleep 1.779
noteon 11 79 102
noteon 11 69 102
sleep 1.779
noteon 2 76 101
noteon 2 67 101
noteon 6 69 102
noteon 6 57 102
sleep 5.338
noteon 12 57 102
sleep 5.338
noteon 3 57 100
noteon 3 45 100
sleep 1.779
noteon 13 45 104
sleep 1.779
noteon 15 45 112
sleep 1.779
noteon 14 33 106
sleep 85.409
noteoff 10 85 0
sleep 1.779
noteoff 0 79 0
noteoff 0 85 0
sleep 1.779
noteoff 11 69 0
noteoff 11 79 0
sleep 1.779
noteoff 2 67 0
noteoff 2 76 0
noteoff 6 57 0
noteoff 6 69 0
sleep 5.338
noteoff 12 57 0
sleep 5.338
noteoff 3 45 0
noteoff 3 57 0
sleep 1.779
noteoff 13 45 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 33 0
sleep 85.409
noteon 10 86 102
sleep 1.779
noteon 0 86 101
noteon 0 78 101
sleep 1.779
noteon 11 69 102
noteon 11 78 102
sleep 1.779
noteon 2 74 101
noteon 2 66 101
noteon 6 74 102
noteon 6 62 102
sleep 5.338
noteon 12 62 102
sleep 5.338
noteon 3 62 100
noteon 3 50 100
sleep 1.779
noteon 13 50 104
sleep 1.779
noteon 15 50 112
sleep 1.779
noteon 14 38 106
sleep 85.409
noteoff 10 86 0
sleep 1.779
noteoff 0 78 0
noteoff 0 86 0
sleep 1.779
noteoff 11 78 0
noteoff 11 69 0
sleep 1.779
noteoff 2 66 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 12 62 0
sleep 5.338
noteoff 3 50 0
noteoff 3 62 0
sleep 1.779
noteoff 13 50 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 38 0
sleep 85.409
noteon 10 62 102
sleep 1.587
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 4 62 94
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
noteon 6 62 102
noteon 6 74 102
sleep 4.761
noteon 5 50 94
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 55.358
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 21.808
noteon 10 78 102
sleep 1.587
noteoff 0 74 0
sleep 1.587
noteoff 1 74 0
noteoff 4 62 0
sleep 1.587
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 64 102
sleep 1.587
noteon 0 76 101
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 1 76 100
noteon 4 62 94
noteon 11 62 102
sleep 1.587
noteon 2 64 101
noteon 2 74 101
noteon 6 62 102
noteon 6 74 102
sleep 4.761
noteon 5 50 94
noteon 12 56 102
sleep 4.761
noteon 3 56 100
sleep 1.587
noteon 13 56 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 44 106
sleep 85.706
noteoff 12 56 0
sleep 4.761
noteoff 3 56 0
sleep 1.587
noteoff 13 56 0
sleep 3.174
noteoff 14 44 0
sleep 86.248
noteon 12 57 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 57 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 59 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 59 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 55.358
noteoff 10 64 0
sleep 3.354
noteoff 11 62 0
sleep 21.808
noteon 10 83 102
sleep 1.587
noteoff 0 74 0
noteoff 0 76 0
sleep 1.587
noteoff 1 76 0
noteoff 1 74 0
noteoff 4 62 0
sleep 1.587
noteoff 2 74 0
noteoff 2 64 0
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 83 0
sleep 109.523
noteon 10 85 102
sleep 90.747
noteoff 10 85 0
sleep 122.775
noteon 10 86 102
sleep 90.747
noteoff 10 86 0
sleep 125.949
noteon 1 69 100
noteon 1 73 100
noteon 11 61 102
sleep 6.349
noteon 12 57 102
sleep 74.603
noteoff 11 61 0
sleep 6.349
noteoff 12 57 0
sleep 7.936
noteoff 1 73 0
noteoff 1 69 0
sleep 95.621
noteon 1 71 100
noteon 1 74 100
noteon 11 62 102
sleep 7.117
noteon 12 59 102
sleep 83.629
noteoff 11 62 0
sleep 7.117
noteoff 12 59 0
sleep 8.896
noteoff 1 74 0
noteoff 1 71 0
sleep 106.761
noteon 1 73 100
noteon 1 76 100
noteon 11 64 102
sleep 7.117
noteon 12 61 102
sleep 83.629
noteoff 11 64 0
sleep 7.117
noteoff 12 61 0
sleep 8.896
noteoff 1 76 0
noteoff 1 73 0
sleep 103.202
noteon 10 83 102
sleep 3.174
noteon 11 76 102
sleep 77.777
noteoff 10 83 0
sleep 3.174
noteoff 11 76 0
sleep 106.349
noteon 10 85 102
sleep 3.558
noteon 11 81 102
sleep 87.188
noteoff 10 85 0
sleep 3.558
noteoff 11 81 0
sleep 119.217
noteon 10 86 102
sleep 3.558
noteon 11 83 102
sleep 87.188
noteoff 10 86 0
sleep 3.558
noteoff 11 83 0
sleep 120.804
noteon 0 85 101
sleep 1.587
noteon 1 81 100
noteon 1 73 100
noteon 4 64 94
sleep 11.111
noteon 3 57 100
sleep 82.539
noteoff 0 85 0
sleep 1.587
noteoff 1 73 0
noteoff 1 81 0
noteoff 4 64 0
sleep 11.111
noteoff 3 57 0
sleep 82.731
noteon 0 83 101
sleep 1.779
noteon 1 80 100
noteon 1 71 100
noteon 4 64 94
sleep 12.455
noteon 3 52 100
sleep 92.526
noteoff 0 83 0
sleep 1.779
noteoff 1 71 0
noteoff 1 80 0
noteoff 4 64 0
sleep 12.455
noteoff 3 52 0
sleep 92.526
noteon 0 81 101
sleep 1.779
noteon 1 73 100
noteon 1 81 100
noteon 4 64 94
sleep 12.455
noteon 3 57 100
sleep 92.526
noteoff 0 81 0
sleep 1.779
noteoff 1 81 0
noteoff 1 73 0
noteoff 4 64 0
sleep 12.455
noteoff 3 57 0
sleep 90.747
noteon 10 81 102
sleep 1.587
noteon 0 85 101
noteon 0 88 101
sleep 1.587
noteon 4 64 94
noteon 11 73 102
noteon 11 64 102
sleep 1.587
noteon 2 73 101
noteon 2 69 101
noteon 6 76 102
sleep 4.761
noteon 5 64 94
sleep 4.761
noteon 3 45 100
noteon 3 57 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 112
sleep 1.587
noteon 14 45 106
sleep 76.19
noteoff 10 81 0
sleep 1.587
noteoff 0 88 0
noteoff 0 85 0
sleep 1.587
noteoff 4 64 0
noteoff 11 64 0
noteoff 11 73 0
sleep 1.587
noteoff 2 69 0
noteoff 2 73 0
noteoff 6 76 0
sleep 4.761
noteoff 5 64 0
sleep 4.761
noteoff 3 57 0
noteoff 3 45 0
sleep 1.587
noteoff 13 57 0
sleep 1.587
noteoff 15 45 0
sleep 1.587
noteoff 14 45 0
sleep 76.19
noteon 10 80 102
sleep 1.779
noteon 0 88 101
noteon 0 83 101
sleep 1.779
noteon 4 64 94
noteon 11 64 102
noteon 11 74 102
sleep 1.779
noteon 2 68 101
noteon 2 71 101
noteon 6 76 102
sleep 5.338
noteon 5 64 94
sleep 5.338
noteon 3 40 100
noteon 3 52 100
sleep 1.779
noteon 13 52 104
sleep 1.779
noteon 15 45 112
sleep 1.779
noteon 14 40 106
sleep 85.409
noteoff 10 80 0
sleep 1.779
noteoff 0 83 0
noteoff 0 88 0
sleep 1.779
noteoff 4 64 0
noteoff 11 74 0
noteoff 11 64 0
sleep 1.779
noteoff 2 71 0
noteoff 2 68 0
noteoff 6 76 0
sleep 5.338
noteoff 5 64 0
sleep 5.338
noteoff 3 52 0
noteoff 3 40 0
sleep 1.779
noteoff 13 52 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 40 0
sleep 85.409
noteon 10 81 102
sleep 2.066
noteon 0 85 101
noteon 0 88 101
sleep 2.066
noteon 4 64 94
noteon 11 73 102
noteon 11 64 102
sleep 2.066
noteon 2 73 101
noteon 2 69 101
noteon 6 76 102
sleep 6.198
noteon 5 64 94
sleep 6.198
noteon 3 45 100
noteon 3 57 100
sleep 2.066
noteon 13 57 104
sleep 2.066
noteon 15 45 112
sleep 2.066
noteon 14 45 106
sleep 99.173
noteoff 10 81 0
sleep 2.066
noteoff 0 88 0
noteoff 0 85 0
sleep 2.066
noteoff 4 64 0
noteoff 11 64 0
noteoff 11 73 0
sleep 2.066
noteoff 2 69 0
noteoff 2 73 0
noteoff 6 76 0
sleep 6.198
noteoff 5 64 0
sleep 6.198
noteoff 3 57 0
noteoff 3 45 0
sleep 2.066
noteoff 13 57 0
sleep 2.066
noteoff 15 45 0
sleep 2.066
noteoff 14 45 0
sleep 99.173
noteon 10 69 102
sleep 3.322
noteon 11 61 102
sleep 6.644
noteon 12 57 102
sleep 6.644
noteon 13 57 104
sleep 3.322
noteon 14 45 106
sleep 83.056
noteoff 11 61 0
sleep 6.644
noteoff 12 57 0
sleep 6.644
noteoff 13 57 0
sleep 3.322
noteoff 14 45 0
sleep 83.292
noteon 11 62 102
sleep 7.117
noteon 12 59 102
sleep 7.117
noteon 13 59 104
sleep 3.558
noteon 14 47 106
sleep 88.967
noteoff 11 62 0
sleep 7.117
noteoff 12 59 0
sleep 7.117
noteoff 13 59 0
sleep 3.558
noteoff 14 47 0
sleep 88.967
noteon 11 64 102
sleep 7.117
noteon 12 61 102
sleep 7.117
noteon 13 61 104
sleep 3.558
noteon 14 49 106
sleep 88.967
noteoff 11 64 0
sleep 7.117
noteoff 12 61 0
sleep 7.117
noteoff 13 61 0
sleep 3.558
noteoff 14 49 0
sleep 49.822
noteoff 10 69 0
sleep 35.587
noteon 10 77 102
sleep 3.322
noteon 11 65 102
sleep 6.644
noteon 12 62 102
sleep 6.644
noteon 13 62 104
sleep 3.322
noteon 14 50 106
sleep 64.784
noteoff 10 77 0
sleep 114.617
noteon 10 76 102
sleep 3.558
noteoff 11 65 0
sleep 7.117
noteoff 12 62 0
sleep 7.117
noteoff 13 62 0
sleep 3.558
noteoff 14 50 0
sleep 69.395
noteoff 10 76 0
sleep 122.775
noteon 10 77 102
sleep 90.747
noteoff 10 77 0
sleep 122.775
noteon 10 65 102
sleep 3.322
noteon 11 57 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 53 104
sleep 3.322
noteon 14 41 106
sleep 83.056
noteoff 11 57 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 53 0
sleep 3.322
noteoff 14 41 0
sleep 83.292
noteon 11 58 102
sleep 7.117
noteon 12 55 102
sleep 7.117
noteon 13 55 104
sleep 3.558
noteon 14 43 106
sleep 88.967
noteoff 11 58 0
sleep 7.117
noteoff 12 55 0
sleep 7.117
noteoff 13 55 0
sleep 3.558
noteoff 14 43 0
sleep 88.967
noteon 11 60 102
sleep 7.117
noteon 12 57 102
sleep 7.117
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 88.967
noteoff 11 60 0
sleep 7.117
noteoff 12 57 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 49.822
noteoff 10 65 0
sleep 35.587
noteon 10 74 102
sleep 3.322
noteon 11 62 102
sleep 6.644
noteon 12 58 102
sleep 6.644
noteon 13 58 104
sleep 3.322
noteon 14 46 106
sleep 64.784
noteoff 10 74 0
sleep 114.617
noteon 10 73 102
sleep 3.558
noteoff 11 62 0
sleep 7.117
noteoff 12 58 0
sleep 7.117
noteoff 13 58 0
sleep 3.558
noteoff 14 46 0
sleep 69.395
noteoff 10 73 0
sleep 122.775
noteon 10 74 102
sleep 90.747
noteoff 10 74 0
sleep 122.775
noteon 10 58 102
sleep 3.322
noteon 11 62 102
noteon 11 58 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 58 0
sleep 3.322
noteoff 11 58 0
noteoff 11 62 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 79.734
noteon 10 77 102
sleep 3.558
noteon 11 58 102
noteon 11 62 102
sleep 7.117
noteon 12 53 102
sleep 7.116
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 782.931
noteoff 10 77 0
sleep 35.587
noteon 10 74 102
sleep 106.761
noteoff 10 74 0
noteon 10 70 102
sleep 74.733
noteoff 11 62 0
noteoff 11 58 0
sleep 32.028
noteoff 10 70 0
noteon 10 69 102
sleep 3.322
noteon 11 60 102
noteon 11 63 102
sleep 96.345
noteoff 10 69 0
noteon 10 72 102
sleep 69.767
noteoff 11 63 0
noteoff 11 60 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 72 0
noteon 10 75 102
sleep 3.558
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 75 0
noteon 10 74 102
sleep 74.733
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 74 0
noteon 10 72 102
sleep 3.558
noteon 11 63 102
noteon 11 60 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 72 0
noteon 10 70 102
sleep 71.174
noteoff 10 70 0
sleep 3.558
noteoff 11 60 0
noteoff 11 63 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteon 10 69 102
sleep 3.322
noteon 11 60 102
noteon 11 63 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 69 0
noteon 10 67 102
sleep 69.767
noteoff 11 63 0
noteoff 11 60 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 67 0
noteon 10 65 102
sleep 3.558
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 65 0
noteon 10 63 102
sleep 74.733
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 63 0
noteon 10 62 102
sleep 1.779
noteon 0 81 101
sleep 1.779
noteon 11 63 102
noteon 11 60 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 62 0
noteon 10 60 102
sleep 71.174
noteoff 10 60 0
sleep 3.558
noteoff 11 60 0
noteoff 11 63 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteon 10 58 102
sleep 1.661
noteoff 0 81 0
noteon 0 82 101
sleep 1.661
noteon 1 70 100
noteon 11 62 102
noteon 11 58 102
noteon 1 74 100
sleep 6.644
noteon 12 53 102
sleep 4.983
noteoff 3 57 0
noteon 3 58 100
sleep 1.661
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 58 0
sleep 1.661
noteoff 0 82 0
sleep 1.661
noteoff 1 74 0
noteoff 1 70 0
noteoff 11 58 0
noteoff 11 62 0
sleep 6.644
noteoff 12 53 0
sleep 4.983
noteoff 3 58 0
sleep 1.661
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 79.734
noteon 10 77 102
sleep 1.779
noteon 0 89 101
sleep 1.779
noteon 1 74 100
noteon 1 70 100
noteon 11 58 102
noteon 11 62 102
sleep 7.117
noteon 12 53 102
sleep 5.337
noteon 3 65 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 782.921
noteoff 10 77 0
sleep 19.572
noteoff 0 89 0
sleep 14.234
noteoff 3 65 0
sleep 1.779
noteon 10 74 102
sleep 1.779
noteon 0 86 101
sleep 14.234
noteon 3 62 100
sleep 90.747
noteoff 10 74 0
noteon 10 70 102
sleep 1.779
noteoff 0 86 0
noteon 0 82 101
sleep 14.234
noteoff 3 62 0
noteon 3 58 100
sleep 55.16
noteoff 10 70 0
sleep 3.558
noteoff 11 62 0
noteoff 11 58 0
sleep 16.014
noteoff 0 82 0
sleep 1.779
noteoff 1 70 0
noteoff 1 74 0
sleep 12.455
noteoff 3 58 0
sleep 1.779
noteon 10 69 102
sleep 1.661
noteon 0 81 101
sleep 1.661
noteon 1 72 100
noteon 1 75 100
noteon 11 60 102
noteon 11 63 102
sleep 11.627
noteon 3 57 100
sleep 84.717
noteoff 10 69 0
noteon 10 72 102
sleep 1.661
noteoff 0 81 0
noteon 0 84 101
sleep 68.106
noteoff 11 63 0
noteoff 11 60 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 72 0
noteon 10 75 102
sleep 1.779
noteoff 0 84 0
noteon 0 87 101
sleep 1.779
noteon 11 63 102
noteon 11 60 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteoff 3 57 0
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 75 0
noteon 10 74 102
sleep 1.779
noteoff 0 87 0
noteon 0 86 101
sleep 72.953
noteoff 11 60 0
noteoff 11 63 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 74 0
noteon 10 72 102
sleep 1.779
noteoff 0 86 0
noteon 0 84 101
sleep 1.779
noteon 11 63 102
noteon 11 60 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 72 0
noteon 10 70 102
sleep 1.779
noteoff 0 84 0
noteon 0 82 101
sleep 72.953
noteoff 11 60 0
noteoff 11 63 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 70 0
noteon 10 69 102
sleep 1.661
noteoff 0 82 0
noteon 0 81 101
sleep 1.661
noteon 11 60 102
noteon 11 63 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 69 0
noteon 10 67 102
sleep 1.661
noteoff 0 81 0
noteon 0 79 101
sleep 68.106
noteoff 11 63 0
noteoff 11 60 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 67 0
noteon 10 65 102
sleep 1.779
noteoff 0 79 0
noteon 0 77 101
sleep 1.779
noteon 11 63 102
noteon 11 60 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteon 3 53 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 65 0
noteon 10 63 102
sleep 1.779
noteoff 0 77 0
noteon 0 75 101
sleep 72.953
noteoff 11 60 0
noteoff 11 63 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 12.455
noteoff 3 53 0
sleep 1.779
noteoff 10 63 0
noteon 10 62 102
sleep 1.779
noteoff 0 75 0
noteon 0 74 101
sleep 1.779
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteon 3 53 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 62 0
noteon 10 60 102
sleep 1.779
noteoff 0 74 0
noteon 0 72 101
sleep 69.395
noteoff 10 60 0
sleep 3.558
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 1.779
noteoff 0 72 0
sleep 1.779
noteoff 1 75 0
noteoff 1 72 0
noteoff 14 34 0
sleep 12.455
noteoff 3 53 0
sleep 1.779
noteon 10 58 102
sleep 1.661
noteon 0 70 101
sleep 1.661
noteon 1 70 100
noteon 1 74 100
noteon 11 62 102
noteon 11 58 102
sleep 6.644
noteon 12 53 102
sleep 4.983
noteon 3 58 100
sleep 1.661
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 58 0
sleep 3.322
noteoff 11 58 0
noteoff 11 62 0
sleep 6.644
noteoff 12 53 0
sleep 89.7
noteon 10 74 102
sleep 1.779
noteoff 0 70 0
sleep 1.779
noteoff 1 74 0
noteoff 1 70 0
noteon 11 62 102
sleep 7.117
noteon 12 58 102
sleep 5.338
noteoff 3 58 0
sleep 1.779
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 85.409
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 74 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 58 0
noteon 12 62 102
sleep 96.085
noteoff 10 74 0
noteon 10 76 102
sleep 71.174
noteoff 10 76 0
sleep 35.587
noteon 10 77 102
sleep 3.3
noteoff 11 65 0
sleep 6.6
noteoff 12 62 0
sleep 4.95
noteon 3 57 100
sleep 1.65
noteon 13 45 104
sleep 3.3
noteon 14 33 106
sleep 79.207
noteoff 10 77 0
sleep 99.009
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 7.117
noteon 12 57 102
sleep 5.338
noteoff 3 57 0
sleep 1.779
noteoff 13 45 0
sleep 3.558
noteoff 14 33 0
sleep 85.409
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 74 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 57 0
noteon 12 62 102
sleep 96.085
noteoff 10 74 0
noteon 10 76 102
sleep 71.174
noteoff 10 76 0
sleep 35.587
noteon 10 77 102
sleep 3.267
noteoff 11 65 0
sleep 6.535
noteoff 12 62 0
sleep 4.901
noteon 3 56 100
sleep 1.633
noteon 13 44 104
sleep 3.267
noteon 14 32 106
sleep 78.431
noteoff 10 77 0
sleep 98.039
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 7.117
noteon 12 59 102
sleep 5.338
noteoff 3 56 0
sleep 1.779
noteoff 13 44 0
sleep 3.558
noteoff 14 32 0
sleep 85.409
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 74 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 59 0
noteon 12 62 102
sleep 96.085
noteoff 10 74 0
noteon 10 76 102
sleep 71.174
noteoff 10 76 0
sleep 35.587
noteon 10 77 102
sleep 3.257
noteoff 11 65 0
noteon 11 65 102
sleep 6.514
noteoff 12 62 0
noteon 12 56 102
noteon 12 59 102
sleep 4.885
noteon 3 56 100
sleep 1.628
noteon 13 56 104
sleep 3.257
noteon 14 44 106
sleep 78.172
noteoff 10 77 0
sleep 3.257
noteoff 11 65 0
sleep 6.514
noteoff 12 59 0
noteoff 12 56 0
sleep 87.942
noteon 10 79 102
sleep 3.558
noteon 11 65 102
sleep 7.117
noteon 12 56 102
noteon 12 59 102
sleep 5.338
noteoff 3 56 0
sleep 1.779
noteoff 13 56 0
sleep 3.558
noteoff 14 44 0
sleep 85.406
noteoff 10 79 0
noteon 10 77 102
sleep 3.558
noteoff 11 65 0
sleep 7.117
noteoff 12 59 0
noteoff 12 56 0
sleep 96.081
noteoff 10 77 0
noteon 10 76 102
sleep 3.558
noteon 11 65 102
sleep 7.117
noteon 12 56 102
noteon 12 59 102
sleep 96.080
noteoff 10 76 0
noteon 10 74 102
sleep 3.558
noteoff 11 65 0
sleep 7.117
noteoff 12 59 0
noteoff 12 56 0
sleep 60.495
noteoff 10 74 0
sleep 35.587
noteon 10 73 102
sleep 3.236
noteon 11 64 102
sleep 6.472
noteon 12 61 102
noteon 12 57 102
sleep 4.854
noteon 3 57 100
sleep 1.618
noteon 13 57 104
sleep 3.236
noteon 14 33 106
sleep 77.669
noteoff 10 73 0
sleep 16.181
noteoff 13 57 0
sleep 80.906
noteon 10 74 102
sleep 3.558
noteoff 11 64 0
sleep 7.117
noteoff 12 57 0
noteoff 12 61 0
sleep 5.338
noteoff 3 57 0
sleep 90.747
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 71 102
sleep 10.676
noteon 12 49 102
sleep 7.117
noteon 13 49 104
sleep 88.967
noteoff 10 71 0
noteon 10 69 102
sleep 71.174
noteoff 10 69 0
sleep 35.587
noteon 10 77 102
sleep 9.677
noteoff 12 49 0
noteon 12 50 102
sleep 6.451
noteoff 13 49 0
noteon 13 50 104
sleep 80.641
noteoff 10 77 0
sleep 96.769
noteon 10 79 102
sleep 10.676
noteoff 12 50 0
noteon 12 53 102
sleep 7.117
noteoff 13 50 0
noteon 13 53 104
sleep 88.966
noteoff 10 79 0
noteon 10 77 102
sleep 106.758
noteoff 10 77 0
noteon 10 76 102
sleep 10.676
noteoff 12 53 0
noteon 12 56 102
sleep 7.117
noteoff 13 53 0
noteon 13 56 104
sleep 88.964
noteoff 10 76 0
noteon 10 74 102
sleep 71.172
noteoff 10 74 0
sleep 10.676
noteoff 12 56 0
sleep 7.117
noteoff 13 56 0
sleep 17.793
noteon 10 73 102
sleep 9.646
noteon 12 57 102
sleep 6.43
noteon 13 57 104
sleep 80.385
noteoff 10 73 0
sleep 9.646
noteoff 12 57 0
sleep 6.43
noteoff 13 57 0
sleep 80.385
noteon 10 74 102
sleep 106.761
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 71 102
sleep 3.558
noteon 11 61 102
sleep 7.117
noteon 12 49 102
sleep 7.117
noteon 13 49 104
sleep 88.967
noteoff 10 71 0
noteon 10 69 102
sleep 71.174
noteoff 10 69 0
sleep 35.587
noteon 10 77 102
sleep 3.205
noteoff 11 61 0
noteon 11 62 102
sleep 6.41
noteoff 12 49 0
noteon 12 50 102
sleep 6.41
noteoff 13 49 0
noteon 13 50 104
sleep 80.128
noteoff 10 77 0
sleep 96.153
noteon 10 79 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 50 0
noteon 12 53 102
sleep 7.117
noteoff 13 50 0
noteon 13 53 104
sleep 88.959
noteoff 10 79 0
noteon 10 77 102
sleep 106.751
noteoff 10 77 0
noteon 10 76 102
sleep 3.558
noteoff 11 65 0
noteon 11 68 102
sleep 7.116
noteoff 12 53 0
noteon 12 56 102
sleep 7.116
noteoff 13 53 0
noteon 13 56 104
sleep 88.959
noteoff 10 76 0
noteon 10 74 102
sleep 71.167
noteoff 10 74 0
sleep 3.558
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 7.117
noteoff 13 56 0
sleep 17.792
noteon 10 73 102
sleep 3.194
noteon 11 69 102
sleep 6.389
noteon 12 57 102
sleep 6.389
noteon 13 57 104
sleep 79.859
noteoff 10 73 0
sleep 3.194
noteoff 11 69 0
sleep 6.388
noteoff 12 57 0
sleep 6.389
noteoff 13 57 0
sleep 79.859
noteon 10 76 102
sleep 3.558
noteon 11 57 102
sleep 17.791
noteoff 14 33 0
sleep 85.400
noteoff 10 76 0
noteon 10 74 102
sleep 106.751
noteoff 10 74 0
noteon 10 73 102
sleep 3.558
noteoff 11 57 0
noteon 11 58 102
sleep 103.193
noteoff 10 73 0
noteon 10 71 102
sleep 71.167
noteoff 10 71 0
sleep 35.583
noteon 10 69 102
sleep 3.174
noteoff 11 58 0
noteon 11 59 102
sleep 92.056
noteoff 10 69 0
noteon 10 68 102
sleep 63.487
noteoff 10 68 0
sleep 31.744
noteon 10 69 102
sleep 3.558
noteoff 11 59 0
noteon 11 60 102
sleep 49.817
noteoff 10 69 0
sleep 53.375
noteon 10 67 102
sleep 53.376
noteoff 10 67 0
sleep 53.375
noteon 10 66 102
sleep 3.558
noteoff 11 60 0
noteon 11 61 102
sleep 49.817
noteoff 10 66 0
sleep 53.375
noteon 10 64 102
sleep 71.420
noteoff 10 64 0
sleep 28.568
noteoff 11 61 0
sleep 42.852
noteon 10 62 102
sleep 1.587
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 62 101
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 46.969
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 15.097
noteoff 0 74 0
sleep 1.677
noteoff 1 74 0
sleep 1.677
noteoff 2 74 0
noteoff 2 62 0
sleep 11.741
noteon 10 78 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 64 102
noteon 10 55 102
sleep 1.587
noteon 0 74 101
noteon 0 76 101
sleep 1.587
noteon 1 76 100
noteon 1 74 100
noteon 4 62 100
noteon 11 59 102
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 2 64 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 5 50 100
noteon 12 67 102
sleep 4.761
noteon 3 55 100
sleep 1.587
noteon 13 55 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 43 106
sleep 85.706
noteoff 12 67 0
sleep 4.761
noteoff 3 55 0
sleep 1.587
noteoff 13 55 0
sleep 3.174
noteoff 14 43 0
sleep 86.248
noteon 12 69 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 69 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 71 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 71 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 46.969
noteoff 10 55 0
noteoff 10 64 0
sleep 3.354
noteoff 11 62 0
noteoff 11 59 0
sleep 15.097
noteoff 0 76 0
noteoff 0 74 0
sleep 1.677
noteoff 1 74 0
noteoff 1 76 0
sleep 1.677
noteoff 2 64 0
noteoff 2 74 0
sleep 11.741
noteon 10 79 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 79 0
sleep 109.523
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 122.775
noteon 10 83 102
sleep 90.747
noteoff 10 83 0
sleep 125.949
noteon 4 64 100
sleep 6.349
noteon 5 57 100
sleep 88.888
noteoff 4 64 0
sleep 6.349
noteoff 5 57 0
sleep 89.272
noteon 4 66 100
sleep 7.117
noteon 5 62 100
sleep 99.644
noteoff 4 66 0
sleep 7.117
noteoff 5 62 0
sleep 99.644
noteon 4 67 100
sleep 7.117
noteon 5 64 100
sleep 99.644
noteoff 4 67 0
sleep 7.117
noteoff 5 64 0
sleep 96.085
noteon 10 76 102
sleep 3.174
noteon 11 69 102
sleep 77.777
noteoff 10 76 0
sleep 3.174
noteoff 11 69 0
sleep 106.349
noteon 10 78 102
sleep 3.558
noteon 11 74 102
sleep 87.188
noteoff 10 78 0
sleep 3.558
noteoff 11 74 0
sleep 119.217
noteon 10 79 102
sleep 3.558
noteon 11 76 102
sleep 87.188
noteoff 10 79 0
sleep 3.558
noteoff 11 76 0
sleep 122.391
noteon 1 78 100
noteon 4 66 100
noteon 1 74 100
sleep 6.349
noteon 5 62 100
sleep 88.888
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 66 0
sleep 6.349
noteoff 5 62 0
sleep 89.272
noteon 1 76 100
noteon 1 69 100
noteon 4 64 100
sleep 7.117
noteon 5 57 100
sleep 99.644
noteoff 1 69 0
noteoff 1 76 0
noteoff 4 64 0
sleep 7.117
noteoff 5 57 0
sleep 99.644
noteon 1 66 100
noteon 1 74 100
noteon 4 62 100
sleep 7.117
noteon 5 54 100
sleep 99.644
noteoff 1 74 0
noteoff 1 66 0
noteoff 4 62 0
sleep 7.117
noteoff 5 54 0
sleep 96.085
noteon 10 86 102
sleep 1.587
noteon 0 78 101
noteon 0 86 101
sleep 1.587
noteon 1 74 100
noteon 1 78 100
noteon 11 69 102
noteon 11 78 102
sleep 1.587
noteon 2 74 101
noteon 2 66 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 12 62 102
sleep 4.761
noteon 3 50 100
noteon 3 62 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 92
sleep 1.587
noteon 14 38 106
sleep 76.19
noteoff 10 86 0
sleep 1.587
noteoff 0 86 0
noteoff 0 78 0
sleep 1.587
noteoff 1 78 0
noteoff 1 74 0
noteoff 11 78 0
noteoff 11 69 0
sleep 1.587
noteoff 2 66 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 12 62 0
sleep 4.761
noteoff 3 62 0
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 1.587
noteoff 15 50 0
sleep 1.587
noteoff 14 38 0
sleep 76.19
noteon 10 85 102
sleep 1.779
noteon 0 85 101
noteon 0 76 101
sleep 1.779
noteon 1 76 100
noteon 1 79 100
noteon 11 79 102
noteon 11 69 102
sleep 1.779
noteon 2 76 101
noteon 2 67 101
noteon 6 69 108
noteon 6 57 108
sleep 5.338
noteon 12 57 102
sleep 5.338
noteon 3 45 100
noteon 3 57 100
sleep 1.779
noteon 13 45 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteon 14 33 106
sleep 85.409
noteoff 10 85 0
sleep 1.779
noteoff 0 76 0
noteoff 0 85 0
sleep 1.779
noteoff 1 79 0
noteoff 1 76 0
noteoff 11 69 0
noteoff 11 79 0
sleep 1.779
noteoff 2 67 0
noteoff 2 76 0
noteoff 6 57 0
noteoff 6 69 0
sleep 5.338
noteoff 12 57 0
sleep 5.338
noteoff 3 57 0
noteoff 3 45 0
sleep 1.779
noteoff 13 45 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 33 0
sleep 85.409
noteon 10 86 102
sleep 1.779
noteon 0 78 101
noteon 0 86 101
sleep 1.779
noteon 1 78 100
noteon 1 74 100
noteon 11 78 102
noteon 11 69 102
sleep 1.779
noteon 2 66 101
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
sleep 5.338
noteon 12 62 102
sleep 5.338
noteon 3 50 100
noteon 3 62 100
sleep 1.779
noteon 13 50 104
sleep 1.779
noteon 15 50 92
sleep 1.779
noteon 14 38 106
sleep 85.409
noteoff 10 86 0
sleep 1.779
noteoff 0 86 0
noteoff 0 78 0
sleep 1.779
noteoff 1 74 0
noteoff 1 78 0
noteoff 11 69 0
noteoff 11 78 0
sleep 1.779
noteoff 2 74 0
noteoff 2 66 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 12 62 0
sleep 5.338
noteoff 3 62 0
noteoff 3 50 0
sleep 1.779
noteoff 13 50 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 38 0
sleep 85.409
noteon 10 62 102
sleep 1.587
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 62 101
noteon 2 74 101
noteon 6 62 108
noteon 6 74 108
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 46.969
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 15.097
noteoff 0 74 0
sleep 1.677
noteoff 1 74 0
sleep 1.677
noteoff 2 74 0
noteoff 2 62 0
sleep 11.741
noteon 10 78 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 55 102
noteon 10 64 102
sleep 1.587
noteon 0 74 101
noteon 0 76 101
sleep 1.587
noteon 1 74 100
noteon 1 76 100
noteon 4 62 100
noteon 11 59 102
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 2 64 101
noteon 6 62 108
noteon 6 74 108
sleep 4.761
noteon 5 50 100
noteon 12 55 102
sleep 4.761
noteon 3 55 100
sleep 1.587
noteon 13 55 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 43 106
sleep 85.706
noteoff 12 55 0
sleep 4.761
noteoff 3 55 0
sleep 1.587
noteoff 13 55 0
sleep 3.174
noteoff 14 43 0
sleep 86.248
noteon 12 57 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 57 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 59 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 59 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 46.969
noteoff 10 64 0
noteoff 10 55 0
sleep 3.354
noteoff 11 62 0
noteoff 11 59 0
sleep 15.097
noteoff 0 76 0
noteoff 0 74 0
sleep 1.677
noteoff 1 76 0
noteoff 1 74 0
sleep 1.677
noteoff 2 64 0
noteoff 2 74 0
sleep 11.741
noteon 10 79 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 79 0
sleep 109.523
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 122.775
noteon 10 83 102
sleep 90.747
noteoff 10 83 0
sleep 137.576
noteon 10 67 102
sleep 83.061
noteoff 10 67 0
sleep 112.377
noteon 10 69 102
sleep 86.148
noteoff 10 69 0
sleep 116.554
noteon 10 71 102
sleep 86.148
noteoff 10 71 0
sleep 119.811
noteon 11 64 102
sleep 83.061
noteoff 11 64 0
sleep 112.498
noteon 11 66 102
sleep 86.148
noteoff 11 66 0
sleep 116.554
noteon 11 67 102
sleep 86.148
noteoff 11 67 0
sleep 113.175
noteon 10 67 102
sleep 83.061
noteoff 10 67 0
sleep 112.377
noteon 10 69 102
sleep 86.148
noteoff 10 69 0
sleep 116.554
noteon 10 70 102
sleep 86.148
noteoff 10 70 0
sleep 119.811
noteon 11 64 102
sleep 83.061
noteoff 11 64 0
sleep 112.498
noteon 11 66 102
sleep 86.148
noteoff 11 66 0
sleep 116.554
noteon 11 67 102
sleep 86.148
noteoff 11 67 0
sleep 113.175
noteon 10 67 102
sleep 83.061
noteoff 10 67 0
sleep 112.376
noteon 10 69 102
sleep 86.148
noteoff 10 69 0
sleep 116.552
noteon 10 70 102
sleep 86.148
noteoff 10 70 0
sleep 116.554
noteon 10 67 102
sleep 3.257
noteon 11 64 102
sleep 79.803
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.12
noteon 10 69 102
sleep 3.424
noteon 11 65 102
sleep 83.904
noteoff 10 69 0
sleep 3.424
noteoff 11 65 0
sleep 114.725
noteon 10 70 102
sleep 3.424
noteon 11 67 102
sleep 83.904
noteoff 10 70 0
sleep 3.424
noteoff 11 67 0
sleep 114.725
noteon 10 67 102
sleep 3.257
noteon 11 64 102
sleep 79.803
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.12
noteon 10 69 102
sleep 3.472
noteon 11 65 102
sleep 85.068
noteoff 10 69 0
sleep 3.472
noteoff 11 65 0
sleep 116.319
noteon 10 70 102
sleep 3.472
noteon 11 67 102
sleep 85.069
noteoff 10 70 0
sleep 3.472
noteoff 11 67 0
sleep 116.318
noteon 10 67 102
sleep 3.257
noteon 11 64 102
sleep 79.804
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.119
noteon 10 69 102
sleep 3.558
noteon 11 65 102
sleep 87.187
noteoff 10 69 0
sleep 3.558
noteoff 11 65 0
sleep 119.217
noteon 10 70 102
sleep 3.558
noteon 11 67 102
sleep 87.186
noteoff 10 70 0
sleep 3.558
noteoff 11 67 0
sleep 119.217
noteon 10 67 102
sleep 3.257
noteon 1 72 100
noteon 11 64 102
sleep 6.514
noteon 12 60 102
sleep 4.885
noteon 3 60 100
sleep 1.628
noteon 13 60 104
sleep 3.257
noteon 14 48 106
sleep 63.517
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.12
noteon 10 69 102
sleep 3.558
noteon 11 65 102
sleep 7.117
noteoff 12 60 0
sleep 7.117
noteoff 13 60 0
sleep 3.558
noteoff 14 48 0
sleep 69.395
noteoff 10 69 0
sleep 3.558
noteoff 11 65 0
sleep 119.217
noteon 10 70 102
sleep 3.558
noteon 11 67 102
sleep 87.188
noteoff 10 70 0
sleep 3.558
noteoff 11 67 0
sleep 96.085
noteoff 1 72 0
sleep 12.455
noteoff 3 60 0
sleep 10.676
noteon 10 69 102
sleep 3.257
noteon 1 77 100
noteon 11 65 102
sleep 6.514
noteon 12 53 102
sleep 4.885
noteon 3 65 100
sleep 1.628
noteon 13 53 104
sleep 3.257
noteon 14 41 106
sleep 81.433
noteoff 1 77 0
sleep 11.4
noteoff 3 65 0
sleep 83.061
noteoff 10 69 0
sleep 3.558
noteoff 11 65 0
noteon 1 76 100
sleep 7.117
noteoff 12 53 0
sleep 5.338
noteon 3 64 100
sleep 1.779
noteoff 13 53 0
sleep 3.558
noteoff 14 41 0
sleep 88.967
noteoff 1 76 0
sleep 12.455
noteoff 3 64 0
sleep 94.306
noteon 1 77 100
sleep 12.455
noteon 3 65 100
sleep 94.306
noteoff 1 77 0
sleep 12.455
noteoff 3 65 0
sleep 90.747
noteon 10 64 102
sleep 3.257
noteon 1 69 100
noteon 11 61 102
sleep 6.514
noteon 12 57 102
sleep 4.885
noteon 3 57 100
sleep 1.628
noteon 13 57 104
sleep 3.257
noteon 14 45 106
sleep 63.517
noteoff 10 64 0
sleep 3.257
noteoff 11 61 0
sleep 109.12
noteon 10 65 102
sleep 3.558
noteon 11 62 102
sleep 7.117
noteoff 12 57 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 69.395
noteoff 10 65 0
sleep 3.558
noteoff 11 62 0
sleep 119.217
noteon 10 67 102
sleep 3.558
noteon 11 64 102
sleep 87.188
noteoff 10 67 0
sleep 3.558
noteoff 11 64 0
sleep 96.085
noteoff 1 69 0
sleep 12.455
noteoff 3 57 0
sleep 10.676
noteon 10 65 102
sleep 3.257
noteon 1 74 100
noteon 11 62 102
sleep 6.514
noteon 12 50 102
sleep 4.885
noteon 3 62 100
sleep 1.628
noteon 13 50 104
sleep 3.257
noteon 14 38 106
sleep 81.433
noteoff 1 74 0
sleep 11.4
noteoff 3 62 0
sleep 83.061
noteoff 10 65 0
sleep 3.558
noteoff 11 62 0
noteon 1 73 100
sleep 7.117
noteoff 12 50 0
sleep 5.338
noteon 3 61 100
sleep 1.779
noteoff 13 50 0
sleep 3.558
noteoff 14 38 0
sleep 88.967
noteoff 1 73 0
sleep 12.455
noteoff 3 61 0
sleep 94.306
noteon 1 74 100
sleep 12.455
noteon 3 62 100
sleep 94.948
noteoff 1 74 0
sleep 14.705
noteoff 3 62 0
sleep 110.306
noteon 4 62 100
noteon 11 62 102
sleep 1.582
noteon 2 62 101
noteon 2 74 101
sleep 4.746
noteon 5 50 100
noteon 12 50 102
sleep 4.746
noteon 3 50 100
noteon 3 46 100
sleep 1.582
noteon 13 50 104
sleep 3.164
noteon 14 34 106
sleep 174.430
noteoff 11 62 0
noteon 11 64 102
sleep 7.117
noteoff 12 50 0
noteon 12 52 102
sleep 5.337
noteoff 3 50 0
noteon 3 52 100
sleep 1.779
noteoff 13 50 0
noteon 13 52 104
sleep 199.274
noteoff 11 64 0
noteon 11 65 102
sleep 7.117
noteoff 12 52 0
noteon 12 53 102
sleep 5.337
noteoff 3 52 0
noteon 3 53 100
sleep 1.779
noteoff 13 52 0
noteon 13 53 104
sleep 198.889
noteoff 11 65 0
noteon 11 66 102
sleep 6.349
noteoff 12 53 0
noteon 12 54 102
sleep 4.761
noteoff 3 53 0
noteon 3 54 100
sleep 1.587
noteoff 13 53 0
noteon 13 54 104
sleep 178.144
noteoff 11 66 0
noteon 11 67 102
sleep 7.117
noteoff 12 54 0
noteon 12 55 102
sleep 5.337
noteoff 3 54 0
noteon 3 55 100
sleep 1.779
noteoff 13 54 0
noteon 13 55 104
sleep 199.274
noteoff 11 67 0
noteon 11 68 102
sleep 7.116
noteoff 12 55 0
noteon 12 56 102
sleep 5.337
noteoff 3 55 0
noteon 3 56 100
sleep 1.779
noteoff 13 55 0
noteon 13 56 104
sleep 163.687
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 3.558
noteoff 2 74 0
noteoff 2 62 0
sleep 3.558
noteoff 13 56 0
sleep 3.558
noteoff 14 34 0
sleep 3.558
noteoff 3 56 0
noteoff 3 46 0
sleep 10.674
noteon 10 69 102
noteon 10 81 102
sleep 3.174
noteoff 4 62 0
noteon 4 57 100
noteon 11 69 102
sleep 1.587
noteon 2 73 101
noteon 2 61 101
noteon 6 69 108
noteon 6 57 108
sleep 4.761
noteoff 5 50 0
noteon 12 57 102
noteon 5 45 100
sleep 4.761
noteon 3 57 100
noteon 3 45 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.206
noteoff 12 57 0
sleep 6.349
noteoff 13 57 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 45 0
noteoff 3 57 0
sleep 9.523
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.259
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 12 69 0
sleep 3.558
noteoff 2 61 0
noteoff 2 73 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 57 0
noteoff 6 69 0
sleep 1.779
noteoff 3 57 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 3.174
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 92.063
noteon 10 76 102
sleep 3.558
noteoff 4 62 0
noteon 11 64 102
sleep 1.779
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 5 50 0
noteoff 12 62 0
sleep 5.338
noteoff 3 50 0
sleep 1.779
noteoff 13 50 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 38 0
sleep 85.409
noteoff 10 76 0
sleep 3.558
noteoff 11 64 0
sleep 103.202
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 103.202
noteoff 10 74 0
sleep 4.329
noteoff 11 62 0
sleep 125.541
noteon 10 72 102
sleep 3.257
noteon 1 72 100
noteon 11 67 102
sleep 6.514
noteon 12 64 102
sleep 4.885
noteon 3 60 100
sleep 1.628
noteon 13 60 104
sleep 3.257
noteon 14 48 106
sleep 81.433
noteoff 11 67 0
sleep 6.514
noteoff 12 64 0
sleep 91.505
noteon 11 69 102
sleep 7.117
noteon 12 65 102
sleep 7.117
noteoff 13 60 0
sleep 3.558
noteoff 14 48 0
sleep 88.967
noteoff 11 69 0
sleep 7.117
noteoff 12 65 0
sleep 99.644
noteon 11 70 102
sleep 7.117
noteon 12 67 102
sleep 99.644
noteoff 11 70 0
sleep 7.117
noteoff 12 67 0
sleep 60.498
noteoff 10 72 0
sleep 12.455
noteoff 1 72 0
sleep 12.455
noteoff 3 60 0
sleep 10.676
noteon 10 77 102
sleep 3.257
noteon 1 77 100
noteon 11 69 102
sleep 6.514
noteon 12 65 102
sleep 4.885
noteon 3 65 100
sleep 1.628
noteon 13 53 104
sleep 3.257
noteon 14 41 106
sleep 78.175
noteoff 10 77 0
sleep 3.257
noteoff 1 77 0
sleep 11.4
noteoff 3 65 0
sleep 83.061
noteon 10 76 102
sleep 3.558
noteoff 11 69 0
noteon 1 76 100
sleep 7.117
noteoff 12 65 0
sleep 5.338
noteon 3 64 100
sleep 1.779
noteoff 13 53 0
sleep 3.558
noteoff 14 41 0
sleep 85.409
noteoff 10 76 0
sleep 3.558
noteoff 1 76 0
sleep 12.455
noteoff 3 64 0
sleep 90.747
noteon 10 77 102
sleep 3.558
noteon 1 77 100
sleep 12.455
noteon 3 65 100
sleep 90.747
noteoff 10 77 0
sleep 3.558
noteoff 1 77 0
sleep 12.455
noteoff 3 65 0
sleep 90.747
noteon 10 69 102
sleep 1.628
noteon 0 81 101
sleep 1.628
noteon 1 69 100
noteon 11 64 102
sleep 6.514
noteon 12 61 102
sleep 4.885
noteon 3 57 100
sleep 1.628
noteon 13 57 104
sleep 3.257
noteon 14 45 106
sleep 81.433
noteoff 11 64 0
sleep 6.514
noteoff 12 61 0
sleep 91.505
noteon 11 65 102
sleep 7.117
noteon 12 62 102
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 88.967
noteoff 11 65 0
sleep 7.117
noteoff 12 62 0
sleep 99.644
noteon 11 67 102
sleep 7.117
noteon 12 64 102
sleep 99.644
noteoff 11 67 0
sleep 7.117
noteoff 12 64 0
sleep 60.498
noteoff 10 69 0
sleep 10.676
noteoff 0 81 0
sleep 1.779
noteoff 1 69 0
sleep 12.455
noteoff 3 57 0
sleep 10.676
noteon 10 74 102
sleep 1.628
noteon 0 86 101
sleep 1.628
noteon 1 74 100
noteon 11 65 102
sleep 6.514
noteon 12 62 102
sleep 4.885
noteon 3 62 100
sleep 1.628
noteon 13 50 104
sleep 3.257
noteon 14 38 106
sleep 78.175
noteoff 10 74 0
sleep 1.628
noteoff 0 86 0
sleep 1.628
noteoff 1 74 0
sleep 11.4
noteoff 3 62 0
sleep 83.061
noteon 10 73 102
sleep 1.779
noteon 0 85 101
sleep 1.779
noteoff 11 65 0
noteon 1 73 100
sleep 7.117
noteoff 12 62 0
sleep 5.338
noteon 3 61 100
sleep 1.779
noteoff 13 50 0
sleep 3.558
noteoff 14 38 0
sleep 85.409
noteoff 10 73 0
sleep 1.779
noteoff 0 85 0
sleep 1.779
noteoff 1 73 0
sleep 12.455
noteoff 3 61 0
sleep 90.747
noteon 10 74 102
sleep 1.779
noteon 0 86 101
sleep 1.779
noteon 1 74 100
sleep 12.455
noteon 3 62 100
sleep 90.747
noteoff 10 74 0
sleep 2.164
noteoff 0 86 0
sleep 2.164
noteoff 1 74 0
sleep 15.151
noteoff 3 62 0
sleep 113.563
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
sleep 4.761
noteon 5 50 100
noteon 12 50 102
sleep 4.761
noteon 3 50 100
noteon 3 46 100
sleep 1.587
noteon 13 50 104
sleep 3.174
noteon 14 34 106
sleep 174.970
noteoff 11 62 0
noteon 11 64 102
sleep 7.117
noteoff 12 50 0
noteon 12 52 102
sleep 5.337
noteoff 3 50 0
noteon 3 52 100
sleep 1.779
noteoff 13 50 0
noteon 13 52 104
sleep 199.274
noteoff 11 64 0
noteon 11 65 102
sleep 7.117
noteoff 12 52 0
noteon 12 53 102
sleep 5.337
noteoff 3 52 0
noteon 3 53 100
sleep 1.779
noteoff 13 52 0
noteon 13 53 104
sleep 198.889
noteoff 11 65 0
noteon 11 66 102
sleep 6.349
noteoff 12 53 0
noteon 12 54 102
sleep 4.761
noteoff 3 53 0
noteon 3 54 100
sleep 1.587
noteoff 13 53 0
noteon 13 54 104
sleep 178.144
noteoff 11 66 0
noteon 11 67 102
sleep 7.117
noteoff 12 54 0
noteon 12 55 102
sleep 5.337
noteoff 3 54 0
noteon 3 55 100
sleep 1.779
noteoff 13 54 0
noteon 13 55 104
sleep 199.274
noteoff 11 67 0
noteon 11 68 102
sleep 7.116
noteoff 12 55 0
noteon 12 56 102
sleep 5.337
noteoff 3 55 0
noteon 3 56 100
sleep 1.779
noteoff 13 55 0
noteon 13 56 104
sleep 163.687
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 3.558
noteoff 2 62 0
noteoff 2 74 0
sleep 3.558
noteoff 13 56 0
sleep 3.558
noteoff 14 34 0
sleep 3.558
noteoff 3 56 0
noteoff 3 46 0
sleep 10.674
noteon 10 81 102
noteon 10 69 102
sleep 3.174
noteoff 4 62 0
noteon 4 57 100
noteon 11 69 102
sleep 1.587
noteon 2 73 101
noteon 2 61 101
noteon 6 57 108
noteon 6 69 108
sleep 4.761
noteoff 5 50 0
noteon 12 57 102
noteon 5 45 100
sleep 4.761
noteon 3 57 100
noteon 3 45 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.206
noteoff 12 57 0
sleep 6.349
noteoff 13 57 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 45 0
noteoff 3 57 0
sleep 9.523
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.259
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 12 69 0
sleep 3.558
noteoff 2 61 0
noteoff 2 73 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 69 0
noteoff 6 57 0
sleep 1.779
noteoff 3 57 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 3.174
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
noteon 6 62 108
noteon 6 74 108
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 62 100
sleep 1.587
noteon 13 62 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 50 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 92.063
noteon 10 76 102
sleep 3.558
noteoff 4 62 0
noteon 11 64 102
sleep 1.779
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 74 0
noteoff 6 62 0
sleep 5.338
noteoff 5 50 0
noteoff 12 62 0
sleep 5.338
noteoff 3 62 0
sleep 1.779
noteoff 13 62 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 50 0
sleep 85.409
noteoff 10 76 0
sleep 3.558
noteoff 11 64 0
sleep 103.202
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 103.202
noteoff 10 74 0
sleep 3.558
noteoff 11 62 0
sleep 106.376
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 62 101
noteon 2 74 101
sleep 4.761
noteon 5 50 100
noteon 12 50 102
sleep 4.761
noteon 3 50 100
noteon 3 46 100
sleep 1.587
noteon 13 50 104
sleep 3.174
noteon 14 34 106
sleep 174.972
noteoff 11 62 0
noteon 11 64 102
sleep 7.117
noteoff 12 50 0
noteon 12 52 102
sleep 5.337
noteoff 3 50 0
noteon 3 52 100
sleep 1.779
noteoff 13 50 0
noteon 13 52 104
sleep 199.271
noteoff 11 64 0
noteon 11 65 102
sleep 7.116
noteoff 12 52 0
noteon 12 53 102
sleep 5.338
noteoff 3 52 0
noteon 3 53 100
sleep 1.779
noteoff 13 52 0
noteon 13 53 104
sleep 198.887
noteoff 11 65 0
noteon 11 66 102
sleep 6.349
noteoff 12 53 0
noteon 12 54 102
sleep 4.761
noteoff 3 53 0
noteon 3 54 100
sleep 1.587
noteoff 13 53 0
noteon 13 54 104
sleep 178.144
noteoff 11 66 0
noteon 11 67 102
sleep 7.117
noteoff 12 54 0
noteon 12 55 102
sleep 5.337
noteoff 3 54 0
noteon 3 55 100
sleep 1.779
noteoff 13 54 0
noteon 13 55 104
sleep 199.272
noteoff 11 67 0
noteon 11 68 102
sleep 7.116
noteoff 12 55 0
noteon 12 56 102
sleep 5.338
noteoff 3 55 0
noteon 3 56 100
sleep 1.779
noteoff 13 55 0
noteon 13 56 104
sleep 163.688
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 3.558
noteoff 2 74 0
noteoff 2 62 0
sleep 3.558
noteoff 13 56 0
sleep 3.558
noteoff 14 34 0
sleep 3.558
noteoff 3 56 0
noteoff 3 46 0
sleep 10.676
noteon 10 69 102
noteon 10 81 102
sleep 3.174
noteoff 4 62 0
noteon 4 57 100
noteon 11 69 102
sleep 1.587
noteon 2 73 101
noteon 2 61 101
noteon 6 69 108
noteon 6 57 108
sleep 4.761
noteoff 5 50 0
noteon 5 45 100
noteon 12 57 102
sleep 4.761
noteon 3 57 100
noteon 3 45 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.187
noteoff 12 57 0
sleep 6.348
noteoff 13 57 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 45 0
noteoff 3 57 0
sleep 9.522
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.337
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.251
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 12 69 0
sleep 3.558
noteoff 2 61 0
noteoff 2 73 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 57 0
noteoff 6 69 0
sleep 1.779
noteoff 3 57 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 3.174
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 5 50 100
noteon 12 74 102
sleep 4.761
noteon 3 62 100
sleep 1.587
noteon 13 62 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 50 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 79.365
noteoff 4 62 0
sleep 6.349
noteoff 5 50 0
sleep 6.349
noteon 10 74 102
sleep 3.558
noteon 4 66 100
noteon 11 62 102
sleep 1.779
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 12 74 0
noteon 5 54 100
sleep 5.338
noteoff 3 62 0
sleep 1.779
noteoff 13 62 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 50 0
sleep 85.409
noteoff 10 74 0
sleep 3.558
noteoff 11 62 0
sleep 88.967
noteoff 4 66 0
sleep 7.117
noteoff 5 54 0
sleep 7.117
noteon 10 78 102
sleep 3.558
noteon 4 62 100
noteon 11 66 102
sleep 7.117
noteon 5 50 100
sleep 96.085
noteoff 10 78 0
sleep 3.558
noteoff 11 66 0
sleep 88.967
noteoff 4 62 0
sleep 7.117
noteoff 5 50 0
sleep 7.117
noteon 10 81 102
noteon 10 69 102
sleep 1.587
noteon 0 85 101
sleep 1.587
noteon 4 57 100
noteon 11 69 102
sleep 1.587
noteon 2 73 101
noteon 6 69 108
noteon 6 57 108
sleep 4.761
noteon 5 45 100
noteon 12 57 102
sleep 4.761
noteon 3 45 100
noteon 3 61 100
sleep 1.587
noteon 13 45 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.186
noteoff 12 57 0
sleep 6.348
noteoff 13 45 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 45 0
sleep 9.522
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.337
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.251
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 0 85 0
noteoff 12 69 0
sleep 3.558
noteoff 2 73 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 57 0
noteoff 6 69 0
sleep 1.779
noteoff 3 57 0
noteoff 3 61 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 1.587
noteon 0 86 101
sleep 1.587
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 5 50 100
noteon 12 74 102
sleep 4.761
noteon 3 50 100
noteon 3 62 100
sleep 1.587
noteon 13 62 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 50 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 79.365
noteoff 4 62 0
sleep 6.349
noteoff 5 50 0
sleep 6.349
noteon 10 74 102
sleep 1.779
noteoff 0 86 0
sleep 1.779
noteon 4 66 100
noteon 11 62 102
sleep 1.779
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 12 74 0
noteon 5 54 100
sleep 5.338
noteoff 3 62 0
noteoff 3 50 0
sleep 1.779
noteoff 13 62 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 50 0
sleep 85.409
noteoff 10 74 0
sleep 3.558
noteoff 11 62 0
sleep 88.967
noteoff 4 66 0
sleep 7.117
noteoff 5 54 0
sleep 7.117
noteon 10 78 102
sleep 3.558
noteon 4 62 100
noteon 11 66 102
sleep 7.117
noteon 5 50 100
sleep 96.085
noteoff 10 78 0
sleep 3.558
noteoff 11 66 0
sleep 88.967
noteoff 4 62 0
sleep 7.117
noteoff 5 50 0
sleep 7.117
noteon 10 81 102
noteon 10 69 102
sleep 1.587
noteon 0 81 101
noteon 0 85 101
sleep 1.587
noteon 1 69 100
noteon 4 57 100
noteon 11 61 102
noteon 1 81 100
sleep 1.587
noteon 2 69 101
noteon 2 73 101
noteon 6 57 108
noteon 6 69 108
sleep 4.761
noteon 5 45 100
noteon 12 57 102
sleep 4.761
noteon 3 61 100
noteon 3 57 100
sleep 1.587
noteon 13 45 104
sleep 1.587
noteon 15 45 92
sleep 1.587
noteon 14 33 106
sleep 84.122
noteoff 15 45 0
sleep 9.523
noteon 15 45 92
sleep 81.330
noteoff 11 61 0
noteon 11 64 102
sleep 5.338
noteoff 15 45 0
sleep 1.779
noteoff 12 57 0
noteon 12 61 102
sleep 7.117
noteoff 13 45 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 33 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 90.747
noteoff 11 64 0
noteon 11 69 102
sleep 5.338
noteoff 15 45 0
sleep 1.779
noteoff 12 61 0
noteon 12 64 102
sleep 7.117
noteoff 13 57 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 45 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 90.362
noteoff 11 69 0
noteon 11 73 102
sleep 4.761
noteoff 15 45 0
sleep 1.587
noteoff 12 64 0
noteon 12 69 102
sleep 6.349
noteoff 13 57 0
noteon 13 57 104
sleep 1.587
noteon 15 45 92
sleep 1.587
noteoff 14 45 0
noteon 14 45 106
sleep 84.126
noteoff 15 45 0
sleep 9.523
noteon 15 45 92
sleep 65.079
noteoff 4 57 0
sleep 6.349
noteoff 5 45 0
sleep 6.349
noteoff 10 81 0
noteon 10 80 102
sleep 1.779
noteoff 0 85 0
noteoff 0 81 0
noteon 0 83 101
noteon 0 86 101
sleep 1.779
noteoff 1 81 0
noteoff 1 69 0
noteoff 11 73 0
noteon 1 68 100
noteon 1 80 100
noteon 4 64 100
noteon 11 74 102
sleep 1.779
noteoff 2 73 0
noteoff 2 69 0
noteon 2 74 101
noteon 2 71 101
sleep 3.558
noteoff 15 45 0
sleep 1.779
noteoff 12 69 0
noteon 5 57 100
noteon 12 71 102
sleep 5.338
noteoff 3 57 0
noteoff 3 61 0
noteon 3 62 100
noteon 3 59 100
sleep 1.779
noteoff 13 57 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 45 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 72.953
noteoff 4 64 0
sleep 7.117
noteoff 5 57 0
sleep 7.117
noteoff 10 80 0
noteon 10 79 102
sleep 1.779
noteoff 0 86 0
noteoff 0 83 0
noteon 0 88 101
noteon 0 85 101
sleep 1.779
noteoff 1 80 0
noteoff 1 68 0
noteoff 11 74 0
noteon 1 79 100
noteon 1 67 100
noteon 4 64 100
noteon 11 76 102
sleep 1.779
noteoff 2 71 0
noteoff 2 74 0
noteon 2 76 101
noteon 2 73 101
sleep 3.558
noteoff 15 45 0
sleep 1.779
noteoff 12 71 0
noteon 5 57 100
noteon 12 73 102
sleep 5.338
noteoff 3 59 0
noteoff 3 62 0
noteon 3 64 100
noteon 3 61 100
sleep 1.779
noteoff 13 57 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 45 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 51.601
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 76 0
sleep 7.117
noteoff 0 85 0
noteoff 0 88 0
noteoff 12 73 0
sleep 1.779
noteoff 1 67 0
noteoff 1 79 0
sleep 1.779
noteoff 2 73 0
noteoff 2 76 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 64 0
noteoff 14 45 0
sleep 1.779
noteoff 6 69 0
noteoff 6 57 0
sleep 1.779
noteoff 3 61 0
noteoff 3 64 0
sleep 3.558
noteoff 5 57 0
sleep 7.117
noteon 10 78 102
sleep 1.587
noteon 0 78 101
noteon 0 90 101
sleep 1.587
noteon 1 78 100
noteon 1 66 100
noteon 4 62 100
noteon 11 78 102
sleep 1.587
noteon 2 78 101
noteon 2 74 101
noteon 6 62 108
noteon 6 74 108
sleep 3.174
noteoff 15 45 0
sleep 1.587
noteon 5 54 100
noteon 12 74 102
sleep 4.761
noteon 3 66 100
noteon 3 62 100
sleep 1.587
noteon 13 62 104
sleep 1.587
noteon 15 50 109
sleep 1.587
noteon 14 50 106
sleep 171.428
noteoff 10 78 0
sleep 1.779
noteoff 0 90 0
noteoff 0 78 0
sleep 1.779
noteoff 1 66 0
noteoff 1 78 0
noteoff 4 62 0
noteoff 11 78 0
sleep 1.779
noteoff 2 74 0
noteoff 2 78 0
noteoff 6 74 0
noteoff 6 62 0
sleep 5.338
noteoff 5 54 0
noteoff 12 74 0
sleep 5.338
noteoff 3 62 0
noteoff 3 66 0
sleep 1.779
noteoff 13 62 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 50 0
sleep 192.17
noteon 10 69 102
noteon 10 85 102
sleep 1.779
noteon 0 85 101
sleep 1.779
noteon 1 76 100
noteon 1 73 100
noteon 4 64 100
noteon 11 69 102
noteon 11 79 102
sleep 1.779
noteon 2 76 101
noteon 2 73 101
noteon 6 69 108
noteon 6 57 108
sleep 5.338
noteon 5 57 100
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteon 14 45 106
sleep 85.409
noteoff 10 85 0
noteoff 10 69 0
sleep 1.779
noteoff 0 85 0
sleep 1.779
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 11 79 0
noteoff 11 69 0
sleep 1.779
noteoff 2 73 0
noteoff 2 76 0
noteoff 6 57 0
noteoff 6 69 0
sleep 3.558
noteoff 15 45 0
sleep 1.779
noteoff 5 57 0
noteoff 12 69 0
sleep 5.338
noteoff 3 57 0
sleep 1.779
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 85.409
noteon 10 62 102
noteon 10 86 102
noteon 10 69 102
sleep 1.858
noteon 0 86 101
sleep 1.858
noteon 1 74 100
noteon 1 78 100
noteon 4 62 100
noteon 11 78 102
noteon 11 62 102
noteon 11 69 102
sleep 1.858
noteon 2 74 101
noteon 6 62 108
noteon 6 74 108
sleep 5.576
noteon 5 54 100
noteon 12 62 102
sleep 5.576
noteon 3 50 100
noteon 3 62 100
sleep 1.858
noteon 13 50 104
sleep 1.858
noteon 15 50 92
sleep 1.858
noteon 14 38 106
sleep 89.219
noteoff 10 69 0
noteoff 10 86 0
noteoff 10 62 0
sleep 1.858
noteoff 0 86 0
sleep 1.858
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 69 0
noteoff 11 62 0
noteoff 11 78 0
sleep 1.858
noteoff 2 74 0
noteoff 6 74 0
noteoff 6 62 0
sleep 3.717
noteoff 15 50 0
sleep 1.858
noteoff 5 54 0
noteoff 12 62 0
sleep 5.576
noteoff 3 62 0
noteoff 3 50 0
sleep 1.858
noteoff 13 50 0
sleep 3.717
noteoff 14 38 0
sleep 535.315
noteon 10 69 102
sleep 3.322
noteon 11 61 102
sleep 6.644
noteon 12 57 102
sleep 6.644
noteon 13 57 104
sleep 3.322
noteon 14 45 106
sleep 83.056
noteoff 11 61 0
sleep 6.644
noteoff 12 57 0
sleep 6.644
noteoff 13 57 0
sleep 3.322
noteoff 14 45 0
sleep 83.292
noteon 11 62 102
sleep 7.117
noteon 12 59 102
sleep 7.117
noteon 13 59 104
sleep 3.558
noteon 14 47 106
sleep 88.967
noteoff 11 62 0
sleep 7.117
noteoff 12 59 0
sleep 7.117
noteoff 13 59 0
sleep 3.558
noteoff 14 47 0
sleep 88.967
noteon 11 64 102
sleep 7.117
noteon 12 61 102
sleep 7.117
noteon 13 61 104
sleep 3.558
noteon 14 49 106
sleep 88.967
noteoff 11 64 0
sleep 7.117
noteoff 12 61 0
sleep 7.117
noteoff 13 61 0
sleep 3.558
noteoff 14 49 0
sleep 49.822
noteoff 10 69 0
sleep 35.587
noteon 10 77 102
sleep 3.322
noteon 11 65 102
sleep 6.644
noteon 12 62 102
sleep 6.644
noteon 13 62 104
sleep 3.322
noteon 14 50 106
sleep 64.784
noteoff 10 77 0
sleep 114.617
noteon 10 76 102
sleep 3.558
noteoff 11 65 0
sleep 7.117
noteoff 12 62 0
sleep 7.117
noteoff 13 62 0
sleep 3.558
noteoff 14 50 0
sleep 69.395
noteoff 10 76 0
sleep 122.775
noteon 10 77 102
sleep 90.747
noteoff 10 77 0
sleep 122.775
noteon 10 65 102
sleep 3.322
noteon 11 57 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 53 104
sleep 3.322
noteon 14 41 106
sleep 83.056
noteoff 11 57 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 53 0
sleep 3.322
noteoff 14 41 0
sleep 83.292
noteon 11 58 102
sleep 7.117
noteon 12 55 102
sleep 7.117
noteon 13 55 104
sleep 3.558
noteon 14 43 106
sleep 88.967
noteoff 11 58 0
sleep 7.117
noteoff 12 55 0
sleep 7.117
noteoff 13 55 0
sleep 3.558
noteoff 14 43 0
sleep 88.967
noteon 11 60 102
sleep 7.117
noteon 12 57 102
sleep 7.117
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 88.967
noteoff 11 60 0
sleep 7.117
noteoff 12 57 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 49.822
noteoff 10 65 0
sleep 35.587
noteon 10 74 102
sleep 3.322
noteon 11 62 102
sleep 6.644
noteon 12 58 102
sleep 6.644
noteon 13 58 104
sleep 3.322
noteon 14 46 106
sleep 64.784
noteoff 10 74 0
sleep 114.617
noteon 10 73 102
sleep 3.558
noteoff 11 62 0
sleep 7.117
noteoff 12 58 0
sleep 7.117
noteoff 13 58 0
sleep 3.558
noteoff 14 46 0
sleep 69.395
noteoff 10 73 0
sleep 122.775
noteon 10 74 102
sleep 90.747
noteoff 10 74 0
sleep 122.775
noteon 10 58 102
sleep 3.322
noteon 11 58 102
noteon 11 62 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 58 0
sleep 3.322
noteoff 11 62 0
noteoff 11 58 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 79.734
noteon 10 77 102
sleep 3.558
noteon 11 62 102
noteon 11 58 102
sleep 7.117
noteon 12 53 102
sleep 7.116
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 782.931
noteoff 10 77 0
sleep 35.587
noteon 10 74 102
sleep 106.761
noteoff 10 74 0
noteon 10 70 102
sleep 74.733
noteoff 11 58 0
noteoff 11 62 0
sleep 32.028
noteoff 10 70 0
noteon 10 69 102
sleep 3.322
noteon 11 63 102
noteon 11 60 102
sleep 96.345
noteoff 10 69 0
noteon 10 72 102
sleep 69.767
noteoff 11 60 0
noteoff 11 63 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 72 0
noteon 10 75 102
sleep 3.558
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 75 0
noteon 10 74 102
sleep 74.733
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 74 0
noteon 10 72 102
sleep 3.558
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 72 0
noteon 10 70 102
sleep 71.174
noteoff 10 70 0
sleep 3.558
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteon 10 69 102
sleep 3.322
noteon 11 60 102
noteon 11 63 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 69 0
noteon 10 67 102
sleep 69.767
noteoff 11 63 0
noteoff 11 60 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 67 0
noteon 10 65 102
sleep 3.558
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 65 0
noteon 10 63 102
sleep 74.733
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 63 0
noteon 10 62 102
sleep 1.779
noteon 0 81 101
sleep 1.779
noteon 11 63 102
noteon 11 60 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 62 0
noteon 10 60 102
sleep 71.174
noteoff 10 60 0
sleep 3.558
noteoff 11 60 0
noteoff 11 63 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteon 10 58 102
sleep 1.661
noteoff 0 81 0
noteon 0 82 101
sleep 1.661
noteon 1 70 100
noteon 1 74 100
noteon 11 58 102
noteon 11 62 102
sleep 6.644
noteon 12 53 102
sleep 4.983
noteoff 3 57 0
noteon 3 58 100
sleep 1.661
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 58 0
sleep 1.661
noteoff 0 82 0
sleep 1.661
noteoff 1 74 0
noteoff 1 70 0
noteoff 11 62 0
noteoff 11 58 0
sleep 6.644
noteoff 12 53 0
sleep 4.983
noteoff 3 58 0
sleep 1.661
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 79.734
noteon 10 77 102
sleep 1.779
noteon 0 89 101
sleep 1.779
noteon 1 74 100
noteon 11 58 102
noteon 1 70 100
noteon 11 62 102
sleep 7.117
noteon 12 53 102
sleep 5.337
noteon 3 65 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 782.921
noteoff 10 77 0
sleep 19.572
noteoff 0 89 0
sleep 14.234
noteoff 3 65 0
sleep 1.779
noteon 10 74 102
sleep 1.779
noteon 0 86 101
sleep 14.234
noteon 3 62 100
sleep 90.747
noteoff 10 74 0
noteon 10 70 102
sleep 1.779
noteoff 0 86 0
noteon 0 82 101
sleep 14.234
noteoff 3 62 0
noteon 3 58 100
sleep 55.16
noteoff 10 70 0
sleep 3.558
noteoff 11 62 0
noteoff 11 58 0
sleep 16.014
noteoff 0 82 0
sleep 1.779
noteoff 1 70 0
noteoff 1 74 0
sleep 12.455
noteoff 3 58 0
sleep 1.779
noteon 10 69 102
sleep 1.661
noteon 0 81 101
sleep 1.661
noteon 1 72 100
noteon 1 75 100
noteon 11 60 102
noteon 11 63 102
sleep 11.627
noteon 3 57 100
sleep 84.717
noteoff 10 69 0
noteon 10 72 102
sleep 1.661
noteoff 0 81 0
noteon 0 84 101
sleep 68.106
noteoff 11 63 0
noteoff 11 60 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 72 0
noteon 10 75 102
sleep 1.779
noteoff 0 84 0
noteon 0 87 101
sleep 1.779
noteon 11 63 102
noteon 11 60 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteoff 3 57 0
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 75 0
noteon 10 74 102
sleep 1.779
noteoff 0 87 0
noteon 0 86 101
sleep 72.953
noteoff 11 60 0
noteoff 11 63 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 74 0
noteon 10 72 102
sleep 1.779
noteoff 0 86 0
noteon 0 84 101
sleep 1.779
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 72 0
noteon 10 70 102
sleep 1.779
noteoff 0 84 0
noteon 0 82 101
sleep 72.953
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 70 0
noteon 10 69 102
sleep 1.661
noteoff 0 82 0
noteon 0 81 101
sleep 1.661
noteon 11 60 102
noteon 11 63 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 69 0
noteon 10 67 102
sleep 1.661
noteoff 0 81 0
noteon 0 79 101
sleep 68.106
noteoff 11 63 0
noteoff 11 60 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 67 0
noteon 10 65 102
sleep 1.779
noteoff 0 79 0
noteon 0 77 101
sleep 1.779
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteon 3 53 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 65 0
noteon 10 63 102
sleep 1.779
noteoff 0 77 0
noteon 0 75 101
sleep 72.953
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 12.455
noteoff 3 53 0
sleep 1.779
noteoff 10 63 0
noteon 10 62 102
sleep 1.779
noteoff 0 75 0
noteon 0 74 101
sleep 1.779
noteon 11 63 102
noteon 11 60 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteon 3 53 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 62 0
noteon 10 60 102
sleep 1.779
noteoff 0 74 0
noteon 0 72 101
sleep 69.395
noteoff 10 60 0
sleep 3.558
noteoff 11 60 0
noteoff 11 63 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 1.779
noteoff 0 72 0
sleep 1.779
noteoff 1 75 0
noteoff 1 72 0
noteoff 14 34 0
sleep 12.455
noteoff 3 53 0
sleep 1.779
noteon 10 58 102
sleep 1.661
noteon 0 70 101
sleep 1.661
noteon 1 70 100
noteon 1 74 100
noteon 11 62 102
noteon 11 58 102
sleep 6.644
noteon 12 53 102
sleep 4.983
noteon 3 58 100
sleep 1.661
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 58 0
sleep 3.322
noteoff 11 58 0
noteoff 11 62 0
sleep 6.644
noteoff 12 53 0
sleep 89.7
noteon 10 74 102
sleep 1.779
noteoff 0 70 0
sleep 1.779
noteoff 1 74 0
noteoff 1 70 0
noteon 11 62 102
sleep 7.117
noteon 12 58 102
sleep 5.338
noteoff 3 58 0
sleep 1.779
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 85.409
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 74 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 58 0
noteon 12 62 102
sleep 96.085
noteoff 10 74 0
noteon 10 76 102
sleep 71.174
noteoff 10 76 0
sleep 35.587
noteon 10 77 102
sleep 3.3
noteoff 11 65 0
sleep 6.6
noteoff 12 62 0
sleep 4.95
noteon 3 57 100
sleep 1.65
noteon 13 45 104
sleep 3.3
noteon 14 33 106
sleep 79.207
noteoff 10 77 0
sleep 99.009
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 7.117
noteon 12 57 102
sleep 5.338
noteoff 3 57 0
sleep 1.779
noteoff 13 45 0
sleep 3.558
noteoff 14 33 0
sleep 85.409
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 74 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 57 0
noteon 12 62 102
sleep 96.085
noteoff 10 74 0
noteon 10 76 102
sleep 71.174
noteoff 10 76 0
sleep 35.587
noteon 10 77 102
sleep 3.267
noteoff 11 65 0
sleep 6.535
noteoff 12 62 0
sleep 4.901
noteon 3 56 100
sleep 1.633
noteon 13 44 104
sleep 3.267
noteon 14 32 106
sleep 78.431
noteoff 10 77 0
sleep 98.039
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 7.117
noteon 12 59 102
sleep 5.338
noteoff 3 56 0
sleep 1.779
noteoff 13 44 0
sleep 3.558
noteoff 14 32 0
sleep 85.409
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 74 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 59 0
noteon 12 62 102
sleep 96.085
noteoff 10 74 0
noteon 10 76 102
sleep 71.174
noteoff 10 76 0
sleep 35.587
noteon 10 77 102
sleep 3.257
noteoff 11 65 0
noteon 11 65 102
sleep 6.514
noteoff 12 62 0
noteon 12 56 102
noteon 12 59 102
sleep 4.885
noteon 3 56 100
sleep 1.628
noteon 13 56 104
sleep 3.257
noteon 14 44 106
sleep 78.172
noteoff 10 77 0
sleep 3.257
noteoff 11 65 0
sleep 6.514
noteoff 12 59 0
noteoff 12 56 0
sleep 87.942
noteon 10 79 102
sleep 3.558
noteon 11 65 102
sleep 7.117
noteon 12 56 102
noteon 12 59 102
sleep 5.338
noteoff 3 56 0
sleep 1.779
noteoff 13 56 0
sleep 3.558
noteoff 14 44 0
sleep 85.406
noteoff 10 79 0
noteon 10 77 102
sleep 3.558
noteoff 11 65 0
sleep 7.117
noteoff 12 59 0
noteoff 12 56 0
sleep 96.081
noteoff 10 77 0
noteon 10 76 102
sleep 3.558
noteon 11 65 102
sleep 7.117
noteon 12 56 102
noteon 12 59 102
sleep 96.080
noteoff 10 76 0
noteon 10 74 102
sleep 3.558
noteoff 11 65 0
sleep 7.117
noteoff 12 59 0
noteoff 12 56 0
sleep 60.495
noteoff 10 74 0
sleep 35.587
noteon 10 73 102
sleep 3.236
noteon 11 64 102
sleep 6.472
noteon 12 61 102
noteon 12 57 102
sleep 4.854
noteon 3 57 100
sleep 1.618
noteon 13 57 104
sleep 3.236
noteon 14 33 106
sleep 77.669
noteoff 10 73 0
sleep 16.181
noteoff 13 57 0
sleep 80.906
noteon 10 74 102
sleep 3.558
noteoff 11 64 0
sleep 7.117
noteoff 12 57 0
noteoff 12 61 0
sleep 5.338
noteoff 3 57 0
sleep 90.747
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 71 102
sleep 10.676
noteon 12 49 102
sleep 7.117
noteon 13 49 104
sleep 88.967
noteoff 10 71 0
noteon 10 69 102
sleep 71.174
noteoff 10 69 0
sleep 35.587
noteon 10 77 102
sleep 9.677
noteoff 12 49 0
noteon 12 50 102
sleep 6.451
noteoff 13 49 0
noteon 13 50 104
sleep 80.641
noteoff 10 77 0
sleep 96.769
noteon 10 79 102
sleep 10.676
noteoff 12 50 0
noteon 12 53 102
sleep 7.117
noteoff 13 50 0
noteon 13 53 104
sleep 88.966
noteoff 10 79 0
noteon 10 77 102
sleep 106.758
noteoff 10 77 0
noteon 10 76 102
sleep 10.676
noteoff 12 53 0
noteon 12 56 102
sleep 7.117
noteoff 13 53 0
noteon 13 56 104
sleep 88.964
noteoff 10 76 0
noteon 10 74 102
sleep 71.172
noteoff 10 74 0
sleep 10.676
noteoff 12 56 0
sleep 7.117
noteoff 13 56 0
sleep 17.793
noteon 10 73 102
sleep 9.646
noteon 12 57 102
sleep 6.43
noteon 13 57 104
sleep 80.385
noteoff 10 73 0
sleep 9.646
noteoff 12 57 0
sleep 6.43
noteoff 13 57 0
sleep 80.385
noteon 10 74 102
sleep 106.761
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 71 102
sleep 3.558
noteon 11 61 102
sleep 7.117
noteon 12 49 102
sleep 7.117
noteon 13 49 104
sleep 88.967
noteoff 10 71 0
noteon 10 69 102
sleep 71.174
noteoff 10 69 0
sleep 35.587
noteon 10 77 102
sleep 3.205
noteoff 11 61 0
noteon 11 62 102
sleep 6.41
noteoff 12 49 0
noteon 12 50 102
sleep 6.41
noteoff 13 49 0
noteon 13 50 104
sleep 80.128
noteoff 10 77 0
sleep 96.153
noteon 10 79 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 50 0
noteon 12 53 102
sleep 7.117
noteoff 13 50 0
noteon 13 53 104
sleep 88.959
noteoff 10 79 0
noteon 10 77 102
sleep 106.751
noteoff 10 77 0
noteon 10 76 102
sleep 3.558
noteoff 11 65 0
noteon 11 68 102
sleep 7.116
noteoff 12 53 0
noteon 12 56 102
sleep 7.116
noteoff 13 53 0
noteon 13 56 104
sleep 88.959
noteoff 10 76 0
noteon 10 74 102
sleep 71.167
noteoff 10 74 0
sleep 3.558
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 7.117
noteoff 13 56 0
sleep 17.792
noteon 10 73 102
sleep 3.194
noteon 11 69 102
sleep 6.389
noteon 12 57 102
sleep 6.389
noteon 13 57 104
sleep 79.859
noteoff 10 73 0
sleep 3.194
noteoff 11 69 0
sleep 6.388
noteoff 12 57 0
sleep 6.389
noteoff 13 57 0
sleep 79.859
noteon 10 76 102
sleep 3.558
noteon 11 57 102
sleep 17.791
noteoff 14 33 0
sleep 85.400
noteoff 10 76 0
noteon 10 74 102
sleep 106.751
noteoff 10 74 0
noteon 10 73 102
sleep 3.558
noteoff 11 57 0
noteon 11 58 102
sleep 103.193
noteoff 10 73 0
noteon 10 71 102
sleep 71.167
noteoff 10 71 0
sleep 35.583
noteon 10 69 102
sleep 3.174
noteoff 11 58 0
noteon 11 59 102
sleep 92.056
noteoff 10 69 0
noteon 10 68 102
sleep 63.487
noteoff 10 68 0
sleep 31.744
noteon 10 69 102
sleep 3.558
noteoff 11 59 0
noteon 11 60 102
sleep 49.817
noteoff 10 69 0
sleep 53.375
noteon 10 67 102
sleep 53.376
noteoff 10 67 0
sleep 53.375
noteon 10 66 102
sleep 3.558
noteoff 11 60 0
noteon 11 61 102
sleep 49.817
noteoff 10 66 0
sleep 53.375
noteon 10 64 102
sleep 71.420
noteoff 10 64 0
sleep 28.568
noteoff 11 61 0
sleep 42.852
noteon 10 62 102
sleep 1.587
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 62 101
noteon 2 74 101
noteon 6 62 108
noteon 6 74 108
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 46.969
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 15.097
noteoff 0 74 0
sleep 1.677
noteoff 1 74 0
sleep 1.677
noteoff 2 74 0
noteoff 2 62 0
sleep 11.741
noteon 10 78 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 55 102
noteon 10 64 102
sleep 1.587
noteon 0 76 101
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 1 76 100
noteon 4 62 100
noteon 11 62 102
noteon 11 59 102
sleep 1.587
noteon 2 64 101
noteon 2 74 101
noteon 6 62 108
noteon 6 74 108
sleep 4.761
noteon 5 50 100
noteon 12 67 102
sleep 4.761
noteon 3 55 100
sleep 1.587
noteon 13 55 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 43 106
sleep 85.706
noteoff 12 67 0
sleep 4.761
noteoff 3 55 0
sleep 1.587
noteoff 13 55 0
sleep 3.174
noteoff 14 43 0
sleep 86.248
noteon 12 69 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 69 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 71 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 71 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 46.969
noteoff 10 64 0
noteoff 10 55 0
sleep 3.354
noteoff 11 59 0
noteoff 11 62 0
sleep 15.097
noteoff 0 74 0
noteoff 0 76 0
sleep 1.677
noteoff 1 76 0
noteoff 1 74 0
sleep 1.677
noteoff 2 74 0
noteoff 2 64 0
sleep 11.741
noteon 10 79 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 79 0
sleep 109.523
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 122.775
noteon 10 83 102
sleep 90.747
noteoff 10 83 0
sleep 125.949
noteon 4 64 100
sleep 6.349
noteon 5 57 100
sleep 88.888
noteoff 4 64 0
sleep 6.349
noteoff 5 57 0
sleep 89.272
noteon 4 66 100
sleep 7.117
noteon 5 62 100
sleep 99.644
noteoff 4 66 0
sleep 7.117
noteoff 5 62 0
sleep 99.644
noteon 4 67 100
sleep 7.117
noteon 5 64 100
sleep 99.644
noteoff 4 67 0
sleep 7.117
noteoff 5 64 0
sleep 96.085
noteon 10 76 102
sleep 3.174
noteon 11 69 102
sleep 77.777
noteoff 10 76 0
sleep 3.174
noteoff 11 69 0
sleep 106.349
noteon 10 78 102
sleep 3.558
noteon 11 74 102
sleep 87.188
noteoff 10 78 0
sleep 3.558
noteoff 11 74 0
sleep 119.217
noteon 10 79 102
sleep 3.558
noteon 11 76 102
sleep 87.188
noteoff 10 79 0
sleep 3.558
noteoff 11 76 0
sleep 122.391
noteon 1 74 100
noteon 4 66 100
noteon 1 78 100
sleep 6.349
noteon 5 62 100
sleep 88.888
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
sleep 6.349
noteoff 5 62 0
sleep 89.272
noteon 1 69 100
noteon 1 76 100
noteon 4 64 100
sleep 7.117
noteon 5 57 100
sleep 99.644
noteoff 1 76 0
noteoff 1 69 0
noteoff 4 64 0
sleep 7.117
noteoff 5 57 0
sleep 99.644
noteon 1 66 100
noteon 1 74 100
noteon 4 62 100
sleep 7.117
noteon 5 54 100
sleep 99.644
noteoff 1 74 0
noteoff 1 66 0
noteoff 4 62 0
sleep 7.117
noteoff 5 54 0
sleep 96.085
noteon 10 86 102
sleep 1.587
noteon 0 86 101
noteon 0 78 101
sleep 1.587
noteon 1 74 100
noteon 1 78 100
noteon 11 69 102
noteon 11 78 102
sleep 1.587
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
noteon 2 66 101
sleep 4.761
noteon 12 62 102
sleep 4.761
noteon 3 50 100
noteon 3 62 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 92
sleep 1.587
noteon 14 38 106
sleep 76.19
noteoff 10 86 0
sleep 1.587
noteoff 0 78 0
noteoff 0 86 0
sleep 1.587
noteoff 1 78 0
noteoff 1 74 0
noteoff 11 78 0
noteoff 11 69 0
sleep 1.587
noteoff 2 66 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 12 62 0
sleep 4.761
noteoff 3 62 0
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 1.587
noteoff 15 50 0
sleep 1.587
noteoff 14 38 0
sleep 76.19
noteon 10 85 102
sleep 1.779
noteon 0 76 101
noteon 0 85 101
sleep 1.779
noteon 1 79 100
noteon 1 76 100
noteon 11 69 102
noteon 11 79 102
sleep 1.779
noteon 2 76 101
noteon 2 67 101
noteon 6 69 108
noteon 6 57 108
sleep 5.338
noteon 12 57 102
sleep 5.338
noteon 3 45 100
noteon 3 57 100
sleep 1.779
noteon 13 45 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteon 14 33 106
sleep 85.409
noteoff 10 85 0
sleep 1.779
noteoff 0 85 0
noteoff 0 76 0
sleep 1.779
noteoff 1 76 0
noteoff 1 79 0
noteoff 11 79 0
noteoff 11 69 0
sleep 1.779
noteoff 2 67 0
noteoff 2 76 0
noteoff 6 57 0
noteoff 6 69 0
sleep 5.338
noteoff 12 57 0
sleep 5.338
noteoff 3 57 0
noteoff 3 45 0
sleep 1.779
noteoff 13 45 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 33 0
sleep 85.409
noteon 10 86 102
sleep 1.779
noteon 0 78 101
noteon 0 86 101
sleep 1.779
noteon 1 74 100
noteon 1 78 100
noteon 11 69 102
noteon 11 78 102
sleep 1.779
noteon 2 74 101
noteon 2 66 101
noteon 6 74 108
noteon 6 62 108
sleep 5.338
noteon 12 62 102
sleep 5.338
noteon 3 50 100
noteon 3 62 100
sleep 1.779
noteon 13 50 104
sleep 1.779
noteon 15 50 92
sleep 1.779
noteon 14 38 106
sleep 85.409
noteoff 10 86 0
sleep 1.779
noteoff 0 86 0
noteoff 0 78 0
sleep 1.779
noteoff 1 78 0
noteoff 1 74 0
noteoff 11 78 0
noteoff 11 69 0
sleep 1.779
noteoff 2 66 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 12 62 0
sleep 5.338
noteoff 3 62 0
noteoff 3 50 0
sleep 1.779
noteoff 13 50 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 38 0
sleep 85.409
noteon 10 62 102
sleep 1.587
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 62 101
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 46.969
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 15.097
noteoff 0 74 0
sleep 1.677
noteoff 1 74 0
sleep 1.677
noteoff 2 74 0
noteoff 2 62 0
sleep 11.741
noteon 10 78 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 64 102
noteon 10 55 102
sleep 1.587
noteon 0 76 101
noteon 0 74 101
sleep 1.587
noteon 1 76 100
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
noteon 11 59 102
sleep 1.587
noteon 2 74 101
noteon 2 64 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 5 50 100
noteon 12 55 102
sleep 4.761
noteon 3 55 100
sleep 1.587
noteon 13 55 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 43 106
sleep 85.706
noteoff 12 55 0
sleep 4.761
noteoff 3 55 0
sleep 1.587
noteoff 13 55 0
sleep 3.174
noteoff 14 43 0
sleep 86.248
noteon 12 57 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 57 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 59 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 59 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 46.969
noteoff 10 55 0
noteoff 10 64 0
sleep 3.354
noteoff 11 59 0
noteoff 11 62 0
sleep 15.097
noteoff 0 74 0
noteoff 0 76 0
sleep 1.677
noteoff 1 74 0
noteoff 1 76 0
sleep 1.677
noteoff 2 64 0
noteoff 2 74 0
sleep 11.741
noteon 10 79 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 79 0
sleep 109.523
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 122.775
noteon 10 83 102
sleep 90.747
noteoff 10 83 0
sleep 137.576
noteon 10 67 102
sleep 83.061
noteoff 10 67 0
sleep 112.377
noteon 10 69 102
sleep 86.148
noteoff 10 69 0
sleep 116.554
noteon 10 71 102
sleep 86.148
noteoff 10 71 0
sleep 119.811
noteon 11 64 102
sleep 83.061
noteoff 11 64 0
sleep 112.498
noteon 11 66 102
sleep 86.148
noteoff 11 66 0
sleep 116.554
noteon 11 67 102
sleep 86.148
noteoff 11 67 0
sleep 113.175
noteon 10 67 102
sleep 83.061
noteoff 10 67 0
sleep 112.377
noteon 10 69 102
sleep 86.148
noteoff 10 69 0
sleep 116.554
noteon 10 70 102
sleep 86.148
noteoff 10 70 0
sleep 119.811
noteon 11 64 102
sleep 83.061
noteoff 11 64 0
sleep 112.498
noteon 11 66 102
sleep 86.148
noteoff 11 66 0
sleep 116.554
noteon 11 67 102
sleep 86.148
noteoff 11 67 0
sleep 113.175
noteon 10 67 102
sleep 83.061
noteoff 10 67 0
sleep 112.376
noteon 10 69 102
sleep 86.148
noteoff 10 69 0
sleep 116.552
noteon 10 70 102
sleep 86.148
noteoff 10 70 0
sleep 116.554
noteon 10 67 102
sleep 3.257
noteon 11 64 102
sleep 79.803
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.12
noteon 10 69 102
sleep 3.424
noteon 11 65 102
sleep 83.904
noteoff 10 69 0
sleep 3.424
noteoff 11 65 0
sleep 114.725
noteon 10 70 102
sleep 3.424
noteon 11 67 102
sleep 83.904
noteoff 10 70 0
sleep 3.424
noteoff 11 67 0
sleep 114.725
noteon 10 67 102
sleep 3.257
noteon 11 64 102
sleep 79.803
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.12
noteon 10 69 102
sleep 3.472
noteon 11 65 102
sleep 85.068
noteoff 10 69 0
sleep 3.472
noteoff 11 65 0
sleep 116.319
noteon 10 70 102
sleep 3.472
noteon 11 67 102
sleep 85.069
noteoff 10 70 0
sleep 3.472
noteoff 11 67 0
sleep 116.318
noteon 10 67 102
sleep 3.257
noteon 11 64 102
sleep 79.804
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.119
noteon 10 69 102
sleep 3.558
noteon 11 65 102
sleep 87.187
noteoff 10 69 0
sleep 3.558
noteoff 11 65 0
sleep 119.217
noteon 10 70 102
sleep 3.558
noteon 11 67 102
sleep 87.186
noteoff 10 70 0
sleep 3.558
noteoff 11 67 0
sleep 119.217
noteon 10 67 102
sleep 3.257
noteon 1 72 100
noteon 11 64 102
sleep 6.514
noteon 12 60 102
sleep 4.885
noteon 3 60 100
sleep 1.628
noteon 13 60 104
sleep 3.257
noteon 14 48 106
sleep 63.517
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.12
noteon 10 69 102
sleep 3.558
noteon 11 65 102
sleep 7.117
noteoff 12 60 0
sleep 7.117
noteoff 13 60 0
sleep 3.558
noteoff 14 48 0
sleep 69.395
noteoff 10 69 0
sleep 3.558
noteoff 11 65 0
sleep 119.217
noteon 10 70 102
sleep 3.558
noteon 11 67 102
sleep 87.188
noteoff 10 70 0
sleep 3.558
noteoff 11 67 0
sleep 96.085
noteoff 1 72 0
sleep 12.455
noteoff 3 60 0
sleep 10.676
noteon 10 69 102
sleep 3.257
noteon 1 77 100
noteon 11 65 102
sleep 6.514
noteon 12 53 102
sleep 4.885
noteon 3 65 100
sleep 1.628
noteon 13 53 104
sleep 3.257
noteon 14 41 106
sleep 81.433
noteoff 1 77 0
sleep 11.4
noteoff 3 65 0
sleep 83.061
noteoff 10 69 0
sleep 3.558
noteoff 11 65 0
noteon 1 76 100
sleep 7.117
noteoff 12 53 0
sleep 5.338
noteon 3 64 100
sleep 1.779
noteoff 13 53 0
sleep 3.558
noteoff 14 41 0
sleep 88.967
noteoff 1 76 0
sleep 12.455
noteoff 3 64 0
sleep 94.306
noteon 1 77 100
sleep 12.455
noteon 3 65 100
sleep 94.306
noteoff 1 77 0
sleep 12.455
noteoff 3 65 0
sleep 90.747
noteon 10 64 102
sleep 3.257
noteon 1 69 100
noteon 11 61 102
sleep 6.514
noteon 12 57 102
sleep 4.885
noteon 3 57 100
sleep 1.628
noteon 13 57 104
sleep 3.257
noteon 14 45 106
sleep 63.517
noteoff 10 64 0
sleep 3.257
noteoff 11 61 0
sleep 109.12
noteon 10 65 102
sleep 3.558
noteon 11 62 102
sleep 7.117
noteoff 12 57 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 69.395
noteoff 10 65 0
sleep 3.558
noteoff 11 62 0
sleep 119.217
noteon 10 67 102
sleep 3.558
noteon 11 64 102
sleep 87.188
noteoff 10 67 0
sleep 3.558
noteoff 11 64 0
sleep 96.085
noteoff 1 69 0
sleep 12.455
noteoff 3 57 0
sleep 10.676
noteon 10 65 102
sleep 3.257
noteon 1 74 100
noteon 11 62 102
sleep 6.514
noteon 12 50 102
sleep 4.885
noteon 3 62 100
sleep 1.628
noteon 13 50 104
sleep 3.257
noteon 14 38 106
sleep 81.433
noteoff 1 74 0
sleep 11.4
noteoff 3 62 0
sleep 83.061
noteoff 10 65 0
sleep 3.558
noteoff 11 62 0
noteon 1 73 100
sleep 7.117
noteoff 12 50 0
sleep 5.338
noteon 3 61 100
sleep 1.779
noteoff 13 50 0
sleep 3.558
noteoff 14 38 0
sleep 88.967
noteoff 1 73 0
sleep 12.455
noteoff 3 61 0
sleep 94.306
noteon 1 74 100
sleep 12.455
noteon 3 62 100
sleep 94.948
noteoff 1 74 0
sleep 14.705
noteoff 3 62 0
sleep 110.306
noteon 4 62 100
noteon 11 62 102
sleep 1.582
noteon 2 74 101
noteon 2 62 101
sleep 4.746
noteon 5 50 100
noteon 12 50 102
sleep 4.746
noteon 3 50 100
noteon 3 46 100
sleep 1.582
noteon 13 50 104
sleep 3.164
noteon 14 34 106
sleep 174.430
noteoff 11 62 0
noteon 11 64 102
sleep 7.117
noteoff 12 50 0
noteon 12 52 102
sleep 5.337
noteoff 3 50 0
noteon 3 52 100
sleep 1.779
noteoff 13 50 0
noteon 13 52 104
sleep 199.274
noteoff 11 64 0
noteon 11 65 102
sleep 7.117
noteoff 12 52 0
noteon 12 53 102
sleep 5.337
noteoff 3 52 0
noteon 3 53 100
sleep 1.779
noteoff 13 52 0
noteon 13 53 104
sleep 198.889
noteoff 11 65 0
noteon 11 66 102
sleep 6.349
noteoff 12 53 0
noteon 12 54 102
sleep 4.761
noteoff 3 53 0
noteon 3 54 100
sleep 1.587
noteoff 13 53 0
noteon 13 54 104
sleep 178.144
noteoff 11 66 0
noteon 11 67 102
sleep 7.117
noteoff 12 54 0
noteon 12 55 102
sleep 5.337
noteoff 3 54 0
noteon 3 55 100
sleep 1.779
noteoff 13 54 0
noteon 13 55 104
sleep 199.274
noteoff 11 67 0
noteon 11 68 102
sleep 7.116
noteoff 12 55 0
noteon 12 56 102
sleep 5.337
noteoff 3 55 0
noteon 3 56 100
sleep 1.779
noteoff 13 55 0
noteon 13 56 104
sleep 163.687
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 3.558
noteoff 2 62 0
noteoff 2 74 0
sleep 3.558
noteoff 13 56 0
sleep 3.558
noteoff 14 34 0
sleep 3.558
noteoff 3 56 0
noteoff 3 46 0
sleep 10.674
noteon 10 69 102
noteon 10 81 102
sleep 3.174
noteoff 4 62 0
noteon 4 57 100
noteon 11 69 102
sleep 1.587
noteon 2 73 101
noteon 6 69 108
noteon 6 57 108
noteon 2 61 101
sleep 4.761
noteoff 5 50 0
noteon 5 45 100
noteon 12 57 102
sleep 4.761
noteon 3 57 100
noteon 3 45 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.206
noteoff 12 57 0
sleep 6.349
noteoff 13 57 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 45 0
noteoff 3 57 0
sleep 9.523
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.259
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 12 69 0
sleep 3.558
noteoff 2 61 0
noteoff 2 73 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 57 0
noteoff 6 69 0
sleep 1.779
noteoff 3 57 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 3.174
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
noteon 6 62 108
noteon 6 74 108
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 92.063
noteon 10 76 102
sleep 3.558
noteoff 4 62 0
noteon 11 64 102
sleep 1.779
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 74 0
noteoff 6 62 0
sleep 5.338
noteoff 5 50 0
noteoff 12 62 0
sleep 5.338
noteoff 3 50 0
sleep 1.779
noteoff 13 50 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 38 0
sleep 85.409
noteoff 10 76 0
sleep 3.558
noteoff 11 64 0
sleep 103.202
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 103.202
noteoff 10 74 0
sleep 4.329
noteoff 11 62 0
sleep 125.541
noteon 10 72 102
sleep 3.257
noteon 1 72 100
noteon 11 67 102
sleep 6.514
noteon 12 64 102
sleep 4.885
noteon 3 60 100
sleep 1.628
noteon 13 60 104
sleep 3.257
noteon 14 48 106
sleep 81.433
noteoff 11 67 0
sleep 6.514
noteoff 12 64 0
sleep 91.505
noteon 11 69 102
sleep 7.117
noteon 12 65 102
sleep 7.117
noteoff 13 60 0
sleep 3.558
noteoff 14 48 0
sleep 88.967
noteoff 11 69 0
sleep 7.117
noteoff 12 65 0
sleep 99.644
noteon 11 70 102
sleep 7.117
noteon 12 67 102
sleep 99.644
noteoff 11 70 0
sleep 7.117
noteoff 12 67 0
sleep 60.498
noteoff 10 72 0
sleep 12.455
noteoff 1 72 0
sleep 12.455
noteoff 3 60 0
sleep 10.676
noteon 10 77 102
sleep 3.257
noteon 1 77 100
noteon 11 69 102
sleep 6.514
noteon 12 65 102
sleep 4.885
noteon 3 65 100
sleep 1.628
noteon 13 53 104
sleep 3.257
noteon 14 41 106
sleep 78.175
noteoff 10 77 0
sleep 3.257
noteoff 1 77 0
sleep 11.4
noteoff 3 65 0
sleep 83.061
noteon 10 76 102
sleep 3.558
noteoff 11 69 0
noteon 1 76 100
sleep 7.117
noteoff 12 65 0
sleep 5.338
noteon 3 64 100
sleep 1.779
noteoff 13 53 0
sleep 3.558
noteoff 14 41 0
sleep 85.409
noteoff 10 76 0
sleep 3.558
noteoff 1 76 0
sleep 12.455
noteoff 3 64 0
sleep 90.747
noteon 10 77 102
sleep 3.558
noteon 1 77 100
sleep 12.455
noteon 3 65 100
sleep 90.747
noteoff 10 77 0
sleep 3.558
noteoff 1 77 0
sleep 12.455
noteoff 3 65 0
sleep 90.747
noteon 10 69 102
sleep 1.628
noteon 0 81 101
sleep 1.628
noteon 1 69 100
noteon 11 64 102
sleep 6.514
noteon 12 61 102
sleep 4.885
noteon 3 57 100
sleep 1.628
noteon 13 57 104
sleep 3.257
noteon 14 45 106
sleep 81.433
noteoff 11 64 0
sleep 6.514
noteoff 12 61 0
sleep 91.505
noteon 11 65 102
sleep 7.117
noteon 12 62 102
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 88.967
noteoff 11 65 0
sleep 7.117
noteoff 12 62 0
sleep 99.644
noteon 11 67 102
sleep 7.117
noteon 12 64 102
sleep 99.644
noteoff 11 67 0
sleep 7.117
noteoff 12 64 0
sleep 60.498
noteoff 10 69 0
sleep 10.676
noteoff 0 81 0
sleep 1.779
noteoff 1 69 0
sleep 12.455
noteoff 3 57 0
sleep 10.676
noteon 10 74 102
sleep 1.628
noteon 0 86 101
sleep 1.628
noteon 1 74 100
noteon 11 65 102
sleep 6.514
noteon 12 62 102
sleep 4.885
noteon 3 62 100
sleep 1.628
noteon 13 50 104
sleep 3.257
noteon 14 38 106
sleep 78.175
noteoff 10 74 0
sleep 1.628
noteoff 0 86 0
sleep 1.628
noteoff 1 74 0
sleep 11.4
noteoff 3 62 0
sleep 83.061
noteon 10 73 102
sleep 1.779
noteon 0 85 101
sleep 1.779
noteoff 11 65 0
noteon 1 73 100
sleep 7.117
noteoff 12 62 0
sleep 5.338
noteon 3 61 100
sleep 1.779
noteoff 13 50 0
sleep 3.558
noteoff 14 38 0
sleep 85.409
noteoff 10 73 0
sleep 1.779
noteoff 0 85 0
sleep 1.779
noteoff 1 73 0
sleep 12.455
noteoff 3 61 0
sleep 90.747
noteon 10 74 102
sleep 1.779
noteon 0 86 101
sleep 1.779
noteon 1 74 100
sleep 12.455
noteon 3 62 100
sleep 90.747
noteoff 10 74 0
sleep 2.164
noteoff 0 86 0
sleep 2.164
noteoff 1 74 0
sleep 15.151
noteoff 3 62 0
sleep 113.563
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
sleep 4.761
noteon 5 50 100
noteon 12 50 102
sleep 4.761
noteon 3 50 100
noteon 3 46 100
sleep 1.587
noteon 13 50 104
sleep 3.174
noteon 14 34 106
sleep 174.970
noteoff 11 62 0
noteon 11 64 102
sleep 7.117
noteoff 12 50 0
noteon 12 52 102
sleep 5.337
noteoff 3 50 0
noteon 3 52 100
sleep 1.779
noteoff 13 50 0
noteon 13 52 104
sleep 199.274
noteoff 11 64 0
noteon 11 65 102
sleep 7.117
noteoff 12 52 0
noteon 12 53 102
sleep 5.337
noteoff 3 52 0
noteon 3 53 100
sleep 1.779
noteoff 13 52 0
noteon 13 53 104
sleep 198.889
noteoff 11 65 0
noteon 11 66 102
sleep 6.349
noteoff 12 53 0
noteon 12 54 102
sleep 4.761
noteoff 3 53 0
noteon 3 54 100
sleep 1.587
noteoff 13 53 0
noteon 13 54 104
sleep 178.144
noteoff 11 66 0
noteon 11 67 102
sleep 7.117
noteoff 12 54 0
noteon 12 55 102
sleep 5.337
noteoff 3 54 0
noteon 3 55 100
sleep 1.779
noteoff 13 54 0
noteon 13 55 104
sleep 199.274
noteoff 11 67 0
noteon 11 68 102
sleep 7.116
noteoff 12 55 0
noteon 12 56 102
sleep 5.337
noteoff 3 55 0
noteon 3 56 100
sleep 1.779
noteoff 13 55 0
noteon 13 56 104
sleep 163.687
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 3.558
noteoff 2 62 0
noteoff 2 74 0
sleep 3.558
noteoff 13 56 0
sleep 3.558
noteoff 14 34 0
sleep 3.558
noteoff 3 56 0
noteoff 3 46 0
sleep 10.674
noteon 10 81 102
noteon 10 69 102
sleep 3.174
noteoff 4 62 0
noteon 11 69 102
noteon 4 57 100
sleep 1.587
noteon 2 61 101
noteon 2 73 101
noteon 6 57 108
noteon 6 69 108
sleep 4.761
noteoff 5 50 0
noteon 12 57 102
noteon 5 45 100
sleep 4.761
noteon 3 57 100
noteon 3 45 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.206
noteoff 12 57 0
sleep 6.349
noteoff 13 57 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 45 0
noteoff 3 57 0
sleep 9.523
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.259
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 12 69 0
sleep 3.558
noteoff 2 73 0
noteoff 2 61 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 69 0
noteoff 6 57 0
sleep 1.779
noteoff 3 57 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 3.174
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 62 101
noteon 2 74 101
noteon 6 62 108
noteon 6 74 108
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 62 100
sleep 1.587
noteon 13 62 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 50 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 92.063
noteon 10 76 102
sleep 3.558
noteoff 4 62 0
noteon 11 64 102
sleep 1.779
noteoff 2 74 0
noteoff 2 62 0
noteoff 6 74 0
noteoff 6 62 0
sleep 5.338
noteoff 5 50 0
noteoff 12 62 0
sleep 5.338
noteoff 3 62 0
sleep 1.779
noteoff 13 62 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 50 0
sleep 85.409
noteoff 10 76 0
sleep 3.558
noteoff 11 64 0
sleep 103.202
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 103.202
noteoff 10 74 0
sleep 3.558
noteoff 11 62 0
sleep 106.376
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 62 101
noteon 2 74 101
sleep 4.761
noteon 5 50 100
noteon 12 50 102
sleep 4.761
noteon 3 50 100
noteon 3 46 100
sleep 1.587
noteon 13 50 104
sleep 3.174
noteon 14 34 106
sleep 174.972
noteoff 11 62 0
noteon 11 64 102
sleep 7.117
noteoff 12 50 0
noteon 12 52 102
sleep 5.337
noteoff 3 50 0
noteon 3 52 100
sleep 1.779
noteoff 13 50 0
noteon 13 52 104
sleep 199.271
noteoff 11 64 0
noteon 11 65 102
sleep 7.116
noteoff 12 52 0
noteon 12 53 102
sleep 5.338
noteoff 3 52 0
noteon 3 53 100
sleep 1.779
noteoff 13 52 0
noteon 13 53 104
sleep 198.887
noteoff 11 65 0
noteon 11 66 102
sleep 6.349
noteoff 12 53 0
noteon 12 54 102
sleep 4.761
noteoff 3 53 0
noteon 3 54 100
sleep 1.587
noteoff 13 53 0
noteon 13 54 104
sleep 178.144
noteoff 11 66 0
noteon 11 67 102
sleep 7.117
noteoff 12 54 0
noteon 12 55 102
sleep 5.337
noteoff 3 54 0
noteon 3 55 100
sleep 1.779
noteoff 13 54 0
noteon 13 55 104
sleep 199.272
noteoff 11 67 0
noteon 11 68 102
sleep 7.116
noteoff 12 55 0
noteon 12 56 102
sleep 5.338
noteoff 3 55 0
noteon 3 56 100
sleep 1.779
noteoff 13 55 0
noteon 13 56 104
sleep 163.688
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 3.558
noteoff 2 74 0
noteoff 2 62 0
sleep 3.558
noteoff 13 56 0
sleep 3.558
noteoff 14 34 0
sleep 3.558
noteoff 3 56 0
noteoff 3 46 0
sleep 10.676
noteon 10 81 102
noteon 10 69 102
sleep 3.174
noteoff 4 62 0
noteon 4 57 100
noteon 11 69 102
sleep 1.587
noteon 2 61 101
noteon 2 73 101
noteon 6 69 108
noteon 6 57 108
sleep 4.761
noteoff 5 50 0
noteon 12 57 102
noteon 5 45 100
sleep 4.761
noteon 3 57 100
noteon 3 45 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.187
noteoff 12 57 0
sleep 6.348
noteoff 13 57 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 45 0
noteoff 3 57 0
sleep 9.522
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.337
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.251
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 12 69 0
sleep 3.558
noteoff 2 73 0
noteoff 2 61 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 57 0
noteoff 6 69 0
sleep 1.779
noteoff 3 57 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 3.174
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 5 50 100
noteon 12 74 102
sleep 4.761
noteon 3 62 100
sleep 1.587
noteon 13 62 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 50 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 79.365
noteoff 4 62 0
sleep 6.349
noteoff 5 50 0
sleep 6.349
noteon 10 74 102
sleep 3.558
noteon 4 66 100
noteon 11 62 102
sleep 1.779
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 12 74 0
noteon 5 54 100
sleep 5.338
noteoff 3 62 0
sleep 1.779
noteoff 13 62 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 50 0
sleep 85.409
noteoff 10 74 0
sleep 3.558
noteoff 11 62 0
sleep 88.967
noteoff 4 66 0
sleep 7.117
noteoff 5 54 0
sleep 7.117
noteon 10 78 102
sleep 3.558
noteon 4 62 100
noteon 11 66 102
sleep 7.117
noteon 5 50 100
sleep 96.085
noteoff 10 78 0
sleep 3.558
noteoff 11 66 0
sleep 88.967
noteoff 4 62 0
sleep 7.117
noteoff 5 50 0
sleep 7.117
noteon 10 81 102
noteon 10 69 102
sleep 1.587
noteon 0 85 101
sleep 1.587
noteon 4 57 100
noteon 11 69 102
sleep 1.587
noteon 2 73 101
noteon 6 57 108
noteon 6 69 108
sleep 4.761
noteon 5 45 100
noteon 12 57 102
sleep 4.761
noteon 3 45 100
noteon 3 61 100
sleep 1.587
noteon 13 45 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.186
noteoff 12 57 0
sleep 6.348
noteoff 13 45 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 45 0
sleep 9.522
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.337
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.251
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 0 85 0
noteoff 12 69 0
sleep 3.558
noteoff 2 73 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 69 0
noteoff 6 57 0
sleep 1.779
noteoff 3 57 0
noteoff 3 61 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 1.587
noteon 0 86 101
sleep 1.587
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 5 50 100
noteon 12 74 102
sleep 4.761
noteon 3 50 100
noteon 3 62 100
sleep 1.587
noteon 13 62 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 50 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 79.365
noteoff 4 62 0
sleep 6.349
noteoff 5 50 0
sleep 6.349
noteon 10 74 102
sleep 1.779
noteoff 0 86 0
sleep 1.779
noteon 4 66 100
noteon 11 62 102
sleep 1.779
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 12 74 0
noteon 5 54 100
sleep 5.338
noteoff 3 62 0
noteoff 3 50 0
sleep 1.779
noteoff 13 62 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 50 0
sleep 85.409
noteoff 10 74 0
sleep 3.558
noteoff 11 62 0
sleep 88.967
noteoff 4 66 0
sleep 7.117
noteoff 5 54 0
sleep 7.117
noteon 10 78 102
sleep 3.558
noteon 4 62 100
noteon 11 66 102
sleep 7.117
noteon 5 50 100
sleep 96.085
noteoff 10 78 0
sleep 3.558
noteoff 11 66 0
sleep 88.967
noteoff 4 62 0
sleep 7.117
noteoff 5 50 0
sleep 7.117
noteon 10 69 102
noteon 10 81 102
sleep 1.587
noteon 0 85 101
noteon 0 81 101
sleep 1.587
noteon 1 69 100
noteon 1 81 100
noteon 4 57 100
noteon 11 61 102
sleep 1.587
noteon 2 69 101
noteon 2 73 101
noteon 6 69 108
noteon 6 57 108
sleep 4.761
noteon 5 45 100
noteon 12 57 102
sleep 4.761
noteon 3 61 100
noteon 3 57 100
sleep 1.587
noteon 13 45 104
sleep 1.587
noteon 15 45 92
sleep 1.587
noteon 14 33 106
sleep 84.122
noteoff 15 45 0
sleep 9.523
noteon 15 45 92
sleep 81.330
noteoff 11 61 0
noteon 11 64 102
sleep 5.338
noteoff 15 45 0
sleep 1.779
noteoff 12 57 0
noteon 12 61 102
sleep 7.117
noteoff 13 45 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 33 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 90.747
noteoff 11 64 0
noteon 11 69 102
sleep 5.338
noteoff 15 45 0
sleep 1.779
noteoff 12 61 0
noteon 12 64 102
sleep 7.117
noteoff 13 57 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 45 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 90.362
noteoff 11 69 0
noteon 11 73 102
sleep 4.761
noteoff 15 45 0
sleep 1.587
noteoff 12 64 0
noteon 12 69 102
sleep 6.349
noteoff 13 57 0
noteon 13 57 104
sleep 1.587
noteon 15 45 92
sleep 1.587
noteoff 14 45 0
noteon 14 45 106
sleep 84.126
noteoff 15 45 0
sleep 9.523
noteon 15 45 92
sleep 65.079
noteoff 4 57 0
sleep 6.349
noteoff 5 45 0
sleep 6.349
noteoff 10 81 0
noteon 10 80 102
sleep 1.779
noteoff 0 81 0
noteoff 0 85 0
noteon 0 86 101
noteon 0 83 101
sleep 1.779
noteoff 1 81 0
noteoff 1 69 0
noteoff 11 73 0
noteon 1 68 100
noteon 1 80 100
noteon 4 64 100
noteon 11 74 102
sleep 1.779
noteoff 2 73 0
noteoff 2 69 0
noteon 2 71 101
noteon 2 74 101
sleep 3.558
noteoff 15 45 0
sleep 1.779
noteoff 12 69 0
noteon 5 57 100
noteon 12 71 102
sleep 5.338
noteoff 3 57 0
noteoff 3 61 0
noteon 3 62 100
noteon 3 59 100
sleep 1.779
noteoff 13 57 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 45 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 72.953
noteoff 4 64 0
sleep 7.117
noteoff 5 57 0
sleep 7.117
noteoff 10 80 0
noteon 10 79 102
sleep 1.779
noteoff 0 83 0
noteoff 0 86 0
noteon 0 88 101
noteon 0 85 101
sleep 1.779
noteoff 1 80 0
noteoff 1 68 0
noteoff 11 74 0
noteon 1 67 100
noteon 1 79 100
noteon 4 64 100
noteon 11 76 102
sleep 1.779
noteoff 2 74 0
noteoff 2 71 0
noteon 2 76 101
noteon 2 73 101
sleep 3.558
noteoff 15 45 0
sleep 1.779
noteoff 12 71 0
noteon 5 57 100
noteon 12 73 102
sleep 5.338
noteoff 3 59 0
noteoff 3 62 0
noteon 3 64 100
noteon 3 61 100
sleep 1.779
noteoff 13 57 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 45 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 51.601
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 76 0
sleep 7.117
noteoff 0 85 0
noteoff 0 88 0
noteoff 12 73 0
sleep 1.779
noteoff 1 79 0
noteoff 1 67 0
sleep 1.779
noteoff 2 73 0
noteoff 2 76 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 64 0
noteoff 14 45 0
sleep 1.779
noteoff 6 57 0
noteoff 6 69 0
sleep 1.779
noteoff 3 61 0
noteoff 3 64 0
sleep 3.558
noteoff 5 57 0
sleep 7.117
noteon 10 78 102
sleep 1.712
noteon 0 90 101
noteon 0 78 101
sleep 1.712
noteon 1 66 100
noteon 1 78 100
noteon 4 62 100
noteon 11 78 102
sleep 1.712
noteon 2 74 101
noteon 2 78 101
noteon 6 74 108
noteon 6 62 108
sleep 3.424
noteoff 15 45 0
sleep 1.712
noteon 5 54 100
noteon 12 74 102
sleep 5.136
noteon 3 66 100
noteon 3 62 100
sleep 1.712
noteon 13 62 104
sleep 1.712
noteon 15 50 109
sleep 1.712
noteon 14 50 106
sleep 184.931
noteoff 10 78 0
sleep 1.953
noteoff 0 78 0
noteoff 0 90 0
sleep 1.953
noteoff 1 78 0
noteoff 1 66 0
noteoff 4 62 0
noteoff 11 78 0
sleep 1.953
noteoff 2 78 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.859
noteoff 5 54 0
noteoff 12 74 0
sleep 5.859
noteoff 3 62 0
noteoff 3 66 0
sleep 1.953
noteoff 13 62 0
sleep 1.953
noteoff 15 50 0
sleep 1.953
noteoff 14 50 0
sleep 210.937
noteon 10 85 102
noteon 10 69 102
sleep 1.953
noteon 0 85 101
sleep 1.953
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
noteon 11 79 102
noteon 11 69 102
sleep 1.953
noteon 2 73 101
noteon 2 76 101
noteon 6 69 108
noteon 6 57 108
sleep 5.859
noteon 5 57 100
noteon 12 69 102
sleep 5.859
noteon 3 57 100
sleep 1.953
noteon 13 57 104
sleep 1.953
noteon 15 45 92
sleep 1.953
noteon 14 45 106
sleep 93.75
noteoff 10 69 0
noteoff 10 85 0
sleep 1.953
noteoff 0 85 0
sleep 1.953
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 11 69 0
noteoff 11 79 0
sleep 1.953
noteoff 2 76 0
noteoff 2 73 0
noteoff 6 57 0
noteoff 6 69 0
sleep 3.906
noteoff 15 45 0
sleep 1.953
noteoff 5 57 0
noteoff 12 69 0
sleep 5.859
noteoff 3 57 0
sleep 1.953
noteoff 13 57 0
sleep 3.906
noteoff 14 45 0
sleep 93.75
noteon 10 86 102
noteon 10 69 102
noteon 10 62 102
sleep 2.38
noteon 0 86 101
sleep 2.38
noteon 1 78 100
noteon 1 74 100
noteon 4 62 100
noteon 11 69 102
noteon 11 62 102
noteon 11 78 102
sleep 2.38
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
sleep 7.142
noteon 5 54 100
noteon 12 62 102
sleep 7.142
noteon 3 50 100
noteon 3 62 100
sleep 2.38
noteon 13 50 104
sleep 2.38
noteon 15 50 92
sleep 2.38
noteon 14 38 106
sleep 114.285
noteoff 10 62 0
noteoff 10 69 0
noteoff 10 86 0
sleep 2.38
noteoff 0 86 0
sleep 2.38
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 62 0
noteoff 11 78 0
noteoff 11 62 0
noteoff 11 69 0
sleep 2.38
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 15 50 0
sleep 2.38
noteoff 5 54 0
noteoff 12 62 0
sleep 7.142
noteoff 3 62 0
noteoff 3 50 0
sleep 2.38
noteoff 13 50 0
sleep 4.761
noteoff 14 38 0
sleep 536.627
noteon 0 74 86
sleep 165.743
noteoff 0 74 0
noteon 0 76 86
sleep 110.494
noteoff 0 76 0
sleep 54.494
noteon 0 74 86
noteon 0 78 86
sleep 16.064
noteon 3 62 85
sleep 720.052
noteoff 0 78 0
noteon 0 79 86
sleep 15.686
noteon 3 59 85
sleep 699.710
noteoff 0 79 0
noteon 0 81 86
sleep 14.981
noteoff 3 59 0
noteon 3 54 85
sleep 209.737
noteoff 0 81 0
noteoff 0 74 0
noteon 0 69 86
noteon 0 79 86
sleep 14.981
noteoff 3 54 0
noteoff 3 62 0
noteon 3 52 85
noteon 3 61 85
sleep 172.284
noteoff 0 79 0
noteoff 0 69 0
sleep 14.981
noteoff 3 61 0
noteoff 3 52 0
sleep 22.639
noteon 0 69 86
noteon 0 78 86
sleep 16.326
noteon 3 50 85
noteon 3 62 85
sleep 106.122
noteoff 0 78 0
noteoff 0 69 0
sleep 16.326
noteoff 3 62 0
noteoff 3 50 0
sleep 105.953
noteon 0 76 86
noteon 0 69 86
sleep 14.981
noteon 3 57 85
noteon 3 61 85
sleep 209.737
noteoff 0 69 0
noteoff 0 76 0
sleep 14.981
noteoff 3 61 0
noteoff 3 57 0
sleep 209.905
noteon 0 78 86
sleep 16.326
noteon 3 54 85
noteon 3 63 85
sleep 187.755
noteoff 0 78 0
sleep 16.326
noteoff 3 63 0
noteoff 3 54 0
sleep 24.320
noteon 0 71 86
noteon 0 79 86
sleep 14.981
noteon 3 52 85
noteon 3 64 85
sleep 679.342
noteoff 0 79 0
noteoff 0 71 0
noteon 0 81 86
noteon 0 73 86
sleep 1.872
noteon 4 64 100
sleep 7.49
noteon 5 57 100
sleep 5.617
noteoff 3 64 0
noteoff 3 52 0
noteon 3 45 85
noteon 3 61 85
sleep 638.663
noteoff 0 73 0
noteoff 0 81 0
sleep 2.04
noteoff 4 64 0
sleep 8.163
noteoff 5 57 0
sleep 6.122
noteoff 3 61 0
noteoff 3 45 0
sleep 24.319
noteon 0 78 86
noteon 0 74 86
sleep 1.872
noteon 4 66 100
sleep 7.49
noteon 5 62 100
sleep 5.617
noteon 3 62 85
noteon 3 50 85
sleep 209.737
noteoff 0 74 0
noteoff 0 78 0
noteon 0 79 86
noteon 0 76 86
sleep 1.872
noteoff 4 66 0
noteon 4 67 100
sleep 7.49
noteoff 5 62 0
noteon 5 64 100
sleep 5.617
noteoff 3 50 0
noteoff 3 62 0
noteon 3 59 85
noteon 3 43 85
sleep 172.284
noteoff 0 76 0
noteoff 0 79 0
sleep 1.872
noteoff 4 67 0
sleep 7.49
noteoff 5 64 0
sleep 5.617
noteoff 3 43 0
noteoff 3 59 0
sleep 22.639
noteon 0 76 86
noteon 0 73 86
sleep 2.04
noteon 4 64 100
sleep 8.163
noteon 5 57 100
sleep 6.122
noteon 3 55 85
noteon 3 45 85
sleep 106.122
noteoff 0 73 0
noteoff 0 76 0
sleep 2.04
noteoff 4 64 0
sleep 8.163
noteoff 5 57 0
sleep 6.122
noteoff 3 45 0
noteoff 3 55 0
sleep 105.953
noteon 0 74 86
sleep 1.872
noteon 4 62 100
sleep 7.49
noteon 5 54 100
sleep 5.617
noteon 3 54 85
noteon 3 50 85
sleep 210.029
noteoff 0 74 0
sleep 2.164
noteoff 4 62 0
sleep 8.658
noteoff 5 54 0
sleep 6.493
noteoff 3 50 0
noteoff 3 54 0
sleep 242.617
noteon 0 74 86
sleep 141.506
noteoff 0 74 0
noteon 0 76 86
sleep 94.336
noteoff 0 76 0
sleep 46.682
noteon 0 74 86
noteon 0 78 86
sleep 14.981
noteon 3 62 85
sleep 679.353
noteoff 0 78 0
noteon 0 79 86
sleep 14.981
noteon 3 59 85
sleep 679.353
noteoff 0 79 0
noteon 0 81 86
sleep 14.981
noteoff 3 59 0
noteon 3 54 85
sleep 209.737
noteoff 0 81 0
noteoff 0 74 0
noteon 0 69 86
noteon 0 79 86
sleep 14.981
noteoff 3 54 0
noteoff 3 62 0
noteon 3 52 85
noteon 3 61 85
sleep 172.284
noteoff 0 79 0
noteoff 0 69 0
sleep 14.981
noteoff 3 61 0
noteoff 3 52 0
sleep 22.639
noteon 0 69 86
noteon 0 78 86
sleep 16.326
noteon 3 62 85
noteon 3 50 85
sleep 106.122
noteoff 0 78 0
noteoff 0 69 0
sleep 16.326
noteoff 3 50 0
noteoff 3 62 0
sleep 105.953
noteon 0 69 86
noteon 0 76 86
sleep 14.981
noteon 3 57 85
noteon 3 61 85
sleep 209.737
noteoff 0 76 0
noteoff 0 69 0
sleep 14.981
noteoff 3 61 0
noteoff 3 57 0
sleep 209.905
noteon 0 78 86
sleep 16.326
noteon 3 54 85
noteon 3 63 85
sleep 187.755
noteoff 0 78 0
sleep 16.326
noteoff 3 63 0
noteoff 3 54 0
sleep 24.320
noteon 0 79 86
noteon 0 71 86
sleep 14.981
noteon 3 64 85
noteon 3 52 85
sleep 679.342
noteoff 0 71 0
noteoff 0 79 0
noteon 0 81 86
noteon 0 73 86
sleep 1.872
noteon 4 64 100
sleep 7.49
noteon 5 57 100
sleep 5.617
noteoff 3 52 0
noteoff 3 64 0
noteon 3 61 85
noteon 3 45 85
sleep 638.663
noteoff 0 73 0
noteoff 0 81 0
sleep 2.04
noteoff 4 64 0
sleep 8.163
noteoff 5 57 0
sleep 6.122
noteoff 3 45 0
noteoff 3 61 0
sleep 24.319
noteon 0 74 86
noteon 0 78 86
sleep 1.872
noteon 4 66 100
sleep 7.49
noteon 5 62 100
sleep 5.617
noteon 3 50 85
noteon 3 62 85
sleep 209.737
noteoff 0 78 0
noteoff 0 74 0
noteon 0 76 86
noteon 0 79 86
sleep 1.872
noteoff 4 66 0
noteon 4 67 100
sleep 7.49
noteoff 5 62 0
noteon 5 64 100
sleep 5.617
noteoff 3 62 0
noteoff 3 50 0
noteon 3 59 85
noteon 3 43 85
sleep 172.284
noteoff 0 79 0
noteoff 0 76 0
sleep 1.872
noteoff 4 67 0
sleep 7.49
noteoff 5 64 0
sleep 5.617
noteoff 3 43 0
noteoff 3 59 0
sleep 22.639
noteon 0 73 86
noteon 0 76 86
sleep 2.04
noteon 4 64 100
sleep 8.163
noteon 5 57 100
sleep 6.122
noteon 3 55 85
noteon 3 45 85
sleep 106.122
noteoff 0 76 0
noteoff 0 73 0
sleep 2.04
noteoff 4 64 0
sleep 8.163
noteoff 5 57 0
sleep 6.122
noteoff 3 45 0
noteoff 3 55 0
sleep 105.953
noteon 0 74 86
sleep 1.872
noteon 4 62 100
sleep 7.49
noteon 5 54 100
sleep 5.617
noteon 3 50 85
noteon 3 54 85
sleep 210.097
noteoff 0 74 0
sleep 2.232
noteoff 4 62 0
sleep 8.928
noteoff 5 54 0
sleep 6.696
noteoff 3 54 0
noteoff 3 50 0
sleep 492.664
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 255.555
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 54 0
sleep 3.703
noteoff 14 42 0
sleep 384.443
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 255.543
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 54 0
sleep 3.703
noteoff 14 42 0
sleep 384.443
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 88.888
noteoff 10 66 0
noteon 10 65 102
sleep 3.703
noteoff 11 66 0
noteon 11 65 102
sleep 7.407
noteoff 12 54 0
noteon 12 53 102
sleep 7.407
noteoff 13 54 0
noteon 13 53 104
sleep 3.703
noteoff 14 42 0
noteon 14 41 106
sleep 51.851
noteoff 10 65 0
sleep 3.703
noteoff 11 65 0
sleep 7.407
noteoff 12 53 0
sleep 7.407
noteoff 13 53 0
sleep 3.703
noteoff 14 41 0
sleep 14.814
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 88.888
noteoff 10 66 0
noteon 10 65 102
sleep 3.703
noteoff 11 66 0
noteon 11 65 102
sleep 7.407
noteoff 12 54 0
noteon 12 53 102
sleep 7.407
noteoff 13 54 0
noteon 13 53 104
sleep 3.703
noteoff 14 42 0
noteon 14 41 106
sleep 51.851
noteoff 10 65 0
sleep 3.703
noteoff 11 65 0
sleep 7.407
noteoff 12 53 0
sleep 7.407
noteoff 13 53 0
sleep 3.703
noteoff 14 41 0
sleep 14.814
noteon 10 66 102
sleep 3.999
noteon 11 66 102
sleep 7.999
noteon 12 54 102
sleep 7.999
noteon 13 54 104
sleep 3.999
noteon 14 42 106
sleep 96.0
noteoff 10 66 0
noteon 10 65 102
sleep 3.999
noteoff 11 66 0
noteon 11 65 102
sleep 7.999
noteoff 12 54 0
noteon 12 53 102
sleep 7.999
noteoff 13 54 0
noteon 13 53 104
sleep 3.999
noteoff 14 42 0
noteon 14 41 106
sleep 55.999
noteoff 10 65 0
sleep 3.999
noteoff 11 65 0
sleep 7.999
noteoff 12 53 0
sleep 7.999
noteoff 13 53 0
sleep 3.999
noteoff 14 41 0
sleep 15.999
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 88.888
noteoff 10 66 0
noteon 10 65 102
sleep 3.703
noteoff 11 66 0
noteon 11 65 102
sleep 7.407
noteoff 12 54 0
noteon 12 53 102
sleep 7.407
noteoff 13 54 0
noteon 13 53 104
sleep 3.703
noteoff 14 42 0
noteon 14 41 106
sleep 51.851
noteoff 10 65 0
sleep 3.703
noteoff 11 65 0
sleep 7.407
noteoff 12 53 0
sleep 7.407
noteoff 13 53 0
sleep 3.703
noteoff 14 41 0
sleep 14.814
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 88.888
noteoff 10 66 0
noteon 10 65 102
sleep 3.703
noteoff 11 66 0
noteon 11 65 102
sleep 7.407
noteoff 12 54 0
noteon 12 53 102
sleep 7.407
noteoff 13 54 0
noteon 13 53 104
sleep 3.703
noteoff 14 42 0
noteon 14 41 106
sleep 51.851
noteoff 10 65 0
sleep 3.703
noteoff 11 65 0
sleep 7.407
noteoff 12 53 0
sleep 7.407
noteoff 13 53 0
sleep 3.703
noteoff 14 41 0
sleep 14.814
noteon 10 66 102
sleep 3.999
noteon 11 66 102
sleep 7.999
noteon 12 54 102
sleep 7.999
noteon 13 54 104
sleep 3.999
noteon 14 42 106
sleep 96.0
noteoff 10 66 0
noteon 10 65 102
sleep 3.999
noteoff 11 66 0
noteon 11 65 102
sleep 7.999
noteoff 12 54 0
noteon 12 53 102
sleep 7.999
noteoff 13 54 0
noteon 13 53 104
sleep 3.999
noteoff 14 42 0
noteon 14 41 106
sleep 55.999
noteoff 10 65 0
sleep 3.999
noteoff 11 65 0
sleep 7.999
noteoff 12 53 0
sleep 7.999
noteoff 13 53 0
sleep 3.999
noteoff 14 41 0
sleep 15.999
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 24.074
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 54 0
sleep 3.703
noteoff 14 42 0
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 24.074
noteon 10 70 102
sleep 3.703
noteon 11 70 102
sleep 7.407
noteon 12 58 102
sleep 7.407
noteon 13 46 104
sleep 3.703
noteon 14 34 106
sleep 61.111
noteoff 10 70 0
sleep 3.703
noteoff 11 70 0
sleep 24.074
noteon 10 70 102
sleep 3.703
noteon 11 70 102
sleep 7.407
noteoff 12 58 0
sleep 7.407
noteoff 13 46 0
sleep 3.703
noteoff 14 34 0
sleep 61.111
noteoff 10 70 0
sleep 3.703
noteoff 11 70 0
sleep 24.074
noteon 10 73 102
sleep 3.999
noteon 11 73 102
sleep 7.999
noteon 12 61 102
sleep 7.999
noteon 13 49 104
sleep 3.999
noteon 14 37 106
sleep 65.999
noteoff 10 73 0
sleep 3.999
noteoff 11 73 0
sleep 26.0
noteon 10 73 102
sleep 3.999
noteon 11 73 102
sleep 7.999
noteoff 12 61 0
sleep 7.999
noteoff 13 49 0
sleep 3.999
noteoff 14 37 0
sleep 65.999
noteoff 10 73 0
sleep 3.999
noteoff 11 73 0
sleep 26.0
noteon 10 78 102
sleep 3.703
noteon 11 78 102
sleep 7.407
noteon 12 66 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 61.107
noteoff 10 78 0
sleep 3.703
noteoff 11 78 0
sleep 24.072
noteon 10 78 102
sleep 3.703
noteon 11 78 102
sleep 7.405
noteoff 12 66 0
sleep 7.407
noteoff 13 54 0
sleep 3.702
noteoff 14 42 0
sleep 61.106
noteoff 10 78 0
sleep 3.703
noteoff 11 78 0
sleep 24.071
noteon 10 73 102
sleep 3.703
noteon 11 73 102
sleep 7.407
noteon 12 61 102
sleep 7.407
noteon 13 49 104
sleep 3.703
noteon 14 37 106
sleep 61.111
noteoff 10 73 0
sleep 3.703
noteoff 11 73 0
sleep 24.074
noteon 10 73 102
sleep 3.703
noteon 11 73 102
sleep 7.407
noteoff 12 61 0
sleep 7.407
noteoff 13 49 0
sleep 3.703
noteoff 14 37 0
sleep 61.111
noteoff 10 73 0
sleep 3.703
noteoff 11 73 0
sleep 24.074
noteon 10 70 102
sleep 3.999
noteon 11 70 102
sleep 7.999
noteon 12 58 102
sleep 7.999
noteon 13 46 104
sleep 3.999
noteon 14 34 106
sleep 65.999
noteoff 10 70 0
sleep 3.999
noteoff 11 70 0
sleep 26.0
noteon 10 70 102
sleep 3.999
noteon 11 70 102
sleep 7.999
noteoff 12 58 0
sleep 7.999
noteoff 13 46 0
sleep 3.999
noteoff 14 34 0
sleep 65.999
noteoff 10 70 0
sleep 3.999
noteoff 11 70 0
sleep 26.0
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 42 104
sleep 3.703
noteon 14 30 106
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 24.074
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 42 0
sleep 3.703
noteoff 14 30 0
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 24.074
noteon 10 70 102
sleep 3.703
noteon 11 70 102
sleep 7.407
noteon 12 58 102
sleep 7.407
noteon 13 46 104
sleep 3.703
noteon 14 34 106
sleep 61.111
noteoff 10 70 0
sleep 3.703
noteoff 11 70 0
sleep 24.074
noteon 10 70 102
sleep 3.703
noteon 11 70 102
sleep 7.407
noteoff 12 58 0
sleep 7.407
noteoff 13 46 0
sleep 3.703
noteoff 14 34 0
sleep 61.111
noteoff 10 70 0
sleep 3.703
noteoff 11 70 0
sleep 24.074
noteon 10 73 102
sleep 3.999
noteon 11 73 102
sleep 7.999
noteon 12 61 102
sleep 7.999
noteon 13 49 104
sleep 3.999
noteon 14 37 106
sleep 65.999
noteoff 10 73 0
sleep 3.999
noteoff 11 73 0
sleep 26.0
noteon 10 73 102
sleep 3.999
noteon 11 73 102
sleep 7.999
noteoff 12 61 0
sleep 7.999
noteoff 13 49 0
sleep 3.999
noteoff 14 37 0
sleep 65.999
noteoff 10 73 0
sleep 3.999
noteoff 11 73 0
sleep 26.0
noteon 10 78 102
sleep 3.703
noteon 11 78 102
sleep 7.407
noteon 12 66 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 61.107
noteoff 10 78 0
sleep 3.703
noteoff 11 78 0
sleep 24.072
noteon 10 78 102
sleep 3.703
noteon 11 78 102
sleep 7.405
noteoff 12 66 0
sleep 7.407
noteoff 13 54 0
sleep 3.702
noteoff 14 42 0
sleep 61.106
noteoff 10 78 0
sleep 3.703
noteoff 11 78 0
sleep 24.071
noteon 10 73 102
sleep 3.703
noteon 11 73 102
sleep 7.407
noteon 12 61 102
sleep 7.407
noteon 13 49 104
sleep 3.703
noteon 14 37 106
sleep 61.111
noteoff 10 73 0
sleep 3.703
noteoff 11 73 0
sleep 24.074
noteon 10 73 102
sleep 3.703
noteon 11 73 102
sleep 7.407
noteoff 12 61 0
sleep 7.407
noteoff 13 49 0
sleep 3.703
noteoff 14 37 0
sleep 61.111
noteoff 10 73 0
sleep 3.703
noteoff 11 73 0
sleep 24.074
noteon 10 70 102
sleep 3.999
noteon 11 70 102
sleep 7.999
noteon 12 58 102
sleep 7.999
noteon 13 46 104
sleep 3.999
noteon 14 34 106
sleep 65.999
noteoff 10 70 0
sleep 3.999
noteoff 11 70 0
sleep 26.0
noteon 10 70 102
sleep 3.999
noteon 11 70 102
sleep 7.999
noteoff 12 58 0
sleep 7.999
noteoff 13 46 0
sleep 3.998
noteoff 14 34 0
sleep 65.991
noteoff 10 70 0
sleep 3.998
noteoff 11 70 0
sleep 25.996
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.404
noteon 12 54 102
sleep 7.406
noteon 13 42 104
sleep 3.703
noteon 14 30 106
sleep 61.097
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.404
noteoff 12 54 0
sleep 7.406
noteoff 13 42 0
sleep 3.703
noteoff 14 30 0
sleep 5.553
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.406
noteon 12 54 102
sleep 7.405
noteon 13 42 104
sleep 3.703
noteon 14 30 106
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 42 0
sleep 3.703
noteoff 14 30 0
sleep 5.555
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 42 104
sleep 3.703
noteon 14 30 106
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 42 0
sleep 3.703
noteoff 14 30 0
sleep 5.555
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.406
noteon 13 42 104
sleep 3.703
noteon 14 30 106
sleep 61.110
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 42 0
sleep 3.703
noteoff 14 30 0
sleep 5.555
noteon 10 66 102
sleep 3.999
noteon 11 66 102
sleep 7.999
noteon 12 54 102
sleep 7.999
noteon 13 42 104
sleep 3.999
noteon 14 30 106
sleep 65.996
noteoff 10 66 0
sleep 3.999
noteoff 11 66 0
sleep 7.999
noteoff 12 54 0
sleep 7.999
noteoff 13 42 0
sleep 3.999
noteoff 14 30 0
sleep 6.0
noteon 10 66 102
sleep 3.999
noteon 11 66 102
sleep 7.999
noteon 12 54 102
sleep 7.999
noteon 13 42 104
sleep 3.999
noteon 14 30 106
sleep 65.995
noteoff 10 66 0
sleep 3.999
noteoff 11 66 0
sleep 7.999
noteoff 12 54 0
sleep 7.999
noteoff 13 42 0
sleep 3.999
noteoff 14 30 0
sleep 6.0
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 61.108
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 54 0
sleep 3.703
noteoff 14 42 0
sleep 5.555
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 61.108
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 54 0
sleep 3.703
noteoff 14 42 0
sleep 5.555
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 61.109
noteoff 10 66 0
sleep 3.702
noteoff 11 66 0
sleep 7.406
noteoff 12 54 0
sleep 7.407
noteoff 13 54 0
sleep 3.703
noteoff 14 42 0
sleep 5.555
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 61.109
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.406
noteoff 12 54 0
sleep 7.405
noteoff 13 54 0
sleep 3.703
noteoff 14 42 0
sleep 5.555
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 54 104
sleep 4.081
noteon 14 42 106
sleep 67.346
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.161
noteoff 12 54 0
sleep 8.162
noteoff 13 54 0
sleep 4.081
noteoff 14 42 0
sleep 6.121
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 54 104
sleep 4.081
noteon 14 42 106
sleep 67.346
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.162
noteoff 13 54 0
sleep 4.080
noteoff 14 42 0
sleep 6.122
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.490
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.797
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.797
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.490
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.796
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.795
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 42 104
sleep 4.081
noteon 14 30 106
sleep 67.344
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.163
noteoff 13 42 0
sleep 4.081
noteoff 14 30 0
sleep 6.122
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 42 104
sleep 4.081
noteon 14 30 106
sleep 67.344
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.163
noteoff 13 42 0
sleep 4.081
noteoff 14 30 0
sleep 6.122
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.794
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.795
noteoff 10 66 0
sleep 3.744
noteoff 11 66 0
sleep 7.489
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.796
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.489
noteoff 12 54 0
sleep 7.489
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.797
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.489
noteoff 12 54 0
sleep 7.489
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 42 104
sleep 4.081
noteon 14 30 106
sleep 67.346
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.162
noteoff 13 42 0
sleep 4.080
noteoff 14 30 0
sleep 6.122
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.162
noteon 12 54 102
sleep 8.163
noteon 13 42 104
sleep 4.081
noteon 14 30 106
sleep 67.346
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.163
noteoff 13 42 0
sleep 4.081
noteoff 14 30 0
sleep 6.122
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 42 104
sleep 4.081
noteon 14 30 106
sleep 1383.654
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.163
noteoff 13 42 0
sleep 4.081
noteoff 14 30 0
sleep 38.605
noteon 0 81 101
sleep 1.872
noteon 1 81 100
noteon 1 69 100
noteon 4 69 100
sleep 1.872
noteon 2 81 101
noteon 2 69 101
noteon 6 69 108
noteon 6 57 108
sleep 5.617
noteon 5 57 100
noteon 15 45 82
sleep 5.617
noteon 3 57 100
noteon 3 45 100
sleep 39.325
noteoff 15 45 0
sleep 18.726
noteon 15 45 75
sleep 44.943
noteoff 15 45 0
sleep 13.108
noteon 15 45 81
sleep 44.943
noteoff 15 45 0
sleep 13.108
noteon 15 45 82
sleep 44.943
noteoff 15 45 0
sleep 11.235
noteon 15 45 83
sleep 44.943
noteoff 15 45 0
sleep 11.235
noteon 15 45 82
sleep 44.943
noteoff 15 45 0
sleep 1.872
noteon 15 45 78
sleep 44.943
noteoff 15 45 0
sleep 22.471
noteon 15 45 76
sleep 46.120
noteoff 15 45 0
sleep 6.122
noteon 15 45 74
sleep 48.979
noteoff 15 45 0
sleep 10.204
noteon 15 45 75
sleep 48.979
noteoff 15 45 0
sleep 6.122
noteon 15 45 80
sleep 48.979
noteoff 15 45 0
sleep 24.489
noteon 15 45 71
sleep 47.969
noteoff 15 45 0
sleep 14.981
noteon 15 45 75
sleep 44.943
noteoff 15 45 0
sleep 11.235
noteon 15 45 72
sleep 44.943
noteoff 15 45 0
sleep 5.617
noteon 15 45 77
sleep 44.943
noteoff 15 45 0
sleep 7.49
noteon 15 45 70
sleep 44.943
noteoff 15 45 0
sleep 22.471
noteon 15 45 70
sleep 44.943
noteoff 15 45 0
sleep 7.49
noteon 15 45 80
sleep 44.943
noteoff 15 45 0
sleep 16.853
noteon 15 45 73
sleep 44.943
noteoff 15 45 0
sleep 11.235
noteon 15 45 73
sleep 26.217
noteon 10 57 102
sleep 4.081
noteon 11 57 102
sleep 8.163
noteon 12 57 102
sleep 8.163
noteoff 15 45 0
noteon 13 45 104
sleep 2.04
noteon 15 45 76
sleep 2.04
noteon 14 33 106
sleep 46.938
noteoff 15 45 0
sleep 4.081
noteon 15 45 77
sleep 48.979
noteoff 15 45 0
sleep 20.408
noteon 15 45 79
sleep 48.979
noteoff 15 45 0
sleep 6.122
noteon 15 45 76
sleep 44.897
noteoff 10 57 0
noteon 10 62 102
sleep 1.872
noteoff 0 81 0
noteon 0 78 101
sleep 1.872
noteoff 1 69 0
noteoff 1 81 0
noteoff 4 69 0
noteoff 15 45 0
noteoff 11 57 0
noteon 1 78 100
noteon 1 74 100
noteon 11 62 102
noteon 4 66 100
sleep 1.872
noteoff 2 69 0
noteoff 2 81 0
noteoff 6 57 0
noteoff 6 69 0
noteon 6 62 108
noteon 2 78 101
noteon 2 66 101
noteon 6 66 108
sleep 5.616
noteoff 5 57 0
noteoff 12 57 0
noteon 5 62 100
noteon 12 62 102
sleep 5.617
noteoff 3 45 0
noteoff 3 57 0
noteon 3 47 100
sleep 1.872
noteoff 13 45 0
noteon 13 50 104
sleep 1.872
noteon 15 50 53
sleep 1.872
noteoff 14 33 0
noteon 14 38 106
sleep 106.717
noteoff 3 47 0
sleep 95.483
noteoff 10 62 0
sleep 1.872
noteoff 0 78 0
sleep 1.872
noteoff 4 66 0
noteoff 11 62 0
sleep 1.872
noteoff 2 66 0
noteoff 2 78 0
noteoff 6 66 0
noteoff 6 62 0
sleep 5.617
noteoff 5 62 0
noteoff 12 62 0
sleep 5.617
noteon 3 62 100
sleep 1.872
noteoff 13 50 0
sleep 1.872
noteoff 15 50 0
sleep 1.872
noteoff 14 38 0
sleep 106.741
noteoff 3 62 0
sleep 113.872
noteon 3 61 100
sleep 122.448
noteoff 3 61 0
sleep 107.826
noteoff 1 78 0
noteon 1 79 100
sleep 13.108
noteon 3 59 100
sleep 112.359
noteoff 3 59 0
sleep 99.25
noteoff 1 74 0
noteon 1 73 100
sleep 13.108
noteon 3 57 100
sleep 112.359
noteoff 3 57 0
sleep 99.586
noteoff 1 73 0
noteon 1 71 100
sleep 14.285
noteon 3 55 100
sleep 122.448
noteoff 3 55 0
sleep 107.826
noteoff 1 71 0
noteoff 1 79 0
noteon 1 69 100
noteon 1 81 100
noteon 4 69 100
sleep 7.49
noteon 5 57 100
sleep 5.617
noteon 3 54 100
sleep 112.359
noteoff 3 54 0
sleep 99.25
noteoff 1 81 0
noteoff 1 69 0
noteoff 4 69 0
noteon 1 79 100
noteon 1 73 100
noteon 4 69 100
sleep 7.49
noteoff 5 57 0
noteon 5 57 100
sleep 5.617
noteon 3 52 100
sleep 112.359
noteoff 3 52 0
sleep 99.586
noteoff 1 73 0
noteoff 1 79 0
noteoff 4 69 0
noteon 1 74 100
noteon 1 78 100
noteon 4 69 100
sleep 8.163
noteoff 5 57 0
noteon 5 57 100
sleep 6.122
noteon 3 50 100
sleep 108.163
noteoff 1 78 0
noteoff 1 74 0
sleep 14.285
noteoff 3 50 0
sleep 107.826
noteoff 4 69 0
noteon 1 76 100
noteon 1 73 100
noteon 4 69 100
sleep 7.49
noteoff 5 57 0
noteon 5 57 100
sleep 5.617
noteon 3 57 100
sleep 211.61
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 69 0
sleep 7.49
noteoff 5 57 0
sleep 5.617
noteoff 3 57 0
sleep 211.946
noteon 1 75 100
noteon 1 78 100
sleep 2.04
noteon 2 69 101
sleep 12.244
noteon 3 54 100
sleep 108.163
noteoff 1 78 0
noteoff 1 75 0
sleep 2.04
noteoff 2 69 0
sleep 12.244
noteoff 3 54 0
sleep 107.826
noteon 1 76 100
noteon 1 79 100
sleep 1.872
noteon 2 71 101
sleep 11.235
noteon 3 52 100
sleep 112.357
noteoff 3 52 0
sleep 112.356
noteon 3 64 100
sleep 112.357
noteoff 3 64 0
sleep 113.868
noteon 3 62 100
sleep 122.445
noteoff 3 62 0
sleep 107.824
noteoff 1 79 0
noteon 4 64 100
noteon 1 81 100
sleep 1.872
noteoff 2 71 0
noteon 2 69 101
sleep 5.617
noteon 5 57 100
sleep 5.617
noteon 3 61 100
sleep 112.350
noteoff 3 61 0
sleep 99.245
noteoff 1 76 0
noteon 1 74 100
sleep 13.108
noteon 3 59 100
sleep 112.353
noteoff 3 59 0
sleep 99.582
noteoff 1 74 0
noteon 1 73 100
sleep 2.04
noteoff 2 69 0
noteon 2 67 101
sleep 12.244
noteon 3 57 100
sleep 122.444
noteoff 3 57 0
sleep 107.822
noteoff 1 73 0
noteoff 1 81 0
noteoff 4 64 0
noteon 1 78 100
noteon 1 74 100
noteon 4 66 100
sleep 1.872
noteoff 2 67 0
noteon 2 66 101
sleep 5.617
noteoff 5 57 0
noteon 5 62 100
sleep 5.617
noteon 3 62 100
sleep 211.61
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 66 0
noteon 1 79 100
noteon 1 76 100
noteon 4 67 100
sleep 1.872
noteoff 2 66 0
noteon 2 71 101
sleep 5.617
noteoff 5 62 0
noteon 5 64 100
sleep 5.617
noteoff 3 62 0
noteon 3 55 100
sleep 211.946
noteoff 1 76 0
noteoff 1 79 0
noteoff 4 67 0
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
sleep 2.04
noteoff 2 71 0
noteon 2 67 101
sleep 6.122
noteoff 5 64 0
noteon 5 57 100
sleep 6.122
noteoff 3 55 0
noteon 3 57 100
sleep 108.163
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
sleep 2.04
noteoff 2 67 0
sleep 6.122
noteoff 5 57 0
sleep 6.122
noteoff 3 57 0
sleep 107.942
noteon 1 74 100
noteon 4 62 100
sleep 1.93
noteon 2 66 101
sleep 5.791
noteon 5 54 100
sleep 5.791
noteon 3 50 100
sleep 218.146
noteoff 1 74 0
noteoff 4 62 0
sleep 1.93
noteoff 2 66 0
sleep 5.791
noteoff 5 54 0
sleep 5.791
noteoff 3 50 0
sleep 214.285
noteon 10 62 87
sleep 140.185
noteoff 10 62 0
noteon 10 64 87
sleep 140.185
noteoff 10 64 0
noteon 10 66 87
sleep 3.745
noteon 4 66 115
noteon 11 62 87
sleep 5.617
select 12 1 0 45
sleep 1.872
noteon 5 50 115
noteon 12 50 24
sleep 5.617
select 13 1 0 45
sleep 1.872
noteon 13 50 26
sleep 1.872
select 14 1 0 45
sleep 1.872
noteon 14 38 28
sleep 213.480
noteoff 12 50 0
noteon 12 62 24
sleep 7.49
noteoff 13 50 0
noteon 13 62 26
sleep 3.745
noteoff 14 38 0
noteon 14 50 28
sleep 214.486
noteoff 12 62 0
noteon 12 61 24
sleep 8.163
noteoff 13 62 0
noteon 13 61 26
sleep 4.081
noteoff 14 50 0
noteon 14 49 28
sleep 220.405
noteoff 10 66 0
noteon 10 67 87
sleep 3.745
noteoff 4 66 0
noteon 4 67 115
sleep 7.49
noteoff 12 61 0
noteon 12 59 24
sleep 7.49
noteoff 13 61 0
noteon 13 59 26
sleep 3.745
noteoff 14 49 0
noteon 14 47 28
sleep 205.988
noteoff 11 62 0
noteon 11 61 87
sleep 7.489
noteoff 12 59 0
noteon 12 57 24
sleep 7.49
noteoff 13 59 0
noteon 13 57 26
sleep 3.745
noteoff 14 47 0
noteon 14 45 28
sleep 206.325
noteoff 11 61 0
noteon 11 59 87
sleep 8.163
noteoff 12 57 0
noteon 12 55 24
sleep 8.163
noteoff 13 57 0
noteon 13 55 26
sleep 4.081
noteoff 14 45 0
noteon 14 43 28
sleep 220.404
noteoff 10 67 0
noteon 10 69 87
sleep 3.744
noteoff 4 67 0
noteoff 11 59 0
noteon 4 69 115
noteon 11 57 87
sleep 7.489
noteoff 5 50 0
noteoff 12 55 0
noteon 5 50 115
noteon 12 54 24
sleep 7.49
noteoff 13 55 0
noteon 13 54 26
sleep 3.745
noteoff 14 43 0
noteon 14 42 28
sleep 202.247
noteoff 10 69 0
noteon 10 67 87
sleep 3.745
noteoff 4 69 0
noteoff 11 57 0
noteon 4 67 115
noteon 11 59 87
sleep 7.49
noteoff 5 50 0
noteoff 12 54 0
noteon 5 49 115
noteon 12 52 24
sleep 7.49
noteoff 13 54 0
noteon 13 52 26
sleep 3.745
noteoff 14 42 0
noteon 14 40 28
sleep 202.247
noteoff 10 67 0
noteon 10 66 87
sleep 4.081
noteoff 4 67 0
noteoff 11 59 0
noteon 4 66 115
noteon 11 62 87
sleep 8.163
noteoff 5 49 0
noteoff 12 52 0
noteon 5 50 115
noteon 12 50 24
sleep 8.163
noteoff 13 52 0
noteon 13 50 26
sleep 4.081
noteoff 14 40 0
noteon 14 38 28
sleep 97.959
noteoff 10 66 0
sleep 4.081
noteoff 4 66 0
noteoff 11 62 0
sleep 8.163
noteoff 5 50 0
sleep 110.204
noteon 10 64 87
sleep 3.745
noteon 4 64 115
noteon 11 61 87
sleep 7.49
noteoff 12 50 0
noteon 5 45 115
noteon 12 57 24
sleep 7.49
noteoff 13 50 0
noteon 13 57 26
sleep 3.745
noteoff 14 38 0
noteon 14 45 28
sleep 202.247
noteoff 10 64 0
sleep 3.745
noteoff 4 64 0
noteoff 11 61 0
sleep 7.49
noteoff 5 45 0
noteoff 12 57 0
sleep 7.49
noteoff 13 57 0
sleep 3.745
noteoff 14 45 0
sleep 202.247
noteon 10 66 87
sleep 4.081
noteon 4 66 115
noteon 11 63 87
sleep 8.163
noteon 12 54 24
sleep 6.122
noteon 3 57 100
sleep 2.04
noteon 13 54 26
sleep 4.081
noteon 14 42 28
sleep 220.408
noteoff 10 66 0
noteon 10 67 87
sleep 3.745
noteoff 4 66 0
noteoff 11 63 0
noteon 4 67 115
noteon 11 64 87
sleep 7.49
noteoff 12 54 0
noteon 12 52 24
sleep 5.617
noteoff 3 57 0
noteon 3 59 100
sleep 1.872
noteoff 13 54 0
noteon 13 52 26
sleep 3.745
noteoff 14 42 0
noteon 14 40 28
sleep 213.474
noteoff 12 52 0
noteon 12 64 29
sleep 7.489
noteoff 13 52 0
noteon 13 64 31
sleep 3.745
noteoff 14 40 0
noteon 14 52 33
sleep 214.483
noteoff 12 64 0
noteon 12 62 36
sleep 8.162
noteoff 13 64 0
noteon 13 62 38
sleep 4.081
noteoff 14 52 0
noteon 14 50 40
sleep 220.401
noteoff 10 67 0
noteon 10 69 87
sleep 3.745
noteoff 4 67 0
noteon 4 69 115
sleep 7.49
noteoff 12 62 0
noteon 5 57 115
noteon 12 61 51
sleep 5.617
noteoff 3 59 0
noteon 3 57 100
sleep 1.872
noteoff 13 62 0
noteon 13 61 53
sleep 3.745
noteoff 14 50 0
noteon 14 49 55
sleep 205.975
noteoff 11 64 0
noteon 11 62 87
sleep 7.489
noteoff 12 61 0
noteon 12 59 42
sleep 7.49
noteoff 13 61 0
noteon 13 59 44
sleep 3.745
noteoff 14 49 0
noteon 14 47 46
sleep 206.314
noteoff 11 62 0
noteon 11 61 87
sleep 8.163
noteoff 12 59 0
noteon 12 57 42
sleep 8.163
noteoff 13 59 0
noteon 13 57 44
sleep 4.081
noteoff 14 47 0
noteon 14 45 46
sleep 220.400
noteoff 10 69 0
noteon 10 66 87
sleep 3.745
noteoff 4 69 0
noteoff 11 61 0
noteon 4 66 115
noteon 11 62 87
sleep 7.49
noteoff 5 57 0
noteoff 12 57 0
noteon 5 62 115
noteon 12 62 24
sleep 5.617
noteoff 3 57 0
noteon 3 57 100
sleep 1.872
noteoff 13 57 0
noteon 13 62 26
sleep 3.745
noteoff 14 45 0
noteon 14 50 28
sleep 202.247
noteoff 10 66 0
noteon 10 67 87
sleep 3.745
noteoff 4 66 0
noteoff 11 62 0
noteon 4 67 115
noteon 11 64 87
sleep 7.49
noteoff 5 62 0
noteoff 12 62 0
noteon 5 64 115
noteon 12 55 24
sleep 5.617
noteoff 3 57 0
noteon 3 59 100
sleep 1.872
noteoff 13 62 0
noteon 13 55 26
sleep 3.745
noteoff 14 50 0
noteon 14 43 28
sleep 202.247
noteoff 10 67 0
noteon 10 64 87
sleep 4.081
noteoff 4 67 0
noteoff 11 64 0
noteon 1 76 100
noteon 1 73 100
noteon 4 64 115
noteon 11 61 87
sleep 8.163
noteoff 5 64 0
noteoff 12 55 0
noteon 5 57 115
noteon 12 57 24
sleep 6.122
noteoff 3 59 0
noteon 3 55 100
sleep 2.04
noteoff 13 55 0
noteon 13 57 26
sleep 4.081
noteoff 14 43 0
noteon 14 45 28
sleep 97.959
noteoff 10 64 0
sleep 4.081
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 11 61 0
sleep 8.163
noteoff 5 57 0
sleep 6.122
noteoff 3 55 0
sleep 104.081
noteon 10 62 87
sleep 3.745
noteon 1 78 100
noteon 1 74 100
noteon 4 62 115
noteon 11 62 87
sleep 7.49
noteoff 12 57 0
noteon 5 54 115
noteon 12 50 24
sleep 5.617
noteon 3 54 100
sleep 1.872
noteoff 13 57 0
noteon 13 50 26
sleep 3.745
noteoff 14 45 0
noteon 14 38 28
sleep 202.247
noteoff 10 62 0
sleep 3.745
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 62 0
noteoff 11 62 0
noteon 1 76 100
noteon 1 79 100
sleep 7.49
noteoff 5 54 0
noteoff 12 50 0
sleep 5.617
noteoff 3 54 0
sleep 1.872
noteoff 13 50 0
sleep 3.745
noteoff 14 38 0
sleep 202.247
noteon 10 64 87
sleep 4.081
noteoff 1 79 0
noteoff 1 76 0
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
noteon 11 61 87
sleep 8.163
noteon 5 57 100
noteon 12 57 24
sleep 6.122
noteon 3 55 100
sleep 2.04
noteon 13 57 26
sleep 4.081
noteon 14 45 28
sleep 97.959
noteoff 10 64 0
sleep 4.081
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 11 61 0
sleep 8.163
noteoff 5 57 0
sleep 6.122
noteoff 3 55 0
sleep 104.081
noteon 10 66 87
sleep 3.703
noteon 1 74 100
noteon 4 66 100
noteon 11 62 87
sleep 7.407
noteoff 12 57 0
noteon 5 62 100
noteon 12 62 24
sleep 5.555
noteon 3 57 100
sleep 1.851
noteoff 13 57 0
noteon 13 62 26
sleep 3.703
noteoff 14 45 0
noteon 14 50 28
sleep 200.0
noteoff 10 66 0
noteon 10 67 87
sleep 3.703
noteoff 1 74 0
noteoff 4 66 0
noteoff 11 62 0
noteon 4 67 100
noteon 11 64 87
sleep 7.407
noteoff 5 62 0
noteoff 12 62 0
noteon 5 64 100
noteon 12 55 24
sleep 5.555
noteoff 3 57 0
noteon 3 59 100
sleep 1.851
noteoff 13 62 0
noteon 13 55 26
sleep 3.703
noteoff 14 50 0
noteon 14 43 28
sleep 200.0
noteoff 10 67 0
noteon 10 64 87
sleep 4.081
noteoff 4 67 0
noteoff 11 64 0
noteon 1 76 100
noteon 1 73 100
noteon 4 64 100
noteon 11 61 87
sleep 8.163
noteoff 5 64 0
noteoff 12 55 0
noteon 5 57 100
noteon 12 57 24
sleep 6.122
noteoff 3 59 0
noteon 3 55 100
sleep 2.04
noteoff 13 55 0
noteon 13 57 26
sleep 4.081
noteoff 14 43 0
noteon 14 45 28
sleep 97.959
noteoff 10 64 0
sleep 4.081
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 11 61 0
sleep 8.163
noteoff 5 57 0
sleep 6.122
noteoff 3 55 0
sleep 104.081
noteon 10 62 87
sleep 3.703
noteon 1 78 100
noteon 1 74 100
noteon 4 62 100
noteon 11 62 87
sleep 7.407
noteoff 12 57 0
noteon 5 54 100
noteon 12 50 24
sleep 5.555
noteon 3 54 100
sleep 1.851
noteoff 13 57 0
noteon 13 50 26
sleep 3.703
noteoff 14 45 0
noteon 14 38 28
sleep 199.998
noteoff 10 62 0
sleep 3.703
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 62 0
noteoff 11 62 0
noteon 1 79 100
noteon 1 76 100
sleep 7.407
noteoff 5 54 0
noteoff 12 50 0
sleep 5.555
noteoff 3 54 0
sleep 1.851
noteoff 13 50 0
sleep 3.703
noteoff 14 38 0
sleep 199.997
noteon 10 64 87
sleep 4.081
noteoff 1 76 0
noteoff 1 79 0
noteon 1 76 100
noteon 1 73 100
noteon 4 64 100
noteon 11 61 87
sleep 8.163
noteon 5 57 100
noteon 12 57 33
sleep 8.163
noteon 13 57 35
sleep 4.081
noteon 14 45 37
sleep 97.957
noteoff 10 64 0
sleep 4.081
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 11 61 0
sleep 8.163
noteoff 5 57 0
sleep 110.204
noteon 10 66 87
sleep 3.663
noteon 1 74 100
noteon 1 78 100
noteon 4 66 100
noteon 11 62 87
sleep 7.326
noteoff 12 57 0
noteon 5 62 100
noteon 12 62 38
sleep 7.326
noteoff 13 57 0
noteon 13 62 40
sleep 3.663
noteoff 14 45 0
noteon 14 50 42
sleep 87.912
noteoff 10 66 0
sleep 3.663
noteoff 11 62 0
sleep 109.890
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
noteon 1 76 100
noteon 1 79 100
noteon 4 67 100
sleep 7.326
noteoff 5 62 0
noteoff 12 62 0
noteon 5 64 100
sleep 7.326
noteoff 13 62 0
sleep 3.663
noteoff 14 50 0
sleep 197.802
noteon 10 64 87
sleep 4.081
noteoff 1 79 0
noteoff 1 76 0
noteoff 4 67 0
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
noteon 11 61 87
sleep 8.163
noteoff 5 64 0
noteon 5 57 100
noteon 12 57 47
sleep 8.163
noteon 13 57 49
sleep 4.081
noteon 14 45 51
sleep 97.957
noteoff 10 64 0
sleep 4.081
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 11 61 0
sleep 8.163
noteoff 5 57 0
sleep 110.204
noteon 10 66 87
sleep 3.745
noteon 1 74 100
noteon 1 78 100
noteon 4 66 100
noteon 11 62 87
sleep 7.49
noteoff 12 57 0
noteon 5 62 100
noteon 12 62 51
sleep 7.49
noteoff 13 57 0
noteon 13 62 53
sleep 3.745
noteoff 14 45 0
noteon 14 50 55
sleep 89.887
noteoff 10 66 0
sleep 3.745
noteoff 11 62 0
sleep 112.359
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
noteon 1 79 100
noteon 1 76 100
noteon 4 67 100
sleep 7.49
noteoff 5 62 0
noteoff 12 62 0
noteon 5 64 100
sleep 7.49
noteoff 13 62 0
sleep 3.745
noteoff 14 50 0
sleep 202.247
noteon 10 73 87
sleep 2.04
noteon 0 88 101
noteon 0 85 101
sleep 2.04
noteoff 1 76 0
noteoff 1 79 0
noteoff 4 67 0
noteon 1 76 100
noteon 1 73 100
noteon 11 64 87
noteon 4 64 100
sleep 2.04
noteon 2 73 101
noteon 2 64 101
sleep 6.122
noteoff 5 64 0
noteon 12 57 24
noteon 5 57 100
sleep 6.122
noteon 3 45 100
noteon 3 57 100
sleep 2.04
noteon 13 57 26
sleep 4.081
noteon 14 45 28
sleep 97.956
noteoff 10 73 0
sleep 2.04
noteoff 0 85 0
noteoff 0 88 0
sleep 2.04
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 11 64 0
sleep 2.04
noteoff 2 64 0
noteoff 2 73 0
sleep 6.122
noteoff 5 57 0
sleep 6.122
noteoff 3 57 0
noteoff 3 45 0
sleep 104.078
noteon 10 74 87
sleep 1.937
noteon 0 86 101
noteon 0 90 101
sleep 1.937
noteon 1 74 100
noteon 1 78 100
noteon 4 66 100
noteon 11 66 87
sleep 1.937
noteon 2 66 101
noteon 2 74 101
sleep 5.813
noteoff 12 57 0
noteon 5 62 100
noteon 12 62 24
sleep 5.813
noteon 3 57 100
noteon 3 50 100
sleep 1.937
noteoff 13 57 0
noteon 13 62 26
sleep 3.875
noteoff 14 45 0
noteon 14 50 28
sleep 209.302
noteoff 10 74 0
noteon 10 76 87
sleep 1.937
noteoff 0 90 0
noteoff 0 86 0
noteon 0 88 101
noteon 0 91 101
sleep 1.937
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
noteoff 11 66 0
noteon 1 79 100
noteon 1 76 100
noteon 4 67 100
noteon 11 67 87
sleep 1.937
noteoff 2 74 0
noteoff 2 66 0
noteon 2 67 101
noteon 2 76 101
sleep 5.813
noteoff 5 62 0
noteoff 12 62 0
noteon 5 64 100
noteon 12 55 24
sleep 5.813
noteoff 3 50 0
noteoff 3 57 0
noteon 3 43 100
noteon 3 59 100
sleep 1.937
noteoff 13 62 0
noteon 13 55 26
sleep 3.875
noteoff 14 50 0
noteon 14 43 28
sleep 209.302
noteoff 10 76 0
noteon 10 73 87
sleep 2.109
noteoff 0 91 0
noteoff 0 88 0
noteon 0 88 101
noteon 0 85 101
sleep 2.109
noteoff 1 76 0
noteoff 1 79 0
noteoff 4 67 0
noteoff 11 67 0
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
noteon 11 64 87
sleep 2.109
noteoff 2 76 0
noteoff 2 67 0
noteon 2 73 101
noteon 2 64 101
sleep 6.329
noteoff 5 64 0
noteoff 12 55 0
noteon 5 57 100
noteon 12 57 24
sleep 6.329
noteoff 3 59 0
noteoff 3 43 0
noteon 3 61 100
noteon 3 45 100
sleep 2.109
noteoff 13 55 0
noteon 13 57 26
sleep 4.219
noteoff 14 43 0
noteon 14 45 28
sleep 101.265
noteoff 10 73 0
sleep 2.109
noteoff 0 85 0
noteoff 0 88 0
sleep 2.109
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 11 64 0
sleep 2.109
noteoff 2 64 0
noteoff 2 73 0
sleep 6.329
noteoff 5 57 0
sleep 6.329
noteoff 3 45 0
noteoff 3 61 0
sleep 107.594
noteon 10 74 87
sleep 1.976
noteon 0 86 101
sleep 1.976
noteon 1 74 100
noteon 4 62 100
noteon 11 66 87
sleep 1.976
noteon 2 66 101
noteon 2 74 101
sleep 5.928
noteoff 12 57 0
noteon 5 54 100
noteon 12 50 24
sleep 5.928
noteon 3 62 100
noteon 3 50 100
sleep 1.976
noteoff 13 57 0
noteon 13 50 26
sleep 3.952
noteoff 14 45 0
noteon 14 38 28
sleep 213.438
noteoff 10 74 0
sleep 1.976
noteoff 0 86 0
sleep 1.976
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 66 0
sleep 1.976
noteoff 2 74 0
noteoff 2 66 0
sleep 5.928
noteoff 5 54 0
noteoff 12 50 0
sleep 5.928
noteoff 3 50 0
noteoff 3 62 0
sleep 1.976
noteoff 13 50 0
sleep 3.952
noteoff 14 38 0
sleep 458.335
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 5.555
select 12 1 0 48
sleep 1.851
noteon 12 54 102
sleep 5.555
select 13 1 0 48
sleep 1.851
noteon 13 54 104
sleep 1.851
select 14 1 0 48
sleep 1.851
noteon 14 42 106
sleep 255.555
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 54 0
sleep 3.703
noteoff 14 42 0
sleep 384.443
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 255.543
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 54 0
sleep 3.703
noteoff 14 42 0
sleep 384.443
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 88.888
noteoff 10 66 0
noteon 10 65 102
sleep 3.703
noteoff 11 66 0
noteon 11 65 102
sleep 7.407
noteoff 12 54 0
noteon 12 53 102
sleep 7.407
noteoff 13 54 0
noteon 13 53 104
sleep 3.703
noteoff 14 42 0
noteon 14 41 106
sleep 51.851
noteoff 10 65 0
sleep 3.703
noteoff 11 65 0
sleep 7.407
noteoff 12 53 0
sleep 7.407
noteoff 13 53 0
sleep 3.703
noteoff 14 41 0
sleep 14.814
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 88.888
noteoff 10 66 0
noteon 10 65 102
sleep 3.703
noteoff 11 66 0
noteon 11 65 102
sleep 7.407
noteoff 12 54 0
noteon 12 53 102
sleep 7.407
noteoff 13 54 0
noteon 13 53 104
sleep 3.703
noteoff 14 42 0
noteon 14 41 106
sleep 51.851
noteoff 10 65 0
sleep 3.703
noteoff 11 65 0
sleep 7.407
noteoff 12 53 0
sleep 7.407
noteoff 13 53 0
sleep 3.703
noteoff 14 41 0
sleep 14.814
noteon 10 66 102
sleep 3.999
noteon 11 66 102
sleep 7.999
noteon 12 54 102
sleep 7.999
noteon 13 54 104
sleep 3.999
noteon 14 42 106
sleep 96.0
noteoff 10 66 0
noteon 10 65 102
sleep 3.999
noteoff 11 66 0
noteon 11 65 102
sleep 7.999
noteoff 12 54 0
noteon 12 53 102
sleep 7.999
noteoff 13 54 0
noteon 13 53 104
sleep 3.999
noteoff 14 42 0
noteon 14 41 106
sleep 55.999
noteoff 10 65 0
sleep 3.999
noteoff 11 65 0
sleep 7.999
noteoff 12 53 0
sleep 7.999
noteoff 13 53 0
sleep 3.999
noteoff 14 41 0
sleep 15.999
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 88.888
noteoff 10 66 0
noteon 10 65 102
sleep 3.703
noteoff 11 66 0
noteon 11 65 102
sleep 7.407
noteoff 12 54 0
noteon 12 53 102
sleep 7.407
noteoff 13 54 0
noteon 13 53 104
sleep 3.703
noteoff 14 42 0
noteon 14 41 106
sleep 51.851
noteoff 10 65 0
sleep 3.703
noteoff 11 65 0
sleep 7.407
noteoff 12 53 0
sleep 7.407
noteoff 13 53 0
sleep 3.703
noteoff 14 41 0
sleep 14.814
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 88.888
noteoff 10 66 0
noteon 10 65 102
sleep 3.703
noteoff 11 66 0
noteon 11 65 102
sleep 7.407
noteoff 12 54 0
noteon 12 53 102
sleep 7.407
noteoff 13 54 0
noteon 13 53 104
sleep 3.703
noteoff 14 42 0
noteon 14 41 106
sleep 51.851
noteoff 10 65 0
sleep 3.703
noteoff 11 65 0
sleep 7.407
noteoff 12 53 0
sleep 7.407
noteoff 13 53 0
sleep 3.703
noteoff 14 41 0
sleep 14.814
noteon 10 66 102
sleep 3.999
noteon 11 66 102
sleep 7.999
noteon 12 54 102
sleep 7.999
noteon 13 54 104
sleep 3.999
noteon 14 42 106
sleep 96.0
noteoff 10 66 0
noteon 10 65 102
sleep 3.999
noteoff 11 66 0
noteon 11 65 102
sleep 7.999
noteoff 12 54 0
noteon 12 53 102
sleep 7.999
noteoff 13 54 0
noteon 13 53 104
sleep 3.999
noteoff 14 42 0
noteon 14 41 106
sleep 55.999
noteoff 10 65 0
sleep 3.999
noteoff 11 65 0
sleep 7.999
noteoff 12 53 0
sleep 7.999
noteoff 13 53 0
sleep 3.999
noteoff 14 41 0
sleep 15.999
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 24.074
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 54 0
sleep 3.703
noteoff 14 42 0
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 24.074
noteon 10 70 102
sleep 3.703
noteon 11 70 102
sleep 7.407
noteon 12 58 102
sleep 7.407
noteon 13 46 104
sleep 3.703
noteon 14 34 106
sleep 61.111
noteoff 10 70 0
sleep 3.703
noteoff 11 70 0
sleep 24.074
noteon 10 70 102
sleep 3.703
noteon 11 70 102
sleep 7.407
noteoff 12 58 0
sleep 7.407
noteoff 13 46 0
sleep 3.703
noteoff 14 34 0
sleep 61.111
noteoff 10 70 0
sleep 3.703
noteoff 11 70 0
sleep 24.074
noteon 10 73 102
sleep 3.999
noteon 11 73 102
sleep 7.999
noteon 12 61 102
sleep 7.999
noteon 13 49 104
sleep 3.999
noteon 14 37 106
sleep 65.999
noteoff 10 73 0
sleep 3.999
noteoff 11 73 0
sleep 26.0
noteon 10 73 102
sleep 3.999
noteon 11 73 102
sleep 7.999
noteoff 12 61 0
sleep 7.999
noteoff 13 49 0
sleep 3.999
noteoff 14 37 0
sleep 65.999
noteoff 10 73 0
sleep 3.999
noteoff 11 73 0
sleep 26.0
noteon 10 78 102
sleep 3.703
noteon 11 78 102
sleep 7.407
noteon 12 66 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 61.107
noteoff 10 78 0
sleep 3.703
noteoff 11 78 0
sleep 24.072
noteon 10 78 102
sleep 3.703
noteon 11 78 102
sleep 7.405
noteoff 12 66 0
sleep 7.407
noteoff 13 54 0
sleep 3.702
noteoff 14 42 0
sleep 61.106
noteoff 10 78 0
sleep 3.703
noteoff 11 78 0
sleep 24.071
noteon 10 73 102
sleep 3.703
noteon 11 73 102
sleep 7.407
noteon 12 61 102
sleep 7.407
noteon 13 49 104
sleep 3.703
noteon 14 37 106
sleep 61.111
noteoff 10 73 0
sleep 3.703
noteoff 11 73 0
sleep 24.074
noteon 10 73 102
sleep 3.703
noteon 11 73 102
sleep 7.407
noteoff 12 61 0
sleep 7.407
noteoff 13 49 0
sleep 3.703
noteoff 14 37 0
sleep 61.111
noteoff 10 73 0
sleep 3.703
noteoff 11 73 0
sleep 24.074
noteon 10 70 102
sleep 3.999
noteon 11 70 102
sleep 7.999
noteon 12 58 102
sleep 7.999
noteon 13 46 104
sleep 3.999
noteon 14 34 106
sleep 65.999
noteoff 10 70 0
sleep 3.999
noteoff 11 70 0
sleep 26.0
noteon 10 70 102
sleep 3.999
noteon 11 70 102
sleep 7.999
noteoff 12 58 0
sleep 7.999
noteoff 13 46 0
sleep 3.999
noteoff 14 34 0
sleep 65.999
noteoff 10 70 0
sleep 3.999
noteoff 11 70 0
sleep 26.0
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 42 104
sleep 3.703
noteon 14 30 106
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 24.074
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 42 0
sleep 3.703
noteoff 14 30 0
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 24.074
noteon 10 70 102
sleep 3.703
noteon 11 70 102
sleep 7.407
noteon 12 58 102
sleep 7.407
noteon 13 46 104
sleep 3.703
noteon 14 34 106
sleep 61.111
noteoff 10 70 0
sleep 3.703
noteoff 11 70 0
sleep 24.074
noteon 10 70 102
sleep 3.703
noteon 11 70 102
sleep 7.407
noteoff 12 58 0
sleep 7.407
noteoff 13 46 0
sleep 3.703
noteoff 14 34 0
sleep 61.111
noteoff 10 70 0
sleep 3.703
noteoff 11 70 0
sleep 24.074
noteon 10 73 102
sleep 3.999
noteon 11 73 102
sleep 7.999
noteon 12 61 102
sleep 7.999
noteon 13 49 104
sleep 3.999
noteon 14 37 106
sleep 65.999
noteoff 10 73 0
sleep 3.999
noteoff 11 73 0
sleep 26.0
noteon 10 73 102
sleep 3.999
noteon 11 73 102
sleep 7.999
noteoff 12 61 0
sleep 7.999
noteoff 13 49 0
sleep 3.999
noteoff 14 37 0
sleep 65.999
noteoff 10 73 0
sleep 3.999
noteoff 11 73 0
sleep 26.0
noteon 10 78 102
sleep 3.703
noteon 11 78 102
sleep 7.407
noteon 12 66 102
sleep 7.407
noteon 13 54 104
sleep 3.703
noteon 14 42 106
sleep 61.107
noteoff 10 78 0
sleep 3.703
noteoff 11 78 0
sleep 24.072
noteon 10 78 102
sleep 3.703
noteon 11 78 102
sleep 7.405
noteoff 12 66 0
sleep 7.407
noteoff 13 54 0
sleep 3.702
noteoff 14 42 0
sleep 61.106
noteoff 10 78 0
sleep 3.703
noteoff 11 78 0
sleep 24.071
noteon 10 73 102
sleep 3.703
noteon 11 73 102
sleep 7.407
noteon 12 61 102
sleep 7.407
noteon 13 49 104
sleep 3.703
noteon 14 37 106
sleep 61.111
noteoff 10 73 0
sleep 3.703
noteoff 11 73 0
sleep 24.074
noteon 10 73 102
sleep 3.703
noteon 11 73 102
sleep 7.407
noteoff 12 61 0
sleep 7.407
noteoff 13 49 0
sleep 3.703
noteoff 14 37 0
sleep 61.111
noteoff 10 73 0
sleep 3.703
noteoff 11 73 0
sleep 24.074
noteon 10 70 102
sleep 3.999
noteon 11 70 102
sleep 7.999
noteon 12 58 102
sleep 7.999
noteon 13 46 104
sleep 3.999
noteon 14 34 106
sleep 65.999
noteoff 10 70 0
sleep 3.999
noteoff 11 70 0
sleep 26.0
noteon 10 70 102
sleep 3.999
noteon 11 70 102
sleep 7.999
noteoff 12 58 0
sleep 7.999
noteoff 13 46 0
sleep 3.998
noteoff 14 34 0
sleep 65.991
noteoff 10 70 0
sleep 3.998
noteoff 11 70 0
sleep 25.996
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.404
noteon 12 54 102
sleep 7.406
noteon 13 42 104
sleep 3.703
noteon 14 30 106
sleep 61.097
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.404
noteoff 12 54 0
sleep 7.406
noteoff 13 42 0
sleep 3.703
noteoff 14 30 0
sleep 5.553
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.406
noteon 12 54 102
sleep 7.405
noteon 13 42 104
sleep 3.703
noteon 14 30 106
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 42 0
sleep 3.703
noteoff 14 30 0
sleep 5.555
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.407
noteon 13 42 104
sleep 3.703
noteon 14 30 106
sleep 61.111
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 42 0
sleep 3.703
noteoff 14 30 0
sleep 5.555
noteon 10 66 102
sleep 3.703
noteon 11 66 102
sleep 7.407
noteon 12 54 102
sleep 7.406
noteon 13 42 104
sleep 3.703
noteon 14 30 106
sleep 61.110
noteoff 10 66 0
sleep 3.703
noteoff 11 66 0
sleep 7.407
noteoff 12 54 0
sleep 7.407
noteoff 13 42 0
sleep 3.703
noteoff 14 30 0
sleep 5.555
noteon 10 66 102
sleep 3.999
noteon 11 66 102
sleep 7.999
noteon 12 54 102
sleep 7.999
noteon 13 42 104
sleep 3.999
noteon 14 30 106
sleep 65.996
noteoff 10 66 0
sleep 3.999
noteoff 11 66 0
sleep 7.999
noteoff 12 54 0
sleep 7.999
noteoff 13 42 0
sleep 3.999
noteoff 14 30 0
sleep 6.0
noteon 10 66 102
sleep 3.999
noteon 11 66 102
sleep 7.999
noteon 12 54 102
sleep 7.999
noteon 13 42 104
sleep 3.999
noteon 14 30 106
sleep 65.995
noteoff 10 66 0
sleep 3.999
noteoff 11 66 0
sleep 7.999
noteoff 12 54 0
sleep 7.999
noteoff 13 42 0
sleep 3.999
noteoff 14 30 0
sleep 6.0
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 54 104
sleep 3.745
noteon 14 42 106
sleep 61.794
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 54 0
sleep 3.745
noteoff 14 42 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 54 104
sleep 3.745
noteon 14 42 106
sleep 61.794
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 54 0
sleep 3.745
noteoff 14 42 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 54 104
sleep 3.745
noteon 14 42 106
sleep 61.795
noteoff 10 66 0
sleep 3.744
noteoff 11 66 0
sleep 7.489
noteoff 12 54 0
sleep 7.49
noteoff 13 54 0
sleep 3.745
noteoff 14 42 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 54 104
sleep 3.745
noteon 14 42 106
sleep 61.796
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.489
noteoff 12 54 0
sleep 7.489
noteoff 13 54 0
sleep 3.745
noteoff 14 42 0
sleep 5.617
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 54 104
sleep 4.081
noteon 14 42 106
sleep 67.346
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.161
noteoff 12 54 0
sleep 8.162
noteoff 13 54 0
sleep 4.081
noteoff 14 42 0
sleep 6.121
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 54 104
sleep 4.081
noteon 14 42 106
sleep 67.346
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.162
noteoff 13 54 0
sleep 4.080
noteoff 14 42 0
sleep 6.122
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.490
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.797
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.797
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.490
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.796
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.795
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 42 104
sleep 4.081
noteon 14 30 106
sleep 67.344
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.163
noteoff 13 42 0
sleep 4.081
noteoff 14 30 0
sleep 6.122
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 42 104
sleep 4.081
noteon 14 30 106
sleep 67.344
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.163
noteoff 13 42 0
sleep 4.081
noteoff 14 30 0
sleep 6.122
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.794
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.49
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.795
noteoff 10 66 0
sleep 3.744
noteoff 11 66 0
sleep 7.489
noteoff 12 54 0
sleep 7.49
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.796
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.489
noteoff 12 54 0
sleep 7.489
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 3.745
noteon 11 66 102
sleep 7.49
noteon 12 54 102
sleep 7.49
noteon 13 42 104
sleep 3.745
noteon 14 30 106
sleep 61.797
noteoff 10 66 0
sleep 3.745
noteoff 11 66 0
sleep 7.489
noteoff 12 54 0
sleep 7.489
noteoff 13 42 0
sleep 3.745
noteoff 14 30 0
sleep 5.617
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 42 104
sleep 4.081
noteon 14 30 106
sleep 67.346
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.162
noteoff 13 42 0
sleep 4.080
noteoff 14 30 0
sleep 6.122
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.162
noteon 12 54 102
sleep 8.163
noteon 13 42 104
sleep 4.081
noteon 14 30 106
sleep 67.346
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.163
noteoff 13 42 0
sleep 4.081
noteoff 14 30 0
sleep 6.122
noteon 10 66 102
sleep 4.081
noteon 11 66 102
sleep 8.163
noteon 12 54 102
sleep 8.163
noteon 13 42 104
sleep 4.081
noteon 14 30 106
sleep 1383.654
noteoff 10 66 0
sleep 4.081
noteoff 11 66 0
sleep 8.163
noteoff 12 54 0
sleep 8.163
noteoff 13 42 0
sleep 4.081
noteoff 14 30 0
sleep 38.605
noteon 0 81 101
sleep 1.872
noteon 1 81 100
noteon 1 69 100
noteon 4 69 100
sleep 1.872
noteon 2 69 101
noteon 6 69 108
noteon 2 81 101
noteon 6 57 108
sleep 5.617
noteon 5 57 100
sleep 5.617
noteon 3 57 100
noteon 3 45 100
sleep 3.745
noteon 15 45 82
sleep 44.943
noteoff 15 45 0
sleep 18.726
noteon 15 45 75
sleep 44.943
noteoff 15 45 0
sleep 13.108
noteon 15 45 81
sleep 44.943
noteoff 15 45 0
sleep 13.108
noteon 15 45 82
sleep 44.943
noteoff 15 45 0
sleep 11.235
noteon 15 45 83
sleep 44.943
noteoff 15 45 0
sleep 11.235
noteon 15 45 82
sleep 44.943
noteoff 15 45 0
sleep 1.872
noteon 15 45 78
sleep 44.943
noteoff 15 45 0
sleep 22.471
noteon 15 45 76
sleep 46.960
noteoff 15 45 0
sleep 6.122
noteon 15 45 74
sleep 48.979
noteoff 15 45 0
sleep 10.204
noteon 15 45 75
sleep 48.979
noteoff 15 45 0
sleep 6.122
noteon 15 45 80
sleep 48.979
noteoff 15 45 0
sleep 24.489
noteon 15 45 71
sleep 47.129
noteoff 15 45 0
sleep 14.981
noteon 15 45 75
sleep 44.943
noteoff 15 45 0
sleep 11.235
noteon 15 45 72
sleep 44.943
noteoff 15 45 0
sleep 5.617
noteon 15 45 77
sleep 44.943
noteoff 15 45 0
sleep 7.49
noteon 15 45 70
sleep 44.943
noteoff 15 45 0
sleep 22.471
noteon 15 45 70
sleep 44.943
noteoff 15 45 0
sleep 7.49
noteon 15 45 80
sleep 44.943
noteoff 15 45 0
sleep 16.853
noteon 15 45 73
sleep 44.943
noteoff 15 45 0
sleep 11.235
noteon 15 45 73
sleep 16.853
noteon 10 57 102
sleep 4.081
noteon 11 57 102
sleep 8.163
noteon 12 57 102
sleep 8.163
noteon 13 45 104
sleep 4.081
noteon 14 33 106
sleep 6.122
noteoff 15 45 0
sleep 2.04
noteon 15 45 76
sleep 48.979
noteoff 15 45 0
sleep 4.081
noteon 15 45 77
sleep 48.979
noteoff 15 45 0
sleep 20.408
noteon 15 45 79
sleep 48.979
noteoff 15 45 0
sleep 6.122
noteon 15 45 76
sleep 34.693
noteoff 10 57 0
noteon 10 62 102
sleep 1.872
noteoff 0 81 0
noteon 0 78 101
sleep 1.872
noteoff 1 69 0
noteoff 1 81 0
noteoff 4 69 0
noteoff 11 57 0
noteon 1 78 100
noteon 1 74 100
noteon 4 66 100
noteon 11 62 102
sleep 1.872
noteoff 2 81 0
noteoff 2 69 0
noteoff 6 57 0
noteoff 6 69 0
noteon 2 66 101
noteon 2 78 101
noteon 6 62 108
noteon 6 66 108
sleep 5.616
noteoff 5 57 0
noteoff 12 57 0
noteon 5 62 100
noteon 12 62 102
sleep 1.872
noteoff 15 45 0
sleep 3.744
noteoff 3 45 0
noteoff 3 57 0
noteon 3 47 100
sleep 1.872
noteoff 13 45 0
noteon 13 50 104
sleep 1.872
noteon 15 50 53
sleep 1.872
noteoff 14 33 0
noteon 14 38 106
sleep 106.717
noteoff 3 47 0
sleep 95.483
noteoff 10 62 0
sleep 1.872
noteoff 0 78 0
sleep 1.872
noteoff 4 66 0
noteoff 11 62 0
sleep 1.872
noteoff 2 78 0
noteoff 2 66 0
noteoff 6 66 0
noteoff 6 62 0
sleep 5.617
noteoff 5 62 0
noteoff 12 62 0
sleep 5.617
noteon 3 62 100
sleep 1.872
noteoff 13 50 0
sleep 1.872
noteoff 15 50 0
sleep 1.872
noteoff 14 38 0
sleep 106.741
noteoff 3 62 0
sleep 113.872
noteon 3 61 100
sleep 122.448
noteoff 3 61 0
sleep 107.826
noteoff 1 78 0
noteon 1 79 100
sleep 13.108
noteon 3 59 100
sleep 112.359
noteoff 3 59 0
sleep 99.25
noteoff 1 74 0
noteon 1 73 100
sleep 13.108
noteon 3 57 100
sleep 112.359
noteoff 3 57 0
sleep 99.586
noteoff 1 73 0
noteon 1 71 100
sleep 14.285
noteon 3 55 100
sleep 122.448
noteoff 3 55 0
sleep 107.826
noteoff 1 71 0
noteoff 1 79 0
noteon 1 69 100
noteon 1 81 100
noteon 4 69 100
sleep 7.49
noteon 5 57 100
sleep 5.617
noteon 3 54 100
sleep 112.359
noteoff 3 54 0
sleep 99.25
noteoff 1 81 0
noteoff 1 69 0
noteoff 4 69 0
noteon 1 79 100
noteon 1 73 100
noteon 4 69 100
sleep 7.49
noteoff 5 57 0
noteon 5 57 100
sleep 5.617
noteon 3 52 100
sleep 112.359
noteoff 3 52 0
sleep 99.586
noteoff 1 73 0
noteoff 1 79 0
noteoff 4 69 0
noteon 1 74 100
noteon 1 78 100
noteon 4 69 100
sleep 8.163
noteoff 5 57 0
noteon 5 57 100
sleep 6.122
noteon 3 50 100
sleep 108.163
noteoff 1 78 0
noteoff 1 74 0
sleep 14.285
noteoff 3 50 0
sleep 107.826
noteoff 4 69 0
noteon 1 76 100
noteon 1 73 100
noteon 4 69 100
sleep 7.49
noteoff 5 57 0
noteon 5 57 100
sleep 5.617
noteon 3 57 100
sleep 211.61
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 69 0
sleep 7.49
noteoff 5 57 0
sleep 5.617
noteoff 3 57 0
sleep 211.946
noteon 1 75 100
noteon 1 78 100
sleep 2.04
noteon 2 69 101
sleep 12.244
noteon 3 54 100
sleep 108.163
noteoff 1 78 0
noteoff 1 75 0
sleep 2.04
noteoff 2 69 0
sleep 12.244
noteoff 3 54 0
sleep 107.826
noteon 1 79 100
noteon 1 76 100
sleep 1.872
noteon 2 71 101
sleep 11.235
noteon 3 52 100
sleep 112.357
noteoff 3 52 0
sleep 112.356
noteon 3 64 100
sleep 112.357
noteoff 3 64 0
sleep 113.868
noteon 3 62 100
sleep 122.445
noteoff 3 62 0
sleep 107.824
noteoff 1 79 0
noteon 4 64 100
noteon 1 81 100
sleep 1.872
noteoff 2 71 0
noteon 2 69 101
sleep 5.617
noteon 5 57 100
sleep 5.617
noteon 3 61 100
sleep 112.350
noteoff 3 61 0
sleep 99.245
noteoff 1 76 0
noteon 1 74 100
sleep 13.108
noteon 3 59 100
sleep 112.353
noteoff 3 59 0
sleep 99.582
noteoff 1 74 0
noteon 1 73 100
sleep 2.04
noteoff 2 69 0
noteon 2 67 101
sleep 12.244
noteon 3 57 100
sleep 122.444
noteoff 3 57 0
sleep 107.822
noteoff 1 73 0
noteoff 1 81 0
noteoff 4 64 0
noteon 1 78 100
noteon 1 74 100
noteon 4 66 100
sleep 1.872
noteoff 2 67 0
noteon 2 66 101
sleep 5.617
noteoff 5 57 0
noteon 5 62 100
sleep 5.617
noteon 3 62 100
sleep 211.61
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 66 0
noteon 1 79 100
noteon 1 76 100
noteon 4 67 100
sleep 1.872
noteoff 2 66 0
noteon 2 71 101
sleep 5.617
noteoff 5 62 0
noteon 5 64 100
sleep 5.617
noteoff 3 62 0
noteon 3 55 100
sleep 211.946
noteoff 1 76 0
noteoff 1 79 0
noteoff 4 67 0
noteon 1 76 100
noteon 1 73 100
noteon 4 64 100
sleep 2.04
noteoff 2 71 0
noteon 2 67 101
sleep 6.122
noteoff 5 64 0
noteon 5 57 100
sleep 6.122
noteoff 3 55 0
noteon 3 57 100
sleep 108.163
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 64 0
sleep 2.04
noteoff 2 67 0
sleep 6.122
noteoff 5 57 0
sleep 6.122
noteoff 3 57 0
sleep 107.942
noteon 1 74 100
noteon 4 62 100
sleep 1.93
noteon 2 66 101
sleep 5.791
noteon 5 54 100
sleep 5.791
noteon 3 50 100
sleep 218.146
noteoff 1 74 0
noteoff 4 62 0
sleep 1.93
noteoff 2 66 0
sleep 5.791
noteoff 5 54 0
sleep 5.791
noteoff 3 50 0
sleep 214.285
noteon 10 62 87
sleep 140.185
noteoff 10 62 0
noteon 10 64 87
sleep 140.185
noteoff 10 64 0
noteon 10 66 87
sleep 3.745
noteon 4 66 115
noteon 11 62 87
sleep 5.617
select 12 1 0 45
sleep 1.872
noteon 5 50 115
noteon 12 50 24
sleep 5.617
select 13 1 0 45
sleep 1.872
noteon 13 50 26
sleep 1.872
select 14 1 0 45
sleep 1.872
noteon 14 38 28
sleep 213.480
noteoff 12 50 0
noteon 12 62 24
sleep 7.49
noteoff 13 50 0
noteon 13 62 26
sleep 3.745
noteoff 14 38 0
noteon 14 50 28
sleep 214.486
noteoff 12 62 0
noteon 12 61 24
sleep 8.163
noteoff 13 62 0
noteon 13 61 26
sleep 4.081
noteoff 14 50 0
noteon 14 49 28
sleep 220.405
noteoff 10 66 0
noteon 10 67 87
sleep 3.745
noteoff 4 66 0
noteon 4 67 115
sleep 7.49
noteoff 12 61 0
noteon 12 59 24
sleep 7.49
noteoff 13 61 0
noteon 13 59 26
sleep 3.745
noteoff 14 49 0
noteon 14 47 28
sleep 205.988
noteoff 11 62 0
noteon 11 61 87
sleep 7.489
noteoff 12 59 0
noteon 12 57 24
sleep 7.49
noteoff 13 59 0
noteon 13 57 26
sleep 3.745
noteoff 14 47 0
noteon 14 45 28
sleep 206.325
noteoff 11 61 0
noteon 11 59 87
sleep 8.163
noteoff 12 57 0
noteon 12 55 24
sleep 8.163
noteoff 13 57 0
noteon 13 55 26
sleep 4.081
noteoff 14 45 0
noteon 14 43 28
sleep 220.404
noteoff 10 67 0
noteon 10 69 87
sleep 3.744
noteoff 4 67 0
noteoff 11 59 0
noteon 4 69 115
noteon 11 57 87
sleep 7.489
noteoff 5 50 0
noteoff 12 55 0
noteon 5 50 115
noteon 12 54 24
sleep 7.49
noteoff 13 55 0
noteon 13 54 26
sleep 3.745
noteoff 14 43 0
noteon 14 42 28
sleep 202.247
noteoff 10 69 0
noteon 10 67 87
sleep 3.745
noteoff 4 69 0
noteoff 11 57 0
noteon 4 67 115
noteon 11 59 87
sleep 7.49
noteoff 5 50 0
noteoff 12 54 0
noteon 5 49 115
noteon 12 52 24
sleep 7.49
noteoff 13 54 0
noteon 13 52 26
sleep 3.745
noteoff 14 42 0
noteon 14 40 28
sleep 202.247
noteoff 10 67 0
noteon 10 66 87
sleep 4.081
noteoff 4 67 0
noteoff 11 59 0
noteon 4 66 115
noteon 11 62 87
sleep 8.163
noteoff 5 49 0
noteoff 12 52 0
noteon 5 50 115
noteon 12 50 24
sleep 8.163
noteoff 13 52 0
noteon 13 50 26
sleep 4.081
noteoff 14 40 0
noteon 14 38 28
sleep 97.959
noteoff 10 66 0
sleep 4.081
noteoff 4 66 0
noteoff 11 62 0
sleep 8.163
noteoff 5 50 0
sleep 110.204
noteon 10 64 87
sleep 3.745
noteon 4 64 115
noteon 11 61 87
sleep 7.49
noteoff 12 50 0
noteon 5 45 115
noteon 12 57 24
sleep 7.49
noteoff 13 50 0
noteon 13 57 26
sleep 3.745
noteoff 14 38 0
noteon 14 45 28
sleep 202.247
noteoff 10 64 0
sleep 3.745
noteoff 4 64 0
noteoff 11 61 0
sleep 7.49
noteoff 5 45 0
noteoff 12 57 0
sleep 7.49
noteoff 13 57 0
sleep 3.745
noteoff 14 45 0
sleep 202.247
noteon 10 66 87
sleep 4.081
noteon 4 66 115
noteon 11 63 87
sleep 8.163
noteon 12 54 24
sleep 6.122
noteon 3 57 100
sleep 2.04
noteon 13 54 26
sleep 4.081
noteon 14 42 28
sleep 220.408
noteoff 10 66 0
noteon 10 67 87
sleep 3.745
noteoff 4 66 0
noteoff 11 63 0
noteon 11 64 87
noteon 4 67 115
sleep 7.49
noteoff 12 54 0
noteon 12 52 24
sleep 5.617
noteoff 3 57 0
noteon 3 59 100
sleep 1.872
noteoff 13 54 0
noteon 13 52 26
sleep 3.745
noteoff 14 42 0
noteon 14 40 28
sleep 213.474
noteoff 12 52 0
noteon 12 64 29
sleep 7.489
noteoff 13 52 0
noteon 13 64 31
sleep 3.745
noteoff 14 40 0
noteon 14 52 33
sleep 214.483
noteoff 12 64 0
noteon 12 62 36
sleep 8.162
noteoff 13 64 0
noteon 13 62 38
sleep 4.081
noteoff 14 52 0
noteon 14 50 40
sleep 220.401
noteoff 10 67 0
noteon 10 69 87
sleep 3.745
noteoff 4 67 0
noteon 4 69 115
sleep 7.49
noteoff 12 62 0
noteon 5 57 115
noteon 12 61 51
sleep 5.617
noteoff 3 59 0
noteon 3 57 100
sleep 1.872
noteoff 13 62 0
noteon 13 61 53
sleep 3.745
noteoff 14 50 0
noteon 14 49 55
sleep 205.975
noteoff 11 64 0
noteon 11 62 87
sleep 7.489
noteoff 12 61 0
noteon 12 59 42
sleep 7.49
noteoff 13 61 0
noteon 13 59 44
sleep 3.745
noteoff 14 49 0
noteon 14 47 46
sleep 206.314
noteoff 11 62 0
noteon 11 61 87
sleep 8.163
noteoff 12 59 0
noteon 12 57 42
sleep 8.163
noteoff 13 59 0
noteon 13 57 44
sleep 4.081
noteoff 14 47 0
noteon 14 45 46
sleep 220.400
noteoff 10 69 0
noteon 10 66 87
sleep 3.745
noteoff 4 69 0
noteoff 11 61 0
noteon 4 66 115
noteon 11 62 87
sleep 7.49
noteoff 5 57 0
noteoff 12 57 0
noteon 5 62 115
noteon 12 62 24
sleep 5.617
noteoff 3 57 0
noteon 3 57 100
sleep 1.872
noteoff 13 57 0
noteon 13 62 26
sleep 3.745
noteoff 14 45 0
noteon 14 50 28
sleep 202.247
noteoff 10 66 0
noteon 10 67 87
sleep 3.745
noteoff 4 66 0
noteoff 11 62 0
noteon 4 67 115
noteon 11 64 87
sleep 7.49
noteoff 5 62 0
noteoff 12 62 0
noteon 5 64 115
noteon 12 55 24
sleep 5.617
noteoff 3 57 0
noteon 3 59 100
sleep 1.872
noteoff 13 62 0
noteon 13 55 26
sleep 3.745
noteoff 14 50 0
noteon 14 43 28
sleep 202.247
noteoff 10 67 0
noteon 10 64 87
sleep 4.081
noteoff 4 67 0
noteoff 11 64 0
noteon 1 73 100
noteon 1 76 100
noteon 4 64 115
noteon 11 61 87
sleep 8.163
noteoff 5 64 0
noteoff 12 55 0
noteon 5 57 115
noteon 12 57 24
sleep 6.122
noteoff 3 59 0
noteon 3 55 100
sleep 2.04
noteoff 13 55 0
noteon 13 57 26
sleep 4.081
noteoff 14 43 0
noteon 14 45 28
sleep 97.959
noteoff 10 64 0
sleep 4.081
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 11 61 0
sleep 8.163
noteoff 5 57 0
sleep 6.122
noteoff 3 55 0
sleep 104.081
noteon 10 62 87
sleep 3.745
noteon 1 74 100
noteon 1 78 100
noteon 4 62 115
noteon 11 62 87
sleep 7.49
noteoff 12 57 0
noteon 5 54 115
noteon 12 50 24
sleep 5.617
noteon 3 54 100
sleep 1.872
noteoff 13 57 0
noteon 13 50 26
sleep 3.745
noteoff 14 45 0
noteon 14 38 28
sleep 202.247
noteoff 10 62 0
sleep 3.745
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 62 0
noteon 1 79 100
noteon 1 76 100
sleep 7.49
noteoff 5 54 0
noteoff 12 50 0
sleep 5.617
noteoff 3 54 0
sleep 1.872
noteoff 13 50 0
sleep 3.745
noteoff 14 38 0
sleep 202.247
noteon 10 64 87
sleep 4.081
noteoff 1 76 0
noteoff 1 79 0
noteon 1 76 100
noteon 1 73 100
noteon 4 64 100
noteon 11 61 87
sleep 8.163
noteon 5 57 100
noteon 12 57 24
sleep 6.122
noteon 3 55 100
sleep 2.04
noteon 13 57 26
sleep 4.081
noteon 14 45 28
sleep 97.959
noteoff 10 64 0
sleep 4.081
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 11 61 0
sleep 8.163
noteoff 5 57 0
sleep 6.122
noteoff 3 55 0
sleep 104.081
noteon 10 66 87
sleep 3.703
noteon 1 74 100
noteon 4 66 100
noteon 11 62 87
sleep 7.407
noteoff 12 57 0
noteon 5 62 100
noteon 12 62 24
sleep 5.555
noteon 3 57 100
sleep 1.851
noteoff 13 57 0
noteon 13 62 26
sleep 3.703
noteoff 14 45 0
noteon 14 50 28
sleep 200.0
noteoff 10 66 0
noteon 10 67 87
sleep 3.703
noteoff 1 74 0
noteoff 4 66 0
noteoff 11 62 0
noteon 4 67 100
noteon 11 64 87
sleep 7.407
noteoff 5 62 0
noteoff 12 62 0
noteon 5 64 100
noteon 12 55 24
sleep 5.555
noteoff 3 57 0
noteon 3 59 100
sleep 1.851
noteoff 13 62 0
noteon 13 55 26
sleep 3.703
noteoff 14 50 0
noteon 14 43 28
sleep 200.0
noteoff 10 67 0
noteon 10 64 87
sleep 4.081
noteoff 4 67 0
noteoff 11 64 0
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
noteon 11 61 87
sleep 8.163
noteoff 5 64 0
noteoff 12 55 0
noteon 5 57 100
noteon 12 57 24
sleep 6.122
noteoff 3 59 0
noteon 3 55 100
sleep 2.04
noteoff 13 55 0
noteon 13 57 26
sleep 4.081
noteoff 14 43 0
noteon 14 45 28
sleep 97.959
noteoff 10 64 0
sleep 4.081
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 11 61 0
sleep 8.163
noteoff 5 57 0
sleep 6.122
noteoff 3 55 0
sleep 104.081
noteon 10 62 87
sleep 3.703
noteon 1 74 100
noteon 1 78 100
noteon 4 62 100
noteon 11 62 87
sleep 7.407
noteoff 12 57 0
noteon 5 54 100
noteon 12 50 24
sleep 5.555
noteon 3 54 100
sleep 1.851
noteoff 13 57 0
noteon 13 50 26
sleep 3.703
noteoff 14 45 0
noteon 14 38 28
sleep 199.998
noteoff 10 62 0
sleep 3.703
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 62 0
noteon 1 79 100
noteon 1 76 100
sleep 7.407
noteoff 5 54 0
noteoff 12 50 0
sleep 5.555
noteoff 3 54 0
sleep 1.851
noteoff 13 50 0
sleep 3.703
noteoff 14 38 0
sleep 199.997
noteon 10 64 87
sleep 4.081
noteoff 1 76 0
noteoff 1 79 0
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
noteon 11 61 87
sleep 8.163
noteon 5 57 100
noteon 12 57 33
sleep 8.163
noteon 13 57 35
sleep 4.081
noteon 14 45 37
sleep 97.957
noteoff 10 64 0
sleep 4.081
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 11 61 0
sleep 8.163
noteoff 5 57 0
sleep 110.204
noteon 10 66 87
sleep 3.663
noteon 1 78 100
noteon 1 74 100
noteon 4 66 100
noteon 11 62 87
sleep 7.326
noteoff 12 57 0
noteon 5 62 100
noteon 12 62 38
sleep 7.326
noteoff 13 57 0
noteon 13 62 40
sleep 3.663
noteoff 14 45 0
noteon 14 50 42
sleep 87.912
noteoff 10 66 0
sleep 3.663
noteoff 11 62 0
sleep 109.890
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 66 0
noteon 1 76 100
noteon 1 79 100
noteon 4 67 100
sleep 7.326
noteoff 5 62 0
noteoff 12 62 0
noteon 5 64 100
sleep 7.326
noteoff 13 62 0
sleep 3.663
noteoff 14 50 0
sleep 197.802
noteon 10 64 87
sleep 4.081
noteoff 1 79 0
noteoff 1 76 0
noteoff 4 67 0
noteon 1 76 100
noteon 1 73 100
noteon 4 64 100
noteon 11 61 87
sleep 8.163
noteoff 5 64 0
noteon 5 57 100
noteon 12 57 47
sleep 8.163
noteon 13 57 49
sleep 4.081
noteon 14 45 51
sleep 97.957
noteoff 10 64 0
sleep 4.081
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 11 61 0
sleep 8.163
noteoff 5 57 0
sleep 110.204
noteon 10 66 87
sleep 3.745
noteon 1 74 100
noteon 4 66 100
noteon 11 62 87
noteon 1 78 100
sleep 7.49
noteoff 12 57 0
noteon 5 62 100
noteon 12 62 51
sleep 7.49
noteoff 13 57 0
noteon 13 62 53
sleep 3.745
noteoff 14 45 0
noteon 14 50 55
sleep 89.887
noteoff 10 66 0
sleep 3.745
noteoff 11 62 0
sleep 112.359
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
noteon 1 79 100
noteon 1 76 100
noteon 4 67 100
sleep 7.49
noteoff 5 62 0
noteoff 12 62 0
noteon 5 64 100
sleep 7.49
noteoff 13 62 0
sleep 3.745
noteoff 14 50 0
sleep 202.247
noteon 10 73 87
sleep 2.04
noteon 0 85 101
noteon 0 88 101
sleep 2.04
noteoff 1 76 0
noteoff 1 79 0
noteoff 4 67 0
noteon 4 64 100
noteon 11 64 87
noteon 1 73 100
noteon 1 76 100
sleep 2.04
noteon 2 73 101
noteon 2 64 101
sleep 6.122
noteoff 5 64 0
noteon 5 57 100
noteon 12 57 24
sleep 6.122
noteon 3 45 100
noteon 3 57 100
sleep 2.04
noteon 13 57 26
sleep 4.081
noteon 14 45 28
sleep 97.956
noteoff 10 73 0
sleep 2.04
noteoff 0 88 0
noteoff 0 85 0
sleep 2.04
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 11 64 0
sleep 2.04
noteoff 2 64 0
noteoff 2 73 0
sleep 6.122
noteoff 5 57 0
sleep 6.122
noteoff 3 57 0
noteoff 3 45 0
sleep 104.078
noteon 10 74 87
sleep 1.937
noteon 0 90 101
noteon 0 86 101
sleep 1.937
noteon 1 78 100
noteon 1 74 100
noteon 4 66 100
noteon 11 66 87
sleep 1.937
noteon 2 66 101
noteon 2 74 101
sleep 5.813
noteoff 12 57 0
noteon 5 62 100
noteon 12 62 24
sleep 5.813
noteon 3 57 100
noteon 3 50 100
sleep 1.937
noteoff 13 57 0
noteon 13 62 26
sleep 3.875
noteoff 14 45 0
noteon 14 50 28
sleep 209.302
noteoff 10 74 0
noteon 10 76 87
sleep 1.937
noteoff 0 86 0
noteoff 0 90 0
noteon 0 88 101
noteon 0 91 101
sleep 1.937
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 66 0
noteoff 11 66 0
noteon 1 76 100
noteon 1 79 100
noteon 4 67 100
noteon 11 67 87
sleep 1.937
noteoff 2 74 0
noteoff 2 66 0
noteon 2 76 101
noteon 2 67 101
sleep 5.813
noteoff 5 62 0
noteoff 12 62 0
noteon 5 64 100
noteon 12 55 24
sleep 5.813
noteoff 3 50 0
noteoff 3 57 0
noteon 3 43 100
noteon 3 59 100
sleep 1.937
noteoff 13 62 0
noteon 13 55 26
sleep 3.875
noteoff 14 50 0
noteon 14 43 28
sleep 209.302
noteoff 10 76 0
noteon 10 73 87
sleep 2.109
noteoff 0 91 0
noteoff 0 88 0
noteon 0 88 101
noteon 0 85 101
sleep 2.109
noteoff 1 79 0
noteoff 1 76 0
noteoff 4 67 0
noteoff 11 67 0
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
noteon 11 64 87
sleep 2.109
noteoff 2 67 0
noteoff 2 76 0
noteon 2 64 101
noteon 2 73 101
sleep 6.329
noteoff 5 64 0
noteoff 12 55 0
noteon 5 57 100
noteon 12 57 24
sleep 6.329
noteoff 3 59 0
noteoff 3 43 0
noteon 3 61 100
noteon 3 45 100
sleep 2.109
noteoff 13 55 0
noteon 13 57 26
sleep 4.219
noteoff 14 43 0
noteon 14 45 28
sleep 101.265
noteoff 10 73 0
sleep 2.109
noteoff 0 85 0
noteoff 0 88 0
sleep 2.109
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 11 64 0
sleep 2.109
noteoff 2 73 0
noteoff 2 64 0
sleep 6.329
noteoff 5 57 0
sleep 6.329
noteoff 3 45 0
noteoff 3 61 0
sleep 107.594
noteon 10 74 87
sleep 1.976
noteon 0 86 101
sleep 1.976
noteon 1 74 100
noteon 4 62 100
noteon 11 66 87
sleep 1.976
noteon 2 66 101
noteon 2 74 101
sleep 5.928
noteoff 12 57 0
noteon 5 54 100
noteon 12 50 24
sleep 5.928
noteon 3 62 100
noteon 3 50 100
sleep 1.976
noteoff 13 57 0
noteon 13 50 26
sleep 3.952
noteoff 14 45 0
noteon 14 38 28
sleep 213.438
noteoff 10 74 0
sleep 1.976
noteoff 0 86 0
sleep 1.976
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 66 0
sleep 1.976
noteoff 2 74 0
noteoff 2 66 0
sleep 5.928
noteoff 5 54 0
noteoff 12 50 0
sleep 5.928
noteoff 3 50 0
noteoff 3 62 0
sleep 1.976
noteoff 13 50 0
sleep 3.952
noteoff 14 38 0
sleep 530.898
noteon 10 62 102
sleep 1.7
noteon 0 74 95
sleep 1.7
noteon 1 74 94
noteon 4 62 94
noteon 11 62 102
sleep 1.7
noteon 2 62 95
noteon 6 62 102
noteon 6 74 102
noteon 2 74 95
sleep 3.401
select 12 1 0 48
sleep 1.7
noteon 5 50 94
noteon 12 62 102
sleep 5.102
select 13 1 0 48
noteon 3 50 100
sleep 1.7
noteon 13 50 104
sleep 1.7
select 14 1 0 48
noteon 15 50 100
sleep 1.7
noteon 14 38 106
sleep 91.828
noteoff 12 62 0
sleep 5.102
noteoff 3 50 0
sleep 1.7
noteoff 13 50 0
sleep 3.400
noteoff 14 38 0
sleep 92.299
noteon 12 64 102
sleep 5.338
noteon 3 52 100
sleep 1.779
noteon 13 52 104
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteon 14 40 106
sleep 96.077
noteoff 12 64 0
sleep 5.338
noteoff 3 52 0
sleep 1.779
noteoff 13 52 0
sleep 3.558
noteoff 14 40 0
sleep 96.078
noteon 12 66 102
sleep 5.338
noteon 3 54 100
sleep 1.779
noteon 13 54 104
sleep 3.558
noteon 14 42 106
sleep 96.077
noteoff 12 66 0
sleep 5.338
noteoff 3 54 0
sleep 1.779
noteoff 13 54 0
sleep 3.558
noteoff 14 42 0
sleep 58.713
noteoff 10 62 0
sleep 3.558
noteoff 11 62 0
sleep 23.130
noteon 10 78 102
sleep 1.655
noteoff 0 74 0
sleep 1.655
noteoff 1 74 0
noteoff 4 62 0
sleep 1.655
noteoff 2 74 0
noteoff 2 62 0
noteoff 6 74 0
noteoff 6 62 0
sleep 4.966
noteoff 5 50 0
sleep 74.503
noteoff 10 78 0
sleep 114.238
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 55 102
noteon 10 64 102
sleep 1.587
noteon 0 74 95
noteon 0 76 95
sleep 1.587
noteon 1 76 94
noteon 1 74 94
noteon 4 62 94
noteon 11 62 102
noteon 11 59 102
sleep 1.587
noteon 2 76 95
noteon 2 64 95
noteon 6 62 102
noteon 6 74 102
sleep 4.761
noteon 5 50 94
noteon 12 55 102
sleep 4.761
noteon 3 55 100
sleep 1.587
noteon 13 55 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 43 106
sleep 85.706
noteoff 12 55 0
sleep 4.761
noteoff 3 55 0
sleep 1.587
noteoff 13 55 0
sleep 3.174
noteoff 14 43 0
sleep 86.248
noteon 12 57 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 57 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 59 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 59 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 55.358
noteoff 10 64 0
noteoff 10 55 0
sleep 3.354
noteoff 11 59 0
noteoff 11 62 0
sleep 21.808
noteon 10 79 102
sleep 1.587
noteoff 0 76 0
noteoff 0 74 0
sleep 1.587
noteoff 1 74 0
noteoff 1 76 0
noteoff 4 62 0
sleep 1.587
noteoff 2 64 0
noteoff 2 76 0
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 79 0
sleep 109.523
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 122.775
noteon 10 83 102
sleep 90.747
noteoff 10 83 0
sleep 125.949
noteon 4 64 94
sleep 6.349
noteon 5 57 94
sleep 88.888
noteoff 4 64 0
sleep 6.349
noteoff 5 57 0
sleep 89.272
noteon 4 66 94
sleep 7.117
noteon 5 62 94
sleep 99.644
noteoff 4 66 0
sleep 7.117
noteoff 5 62 0
sleep 99.644
noteon 4 67 94
sleep 7.117
noteon 5 64 94
sleep 99.644
noteoff 4 67 0
sleep 7.117
noteoff 5 64 0
sleep 96.085
noteon 10 76 102
sleep 3.174
noteon 11 69 102
sleep 77.777
noteoff 10 76 0
sleep 3.174
noteoff 11 69 0
sleep 106.349
noteon 10 78 102
sleep 3.558
noteon 11 74 102
sleep 87.188
noteoff 10 78 0
sleep 3.558
noteoff 11 74 0
sleep 119.217
noteon 10 79 102
sleep 3.558
noteon 11 76 102
sleep 87.188
noteoff 10 79 0
sleep 3.558
noteoff 11 76 0
sleep 122.391
noteon 1 74 94
noteon 4 66 94
noteon 1 78 94
sleep 6.349
noteon 5 62 94
sleep 88.888
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
sleep 6.349
noteoff 5 62 0
sleep 89.272
noteon 1 76 94
noteon 1 69 94
noteon 4 64 94
sleep 7.117
noteon 5 57 94
sleep 99.644
noteoff 1 69 0
noteoff 1 76 0
noteoff 4 64 0
sleep 7.117
noteoff 5 57 0
sleep 99.644
noteon 1 74 94
noteon 1 66 94
noteon 4 62 94
sleep 7.117
noteon 5 54 94
sleep 99.644
noteoff 1 66 0
noteoff 1 74 0
noteoff 4 62 0
sleep 7.117
noteoff 5 54 0
sleep 96.085
noteon 10 86 102
sleep 1.587
noteon 0 86 101
noteon 0 78 101
sleep 1.587
noteon 11 69 102
noteon 11 78 102
sleep 1.587
noteon 2 74 101
noteon 2 66 101
noteon 6 74 102
noteon 6 62 102
sleep 4.761
noteon 12 62 102
sleep 4.761
noteon 3 50 100
noteon 3 62 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 112
sleep 1.587
noteon 14 38 106
sleep 76.19
noteoff 10 86 0
sleep 1.587
noteoff 0 78 0
noteoff 0 86 0
sleep 1.587
noteoff 11 78 0
noteoff 11 69 0
sleep 1.587
noteoff 2 66 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 12 62 0
sleep 4.761
noteoff 3 62 0
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 1.587
noteoff 15 50 0
sleep 1.587
noteoff 14 38 0
sleep 76.19
noteon 10 85 102
sleep 1.779
noteon 0 85 101
noteon 0 79 101
sleep 1.779
noteon 11 79 102
noteon 11 69 102
sleep 1.779
noteon 2 67 101
noteon 2 76 101
noteon 6 57 102
noteon 6 69 102
sleep 5.338
noteon 12 57 102
sleep 5.338
noteon 3 57 100
noteon 3 45 100
sleep 1.779
noteon 13 45 104
sleep 1.779
noteon 15 45 112
sleep 1.779
noteon 14 33 106
sleep 85.409
noteoff 10 85 0
sleep 1.779
noteoff 0 79 0
noteoff 0 85 0
sleep 1.779
noteoff 11 69 0
noteoff 11 79 0
sleep 1.779
noteoff 2 76 0
noteoff 2 67 0
noteoff 6 69 0
noteoff 6 57 0
sleep 5.338
noteoff 12 57 0
sleep 5.338
noteoff 3 45 0
noteoff 3 57 0
sleep 1.779
noteoff 13 45 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 33 0
sleep 85.409
noteon 10 86 102
sleep 1.779
noteon 0 78 101
noteon 0 86 101
sleep 1.779
noteon 11 69 102
noteon 11 78 102
sleep 1.779
noteon 2 66 101
noteon 2 74 101
noteon 6 62 102
noteon 6 74 102
sleep 5.338
noteon 12 62 102
sleep 5.338
noteon 3 62 100
noteon 3 50 100
sleep 1.779
noteon 13 50 104
sleep 1.779
noteon 15 50 112
sleep 1.779
noteon 14 38 106
sleep 85.409
noteoff 10 86 0
sleep 1.779
noteoff 0 86 0
noteoff 0 78 0
sleep 1.779
noteoff 11 78 0
noteoff 11 69 0
sleep 1.779
noteoff 2 74 0
noteoff 2 66 0
noteoff 6 74 0
noteoff 6 62 0
sleep 5.338
noteoff 12 62 0
sleep 5.338
noteoff 3 50 0
noteoff 3 62 0
sleep 1.779
noteoff 13 50 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 38 0
sleep 85.409
noteon 10 62 102
sleep 1.587
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 4 62 94
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 6 74 102
noteon 6 62 102
noteon 2 62 101
sleep 4.761
noteon 5 50 94
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 55.358
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 21.808
noteon 10 78 102
sleep 1.587
noteoff 0 74 0
sleep 1.587
noteoff 1 74 0
noteoff 4 62 0
sleep 1.587
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 64 102
sleep 1.587
noteon 0 74 101
noteon 0 76 101
sleep 1.587
noteon 1 74 100
noteon 1 76 100
noteon 4 62 94
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 2 64 101
noteon 6 74 102
noteon 6 62 102
sleep 4.761
noteon 5 50 94
noteon 12 56 102
sleep 4.761
noteon 3 56 100
sleep 1.587
noteon 13 56 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 44 106
sleep 85.706
noteoff 12 56 0
sleep 4.761
noteoff 3 56 0
sleep 1.587
noteoff 13 56 0
sleep 3.174
noteoff 14 44 0
sleep 86.248
noteon 12 57 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 57 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 59 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 59 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 55.358
noteoff 10 64 0
sleep 3.354
noteoff 11 62 0
sleep 21.808
noteon 10 83 102
sleep 1.587
noteoff 0 76 0
noteoff 0 74 0
sleep 1.587
noteoff 1 76 0
noteoff 1 74 0
noteoff 4 62 0
sleep 1.587
noteoff 2 64 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 83 0
sleep 109.523
noteon 10 85 102
sleep 90.747
noteoff 10 85 0
sleep 122.775
noteon 10 86 102
sleep 90.747
noteoff 10 86 0
sleep 125.949
noteon 1 73 100
noteon 11 61 102
noteon 1 69 100
sleep 6.349
noteon 12 57 102
sleep 74.603
noteoff 11 61 0
sleep 6.349
noteoff 12 57 0
sleep 7.936
noteoff 1 69 0
noteoff 1 73 0
sleep 95.621
noteon 1 71 100
noteon 1 74 100
noteon 11 62 102
sleep 7.117
noteon 12 59 102
sleep 83.629
noteoff 11 62 0
sleep 7.117
noteoff 12 59 0
sleep 8.896
noteoff 1 74 0
noteoff 1 71 0
sleep 106.761
noteon 1 73 100
noteon 1 76 100
noteon 11 64 102
sleep 7.117
noteon 12 61 102
sleep 83.629
noteoff 11 64 0
sleep 7.117
noteoff 12 61 0
sleep 8.896
noteoff 1 76 0
noteoff 1 73 0
sleep 103.202
noteon 10 83 102
sleep 3.174
noteon 11 76 102
sleep 77.777
noteoff 10 83 0
sleep 3.174
noteoff 11 76 0
sleep 106.349
noteon 10 85 102
sleep 3.558
noteon 11 81 102
sleep 87.188
noteoff 10 85 0
sleep 3.558
noteoff 11 81 0
sleep 119.217
noteon 10 86 102
sleep 3.558
noteon 11 83 102
sleep 87.188
noteoff 10 86 0
sleep 3.558
noteoff 11 83 0
sleep 120.804
noteon 0 85 101
sleep 1.587
noteon 1 81 100
noteon 4 64 94
noteon 1 73 100
sleep 11.111
noteon 3 57 100
sleep 82.539
noteoff 0 85 0
sleep 1.587
noteoff 1 73 0
noteoff 1 81 0
noteoff 4 64 0
sleep 11.111
noteoff 3 57 0
sleep 82.731
noteon 0 83 101
sleep 1.779
noteon 1 71 100
noteon 1 80 100
noteon 4 64 94
sleep 12.455
noteon 3 52 100
sleep 92.526
noteoff 0 83 0
sleep 1.779
noteoff 1 80 0
noteoff 1 71 0
noteoff 4 64 0
sleep 12.455
noteoff 3 52 0
sleep 92.526
noteon 0 81 101
sleep 1.779
noteon 1 73 100
noteon 1 81 100
noteon 4 64 94
sleep 12.455
noteon 3 57 100
sleep 92.526
noteoff 0 81 0
sleep 1.779
noteoff 1 81 0
noteoff 1 73 0
noteoff 4 64 0
sleep 12.455
noteoff 3 57 0
sleep 90.747
noteon 10 81 102
sleep 1.587
noteon 0 85 101
noteon 0 88 101
sleep 1.587
noteon 4 64 94
noteon 11 64 102
noteon 11 73 102
sleep 1.587
noteon 2 73 101
noteon 2 69 101
noteon 6 76 102
sleep 4.761
noteon 5 64 94
sleep 4.761
noteon 3 45 100
noteon 3 57 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 112
sleep 1.587
noteon 14 45 106
sleep 76.19
noteoff 10 81 0
sleep 1.587
noteoff 0 88 0
noteoff 0 85 0
sleep 1.587
noteoff 4 64 0
noteoff 11 73 0
noteoff 11 64 0
sleep 1.587
noteoff 2 69 0
noteoff 2 73 0
noteoff 6 76 0
sleep 4.761
noteoff 5 64 0
sleep 4.761
noteoff 3 57 0
noteoff 3 45 0
sleep 1.587
noteoff 13 57 0
sleep 1.587
noteoff 15 45 0
sleep 1.587
noteoff 14 45 0
sleep 76.19
noteon 10 80 102
sleep 1.779
noteon 0 83 101
noteon 0 88 101
sleep 1.779
noteon 4 64 94
noteon 11 64 102
noteon 11 74 102
sleep 1.779
noteon 2 71 101
noteon 2 68 101
noteon 6 76 102
sleep 5.338
noteon 5 64 94
sleep 5.338
noteon 3 52 100
noteon 3 40 100
sleep 1.779
noteon 13 52 104
sleep 1.779
noteon 15 45 112
sleep 1.779
noteon 14 40 106
sleep 85.409
noteoff 10 80 0
sleep 1.779
noteoff 0 88 0
noteoff 0 83 0
sleep 1.779
noteoff 4 64 0
noteoff 11 74 0
noteoff 11 64 0
sleep 1.779
noteoff 2 68 0
noteoff 2 71 0
noteoff 6 76 0
sleep 5.338
noteoff 5 64 0
sleep 5.338
noteoff 3 40 0
noteoff 3 52 0
sleep 1.779
noteoff 13 52 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 40 0
sleep 85.409
noteon 10 81 102
sleep 1.779
noteon 0 88 101
noteon 0 85 101
sleep 1.779
noteon 4 64 94
noteon 11 73 102
noteon 11 64 102
sleep 1.779
noteon 2 69 101
noteon 2 73 101
noteon 6 76 102
sleep 5.338
noteon 5 64 94
sleep 5.338
noteon 3 57 100
noteon 3 45 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteon 15 45 112
sleep 1.779
noteon 14 45 106
sleep 85.409
noteoff 10 81 0
sleep 1.779
noteoff 0 85 0
noteoff 0 88 0
sleep 1.779
noteoff 4 64 0
noteoff 11 64 0
noteoff 11 73 0
sleep 1.779
noteoff 2 73 0
noteoff 2 69 0
noteoff 6 76 0
sleep 5.338
noteoff 5 64 0
sleep 5.338
noteoff 3 45 0
noteoff 3 57 0
sleep 1.779
noteoff 13 57 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 45 0
sleep 85.409
noteon 10 62 102
sleep 1.587
noteon 0 74 95
sleep 1.587
noteon 1 74 94
noteon 4 62 94
noteon 11 62 102
sleep 1.587
noteon 2 74 95
noteon 6 74 102
noteon 6 62 102
noteon 2 62 95
sleep 4.761
noteon 5 50 94
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 55.358
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 21.808
noteon 10 78 102
sleep 1.587
noteoff 0 74 0
sleep 1.587
noteoff 1 74 0
noteoff 4 62 0
sleep 1.587
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 55 102
noteon 10 64 102
sleep 1.587
noteon 0 76 95
noteon 0 74 95
sleep 1.587
noteon 1 74 94
noteon 1 76 94
noteon 4 62 94
noteon 11 59 102
noteon 11 62 102
sleep 1.587
noteon 2 64 95
noteon 6 62 102
noteon 6 74 102
noteon 2 76 95
sleep 4.761
noteon 5 50 94
noteon 12 55 102
sleep 4.761
noteon 3 55 100
sleep 1.587
noteon 13 55 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 43 106
sleep 85.706
noteoff 12 55 0
sleep 4.761
noteoff 3 55 0
sleep 1.587
noteoff 13 55 0
sleep 3.174
noteoff 14 43 0
sleep 86.248
noteon 12 57 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 57 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 59 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 59 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 55.358
noteoff 10 64 0
noteoff 10 55 0
sleep 3.354
noteoff 11 62 0
noteoff 11 59 0
sleep 21.808
noteon 10 79 102
sleep 1.587
noteoff 0 74 0
noteoff 0 76 0
sleep 1.587
noteoff 1 76 0
noteoff 1 74 0
noteoff 4 62 0
sleep 1.587
noteoff 2 76 0
noteoff 2 64 0
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 79 0
sleep 109.523
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 122.775
noteon 10 83 102
sleep 90.747
noteoff 10 83 0
sleep 125.949
noteon 4 64 94
sleep 6.349
noteon 5 57 94
sleep 88.888
noteoff 4 64 0
sleep 6.349
noteoff 5 57 0
sleep 89.272
noteon 4 66 94
sleep 7.117
noteon 5 62 94
sleep 99.644
noteoff 4 66 0
sleep 7.117
noteoff 5 62 0
sleep 99.644
noteon 4 67 94
sleep 7.117
noteon 5 64 94
sleep 99.644
noteoff 4 67 0
sleep 7.117
noteoff 5 64 0
sleep 96.085
noteon 10 76 102
sleep 3.174
noteon 11 69 102
sleep 77.777
noteoff 10 76 0
sleep 3.174
noteoff 11 69 0
sleep 106.349
noteon 10 78 102
sleep 3.558
noteon 11 74 102
sleep 87.188
noteoff 10 78 0
sleep 3.558
noteoff 11 74 0
sleep 119.217
noteon 10 79 102
sleep 3.558
noteon 11 76 102
sleep 87.188
noteoff 10 79 0
sleep 3.558
noteoff 11 76 0
sleep 122.391
noteon 1 74 94
noteon 1 78 94
noteon 4 66 94
sleep 6.349
noteon 5 62 94
sleep 88.888
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
sleep 6.349
noteoff 5 62 0
sleep 89.272
noteon 1 69 94
noteon 1 76 94
noteon 4 64 94
sleep 7.117
noteon 5 57 94
sleep 99.644
noteoff 1 76 0
noteoff 1 69 0
noteoff 4 64 0
sleep 7.117
noteoff 5 57 0
sleep 99.644
noteon 1 66 94
noteon 1 74 94
noteon 4 62 94
sleep 7.117
noteon 5 54 94
sleep 99.644
noteoff 1 74 0
noteoff 1 66 0
noteoff 4 62 0
sleep 7.117
noteoff 5 54 0
sleep 96.085
noteon 10 86 102
sleep 1.587
noteon 0 86 101
noteon 0 78 101
sleep 1.587
noteon 11 78 102
noteon 11 69 102
sleep 1.587
noteon 2 66 101
noteon 2 74 101
noteon 6 74 102
noteon 6 62 102
sleep 4.761
noteon 12 62 102
sleep 4.761
noteon 3 62 100
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 112
sleep 1.587
noteon 14 38 106
sleep 76.19
noteoff 10 86 0
sleep 1.587
noteoff 0 78 0
noteoff 0 86 0
sleep 1.587
noteoff 11 69 0
noteoff 11 78 0
sleep 1.587
noteoff 2 74 0
noteoff 2 66 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
noteoff 3 62 0
sleep 1.587
noteoff 13 50 0
sleep 1.587
noteoff 15 50 0
sleep 1.587
noteoff 14 38 0
sleep 76.19
noteon 10 85 102
sleep 1.779
noteon 0 85 101
noteon 0 79 101
sleep 1.779
noteon 11 79 102
noteon 11 69 102
sleep 1.779
noteon 2 67 101
noteon 2 76 101
noteon 6 57 102
noteon 6 69 102
sleep 5.338
noteon 12 57 102
sleep 5.338
noteon 3 57 100
noteon 3 45 100
sleep 1.779
noteon 13 45 104
sleep 1.779
noteon 15 45 112
sleep 1.779
noteon 14 33 106
sleep 85.409
noteoff 10 85 0
sleep 1.779
noteoff 0 79 0
noteoff 0 85 0
sleep 1.779
noteoff 11 69 0
noteoff 11 79 0
sleep 1.779
noteoff 2 76 0
noteoff 2 67 0
noteoff 6 69 0
noteoff 6 57 0
sleep 5.338
noteoff 12 57 0
sleep 5.338
noteoff 3 45 0
noteoff 3 57 0
sleep 1.779
noteoff 13 45 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 33 0
sleep 85.409
noteon 10 86 102
sleep 1.779
noteon 0 86 101
noteon 0 78 101
sleep 1.779
noteon 11 78 102
noteon 11 69 102
sleep 1.779
noteon 2 66 101
noteon 2 74 101
noteon 6 74 102
noteon 6 62 102
sleep 5.338
noteon 12 62 102
sleep 5.338
noteon 3 62 100
noteon 3 50 100
sleep 1.779
noteon 13 50 104
sleep 1.779
noteon 15 50 112
sleep 1.779
noteon 14 38 106
sleep 85.409
noteoff 10 86 0
sleep 1.779
noteoff 0 78 0
noteoff 0 86 0
sleep 1.779
noteoff 11 69 0
noteoff 11 78 0
sleep 1.779
noteoff 2 74 0
noteoff 2 66 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 12 62 0
sleep 5.338
noteoff 3 50 0
noteoff 3 62 0
sleep 1.779
noteoff 13 50 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 38 0
sleep 85.409
noteon 10 62 102
sleep 1.587
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 4 62 94
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
noteon 6 74 102
noteon 6 62 102
sleep 4.761
noteon 5 50 94
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 55.358
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 21.808
noteon 10 78 102
sleep 1.587
noteoff 0 74 0
sleep 1.587
noteoff 1 74 0
noteoff 4 62 0
sleep 1.587
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 64 102
sleep 1.587
noteon 0 74 101
noteon 0 76 101
sleep 1.587
noteon 1 76 100
noteon 1 74 100
noteon 4 62 94
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 6 62 102
noteon 6 74 102
noteon 2 64 101
sleep 4.761
noteon 5 50 94
noteon 12 56 102
sleep 4.761
noteon 3 56 100
sleep 1.587
noteon 13 56 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 44 106
sleep 85.706
noteoff 12 56 0
sleep 4.761
noteoff 3 56 0
sleep 1.587
noteoff 13 56 0
sleep 3.174
noteoff 14 44 0
sleep 86.248
noteon 12 57 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 57 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 59 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 59 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 55.358
noteoff 10 64 0
sleep 3.354
noteoff 11 62 0
sleep 21.808
noteon 10 83 102
sleep 1.587
noteoff 0 76 0
noteoff 0 74 0
sleep 1.587
noteoff 1 74 0
noteoff 1 76 0
noteoff 4 62 0
sleep 1.587
noteoff 2 64 0
noteoff 2 74 0
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 83 0
sleep 109.523
noteon 10 85 102
sleep 90.747
noteoff 10 85 0
sleep 122.775
noteon 10 86 102
sleep 90.747
noteoff 10 86 0
sleep 125.949
noteon 1 69 100
noteon 1 73 100
noteon 11 61 102
sleep 6.349
noteon 12 57 102
sleep 74.603
noteoff 11 61 0
sleep 6.349
noteoff 12 57 0
sleep 7.936
noteoff 1 73 0
noteoff 1 69 0
sleep 95.621
noteon 1 71 100
noteon 1 74 100
noteon 11 62 102
sleep 7.117
noteon 12 59 102
sleep 83.629
noteoff 11 62 0
sleep 7.117
noteoff 12 59 0
sleep 8.896
noteoff 1 74 0
noteoff 1 71 0
sleep 106.761
noteon 1 73 100
noteon 1 76 100
noteon 11 64 102
sleep 7.117
noteon 12 61 102
sleep 83.629
noteoff 11 64 0
sleep 7.117
noteoff 12 61 0
sleep 8.896
noteoff 1 76 0
noteoff 1 73 0
sleep 103.202
noteon 10 83 102
sleep 3.174
noteon 11 76 102
sleep 77.777
noteoff 10 83 0
sleep 3.174
noteoff 11 76 0
sleep 106.349
noteon 10 85 102
sleep 3.558
noteon 11 81 102
sleep 87.188
noteoff 10 85 0
sleep 3.558
noteoff 11 81 0
sleep 119.217
noteon 10 86 102
sleep 3.558
noteon 11 83 102
sleep 87.188
noteoff 10 86 0
sleep 3.558
noteoff 11 83 0
sleep 120.804
noteon 0 85 101
sleep 1.587
noteon 1 81 100
noteon 1 73 100
noteon 4 64 94
sleep 11.111
noteon 3 57 100
sleep 82.539
noteoff 0 85 0
sleep 1.587
noteoff 1 73 0
noteoff 1 81 0
noteoff 4 64 0
sleep 11.111
noteoff 3 57 0
sleep 82.731
noteon 0 83 101
sleep 1.779
noteon 1 80 100
noteon 1 71 100
noteon 4 64 94
sleep 12.455
noteon 3 52 100
sleep 92.526
noteoff 0 83 0
sleep 1.779
noteoff 1 71 0
noteoff 1 80 0
noteoff 4 64 0
sleep 12.455
noteoff 3 52 0
sleep 92.526
noteon 0 81 101
sleep 1.779
noteon 1 73 100
noteon 1 81 100
noteon 4 64 94
sleep 12.455
noteon 3 57 100
sleep 92.526
noteoff 0 81 0
sleep 1.779
noteoff 1 81 0
noteoff 1 73 0
noteoff 4 64 0
sleep 12.455
noteoff 3 57 0
sleep 90.747
noteon 10 81 102
sleep 1.587
noteon 0 85 101
noteon 0 88 101
sleep 1.587
noteon 4 64 94
noteon 11 64 102
noteon 11 73 102
sleep 1.587
noteon 2 69 101
noteon 6 76 102
noteon 2 73 101
sleep 4.761
noteon 5 64 94
sleep 4.761
noteon 3 57 100
noteon 3 45 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 112
sleep 1.587
noteon 14 45 106
sleep 76.19
noteoff 10 81 0
sleep 1.587
noteoff 0 88 0
noteoff 0 85 0
sleep 1.587
noteoff 4 64 0
noteoff 11 73 0
noteoff 11 64 0
sleep 1.587
noteoff 2 73 0
noteoff 2 69 0
noteoff 6 76 0
sleep 4.761
noteoff 5 64 0
sleep 4.761
noteoff 3 45 0
noteoff 3 57 0
sleep 1.587
noteoff 13 57 0
sleep 1.587
noteoff 15 45 0
sleep 1.587
noteoff 14 45 0
sleep 76.19
noteon 10 80 102
sleep 1.779
noteon 0 83 101
noteon 0 88 101
sleep 1.779
noteon 4 64 94
noteon 11 74 102
noteon 11 64 102
sleep 1.779
noteon 2 68 101
noteon 2 71 101
noteon 6 76 102
sleep 5.338
noteon 5 64 94
sleep 5.338
noteon 3 40 100
noteon 3 52 100
sleep 1.779
noteon 13 52 104
sleep 1.779
noteon 15 45 112
sleep 1.779
noteon 14 40 106
sleep 85.409
noteoff 10 80 0
sleep 1.779
noteoff 0 88 0
noteoff 0 83 0
sleep 1.779
noteoff 4 64 0
noteoff 11 64 0
noteoff 11 74 0
sleep 1.779
noteoff 2 71 0
noteoff 2 68 0
noteoff 6 76 0
sleep 5.338
noteoff 5 64 0
sleep 5.338
noteoff 3 52 0
noteoff 3 40 0
sleep 1.779
noteoff 13 52 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 40 0
sleep 85.409
noteon 10 81 102
sleep 2.066
noteon 0 88 101
noteon 0 85 101
sleep 2.066
noteon 4 64 94
noteon 11 73 102
noteon 11 64 102
sleep 2.066
noteon 2 73 101
noteon 2 69 101
noteon 6 76 102
sleep 6.198
noteon 5 64 94
sleep 6.198
noteon 3 45 100
noteon 3 57 100
sleep 2.066
noteon 13 57 104
sleep 2.066
noteon 15 45 112
sleep 2.066
noteon 14 45 106
sleep 99.173
noteoff 10 81 0
sleep 2.066
noteoff 0 85 0
noteoff 0 88 0
sleep 2.066
noteoff 4 64 0
noteoff 11 64 0
noteoff 11 73 0
sleep 2.066
noteoff 2 69 0
noteoff 2 73 0
noteoff 6 76 0
sleep 6.198
noteoff 5 64 0
sleep 6.198
noteoff 3 57 0
noteoff 3 45 0
sleep 2.066
noteoff 13 57 0
sleep 2.066
noteoff 15 45 0
sleep 2.066
noteoff 14 45 0
sleep 99.173
noteon 10 69 102
sleep 3.322
noteon 11 61 102
sleep 6.644
noteon 12 57 102
sleep 6.644
noteon 13 57 104
sleep 3.322
noteon 14 45 106
sleep 83.056
noteoff 11 61 0
sleep 6.644
noteoff 12 57 0
sleep 6.644
noteoff 13 57 0
sleep 3.322
noteoff 14 45 0
sleep 83.292
noteon 11 62 102
sleep 7.117
noteon 12 59 102
sleep 7.117
noteon 13 59 104
sleep 3.558
noteon 14 47 106
sleep 88.967
noteoff 11 62 0
sleep 7.117
noteoff 12 59 0
sleep 7.117
noteoff 13 59 0
sleep 3.558
noteoff 14 47 0
sleep 88.967
noteon 11 64 102
sleep 7.117
noteon 12 61 102
sleep 7.117
noteon 13 61 104
sleep 3.558
noteon 14 49 106
sleep 88.967
noteoff 11 64 0
sleep 7.117
noteoff 12 61 0
sleep 7.117
noteoff 13 61 0
sleep 3.558
noteoff 14 49 0
sleep 49.822
noteoff 10 69 0
sleep 35.587
noteon 10 77 102
sleep 3.322
noteon 11 65 102
sleep 6.644
noteon 12 62 102
sleep 6.644
noteon 13 62 104
sleep 3.322
noteon 14 50 106
sleep 64.784
noteoff 10 77 0
sleep 114.617
noteon 10 76 102
sleep 3.558
noteoff 11 65 0
sleep 7.117
noteoff 12 62 0
sleep 7.117
noteoff 13 62 0
sleep 3.558
noteoff 14 50 0
sleep 69.395
noteoff 10 76 0
sleep 122.775
noteon 10 77 102
sleep 90.747
noteoff 10 77 0
sleep 122.775
noteon 10 65 102
sleep 3.322
noteon 11 57 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 53 104
sleep 3.322
noteon 14 41 106
sleep 83.056
noteoff 11 57 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 53 0
sleep 3.322
noteoff 14 41 0
sleep 83.292
noteon 11 58 102
sleep 7.117
noteon 12 55 102
sleep 7.117
noteon 13 55 104
sleep 3.558
noteon 14 43 106
sleep 88.967
noteoff 11 58 0
sleep 7.117
noteoff 12 55 0
sleep 7.117
noteoff 13 55 0
sleep 3.558
noteoff 14 43 0
sleep 88.967
noteon 11 60 102
sleep 7.117
noteon 12 57 102
sleep 7.117
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 88.967
noteoff 11 60 0
sleep 7.117
noteoff 12 57 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 49.822
noteoff 10 65 0
sleep 35.587
noteon 10 74 102
sleep 3.322
noteon 11 62 102
sleep 6.644
noteon 12 58 102
sleep 6.644
noteon 13 58 104
sleep 3.322
noteon 14 46 106
sleep 64.784
noteoff 10 74 0
sleep 114.617
noteon 10 73 102
sleep 3.558
noteoff 11 62 0
sleep 7.117
noteoff 12 58 0
sleep 7.117
noteoff 13 58 0
sleep 3.558
noteoff 14 46 0
sleep 69.395
noteoff 10 73 0
sleep 122.775
noteon 10 74 102
sleep 90.747
noteoff 10 74 0
sleep 122.775
noteon 10 58 102
sleep 3.322
noteon 11 62 102
noteon 11 58 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 58 0
sleep 3.322
noteoff 11 58 0
noteoff 11 62 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 79.734
noteon 10 77 102
sleep 3.558
noteon 11 58 102
noteon 11 62 102
sleep 7.117
noteon 12 53 102
sleep 7.116
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 782.931
noteoff 10 77 0
sleep 35.587
noteon 10 74 102
sleep 106.761
noteoff 10 74 0
noteon 10 70 102
sleep 74.733
noteoff 11 62 0
noteoff 11 58 0
sleep 32.028
noteoff 10 70 0
noteon 10 69 102
sleep 3.322
noteon 11 60 102
noteon 11 63 102
sleep 96.345
noteoff 10 69 0
noteon 10 72 102
sleep 69.767
noteoff 11 63 0
noteoff 11 60 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 72 0
noteon 10 75 102
sleep 3.558
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 75 0
noteon 10 74 102
sleep 74.733
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 74 0
noteon 10 72 102
sleep 3.558
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 72 0
noteon 10 70 102
sleep 71.174
noteoff 10 70 0
sleep 3.558
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteon 10 69 102
sleep 3.322
noteon 11 60 102
noteon 11 63 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 69 0
noteon 10 67 102
sleep 69.767
noteoff 11 63 0
noteoff 11 60 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 67 0
noteon 10 65 102
sleep 3.558
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 65 0
noteon 10 63 102
sleep 74.733
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 63 0
noteon 10 62 102
sleep 1.779
noteon 0 81 101
sleep 1.779
noteon 11 63 102
noteon 11 60 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 62 0
noteon 10 60 102
sleep 71.174
noteoff 10 60 0
sleep 3.558
noteoff 11 60 0
noteoff 11 63 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteon 10 58 102
sleep 1.661
noteoff 0 81 0
noteon 0 82 101
sleep 1.661
noteon 1 70 100
noteon 1 74 100
noteon 11 58 102
noteon 11 62 102
sleep 6.644
noteon 12 53 102
sleep 4.983
noteoff 3 57 0
noteon 3 58 100
sleep 1.661
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 58 0
sleep 1.661
noteoff 0 82 0
sleep 1.661
noteoff 1 74 0
noteoff 1 70 0
noteoff 11 62 0
noteoff 11 58 0
sleep 6.644
noteoff 12 53 0
sleep 4.983
noteoff 3 58 0
sleep 1.661
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 79.734
noteon 10 77 102
sleep 1.779
noteon 0 89 101
sleep 1.779
noteon 1 70 100
noteon 11 58 102
noteon 11 62 102
noteon 1 74 100
sleep 7.117
noteon 12 53 102
sleep 5.337
noteon 3 65 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 782.921
noteoff 10 77 0
sleep 19.572
noteoff 0 89 0
sleep 14.234
noteoff 3 65 0
sleep 1.779
noteon 10 74 102
sleep 1.779
noteon 0 86 101
sleep 14.234
noteon 3 62 100
sleep 90.747
noteoff 10 74 0
noteon 10 70 102
sleep 1.779
noteoff 0 86 0
noteon 0 82 101
sleep 14.234
noteoff 3 62 0
noteon 3 58 100
sleep 55.16
noteoff 10 70 0
sleep 3.558
noteoff 11 62 0
noteoff 11 58 0
sleep 16.014
noteoff 0 82 0
sleep 1.779
noteoff 1 74 0
noteoff 1 70 0
sleep 12.455
noteoff 3 58 0
sleep 1.779
noteon 10 69 102
sleep 1.661
noteon 0 81 101
sleep 1.661
noteon 1 75 100
noteon 1 72 100
noteon 11 63 102
noteon 11 60 102
sleep 11.627
noteon 3 57 100
sleep 84.717
noteoff 10 69 0
noteon 10 72 102
sleep 1.661
noteoff 0 81 0
noteon 0 84 101
sleep 68.106
noteoff 11 60 0
noteoff 11 63 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 72 0
noteon 10 75 102
sleep 1.779
noteoff 0 84 0
noteon 0 87 101
sleep 1.779
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteoff 3 57 0
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 75 0
noteon 10 74 102
sleep 1.779
noteoff 0 87 0
noteon 0 86 101
sleep 72.953
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 74 0
noteon 10 72 102
sleep 1.779
noteoff 0 86 0
noteon 0 84 101
sleep 1.779
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 7.117
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 72 0
noteon 10 70 102
sleep 1.779
noteoff 0 84 0
noteon 0 82 101
sleep 72.953
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 14.234
noteoff 10 70 0
noteon 10 69 102
sleep 1.661
noteoff 0 82 0
noteon 0 81 101
sleep 1.661
noteon 11 60 102
noteon 11 63 102
sleep 6.644
noteon 12 53 102
sleep 6.644
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 69 0
noteon 10 67 102
sleep 1.661
noteoff 0 81 0
noteon 0 79 101
sleep 68.106
noteoff 11 63 0
noteoff 11 60 0
sleep 6.644
noteoff 12 53 0
sleep 6.644
noteoff 13 46 0
sleep 3.322
noteoff 14 34 0
sleep 13.289
noteoff 10 67 0
noteon 10 65 102
sleep 1.779
noteoff 0 79 0
noteon 0 77 101
sleep 1.779
noteon 11 63 102
noteon 11 60 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteon 3 53 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 65 0
noteon 10 63 102
sleep 1.779
noteoff 0 77 0
noteon 0 75 101
sleep 72.953
noteoff 11 60 0
noteoff 11 63 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 12.455
noteoff 3 53 0
sleep 1.779
noteoff 10 63 0
noteon 10 62 102
sleep 1.779
noteoff 0 75 0
noteon 0 74 101
sleep 1.779
noteon 11 60 102
noteon 11 63 102
sleep 7.117
noteon 12 53 102
sleep 5.338
noteon 3 53 100
sleep 1.779
noteon 13 46 104
sleep 3.558
noteon 14 34 106
sleep 85.409
noteoff 10 62 0
noteon 10 60 102
sleep 1.779
noteoff 0 74 0
noteon 0 72 101
sleep 69.395
noteoff 10 60 0
sleep 3.558
noteoff 11 63 0
noteoff 11 60 0
sleep 7.117
noteoff 12 53 0
sleep 7.117
noteoff 13 46 0
sleep 1.779
noteoff 0 72 0
sleep 1.779
noteoff 1 72 0
noteoff 1 75 0
noteoff 14 34 0
sleep 12.455
noteoff 3 53 0
sleep 1.779
noteon 10 58 102
sleep 1.661
noteon 0 70 101
sleep 1.661
noteon 1 74 100
noteon 1 70 100
noteon 11 58 102
noteon 11 62 102
sleep 6.644
noteon 12 53 102
sleep 4.983
noteon 3 58 100
sleep 1.661
noteon 13 46 104
sleep 3.322
noteon 14 34 106
sleep 79.734
noteoff 10 58 0
sleep 3.322
noteoff 11 62 0
noteoff 11 58 0
sleep 6.644
noteoff 12 53 0
sleep 89.7
noteon 10 74 102
sleep 1.779
noteoff 0 70 0
sleep 1.779
noteoff 1 70 0
noteoff 1 74 0
noteon 11 62 102
sleep 7.117
noteon 12 58 102
sleep 5.338
noteoff 3 58 0
sleep 1.779
noteoff 13 46 0
sleep 3.558
noteoff 14 34 0
sleep 85.409
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 74 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 58 0
noteon 12 62 102
sleep 96.085
noteoff 10 74 0
noteon 10 76 102
sleep 71.174
noteoff 10 76 0
sleep 35.587
noteon 10 77 102
sleep 3.3
noteoff 11 65 0
sleep 6.6
noteoff 12 62 0
sleep 4.95
noteon 3 57 100
sleep 1.65
noteon 13 45 104
sleep 3.3
noteon 14 33 106
sleep 79.207
noteoff 10 77 0
sleep 99.009
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 7.117
noteon 12 57 102
sleep 5.338
noteoff 3 57 0
sleep 1.779
noteoff 13 45 0
sleep 3.558
noteoff 14 33 0
sleep 85.409
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 74 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 57 0
noteon 12 62 102
sleep 96.085
noteoff 10 74 0
noteon 10 76 102
sleep 71.174
noteoff 10 76 0
sleep 35.587
noteon 10 77 102
sleep 3.267
noteoff 11 65 0
sleep 6.535
noteoff 12 62 0
sleep 4.901
noteon 3 56 100
sleep 1.633
noteon 13 44 104
sleep 3.267
noteon 14 32 106
sleep 78.431
noteoff 10 77 0
sleep 98.039
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 7.117
noteon 12 59 102
sleep 5.338
noteoff 3 56 0
sleep 1.779
noteoff 13 44 0
sleep 3.558
noteoff 14 32 0
sleep 85.409
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 74 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 59 0
noteon 12 62 102
sleep 96.085
noteoff 10 74 0
noteon 10 76 102
sleep 71.174
noteoff 10 76 0
sleep 35.587
noteon 10 77 102
sleep 3.257
noteoff 11 65 0
noteon 11 65 102
sleep 6.514
noteoff 12 62 0
noteon 12 56 102
noteon 12 59 102
sleep 4.885
noteon 3 56 100
sleep 1.628
noteon 13 56 104
sleep 3.257
noteon 14 44 106
sleep 78.172
noteoff 10 77 0
sleep 3.257
noteoff 11 65 0
sleep 6.514
noteoff 12 59 0
noteoff 12 56 0
sleep 87.942
noteon 10 79 102
sleep 3.558
noteon 11 65 102
sleep 7.117
noteon 12 56 102
noteon 12 59 102
sleep 5.338
noteoff 3 56 0
sleep 1.779
noteoff 13 56 0
sleep 3.558
noteoff 14 44 0
sleep 85.406
noteoff 10 79 0
noteon 10 77 102
sleep 3.558
noteoff 11 65 0
sleep 7.117
noteoff 12 59 0
noteoff 12 56 0
sleep 96.081
noteoff 10 77 0
noteon 10 76 102
sleep 3.558
noteon 11 65 102
sleep 7.117
noteon 12 56 102
noteon 12 59 102
sleep 96.080
noteoff 10 76 0
noteon 10 74 102
sleep 3.558
noteoff 11 65 0
sleep 7.117
noteoff 12 59 0
noteoff 12 56 0
sleep 60.495
noteoff 10 74 0
sleep 35.587
noteon 10 73 102
sleep 3.236
noteon 11 64 102
sleep 6.472
noteon 12 61 102
noteon 12 57 102
sleep 4.854
noteon 3 57 100
sleep 1.618
noteon 13 57 104
sleep 3.236
noteon 14 33 106
sleep 77.669
noteoff 10 73 0
sleep 16.181
noteoff 13 57 0
sleep 80.906
noteon 10 74 102
sleep 3.558
noteoff 11 64 0
sleep 7.117
noteoff 12 57 0
noteoff 12 61 0
sleep 5.338
noteoff 3 57 0
sleep 90.747
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 71 102
sleep 10.676
noteon 12 49 102
sleep 7.117
noteon 13 49 104
sleep 88.967
noteoff 10 71 0
noteon 10 69 102
sleep 71.174
noteoff 10 69 0
sleep 35.587
noteon 10 77 102
sleep 9.677
noteoff 12 49 0
noteon 12 50 102
sleep 6.451
noteoff 13 49 0
noteon 13 50 104
sleep 80.641
noteoff 10 77 0
sleep 96.769
noteon 10 79 102
sleep 10.676
noteoff 12 50 0
noteon 12 53 102
sleep 7.117
noteoff 13 50 0
noteon 13 53 104
sleep 88.966
noteoff 10 79 0
noteon 10 77 102
sleep 106.758
noteoff 10 77 0
noteon 10 76 102
sleep 10.676
noteoff 12 53 0
noteon 12 56 102
sleep 7.117
noteoff 13 53 0
noteon 13 56 104
sleep 88.964
noteoff 10 76 0
noteon 10 74 102
sleep 71.172
noteoff 10 74 0
sleep 10.676
noteoff 12 56 0
sleep 7.117
noteoff 13 56 0
sleep 17.793
noteon 10 73 102
sleep 9.646
noteon 12 57 102
sleep 6.43
noteon 13 57 104
sleep 80.385
noteoff 10 73 0
sleep 9.646
noteoff 12 57 0
sleep 6.43
noteoff 13 57 0
sleep 80.385
noteon 10 74 102
sleep 106.761
noteoff 10 74 0
noteon 10 73 102
sleep 106.761
noteoff 10 73 0
noteon 10 71 102
sleep 3.558
noteon 11 61 102
sleep 7.117
noteon 12 49 102
sleep 7.117
noteon 13 49 104
sleep 88.967
noteoff 10 71 0
noteon 10 69 102
sleep 71.174
noteoff 10 69 0
sleep 35.587
noteon 10 77 102
sleep 3.205
noteoff 11 61 0
noteon 11 62 102
sleep 6.41
noteoff 12 49 0
noteon 12 50 102
sleep 6.41
noteoff 13 49 0
noteon 13 50 104
sleep 80.128
noteoff 10 77 0
sleep 96.153
noteon 10 79 102
sleep 3.558
noteoff 11 62 0
noteon 11 65 102
sleep 7.117
noteoff 12 50 0
noteon 12 53 102
sleep 7.117
noteoff 13 50 0
noteon 13 53 104
sleep 88.959
noteoff 10 79 0
noteon 10 77 102
sleep 106.751
noteoff 10 77 0
noteon 10 76 102
sleep 3.558
noteoff 11 65 0
noteon 11 68 102
sleep 7.116
noteoff 12 53 0
noteon 12 56 102
sleep 7.116
noteoff 13 53 0
noteon 13 56 104
sleep 88.959
noteoff 10 76 0
noteon 10 74 102
sleep 71.167
noteoff 10 74 0
sleep 3.558
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 7.117
noteoff 13 56 0
sleep 17.792
noteon 10 73 102
sleep 3.194
noteon 11 69 102
sleep 6.389
noteon 12 57 102
sleep 6.389
noteon 13 57 104
sleep 79.859
noteoff 10 73 0
sleep 3.194
noteoff 11 69 0
sleep 6.388
noteoff 12 57 0
sleep 6.389
noteoff 13 57 0
sleep 79.859
noteon 10 76 102
sleep 3.558
noteon 11 57 102
sleep 17.791
noteoff 14 33 0
sleep 85.400
noteoff 10 76 0
noteon 10 74 102
sleep 106.751
noteoff 10 74 0
noteon 10 73 102
sleep 3.558
noteoff 11 57 0
noteon 11 58 102
sleep 103.193
noteoff 10 73 0
noteon 10 71 102
sleep 71.167
noteoff 10 71 0
sleep 35.583
noteon 10 69 102
sleep 3.174
noteoff 11 58 0
noteon 11 59 102
sleep 92.056
noteoff 10 69 0
noteon 10 68 102
sleep 63.487
noteoff 10 68 0
sleep 31.744
noteon 10 69 102
sleep 3.558
noteoff 11 59 0
noteon 11 60 102
sleep 49.817
noteoff 10 69 0
sleep 53.375
noteon 10 67 102
sleep 53.376
noteoff 10 67 0
sleep 53.375
noteon 10 66 102
sleep 3.558
noteoff 11 60 0
noteon 11 61 102
sleep 49.817
noteoff 10 66 0
sleep 53.375
noteon 10 64 102
sleep 71.420
noteoff 10 64 0
sleep 28.568
noteoff 11 61 0
sleep 42.852
noteon 10 62 102
sleep 1.587
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 62 101
noteon 2 74 101
noteon 6 62 108
noteon 6 74 108
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 46.969
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 15.097
noteoff 0 74 0
sleep 1.677
noteoff 1 74 0
sleep 1.677
noteoff 2 74 0
noteoff 2 62 0
sleep 11.741
noteon 10 78 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 55 102
noteon 10 64 102
sleep 1.587
noteon 0 76 101
noteon 0 74 101
sleep 1.587
noteon 1 76 100
noteon 4 62 100
noteon 11 59 102
noteon 11 62 102
noteon 1 74 100
sleep 1.587
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
noteon 2 64 101
sleep 4.761
noteon 5 50 100
noteon 12 67 102
sleep 4.761
noteon 3 55 100
sleep 1.587
noteon 13 55 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 43 106
sleep 85.706
noteoff 12 67 0
sleep 4.761
noteoff 3 55 0
sleep 1.587
noteoff 13 55 0
sleep 3.174
noteoff 14 43 0
sleep 86.248
noteon 12 69 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 69 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 71 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 71 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 46.969
noteoff 10 64 0
noteoff 10 55 0
sleep 3.354
noteoff 11 62 0
noteoff 11 59 0
sleep 15.097
noteoff 0 74 0
noteoff 0 76 0
sleep 1.677
noteoff 1 74 0
noteoff 1 76 0
sleep 1.677
noteoff 2 64 0
noteoff 2 74 0
sleep 11.741
noteon 10 79 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 79 0
sleep 109.523
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 122.775
noteon 10 83 102
sleep 90.747
noteoff 10 83 0
sleep 125.949
noteon 4 64 100
sleep 6.349
noteon 5 57 100
sleep 88.888
noteoff 4 64 0
sleep 6.349
noteoff 5 57 0
sleep 89.272
noteon 4 66 100
sleep 7.117
noteon 5 62 100
sleep 99.644
noteoff 4 66 0
sleep 7.117
noteoff 5 62 0
sleep 99.644
noteon 4 67 100
sleep 7.117
noteon 5 64 100
sleep 99.644
noteoff 4 67 0
sleep 7.117
noteoff 5 64 0
sleep 96.085
noteon 10 76 102
sleep 3.174
noteon 11 69 102
sleep 77.777
noteoff 10 76 0
sleep 3.174
noteoff 11 69 0
sleep 106.349
noteon 10 78 102
sleep 3.558
noteon 11 74 102
sleep 87.188
noteoff 10 78 0
sleep 3.558
noteoff 11 74 0
sleep 119.217
noteon 10 79 102
sleep 3.558
noteon 11 76 102
sleep 87.188
noteoff 10 79 0
sleep 3.558
noteoff 11 76 0
sleep 122.391
noteon 1 78 100
noteon 1 74 100
noteon 4 66 100
sleep 6.349
noteon 5 62 100
sleep 88.888
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 66 0
sleep 6.349
noteoff 5 62 0
sleep 89.272
noteon 1 76 100
noteon 1 69 100
noteon 4 64 100
sleep 7.117
noteon 5 57 100
sleep 99.644
noteoff 1 69 0
noteoff 1 76 0
noteoff 4 64 0
sleep 7.117
noteoff 5 57 0
sleep 99.644
noteon 1 74 100
noteon 1 66 100
noteon 4 62 100
sleep 7.117
noteon 5 54 100
sleep 99.644
noteoff 1 66 0
noteoff 1 74 0
noteoff 4 62 0
sleep 7.117
noteoff 5 54 0
sleep 96.085
noteon 10 86 102
sleep 1.587
noteon 0 78 101
noteon 0 86 101
sleep 1.587
noteon 1 78 100
noteon 1 74 100
noteon 11 69 102
noteon 11 78 102
sleep 1.587
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
noteon 2 66 101
sleep 4.761
noteon 12 62 102
sleep 4.761
noteon 3 50 100
noteon 3 62 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 92
sleep 1.587
noteon 14 38 106
sleep 76.19
noteoff 10 86 0
sleep 1.587
noteoff 0 86 0
noteoff 0 78 0
sleep 1.587
noteoff 1 74 0
noteoff 1 78 0
noteoff 11 78 0
noteoff 11 69 0
sleep 1.587
noteoff 2 66 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 12 62 0
sleep 4.761
noteoff 3 62 0
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 1.587
noteoff 15 50 0
sleep 1.587
noteoff 14 38 0
sleep 76.19
noteon 10 85 102
sleep 1.779
noteon 0 85 101
noteon 0 76 101
sleep 1.779
noteon 1 79 100
noteon 1 76 100
noteon 11 69 102
noteon 11 79 102
sleep 1.779
noteon 2 76 101
noteon 2 67 101
noteon 6 69 108
noteon 6 57 108
sleep 5.338
noteon 12 57 102
sleep 5.338
noteon 3 45 100
noteon 3 57 100
sleep 1.779
noteon 13 45 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteon 14 33 106
sleep 85.409
noteoff 10 85 0
sleep 1.779
noteoff 0 76 0
noteoff 0 85 0
sleep 1.779
noteoff 1 76 0
noteoff 1 79 0
noteoff 11 79 0
noteoff 11 69 0
sleep 1.779
noteoff 2 67 0
noteoff 2 76 0
noteoff 6 57 0
noteoff 6 69 0
sleep 5.338
noteoff 12 57 0
sleep 5.338
noteoff 3 57 0
noteoff 3 45 0
sleep 1.779
noteoff 13 45 0
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteoff 14 33 0
sleep 85.409
noteon 10 86 102
sleep 1.779
noteon 0 78 101
noteon 0 86 101
sleep 1.779
noteon 1 74 100
noteon 1 78 100
noteon 11 78 102
noteon 11 69 102
sleep 1.779
noteon 2 74 101
noteon 2 66 101
noteon 6 74 108
noteon 6 62 108
sleep 5.338
noteon 12 62 102
sleep 5.338
noteon 3 62 100
noteon 3 50 100
sleep 1.779
noteon 13 50 104
sleep 1.779
noteon 15 50 92
sleep 1.779
noteon 14 38 106
sleep 85.409
noteoff 10 86 0
sleep 1.779
noteoff 0 86 0
noteoff 0 78 0
sleep 1.779
noteoff 1 78 0
noteoff 1 74 0
noteoff 11 69 0
noteoff 11 78 0
sleep 1.779
noteoff 2 66 0
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 12 62 0
sleep 5.338
noteoff 3 50 0
noteoff 3 62 0
sleep 1.779
noteoff 13 50 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 38 0
sleep 85.409
noteon 10 62 102
sleep 1.587
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 62 101
noteon 6 62 108
noteon 6 74 108
noteon 2 74 101
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 85.706
noteoff 12 62 0
sleep 4.761
noteoff 3 50 0
sleep 1.587
noteoff 13 50 0
sleep 3.174
noteoff 14 38 0
sleep 86.248
noteon 12 64 102
sleep 5.033
noteon 3 52 100
sleep 1.677
noteon 13 52 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 40 106
sleep 90.590
noteoff 12 64 0
sleep 5.033
noteoff 3 52 0
sleep 1.677
noteoff 13 52 0
sleep 3.355
noteoff 14 40 0
sleep 90.590
noteon 12 66 102
sleep 5.033
noteon 3 54 100
sleep 1.677
noteon 13 54 104
sleep 3.355
noteon 14 42 106
sleep 90.589
noteoff 12 66 0
sleep 5.033
noteoff 3 54 0
sleep 1.677
noteoff 13 54 0
sleep 3.354
noteoff 14 42 0
sleep 46.969
noteoff 10 62 0
sleep 3.354
noteoff 11 62 0
sleep 15.097
noteoff 0 74 0
sleep 1.677
noteoff 1 74 0
sleep 1.677
noteoff 2 74 0
noteoff 2 62 0
sleep 11.741
noteon 10 78 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 74 0
noteoff 6 62 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 78 0
sleep 109.523
noteon 10 79 102
sleep 90.747
noteoff 10 79 0
sleep 122.775
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 140.823
noteon 10 55 102
noteon 10 64 102
sleep 1.587
noteon 0 76 101
noteon 0 74 101
sleep 1.587
noteon 1 74 100
noteon 1 76 100
noteon 4 62 100
noteon 11 62 102
noteon 11 59 102
sleep 1.587
noteon 2 64 101
noteon 6 74 108
noteon 6 62 108
noteon 2 74 101
sleep 4.761
noteon 5 50 100
noteon 12 55 102
sleep 4.761
noteon 3 55 100
sleep 1.587
noteon 13 55 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 43 106
sleep 85.706
noteoff 12 55 0
sleep 4.761
noteoff 3 55 0
sleep 1.587
noteoff 13 55 0
sleep 3.174
noteoff 14 43 0
sleep 86.248
noteon 12 57 102
sleep 5.033
noteon 3 57 100
sleep 1.677
noteon 13 57 104
sleep 1.677
noteoff 15 50 0
sleep 1.677
noteon 14 45 106
sleep 90.590
noteoff 12 57 0
sleep 5.033
noteoff 3 57 0
sleep 1.677
noteoff 13 57 0
sleep 3.355
noteoff 14 45 0
sleep 90.590
noteon 12 59 102
sleep 5.033
noteon 3 59 100
sleep 1.677
noteon 13 59 104
sleep 3.355
noteon 14 47 106
sleep 90.589
noteoff 12 59 0
sleep 5.033
noteoff 3 59 0
sleep 1.677
noteoff 13 59 0
sleep 3.354
noteoff 14 47 0
sleep 46.969
noteoff 10 64 0
noteoff 10 55 0
sleep 3.354
noteoff 11 59 0
noteoff 11 62 0
sleep 15.097
noteoff 0 74 0
noteoff 0 76 0
sleep 1.677
noteoff 1 76 0
noteoff 1 74 0
sleep 1.677
noteoff 2 74 0
noteoff 2 64 0
sleep 11.741
noteon 10 79 102
sleep 3.174
noteoff 4 62 0
sleep 1.587
noteoff 6 62 0
noteoff 6 74 0
sleep 4.761
noteoff 5 50 0
sleep 71.428
noteoff 10 79 0
sleep 109.523
noteon 10 81 102
sleep 90.747
noteoff 10 81 0
sleep 122.775
noteon 10 83 102
sleep 90.747
noteoff 10 83 0
sleep 137.576
noteon 10 67 102
sleep 83.061
noteoff 10 67 0
sleep 112.377
noteon 10 69 102
sleep 86.148
noteoff 10 69 0
sleep 116.554
noteon 10 71 102
sleep 86.148
noteoff 10 71 0
sleep 119.811
noteon 11 64 102
sleep 83.061
noteoff 11 64 0
sleep 112.498
noteon 11 66 102
sleep 86.148
noteoff 11 66 0
sleep 116.554
noteon 11 67 102
sleep 86.148
noteoff 11 67 0
sleep 113.175
noteon 10 67 102
sleep 83.061
noteoff 10 67 0
sleep 112.377
noteon 10 69 102
sleep 86.148
noteoff 10 69 0
sleep 116.554
noteon 10 70 102
sleep 86.148
noteoff 10 70 0
sleep 119.811
noteon 11 64 102
sleep 83.061
noteoff 11 64 0
sleep 112.498
noteon 11 66 102
sleep 86.148
noteoff 11 66 0
sleep 116.554
noteon 11 67 102
sleep 86.148
noteoff 11 67 0
sleep 113.175
noteon 10 67 102
sleep 83.061
noteoff 10 67 0
sleep 112.376
noteon 10 69 102
sleep 86.148
noteoff 10 69 0
sleep 116.552
noteon 10 70 102
sleep 86.148
noteoff 10 70 0
sleep 116.554
noteon 10 67 102
sleep 3.257
noteon 11 64 102
sleep 79.803
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.12
noteon 10 69 102
sleep 3.424
noteon 11 65 102
sleep 83.904
noteoff 10 69 0
sleep 3.424
noteoff 11 65 0
sleep 114.725
noteon 10 70 102
sleep 3.424
noteon 11 67 102
sleep 83.904
noteoff 10 70 0
sleep 3.424
noteoff 11 67 0
sleep 114.725
noteon 10 67 102
sleep 3.257
noteon 11 64 102
sleep 79.803
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.12
noteon 10 69 102
sleep 3.472
noteon 11 65 102
sleep 85.068
noteoff 10 69 0
sleep 3.472
noteoff 11 65 0
sleep 116.319
noteon 10 70 102
sleep 3.472
noteon 11 67 102
sleep 85.069
noteoff 10 70 0
sleep 3.472
noteoff 11 67 0
sleep 116.318
noteon 10 67 102
sleep 3.257
noteon 11 64 102
sleep 79.804
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.119
noteon 10 69 102
sleep 3.558
noteon 11 65 102
sleep 87.187
noteoff 10 69 0
sleep 3.558
noteoff 11 65 0
sleep 119.217
noteon 10 70 102
sleep 3.558
noteon 11 67 102
sleep 87.186
noteoff 10 70 0
sleep 3.558
noteoff 11 67 0
sleep 119.217
noteon 10 67 102
sleep 3.257
noteon 1 72 100
noteon 11 64 102
sleep 6.514
noteon 12 60 102
sleep 4.885
noteon 3 60 100
sleep 1.628
noteon 13 60 104
sleep 3.257
noteon 14 48 106
sleep 63.517
noteoff 10 67 0
sleep 3.257
noteoff 11 64 0
sleep 109.12
noteon 10 69 102
sleep 3.558
noteon 11 65 102
sleep 7.117
noteoff 12 60 0
sleep 7.117
noteoff 13 60 0
sleep 3.558
noteoff 14 48 0
sleep 69.395
noteoff 10 69 0
sleep 3.558
noteoff 11 65 0
sleep 119.217
noteon 10 70 102
sleep 3.558
noteon 11 67 102
sleep 87.188
noteoff 10 70 0
sleep 3.558
noteoff 11 67 0
sleep 96.085
noteoff 1 72 0
sleep 12.455
noteoff 3 60 0
sleep 10.676
noteon 10 69 102
sleep 3.257
noteon 1 77 100
noteon 11 65 102
sleep 6.514
noteon 12 53 102
sleep 4.885
noteon 3 65 100
sleep 1.628
noteon 13 53 104
sleep 3.257
noteon 14 41 106
sleep 81.433
noteoff 1 77 0
sleep 11.4
noteoff 3 65 0
sleep 83.061
noteoff 10 69 0
sleep 3.558
noteoff 11 65 0
noteon 1 76 100
sleep 7.117
noteoff 12 53 0
sleep 5.338
noteon 3 64 100
sleep 1.779
noteoff 13 53 0
sleep 3.558
noteoff 14 41 0
sleep 88.967
noteoff 1 76 0
sleep 12.455
noteoff 3 64 0
sleep 94.306
noteon 1 77 100
sleep 12.455
noteon 3 65 100
sleep 94.306
noteoff 1 77 0
sleep 12.455
noteoff 3 65 0
sleep 90.747
noteon 10 64 102
sleep 3.257
noteon 1 69 100
noteon 11 61 102
sleep 6.514
noteon 12 57 102
sleep 4.885
noteon 3 57 100
sleep 1.628
noteon 13 57 104
sleep 3.257
noteon 14 45 106
sleep 63.517
noteoff 10 64 0
sleep 3.257
noteoff 11 61 0
sleep 109.12
noteon 10 65 102
sleep 3.558
noteon 11 62 102
sleep 7.117
noteoff 12 57 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 69.395
noteoff 10 65 0
sleep 3.558
noteoff 11 62 0
sleep 119.217
noteon 10 67 102
sleep 3.558
noteon 11 64 102
sleep 87.188
noteoff 10 67 0
sleep 3.558
noteoff 11 64 0
sleep 96.085
noteoff 1 69 0
sleep 12.455
noteoff 3 57 0
sleep 10.676
noteon 10 65 102
sleep 3.257
noteon 1 74 100
noteon 11 62 102
sleep 6.514
noteon 12 50 102
sleep 4.885
noteon 3 62 100
sleep 1.628
noteon 13 50 104
sleep 3.257
noteon 14 38 106
sleep 81.433
noteoff 1 74 0
sleep 11.4
noteoff 3 62 0
sleep 83.061
noteoff 10 65 0
sleep 3.558
noteoff 11 62 0
noteon 1 73 100
sleep 7.117
noteoff 12 50 0
sleep 5.338
noteon 3 61 100
sleep 1.779
noteoff 13 50 0
sleep 3.558
noteoff 14 38 0
sleep 88.967
noteoff 1 73 0
sleep 12.455
noteoff 3 61 0
sleep 94.306
noteon 1 74 100
sleep 12.455
noteon 3 62 100
sleep 94.948
noteoff 1 74 0
sleep 14.705
noteoff 3 62 0
sleep 110.306
noteon 4 62 100
noteon 11 62 102
sleep 1.582
noteon 2 62 101
noteon 2 74 101
sleep 4.746
noteon 5 50 100
noteon 12 50 102
sleep 4.746
noteon 3 46 100
noteon 3 50 100
sleep 1.582
noteon 13 50 104
sleep 3.164
noteon 14 34 106
sleep 174.430
noteoff 11 62 0
noteon 11 64 102
sleep 7.117
noteoff 12 50 0
noteon 12 52 102
sleep 5.337
noteoff 3 50 0
noteon 3 52 100
sleep 1.779
noteoff 13 50 0
noteon 13 52 104
sleep 199.274
noteoff 11 64 0
noteon 11 65 102
sleep 7.117
noteoff 12 52 0
noteon 12 53 102
sleep 5.337
noteoff 3 52 0
noteon 3 53 100
sleep 1.779
noteoff 13 52 0
noteon 13 53 104
sleep 198.889
noteoff 11 65 0
noteon 11 66 102
sleep 6.349
noteoff 12 53 0
noteon 12 54 102
sleep 4.761
noteoff 3 53 0
noteon 3 54 100
sleep 1.587
noteoff 13 53 0
noteon 13 54 104
sleep 178.144
noteoff 11 66 0
noteon 11 67 102
sleep 7.117
noteoff 12 54 0
noteon 12 55 102
sleep 5.337
noteoff 3 54 0
noteon 3 55 100
sleep 1.779
noteoff 13 54 0
noteon 13 55 104
sleep 199.274
noteoff 11 67 0
noteon 11 68 102
sleep 7.116
noteoff 12 55 0
noteon 12 56 102
sleep 5.337
noteoff 3 55 0
noteon 3 56 100
sleep 1.779
noteoff 13 55 0
noteon 13 56 104
sleep 163.687
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 3.558
noteoff 2 74 0
noteoff 2 62 0
sleep 3.558
noteoff 13 56 0
sleep 3.558
noteoff 14 34 0
sleep 3.558
noteoff 3 56 0
noteoff 3 46 0
sleep 10.674
noteon 10 69 102
noteon 10 81 102
sleep 3.174
noteoff 4 62 0
noteon 11 69 102
noteon 4 57 100
sleep 1.587
noteon 2 73 101
noteon 6 57 108
noteon 2 61 101
noteon 6 69 108
sleep 4.761
noteoff 5 50 0
noteon 12 57 102
noteon 5 45 100
sleep 4.761
noteon 3 45 100
noteon 3 57 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.206
noteoff 12 57 0
sleep 6.349
noteoff 13 57 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 57 0
noteoff 3 45 0
sleep 9.523
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.259
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 12 69 0
sleep 3.558
noteoff 2 61 0
noteoff 2 73 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 69 0
noteoff 6 57 0
sleep 1.779
noteoff 3 57 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 3.174
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
noteon 6 62 108
noteon 6 74 108
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 38 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 92.063
noteon 10 76 102
sleep 3.558
noteoff 4 62 0
noteon 11 64 102
sleep 1.779
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 74 0
noteoff 6 62 0
sleep 5.338
noteoff 5 50 0
noteoff 12 62 0
sleep 5.338
noteoff 3 50 0
sleep 1.779
noteoff 13 50 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 38 0
sleep 85.409
noteoff 10 76 0
sleep 3.558
noteoff 11 64 0
sleep 103.202
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 103.202
noteoff 10 74 0
sleep 4.329
noteoff 11 62 0
sleep 125.541
noteon 10 72 102
sleep 3.257
noteon 1 72 100
noteon 11 67 102
sleep 6.514
noteon 12 64 102
sleep 4.885
noteon 3 60 100
sleep 1.628
noteon 13 60 104
sleep 3.257
noteon 14 48 106
sleep 81.433
noteoff 11 67 0
sleep 6.514
noteoff 12 64 0
sleep 91.505
noteon 11 69 102
sleep 7.117
noteon 12 65 102
sleep 7.117
noteoff 13 60 0
sleep 3.558
noteoff 14 48 0
sleep 88.967
noteoff 11 69 0
sleep 7.117
noteoff 12 65 0
sleep 99.644
noteon 11 70 102
sleep 7.117
noteon 12 67 102
sleep 99.644
noteoff 11 70 0
sleep 7.117
noteoff 12 67 0
sleep 60.498
noteoff 10 72 0
sleep 12.455
noteoff 1 72 0
sleep 12.455
noteoff 3 60 0
sleep 10.676
noteon 10 77 102
sleep 3.257
noteon 1 77 100
noteon 11 69 102
sleep 6.514
noteon 12 65 102
sleep 4.885
noteon 3 65 100
sleep 1.628
noteon 13 53 104
sleep 3.257
noteon 14 41 106
sleep 78.175
noteoff 10 77 0
sleep 3.257
noteoff 1 77 0
sleep 11.4
noteoff 3 65 0
sleep 83.061
noteon 10 76 102
sleep 3.558
noteoff 11 69 0
noteon 1 76 100
sleep 7.117
noteoff 12 65 0
sleep 5.338
noteon 3 64 100
sleep 1.779
noteoff 13 53 0
sleep 3.558
noteoff 14 41 0
sleep 85.409
noteoff 10 76 0
sleep 3.558
noteoff 1 76 0
sleep 12.455
noteoff 3 64 0
sleep 90.747
noteon 10 77 102
sleep 3.558
noteon 1 77 100
sleep 12.455
noteon 3 65 100
sleep 90.747
noteoff 10 77 0
sleep 3.558
noteoff 1 77 0
sleep 12.455
noteoff 3 65 0
sleep 90.747
noteon 10 69 102
sleep 1.628
noteon 0 81 101
sleep 1.628
noteon 1 69 100
noteon 11 64 102
sleep 6.514
noteon 12 61 102
sleep 4.885
noteon 3 57 100
sleep 1.628
noteon 13 57 104
sleep 3.257
noteon 14 45 106
sleep 81.433
noteoff 11 64 0
sleep 6.514
noteoff 12 61 0
sleep 91.505
noteon 11 65 102
sleep 7.117
noteon 12 62 102
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 88.967
noteoff 11 65 0
sleep 7.117
noteoff 12 62 0
sleep 99.644
noteon 11 67 102
sleep 7.117
noteon 12 64 102
sleep 99.644
noteoff 11 67 0
sleep 7.117
noteoff 12 64 0
sleep 60.498
noteoff 10 69 0
sleep 10.676
noteoff 0 81 0
sleep 1.779
noteoff 1 69 0
sleep 12.455
noteoff 3 57 0
sleep 10.676
noteon 10 74 102
sleep 1.628
noteon 0 86 101
sleep 1.628
noteon 1 74 100
noteon 11 65 102
sleep 6.514
noteon 12 62 102
sleep 4.885
noteon 3 62 100
sleep 1.628
noteon 13 50 104
sleep 3.257
noteon 14 38 106
sleep 78.175
noteoff 10 74 0
sleep 1.628
noteoff 0 86 0
sleep 1.628
noteoff 1 74 0
sleep 11.4
noteoff 3 62 0
sleep 83.061
noteon 10 73 102
sleep 1.779
noteon 0 85 101
sleep 1.779
noteoff 11 65 0
noteon 1 73 100
sleep 7.117
noteoff 12 62 0
sleep 5.338
noteon 3 61 100
sleep 1.779
noteoff 13 50 0
sleep 3.558
noteoff 14 38 0
sleep 85.409
noteoff 10 73 0
sleep 1.779
noteoff 0 85 0
sleep 1.779
noteoff 1 73 0
sleep 12.455
noteoff 3 61 0
sleep 90.747
noteon 10 74 102
sleep 1.779
noteon 0 86 101
sleep 1.779
noteon 1 74 100
sleep 12.455
noteon 3 62 100
sleep 90.747
noteoff 10 74 0
sleep 2.164
noteoff 0 86 0
sleep 2.164
noteoff 1 74 0
sleep 15.151
noteoff 3 62 0
sleep 113.563
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
sleep 4.761
noteon 5 50 100
noteon 12 50 102
sleep 4.761
noteon 3 50 100
noteon 3 46 100
sleep 1.587
noteon 13 50 104
sleep 3.174
noteon 14 34 106
sleep 174.970
noteoff 11 62 0
noteon 11 64 102
sleep 7.117
noteoff 12 50 0
noteon 12 52 102
sleep 5.337
noteoff 3 50 0
noteon 3 52 100
sleep 1.779
noteoff 13 50 0
noteon 13 52 104
sleep 199.274
noteoff 11 64 0
noteon 11 65 102
sleep 7.117
noteoff 12 52 0
noteon 12 53 102
sleep 5.337
noteoff 3 52 0
noteon 3 53 100
sleep 1.779
noteoff 13 52 0
noteon 13 53 104
sleep 198.889
noteoff 11 65 0
noteon 11 66 102
sleep 6.349
noteoff 12 53 0
noteon 12 54 102
sleep 4.761
noteoff 3 53 0
noteon 3 54 100
sleep 1.587
noteoff 13 53 0
noteon 13 54 104
sleep 178.144
noteoff 11 66 0
noteon 11 67 102
sleep 7.117
noteoff 12 54 0
noteon 12 55 102
sleep 5.337
noteoff 3 54 0
noteon 3 55 100
sleep 1.779
noteoff 13 54 0
noteon 13 55 104
sleep 199.274
noteoff 11 67 0
noteon 11 68 102
sleep 7.116
noteoff 12 55 0
noteon 12 56 102
sleep 5.337
noteoff 3 55 0
noteon 3 56 100
sleep 1.779
noteoff 13 55 0
noteon 13 56 104
sleep 163.687
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 3.558
noteoff 2 62 0
noteoff 2 74 0
sleep 3.558
noteoff 13 56 0
sleep 3.558
noteoff 14 34 0
sleep 3.558
noteoff 3 56 0
noteoff 3 46 0
sleep 10.674
noteon 10 81 102
noteon 10 69 102
sleep 3.174
noteoff 4 62 0
noteon 4 57 100
noteon 11 69 102
sleep 1.587
noteon 2 73 101
noteon 2 61 101
noteon 6 57 108
noteon 6 69 108
sleep 4.761
noteoff 5 50 0
noteon 12 57 102
noteon 5 45 100
sleep 4.761
noteon 3 57 100
noteon 3 45 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.206
noteoff 12 57 0
sleep 6.349
noteoff 13 57 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 45 0
noteoff 3 57 0
sleep 9.523
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.259
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 12 69 0
sleep 3.558
noteoff 2 61 0
noteoff 2 73 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 69 0
noteoff 6 57 0
sleep 1.779
noteoff 3 57 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 3.174
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 62 101
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 5 50 100
noteon 12 62 102
sleep 4.761
noteon 3 62 100
sleep 1.587
noteon 13 62 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 50 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 92.063
noteon 10 76 102
sleep 3.558
noteoff 4 62 0
noteon 11 64 102
sleep 1.779
noteoff 2 74 0
noteoff 2 62 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 5 50 0
noteoff 12 62 0
sleep 5.338
noteoff 3 62 0
sleep 1.779
noteoff 13 62 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 50 0
sleep 85.409
noteoff 10 76 0
sleep 3.558
noteoff 11 64 0
sleep 103.202
noteon 10 74 102
sleep 3.558
noteon 11 62 102
sleep 103.202
noteoff 10 74 0
sleep 3.558
noteoff 11 62 0
sleep 106.376
noteon 4 62 100
noteon 11 62 102
sleep 1.587
noteon 2 62 101
noteon 2 74 101
sleep 4.761
noteon 5 50 100
noteon 12 50 102
sleep 4.761
noteon 3 46 100
noteon 3 50 100
sleep 1.587
noteon 13 50 104
sleep 3.174
noteon 14 34 106
sleep 174.972
noteoff 11 62 0
noteon 11 64 102
sleep 7.117
noteoff 12 50 0
noteon 12 52 102
sleep 5.337
noteoff 3 50 0
noteon 3 52 100
sleep 1.779
noteoff 13 50 0
noteon 13 52 104
sleep 199.271
noteoff 11 64 0
noteon 11 65 102
sleep 7.116
noteoff 12 52 0
noteon 12 53 102
sleep 5.338
noteoff 3 52 0
noteon 3 53 100
sleep 1.779
noteoff 13 52 0
noteon 13 53 104
sleep 198.887
noteoff 11 65 0
noteon 11 66 102
sleep 6.349
noteoff 12 53 0
noteon 12 54 102
sleep 4.761
noteoff 3 53 0
noteon 3 54 100
sleep 1.587
noteoff 13 53 0
noteon 13 54 104
sleep 178.144
noteoff 11 66 0
noteon 11 67 102
sleep 7.117
noteoff 12 54 0
noteon 12 55 102
sleep 5.337
noteoff 3 54 0
noteon 3 55 100
sleep 1.779
noteoff 13 54 0
noteon 13 55 104
sleep 199.272
noteoff 11 67 0
noteon 11 68 102
sleep 7.116
noteoff 12 55 0
noteon 12 56 102
sleep 5.338
noteoff 3 55 0
noteon 3 56 100
sleep 1.779
noteoff 13 55 0
noteon 13 56 104
sleep 163.688
noteoff 11 68 0
sleep 7.116
noteoff 12 56 0
sleep 3.558
noteoff 2 74 0
noteoff 2 62 0
sleep 3.558
noteoff 13 56 0
sleep 3.558
noteoff 14 34 0
sleep 3.558
noteoff 3 56 0
noteoff 3 46 0
sleep 10.676
noteon 10 81 102
noteon 10 69 102
sleep 3.174
noteoff 4 62 0
noteon 4 57 100
noteon 11 69 102
sleep 1.587
noteon 2 61 101
noteon 6 69 108
noteon 2 73 101
noteon 6 57 108
sleep 4.761
noteoff 5 50 0
noteon 12 57 102
noteon 5 45 100
sleep 4.761
noteon 3 57 100
noteon 3 45 100
sleep 1.587
noteon 13 57 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.187
noteoff 12 57 0
sleep 6.348
noteoff 13 57 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 45 0
noteoff 3 57 0
sleep 9.522
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.337
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.251
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 12 69 0
sleep 3.558
noteoff 2 73 0
noteoff 2 61 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 57 0
noteoff 6 69 0
sleep 1.779
noteoff 3 57 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 3.174
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 74 101
noteon 2 62 101
noteon 6 62 108
noteon 6 74 108
sleep 4.761
noteon 5 50 100
noteon 12 74 102
sleep 4.761
noteon 3 62 100
sleep 1.587
noteon 13 62 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 50 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 79.365
noteoff 4 62 0
sleep 6.349
noteoff 5 50 0
sleep 6.349
noteon 10 74 102
sleep 3.558
noteon 4 66 100
noteon 11 62 102
sleep 1.779
noteoff 2 62 0
noteoff 2 74 0
noteoff 6 74 0
noteoff 6 62 0
sleep 5.338
noteoff 12 74 0
noteon 5 54 100
sleep 5.338
noteoff 3 62 0
sleep 1.779
noteoff 13 62 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 50 0
sleep 85.409
noteoff 10 74 0
sleep 3.558
noteoff 11 62 0
sleep 88.967
noteoff 4 66 0
sleep 7.117
noteoff 5 54 0
sleep 7.117
noteon 10 78 102
sleep 3.558
noteon 4 62 100
noteon 11 66 102
sleep 7.117
noteon 5 50 100
sleep 96.085
noteoff 10 78 0
sleep 3.558
noteoff 11 66 0
sleep 88.967
noteoff 4 62 0
sleep 7.117
noteoff 5 50 0
sleep 7.117
noteon 10 69 102
noteon 10 81 102
sleep 1.587
noteon 0 85 101
sleep 1.587
noteon 4 57 100
noteon 11 69 102
sleep 1.587
noteon 2 73 101
noteon 6 69 108
noteon 6 57 108
sleep 4.761
noteon 5 45 100
noteon 12 57 102
sleep 4.761
noteon 3 61 100
noteon 3 45 100
sleep 1.587
noteon 13 45 104
sleep 1.587
noteon 15 45 100
sleep 1.587
noteon 14 33 106
sleep 149.186
noteoff 12 57 0
sleep 6.348
noteoff 13 45 0
sleep 3.174
noteoff 14 33 0
sleep 3.174
noteoff 3 45 0
sleep 9.522
noteoff 10 81 0
noteon 10 80 102
sleep 3.558
noteoff 11 69 0
noteon 11 68 102
sleep 7.117
noteon 12 69 102
sleep 5.337
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 1.779
noteoff 15 45 0
sleep 1.779
noteon 14 45 106
sleep 167.251
noteoff 12 69 0
sleep 7.117
noteoff 13 57 0
sleep 3.558
noteoff 14 45 0
sleep 3.558
noteoff 3 57 0
sleep 10.676
noteoff 10 80 0
noteon 10 79 102
sleep 3.558
noteoff 11 68 0
noteon 11 67 102
sleep 7.117
noteon 12 69 102
sleep 5.338
noteon 3 57 100
sleep 1.779
noteon 13 57 104
sleep 3.558
noteon 14 45 106
sleep 156.583
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 67 0
sleep 7.117
noteoff 0 85 0
noteoff 12 69 0
sleep 3.558
noteoff 2 73 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 57 0
noteoff 14 45 0
sleep 1.779
noteoff 6 57 0
noteoff 6 69 0
sleep 1.779
noteoff 3 57 0
noteoff 3 61 0
sleep 3.558
noteoff 5 45 0
sleep 7.117
noteon 10 78 102
sleep 1.587
noteon 0 86 101
sleep 1.587
noteon 4 62 100
noteon 11 66 102
sleep 1.587
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
sleep 4.761
noteon 5 50 100
noteon 12 74 102
sleep 4.761
noteon 3 50 100
noteon 3 62 100
sleep 1.587
noteon 13 62 104
sleep 1.587
noteon 15 50 100
sleep 1.587
noteon 14 50 106
sleep 76.19
noteoff 10 78 0
sleep 3.174
noteoff 11 66 0
sleep 79.365
noteoff 4 62 0
sleep 6.349
noteoff 5 50 0
sleep 6.349
noteon 10 74 102
sleep 1.779
noteoff 0 86 0
sleep 1.779
noteon 4 66 100
noteon 11 62 102
sleep 1.779
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 12 74 0
noteon 5 54 100
sleep 5.338
noteoff 3 62 0
noteoff 3 50 0
sleep 1.779
noteoff 13 62 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 50 0
sleep 85.409
noteoff 10 74 0
sleep 3.558
noteoff 11 62 0
sleep 88.967
noteoff 4 66 0
sleep 7.117
noteoff 5 54 0
sleep 7.117
noteon 10 78 102
sleep 3.558
noteon 4 62 100
noteon 11 66 102
sleep 7.117
noteon 5 50 100
sleep 96.085
noteoff 10 78 0
sleep 3.558
noteoff 11 66 0
sleep 88.967
noteoff 4 62 0
sleep 7.117
noteoff 5 50 0
sleep 7.117
noteon 10 81 102
noteon 10 69 102
sleep 1.587
noteon 0 85 101
noteon 0 81 101
sleep 1.587
noteon 1 69 100
noteon 1 81 100
noteon 4 57 100
noteon 11 61 102
sleep 1.587
noteon 2 73 101
noteon 2 69 101
noteon 6 69 108
noteon 6 57 108
sleep 4.761
noteon 5 45 100
noteon 12 57 102
sleep 4.761
noteon 3 61 100
noteon 3 57 100
sleep 1.587
noteon 13 45 104
sleep 1.587
noteon 15 45 92
sleep 1.587
noteon 14 33 106
sleep 84.122
noteoff 15 45 0
sleep 9.523
noteon 15 45 92
sleep 81.330
noteoff 11 61 0
noteon 11 64 102
sleep 5.338
noteoff 15 45 0
sleep 1.779
noteoff 12 57 0
noteon 12 61 102
sleep 7.117
noteoff 13 45 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 33 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 90.747
noteoff 11 64 0
noteon 11 69 102
sleep 5.338
noteoff 15 45 0
sleep 1.779
noteoff 12 61 0
noteon 12 64 102
sleep 7.117
noteoff 13 57 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 45 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 90.362
noteoff 11 69 0
noteon 11 73 102
sleep 4.761
noteoff 15 45 0
sleep 1.587
noteoff 12 64 0
noteon 12 69 102
sleep 6.349
noteoff 13 57 0
noteon 13 57 104
sleep 1.587
noteon 15 45 92
sleep 1.587
noteoff 14 45 0
noteon 14 45 106
sleep 84.126
noteoff 15 45 0
sleep 9.523
noteon 15 45 92
sleep 65.079
noteoff 4 57 0
sleep 6.349
noteoff 5 45 0
sleep 6.349
noteoff 10 81 0
noteon 10 80 102
sleep 1.779
noteoff 0 81 0
noteoff 0 85 0
noteon 0 83 101
noteon 0 86 101
sleep 1.779
noteoff 1 81 0
noteoff 1 69 0
noteoff 11 73 0
noteon 1 80 100
noteon 1 68 100
noteon 4 64 100
noteon 11 74 102
sleep 1.779
noteoff 2 69 0
noteoff 2 73 0
noteon 2 74 101
noteon 2 71 101
sleep 3.558
noteoff 15 45 0
sleep 1.779
noteoff 12 69 0
noteon 5 57 100
noteon 12 71 102
sleep 5.338
noteoff 3 57 0
noteoff 3 61 0
noteon 3 62 100
noteon 3 59 100
sleep 1.779
noteoff 13 57 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 45 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 72.953
noteoff 4 64 0
sleep 7.117
noteoff 5 57 0
sleep 7.117
noteoff 10 80 0
noteon 10 79 102
sleep 1.779
noteoff 0 86 0
noteoff 0 83 0
noteon 0 88 101
noteon 0 85 101
sleep 1.779
noteoff 1 68 0
noteoff 1 80 0
noteoff 11 74 0
noteon 1 79 100
noteon 1 67 100
noteon 4 64 100
noteon 11 76 102
sleep 1.779
noteoff 2 71 0
noteoff 2 74 0
noteon 2 73 101
noteon 2 76 101
sleep 3.558
noteoff 15 45 0
sleep 1.779
noteoff 12 71 0
noteon 5 57 100
noteon 12 73 102
sleep 5.338
noteoff 3 59 0
noteoff 3 62 0
noteon 3 64 100
noteon 3 61 100
sleep 1.779
noteoff 13 57 0
noteon 13 57 104
sleep 1.779
noteon 15 45 92
sleep 1.779
noteoff 14 45 0
noteon 14 45 106
sleep 94.306
noteoff 15 45 0
sleep 10.676
noteon 15 45 92
sleep 51.601
noteoff 10 79 0
noteoff 10 69 0
sleep 3.558
noteoff 11 76 0
sleep 7.117
noteoff 0 85 0
noteoff 0 88 0
noteoff 12 73 0
sleep 1.779
noteoff 1 67 0
noteoff 1 79 0
sleep 1.779
noteoff 2 76 0
noteoff 2 73 0
sleep 3.558
noteoff 13 57 0
sleep 3.558
noteoff 4 64 0
noteoff 14 45 0
sleep 1.779
noteoff 6 57 0
noteoff 6 69 0
sleep 1.779
noteoff 3 61 0
noteoff 3 64 0
sleep 3.558
noteoff 5 57 0
sleep 7.117
noteon 10 78 102
sleep 1.7
noteon 0 90 101
noteon 0 78 101
sleep 1.7
noteon 1 78 100
noteon 1 66 100
noteon 4 62 100
noteon 11 78 102
sleep 1.7
noteon 2 78 101
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
sleep 3.401
noteoff 15 45 0
sleep 1.7
noteon 5 54 100
noteon 12 74 102
sleep 5.102
noteon 3 66 100
noteon 3 62 100
sleep 1.7
noteon 13 62 104
sleep 1.7
noteon 15 50 109
sleep 1.7
noteon 14 50 106
sleep 183.673
noteoff 10 78 0
sleep 1.779
noteoff 0 78 0
noteoff 0 90 0
sleep 1.779
noteoff 1 66 0
noteoff 1 78 0
noteoff 4 62 0
noteoff 11 78 0
sleep 1.779
noteoff 2 74 0
noteoff 2 78 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.338
noteoff 5 54 0
noteoff 12 74 0
sleep 5.338
noteoff 3 62 0
noteoff 3 66 0
sleep 1.779
noteoff 13 62 0
sleep 1.779
noteoff 15 50 0
sleep 1.779
noteoff 14 50 0
sleep 192.17
noteon 10 85 102
noteon 10 69 102
sleep 1.992
noteon 0 85 101
sleep 1.992
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
noteon 11 79 102
noteon 11 69 102
sleep 1.992
noteon 2 76 101
noteon 2 73 101
noteon 6 57 108
noteon 6 69 108
sleep 5.976
noteon 5 57 100
noteon 12 69 102
sleep 5.976
noteon 3 57 100
sleep 1.992
noteon 13 57 104
sleep 1.992
noteon 15 45 92
sleep 1.992
noteon 14 45 106
sleep 95.617
noteoff 10 69 0
noteoff 10 85 0
sleep 1.992
noteoff 0 85 0
sleep 1.992
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 11 69 0
noteoff 11 79 0
sleep 1.992
noteoff 2 73 0
noteoff 2 76 0
noteoff 6 69 0
noteoff 6 57 0
sleep 3.984
noteoff 15 45 0
sleep 1.992
noteoff 5 57 0
noteoff 12 69 0
sleep 5.976
noteoff 3 57 0
sleep 1.992
noteoff 13 57 0
sleep 3.984
noteoff 14 45 0
sleep 95.617
noteon 10 62 102
noteon 10 69 102
noteon 10 86 102
sleep 2.976
noteon 0 86 101
sleep 2.976
noteon 1 74 100
noteon 1 78 100
noteon 4 62 100
noteon 11 69 102
noteon 11 78 102
noteon 11 62 102
sleep 2.976
noteon 2 74 101
noteon 6 74 108
noteon 6 62 108
sleep 8.928
noteon 5 54 100
noteon 12 62 102
sleep 8.928
noteon 3 62 100
noteon 3 50 100
sleep 2.976
noteon 13 50 104
sleep 2.976
noteon 15 50 92
sleep 2.976
noteon 14 38 106
sleep 142.857
noteoff 10 86 0
noteoff 10 69 0
noteoff 10 62 0
sleep 2.976
noteoff 0 86 0
sleep 2.976
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 62 0
noteoff 11 78 0
noteoff 11 69 0
sleep 2.976
noteoff 2 74 0
noteoff 6 62 0
noteoff 6 74 0
sleep 5.952
noteoff 15 50 0
sleep 2.976
noteoff 5 54 0
noteoff 12 62 0
sleep 8.928
noteoff 3 50 0
noteoff 3 62 0
sleep 2.976
noteoff 13 50 0
sleep 5.952
noteoff 14 38 0
sleep 1342.850
quit
