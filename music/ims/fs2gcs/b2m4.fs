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
echo "Header 20 120"
echo "Title 'Symphony n^0 2 in D major'"
echo "Title 'IV : Allegro molto'"
echo "L. Van Beethoven (1770-1827)"
echo "Tempo sup'erieur `a 250 : pr'evoir correction avant publication\012"
echo "Partition compl`ete - dur'ee : 00:06:20\012"
echo "D'ebut du travail : 29 septembre 2000\012"
echo "Fin de la version 0 : 18 octobre 2000\012"
echo "Version 1.0 : 24 octobre 2000\012"
echo "Version 1.01 : 17 janvier 2001 : r'evision avant publcation\012"
echo "Dur'ee d''edition et d'audition : 19 h\012"
echo "-----\012"
echo "Full score - length : 00:06:20\012"
echo "Beginning of writing : 29 september 2000\012"
echo "End of version 0 : 18 october 2000\012"
echo "Version 1.0 : 24 october 2000\012"
echo "Version 1.01 : 17 january 2001 : revision before publcation\012"
echo "Editing and audition time : 19 h\012"
echo "meter 4 2 24 8"
echo "key 0 D 'major'"
echo "0 tempo_s=240 tempo_l=0.25"
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
echo "Title 'Beethoven - Symphony no 2 in D - IV : Allegro molto'"
sleep 10.415
select 10 1 0 48
sleep 2.083
select 0 1 0 73
sleep 2.083
select 1 1 0 68
select 4 1 0 60
select 11 1 0 48
sleep 2.083
select 6 1 0 56
sleep 4.166
select 2 1 0 71
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
pitch_bend 6 8192
sleep 4.166
pitch_bend 2 8192
sleep 2.083
pitch_bend 5 8092
pitch_bend 12 8492
select 13 1 0 48
sleep 2.083
select 3 1 0 70
sleep 2.083
select 15 1 0 47
sleep 2.083
select 14 1 0 48
sleep 4.166
pitch_bend 13 7892
sleep 2.083
pitch_bend 3 8192
sleep 2.083
pitch_bend 15 8192
sleep 2.083
pitch_bend 14 8492
sleep 699.993
echo "360 tempo_s=307 tempo_l=0.25"
sleep 97.719
noteon 10 78 102
sleep 1.628
noteon 0 90 101
sleep 1.628
noteon 1 78 100
noteon 11 66 102
sleep 16.286
noteon 3 66 100
sleep 78.175
echo "480 tempo_s=278 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteoff 11 66 0
noteon 1 79 100
noteon 11 67 102
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 86.33
noteoff 10 79 0
sleep 1.798
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
noteoff 11 67 0
sleep 17.985
noteoff 3 67 0
sleep 86.33
echo "600 tempo_s=246 tempo_l=0.25"
sleep 121.951
noteon 10 61 102
sleep 2.032
noteon 0 73 101
sleep 2.032
noteon 1 73 100
noteon 11 61 102
sleep 8.13
noteon 12 61 102
sleep 10.162
noteon 13 49 104
sleep 2.032
noteon 3 49 100
sleep 4.065
noteon 14 37 106
sleep 73.17
noteoff 10 61 0
sleep 2.032
noteoff 0 73 0
sleep 2.032
noteoff 1 73 0
noteoff 11 61 0
sleep 8.13
noteoff 12 61 0
sleep 8.13
echo "720 tempo_s=310 tempo_l=0.25"
noteon 10 62 102
sleep 1.612
noteoff 13 49 0
noteon 0 74 101
sleep 1.612
noteoff 3 49 0
noteon 1 74 100
noteon 11 62 102
sleep 3.225
noteoff 14 37 0
sleep 3.224
noteon 12 62 102
sleep 8.063
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 3.224
noteon 14 38 106
sleep 4.837
noteoff 0 74 0
noteon 0 73 101
sleep 4.836
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 6.448
noteoff 12 62 0
noteon 12 61 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 1.612
noteoff 0 73 0
noteon 0 74 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.450
noteoff 12 61 0
noteon 12 62 102
sleep 4.836
noteoff 0 74 0
noteon 0 73 101
sleep 8.062
noteoff 14 38 0
noteon 14 37 106
sleep 9.674
noteoff 10 62 0
noteon 10 61 102
sleep 3.224
noteoff 1 74 0
noteoff 11 61 0
noteon 1 73 100
noteon 11 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 8.062
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 12.898
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 1 73 0
noteon 1 74 100
sleep 6.448
noteoff 12 61 0
noteon 12 62 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 4.836
noteoff 0 73 0
noteon 0 74 101
sleep 4.838
noteoff 10 62 0
noteon 10 61 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 74 0
noteoff 3 50 0
noteon 1 73 100
noteon 3 49 100
sleep 6.450
noteoff 12 62 0
noteon 12 61 102
sleep 11.286
noteoff 0 74 0
noteon 0 73 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 3.224
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 6.449
noteoff 12 61 0
noteon 12 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 3.224
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.224
noteoff 14 38 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 4.838
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 9.677
noteoff 11 62 0
noteon 11 61 102
sleep 12.903
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 73 0
noteoff 13 50 0
noteon 0 74 101
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.451
noteoff 12 61 0
noteon 12 62 102
sleep 12.903
noteoff 14 37 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 59 102
sleep 1.612
noteoff 0 74 0
noteon 0 71 101
sleep 1.612
noteoff 1 74 0
noteoff 11 61 0
noteon 1 71 100
noteon 11 59 102
sleep 3.225
noteoff 14 37 0
sleep 3.225
noteoff 12 62 0
noteon 12 59 102
sleep 8.064
noteoff 13 49 0
noteon 13 47 104
sleep 1.612
noteoff 3 49 0
noteon 3 47 100
sleep 3.225
noteon 14 35 106
sleep 25.806
noteoff 10 59 0
noteon 10 61 102
sleep 1.612
noteoff 0 71 0
noteon 0 73 101
sleep 1.612
noteoff 1 71 0
noteoff 11 59 0
noteon 1 73 100
noteon 11 61 102
sleep 6.451
noteoff 12 59 0
noteon 12 61 102
sleep 8.064
noteoff 13 47 0
noteon 13 49 104
sleep 1.612
noteoff 3 47 0
noteon 3 49 100
sleep 3.225
noteoff 14 35 0
noteon 14 37 106
sleep 25.806
echo "960 tempo_s=278 tempo_l=0.25"
noteoff 10 61 0
noteon 10 64 102
sleep 1.798
noteoff 0 73 0
noteon 0 76 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 76 100
noteon 11 64 102
sleep 7.194
noteoff 12 61 0
noteon 12 64 102
sleep 8.992
noteoff 13 49 0
noteon 13 52 104
sleep 1.798
noteoff 3 49 0
noteon 3 52 100
sleep 3.597
noteoff 14 37 0
noteon 14 40 106
sleep 82.733
noteoff 10 64 0
sleep 1.798
noteoff 0 76 0
sleep 1.798
noteoff 1 76 0
noteoff 11 64 0
sleep 7.194
noteoff 12 64 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 3.597
noteoff 14 40 0
sleep 82.733
noteon 10 57 102
sleep 1.798
noteon 0 69 101
sleep 1.798
noteon 1 69 100
noteon 11 57 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
sleep 3.597
noteon 14 33 106
sleep 82.733
noteoff 10 57 0
sleep 1.798
noteoff 0 69 0
sleep 1.798
noteoff 1 69 0
noteoff 11 57 0
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
sleep 3.597
noteoff 14 33 0
sleep 82.733
echo "1200 tempo_s=310 tempo_l=0.25"
sleep 193.548
noteon 10 69 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteon 12 67 102
sleep 8.064
noteon 13 67 104
sleep 30.645
noteoff 10 69 0
sleep 48.387
noteon 10 69 102
sleep 48.387
noteoff 10 69 0
sleep 48.387
echo "1440 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 10.791
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 67 0
noteon 13 66 104
sleep 88.129
noteoff 10 74 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 17.985
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
echo "1680 tempo_s=310 tempo_l=0.25"
noteon 10 76 102
sleep 9.677
noteoff 12 66 0
noteon 12 61 102
sleep 8.064
noteoff 13 66 0
noteon 13 61 104
sleep 79.032
noteoff 10 76 0
noteon 10 69 102
sleep 80.645
noteoff 10 69 0
sleep 16.129
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 48.387
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 41.935
noteoff 12 61 0
sleep 6.451
echo "1920 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 1.798
noteoff 13 61 0
sleep 8.992
noteon 12 62 102
sleep 8.992
noteon 13 62 104
sleep 88.129
noteoff 10 78 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 17.985
noteon 10 81 102
sleep 3.597
noteoff 11 69 0
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
echo "2160 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 3.225
noteon 11 71 102
sleep 6.451
noteoff 12 62 0
noteon 12 67 102
sleep 8.064
noteoff 13 62 0
noteon 13 55 104
sleep 4.838
noteon 14 31 106
sleep 74.193
noteoff 10 81 0
noteon 10 79 102
sleep 80.645
noteoff 10 79 0
sleep 16.129
noteon 10 78 102
sleep 22.58
noteoff 14 31 0
sleep 25.806
noteoff 10 78 0
sleep 48.387
noteon 10 76 102
sleep 48.387
noteoff 10 76 0
sleep 48.387
echo "2400 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 3.597
noteoff 11 71 0
noteon 11 69 102
sleep 7.194
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 55 0
noteon 13 57 104
sleep 5.395
noteon 14 33 106
sleep 82.733
noteoff 10 76 0
noteon 10 74 102
sleep 89.928
noteoff 10 74 0
sleep 17.985
noteon 10 74 102
sleep 25.179
noteoff 14 33 0
sleep 28.776
noteoff 10 74 0
sleep 53.956
noteon 10 74 102
sleep 53.956
noteoff 10 74 0
sleep 53.956
echo "2640 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 3.225
noteoff 11 69 0
noteon 4 64 115
noteon 11 67 102
sleep 6.451
noteoff 12 66 0
noteon 5 57 115
noteon 12 64 102
sleep 8.064
noteoff 13 57 0
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 74.193
noteoff 10 74 0
noteon 10 73 102
sleep 80.645
noteoff 10 73 0
sleep 16.129
noteon 10 71 102
sleep 22.58
noteoff 14 33 0
sleep 25.806
noteoff 10 71 0
sleep 48.387
noteon 10 73 102
sleep 48.387
noteoff 10 73 0
sleep 35.483
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
echo "2880 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteoff 13 45 0
noteon 0 86 101
sleep 1.798
noteoff 4 64 0
noteon 1 74 100
noteon 1 66 100
noteon 11 66 102
noteon 4 62 100
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 74 101
noteon 2 66 101
sleep 1.798
noteoff 5 57 0
noteon 12 62 102
noteon 5 54 100
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 62 100
noteon 3 50 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 82.733
noteoff 10 74 0
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteoff 1 66 0
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 66 0
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 3.597
noteoff 2 66 0
noteoff 2 74 0
sleep 1.798
noteoff 5 54 0
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
noteoff 3 62 0
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 82.733
noteon 10 67 102
noteon 10 76 102
sleep 1.798
noteon 0 85 101
noteon 0 88 101
sleep 1.798
noteon 1 76 100
noteon 1 69 100
noteon 4 64 100
noteon 11 61 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 76 101
noteon 2 69 101
sleep 1.798
noteon 5 57 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 57 100
noteon 3 45 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 82.733
noteoff 10 76 0
noteoff 10 67 0
sleep 1.798
noteoff 0 88 0
noteoff 0 85 0
sleep 1.798
noteoff 1 69 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 11 61 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 69 0
noteoff 2 76 0
sleep 1.798
noteoff 5 57 0
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
noteoff 3 57 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 82.733
echo "3120 tempo_s=257 tempo_l=0.25"
sleep 233.463
echo "3240 tempo_s=310 tempo_l=0.25"
sleep 96.774
noteon 10 78 102
sleep 1.612
noteon 0 90 101
sleep 1.612
noteon 1 78 100
noteon 11 66 102
sleep 16.127
noteon 3 66 100
sleep 77.418
echo "3360 tempo_s=278 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteoff 11 66 0
noteon 1 79 100
noteon 11 67 102
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 86.33
noteoff 10 79 0
sleep 1.798
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
noteoff 11 67 0
sleep 17.985
noteoff 3 67 0
sleep 86.33
echo "3480 tempo_s=246 tempo_l=0.25"
sleep 121.951
noteon 10 61 102
sleep 2.032
noteon 0 73 101
sleep 2.032
noteon 1 73 100
noteon 11 61 102
sleep 8.13
noteon 12 61 102
sleep 10.162
noteon 13 49 104
sleep 2.032
noteon 3 49 100
sleep 4.065
noteon 14 37 106
sleep 73.17
noteoff 10 61 0
sleep 2.032
noteoff 0 73 0
sleep 2.032
noteoff 1 73 0
noteoff 11 61 0
sleep 8.13
noteoff 12 61 0
sleep 8.13
echo "3600 tempo_s=310 tempo_l=0.25"
noteon 10 62 102
sleep 1.612
noteoff 13 49 0
noteon 0 74 101
sleep 1.612
noteoff 3 49 0
noteon 1 74 100
noteon 11 62 102
sleep 3.225
noteoff 14 37 0
sleep 3.224
noteon 12 62 102
sleep 8.063
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 3.224
noteon 14 38 106
sleep 4.837
noteoff 0 74 0
noteon 0 73 101
sleep 4.836
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 6.448
noteoff 12 62 0
noteon 12 61 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 1.612
noteoff 0 73 0
noteon 0 74 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.450
noteoff 12 61 0
noteon 12 62 102
sleep 4.836
noteoff 0 74 0
noteon 0 73 101
sleep 8.062
noteoff 14 38 0
noteon 14 37 106
sleep 9.674
noteoff 10 62 0
noteon 10 61 102
sleep 3.224
noteoff 1 74 0
noteoff 11 61 0
noteon 1 73 100
noteon 11 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 8.062
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 12.898
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 1 73 0
noteon 1 74 100
sleep 6.448
noteoff 12 61 0
noteon 12 62 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 4.836
noteoff 0 73 0
noteon 0 74 101
sleep 4.838
noteoff 10 62 0
noteon 10 61 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 74 0
noteoff 3 50 0
noteon 1 73 100
noteon 3 49 100
sleep 6.450
noteoff 12 62 0
noteon 12 61 102
sleep 11.286
noteoff 0 74 0
noteon 0 73 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 3.224
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 6.449
noteoff 12 61 0
noteon 12 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 3.224
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.224
noteoff 14 38 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 4.838
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 9.677
noteoff 11 62 0
noteon 11 61 102
sleep 12.903
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 73 0
noteoff 13 50 0
noteon 0 74 101
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.451
noteoff 12 61 0
noteon 12 62 102
sleep 12.903
noteoff 14 37 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 59 102
sleep 1.612
noteoff 0 74 0
noteon 0 71 101
sleep 1.612
noteoff 1 74 0
noteoff 11 61 0
noteon 1 71 100
noteon 11 59 102
sleep 3.225
noteoff 14 37 0
sleep 3.225
noteoff 12 62 0
noteon 12 59 102
sleep 8.064
noteoff 13 49 0
noteon 13 47 104
sleep 1.612
noteoff 3 49 0
noteon 3 47 100
sleep 3.225
noteon 14 35 106
sleep 25.806
noteoff 10 59 0
noteon 10 61 102
sleep 1.612
noteoff 0 71 0
noteon 0 73 101
sleep 1.612
noteoff 1 71 0
noteoff 11 59 0
noteon 1 73 100
noteon 11 61 102
sleep 6.451
noteoff 12 59 0
noteon 12 61 102
sleep 8.064
noteoff 13 47 0
noteon 13 49 104
sleep 1.612
noteoff 3 47 0
noteon 3 49 100
sleep 3.225
noteoff 14 35 0
noteon 14 37 106
sleep 25.806
echo "3840 tempo_s=278 tempo_l=0.25"
noteoff 10 61 0
noteon 10 64 102
sleep 1.798
noteoff 0 73 0
noteon 0 76 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 76 100
noteon 4 57 100
noteon 11 64 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 64 101
noteon 2 76 101
sleep 1.798
noteoff 12 61 0
noteon 5 45 100
noteon 12 64 102
sleep 8.992
noteoff 13 49 0
noteon 13 52 104
sleep 1.798
noteoff 3 49 0
noteon 3 52 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteoff 14 37 0
noteon 14 40 106
sleep 82.733
noteoff 10 64 0
sleep 1.798
noteoff 0 76 0
sleep 1.798
noteoff 1 76 0
noteoff 4 57 0
noteoff 11 64 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 76 0
noteoff 2 64 0
sleep 1.798
noteoff 5 45 0
noteoff 12 64 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 40 0
sleep 82.733
noteon 10 57 102
sleep 1.798
noteon 0 69 101
sleep 1.798
noteon 1 69 100
noteon 4 57 100
noteon 11 57 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 57 101
noteon 2 69 101
sleep 1.798
noteon 5 45 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 82.733
noteoff 10 57 0
sleep 1.798
noteoff 0 69 0
sleep 1.798
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 57 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 69 0
noteoff 2 57 0
sleep 1.798
noteoff 5 45 0
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 82.733
echo "4080 tempo_s=310 tempo_l=0.25"
sleep 193.548
noteon 10 69 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteon 12 67 102
sleep 8.064
noteon 13 67 104
sleep 30.645
noteoff 10 69 0
sleep 48.387
noteon 10 69 102
sleep 48.387
noteoff 10 69 0
sleep 48.387
echo "4320 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 10.791
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 67 0
noteon 13 66 104
sleep 88.129
noteoff 10 74 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 17.985
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
echo "4560 tempo_s=310 tempo_l=0.25"
noteon 10 76 102
sleep 9.677
noteoff 12 66 0
noteon 12 61 102
sleep 8.064
noteoff 13 66 0
noteon 13 61 104
sleep 79.032
noteoff 10 76 0
noteon 10 69 102
sleep 80.645
noteoff 10 69 0
sleep 16.129
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 48.387
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 41.935
noteoff 12 61 0
sleep 6.451
echo "4800 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 1.798
noteoff 13 61 0
sleep 8.992
noteon 12 62 102
sleep 8.992
noteon 13 62 104
sleep 88.129
noteoff 10 78 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 3.597
noteoff 11 69 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
echo "5040 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 3.225
noteon 11 71 102
sleep 6.451
noteoff 12 62 0
noteon 12 67 102
sleep 8.064
noteoff 13 62 0
noteon 13 55 104
sleep 4.838
noteon 14 31 106
sleep 74.193
noteoff 10 81 0
noteon 10 79 102
sleep 80.645
noteoff 10 79 0
sleep 16.129
noteon 10 78 102
sleep 22.58
noteoff 14 31 0
sleep 25.806
noteoff 10 78 0
sleep 48.387
noteon 10 76 102
sleep 48.387
noteoff 10 76 0
sleep 48.387
echo "5280 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 3.597
noteoff 11 71 0
noteon 11 69 102
sleep 7.194
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 55 0
noteon 13 57 104
sleep 5.395
noteon 14 33 106
sleep 82.733
noteoff 10 76 0
noteon 10 74 102
sleep 89.928
noteoff 10 74 0
sleep 17.985
noteon 10 74 102
sleep 25.179
noteoff 14 33 0
sleep 28.776
noteoff 10 74 0
sleep 53.956
noteon 10 74 102
sleep 53.956
noteoff 10 74 0
sleep 53.956
echo "5520 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 3.225
noteoff 11 69 0
noteon 4 64 115
noteon 11 67 102
sleep 4.838
noteon 2 73 116
noteon 2 67 116
sleep 1.612
noteoff 12 66 0
noteon 5 57 115
noteon 12 64 102
sleep 8.064
noteoff 13 57 0
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 74.193
noteoff 10 74 0
noteon 10 73 102
sleep 80.645
noteoff 10 73 0
sleep 16.129
noteon 10 71 102
sleep 22.58
noteoff 14 33 0
sleep 25.806
noteoff 10 71 0
sleep 48.387
echo "5700 tempo_s=235 tempo_l=0.25"
noteon 10 73 102
sleep 63.829
noteoff 10 73 0
sleep 46.808
noteoff 11 67 0
sleep 8.51
noteoff 12 64 0
sleep 8.51
echo "5760 tempo_s=278 tempo_l=0.25"
noteon 10 62 102
noteon 10 74 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteoff 4 64 0
noteon 11 66 102
noteon 4 62 100
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteoff 2 67 0
noteoff 2 73 0
noteon 2 66 101
noteon 2 74 101
sleep 1.798
noteoff 5 57 0
noteon 12 62 102
noteon 5 50 100
sleep 8.992
noteon 13 50 104
sleep 3.597
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 190.647
noteoff 10 74 0
noteoff 10 62 0
sleep 3.597
noteoff 11 66 0
sleep 5.395
noteoff 2 74 0
noteoff 2 66 0
sleep 1.798
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 3.597
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 84.532
noteon 0 85 101
sleep 1.798
noteon 1 73 100
sleep 17.985
noteon 3 61 100
sleep 86.33
echo "6000 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 0 85 0
noteon 0 86 101
sleep 1.612
noteoff 1 73 0
noteon 1 74 100
sleep 16.129
noteoff 3 61 0
noteon 3 62 100
sleep 79.032
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
sleep 16.129
noteoff 3 62 0
sleep 174.193
noteon 10 62 102
sleep 3.225
noteon 11 62 102
sleep 6.451
noteon 12 54 102
sleep 8.064
noteon 13 54 104
sleep 4.838
noteon 14 42 106
sleep 25.806
noteoff 10 62 0
sleep 3.225
noteoff 11 62 0
sleep 6.451
noteoff 12 54 0
sleep 8.064
noteoff 13 54 0
sleep 4.838
noteoff 14 42 0
sleep 25.806
echo "6240 tempo_s=278 tempo_l=0.25"
noteon 10 64 102
sleep 3.597
noteon 11 64 102
sleep 7.194
noteon 12 55 102
sleep 8.992
noteon 13 55 104
sleep 5.395
noteon 14 43 106
sleep 82.733
noteoff 10 64 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 55 0
sleep 8.992
noteoff 13 55 0
sleep 5.395
noteoff 14 43 0
sleep 82.733
noteon 10 66 102
sleep 3.597
noteon 11 66 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 82.733
noteoff 10 66 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 5.395
noteoff 14 45 0
sleep 82.733
echo "6480 tempo_s=310 tempo_l=0.25"
noteon 10 67 102
sleep 3.225
noteon 11 67 102
sleep 6.451
noteon 12 59 102
sleep 8.064
noteon 13 59 104
sleep 4.838
noteon 14 47 106
sleep 74.193
noteoff 10 67 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 59 0
sleep 8.064
noteoff 13 59 0
sleep 4.838
noteoff 14 47 0
sleep 74.193
noteon 10 65 102
sleep 3.225
noteon 11 65 102
sleep 6.451
noteon 12 56 102
sleep 8.064
noteon 13 56 104
sleep 4.838
noteon 14 44 106
sleep 74.193
noteoff 10 65 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 56 0
sleep 8.064
noteoff 13 56 0
sleep 4.838
noteoff 14 44 0
sleep 74.193
echo "6720 tempo_s=278 tempo_l=0.25"
noteon 10 66 102
sleep 3.597
noteoff 4 62 0
noteon 11 66 102
noteon 4 62 100
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 5.395
noteoff 5 50 0
noteon 12 57 102
noteon 5 50 100
sleep 8.992
noteon 13 57 104
sleep 3.597
noteon 15 50 90
sleep 1.798
noteon 14 45 106
sleep 190.605
noteoff 10 66 0
sleep 3.596
noteoff 11 66 0
sleep 7.192
noteoff 12 57 0
sleep 8.991
noteoff 13 57 0
sleep 3.597
noteoff 15 50 0
sleep 1.798
noteoff 14 45 0
sleep 84.525
noteon 0 85 101
sleep 1.798
noteon 1 73 100
sleep 17.984
noteon 3 61 100
sleep 86.324
echo "6960 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 0 85 0
noteon 0 86 101
sleep 1.612
noteoff 1 73 0
noteon 1 74 100
sleep 16.127
noteoff 3 61 0
noteon 3 62 100
sleep 79.032
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
sleep 16.129
noteoff 3 62 0
sleep 174.193
noteon 10 66 102
sleep 3.225
noteon 11 66 102
sleep 6.451
noteon 12 58 102
sleep 8.064
noteon 13 58 104
sleep 4.838
noteon 14 46 106
sleep 25.806
noteoff 10 66 0
sleep 3.225
noteoff 11 66 0
sleep 6.451
noteoff 12 58 0
sleep 8.064
noteoff 13 58 0
sleep 4.838
noteoff 14 46 0
sleep 25.806
echo "7200 tempo_s=278 tempo_l=0.25"
noteon 10 67 102
sleep 3.597
noteon 11 67 102
sleep 7.194
noteon 12 59 102
sleep 8.992
noteon 13 59 104
sleep 5.395
noteon 14 47 106
sleep 82.733
noteoff 10 67 0
sleep 3.597
noteoff 11 67 0
sleep 7.194
noteoff 12 59 0
sleep 8.992
noteoff 13 59 0
sleep 5.395
noteoff 14 47 0
sleep 82.733
noteon 10 69 102
sleep 3.597
noteon 11 57 102
sleep 7.194
noteon 12 54 102
sleep 8.992
noteon 13 54 104
sleep 5.395
noteon 14 42 106
sleep 82.733
noteoff 10 69 0
sleep 3.597
noteoff 11 57 0
sleep 7.194
noteoff 12 54 0
sleep 8.992
noteoff 13 54 0
sleep 5.395
noteoff 14 42 0
sleep 82.733
echo "7440 tempo_s=310 tempo_l=0.25"
noteon 10 71 102
sleep 3.225
noteon 11 59 102
sleep 6.451
noteon 12 55 102
sleep 8.064
noteon 13 55 104
sleep 4.838
noteon 14 43 106
sleep 74.193
noteoff 10 71 0
sleep 3.225
noteoff 11 59 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 55 0
sleep 4.838
noteoff 14 43 0
sleep 74.193
noteon 10 73 102
sleep 3.225
noteon 11 61 102
sleep 6.451
noteon 12 52 102
sleep 8.064
noteon 13 52 104
sleep 4.838
noteon 14 40 106
sleep 74.193
noteoff 10 73 0
sleep 3.225
noteoff 11 61 0
sleep 6.451
noteoff 12 52 0
sleep 8.064
noteoff 13 52 0
sleep 4.838
noteoff 14 40 0
sleep 74.193
echo "7680 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteon 0 86 101
sleep 1.798
noteoff 4 62 0
noteon 1 74 100
noteon 11 62 102
noteon 4 62 100
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 5.395
noteoff 5 50 0
noteon 12 50 102
noteon 5 50 100
sleep 8.991
noteon 13 50 104
sleep 1.798
noteon 3 62 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteon 14 38 106
sleep 84.510
noteoff 0 86 0
noteon 0 85 101
sleep 1.798
noteoff 1 74 0
noteon 1 73 100
sleep 17.981
noteoff 3 62 0
noteon 3 61 100
sleep 86.308
noteoff 10 74 0
sleep 1.798
noteoff 0 85 0
noteon 0 86 101
sleep 1.798
noteoff 1 73 0
noteoff 11 62 0
noteon 1 74 100
sleep 7.192
noteoff 12 50 0
sleep 8.991
noteoff 13 50 0
sleep 1.798
noteoff 3 61 0
noteon 3 62 100
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 84.525
noteoff 0 86 0
noteon 0 85 101
sleep 1.798
noteoff 1 74 0
noteon 1 73 100
sleep 17.984
noteoff 3 62 0
noteon 3 61 100
sleep 79.130
noteoff 0 85 0
sleep 1.798
noteoff 1 73 0
sleep 5.395
echo "7920 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 86 101
sleep 1.612
noteon 1 74 100
sleep 8.062
noteoff 3 61 0
sleep 8.064
noteon 3 62 100
sleep 79.032
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
sleep 16.129
noteoff 3 62 0
sleep 77.419
noteon 10 74 102
sleep 3.225
noteon 11 62 102
sleep 6.451
noteon 12 54 102
sleep 8.064
noteon 13 54 104
sleep 4.838
noteon 14 42 106
sleep 58.064
noteoff 10 74 0
sleep 3.225
noteoff 11 62 0
sleep 12.903
noteon 10 74 102
sleep 3.225
noteon 11 62 102
sleep 6.451
noteoff 12 54 0
sleep 8.064
noteoff 13 54 0
sleep 4.838
noteoff 14 42 0
sleep 58.064
noteoff 10 74 0
sleep 3.225
noteoff 11 62 0
sleep 12.903
echo "8160 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 3.597
noteon 11 64 102
sleep 7.194
noteon 12 55 102
sleep 8.992
noteon 13 55 104
sleep 5.395
noteon 14 43 106
sleep 64.748
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
sleep 14.388
noteon 10 76 102
sleep 3.597
noteon 11 64 102
sleep 7.194
noteoff 12 55 0
sleep 8.992
noteoff 13 55 0
sleep 5.395
noteoff 14 43 0
sleep 64.748
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
sleep 14.388
noteon 10 78 102
sleep 3.597
noteon 11 66 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 14.388
noteon 10 78 102
sleep 3.597
noteon 11 66 102
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 5.395
noteoff 14 45 0
sleep 64.748
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 14.388
echo "8400 tempo_s=310 tempo_l=0.25"
noteon 10 79 102
sleep 3.225
noteon 11 67 102
sleep 6.451
noteon 12 59 102
sleep 8.064
noteon 13 59 104
sleep 4.838
noteon 14 47 106
sleep 58.064
noteoff 10 79 0
sleep 3.225
noteoff 11 67 0
sleep 12.903
noteon 10 79 102
sleep 3.225
noteon 11 67 102
sleep 6.451
noteoff 12 59 0
sleep 8.064
noteoff 13 59 0
sleep 4.838
noteoff 14 47 0
sleep 58.064
noteoff 10 79 0
sleep 3.225
noteoff 11 67 0
sleep 12.903
noteon 10 77 102
sleep 3.225
noteon 11 65 102
sleep 6.451
noteon 12 56 102
sleep 8.064
noteon 13 56 104
sleep 4.838
noteon 14 44 106
sleep 58.064
noteoff 10 77 0
sleep 3.225
noteoff 11 65 0
sleep 12.903
noteon 10 77 102
sleep 3.225
noteon 11 65 102
sleep 6.451
noteoff 12 56 0
sleep 8.064
noteoff 13 56 0
sleep 4.838
noteoff 14 44 0
sleep 58.064
noteoff 10 77 0
sleep 3.225
noteoff 11 65 0
sleep 12.903
echo "8640 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 1.798
noteon 0 86 101
sleep 1.798
noteoff 4 62 0
noteon 1 74 100
noteon 11 66 102
noteon 4 62 100
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 5.395
noteoff 5 50 0
noteon 12 57 102
noteon 5 50 100
sleep 8.991
noteon 13 57 104
sleep 1.798
noteon 3 62 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteon 14 45 106
sleep 84.510
noteoff 0 86 0
noteon 0 85 101
sleep 1.798
noteoff 1 74 0
noteon 1 73 100
sleep 17.981
noteoff 3 62 0
noteon 3 61 100
sleep 86.308
noteoff 10 78 0
sleep 1.798
noteoff 0 85 0
noteon 0 86 101
sleep 1.798
noteoff 1 73 0
noteoff 11 66 0
noteon 1 74 100
sleep 7.192
noteoff 12 57 0
sleep 8.991
noteoff 13 57 0
sleep 1.798
noteoff 3 61 0
noteon 3 62 100
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 45 0
sleep 84.525
noteoff 0 86 0
noteon 0 85 101
sleep 1.798
noteoff 1 74 0
noteon 1 73 100
sleep 17.984
noteoff 3 62 0
noteon 3 61 100
sleep 79.130
noteoff 0 85 0
sleep 1.798
noteoff 1 73 0
sleep 5.395
echo "8880 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 86 101
sleep 1.612
noteon 1 74 100
sleep 8.062
noteoff 3 61 0
sleep 8.064
noteon 3 62 100
sleep 79.032
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
sleep 16.129
noteoff 3 62 0
sleep 77.419
noteon 10 78 102
sleep 1.612
noteon 0 86 101
noteon 0 78 101
sleep 1.612
noteoff 4 62 0
noteon 1 78 100
noteon 1 74 100
noteon 4 62 100
noteon 11 66 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
noteon 6 74 108
noteon 6 62 108
sleep 3.225
noteon 2 74 101
noteon 2 66 101
sleep 1.612
noteoff 5 50 0
noteon 5 50 100
noteon 12 60 102
sleep 8.064
noteon 13 60 104
sleep 1.612
noteon 3 60 100
noteon 3 48 100
sleep 1.612
noteon 15 50 80
sleep 1.612
noteon 14 48 106
sleep 58.064
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 6.451
noteoff 12 60 0
sleep 6.451
noteon 10 78 102
sleep 1.612
noteoff 0 78 0
noteoff 0 86 0
noteoff 15 50 0
sleep 1.612
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 62 0
noteon 11 66 102
sleep 1.612
noteoff 6 62 0
noteoff 6 74 0
sleep 3.225
noteoff 2 66 0
noteoff 2 74 0
sleep 1.612
noteoff 5 50 0
noteon 12 60 102
sleep 8.064
noteoff 13 60 0
sleep 1.612
noteoff 3 48 0
noteoff 3 60 0
sleep 1.612
noteon 15 50 90
sleep 1.612
noteoff 14 48 0
sleep 58.064
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 6.451
noteoff 12 60 0
sleep 6.451
echo "9120 tempo_s=278 tempo_l=0.25"
noteon 10 79 102
sleep 1.798
noteoff 15 50 0
noteon 0 86 101
noteon 0 79 101
sleep 1.798
noteon 1 79 100
noteon 1 74 100
noteon 4 62 100
noteon 11 67 102
sleep 1.798
noteon 6 74 108
noteon 6 62 108
sleep 3.597
noteon 2 74 101
noteon 2 67 101
sleep 1.798
noteon 5 50 100
noteon 12 59 102
sleep 8.992
noteon 13 59 104
sleep 1.798
noteon 3 59 100
noteon 3 47 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteon 14 47 106
sleep 64.748
noteoff 10 79 0
sleep 3.597
noteoff 11 67 0
sleep 7.194
noteoff 12 59 0
sleep 7.194
noteon 10 79 102
sleep 1.798
noteoff 0 79 0
noteoff 0 86 0
noteoff 15 50 0
sleep 1.798
noteoff 1 74 0
noteoff 1 79 0
noteoff 4 62 0
noteon 11 67 102
sleep 1.798
noteoff 6 62 0
noteoff 6 74 0
sleep 3.597
noteoff 2 67 0
noteoff 2 74 0
sleep 1.798
noteoff 5 50 0
noteon 12 59 102
sleep 8.992
noteoff 13 59 0
sleep 1.798
noteoff 3 47 0
noteoff 3 59 0
sleep 1.798
noteon 15 50 90
sleep 1.798
noteoff 14 47 0
sleep 64.748
noteoff 10 79 0
sleep 3.597
noteoff 11 67 0
sleep 7.194
noteoff 12 59 0
sleep 7.194
noteon 10 78 102
sleep 1.798
noteoff 15 50 0
noteon 0 86 101
noteon 0 78 101
sleep 1.798
noteon 1 78 100
noteon 1 74 100
noteon 4 62 100
noteon 11 66 102
sleep 1.798
noteon 6 74 108
noteon 6 62 108
sleep 3.597
noteon 2 74 101
noteon 2 66 101
sleep 1.798
noteon 5 50 100
noteon 12 60 102
sleep 8.992
noteon 13 60 104
sleep 1.798
noteon 3 60 100
noteon 3 48 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteon 14 48 106
sleep 64.748
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 60 0
sleep 7.194
noteon 10 78 102
sleep 1.798
noteoff 0 78 0
noteoff 0 86 0
noteoff 15 50 0
sleep 1.798
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 62 0
noteon 11 66 102
sleep 1.798
noteoff 6 62 0
noteoff 6 74 0
sleep 3.597
noteoff 2 66 0
noteoff 2 74 0
sleep 1.798
noteoff 5 50 0
noteon 12 60 102
sleep 8.992
noteoff 13 60 0
sleep 1.798
noteoff 3 48 0
noteoff 3 60 0
sleep 1.798
noteon 15 50 90
sleep 1.798
noteoff 14 48 0
sleep 64.748
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 60 0
sleep 7.194
echo "9360 tempo_s=310 tempo_l=0.25"
noteon 10 79 102
sleep 1.612
noteoff 15 50 0
noteon 0 86 101
noteon 0 79 101
sleep 1.612
noteon 1 79 100
noteon 1 74 100
noteon 4 62 100
noteon 11 67 102
sleep 1.612
noteon 6 74 108
noteon 6 62 108
sleep 3.225
noteon 2 74 101
noteon 2 67 101
sleep 1.612
noteon 5 50 100
noteon 12 59 102
sleep 8.064
noteon 13 59 104
sleep 1.612
noteon 3 59 100
noteon 3 47 100
sleep 1.612
noteon 15 50 90
sleep 1.612
noteon 14 47 106
sleep 58.064
noteoff 10 79 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 59 0
sleep 6.451
noteon 10 79 102
sleep 1.612
noteoff 0 79 0
noteoff 0 86 0
noteoff 15 50 0
sleep 1.612
noteoff 1 74 0
noteoff 1 79 0
noteoff 4 62 0
noteon 11 67 102
sleep 1.612
noteoff 6 62 0
noteoff 6 74 0
sleep 3.225
noteoff 2 67 0
noteoff 2 74 0
sleep 1.612
noteoff 5 50 0
noteon 12 59 102
sleep 8.064
noteoff 13 59 0
sleep 1.612
noteoff 3 47 0
noteoff 3 59 0
sleep 1.612
noteon 15 50 90
sleep 1.612
noteoff 14 47 0
sleep 58.064
noteoff 10 79 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 59 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 15 50 0
noteon 0 86 101
noteon 0 80 101
sleep 1.612
noteon 1 80 100
noteon 1 74 100
noteon 4 62 100
noteon 11 68 102
sleep 1.612
noteon 6 74 108
noteon 6 62 108
sleep 3.225
noteon 2 74 101
noteon 2 68 101
sleep 1.612
noteon 5 50 100
noteon 12 58 102
sleep 8.064
noteon 13 58 104
sleep 1.612
noteon 3 58 100
noteon 3 46 100
sleep 1.612
noteon 15 50 90
sleep 1.612
noteon 14 46 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 68 0
sleep 6.451
noteoff 12 58 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 0 80 0
noteoff 0 86 0
noteoff 15 50 0
sleep 1.612
noteoff 1 74 0
noteoff 1 80 0
noteoff 4 62 0
noteon 11 68 102
sleep 1.612
noteoff 6 62 0
noteoff 6 74 0
sleep 3.224
noteoff 2 68 0
noteoff 2 74 0
sleep 1.612
noteoff 5 50 0
noteon 12 58 102
sleep 8.062
noteoff 13 58 0
sleep 1.612
noteoff 3 46 0
noteoff 3 58 0
sleep 1.612
noteon 15 50 90
sleep 1.612
noteoff 14 46 0
sleep 58.041
noteoff 10 80 0
sleep 3.224
noteoff 11 68 0
sleep 6.449
noteoff 12 58 0
sleep 6.449
echo "9600 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
sleep 1.798
noteoff 15 50 0
noteon 0 81 101
noteon 0 85 101
sleep 1.798
noteon 1 81 100
noteon 1 73 100
noteon 4 69 100
noteon 11 69 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 73 101
noteon 2 69 101
sleep 1.798
noteon 5 57 100
noteon 12 57 102
sleep 8.991
noteon 13 45 104
sleep 1.798
noteon 3 45 100
noteon 3 57 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 33 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 64 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 0 85 0
noteoff 0 81 0
noteoff 13 57 0
sleep 1.798
noteoff 1 73 0
noteoff 1 81 0
noteoff 4 69 0
noteon 11 64 102
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 1.798
noteoff 14 45 0
sleep 1.798
noteoff 2 69 0
noteoff 2 73 0
sleep 1.798
noteoff 5 57 0
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 57 0
noteoff 3 45 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 64 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
echo "9840 tempo_s=310 tempo_l=0.25"
noteon 10 85 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 1 79 100
noteon 1 76 100
noteon 11 67 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 1.612
noteon 3 67 100
noteon 3 64 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 85 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 67 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 67 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 67 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
echo "10080 tempo_s=278 tempo_l=0.25"
noteon 10 86 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteoff 1 76 0
noteoff 1 79 0
noteon 1 74 100
noteon 1 78 100
noteon 11 66 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 64 0
noteoff 3 67 0
noteon 3 62 100
noteon 3 66 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 86 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 66 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 66 102
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 3.597
noteoff 15 45 0
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 66 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 62 0
sleep 1.798
noteoff 1 74 0
sleep 5.395
echo "10320 tempo_s=310 tempo_l=0.25"
noteon 10 86 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteoff 1 78 0
noteon 1 77 100
noteon 1 74 100
noteon 11 65 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 1.612
noteoff 3 62 0
sleep 6.451
noteon 13 57 104
sleep 1.612
noteoff 3 66 0
noteon 3 65 100
noteon 3 62 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 86 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 65 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 65 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 65 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
echo "10560 tempo_s=278 tempo_l=0.25"
noteon 10 83 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteoff 1 74 0
noteoff 1 77 0
noteon 1 73 100
noteon 1 76 100
noteon 4 69 100
noteon 11 64 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 5 57 100
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 62 0
noteoff 3 65 0
noteon 3 61 100
noteon 3 64 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 83 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 76 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 76 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 76 102
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 3.597
noteoff 15 45 0
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 76 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 76 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 76 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
echo "10800 tempo_s=310 tempo_l=0.25"
noteon 10 85 102
sleep 1.612
noteoff 13 57 0
noteon 0 85 101
noteon 0 81 101
sleep 1.612
noteoff 1 76 0
noteoff 1 73 0
noteon 1 79 100
noteon 1 76 100
noteon 11 79 102
sleep 3.225
noteoff 14 45 0
sleep 1.612
noteon 2 69 101
noteon 2 81 101
sleep 1.612
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 1.612
noteoff 3 64 0
noteoff 3 61 0
noteon 3 67 100
noteon 3 64 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 85 0
sleep 3.225
noteoff 11 79 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 79 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 79 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 79 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
echo "11040 tempo_s=278 tempo_l=0.25"
noteon 10 86 102
sleep 1.798
noteoff 0 85 0
noteoff 13 57 0
noteon 0 86 101
sleep 1.798
noteoff 1 76 0
noteoff 1 79 0
noteon 1 78 100
noteon 1 74 100
noteon 11 78 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 64 0
noteoff 3 67 0
noteon 3 66 100
noteon 3 62 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 86 0
sleep 3.597
noteoff 11 78 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 78 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 78 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 78 102
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 3.597
noteoff 15 45 0
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 78 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 78 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 78 0
sleep 7.194
noteoff 0 86 0
noteoff 12 62 0
sleep 1.798
noteoff 1 74 0
sleep 5.395
echo "11280 tempo_s=310 tempo_l=0.25"
noteon 10 86 102
sleep 1.612
noteoff 0 81 0
noteoff 13 57 0
noteon 0 80 101
noteon 0 86 101
sleep 1.612
noteoff 1 78 0
noteon 1 74 100
noteon 1 77 100
noteon 11 77 102
sleep 3.225
noteoff 14 45 0
sleep 1.612
noteoff 2 81 0
noteoff 2 69 0
noteon 2 80 101
noteon 2 68 101
sleep 1.612
noteon 12 62 102
sleep 1.612
noteoff 3 62 0
sleep 6.451
noteon 13 57 104
sleep 1.612
noteoff 3 66 0
noteon 3 62 100
noteon 3 65 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 86 0
sleep 3.225
noteoff 11 77 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 77 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 77 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 77 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 77 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 77 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 77 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
echo "11520 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
sleep 1.798
noteoff 0 86 0
noteoff 0 80 0
noteoff 13 57 0
noteon 0 85 101
noteon 0 81 101
sleep 1.798
noteoff 1 77 0
noteoff 1 74 0
noteon 1 73 100
noteon 1 76 100
noteon 11 76 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 1.798
noteoff 14 45 0
sleep 1.798
noteoff 2 68 0
noteoff 2 80 0
noteon 2 81 101
noteon 2 69 101
sleep 1.798
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 65 0
noteoff 3 62 0
noteon 3 61 100
noteon 3 64 100
sleep 1.798
noteon 15 45 68
sleep 1.798
noteon 14 45 106
sleep 111.51
noteoff 10 81 0
sleep 1.798
noteoff 0 81 0
noteoff 0 85 0
sleep 1.798
noteoff 1 76 0
noteoff 1 73 0
noteoff 11 76 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 69 0
noteoff 2 81 0
sleep 1.798
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 1.798
noteoff 3 64 0
noteoff 3 61 0
sleep 3.597
noteoff 14 45 0
sleep 12.589
noteoff 4 69 0
sleep 7.194
noteoff 5 57 0
sleep 14.388
noteoff 15 45 0
sleep 19.784
noteon 10 57 102
sleep 1.798
noteon 0 69 101
noteon 0 81 101
sleep 1.798
noteon 1 69 100
noteon 4 57 100
noteon 11 57 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 57 101
noteon 2 69 101
sleep 1.798
noteon 5 57 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
noteon 3 57 100
sleep 1.798
noteon 15 45 68
sleep 1.798
noteon 14 33 106
sleep 111.51
noteoff 10 57 0
sleep 1.798
noteoff 0 81 0
noteoff 0 69 0
sleep 1.798
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 57 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 69 0
noteoff 2 57 0
sleep 1.798
noteoff 5 57 0
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 57 0
noteoff 3 45 0
sleep 3.597
noteoff 14 33 0
sleep 34.172
noteoff 15 45 0
sleep 19.784
echo "11760 tempo_s=299 tempo_l=0.25"
noteon 10 57 102
sleep 1.672
noteon 0 69 101
noteon 0 81 101
sleep 1.672
noteon 1 69 100
noteon 4 57 100
noteon 11 57 102
sleep 1.672
noteon 6 57 108
noteon 6 69 108
sleep 3.344
noteon 2 57 101
noteon 2 69 101
sleep 1.672
noteon 5 57 100
noteon 12 57 102
sleep 8.361
noteon 13 45 104
sleep 1.672
noteon 3 45 100
noteon 3 57 100
sleep 1.672
noteon 15 45 68
sleep 1.672
noteon 14 33 106
sleep 103.678
noteoff 10 57 0
sleep 1.672
noteoff 0 81 0
noteoff 0 69 0
sleep 1.672
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 57 0
sleep 1.672
noteoff 6 69 0
noteoff 6 57 0
sleep 3.344
noteoff 2 69 0
noteoff 2 57 0
sleep 1.672
noteoff 5 57 0
noteoff 12 57 0
sleep 8.361
noteoff 13 45 0
sleep 1.672
noteoff 3 57 0
noteoff 3 45 0
sleep 3.344
noteoff 14 33 0
sleep 31.772
noteoff 15 45 0
sleep 18.394
echo "11880 tempo_s=278 tempo_l=0.25"
noteon 10 57 102
sleep 1.798
noteon 0 69 101
noteon 0 81 101
sleep 1.798
noteon 1 69 100
noteon 4 57 100
noteon 11 57 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 57 101
noteon 2 69 101
sleep 1.798
noteon 5 57 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
noteon 3 57 100
sleep 1.798
noteon 15 45 68
sleep 1.798
noteon 14 33 106
sleep 111.51
noteoff 10 57 0
sleep 1.798
noteoff 0 81 0
noteoff 0 69 0
sleep 1.798
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 57 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 69 0
noteoff 2 57 0
sleep 1.798
noteoff 5 57 0
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 57 0
noteoff 3 45 0
sleep 3.597
noteoff 14 33 0
sleep 34.172
noteoff 15 45 0
sleep 19.784
echo "12000 tempo_s=257 tempo_l=0.25"
noteon 10 57 102
sleep 1.945
noteon 0 81 101
noteon 0 69 101
sleep 1.945
noteon 1 69 100
noteon 4 57 100
noteon 11 57 102
sleep 1.945
noteon 6 57 108
noteon 6 69 108
sleep 3.891
noteon 2 57 101
noteon 2 69 101
sleep 1.945
noteon 5 57 100
noteon 12 57 102
sleep 9.727
noteon 13 45 104
sleep 1.945
noteon 3 45 100
noteon 3 57 100
sleep 1.945
noteon 15 45 68
sleep 1.945
noteon 14 33 106
sleep 206.225
noteoff 10 57 0
sleep 1.945
noteoff 0 69 0
noteoff 0 81 0
sleep 1.945
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 57 0
sleep 1.945
noteoff 6 69 0
noteoff 6 57 0
sleep 3.891
noteoff 2 69 0
noteoff 2 57 0
sleep 1.945
noteoff 5 57 0
noteoff 12 57 0
sleep 9.727
noteoff 13 45 0
sleep 1.945
noteoff 3 57 0
noteoff 3 45 0
sleep 1.945
noteoff 15 45 0
sleep 1.945
noteoff 14 33 0
sleep 694.552
noteon 13 50 104
sleep 3.891
select 14 1 0 45
sleep 1.945
noteon 14 38 41
sleep 233.459
noteoff 14 38 0
sleep 206.225
echo "12720 tempo_s=283 tempo_l=0.25"
sleep 19.434
noteoff 13 50 0
noteon 13 52 104
sleep 212.014
noteoff 13 52 0
noteon 13 54 104
sleep 192.579
echo "12960 tempo_s=268 tempo_l=0.25"
noteon 10 69 102
sleep 1.865
noteoff 13 54 0
sleep 1.865
noteon 11 61 102
sleep 7.462
noteon 12 57 102
sleep 9.328
noteon 13 55 104
sleep 5.597
noteon 14 33 46
sleep 223.868
noteoff 14 33 0
sleep 197.751
echo "13200 tempo_s=281 tempo_l=0.25"
sleep 3.558
noteoff 11 61 0
noteon 11 62 102
sleep 7.117
noteoff 12 57 0
noteon 12 59 102
sleep 204.622
noteoff 13 55 0
sleep 1.779
noteoff 11 62 0
noteon 11 64 102
sleep 7.116
noteoff 12 59 0
noteon 12 61 102
sleep 8.896
noteon 13 55 104
sleep 179.711
noteoff 11 64 0
sleep 7.117
noteoff 12 61 0
sleep 7.117
echo "13440 tempo_s=268 tempo_l=0.25"
sleep 1.865
noteoff 13 55 0
sleep 1.865
noteon 11 66 102
sleep 7.462
noteon 12 62 102
sleep 9.328
noteon 13 55 104
sleep 5.597
noteon 14 38 46
sleep 218.283
noteoff 13 55 0
noteon 13 54 104
sleep 5.597
noteoff 14 38 0
sleep 197.761
echo "13680 tempo_s=281 tempo_l=0.25"
sleep 19.572
noteoff 13 54 0
noteon 13 52 104
sleep 213.523
noteoff 13 52 0
noteon 13 50 104
sleep 179.715
noteoff 11 66 0
sleep 7.117
noteoff 12 62 0
sleep 7.117
echo "13920 tempo_s=268 tempo_l=0.25"
sleep 1.865
noteoff 13 50 0
sleep 1.865
noteon 11 61 102
sleep 7.462
noteon 12 57 102
sleep 9.328
noteon 13 55 104
sleep 5.597
noteon 14 33 46
sleep 223.88
noteoff 14 33 0
sleep 197.761
echo "14160 tempo_s=281 tempo_l=0.25"
sleep 3.558
noteoff 11 61 0
noteon 11 62 102
sleep 7.117
noteoff 12 57 0
noteon 12 59 102
sleep 204.626
noteoff 13 55 0
sleep 1.779
noteoff 11 62 0
noteon 11 64 102
sleep 7.117
noteoff 12 59 0
noteon 12 61 102
sleep 8.896
noteon 13 55 104
sleep 179.715
noteoff 11 64 0
sleep 7.117
noteoff 12 61 0
sleep 7.117
echo "14400 tempo_s=268 tempo_l=0.25"
sleep 1.865
noteoff 13 55 0
sleep 1.865
noteon 11 66 102
sleep 7.462
noteon 12 62 102
sleep 9.328
noteon 13 54 104
sleep 5.597
noteon 14 38 46
sleep 223.88
noteoff 14 38 0
sleep 197.761
echo "14640 tempo_s=281 tempo_l=0.25"
noteoff 10 69 0
noteon 10 67 102
sleep 19.572
noteoff 13 54 0
noteon 13 52 104
sleep 193.95
noteoff 10 67 0
noteon 10 66 102
sleep 19.572
noteoff 13 52 0
noteon 13 50 104
sleep 176.156
noteoff 10 66 0
sleep 3.558
noteoff 11 66 0
sleep 7.117
noteoff 12 62 0
sleep 7.117
echo "14880 tempo_s=268 tempo_l=0.25"
noteon 10 71 102
sleep 1.865
noteoff 13 50 0
sleep 1.865
noteon 11 74 102
sleep 7.462
noteon 12 62 102
sleep 9.328
noteon 13 56 104
sleep 427.238
echo "15120 tempo_s=281 tempo_l=0.25"
noteoff 10 71 0
noteon 10 69 102
sleep 19.572
noteoff 13 56 0
noteon 13 54 104
sleep 193.95
noteoff 10 69 0
noteon 10 68 102
sleep 19.572
noteoff 13 54 0
noteon 13 52 104
sleep 5.338
noteon 14 40 46
sleep 170.818
noteoff 10 68 0
sleep 17.793
echo "15360 tempo_s=268 tempo_l=0.25"
noteon 10 73 102
sleep 3.731
noteon 4 64 100
sleep 5.597
noteon 2 69 101
sleep 1.865
noteon 5 64 100
sleep 9.328
noteoff 13 52 0
noteon 13 57 104
sleep 1.865
noteon 3 57 100
sleep 3.731
noteoff 14 40 0
noteon 14 33 46
sleep 197.755
noteoff 10 73 0
sleep 3.731
noteoff 11 74 0
noteon 11 73 102
sleep 7.462
noteoff 12 62 0
noteon 12 61 102
sleep 14.924
noteoff 14 33 0
sleep 197.756
echo "15600 tempo_s=281 tempo_l=0.25"
sleep 3.558
noteoff 11 73 0
noteon 11 71 102
sleep 5.338
noteoff 2 69 0
noteon 2 71 101
sleep 1.779
noteoff 12 61 0
noteon 12 59 102
sleep 10.676
noteoff 3 57 0
noteon 3 59 100
sleep 193.947
noteoff 13 57 0
sleep 1.779
noteoff 11 71 0
noteon 11 69 102
sleep 5.338
noteoff 2 71 0
noteon 2 73 101
sleep 1.779
noteoff 12 59 0
noteon 12 57 102
sleep 8.896
noteon 13 57 104
sleep 1.779
noteoff 3 59 0
noteon 3 61 100
sleep 177.934
noteoff 11 69 0
sleep 7.117
noteoff 12 57 0
sleep 7.117
echo "15840 tempo_s=268 tempo_l=0.25"
noteoff 2 73 0
noteon 10 76 102
sleep 1.865
noteoff 13 57 0
sleep 1.865
noteon 11 69 102
sleep 5.597
noteon 2 74 101
sleep 1.865
noteon 12 52 102
sleep 1.865
noteoff 3 61 0
sleep 7.462
noteon 13 57 104
sleep 1.865
noteon 3 62 100
sleep 3.731
noteon 14 40 46
sleep 201.492
noteoff 11 69 0
noteon 11 68 102
sleep 16.791
noteoff 13 57 0
noteon 13 56 104
sleep 5.597
noteoff 14 40 0
sleep 197.761
echo "16080 tempo_s=281 tempo_l=0.25"
noteoff 10 76 0
noteon 10 78 102
sleep 3.558
noteoff 11 68 0
noteon 11 66 102
sleep 16.014
noteoff 13 56 0
noteon 13 54 104
sleep 193.95
noteoff 2 74 0
noteoff 10 78 0
noteon 10 80 102
sleep 3.558
noteoff 11 66 0
noteon 11 64 102
sleep 5.338
noteon 2 74 101
sleep 3.558
noteoff 3 62 0
sleep 7.117
noteoff 13 54 0
noteon 13 52 104
sleep 1.779
noteon 3 62 100
sleep 174.377
noteoff 10 80 0
sleep 3.558
noteoff 11 64 0
sleep 14.234
echo "16320 tempo_s=268 tempo_l=0.25"
noteoff 2 74 0
noteon 10 81 87
sleep 1.865
noteoff 13 52 0
sleep 1.865
noteon 11 69 102
sleep 5.597
noteon 2 74 101
sleep 3.731
noteoff 3 62 0
sleep 7.462
noteon 13 57 104
sleep 1.865
noteon 3 62 100
sleep 3.731
noteon 14 33 46
sleep 85.82
noteoff 10 81 0
sleep 121.268
noteoff 2 74 0
noteon 2 73 101
sleep 13.059
noteoff 3 62 0
noteon 3 61 100
sleep 3.731
noteoff 14 33 0
sleep 197.761
echo "16560 tempo_s=281 tempo_l=0.25"
sleep 8.896
noteoff 2 73 0
noteon 2 71 101
sleep 12.455
noteoff 3 61 0
noteon 3 59 100
sleep 177.935
noteoff 11 69 0
sleep 16.014
noteoff 13 57 0
sleep 1.779
noteon 11 69 102
sleep 5.338
noteoff 2 71 0
noteon 2 69 101
sleep 10.676
noteon 13 57 104
sleep 1.779
noteoff 3 59 0
noteon 3 57 100
sleep 177.935
noteoff 11 69 0
sleep 14.234
echo "16800 tempo_s=270 tempo_l=0.25"
noteoff 2 69 0
noteon 10 76 102
sleep 1.851
noteoff 13 57 0
sleep 1.851
noteon 11 69 102
sleep 5.555
noteon 2 74 101
sleep 3.703
noteoff 3 57 0
sleep 7.407
noteon 13 57 104
sleep 1.851
noteon 3 62 100
sleep 3.703
noteon 14 40 46
sleep 200.0
noteoff 11 69 0
noteon 11 68 102
sleep 16.666
noteoff 13 57 0
noteon 13 56 104
sleep 5.555
noteoff 14 40 0
sleep 196.296
echo "17040 tempo_s=285 tempo_l=0.25"
noteoff 10 76 0
noteon 10 78 102
sleep 3.508
noteoff 11 68 0
noteon 11 66 102
sleep 15.789
noteoff 13 56 0
noteon 13 54 104
sleep 191.228
noteoff 2 74 0
noteoff 10 78 0
noteon 10 80 102
sleep 3.508
noteoff 11 66 0
noteon 11 64 102
sleep 5.263
noteon 2 74 101
sleep 3.508
noteoff 3 62 0
sleep 7.017
noteoff 13 54 0
noteon 13 52 104
sleep 1.754
noteon 3 62 100
sleep 171.929
noteoff 10 80 0
sleep 3.508
noteoff 11 64 0
sleep 14.035
echo "17280 tempo_s=269 tempo_l=0.25"
noteoff 2 74 0
noteon 10 81 87
sleep 1.858
noteoff 13 52 0
sleep 1.858
noteon 11 69 102
sleep 5.576
noteon 2 74 101
sleep 3.717
noteoff 3 62 0
sleep 7.434
noteon 13 57 104
sleep 1.858
noteon 3 62 100
sleep 1.858
select 14 1 0 48
sleep 1.858
noteon 14 33 106
sleep 85.499
noteoff 10 81 0
sleep 111.520
echo "17400 tempo_s=256 tempo_l=0.25"
sleep 3.906
noteoff 4 64 0
noteoff 11 69 0
sleep 5.859
noteoff 2 74 0
noteon 2 73 101
sleep 1.953
noteoff 5 64 0
sleep 9.765
noteoff 13 57 0
sleep 1.953
noteoff 3 62 0
noteon 3 61 100
sleep 203.118
noteoff 12 52 0
sleep 7.812
echo "17520 tempo_s=288 tempo_l=0.25"
noteon 10 69 102
sleep 3.472
noteon 4 57 100
noteon 11 57 102
sleep 5.208
noteoff 2 73 0
noteon 2 76 101
noteon 2 73 101
sleep 1.736
noteon 5 45 100
noteon 12 49 102
sleep 8.680
noteon 13 45 104
sleep 1.736
noteoff 3 61 0
noteon 3 64 100
noteon 3 61 100
sleep 3.472
noteoff 14 33 0
noteon 14 33 106
sleep 192.696
noteoff 2 73 0
noteoff 2 76 0
noteon 2 71 101
noteon 2 74 101
sleep 12.152
noteoff 3 61 0
noteoff 3 64 0
noteon 3 62 100
noteon 3 59 100
sleep 180.545
noteoff 12 49 0
sleep 6.944
echo "17760 tempo_s=263 tempo_l=0.25"
sleep 9.505
noteoff 2 74 0
noteoff 2 71 0
noteon 2 69 101
noteon 2 73 101
sleep 1.901
noteon 12 50 102
sleep 9.505
noteoff 13 45 0
noteon 13 47 104
sleep 1.901
noteoff 3 59 0
noteoff 3 62 0
noteon 3 61 100
noteon 3 57 100
sleep 3.802
noteoff 14 33 0
noteon 14 35 106
sleep 211.018
noteoff 2 73 0
noteoff 2 69 0
noteon 2 67 101
noteon 2 71 101
sleep 13.307
noteoff 3 57 0
noteoff 3 61 0
noteon 3 59 100
noteon 3 55 100
sleep 186.305
noteoff 10 69 0
sleep 3.802
noteoff 11 57 0
sleep 15.208
echo "18000 tempo_s=288 tempo_l=0.25"
noteon 10 69 102
sleep 1.736
noteon 0 81 101
sleep 1.736
noteoff 4 57 0
noteon 11 57 102
noteon 4 57 100
sleep 5.208
noteoff 2 71 0
noteoff 2 67 0
noteon 2 66 101
noteon 2 69 101
sleep 1.736
noteoff 5 45 0
noteoff 12 50 0
noteon 12 52 102
noteon 5 45 100
sleep 8.68
noteoff 13 47 0
noteon 13 49 104
sleep 1.736
noteoff 3 55 0
noteoff 3 59 0
noteon 3 57 100
noteon 3 54 100
sleep 3.472
noteoff 14 35 0
noteon 14 37 106
sleep 192.701
noteoff 2 69 0
noteoff 2 66 0
noteon 2 64 101
noteon 2 67 101
sleep 12.152
noteoff 3 54 0
noteoff 3 57 0
noteon 3 55 100
noteon 3 52 100
sleep 187.491
echo "18240 tempo_s=269 tempo_l=0.25"
noteoff 2 67 0
noteoff 2 64 0
sleep 1.858
noteoff 13 49 0
sleep 1.858
noteoff 4 57 0
noteon 1 74 100
noteon 1 66 100
noteon 4 62 100
sleep 1.858
noteon 6 57 108
noteon 6 69 108
sleep 1.858
noteoff 14 37 0
sleep 1.858
noteon 2 62 101
noteon 2 66 101
sleep 1.858
noteoff 5 45 0
noteoff 12 52 0
noteon 12 62 102
noteon 5 54 100
sleep 1.858
noteoff 3 52 0
noteoff 3 55 0
sleep 7.432
noteon 13 50 104
sleep 1.858
noteon 3 50 100
noteon 3 54 100
sleep 1.858
noteon 15 50 35
sleep 1.858
noteon 14 38 106
sleep 197.007
noteoff 10 69 0
sleep 3.717
noteoff 11 57 0
sleep 5.575
noteoff 2 66 0
noteoff 2 62 0
sleep 13.010
noteoff 3 54 0
noteoff 3 50 0
sleep 1.858
noteoff 15 50 0
sleep 198.870
echo "18480 tempo_s=289 tempo_l=0.25"
sleep 3.46
noteoff 1 66 0
noteoff 1 74 0
noteoff 4 62 0
noteon 1 69 100
noteon 1 76 100
noteon 4 64 100
sleep 6.92
noteoff 5 54 0
noteon 5 57 100
sleep 190.309
noteoff 12 62 0
sleep 8.65
noteoff 13 50 0
sleep 1.73
noteoff 1 76 0
noteoff 1 69 0
noteoff 4 64 0
noteon 4 66 100
noteon 1 74 100
noteon 1 78 100
sleep 3.46
noteoff 14 38 0
sleep 3.46
noteoff 5 57 0
noteon 5 62 100
noteon 12 62 102
sleep 8.65
noteon 13 50 104
sleep 5.19
noteon 14 38 106
sleep 176.47
noteoff 12 62 0
sleep 1.73
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
sleep 5.19
echo "18720 tempo_s=269 tempo_l=0.25"
noteon 10 57 87
noteon 10 67 87
sleep 1.858
noteoff 5 62 0
noteoff 13 50 0
sleep 1.858
noteon 1 79 100
noteon 1 76 100
noteon 4 67 100
noteon 11 64 87
sleep 3.717
noteoff 14 38 0
sleep 3.717
noteon 5 64 100
noteon 12 62 102
sleep 9.292
noteon 13 50 104
sleep 3.716
noteon 15 45 35
sleep 1.858
noteon 14 38 106
sleep 197.021
noteoff 10 67 0
noteoff 10 57 0
sleep 3.717
noteoff 11 64 0
sleep 7.434
noteoff 12 62 0
noteon 12 61 102
sleep 9.293
noteoff 13 50 0
noteon 13 49 104
sleep 3.717
noteoff 15 45 0
sleep 1.858
noteoff 14 38 0
noteon 14 37 106
sleep 197.026
echo "18960 tempo_s=289 tempo_l=0.25"
sleep 1.73
noteoff 0 81 0
noteon 0 83 101
sleep 8.65
noteoff 12 61 0
noteon 12 59 102
sleep 8.65
noteoff 13 49 0
noteon 13 47 104
sleep 5.19
noteoff 14 37 0
noteon 14 35 106
sleep 178.2
noteoff 1 76 0
noteoff 1 79 0
noteoff 4 67 0
sleep 6.92
noteoff 0 83 0
noteoff 5 64 0
noteon 0 85 101
sleep 1.73
noteon 1 76 100
noteon 1 79 100
noteon 4 67 100
sleep 6.92
noteoff 12 59 0
noteon 5 64 100
noteon 12 57 102
sleep 8.65
noteoff 13 47 0
noteon 13 45 104
sleep 5.19
noteoff 14 35 0
noteon 14 33 106
sleep 176.47
noteoff 0 85 0
noteoff 12 57 0
sleep 1.73
noteoff 1 79 0
noteoff 1 76 0
noteoff 4 67 0
sleep 5.19
echo "19200 tempo_s=269 tempo_l=0.25"
noteon 10 66 87
noteon 10 57 87
sleep 1.858
noteoff 5 64 0
noteoff 13 45 0
noteon 0 86 101
sleep 1.858
noteon 1 76 100
noteon 1 79 100
noteon 4 67 100
noteon 11 62 87
sleep 3.717
noteoff 14 33 0
sleep 3.717
noteon 5 64 100
noteon 12 62 102
sleep 9.292
noteon 13 50 104
sleep 3.716
noteon 15 50 35
sleep 1.858
noteon 14 38 106
sleep 197.021
noteoff 10 57 0
noteoff 10 66 0
sleep 1.858
noteoff 0 86 0
sleep 1.858
noteoff 1 79 0
noteoff 1 76 0
noteoff 4 67 0
noteoff 11 62 0
noteon 1 74 100
noteon 1 78 100
noteon 4 66 100
sleep 7.434
noteoff 5 64 0
noteon 5 62 100
sleep 13.011
noteoff 15 50 0
sleep 198.884
echo "19440 tempo_s=289 tempo_l=0.25"
sleep 3.46
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
noteon 1 69 100
noteon 1 76 100
noteon 4 64 100
sleep 6.92
noteoff 5 62 0
noteon 5 57 100
sleep 190.311
noteoff 12 62 0
sleep 8.65
noteoff 13 50 0
sleep 1.73
noteoff 1 76 0
noteoff 1 69 0
noteoff 4 64 0
noteon 1 66 100
noteon 1 74 100
noteon 4 62 100
sleep 3.46
noteoff 14 38 0
sleep 3.46
noteoff 5 57 0
noteon 5 54 100
noteon 12 62 102
sleep 8.65
noteon 13 50 104
sleep 5.19
noteon 14 38 106
sleep 176.47
noteoff 12 62 0
sleep 1.73
noteoff 1 74 0
noteoff 1 66 0
noteoff 4 62 0
sleep 5.19
echo "19680 tempo_s=269 tempo_l=0.25"
noteon 10 67 87
noteon 10 57 87
sleep 1.858
noteoff 5 54 0
noteoff 13 50 0
noteon 0 81 101
sleep 1.858
noteon 1 79 100
noteon 1 76 100
noteon 4 67 100
noteon 11 64 87
sleep 3.717
noteoff 14 38 0
sleep 1.858
noteon 2 69 101
noteon 2 67 101
sleep 1.858
noteon 5 64 100
noteon 12 62 102
sleep 9.292
noteon 13 50 104
sleep 3.716
noteon 15 45 35
sleep 1.858
noteon 14 38 106
sleep 197.021
noteoff 10 57 0
noteoff 10 67 0
sleep 3.717
noteoff 11 64 0
sleep 1.858
noteoff 6 69 0
noteoff 6 57 0
sleep 5.576
noteoff 12 62 0
noteon 12 61 102
sleep 9.293
noteoff 13 50 0
noteon 13 49 104
sleep 3.717
noteoff 15 45 0
sleep 1.858
noteoff 14 38 0
noteon 14 37 106
sleep 197.026
echo "19920 tempo_s=289 tempo_l=0.25"
sleep 1.73
noteoff 0 81 0
noteon 0 83 101
sleep 6.92
noteoff 2 69 0
noteon 2 71 101
sleep 1.73
noteoff 12 61 0
noteon 12 59 102
sleep 8.65
noteoff 13 49 0
noteon 13 47 104
sleep 5.19
noteoff 14 37 0
noteon 14 35 106
sleep 178.2
noteoff 1 76 0
noteoff 1 79 0
noteoff 4 67 0
sleep 6.92
noteoff 0 83 0
noteoff 5 64 0
noteon 0 85 101
sleep 1.73
noteon 1 74 100
noteon 1 78 100
noteon 4 67 100
sleep 5.19
noteoff 2 71 0
noteoff 2 67 0
noteon 2 67 101
noteon 2 73 101
sleep 1.73
noteoff 12 59 0
noteon 5 64 100
noteon 12 57 102
sleep 8.65
noteoff 13 47 0
noteon 13 45 104
sleep 5.19
noteoff 14 35 0
noteon 14 33 106
sleep 176.47
noteoff 12 57 0
sleep 1.73
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 67 0
sleep 5.19
echo "20160 tempo_s=269 tempo_l=0.25"
noteon 10 57 102
noteon 10 66 102
sleep 1.858
noteoff 0 85 0
noteoff 5 64 0
noteoff 13 45 0
noteon 0 86 101
sleep 1.858
noteon 1 78 100
noteon 1 74 100
noteon 4 66 100
noteon 11 62 102
sleep 3.717
noteoff 14 33 0
sleep 1.858
noteoff 2 73 0
noteoff 2 67 0
noteon 2 66 101
noteon 2 74 101
sleep 1.858
noteon 5 62 100
noteon 12 62 102
sleep 9.292
noteon 13 50 104
sleep 1.858
noteon 3 66 100
sleep 3.717
noteon 14 38 106
sleep 178.429
noteoff 10 66 0
noteoff 10 57 0
sleep 3.716
noteoff 11 62 0
sleep 9.293
noteoff 4 66 0
sleep 5.576
noteon 10 69 102
sleep 1.858
noteoff 0 86 0
noteoff 5 62 0
noteon 0 88 101
sleep 1.858
noteoff 1 74 0
noteon 4 66 100
noteon 11 57 102
sleep 5.576
noteoff 2 74 0
noteoff 2 66 0
sleep 1.858
noteon 5 54 100
sleep 211.873
echo "20400 tempo_s=289 tempo_l=0.25"
sleep 1.73
noteoff 0 88 0
noteon 0 90 101
sleep 200.680
noteoff 4 66 0
sleep 5.190
noteoff 10 69 0
noteon 10 70 102
sleep 1.73
noteoff 0 90 0
noteoff 5 54 0
noteon 0 88 101
sleep 1.73
noteoff 1 78 0
noteoff 11 57 0
noteon 1 76 100
noteon 4 66 100
noteon 11 58 102
sleep 6.920
noteoff 12 62 0
noteon 5 54 100
noteon 12 61 102
sleep 8.650
noteoff 13 50 0
noteon 13 49 104
sleep 1.73
noteoff 3 66 0
noteon 3 64 100
sleep 3.46
noteoff 14 38 0
noteon 14 37 106
sleep 178.190
noteoff 4 66 0
sleep 5.190
echo "20640 tempo_s=269 tempo_l=0.25"
noteoff 10 70 0
noteon 10 71 102
sleep 1.858
noteoff 0 88 0
noteoff 5 54 0
noteon 0 86 101
sleep 1.858
noteoff 1 76 0
noteoff 11 58 0
noteon 1 74 100
noteon 4 66 100
noteon 11 59 102
sleep 7.433
noteoff 12 61 0
noteon 5 54 100
noteon 12 66 102
sleep 9.292
noteoff 13 49 0
noteon 13 54 104
sleep 1.858
noteoff 3 64 0
noteon 3 62 100
sleep 3.717
noteoff 14 37 0
noteon 14 42 106
sleep 191.426
noteoff 4 66 0
sleep 5.576
noteoff 10 71 0
noteon 10 73 102
sleep 1.858
noteoff 0 86 0
noteoff 5 54 0
noteon 0 82 101
noteon 0 85 101
sleep 1.858
noteoff 1 74 0
noteoff 11 59 0
noteon 1 73 100
noteon 4 66 100
noteon 11 61 102
sleep 7.432
noteoff 12 66 0
noteon 5 54 100
noteon 12 64 102
sleep 9.292
noteoff 13 54 0
noteon 13 52 104
sleep 1.858
noteoff 3 62 0
noteon 3 58 100
noteon 3 61 100
sleep 3.717
noteoff 14 42 0
noteon 14 40 106
sleep 191.429
noteoff 4 66 0
sleep 5.574
echo "20880 tempo_s=294 tempo_l=0.25"
noteoff 10 73 0
noteon 10 74 102
sleep 1.7
noteoff 0 85 0
noteoff 0 82 0
noteoff 5 54 0
noteon 0 83 101
sleep 1.7
noteoff 1 73 0
noteoff 11 61 0
noteon 1 71 100
noteon 4 66 100
noteon 11 62 102
sleep 6.802
noteoff 12 64 0
noteon 5 54 100
noteon 12 62 102
sleep 8.503
noteoff 13 52 0
noteon 13 50 104
sleep 1.7
noteoff 3 61 0
noteoff 3 58 0
noteon 3 59 100
sleep 3.401
noteoff 14 40 0
noteon 14 38 106
sleep 175.152
noteoff 4 66 0
sleep 5.102
noteoff 10 74 0
noteon 10 78 102
sleep 1.7
noteoff 0 83 0
noteoff 5 54 0
noteon 0 78 101
noteon 0 81 101
sleep 1.7
noteoff 1 71 0
noteoff 11 62 0
noteon 1 69 100
noteon 1 71 100
noteon 4 66 100
noteon 11 66 102
sleep 6.802
noteoff 12 62 0
noteon 5 66 100
noteon 12 63 102
sleep 8.503
noteoff 13 50 0
noteon 13 51 104
sleep 1.7
noteoff 3 59 0
noteon 3 54 100
noteon 3 57 100
sleep 3.401
noteoff 14 38 0
noteon 14 39 106
sleep 163.248
noteoff 10 78 0
sleep 3.400
noteoff 11 66 0
sleep 6.802
noteoff 12 63 0
sleep 1.7
noteoff 4 66 0
sleep 5.102
echo "21120 tempo_s=276 tempo_l=0.25"
noteon 10 64 102
sleep 1.811
noteoff 0 81 0
noteoff 0 78 0
noteoff 5 66 0
noteoff 13 51 0
noteon 0 76 101
noteon 0 80 101
sleep 1.811
noteoff 1 69 0
noteon 4 64 100
noteon 11 64 102
noteon 1 68 100
sleep 3.623
noteoff 14 39 0
sleep 3.622
noteon 5 64 100
noteon 12 64 102
sleep 9.057
noteon 13 52 104
sleep 1.811
noteoff 3 57 0
noteoff 3 54 0
noteon 3 52 100
noteon 3 56 100
sleep 3.623
noteon 14 40 106
sleep 65.216
noteoff 10 64 0
sleep 3.623
noteoff 11 64 0
sleep 7.245
noteoff 12 64 0
sleep 7.245
noteon 10 64 102
sleep 1.811
noteoff 13 52 0
sleep 1.811
noteon 11 68 102
sleep 3.623
noteoff 14 40 0
sleep 3.623
noteon 12 64 102
sleep 9.057
noteon 13 52 104
sleep 5.434
noteon 14 40 106
sleep 65.216
noteoff 10 64 0
sleep 3.623
noteoff 11 68 0
sleep 7.246
noteoff 12 64 0
sleep 7.245
noteon 10 68 102
sleep 1.811
noteoff 0 80 0
noteoff 0 76 0
noteoff 13 52 0
sleep 1.811
noteoff 1 68 0
noteoff 1 71 0
noteoff 4 64 0
noteon 11 68 102
sleep 3.622
noteoff 14 40 0
sleep 3.623
noteoff 5 64 0
noteon 12 64 102
sleep 9.057
noteon 13 52 104
sleep 1.811
noteoff 3 56 0
noteoff 3 52 0
sleep 3.623
noteon 14 40 106
sleep 65.216
noteoff 10 68 0
sleep 3.623
noteoff 11 68 0
sleep 7.246
noteoff 12 64 0
sleep 7.246
noteon 10 68 102
sleep 1.811
noteoff 13 52 0
sleep 1.811
noteon 11 68 102
sleep 3.623
noteoff 14 40 0
sleep 3.622
noteon 12 64 102
sleep 9.057
noteon 13 52 104
sleep 5.434
noteon 14 40 106
sleep 65.216
noteoff 10 68 0
sleep 3.623
noteoff 11 68 0
sleep 7.246
noteoff 12 64 0
sleep 7.246
echo "21360 tempo_s=300 tempo_l=0.25"
noteon 10 71 102
sleep 1.666
noteoff 13 52 0
sleep 1.666
noteon 11 68 102
sleep 3.333
noteoff 14 40 0
sleep 3.333
noteon 12 64 102
sleep 8.333
noteon 13 52 104
sleep 5.0
noteon 14 40 106
sleep 59.998
noteoff 10 71 0
sleep 3.333
noteoff 11 68 0
sleep 6.666
noteoff 12 64 0
sleep 6.666
noteon 10 71 102
sleep 1.666
noteoff 13 52 0
sleep 1.666
noteon 11 68 102
sleep 3.333
noteoff 14 40 0
sleep 3.333
noteon 12 64 102
sleep 8.333
noteon 13 52 104
sleep 5.0
noteon 14 40 106
sleep 59.999
noteoff 10 71 0
sleep 3.333
noteoff 11 68 0
sleep 6.666
noteoff 12 64 0
sleep 6.666
noteon 10 76 102
sleep 1.666
noteoff 13 52 0
sleep 1.666
noteon 11 68 102
sleep 3.333
noteoff 14 40 0
sleep 3.333
noteon 12 64 102
sleep 8.333
noteon 13 52 104
sleep 5.0
noteon 14 40 106
sleep 59.999
noteoff 10 76 0
sleep 3.333
noteoff 11 68 0
sleep 6.666
noteoff 12 64 0
sleep 6.666
noteon 10 76 102
sleep 1.666
noteoff 13 52 0
sleep 1.666
noteon 11 68 102
sleep 3.333
noteoff 14 40 0
sleep 3.333
noteon 12 64 102
sleep 8.333
noteon 13 52 104
sleep 5.0
noteon 14 40 106
sleep 59.999
noteoff 10 76 0
sleep 3.333
noteoff 11 68 0
sleep 6.666
noteoff 12 64 0
sleep 6.666
echo "21600 tempo_s=278 tempo_l=0.25"
noteon 10 80 102
sleep 1.798
noteoff 13 52 0
noteon 0 80 101
sleep 1.798
noteon 1 71 100
noteon 1 76 100
noteon 4 64 100
noteon 11 68 102
sleep 1.798
noteon 6 76 108
sleep 1.798
noteoff 14 40 0
sleep 1.798
noteon 2 68 101
noteon 2 71 101
sleep 1.798
noteon 5 64 100
noteon 12 64 102
sleep 8.991
noteon 13 52 104
sleep 1.798
noteon 3 52 100
noteon 3 64 100
sleep 3.596
noteon 14 40 106
sleep 68.327
noteoff 11 68 0
sleep 7.192
noteoff 12 64 0
sleep 8.991
noteoff 13 52 0
sleep 1.798
noteon 11 68 102
sleep 3.596
noteoff 14 40 0
sleep 3.596
noteon 12 64 102
sleep 8.991
noteon 13 52 104
sleep 5.394
noteon 14 40 106
sleep 68.327
noteoff 11 68 0
sleep 7.192
noteoff 12 64 0
sleep 8.991
noteoff 13 52 0
sleep 1.798
noteon 11 71 102
sleep 3.596
noteoff 14 40 0
sleep 3.596
noteon 12 64 102
sleep 8.991
noteon 13 52 104
sleep 5.394
noteon 14 40 106
sleep 68.333
noteoff 11 71 0
sleep 7.194
noteoff 12 64 0
sleep 8.991
noteoff 13 52 0
sleep 1.798
noteon 11 71 102
sleep 3.596
noteoff 14 40 0
sleep 3.597
noteon 12 64 102
sleep 8.992
noteon 13 52 104
sleep 5.394
noteon 14 40 106
sleep 68.333
noteoff 11 71 0
sleep 7.194
noteoff 12 64 0
sleep 7.193
echo "21840 tempo_s=303 tempo_l=0.25"
sleep 1.65
noteoff 13 52 0
sleep 1.65
noteon 11 76 102
sleep 3.300
noteoff 14 40 0
sleep 3.3
noteon 12 64 102
sleep 8.25
noteon 13 52 104
sleep 4.95
noteon 14 40 106
sleep 62.706
noteoff 11 76 0
sleep 6.6
noteoff 12 64 0
sleep 8.25
noteoff 13 52 0
sleep 1.65
noteon 11 76 102
sleep 3.3
noteoff 14 40 0
sleep 3.3
noteon 12 64 102
sleep 8.25
noteon 13 52 104
sleep 4.95
noteon 14 40 106
sleep 59.405
noteoff 10 80 0
sleep 3.3
noteoff 11 76 0
sleep 6.6
noteoff 12 64 0
sleep 6.6
noteon 10 64 102
sleep 1.65
noteoff 13 52 0
sleep 1.65
noteon 11 80 102
sleep 3.3
noteoff 14 40 0
sleep 3.3
noteon 12 64 102
sleep 8.25
noteon 13 52 104
sleep 4.95
noteon 14 40 106
sleep 59.405
noteoff 10 64 0
sleep 3.3
noteoff 11 80 0
sleep 6.6
noteoff 12 64 0
sleep 6.6
noteon 10 64 102
sleep 1.65
noteoff 13 52 0
sleep 1.65
noteon 11 80 102
sleep 3.3
noteoff 14 40 0
sleep 3.3
noteon 12 64 102
sleep 8.25
noteon 13 52 104
sleep 4.95
noteon 14 40 106
sleep 59.405
noteoff 10 64 0
sleep 3.3
noteoff 11 80 0
sleep 6.6
noteoff 12 64 0
sleep 6.6
echo "22080 tempo_s=279 tempo_l=0.25"
noteon 10 66 102
sleep 1.792
noteoff 0 80 0
noteoff 13 52 0
sleep 1.792
noteoff 1 76 0
noteoff 1 71 0
noteoff 4 64 0
noteon 11 81 102
sleep 1.792
noteoff 6 76 0
sleep 1.792
noteoff 14 40 0
sleep 1.792
noteoff 2 71 0
noteoff 2 68 0
sleep 1.792
noteoff 5 64 0
noteon 12 63 102
sleep 8.96
noteon 13 52 104
sleep 1.792
noteoff 3 64 0
noteoff 3 52 0
sleep 3.584
noteon 14 40 106
sleep 64.516
noteoff 10 66 0
sleep 10.752
noteoff 12 63 0
sleep 7.168
noteon 10 66 102
sleep 1.792
noteoff 13 52 0
sleep 5.376
noteoff 14 40 0
sleep 3.584
noteon 12 63 102
sleep 8.96
noteon 13 52 104
sleep 5.376
noteon 14 40 106
sleep 64.516
noteoff 10 66 0
sleep 10.752
noteoff 12 63 0
sleep 7.168
noteon 10 69 102
sleep 1.792
noteoff 13 52 0
sleep 1.792
noteoff 11 81 0
sleep 3.584
noteoff 14 40 0
sleep 3.584
noteon 12 63 102
sleep 8.96
noteon 13 52 104
sleep 5.376
noteon 14 40 106
sleep 64.516
noteoff 10 69 0
sleep 10.752
noteoff 12 63 0
sleep 7.168
noteon 10 69 102
sleep 1.792
noteoff 13 52 0
sleep 5.376
noteoff 14 40 0
sleep 3.584
noteon 12 63 102
sleep 8.96
noteon 13 52 104
sleep 5.376
noteon 14 40 106
sleep 64.516
noteoff 10 69 0
sleep 10.752
noteoff 12 63 0
sleep 7.168
echo "22320 tempo_s=306 tempo_l=0.25"
noteon 10 75 102
sleep 1.633
noteoff 13 52 0
sleep 4.901
noteoff 14 40 0
sleep 3.267
noteon 12 63 102
sleep 8.169
noteon 13 52 104
sleep 4.901
noteon 14 40 106
sleep 58.823
noteoff 10 75 0
sleep 9.803
noteoff 12 63 0
sleep 6.535
noteon 10 75 102
sleep 1.633
noteoff 13 52 0
sleep 4.901
noteoff 14 40 0
sleep 3.267
noteon 12 63 102
sleep 8.169
noteon 13 52 104
sleep 4.901
noteon 14 40 106
sleep 58.823
noteoff 10 75 0
sleep 9.803
noteoff 12 63 0
sleep 6.535
noteon 10 78 102
sleep 1.633
noteoff 13 52 0
sleep 4.901
noteoff 14 40 0
sleep 3.267
noteon 12 63 102
sleep 8.169
noteon 13 52 104
sleep 4.901
noteon 14 40 106
sleep 58.823
noteoff 10 78 0
sleep 9.803
noteoff 12 63 0
sleep 6.535
noteon 10 78 102
sleep 1.633
noteoff 13 52 0
sleep 4.901
noteoff 14 40 0
sleep 3.267
noteon 12 63 102
sleep 8.169
noteon 13 52 104
sleep 4.901
noteon 14 40 106
sleep 58.823
noteoff 10 78 0
sleep 9.803
noteoff 12 63 0
sleep 6.535
echo "22560 tempo_s=277 tempo_l=0.25"
noteon 10 81 102
sleep 1.805
noteoff 13 52 0
noteon 0 81 101
sleep 1.805
noteon 1 75 100
noteon 1 78 100
noteon 4 64 100
noteon 11 69 102
sleep 1.805
noteon 6 76 108
sleep 1.805
noteoff 14 40 0
sleep 1.805
noteon 2 69 101
noteon 2 75 101
sleep 1.805
noteon 5 64 100
noteon 12 63 102
sleep 9.025
noteon 13 52 104
sleep 1.805
noteon 3 52 100
noteon 3 63 100
sleep 3.610
noteon 14 40 106
sleep 68.590
noteoff 11 69 0
sleep 7.220
noteoff 12 63 0
sleep 9.025
noteoff 13 52 0
sleep 1.805
noteon 11 69 102
sleep 3.610
noteoff 14 40 0
sleep 3.610
noteon 12 63 102
sleep 9.025
noteon 13 52 104
sleep 5.415
noteon 14 40 106
sleep 68.590
noteoff 11 69 0
sleep 7.220
noteoff 12 63 0
sleep 9.025
noteoff 13 52 0
sleep 1.805
noteon 11 75 102
sleep 3.610
noteoff 14 40 0
sleep 3.610
noteon 12 63 102
sleep 9.025
noteon 13 52 104
sleep 5.415
noteon 14 40 106
sleep 68.590
noteoff 11 75 0
sleep 7.220
noteoff 12 63 0
sleep 9.025
noteoff 13 52 0
sleep 1.805
noteon 11 75 102
sleep 3.610
noteoff 14 40 0
sleep 3.61
noteon 12 63 102
sleep 9.025
noteon 13 52 104
sleep 5.415
noteon 14 40 106
sleep 68.590
noteoff 11 75 0
sleep 7.220
noteoff 12 63 0
sleep 7.220
echo "22800 tempo_s=307 tempo_l=0.25"
sleep 1.628
noteoff 13 52 0
sleep 1.628
noteon 11 78 102
sleep 3.256
noteoff 14 40 0
sleep 3.257
noteon 12 63 102
sleep 8.143
noteon 13 52 104
sleep 4.885
noteon 14 40 106
sleep 61.889
noteoff 11 78 0
sleep 6.514
noteoff 12 63 0
sleep 8.143
noteoff 13 52 0
sleep 1.628
noteon 11 78 102
sleep 3.257
noteoff 14 40 0
sleep 3.257
noteon 12 63 102
sleep 8.143
noteon 13 52 104
sleep 4.885
noteon 14 40 106
sleep 58.631
noteoff 10 81 0
sleep 3.257
noteoff 11 78 0
sleep 6.514
noteoff 12 63 0
sleep 6.514
noteon 10 66 102
sleep 1.628
noteoff 13 52 0
sleep 1.628
noteon 11 81 102
sleep 3.257
noteoff 14 40 0
sleep 3.257
noteon 12 63 102
sleep 8.143
noteon 13 52 104
sleep 4.885
noteon 14 40 106
sleep 58.631
noteoff 10 66 0
sleep 3.257
noteoff 11 81 0
sleep 6.514
noteoff 12 63 0
sleep 6.514
noteon 10 66 102
sleep 1.628
noteoff 13 52 0
sleep 1.628
noteon 11 81 102
sleep 3.257
noteoff 14 40 0
sleep 3.257
noteon 12 63 102
sleep 8.143
noteon 13 52 104
sleep 4.885
noteon 14 40 106
sleep 58.631
noteoff 10 66 0
sleep 3.257
noteoff 11 81 0
sleep 6.514
noteoff 12 63 0
sleep 6.514
echo "23040 tempo_s=278 tempo_l=0.25"
noteon 10 68 102
sleep 1.798
noteoff 0 81 0
noteoff 13 52 0
sleep 1.798
noteoff 1 78 0
noteoff 1 75 0
noteoff 4 64 0
noteon 11 83 102
sleep 1.798
noteoff 6 76 0
sleep 1.798
noteoff 14 40 0
sleep 1.798
noteoff 2 75 0
noteoff 2 69 0
sleep 1.798
noteoff 5 64 0
noteon 12 62 102
sleep 8.992
noteon 13 52 104
sleep 1.798
noteoff 3 63 0
noteoff 3 52 0
sleep 3.597
noteon 14 40 106
sleep 64.748
noteoff 10 68 0
sleep 10.791
noteoff 12 62 0
sleep 7.194
noteon 10 68 102
sleep 1.798
noteoff 13 52 0
sleep 5.395
noteoff 14 40 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 52 104
sleep 5.395
noteon 14 40 106
sleep 64.748
noteoff 10 68 0
sleep 3.597
noteoff 11 83 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 83 102
sleep 1.798
noteoff 13 52 0
sleep 1.798
noteon 11 68 102
noteon 11 71 102
sleep 3.597
noteoff 14 40 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 52 104
sleep 5.395
noteon 14 40 106
sleep 64.748
noteoff 10 83 0
sleep 3.597
noteoff 11 71 0
noteoff 11 68 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 83 102
sleep 1.798
noteoff 13 52 0
sleep 1.798
noteon 11 68 102
noteon 11 71 102
sleep 3.597
noteoff 14 40 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 52 104
sleep 5.395
noteon 14 40 106
sleep 64.748
noteoff 10 83 0
sleep 3.597
noteoff 11 71 0
noteoff 11 68 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
echo "23280 tempo_s=310 tempo_l=0.25"
noteon 10 83 117
sleep 1.612
noteoff 13 52 0
noteon 0 83 101
sleep 1.612
noteon 1 80 100
noteon 1 71 100
noteon 4 64 100
noteon 11 68 117
noteon 11 71 117
sleep 1.612
noteon 6 76 108
sleep 1.612
noteoff 14 40 0
sleep 1.612
noteon 2 71 101
noteon 2 68 101
sleep 1.612
noteon 5 64 100
noteon 12 62 117
sleep 8.061
noteon 13 52 119
sleep 1.612
noteon 3 62 100
noteon 3 59 100
sleep 3.224
noteon 14 40 121
sleep 58.039
noteoff 10 83 0
sleep 3.224
noteoff 11 71 0
noteoff 11 68 0
sleep 6.448
noteoff 12 62 0
sleep 6.449
noteon 10 83 117
sleep 1.612
noteoff 13 52 0
sleep 1.612
noteon 11 68 117
noteon 11 71 117
sleep 3.224
noteoff 14 40 0
sleep 3.224
noteon 12 62 117
sleep 8.062
noteon 13 52 119
sleep 4.837
noteon 14 40 121
sleep 58.041
noteoff 10 83 0
sleep 3.224
noteoff 11 71 0
noteoff 11 68 0
sleep 6.448
noteoff 12 62 0
sleep 6.449
noteon 10 68 102
sleep 1.612
noteoff 13 52 0
sleep 1.612
noteon 11 68 102
noteon 11 71 102
sleep 3.224
noteoff 14 40 0
sleep 3.224
noteon 12 62 102
sleep 8.062
noteon 13 52 104
sleep 4.837
noteon 14 40 106
sleep 58.059
noteoff 10 68 0
sleep 3.224
noteoff 11 71 0
noteoff 11 68 0
sleep 6.450
noteoff 12 62 0
sleep 6.451
noteon 10 68 102
sleep 1.612
noteoff 13 52 0
sleep 1.612
noteon 11 68 102
noteon 11 71 102
sleep 3.224
noteoff 14 40 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 52 104
sleep 4.838
noteon 14 40 106
sleep 58.059
noteoff 10 68 0
sleep 3.224
noteoff 11 71 0
noteoff 11 68 0
sleep 6.450
noteoff 12 62 0
sleep 6.451
echo "23520 tempo_s=278 tempo_l=0.25"
noteon 10 69 102
sleep 1.798
noteoff 0 83 0
noteoff 13 52 0
sleep 1.798
noteoff 1 71 0
noteoff 1 80 0
noteoff 4 64 0
noteon 11 69 102
noteon 11 73 102
sleep 1.798
noteoff 6 76 0
sleep 1.798
noteoff 14 40 0
sleep 1.798
noteoff 2 68 0
noteoff 2 71 0
sleep 1.798
noteoff 5 64 0
noteon 12 61 102
sleep 8.992
noteon 13 52 104
sleep 1.798
noteoff 3 59 0
noteoff 3 62 0
sleep 3.597
noteon 14 40 106
sleep 64.748
noteoff 10 69 0
sleep 3.597
noteoff 11 73 0
noteoff 11 69 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 69 102
sleep 1.798
noteoff 13 52 0
sleep 1.798
noteon 11 73 102
noteon 11 69 102
sleep 3.597
noteoff 14 40 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 52 104
sleep 5.395
noteon 14 40 106
sleep 64.748
noteoff 10 69 0
sleep 3.597
noteoff 11 69 0
noteoff 11 73 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 85 102
sleep 1.798
noteoff 13 52 0
sleep 1.798
noteon 11 73 102
noteon 11 69 102
sleep 3.597
noteoff 14 40 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 52 104
sleep 5.395
noteon 14 40 106
sleep 64.748
noteoff 10 85 0
sleep 3.597
noteoff 11 69 0
noteoff 11 73 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 85 102
sleep 1.798
noteoff 13 52 0
sleep 1.798
noteon 11 73 102
noteon 11 69 102
sleep 3.597
noteoff 14 40 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 52 104
sleep 5.395
noteon 14 40 106
sleep 64.748
noteoff 10 85 0
sleep 3.597
noteoff 11 69 0
noteoff 11 73 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
echo "23760 tempo_s=310 tempo_l=0.25"
noteon 10 85 117
sleep 1.612
noteoff 13 52 0
noteon 0 85 101
sleep 1.612
noteon 1 73 100
noteon 1 81 100
noteon 4 64 100
noteon 11 73 117
noteon 11 69 117
sleep 1.612
noteon 6 76 108
sleep 1.612
noteoff 14 40 0
sleep 1.612
noteon 2 73 101
noteon 2 69 101
sleep 1.612
noteon 5 64 100
noteon 12 61 117
sleep 8.061
noteon 13 52 119
sleep 1.612
noteon 3 57 100
noteon 3 61 100
sleep 3.224
noteon 14 40 121
sleep 58.039
noteoff 10 85 0
sleep 3.224
noteoff 11 69 0
noteoff 11 73 0
sleep 6.448
noteoff 12 61 0
sleep 6.449
noteon 10 85 117
sleep 1.612
noteoff 13 52 0
sleep 1.612
noteon 11 73 117
noteon 11 69 117
sleep 3.224
noteoff 14 40 0
sleep 3.224
noteon 12 61 117
sleep 8.062
noteon 13 52 119
sleep 4.837
noteon 14 40 121
sleep 58.041
noteoff 10 85 0
sleep 3.224
noteoff 11 69 0
noteoff 11 73 0
sleep 6.448
noteoff 12 61 0
sleep 6.449
noteon 10 69 102
sleep 1.612
noteoff 13 52 0
sleep 1.612
noteon 11 73 102
noteon 11 69 102
sleep 3.224
noteoff 14 40 0
sleep 3.224
noteon 12 61 102
sleep 8.062
noteon 13 52 104
sleep 4.837
noteon 14 40 106
sleep 58.059
noteoff 10 69 0
sleep 3.224
noteoff 11 69 0
noteoff 11 73 0
sleep 6.450
noteoff 12 61 0
sleep 6.451
echo "23940 tempo_s=249 tempo_l=0.25"
noteon 10 69 102
sleep 2.008
noteoff 13 52 0
sleep 2.008
noteon 11 69 102
noteon 11 73 102
sleep 4.016
noteoff 14 40 0
sleep 4.016
noteon 12 61 102
sleep 10.04
noteon 13 52 104
sleep 6.024
noteon 14 40 106
sleep 72.288
noteoff 10 69 0
sleep 4.016
noteoff 11 73 0
noteoff 11 69 0
sleep 8.032
noteoff 12 61 0
sleep 8.032
echo "24000 tempo_s=278 tempo_l=0.25"
noteon 10 71 102
sleep 1.798
noteoff 0 85 0
noteoff 13 52 0
noteon 0 86 101
sleep 1.798
noteoff 1 81 0
noteoff 1 73 0
noteoff 4 64 0
noteon 1 74 100
noteon 1 83 100
noteon 11 74 102
noteon 4 64 100
sleep 1.798
noteoff 6 76 0
noteon 6 76 108
sleep 1.798
noteoff 14 40 0
sleep 1.798
noteoff 2 69 0
noteoff 2 73 0
noteon 2 68 101
noteon 2 74 101
sleep 1.798
noteoff 5 64 0
noteon 12 56 102
noteon 5 64 100
sleep 8.992
noteon 13 52 104
sleep 1.798
noteoff 3 61 0
noteoff 3 57 0
noteon 3 56 100
noteon 3 59 100
sleep 3.597
noteon 14 40 106
sleep 64.748
noteoff 10 71 0
sleep 17.985
noteon 10 71 102
sleep 89.928
noteoff 10 71 0
sleep 17.985
noteon 10 86 102
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteoff 1 83 0
noteoff 1 74 0
noteoff 4 64 0
noteoff 11 74 0
sleep 1.798
noteoff 6 76 0
sleep 3.597
noteoff 2 74 0
noteoff 2 68 0
sleep 1.798
noteoff 5 64 0
noteoff 12 56 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 59 0
noteoff 3 56 0
sleep 3.597
noteoff 14 40 0
sleep 64.748
noteoff 10 86 0
sleep 17.985
noteon 10 86 102
sleep 89.928
noteoff 10 86 0
sleep 17.985
echo "24240 tempo_s=292 tempo_l=0.25"
noteon 10 86 117
sleep 85.608
noteoff 10 86 0
sleep 17.122
noteon 10 86 117
sleep 85.608
noteoff 10 86 0
sleep 17.122
noteon 10 83 102
sleep 85.616
noteoff 10 83 0
sleep 17.123
noteon 10 83 102
sleep 85.616
noteoff 10 83 0
sleep 17.123
echo "24480 tempo_s=278 tempo_l=0.25"
noteon 10 80 102
sleep 89.925
noteoff 10 80 0
sleep 17.985
noteon 10 80 102
sleep 89.925
noteoff 10 80 0
sleep 17.985
noteon 10 76 102
sleep 89.925
noteoff 10 76 0
sleep 17.985
noteon 10 76 102
sleep 89.925
noteoff 10 76 0
sleep 17.985
echo "24720 tempo_s=291 tempo_l=0.25"
noteon 10 74 102
sleep 85.908
noteoff 10 74 0
sleep 17.182
noteon 10 74 102
sleep 85.908
noteoff 10 74 0
sleep 17.182
noteon 10 71 102
sleep 85.908
noteoff 10 71 0
sleep 17.182
echo "24900 tempo_s=242 tempo_l=0.25"
noteon 10 71 102
sleep 103.304
noteoff 10 71 0
sleep 20.661
echo "24960 tempo_s=269 tempo_l=0.25"
noteon 10 73 92
sleep 9.293
noteon 2 76 101
sleep 11.152
noteon 13 57 104
sleep 1.858
noteon 3 64 100
sleep 3.717
noteon 14 45 106
sleep 85.498
noteoff 10 73 0
sleep 111.521
noteon 10 76 92
sleep 3.717
noteon 11 73 92
sleep 7.434
noteon 12 61 92
noteon 12 57 92
sleep 9.293
noteoff 13 57 0
sleep 5.576
noteoff 14 45 0
sleep 85.501
noteoff 10 76 0
sleep 3.717
noteoff 11 73 0
sleep 7.434
noteoff 12 57 0
noteoff 12 61 0
sleep 100.371
echo "25200 tempo_s=287 tempo_l=0.25"
noteon 10 73 92
sleep 3.484
noteon 11 69 92
sleep 6.968
noteon 12 64 92
noteon 12 61 92
sleep 94.076
noteoff 10 73 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 61 0
noteoff 12 64 0
sleep 94.076
noteon 10 76 92
sleep 3.484
noteon 11 73 92
sleep 6.968
noteon 12 57 92
noteon 12 61 92
sleep 94.076
noteoff 10 76 0
sleep 3.484
noteoff 11 73 0
sleep 6.968
noteoff 12 61 0
noteoff 12 57 0
sleep 94.076
echo "25440 tempo_s=269 tempo_l=0.25"
sleep 9.293
noteoff 2 76 0
noteon 2 73 101
sleep 11.152
noteon 13 57 104
sleep 1.858
noteoff 3 64 0
noteon 3 61 100
sleep 3.717
noteon 14 45 106
sleep 197.026
noteon 10 76 92
sleep 3.717
noteon 11 73 92
sleep 7.434
noteon 12 61 92
noteon 12 57 92
sleep 9.293
noteoff 13 57 0
sleep 5.576
noteoff 14 45 0
sleep 85.501
noteoff 10 76 0
sleep 3.717
noteoff 11 73 0
sleep 7.434
noteoff 12 57 0
noteoff 12 61 0
sleep 100.371
echo "25680 tempo_s=287 tempo_l=0.25"
noteon 10 73 92
sleep 3.484
noteon 11 69 92
sleep 5.226
noteoff 2 73 0
noteon 2 69 101
sleep 1.742
noteon 12 64 92
noteon 12 61 92
sleep 10.452
noteoff 3 61 0
noteon 3 57 100
sleep 83.623
noteoff 10 73 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 61 0
noteoff 12 64 0
sleep 94.076
noteon 10 76 92
sleep 3.484
noteon 11 73 92
sleep 6.968
noteon 12 57 92
noteon 12 61 92
sleep 94.076
noteoff 10 76 0
sleep 3.484
noteoff 11 73 0
sleep 6.968
noteoff 12 61 0
noteoff 12 57 0
sleep 94.076
echo "25920 tempo_s=269 tempo_l=0.25"
noteoff 2 69 0
sleep 3.717
noteon 1 76 100
sleep 5.576
noteon 2 64 101
sleep 3.717
noteoff 3 57 0
sleep 7.434
noteon 13 56 104
sleep 1.858
noteon 3 52 100
sleep 3.717
noteon 14 44 106
sleep 197.026
noteon 10 76 92
sleep 3.717
noteon 11 74 92
sleep 5.576
noteoff 2 64 0
sleep 1.858
noteon 12 62 92
noteon 12 59 92
sleep 9.293
noteoff 13 56 0
sleep 1.858
noteoff 3 52 0
sleep 3.717
noteoff 14 44 0
sleep 85.501
noteoff 10 76 0
sleep 3.717
noteoff 11 74 0
sleep 7.434
noteoff 12 59 0
noteoff 12 62 0
sleep 100.371
echo "26160 tempo_s=287 tempo_l=0.25"
noteon 10 74 92
sleep 3.484
noteon 11 71 92
sleep 6.968
noteon 12 64 92
noteon 12 62 92
sleep 94.076
noteoff 10 74 0
sleep 3.484
noteoff 11 71 0
sleep 6.968
noteoff 12 62 0
noteoff 12 64 0
sleep 94.076
noteon 10 76 92
sleep 3.484
noteon 11 74 92
sleep 6.968
noteon 12 59 92
noteon 12 62 92
sleep 94.076
noteoff 10 76 0
sleep 3.484
noteoff 11 74 0
sleep 6.968
noteoff 12 62 0
noteoff 12 59 0
sleep 94.076
echo "26400 tempo_s=268 tempo_l=0.25"
sleep 1.865
noteon 0 88 101
sleep 1.865
noteoff 1 76 0
noteon 1 74 100
sleep 16.791
noteon 13 52 104
sleep 5.596
noteon 14 40 106
sleep 197.757
noteon 10 83 102
sleep 3.730
noteon 11 74 92
sleep 7.460
noteon 12 62 92
noteon 12 59 92
sleep 9.325
noteoff 13 52 0
sleep 5.595
noteoff 14 40 0
sleep 85.802
noteoff 10 83 0
noteon 10 81 102
sleep 3.730
noteoff 11 74 0
sleep 7.461
noteoff 12 59 0
noteoff 12 62 0
sleep 100.727
echo "26640 tempo_s=287 tempo_l=0.25"
noteoff 10 81 0
noteon 10 80 102
sleep 3.484
noteoff 1 74 0
noteon 1 71 100
noteon 11 71 92
sleep 6.968
noteon 12 62 92
noteon 12 64 92
sleep 94.076
noteoff 10 80 0
noteon 10 78 102
sleep 3.484
noteoff 11 71 0
sleep 6.968
noteoff 12 64 0
noteoff 12 62 0
sleep 94.076
noteoff 10 78 0
noteon 10 76 102
sleep 3.484
noteon 11 74 92
sleep 6.968
noteon 12 62 92
noteon 12 59 92
sleep 94.076
noteoff 10 76 0
noteon 10 80 102
sleep 3.484
noteoff 11 74 0
sleep 6.968
noteoff 12 59 0
noteoff 12 62 0
sleep 76.655
noteoff 10 80 0
sleep 10.452
noteoff 0 88 0
sleep 1.742
noteoff 1 71 0
sleep 5.226
echo "26880 tempo_s=268 tempo_l=0.25"
noteon 10 81 92
sleep 1.865
noteon 0 85 101
sleep 1.865
noteon 1 73 100
noteon 11 73 92
sleep 5.597
noteon 2 76 101
sleep 1.865
noteon 12 61 92
noteon 12 57 92
sleep 9.328
noteon 13 57 104
sleep 1.865
noteon 3 64 100
sleep 3.731
noteon 14 45 106
sleep 85.815
noteoff 10 81 0
sleep 3.731
noteoff 11 73 0
sleep 7.462
noteoff 12 57 0
noteoff 12 61 0
sleep 100.742
noteon 10 76 92
sleep 1.865
noteoff 0 85 0
sleep 1.865
noteoff 1 73 0
noteon 11 73 92
sleep 7.462
noteon 12 64 92
noteon 12 61 92
sleep 9.328
noteoff 13 57 0
sleep 5.597
noteoff 14 45 0
sleep 85.82
noteoff 10 76 0
sleep 3.731
noteoff 11 73 0
sleep 7.462
noteoff 12 61 0
noteoff 12 64 0
sleep 100.746
echo "27120 tempo_s=287 tempo_l=0.25"
noteon 10 73 92
sleep 3.484
noteon 11 69 92
sleep 6.968
noteon 12 57 92
noteon 12 61 92
sleep 94.076
noteoff 10 73 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 61 0
noteoff 12 57 0
sleep 94.076
noteon 10 76 92
sleep 3.484
noteon 11 73 92
sleep 6.968
noteon 12 64 92
noteon 12 61 92
sleep 94.076
noteoff 10 76 0
sleep 3.484
noteoff 11 73 0
sleep 6.968
noteoff 12 61 0
noteoff 12 64 0
sleep 94.076
echo "27360 tempo_s=268 tempo_l=0.25"
sleep 9.328
noteoff 2 76 0
noteon 2 73 101
sleep 11.194
noteon 13 57 104
sleep 1.865
noteoff 3 64 0
noteon 3 61 100
sleep 3.731
noteon 14 45 106
sleep 197.761
noteon 10 76 92
sleep 3.731
noteon 11 73 92
sleep 7.462
noteon 12 57 92
noteon 12 61 92
sleep 9.328
noteoff 13 57 0
sleep 5.597
noteoff 14 45 0
sleep 85.82
noteoff 10 76 0
sleep 3.731
noteoff 11 73 0
sleep 7.462
noteoff 12 61 0
noteoff 12 57 0
sleep 100.746
echo "27600 tempo_s=287 tempo_l=0.25"
noteon 10 73 92
sleep 3.484
noteon 11 69 92
sleep 5.226
noteoff 2 73 0
noteon 2 69 101
sleep 1.742
noteon 12 61 92
noteon 12 64 92
sleep 10.452
noteoff 3 61 0
noteon 3 57 100
sleep 83.623
noteoff 10 73 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 64 0
noteoff 12 61 0
sleep 94.076
noteon 10 76 92
sleep 3.484
noteon 11 73 92
sleep 6.968
noteon 12 61 92
noteon 12 57 92
sleep 94.076
noteoff 10 76 0
sleep 3.484
noteoff 11 73 0
sleep 6.968
noteoff 12 57 0
noteoff 12 61 0
sleep 94.076
echo "27840 tempo_s=268 tempo_l=0.25"
noteoff 2 69 0
sleep 3.731
noteon 1 76 100
sleep 5.597
noteon 2 64 101
sleep 3.731
noteoff 3 57 0
sleep 7.462
noteon 13 56 104
sleep 1.865
noteon 3 52 100
sleep 3.731
noteon 14 44 106
sleep 197.761
noteon 10 76 92
sleep 3.731
noteon 11 74 92
sleep 5.597
noteoff 2 64 0
sleep 1.865
noteon 12 62 92
noteon 12 59 92
sleep 9.328
noteoff 13 56 0
sleep 1.865
noteoff 3 52 0
sleep 3.731
noteoff 14 44 0
sleep 85.82
noteoff 10 76 0
sleep 3.731
noteoff 11 74 0
sleep 7.462
noteoff 12 59 0
noteoff 12 62 0
sleep 100.746
echo "28080 tempo_s=287 tempo_l=0.25"
noteon 10 74 92
sleep 3.484
noteon 11 71 92
sleep 6.968
noteon 12 64 92
noteon 12 62 92
sleep 94.076
noteoff 10 74 0
sleep 3.484
noteoff 11 71 0
sleep 6.968
noteoff 12 62 0
noteoff 12 64 0
sleep 94.076
noteon 10 76 92
sleep 3.484
noteon 11 74 92
sleep 6.968
noteon 12 59 92
noteon 12 62 92
sleep 94.076
noteoff 10 76 0
sleep 3.484
noteoff 11 74 0
sleep 6.968
noteoff 12 62 0
noteoff 12 59 0
sleep 94.076
echo "28320 tempo_s=268 tempo_l=0.25"
sleep 1.865
noteon 0 88 101
sleep 1.865
noteoff 1 76 0
noteon 1 74 100
sleep 16.791
noteon 13 52 104
sleep 5.596
noteon 14 40 106
sleep 197.757
noteon 10 83 102
sleep 3.730
noteon 11 74 92
sleep 7.460
noteon 12 59 92
noteon 12 62 92
sleep 9.325
noteoff 13 52 0
sleep 5.595
noteoff 14 40 0
sleep 85.802
noteoff 10 83 0
noteon 10 81 102
sleep 3.730
noteoff 11 74 0
sleep 7.461
noteoff 12 62 0
noteoff 12 59 0
sleep 100.727
echo "28560 tempo_s=287 tempo_l=0.25"
noteoff 10 81 0
noteon 10 80 102
sleep 3.484
noteoff 1 74 0
noteon 1 71 100
noteon 11 71 92
sleep 6.968
noteon 12 64 92
noteon 12 62 92
sleep 94.076
noteoff 10 80 0
noteon 10 78 102
sleep 3.484
noteoff 11 71 0
sleep 6.968
noteoff 12 62 0
noteoff 12 64 0
sleep 94.076
noteoff 10 78 0
noteon 10 76 102
sleep 3.484
noteoff 1 71 0
noteon 1 76 100
noteon 11 74 92
sleep 6.968
noteon 12 62 92
noteon 12 59 92
sleep 94.076
noteoff 10 76 0
noteon 10 80 102
sleep 3.484
noteoff 11 74 0
sleep 6.968
noteoff 12 59 0
noteoff 12 62 0
sleep 76.655
noteoff 10 80 0
sleep 10.452
noteoff 0 88 0
sleep 1.742
noteoff 1 76 0
sleep 5.226
echo "28800 tempo_s=268 tempo_l=0.25"
noteon 10 81 92
sleep 1.865
noteon 0 85 101
sleep 1.865
noteon 1 76 100
noteon 11 73 92
sleep 7.462
noteon 12 57 92
noteon 12 61 92
sleep 9.328
noteon 13 57 104
sleep 1.865
noteon 3 64 100
sleep 3.731
noteon 14 45 106
sleep 85.820
noteoff 10 81 0
sleep 3.731
noteoff 11 73 0
sleep 7.462
noteoff 12 61 0
noteoff 12 57 0
sleep 100.746
noteon 10 76 92
sleep 1.865
noteoff 0 85 0
sleep 1.865
noteon 11 73 92
sleep 7.462
noteon 12 61 92
noteon 12 57 92
sleep 9.327
noteoff 13 57 0
sleep 5.597
noteoff 14 45 0
sleep 85.819
noteoff 10 76 0
sleep 3.731
noteoff 11 73 0
sleep 7.461
noteoff 12 57 0
noteoff 12 61 0
sleep 100.743
echo "29040 tempo_s=287 tempo_l=0.25"
noteon 10 73 92
sleep 3.484
noteon 11 69 92
sleep 6.968
noteon 12 61 92
noteon 12 64 92
sleep 94.074
noteoff 10 73 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 64 0
noteoff 12 61 0
sleep 94.073
noteon 10 76 92
sleep 3.484
noteon 11 73 92
sleep 6.968
noteon 12 57 92
noteon 12 61 92
sleep 94.072
noteoff 10 76 0
sleep 3.484
noteoff 11 73 0
sleep 6.968
noteoff 12 61 0
noteoff 12 57 0
sleep 94.072
echo "29280 tempo_s=268 tempo_l=0.25"
sleep 3.731
noteoff 1 76 0
noteon 1 78 100
sleep 16.790
noteon 13 54 104
sleep 1.865
noteoff 3 64 0
noteon 3 66 100
sleep 3.731
noteon 14 42 106
sleep 197.755
noteon 10 78 92
sleep 3.730
noteon 11 73 92
sleep 7.462
noteon 12 58 92
noteon 12 61 92
sleep 9.328
noteoff 13 54 0
sleep 5.597
noteoff 14 42 0
sleep 85.818
noteoff 10 78 0
sleep 3.731
noteoff 11 73 0
sleep 7.462
noteoff 12 61 0
noteoff 12 58 0
sleep 100.744
echo "29520 tempo_s=287 tempo_l=0.25"
noteon 10 73 92
sleep 3.484
noteon 11 70 92
sleep 6.968
noteon 12 61 92
noteon 12 66 92
sleep 94.073
noteoff 10 73 0
sleep 3.484
noteoff 11 70 0
sleep 6.968
noteoff 12 66 0
noteoff 12 61 0
sleep 94.074
noteon 10 78 92
sleep 3.484
noteon 11 73 92
sleep 6.968
noteon 12 58 92
noteon 12 61 92
sleep 94.073
noteoff 10 78 0
sleep 3.484
noteoff 11 73 0
sleep 6.968
noteoff 12 61 0
noteoff 12 58 0
sleep 94.073
echo "29760 tempo_s=269 tempo_l=0.25"
sleep 20.443
noteon 13 59 104
sleep 5.576
noteon 14 47 106
sleep 197.019
noteon 10 78 92
sleep 3.716
noteon 11 74 92
sleep 7.434
noteon 12 59 92
noteon 12 62 92
sleep 9.293
noteoff 13 59 0
sleep 5.576
noteoff 14 47 0
sleep 85.498
noteoff 10 78 0
sleep 3.717
noteoff 11 74 0
sleep 7.434
noteoff 12 62 0
noteoff 12 59 0
sleep 100.367
echo "30000 tempo_s=293 tempo_l=0.25"
noteon 10 74 92
sleep 3.412
noteon 11 71 92
sleep 6.825
noteon 12 66 92
noteon 12 62 92
sleep 92.145
noteoff 10 74 0
sleep 3.412
noteoff 11 71 0
sleep 6.825
noteoff 12 62 0
noteoff 12 66 0
sleep 92.145
noteon 10 78 92
sleep 3.412
noteon 11 74 92
sleep 6.825
noteon 12 59 92
noteon 12 62 92
sleep 92.146
noteoff 10 78 0
sleep 3.412
noteoff 11 74 0
sleep 6.824
noteoff 12 62 0
noteoff 12 59 0
sleep 92.145
echo "30240 tempo_s=278 tempo_l=0.25"
sleep 3.597
noteoff 1 78 0
noteon 4 69 100
noteon 1 81 100
sleep 7.194
noteon 5 57 100
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 66 0
noteon 3 67 100
sleep 3.597
noteon 14 45 106
sleep 190.623
noteon 10 81 92
sleep 3.596
noteon 11 76 92
sleep 7.192
noteon 12 61 92
noteon 12 64 92
sleep 8.990
noteoff 13 57 0
sleep 5.394
noteoff 14 45 0
sleep 82.721
noteoff 10 81 0
sleep 3.596
noteoff 11 76 0
sleep 7.193
noteoff 12 64 0
noteoff 12 61 0
sleep 97.109
echo "30480 tempo_s=297 tempo_l=0.25"
noteon 10 76 92
sleep 3.366
noteon 11 73 92
sleep 6.733
noteon 12 69 92
noteon 12 64 92
sleep 90.909
noteoff 10 76 0
sleep 3.367
noteoff 11 73 0
sleep 6.734
noteoff 12 64 0
noteoff 12 69 0
sleep 90.905
noteon 10 81 92
sleep 3.367
noteon 11 76 92
sleep 6.734
noteon 12 64 92
noteon 12 61 92
sleep 90.909
noteoff 10 81 0
sleep 3.367
noteoff 11 76 0
sleep 6.733
noteoff 12 61 0
noteoff 12 64 0
sleep 90.909
echo "30720 tempo_s=280 tempo_l=0.25"
sleep 19.641
noteon 13 62 104
sleep 1.785
noteoff 3 67 0
noteon 3 66 100
sleep 3.571
noteon 14 50 106
sleep 189.267
noteon 10 81 92
sleep 3.570
noteon 11 78 92
sleep 7.142
noteon 12 62 92
noteon 12 66 92
sleep 8.928
noteoff 13 62 0
sleep 5.357
noteoff 14 50 0
sleep 82.133
noteoff 10 81 0
sleep 3.571
noteoff 11 78 0
sleep 7.142
noteoff 12 66 0
noteoff 12 62 0
sleep 96.419
echo "30960 tempo_s=296 tempo_l=0.25"
noteon 10 78 92
sleep 3.378
noteon 11 74 92
sleep 6.756
noteon 12 69 92
noteon 12 66 92
sleep 91.207
noteoff 10 78 0
sleep 3.378
noteoff 11 74 0
sleep 6.756
noteoff 12 66 0
noteoff 12 69 0
sleep 91.206
noteon 10 81 92
sleep 3.378
noteon 11 78 92
sleep 6.756
noteon 12 62 92
noteon 12 66 92
sleep 91.206
noteoff 10 81 0
sleep 3.378
noteoff 11 78 0
sleep 6.756
noteoff 12 66 0
noteoff 12 62 0
sleep 91.207
echo "31200 tempo_s=277 tempo_l=0.25"
sleep 3.61
noteoff 1 81 0
noteon 1 81 100
sleep 16.245
noteon 13 50 104
sleep 1.805
noteon 3 50 100
sleep 3.61
noteon 14 38 106
sleep 191.335
noteon 10 78 92
sleep 3.61
noteon 11 74 92
sleep 7.22
noteon 12 54 92
noteon 12 59 92
sleep 9.025
noteoff 13 50 0
sleep 1.805
noteoff 3 50 0
sleep 3.61
noteoff 14 38 0
sleep 83.032
noteoff 10 78 0
sleep 3.61
noteoff 11 74 0
sleep 7.22
noteoff 12 59 0
noteoff 12 54 0
sleep 97.472
echo "31440 tempo_s=293 tempo_l=0.25"
noteon 10 74 92
sleep 3.412
noteoff 1 81 0
noteon 1 78 100
noteon 11 71 92
sleep 6.825
noteon 12 59 92
noteon 12 62 92
sleep 10.238
noteoff 3 66 0
noteon 3 62 100
sleep 81.911
noteoff 10 74 0
sleep 3.412
noteoff 11 71 0
sleep 6.825
noteoff 12 62 0
noteoff 12 59 0
sleep 92.15
noteon 10 78 92
sleep 3.412
noteoff 4 69 0
noteon 4 62 100
noteon 11 74 92
sleep 6.825
noteoff 5 57 0
noteon 5 62 100
noteon 12 54 92
noteon 12 59 92
sleep 92.15
noteoff 10 78 0
sleep 3.412
noteoff 1 78 0
noteoff 11 74 0
noteon 1 74 100
sleep 6.825
noteoff 12 59 0
noteoff 12 54 0
sleep 10.238
noteoff 3 62 0
noteon 3 59 100
sleep 76.791
noteoff 1 74 0
sleep 5.119
echo "31680 tempo_s=272 tempo_l=0.25"
sleep 3.676
noteoff 4 62 0
noteon 1 73 100
noteon 4 64 100
sleep 7.352
noteoff 5 62 0
noteon 5 64 100
sleep 1.838
noteoff 3 59 0
sleep 7.352
noteon 13 52 104
sleep 1.838
noteon 3 57 100
noteon 3 52 100
sleep 3.676
noteon 14 40 106
sleep 194.844
noteon 10 76 92
sleep 3.676
noteoff 4 64 0
noteon 11 73 92
sleep 7.352
noteoff 5 64 0
noteon 12 61 92
noteon 12 57 92
sleep 9.191
noteoff 13 52 0
sleep 1.838
noteoff 3 52 0
sleep 3.676
noteoff 14 40 0
sleep 84.555
noteoff 10 76 0
sleep 3.676
noteoff 11 73 0
sleep 7.352
noteoff 12 57 0
noteoff 12 61 0
sleep 99.260
echo "31920 tempo_s=291 tempo_l=0.25"
noteon 10 73 92
sleep 3.436
noteoff 1 73 0
noteon 4 64 100
noteon 11 69 92
noteon 1 76 100
sleep 6.872
noteon 5 64 100
noteon 12 61 92
noteon 12 64 92
sleep 10.309
noteoff 3 57 0
noteon 3 61 100
sleep 82.472
noteoff 10 73 0
sleep 3.436
noteoff 11 69 0
sleep 6.872
noteoff 12 64 0
noteoff 12 61 0
sleep 92.781
noteon 10 76 92
sleep 3.436
noteoff 4 64 0
noteon 11 73 92
sleep 6.872
noteoff 5 64 0
noteon 12 57 92
noteon 12 61 92
sleep 92.780
noteoff 10 76 0
sleep 3.436
noteoff 11 73 0
sleep 6.872
noteoff 12 61 0
noteoff 12 57 0
sleep 92.780
echo "32160 tempo_s=269 tempo_l=0.25"
sleep 20.444
noteon 13 52 104
sleep 1.858
noteon 3 52 100
sleep 3.717
noteon 14 40 106
sleep 197.020
noteon 10 76 92
sleep 3.717
noteon 11 71 92
sleep 7.434
noteon 12 56 92
noteon 12 59 92
sleep 9.293
noteoff 13 52 0
sleep 1.858
noteoff 3 52 0
sleep 3.717
noteoff 14 40 0
sleep 85.499
noteoff 10 76 0
sleep 3.717
noteoff 11 71 0
sleep 7.434
noteoff 12 59 0
noteoff 12 56 0
sleep 100.368
echo "32400 tempo_s=291 tempo_l=0.25"
noteon 10 71 92
sleep 3.436
noteoff 1 76 0
noteon 4 64 100
noteon 11 68 92
noteon 1 74 100
sleep 6.872
noteon 5 64 100
noteon 12 64 92
noteon 12 59 92
sleep 10.309
noteoff 3 61 0
noteon 3 59 100
sleep 82.471
noteoff 10 71 0
sleep 3.436
noteoff 11 68 0
sleep 6.872
noteoff 12 59 0
noteoff 12 64 0
sleep 92.780
noteon 10 76 92
sleep 3.436
noteoff 4 64 0
noteon 11 71 92
sleep 6.872
noteoff 5 64 0
noteon 12 56 92
noteon 12 59 92
sleep 92.780
echo "32580 tempo_s=210 tempo_l=0.25"
noteoff 10 76 0
sleep 4.761
noteoff 1 74 0
noteoff 11 71 0
noteon 1 71 100
sleep 9.523
noteoff 12 59 0
noteoff 12 56 0
sleep 14.285
noteoff 3 59 0
noteon 3 56 100
sleep 95.234
noteoff 1 71 0
sleep 19.047
echo "32640 tempo_s=269 tempo_l=0.25"
noteon 10 76 92
sleep 1.858
noteon 0 88 86
sleep 1.858
noteoff 3 56 0
noteon 1 69 100
noteon 4 64 100
noteon 11 72 92
sleep 5.576
noteon 2 76 86
sleep 1.858
noteon 5 57 100
noteon 12 57 92
noteon 12 60 92
sleep 9.293
noteon 13 57 104
sleep 1.858
noteon 3 64 85
noteon 3 57 100
sleep 3.717
noteon 14 45 106
sleep 85.495
noteoff 10 76 0
sleep 3.717
noteoff 11 72 0
sleep 7.434
noteoff 12 60 0
noteoff 12 57 0
sleep 100.364
noteon 10 76 92
sleep 3.716
noteoff 1 69 0
noteoff 4 64 0
noteon 11 72 92
sleep 7.434
noteoff 5 57 0
noteon 12 60 92
noteon 12 57 92
sleep 9.293
noteoff 13 57 0
sleep 1.858
noteoff 3 57 0
sleep 3.717
noteoff 14 45 0
sleep 85.495
noteoff 10 76 0
sleep 3.717
noteoff 11 72 0
sleep 7.434
noteoff 12 57 0
noteoff 12 60 0
sleep 100.363
echo "32880 tempo_s=287 tempo_l=0.25"
noteon 10 72 92
sleep 3.484
noteon 11 69 92
sleep 6.968
noteon 12 64 92
noteon 12 60 92
sleep 94.072
noteoff 10 72 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 60 0
noteoff 12 64 0
sleep 94.074
noteon 10 76 92
sleep 3.484
noteon 11 72 92
sleep 6.968
noteon 12 57 92
noteon 12 60 92
sleep 94.072
noteoff 10 76 0
sleep 3.484
noteoff 11 72 0
sleep 6.968
noteoff 12 60 0
noteoff 12 57 0
sleep 94.076
echo "33120 tempo_s=269 tempo_l=0.25"
sleep 1.858
noteoff 0 88 0
noteon 0 84 86
sleep 7.434
noteoff 2 76 0
noteon 2 72 86
sleep 11.151
noteon 13 57 104
sleep 1.858
noteoff 3 64 0
noteon 3 60 85
sleep 3.717
noteon 14 45 106
sleep 197.026
noteon 10 76 92
sleep 3.717
noteon 11 72 92
sleep 7.434
noteon 12 60 92
noteon 12 57 92
sleep 9.293
noteoff 13 57 0
sleep 5.576
noteoff 14 45 0
sleep 85.501
noteoff 10 76 0
sleep 3.717
noteoff 11 72 0
sleep 7.434
noteoff 12 57 0
noteoff 12 60 0
sleep 100.371
echo "33360 tempo_s=287 tempo_l=0.25"
noteon 10 72 92
sleep 1.742
noteoff 0 84 0
noteon 0 81 86
sleep 1.742
noteon 11 69 92
sleep 5.226
noteoff 2 72 0
noteon 2 69 86
sleep 1.742
noteon 12 60 92
noteon 12 64 92
sleep 10.452
noteoff 3 60 0
noteon 3 57 85
sleep 83.623
noteoff 10 72 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 64 0
noteoff 12 60 0
sleep 94.076
noteon 10 76 92
sleep 3.484
noteon 11 72 92
sleep 6.968
noteon 12 60 92
noteon 12 57 92
sleep 94.076
noteoff 10 76 0
sleep 3.484
noteoff 11 72 0
sleep 6.968
noteoff 12 57 0
noteoff 12 60 0
sleep 87.108
noteoff 0 81 0
sleep 6.968
echo "33600 tempo_s=269 tempo_l=0.25"
noteoff 2 69 0
sleep 1.858
noteon 0 76 86
sleep 1.858
noteon 1 76 100
sleep 5.576
noteon 2 64 86
sleep 3.717
noteoff 3 57 0
sleep 7.434
noteon 13 56 104
sleep 1.858
noteon 3 52 85
sleep 3.717
noteon 14 44 106
sleep 197.026
noteon 10 76 92
sleep 1.858
noteoff 0 76 0
sleep 1.858
noteon 11 74 92
sleep 5.576
noteoff 2 64 0
sleep 1.858
noteon 12 59 92
noteon 12 62 92
sleep 9.293
noteoff 13 56 0
sleep 1.858
noteoff 3 52 0
sleep 3.717
noteoff 14 44 0
sleep 85.501
noteoff 10 76 0
sleep 3.717
noteoff 11 74 0
sleep 7.434
noteoff 12 62 0
noteoff 12 59 0
sleep 100.371
echo "33840 tempo_s=287 tempo_l=0.25"
noteon 10 74 92
sleep 3.484
noteon 11 71 92
sleep 6.968
noteon 12 62 92
noteon 12 64 92
sleep 94.076
noteoff 10 74 0
sleep 3.484
noteoff 11 71 0
sleep 6.968
noteoff 12 64 0
noteoff 12 62 0
sleep 94.076
noteon 10 76 92
sleep 3.484
noteon 11 74 92
sleep 6.968
noteon 12 59 92
noteon 12 62 92
sleep 94.076
noteoff 10 76 0
sleep 3.484
noteoff 11 74 0
sleep 6.968
noteoff 12 62 0
noteoff 12 59 0
sleep 94.076
echo "34080 tempo_s=269 tempo_l=0.25"
sleep 1.858
noteon 0 88 101
sleep 1.858
noteoff 1 76 0
noteon 1 74 100
sleep 16.728
noteon 13 52 104
sleep 5.575
noteon 14 40 106
sleep 197.015
noteon 10 83 102
sleep 3.716
noteon 11 74 92
sleep 7.432
noteon 12 59 92
noteon 12 62 92
sleep 9.290
noteoff 13 52 0
sleep 5.574
noteoff 14 40 0
sleep 85.480
noteoff 10 83 0
noteon 10 81 102
sleep 3.716
noteoff 11 74 0
sleep 7.433
noteoff 12 62 0
noteoff 12 59 0
sleep 100.349
echo "34320 tempo_s=287 tempo_l=0.25"
noteoff 10 81 0
noteon 10 80 102
sleep 3.484
noteoff 1 74 0
noteon 1 71 100
noteon 11 71 92
sleep 6.968
noteon 12 64 92
noteon 12 62 92
sleep 94.076
noteoff 10 80 0
noteon 10 78 102
sleep 3.484
noteoff 11 71 0
sleep 6.968
noteoff 12 62 0
noteoff 12 64 0
sleep 94.076
noteoff 10 78 0
noteon 10 76 102
sleep 3.484
noteon 11 74 92
sleep 6.968
noteon 12 62 92
noteon 12 59 92
sleep 94.076
noteoff 10 76 0
noteon 10 80 102
sleep 3.484
noteoff 11 74 0
sleep 6.968
noteoff 12 59 0
noteoff 12 62 0
sleep 76.655
noteoff 10 80 0
sleep 10.452
noteoff 0 88 0
sleep 1.742
noteoff 1 71 0
sleep 5.226
echo "34560 tempo_s=269 tempo_l=0.25"
noteon 10 81 92
sleep 1.858
noteon 0 84 101
sleep 1.858
noteon 1 72 100
noteon 11 72 92
sleep 7.434
noteon 12 60 92
noteon 12 57 92
sleep 9.293
noteon 13 57 104
sleep 1.858
noteon 3 64 100
sleep 3.717
noteon 14 45 106
sleep 85.501
noteoff 10 81 0
sleep 3.717
noteoff 11 72 0
sleep 7.434
noteoff 12 57 0
noteoff 12 60 0
sleep 100.371
noteon 10 76 92
sleep 1.858
noteoff 0 84 0
sleep 1.858
noteoff 1 72 0
noteon 11 72 92
sleep 7.434
noteon 12 60 92
noteon 12 57 92
sleep 9.292
noteoff 13 57 0
sleep 5.576
noteoff 14 45 0
sleep 85.501
noteoff 10 76 0
sleep 3.717
noteoff 11 72 0
sleep 7.434
noteoff 12 57 0
noteoff 12 60 0
sleep 100.369
echo "34800 tempo_s=287 tempo_l=0.25"
noteon 10 72 92
sleep 3.484
noteon 11 69 92
sleep 6.968
noteon 12 64 92
noteon 12 60 92
sleep 94.073
noteoff 10 72 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 60 0
noteoff 12 64 0
sleep 94.073
noteon 10 76 92
sleep 3.484
noteon 11 72 92
sleep 6.968
noteon 12 57 92
noteon 12 60 92
sleep 94.074
noteoff 10 76 0
sleep 3.484
noteoff 11 72 0
sleep 6.968
noteoff 12 60 0
noteoff 12 57 0
sleep 94.076
echo "35040 tempo_s=269 tempo_l=0.25"
sleep 20.444
noteon 13 57 104
sleep 5.576
noteon 14 45 106
sleep 197.022
noteon 10 76 92
sleep 3.717
noteon 11 72 92
sleep 7.434
noteon 12 60 92
noteon 12 57 92
sleep 9.293
noteoff 13 57 0
sleep 5.576
noteoff 14 45 0
sleep 85.499
noteoff 10 76 0
sleep 3.717
noteoff 11 72 0
sleep 7.434
noteoff 12 57 0
noteoff 12 60 0
sleep 100.368
echo "35280 tempo_s=287 tempo_l=0.25"
noteon 10 72 92
sleep 3.484
noteon 11 69 92
sleep 6.968
noteon 12 60 92
noteon 12 64 92
sleep 10.452
noteoff 3 64 0
noteon 3 62 100
sleep 83.621
noteoff 10 72 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 64 0
noteoff 12 60 0
sleep 94.076
noteon 10 76 92
sleep 3.484
noteon 11 72 92
sleep 6.968
noteon 12 60 92
noteon 12 57 92
sleep 10.452
noteoff 3 62 0
noteon 3 60 100
sleep 83.622
noteoff 10 76 0
sleep 3.484
noteoff 11 72 0
sleep 6.968
noteoff 12 57 0
noteoff 12 60 0
sleep 94.073
echo "35520 tempo_s=269 tempo_l=0.25"
sleep 13.011
noteoff 3 60 0
sleep 7.434
noteon 13 55 104
sleep 1.858
noteon 3 67 100
sleep 3.717
noteon 14 43 106
sleep 197.018
noteon 10 79 92
sleep 3.717
noteon 11 74 92
sleep 7.434
noteon 12 62 92
noteon 12 59 92
sleep 9.293
noteoff 13 55 0
sleep 5.576
noteoff 14 43 0
sleep 85.498
noteoff 10 79 0
sleep 3.717
noteoff 11 74 0
sleep 7.434
noteoff 12 59 0
noteoff 12 62 0
sleep 100.371
echo "35760 tempo_s=287 tempo_l=0.25"
noteon 10 74 92
sleep 3.484
noteon 11 71 92
sleep 6.968
noteon 12 62 92
noteon 12 67 92
sleep 94.075
noteoff 10 74 0
sleep 3.484
noteoff 11 71 0
sleep 6.968
noteoff 12 67 0
noteoff 12 62 0
sleep 94.073
noteon 10 79 92
sleep 3.484
noteon 11 74 92
sleep 6.968
noteon 12 62 92
noteon 12 59 92
sleep 94.073
noteoff 10 79 0
sleep 3.484
noteoff 11 74 0
sleep 6.968
noteoff 12 59 0
noteoff 12 62 0
sleep 94.073
echo "36000 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteon 1 79 100
sleep 16.728
noteon 13 53 104
sleep 5.575
noteon 14 41 106
sleep 197.017
noteon 10 79 92
sleep 3.717
noteon 11 74 92
sleep 7.434
noteon 12 62 92
noteon 12 59 92
sleep 9.293
noteoff 13 53 0
sleep 5.576
noteoff 14 41 0
sleep 85.499
noteoff 10 79 0
sleep 3.717
noteoff 11 74 0
sleep 7.434
noteoff 12 59 0
noteoff 12 62 0
sleep 100.367
echo "36240 tempo_s=287 tempo_l=0.25"
noteon 10 74 92
sleep 3.484
noteon 11 71 92
sleep 6.968
noteon 12 62 92
noteon 12 67 92
sleep 94.073
noteoff 10 74 0
sleep 3.484
noteoff 11 71 0
sleep 6.968
noteoff 12 67 0
noteoff 12 62 0
sleep 94.073
noteon 10 79 92
sleep 3.484
noteon 11 74 92
sleep 6.968
noteon 12 59 92
noteon 12 62 92
sleep 94.074
noteoff 10 79 0
sleep 3.484
noteoff 11 74 0
sleep 6.968
noteoff 12 62 0
noteoff 12 59 0
sleep 94.076
echo "36480 tempo_s=269 tempo_l=0.25"
sleep 20.444
noteon 13 52 104
sleep 5.576
noteon 14 40 106
sleep 197.010
noteon 10 79 92
sleep 3.716
noteon 11 76 92
sleep 7.434
noteon 12 64 92
noteon 12 60 92
sleep 9.293
noteoff 13 52 0
sleep 5.576
noteoff 14 40 0
sleep 85.494
noteoff 10 79 0
sleep 3.717
noteoff 11 76 0
sleep 7.434
noteoff 12 60 0
noteoff 12 64 0
sleep 100.363
echo "36720 tempo_s=287 tempo_l=0.25"
noteon 10 76 92
sleep 3.484
noteon 11 72 92
sleep 6.968
noteon 12 67 92
noteon 12 64 92
sleep 10.452
noteoff 3 67 0
noteon 3 65 100
sleep 83.616
noteoff 10 76 0
sleep 3.484
noteoff 11 72 0
sleep 6.968
noteoff 12 64 0
noteoff 12 67 0
sleep 94.068
noteon 10 79 92
sleep 3.484
noteon 11 76 92
sleep 6.968
noteon 12 64 92
noteon 12 60 92
sleep 10.452
noteoff 3 65 0
noteon 3 64 100
sleep 83.616
noteoff 10 79 0
sleep 3.484
noteoff 11 76 0
sleep 6.968
noteoff 12 60 0
noteoff 12 64 0
sleep 94.068
echo "36960 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteoff 1 79 0
noteon 1 77 100
sleep 9.292
noteoff 3 64 0
sleep 7.434
noteon 13 53 104
sleep 1.858
noteon 3 62 100
sleep 3.717
noteon 14 41 106
sleep 197.012
noteon 10 77 92
sleep 3.717
noteon 11 69 92
sleep 7.434
noteon 12 53 92
noteon 12 62 92
sleep 9.293
noteoff 13 53 0
sleep 5.576
noteoff 14 41 0
sleep 85.501
noteoff 10 77 0
sleep 3.717
noteoff 11 69 0
sleep 7.434
noteoff 12 62 0
noteoff 12 53 0
sleep 100.371
echo "37200 tempo_s=287 tempo_l=0.25"
noteon 10 74 92
sleep 3.484
noteoff 1 77 0
noteon 1 79 100
noteon 11 65 92
sleep 6.968
noteon 12 65 92
noteon 12 57 92
sleep 10.452
noteoff 3 62 0
noteon 3 64 100
sleep 83.623
noteoff 10 74 0
sleep 3.484
noteoff 11 65 0
sleep 6.968
noteoff 12 57 0
noteoff 12 65 0
sleep 94.076
noteon 10 77 92
sleep 3.484
noteoff 1 79 0
noteon 1 81 100
noteon 11 69 92
sleep 6.968
noteon 12 62 92
noteon 12 53 92
sleep 10.452
noteoff 3 64 0
noteon 3 65 100
sleep 83.623
noteoff 10 77 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 53 0
noteoff 12 62 0
sleep 88.85
noteoff 1 81 0
sleep 5.226
echo "37440 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteon 1 72 100
sleep 9.293
noteoff 3 65 0
sleep 7.434
noteon 13 55 104
sleep 1.858
noteon 3 60 100
sleep 3.717
noteon 14 43 106
sleep 197.026
noteon 10 76 92
sleep 3.717
noteon 11 67 92
sleep 7.434
noteon 12 60 92
noteon 12 52 92
sleep 9.293
noteoff 13 55 0
sleep 5.576
noteoff 14 43 0
sleep 85.501
noteoff 10 76 0
sleep 3.717
noteoff 11 67 0
sleep 7.434
noteoff 12 52 0
noteoff 12 60 0
sleep 100.371
echo "37680 tempo_s=287 tempo_l=0.25"
noteon 10 72 92
sleep 3.484
noteon 11 64 92
sleep 6.968
noteon 12 64 92
noteon 12 55 92
sleep 94.076
noteoff 10 72 0
sleep 3.484
noteoff 11 64 0
sleep 6.968
noteoff 12 55 0
noteoff 12 64 0
sleep 94.076
noteon 10 76 92
sleep 3.484
noteon 11 67 92
sleep 6.968
noteon 12 60 92
noteon 12 52 92
sleep 94.076
noteoff 10 76 0
sleep 3.484
noteoff 11 67 0
sleep 6.968
noteoff 12 52 0
noteoff 12 60 0
sleep 94.076
echo "37920 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteoff 1 72 0
noteon 1 71 100
sleep 16.728
noteon 13 43 104
sleep 1.858
noteoff 3 60 0
noteon 3 59 100
sleep 3.717
noteon 14 31 106
sleep 197.026
noteon 10 74 92
sleep 3.717
noteon 11 65 92
sleep 7.434
noteon 12 59 92
noteon 12 50 92
sleep 9.293
noteoff 13 43 0
sleep 5.576
noteoff 14 31 0
sleep 85.501
noteoff 10 74 0
sleep 3.717
noteoff 11 65 0
sleep 7.434
noteoff 12 50 0
noteoff 12 59 0
sleep 100.371
echo "38160 tempo_s=291 tempo_l=0.25"
noteon 10 71 92
sleep 3.436
noteon 11 62 92
sleep 6.872
noteon 12 53 92
noteon 12 62 92
sleep 92.783
noteoff 10 71 0
sleep 3.436
noteoff 11 62 0
sleep 6.872
noteoff 12 62 0
noteoff 12 53 0
sleep 92.783
noteon 10 74 92
sleep 3.436
noteon 11 65 92
sleep 6.872
noteon 12 50 92
noteon 12 59 92
sleep 92.783
echo "38340 tempo_s=235 tempo_l=0.25"
noteoff 10 74 0
sleep 4.255
noteoff 11 65 0
sleep 8.51
noteoff 12 59 0
noteoff 12 50 0
sleep 108.51
noteoff 1 71 0
sleep 6.382
echo "38400 tempo_s=269 tempo_l=0.25"
sleep 1.858
noteon 0 88 101
sleep 1.858
noteon 1 76 100
sleep 9.292
noteoff 3 59 0
sleep 7.434
noteon 13 48 104
sleep 1.858
noteon 3 64 100
sleep 3.717
noteon 14 36 106
sleep 197.016
noteon 10 76 92
sleep 3.716
noteon 11 72 92
sleep 7.434
noteon 12 57 92
noteon 12 60 92
sleep 9.293
noteoff 13 48 0
sleep 5.576
noteoff 14 36 0
sleep 85.498
noteoff 10 76 0
sleep 3.717
noteoff 11 72 0
sleep 7.434
noteoff 12 60 0
noteoff 12 57 0
sleep 100.364
echo "38640 tempo_s=291 tempo_l=0.25"
noteon 10 72 92
sleep 3.436
noteon 11 69 92
sleep 6.872
noteon 12 60 92
noteon 12 64 92
sleep 92.780
noteoff 10 72 0
sleep 3.436
noteoff 11 69 0
sleep 6.872
noteoff 12 64 0
noteoff 12 60 0
sleep 92.778
noteon 10 76 92
sleep 3.436
noteon 11 72 92
sleep 6.872
noteon 12 60 92
noteon 12 57 92
sleep 92.780
noteoff 10 76 0
sleep 3.436
noteoff 11 72 0
sleep 6.872
noteoff 12 57 0
noteoff 12 60 0
sleep 92.779
echo "38880 tempo_s=269 tempo_l=0.25"
sleep 1.858
noteoff 0 88 0
noteon 0 86 101
sleep 1.858
noteoff 1 76 0
noteon 1 74 100
sleep 16.728
noteon 13 50 104
sleep 1.858
noteoff 3 64 0
noteon 3 62 100
sleep 3.717
noteon 14 38 106
sleep 197.014
noteon 10 74 92
sleep 3.717
noteon 11 70 92
sleep 7.434
noteon 12 53 92
noteon 12 58 92
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 85.495
noteoff 10 74 0
sleep 3.717
noteoff 11 70 0
sleep 7.434
noteoff 12 58 0
noteoff 12 53 0
sleep 100.368
echo "39120 tempo_s=294 tempo_l=0.25"
noteon 10 70 92
sleep 1.7
noteoff 0 86 0
noteon 0 88 101
sleep 1.7
noteoff 1 74 0
noteon 11 65 92
noteon 1 76 100
sleep 6.802
noteon 12 62 92
noteon 12 58 92
sleep 10.203
noteoff 3 62 0
noteon 3 64 100
sleep 81.629
noteoff 10 70 0
sleep 3.401
noteoff 11 65 0
sleep 6.802
noteoff 12 58 0
noteoff 12 62 0
sleep 91.833
noteon 10 74 92
sleep 1.7
noteoff 0 88 0
noteon 0 89 101
sleep 1.7
noteoff 1 76 0
noteon 1 77 100
noteon 11 70 92
sleep 6.802
noteon 12 58 92
noteon 12 53 92
sleep 10.203
noteoff 3 64 0
noteon 3 65 100
sleep 81.628
noteoff 10 74 0
sleep 3.401
noteoff 11 70 0
sleep 6.802
noteoff 12 53 0
noteoff 12 58 0
sleep 85.030
noteoff 0 89 0
sleep 1.7
noteoff 1 77 0
sleep 5.102
echo "39360 tempo_s=269 tempo_l=0.25"
sleep 1.858
noteon 0 81 101
sleep 1.858
noteon 1 69 100
sleep 9.292
noteoff 3 65 0
sleep 7.434
noteon 13 52 104
sleep 1.858
noteon 3 57 100
sleep 3.717
noteon 14 40 106
sleep 197.016
noteon 10 72 92
sleep 3.716
noteon 11 69 92
sleep 7.434
noteon 12 57 92
noteon 12 52 92
sleep 9.293
noteoff 13 52 0
sleep 5.576
noteoff 14 40 0
sleep 85.498
noteoff 10 72 0
sleep 3.717
noteoff 11 69 0
sleep 7.434
noteoff 12 52 0
noteoff 12 57 0
sleep 100.364
echo "39600 tempo_s=296 tempo_l=0.25"
noteon 10 69 92
sleep 3.378
noteon 11 64 92
sleep 6.756
noteon 12 57 92
noteon 12 60 92
sleep 91.212
noteoff 10 69 0
sleep 3.378
noteoff 11 64 0
sleep 6.756
noteoff 12 60 0
noteoff 12 57 0
sleep 91.210
noteon 10 72 92
sleep 3.378
noteon 11 69 92
sleep 6.756
noteon 12 52 92
noteon 12 57 92
sleep 91.213
noteoff 10 72 0
sleep 3.378
noteoff 11 69 0
sleep 6.756
noteoff 12 57 0
noteoff 12 52 0
sleep 91.211
echo "39840 tempo_s=269 tempo_l=0.25"
sleep 1.858
noteoff 0 81 0
noteon 0 80 101
sleep 1.858
noteoff 1 69 0
noteon 1 68 100
sleep 16.728
noteon 13 52 104
sleep 1.858
noteoff 3 57 0
noteon 3 56 100
sleep 3.717
noteon 14 40 106
sleep 197.014
noteon 10 71 92
sleep 3.717
noteon 11 68 92
sleep 7.434
noteon 12 56 92
noteon 12 52 92
sleep 9.293
noteoff 13 52 0
sleep 5.576
noteoff 14 40 0
sleep 85.495
noteoff 10 71 0
sleep 3.717
noteoff 11 68 0
sleep 7.434
noteoff 12 52 0
noteoff 12 56 0
sleep 100.368
echo "40080 tempo_s=297 tempo_l=0.25"
noteon 10 68 92
sleep 3.366
noteon 11 64 92
sleep 6.734
noteon 12 56 92
noteon 12 59 92
sleep 90.904
noteoff 10 68 0
sleep 3.367
noteoff 11 64 0
sleep 6.734
noteoff 12 59 0
noteoff 12 56 0
sleep 90.908
noteon 10 71 92
sleep 3.367
noteon 11 68 92
sleep 6.734
noteon 12 52 92
noteon 12 56 92
sleep 82.488
echo "40255 tempo_s=242 tempo_l=0.25"
sleep 10.330
noteoff 10 71 0
sleep 4.132
noteoff 11 68 0
sleep 8.264
noteoff 12 56 0
noteoff 12 52 0
sleep 111.566
echo "40320 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 1.798
noteoff 0 80 0
noteon 0 81 101
sleep 1.798
noteoff 1 68 0
noteon 1 76 100
noteon 4 69 100
noteon 11 64 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 64 101
noteon 2 69 101
sleep 1.798
noteon 5 45 100
noteon 12 49 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteoff 3 56 0
noteon 3 49 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 37 106
sleep 64.748
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
sleep 14.388
noteon 10 76 102
sleep 3.597
noteon 11 64 102
sleep 7.194
noteoff 12 49 0
sleep 8.992
noteoff 13 49 0
sleep 1.798
noteoff 3 49 0
sleep 3.597
noteoff 14 37 0
sleep 64.748
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
sleep 14.388
noteon 10 76 102
sleep 3.597
noteon 11 64 102
sleep 7.194
noteon 12 52 102
sleep 8.992
noteon 13 52 104
sleep 1.798
noteon 3 52 100
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 14 40 106
sleep 64.748
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
sleep 14.388
noteon 10 76 102
sleep 3.597
noteon 11 64 102
sleep 7.194
noteoff 12 52 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 3.597
noteoff 14 40 0
sleep 64.748
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
sleep 14.388
echo "40560 tempo_s=310 tempo_l=0.25"
noteon 10 76 102
sleep 3.225
noteon 11 64 102
sleep 6.451
noteon 12 57 102
sleep 8.064
noteon 13 57 104
sleep 1.612
noteon 3 57 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 76 0
sleep 3.225
noteoff 11 64 0
sleep 12.903
noteon 10 76 102
sleep 3.225
noteon 11 64 102
sleep 6.451
noteoff 12 57 0
sleep 8.064
noteoff 13 57 0
sleep 1.612
noteoff 3 57 0
sleep 3.225
noteoff 14 45 0
sleep 58.064
noteoff 10 76 0
sleep 3.225
noteoff 11 64 0
sleep 12.903
noteon 10 76 102
sleep 3.225
noteon 11 64 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteon 13 61 104
sleep 1.612
noteon 3 61 100
sleep 3.225
noteon 14 49 106
sleep 58.064
noteoff 10 76 0
sleep 3.225
noteoff 11 64 0
sleep 12.903
noteon 10 76 102
sleep 3.225
noteon 11 64 102
sleep 6.451
noteoff 12 61 0
sleep 8.064
noteoff 13 61 0
sleep 1.612
noteoff 3 61 0
sleep 3.225
noteoff 14 49 0
sleep 58.064
noteoff 10 76 0
sleep 3.225
noteoff 11 64 0
sleep 12.903
echo "40800 tempo_s=278 tempo_l=0.25"
noteon 10 73 102
sleep 3.597
noteoff 1 76 0
noteon 1 73 100
noteon 11 61 102
sleep 7.194
noteon 12 64 102
sleep 8.992
noteon 13 64 104
sleep 1.798
noteon 3 64 100
sleep 3.597
noteon 14 52 106
sleep 64.738
noteoff 10 73 0
sleep 3.596
noteoff 11 61 0
sleep 14.385
noteon 10 73 102
sleep 3.597
noteon 11 61 102
sleep 86.314
noteoff 10 73 0
sleep 3.596
noteoff 11 61 0
sleep 14.386
noteon 10 73 102
sleep 3.596
noteon 11 61 102
sleep 86.328
noteoff 10 73 0
sleep 3.597
noteoff 11 61 0
sleep 14.388
noteon 10 73 102
sleep 3.597
noteon 11 61 102
sleep 86.33
noteoff 10 73 0
sleep 3.597
noteoff 11 61 0
sleep 14.388
echo "41040 tempo_s=310 tempo_l=0.25"
noteon 10 73 102
sleep 3.225
noteon 11 61 102
sleep 77.419
noteoff 10 73 0
sleep 3.225
noteoff 11 61 0
sleep 12.903
noteon 10 73 102
sleep 3.225
noteon 11 61 102
sleep 70.967
noteoff 12 64 0
sleep 6.451
noteoff 10 73 0
sleep 1.612
noteoff 13 64 0
sleep 1.612
noteoff 3 64 0
noteoff 11 61 0
sleep 3.225
noteoff 14 52 0
sleep 9.677
noteon 10 69 102
sleep 3.225
noteoff 1 73 0
noteon 1 69 100
noteon 11 57 102
sleep 4.838
noteoff 2 64 0
noteon 2 61 101
sleep 1.612
noteon 12 49 102
sleep 8.064
noteon 13 49 104
sleep 1.612
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 58.064
noteoff 10 69 0
sleep 3.225
noteoff 11 57 0
sleep 12.903
noteon 10 69 102
sleep 3.225
noteon 11 57 102
sleep 6.451
noteoff 12 49 0
sleep 8.064
noteoff 13 49 0
sleep 1.612
noteoff 3 49 0
sleep 3.225
noteoff 14 37 0
sleep 58.064
noteoff 10 69 0
sleep 3.225
noteoff 11 57 0
sleep 12.903
echo "41280 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 1.798
noteoff 0 81 0
noteon 0 83 101
noteon 0 81 101
sleep 1.798
noteoff 1 69 0
noteoff 4 69 0
noteon 11 66 102
noteon 1 78 100
noteon 4 69 100
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteoff 2 61 0
noteoff 2 69 0
noteon 2 69 101
noteon 2 71 101
sleep 1.798
noteoff 5 45 0
noteon 12 50 102
noteon 5 45 100
sleep 8.991
noteon 13 50 104
sleep 1.798
noteon 3 50 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 38 106
sleep 64.734
noteoff 10 78 0
sleep 3.596
noteoff 11 66 0
sleep 14.385
noteon 10 78 102
sleep 3.596
noteon 11 66 102
sleep 7.193
noteoff 12 50 0
sleep 8.991
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
sleep 3.596
noteoff 14 38 0
sleep 64.734
noteoff 10 78 0
sleep 3.596
noteoff 11 66 0
sleep 14.385
noteon 10 78 102
sleep 3.596
noteon 11 66 102
sleep 7.193
noteon 12 54 102
sleep 8.992
noteon 13 54 104
sleep 1.798
noteon 3 54 100
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 14 42 106
sleep 64.743
noteoff 10 78 0
sleep 3.596
noteoff 11 66 0
sleep 14.388
noteon 10 78 102
sleep 3.597
noteon 11 66 102
sleep 7.193
noteoff 12 54 0
sleep 8.992
noteoff 13 54 0
sleep 1.798
noteoff 3 54 0
sleep 3.597
noteoff 14 42 0
sleep 64.743
noteoff 10 78 0
sleep 3.596
noteoff 11 66 0
sleep 14.388
echo "41520 tempo_s=310 tempo_l=0.25"
noteon 10 78 102
sleep 3.225
noteon 11 66 102
sleep 6.450
noteon 12 57 102
sleep 8.064
noteon 13 57 104
sleep 1.612
noteon 3 57 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 12.903
noteon 10 78 102
sleep 3.225
noteon 11 66 102
sleep 6.451
noteoff 12 57 0
sleep 8.064
noteoff 13 57 0
sleep 1.612
noteoff 3 57 0
sleep 3.225
noteoff 14 45 0
sleep 58.064
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 12.903
noteon 10 78 102
sleep 3.225
noteon 11 66 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 62 104
sleep 1.612
noteon 3 62 100
sleep 3.225
noteon 14 50 106
sleep 58.064
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 12.903
noteon 10 78 102
sleep 3.225
noteon 11 66 102
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 62 0
sleep 1.612
noteoff 3 62 0
sleep 3.225
noteoff 14 50 0
sleep 58.064
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 12.903
echo "41760 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 3.597
noteoff 1 78 0
noteon 1 74 100
noteon 11 62 102
sleep 7.194
noteon 12 66 102
sleep 8.992
noteon 13 66 104
sleep 1.798
noteon 3 66 100
sleep 3.597
noteon 14 54 106
sleep 64.738
noteoff 10 74 0
sleep 3.596
noteoff 11 62 0
sleep 14.385
noteon 10 74 102
sleep 3.597
noteon 11 62 102
sleep 86.314
noteoff 10 74 0
sleep 3.596
noteoff 11 62 0
sleep 14.386
noteon 10 74 102
sleep 3.596
noteon 11 62 102
sleep 86.328
noteoff 10 74 0
sleep 3.597
noteoff 11 62 0
sleep 14.388
noteon 10 74 102
sleep 3.597
noteon 11 62 102
sleep 86.33
noteoff 10 74 0
sleep 3.597
noteoff 11 62 0
sleep 14.388
echo "42000 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 3.225
noteon 11 62 102
sleep 77.419
noteoff 10 74 0
sleep 3.225
noteoff 11 62 0
sleep 12.903
noteon 10 74 102
sleep 3.225
noteon 11 62 102
sleep 70.967
noteoff 12 66 0
sleep 6.451
noteoff 10 74 0
sleep 1.612
noteoff 13 66 0
sleep 1.612
noteoff 3 66 0
noteoff 11 62 0
sleep 3.225
noteoff 14 54 0
sleep 9.677
noteon 10 71 102
sleep 3.225
noteoff 1 74 0
noteon 1 71 100
noteon 11 59 102
sleep 6.451
noteon 12 63 102
sleep 8.064
noteon 13 51 104
sleep 1.612
noteon 3 51 100
sleep 3.225
noteon 14 39 106
sleep 58.064
noteoff 10 71 0
sleep 3.225
noteoff 11 59 0
sleep 12.903
noteon 10 71 102
sleep 3.225
noteon 11 59 102
sleep 6.451
noteoff 12 63 0
sleep 8.064
noteoff 13 51 0
sleep 1.612
noteoff 3 51 0
sleep 3.225
noteoff 14 39 0
sleep 58.064
noteoff 10 71 0
sleep 3.225
noteoff 11 59 0
sleep 12.903
echo "42240 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 1.798
noteoff 0 81 0
noteoff 0 83 0
noteon 0 85 101
noteon 0 81 101
sleep 1.798
noteoff 1 71 0
noteoff 4 69 0
noteon 11 64 102
noteon 1 76 100
noteon 1 73 100
noteon 4 69 100
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
noteon 6 76 108
sleep 3.597
noteoff 2 71 0
noteoff 2 69 0
noteon 2 73 101
noteon 2 69 101
sleep 1.798
noteoff 5 45 0
noteon 12 64 102
noteon 5 64 100
sleep 8.992
noteon 13 52 104
sleep 1.798
noteon 3 52 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 40 106
sleep 64.734
noteoff 10 76 0
sleep 3.596
noteoff 11 64 0
sleep 14.385
noteon 10 76 102
sleep 3.596
noteon 11 64 102
sleep 7.192
noteoff 12 64 0
sleep 8.990
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 3.596
noteoff 14 40 0
sleep 64.730
noteoff 10 76 0
sleep 3.596
noteoff 11 64 0
sleep 14.385
noteon 10 76 102
sleep 3.596
noteon 11 64 102
sleep 7.193
noteon 12 76 102
sleep 8.992
noteon 13 64 104
sleep 1.798
noteon 3 64 100
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 14 52 106
sleep 64.740
noteoff 10 76 0
sleep 3.596
noteoff 11 64 0
sleep 7.193
noteoff 12 76 0
sleep 7.194
noteon 10 76 102
sleep 3.596
noteon 11 64 102
sleep 16.185
noteoff 13 64 0
sleep 1.798
noteoff 3 64 0
sleep 3.596
noteoff 14 52 0
sleep 64.740
noteoff 10 76 0
sleep 3.596
noteoff 11 64 0
sleep 14.387
echo "42480 tempo_s=310 tempo_l=0.25"
noteon 10 73 102
sleep 3.224
noteon 11 61 102
sleep 77.416
noteoff 10 73 0
sleep 3.225
noteoff 11 61 0
sleep 12.903
noteon 10 73 102
sleep 3.225
noteon 11 61 102
sleep 77.419
noteoff 10 73 0
sleep 3.225
noteoff 11 61 0
sleep 12.903
noteon 10 69 102
sleep 3.225
noteon 11 57 102
sleep 6.451
noteon 12 64 102
sleep 8.064
noteon 13 52 104
sleep 1.612
noteon 3 52 100
sleep 3.225
noteon 14 40 106
sleep 58.064
noteoff 10 69 0
sleep 3.225
noteoff 11 57 0
sleep 12.903
noteon 10 69 102
sleep 3.225
noteon 11 57 102
sleep 6.451
noteoff 12 64 0
sleep 8.064
noteoff 13 52 0
sleep 1.612
noteoff 3 52 0
sleep 3.225
noteoff 14 40 0
sleep 58.064
noteoff 10 69 0
sleep 3.225
noteoff 11 57 0
sleep 12.903
echo "42720 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 1.798
noteoff 0 81 0
noteoff 0 85 0
noteon 0 80 101
noteon 0 83 101
sleep 1.798
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 69 0
noteon 11 64 102
noteon 1 71 100
noteon 1 76 100
noteon 4 64 100
sleep 1.798
noteoff 6 76 0
noteon 6 76 108
sleep 3.597
noteoff 2 69 0
noteoff 2 73 0
noteon 2 68 101
noteon 2 71 101
sleep 1.798
noteoff 5 64 0
noteon 12 62 102
noteon 5 64 100
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 50 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 64.734
noteoff 10 76 0
sleep 3.596
noteoff 11 64 0
sleep 14.385
noteon 10 76 102
sleep 3.596
noteon 11 64 102
sleep 7.192
noteoff 12 62 0
sleep 8.990
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
sleep 3.596
noteoff 14 38 0
sleep 64.730
noteoff 10 76 0
sleep 3.596
noteoff 11 64 0
sleep 14.385
noteon 10 76 102
sleep 3.596
noteon 11 64 102
sleep 7.193
noteon 12 74 102
sleep 8.992
noteon 13 62 104
sleep 1.798
noteon 3 62 100
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteon 14 50 106
sleep 64.740
noteoff 10 76 0
sleep 3.596
noteoff 11 64 0
sleep 14.387
noteon 10 76 102
sleep 3.596
noteon 11 64 102
sleep 7.193
noteoff 12 74 0
sleep 8.992
noteoff 13 62 0
sleep 1.798
noteoff 3 62 0
sleep 3.596
noteoff 14 50 0
sleep 64.740
noteoff 10 76 0
sleep 3.596
noteoff 11 64 0
sleep 14.387
echo "42960 tempo_s=310 tempo_l=0.25"
noteon 10 71 102
sleep 3.224
noteon 11 59 102
sleep 77.416
noteoff 10 71 0
sleep 3.225
noteoff 11 59 0
sleep 12.903
noteon 10 71 102
sleep 3.225
noteon 11 59 102
sleep 77.419
noteoff 10 71 0
sleep 3.225
noteoff 11 59 0
sleep 12.903
noteon 10 68 102
sleep 3.225
noteon 11 56 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 3.225
noteon 14 38 106
sleep 58.064
noteoff 10 68 0
sleep 3.225
noteoff 11 56 0
sleep 12.903
echo "43140 tempo_s=255 tempo_l=0.25"
noteon 10 68 102
sleep 3.921
noteon 11 56 102
sleep 7.843
noteoff 12 62 0
sleep 9.803
noteoff 13 50 0
sleep 1.96
noteoff 3 50 0
sleep 3.921
noteoff 14 38 0
sleep 70.588
noteoff 10 68 0
sleep 3.921
noteoff 11 56 0
sleep 15.686
echo "43200 tempo_s=278 tempo_l=0.25"
noteon 10 69 102
noteon 10 81 102
sleep 1.798
noteoff 0 83 0
noteoff 0 80 0
noteon 0 88 101
noteon 0 81 101
sleep 1.798
noteoff 1 76 0
noteoff 1 71 0
noteoff 4 64 0
noteon 1 76 100
noteon 4 69 100
noteon 11 76 102
sleep 1.798
noteoff 6 76 0
noteon 6 69 108
sleep 3.597
noteoff 2 71 0
noteoff 2 68 0
sleep 1.798
noteoff 5 64 0
noteon 5 45 100
noteon 12 49 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteon 3 64 100
noteon 3 49 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 37 106
sleep 64.748
noteoff 10 81 0
noteoff 10 69 0
sleep 3.597
noteoff 11 76 0
sleep 14.388
noteon 10 69 102
noteon 10 81 102
sleep 3.597
noteon 11 76 102
sleep 7.194
noteoff 12 49 0
sleep 8.992
noteoff 13 49 0
sleep 1.798
noteoff 3 49 0
sleep 3.597
noteoff 14 37 0
sleep 64.748
noteoff 10 81 0
noteoff 10 69 0
sleep 3.597
noteoff 11 76 0
sleep 14.388
noteon 10 81 102
noteon 10 69 102
sleep 3.597
noteon 11 76 102
sleep 7.194
noteon 12 52 102
sleep 8.992
noteon 13 52 104
sleep 1.798
noteon 3 52 100
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 14 40 106
sleep 64.748
noteoff 10 69 0
noteoff 10 81 0
sleep 3.597
noteoff 11 76 0
sleep 14.388
noteon 10 69 102
noteon 10 81 102
sleep 3.597
noteon 11 76 102
sleep 7.194
noteoff 12 52 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 3.597
noteoff 14 40 0
sleep 64.748
noteoff 10 81 0
noteoff 10 69 0
sleep 3.597
noteoff 11 76 0
sleep 14.388
echo "43440 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
noteon 10 69 102
sleep 3.225
noteon 11 76 102
sleep 6.451
noteon 12 57 102
sleep 8.064
noteon 13 57 104
sleep 1.612
noteon 3 57 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 76 0
sleep 12.903
noteon 10 81 102
noteon 10 69 102
sleep 3.225
noteon 11 76 102
sleep 6.451
noteoff 12 57 0
sleep 8.064
noteoff 13 57 0
sleep 1.612
noteoff 3 57 0
sleep 3.225
noteoff 14 45 0
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 76 0
sleep 12.903
noteon 10 81 102
noteon 10 69 102
sleep 3.225
noteon 11 76 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteon 13 61 104
sleep 1.612
noteon 3 61 100
sleep 3.225
noteon 14 49 106
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 76 0
sleep 12.903
noteon 10 69 102
noteon 10 81 102
sleep 3.225
noteon 11 76 102
sleep 6.451
noteoff 12 61 0
sleep 8.064
noteoff 13 61 0
sleep 1.612
noteoff 3 61 0
sleep 3.225
noteoff 14 49 0
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 76 0
sleep 12.903
echo "43680 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
noteon 10 69 102
sleep 1.798
noteoff 0 88 0
noteon 0 85 101
sleep 1.798
noteoff 1 76 0
noteon 11 73 102
noteon 1 73 100
sleep 7.193
noteon 12 64 102
sleep 8.992
noteon 13 64 104
sleep 1.798
noteoff 3 64 0
noteon 3 52 100
noteon 3 61 100
sleep 3.596
noteon 14 52 106
sleep 64.734
noteoff 10 69 0
noteoff 10 81 0
sleep 3.596
noteoff 11 73 0
sleep 14.385
noteon 10 81 102
noteon 10 69 102
sleep 3.596
noteon 11 73 102
sleep 86.308
noteoff 10 69 0
noteoff 10 81 0
sleep 3.596
noteoff 11 73 0
sleep 14.385
noteon 10 69 102
noteon 10 81 102
sleep 3.596
noteon 11 73 102
sleep 86.325
noteoff 10 81 0
noteoff 10 69 0
sleep 3.597
noteoff 11 73 0
sleep 14.388
noteon 10 69 102
noteon 10 81 102
sleep 3.596
noteon 11 73 102
sleep 86.328
noteoff 10 81 0
noteoff 10 69 0
sleep 3.597
noteoff 11 73 0
sleep 14.388
echo "43920 tempo_s=310 tempo_l=0.25"
noteon 10 69 102
noteon 10 81 102
sleep 3.224
noteon 11 73 102
sleep 77.419
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 73 0
sleep 12.903
noteon 10 81 102
noteon 10 69 102
sleep 3.225
noteon 11 73 102
sleep 70.967
noteoff 12 64 0
sleep 6.451
noteoff 10 69 0
noteoff 10 81 0
sleep 1.612
noteoff 13 64 0
sleep 1.612
noteoff 3 61 0
noteoff 3 52 0
noteoff 11 73 0
sleep 3.225
noteoff 14 52 0
sleep 9.677
noteon 10 69 102
noteon 10 81 102
sleep 1.612
noteoff 0 85 0
noteon 0 81 101
sleep 1.612
noteoff 1 73 0
noteon 1 69 100
noteon 11 69 102
sleep 6.451
noteon 12 49 102
sleep 8.064
noteon 13 49 104
sleep 1.612
noteon 3 57 100
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 69 0
sleep 12.903
noteon 10 81 102
noteon 10 69 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteoff 12 49 0
sleep 8.064
noteoff 13 49 0
sleep 1.612
noteoff 3 49 0
noteoff 3 57 0
sleep 3.225
noteoff 14 37 0
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 69 0
sleep 12.903
echo "44160 tempo_s=278 tempo_l=0.25"
noteon 10 69 102
noteon 10 81 102
sleep 1.798
noteoff 0 81 0
noteon 0 90 101
noteon 0 81 101
sleep 1.798
noteoff 1 69 0
noteoff 4 69 0
noteon 1 78 100
noteon 4 69 100
noteon 11 78 102
sleep 1.798
noteoff 6 69 0
noteon 6 69 108
sleep 5.395
noteoff 5 45 0
noteon 5 45 100
noteon 12 50 102
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 50 100
noteon 3 66 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 38 106
sleep 64.748
noteoff 10 81 0
noteoff 10 69 0
sleep 3.597
noteoff 11 78 0
sleep 14.388
noteon 10 69 102
noteon 10 81 102
sleep 3.597
noteon 11 78 102
sleep 7.194
noteoff 12 50 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
sleep 3.597
noteoff 14 38 0
sleep 64.748
noteoff 10 81 0
noteoff 10 69 0
sleep 3.597
noteoff 11 78 0
sleep 14.388
noteon 10 69 102
noteon 10 81 102
sleep 3.597
noteon 11 78 102
sleep 7.194
noteon 12 54 102
sleep 8.992
noteon 13 54 104
sleep 1.798
noteon 3 54 100
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 14 42 106
sleep 64.748
noteoff 10 81 0
noteoff 10 69 0
sleep 3.597
noteoff 11 78 0
sleep 14.388
noteon 10 81 102
noteon 10 69 102
sleep 3.597
noteon 11 78 102
sleep 7.194
noteoff 12 54 0
sleep 8.992
noteoff 13 54 0
sleep 1.798
noteoff 3 54 0
sleep 3.597
noteoff 14 42 0
sleep 64.748
noteoff 10 69 0
noteoff 10 81 0
sleep 3.597
noteoff 11 78 0
sleep 14.388
echo "44400 tempo_s=310 tempo_l=0.25"
noteon 10 69 102
noteon 10 81 102
sleep 3.225
noteon 11 78 102
sleep 6.451
noteon 12 57 102
sleep 8.064
noteon 13 57 104
sleep 1.612
noteon 3 57 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 78 0
sleep 12.903
noteon 10 69 102
noteon 10 81 102
sleep 3.225
noteon 11 78 102
sleep 6.451
noteoff 12 57 0
sleep 8.064
noteoff 13 57 0
sleep 1.612
noteoff 3 57 0
sleep 3.225
noteoff 14 45 0
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 78 0
sleep 12.903
noteon 10 69 102
noteon 10 81 102
sleep 3.225
noteon 11 78 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 62 104
sleep 1.612
noteon 3 62 100
sleep 3.225
noteon 14 50 106
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 78 0
sleep 12.903
noteon 10 69 102
noteon 10 81 102
sleep 3.225
noteon 11 78 102
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 62 0
sleep 1.612
noteoff 3 62 0
sleep 3.225
noteoff 14 50 0
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 78 0
sleep 12.903
echo "44640 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
noteon 10 69 102
sleep 1.798
noteoff 0 90 0
noteon 0 86 101
sleep 1.798
noteoff 1 78 0
noteon 11 74 102
noteon 1 74 100
sleep 7.193
noteon 12 66 102
sleep 8.992
noteon 13 66 104
sleep 1.798
noteoff 3 66 0
noteon 3 54 100
noteon 3 62 100
sleep 3.596
noteon 14 54 106
sleep 64.734
noteoff 10 69 0
noteoff 10 81 0
sleep 3.596
noteoff 11 74 0
sleep 14.385
noteon 10 81 102
noteon 10 69 102
sleep 3.596
noteon 11 74 102
sleep 86.308
noteoff 10 69 0
noteoff 10 81 0
sleep 3.596
noteoff 11 74 0
sleep 14.385
noteon 10 81 102
noteon 10 69 102
sleep 3.596
noteon 11 74 102
sleep 86.325
noteoff 10 69 0
noteoff 10 81 0
sleep 3.597
noteoff 11 74 0
sleep 14.388
noteon 10 81 102
noteon 10 69 102
sleep 3.596
noteon 11 74 102
sleep 86.328
noteoff 10 69 0
noteoff 10 81 0
sleep 3.597
noteoff 11 74 0
sleep 14.388
echo "44880 tempo_s=310 tempo_l=0.25"
noteon 10 69 102
noteon 10 81 102
sleep 3.224
noteon 11 74 102
sleep 77.419
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 74 0
sleep 12.903
noteon 10 81 102
noteon 10 69 102
sleep 3.225
noteon 11 74 102
sleep 70.967
noteoff 12 66 0
sleep 6.451
noteoff 10 69 0
noteoff 10 81 0
sleep 1.612
noteoff 13 66 0
sleep 1.612
noteoff 3 62 0
noteoff 3 54 0
noteoff 11 74 0
sleep 3.225
noteoff 14 54 0
sleep 9.677
noteon 10 81 102
noteon 10 69 102
sleep 1.612
noteoff 0 86 0
noteon 0 81 101
sleep 1.612
noteoff 1 74 0
noteon 1 69 100
noteon 11 69 102
sleep 6.451
noteon 12 51 102
sleep 8.064
noteon 13 51 104
sleep 1.612
noteon 3 51 100
noteon 3 57 100
sleep 3.225
noteon 14 39 106
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 69 0
sleep 12.903
noteon 10 69 102
noteon 10 81 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteoff 12 51 0
sleep 8.064
noteoff 13 51 0
sleep 1.612
noteoff 3 57 0
noteoff 3 51 0
sleep 3.225
noteoff 14 39 0
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 69 0
sleep 12.903
echo "45120 tempo_s=287 tempo_l=0.25"
noteon 10 69 102
noteon 10 81 102
sleep 1.742
noteoff 0 81 0
noteon 0 91 101
noteon 0 81 101
sleep 1.742
noteoff 1 69 0
noteoff 4 69 0
noteon 11 69 102
noteon 1 79 100
noteon 1 69 100
noteon 4 69 100
noteon 11 79 102
sleep 1.742
noteoff 6 69 0
noteon 6 69 108
sleep 3.484
noteon 2 79 101
noteon 2 69 101
sleep 1.742
noteoff 5 45 0
noteon 12 52 102
noteon 5 67 100
sleep 8.71
noteon 13 52 104
sleep 1.742
noteon 3 52 100
sleep 1.742
noteon 15 45 81
sleep 1.742
noteon 14 40 106
sleep 62.717
noteoff 10 81 0
noteoff 10 69 0
sleep 3.484
noteoff 11 79 0
noteoff 11 69 0
sleep 13.937
noteon 10 69 102
noteon 10 81 102
sleep 1.742
noteoff 15 45 0
sleep 1.742
noteon 11 69 102
noteon 11 79 102
sleep 6.968
noteoff 12 52 0
sleep 8.71
noteoff 13 52 0
sleep 1.742
noteoff 3 52 0
sleep 1.742
noteon 15 45 81
sleep 1.742
noteoff 14 40 0
sleep 62.717
noteoff 10 81 0
noteoff 10 69 0
sleep 3.484
noteoff 11 79 0
noteoff 11 69 0
sleep 13.937
noteon 10 69 102
noteon 10 81 102
sleep 1.742
noteoff 15 45 0
sleep 1.742
noteon 11 69 102
noteon 11 79 102
sleep 6.968
noteon 12 55 102
sleep 8.71
noteon 13 55 104
sleep 1.742
noteon 3 55 100
sleep 1.742
noteon 15 45 81
sleep 1.742
noteon 14 43 106
sleep 62.717
noteoff 10 81 0
noteoff 10 69 0
sleep 3.484
noteoff 11 79 0
noteoff 11 69 0
sleep 13.937
noteon 10 69 102
noteon 10 81 102
sleep 1.742
noteoff 15 45 0
sleep 1.742
noteon 11 79 102
noteon 11 69 102
sleep 6.968
noteoff 12 55 0
sleep 8.71
noteoff 13 55 0
sleep 1.742
noteoff 3 55 0
sleep 1.742
noteon 15 45 81
sleep 1.742
noteoff 14 43 0
sleep 62.717
noteoff 10 81 0
noteoff 10 69 0
sleep 3.484
noteoff 11 69 0
noteoff 11 79 0
sleep 13.937
echo "45360 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
noteon 10 69 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 79 102
noteon 11 69 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteon 13 61 104
sleep 1.612
noteon 3 61 100
sleep 1.612
noteon 15 45 81
sleep 1.612
noteon 14 49 106
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 69 0
noteoff 11 79 0
sleep 12.903
noteon 10 81 102
noteon 10 69 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 69 102
noteon 11 79 102
sleep 6.451
noteoff 12 61 0
sleep 8.064
noteoff 13 61 0
sleep 1.612
noteoff 3 61 0
sleep 1.612
noteon 15 45 81
sleep 1.612
noteoff 14 49 0
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
noteoff 11 69 0
sleep 12.903
noteon 10 69 102
noteon 10 81 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 69 102
noteon 11 79 102
sleep 6.451
noteon 12 64 102
sleep 8.064
noteon 13 64 104
sleep 1.612
noteon 3 64 100
sleep 1.612
noteon 15 45 81
sleep 1.612
noteon 14 52 106
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 79 0
noteoff 11 69 0
sleep 12.903
noteon 10 69 102
noteon 10 81 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 79 102
noteon 11 69 102
sleep 6.451
noteoff 12 64 0
sleep 8.064
noteoff 13 64 0
sleep 1.612
noteoff 3 64 0
sleep 1.612
noteon 15 45 81
sleep 1.612
noteoff 14 52 0
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 69 0
noteoff 11 79 0
sleep 12.903
echo "45600 tempo_s=287 tempo_l=0.25"
noteon 10 81 102
noteon 10 69 102
sleep 1.742
noteoff 0 81 0
noteoff 0 91 0
noteoff 15 45 0
noteon 0 76 101
sleep 1.742
noteoff 1 69 0
noteoff 1 79 0
noteon 1 64 100
noteon 11 69 102
noteon 11 79 102
sleep 6.968
noteon 12 67 102
sleep 8.71
noteon 13 67 104
sleep 1.742
noteon 3 67 100
sleep 1.742
noteon 15 45 81
sleep 1.742
noteon 14 55 106
sleep 62.717
noteoff 10 69 0
noteoff 10 81 0
sleep 3.484
noteoff 11 79 0
noteoff 11 69 0
sleep 13.937
noteon 10 69 102
noteon 10 81 102
sleep 1.742
noteoff 0 76 0
noteoff 15 45 0
sleep 1.742
noteoff 1 64 0
noteon 11 79 102
noteon 11 69 102
sleep 6.968
noteoff 12 67 0
sleep 8.71
noteoff 13 67 0
sleep 1.742
noteoff 3 67 0
sleep 1.742
noteon 15 45 81
sleep 1.742
noteoff 14 55 0
sleep 62.717
noteoff 10 81 0
noteoff 10 69 0
sleep 3.484
noteoff 11 69 0
noteoff 11 79 0
sleep 13.937
noteon 10 81 102
noteon 10 69 102
sleep 1.742
noteoff 15 45 0
noteon 0 79 101
sleep 1.742
noteon 1 67 100
noteon 11 79 102
noteon 11 69 102
sleep 6.968
noteon 12 64 102
sleep 8.71
noteon 13 64 104
sleep 1.742
noteon 3 64 100
sleep 1.742
noteon 15 45 81
sleep 1.742
noteon 14 52 106
sleep 62.717
noteoff 10 69 0
noteoff 10 81 0
sleep 3.484
noteoff 11 69 0
noteoff 11 79 0
sleep 13.937
noteon 10 69 102
noteon 10 81 102
sleep 1.742
noteoff 0 79 0
noteoff 15 45 0
sleep 1.742
noteoff 1 67 0
noteon 11 69 102
noteon 11 79 102
sleep 6.968
noteoff 12 64 0
sleep 8.71
noteoff 13 64 0
sleep 1.742
noteoff 3 64 0
sleep 1.742
noteon 15 45 81
sleep 1.742
noteoff 14 52 0
sleep 62.717
noteoff 10 81 0
noteoff 10 69 0
sleep 3.484
noteoff 11 79 0
noteoff 11 69 0
sleep 13.937
echo "45840 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
noteon 10 69 102
sleep 1.612
noteoff 15 45 0
noteon 0 85 101
sleep 1.612
noteon 1 73 100
noteon 11 69 102
noteon 11 79 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteon 13 61 104
sleep 1.612
noteon 3 61 100
sleep 1.612
noteon 15 45 81
sleep 1.612
noteon 14 49 106
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
noteoff 11 69 0
sleep 12.903
noteon 10 81 102
noteon 10 69 102
sleep 1.612
noteoff 0 85 0
noteoff 15 45 0
sleep 1.612
noteoff 1 73 0
noteon 11 69 102
noteon 11 79 102
sleep 6.451
noteoff 12 61 0
sleep 8.064
noteoff 13 61 0
sleep 1.612
noteoff 3 61 0
sleep 1.612
noteon 15 45 81
sleep 1.612
noteoff 14 49 0
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
noteoff 11 69 0
sleep 12.903
noteon 10 81 102
noteon 10 69 102
sleep 1.612
noteoff 15 45 0
noteon 0 88 101
sleep 1.612
noteon 1 76 100
noteon 11 69 102
noteon 11 79 102
sleep 6.451
noteon 12 55 102
sleep 8.064
noteon 13 55 104
sleep 1.612
noteon 3 55 100
sleep 1.612
noteon 15 45 81
sleep 1.612
noteon 14 43 106
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
noteoff 11 69 0
sleep 12.903
noteon 10 69 102
noteon 10 81 102
sleep 1.612
noteoff 0 88 0
noteoff 15 45 0
sleep 1.612
noteoff 1 76 0
noteon 11 69 102
noteon 11 79 102
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 55 0
sleep 1.612
noteoff 3 55 0
sleep 1.612
noteon 15 45 81
sleep 1.612
noteoff 14 43 0
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 79 0
noteoff 11 69 0
sleep 12.903
echo "46080 tempo_s=287 tempo_l=0.25"
noteon 10 81 102
noteon 10 69 102
sleep 1.742
noteoff 15 45 0
noteon 0 91 101
sleep 1.742
noteon 1 79 100
noteon 11 69 102
noteon 11 79 102
sleep 6.968
noteon 12 52 102
sleep 8.71
noteon 13 52 104
sleep 1.742
noteon 3 52 100
sleep 1.742
noteon 15 45 81
sleep 1.742
noteon 14 40 106
sleep 62.717
noteoff 10 69 0
noteoff 10 81 0
sleep 3.484
noteoff 11 79 0
noteoff 11 69 0
sleep 13.937
noteon 10 81 102
noteon 10 69 102
sleep 1.742
noteoff 0 91 0
noteoff 15 45 0
sleep 1.742
noteoff 1 79 0
noteon 11 69 102
noteon 11 79 102
sleep 6.968
noteoff 12 52 0
sleep 8.71
noteoff 13 52 0
sleep 1.742
noteoff 3 52 0
sleep 1.742
noteon 15 45 81
sleep 1.742
noteoff 14 40 0
sleep 62.717
noteoff 10 69 0
noteoff 10 81 0
sleep 3.484
noteoff 11 79 0
noteoff 11 69 0
sleep 13.937
noteon 10 69 102
noteon 10 81 102
sleep 1.742
noteoff 15 45 0
noteon 0 88 101
sleep 1.742
noteon 1 76 100
noteon 11 79 102
noteon 11 69 102
sleep 6.968
noteon 12 55 102
sleep 8.71
noteon 13 55 104
sleep 1.742
noteon 3 55 100
sleep 1.742
noteon 15 45 81
sleep 1.742
noteon 14 43 106
sleep 62.717
noteoff 10 81 0
noteoff 10 69 0
sleep 3.484
noteoff 11 69 0
noteoff 11 79 0
sleep 13.937
noteon 10 69 102
noteon 10 81 102
sleep 1.742
noteoff 0 88 0
noteoff 15 45 0
sleep 1.742
noteoff 1 76 0
noteon 11 69 102
noteon 11 79 102
sleep 6.968
noteoff 12 55 0
sleep 8.71
noteoff 13 55 0
sleep 1.742
noteoff 3 55 0
sleep 1.742
noteon 15 45 81
sleep 1.742
noteoff 14 43 0
sleep 62.717
noteoff 10 81 0
noteoff 10 69 0
sleep 3.484
noteoff 11 79 0
noteoff 11 69 0
sleep 13.937
echo "46320 tempo_s=310 tempo_l=0.25"
noteon 10 69 102
noteon 10 81 102
sleep 1.612
noteoff 15 45 0
noteon 0 85 101
sleep 1.612
noteon 1 73 100
noteon 11 69 102
noteon 11 79 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteon 13 61 104
sleep 1.612
noteon 3 61 100
sleep 1.612
noteon 15 45 81
sleep 1.612
noteon 14 49 106
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 79 0
noteoff 11 69 0
sleep 12.903
noteon 10 81 102
noteon 10 69 102
sleep 1.612
noteoff 0 85 0
noteoff 15 45 0
sleep 1.612
noteoff 1 73 0
noteon 11 69 102
noteon 11 79 102
sleep 6.451
noteoff 12 61 0
sleep 8.064
noteoff 13 61 0
sleep 1.612
noteoff 3 61 0
sleep 1.612
noteon 15 45 81
sleep 1.612
noteoff 14 49 0
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
noteoff 11 69 0
sleep 12.903
noteon 10 69 102
noteon 10 81 102
sleep 1.612
noteoff 15 45 0
noteon 0 79 101
sleep 1.612
noteon 1 67 100
noteon 11 69 102
noteon 11 79 102
sleep 6.451
noteon 12 64 102
sleep 8.064
noteon 13 64 104
sleep 1.612
noteon 3 64 100
sleep 1.612
noteon 15 45 81
sleep 1.612
noteon 14 52 106
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 79 0
noteoff 11 69 0
sleep 12.903
noteon 10 81 102
noteon 10 69 102
sleep 1.612
noteoff 0 79 0
noteoff 15 45 0
sleep 1.612
noteoff 1 67 0
noteon 11 79 102
noteon 11 69 102
sleep 6.451
noteoff 12 64 0
sleep 8.064
noteoff 13 64 0
sleep 1.612
noteoff 3 64 0
sleep 1.612
noteon 15 45 81
sleep 1.612
noteoff 14 52 0
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 69 0
noteoff 11 79 0
sleep 12.903
echo "46560 tempo_s=287 tempo_l=0.25"
noteon 10 81 102
noteon 10 69 102
sleep 1.742
noteoff 15 45 0
noteon 0 76 101
sleep 1.742
noteon 1 64 100
noteon 11 69 102
noteon 11 79 102
sleep 6.968
noteon 12 67 102
sleep 8.71
noteon 13 67 104
sleep 1.742
noteon 3 67 100
sleep 1.742
noteon 15 45 81
sleep 1.742
noteon 14 55 106
sleep 62.717
noteoff 10 69 0
noteoff 10 81 0
sleep 3.484
noteoff 11 79 0
noteoff 11 69 0
sleep 13.937
noteon 10 69 102
noteon 10 81 102
sleep 1.742
noteoff 0 76 0
noteoff 15 45 0
sleep 1.742
noteoff 1 64 0
noteon 11 79 102
noteon 11 69 102
sleep 6.968
noteoff 12 67 0
sleep 8.71
noteoff 13 67 0
sleep 1.742
noteoff 3 67 0
sleep 1.742
noteon 15 45 81
sleep 1.742
noteoff 14 55 0
sleep 62.717
noteoff 10 81 0
noteoff 10 69 0
sleep 3.484
noteoff 11 69 0
noteoff 11 79 0
sleep 13.937
noteon 10 69 102
noteon 10 81 102
sleep 1.742
noteoff 15 45 0
noteon 0 79 101
sleep 1.742
noteon 1 67 100
noteon 11 69 102
noteon 11 79 102
sleep 6.968
noteon 12 64 102
sleep 8.71
noteon 13 64 104
sleep 1.742
noteon 3 64 100
sleep 1.742
noteon 15 45 81
sleep 1.742
noteon 14 52 106
sleep 62.717
noteoff 10 81 0
noteoff 10 69 0
sleep 3.484
noteoff 11 79 0
noteoff 11 69 0
sleep 13.937
noteon 10 69 102
noteon 10 81 102
sleep 1.742
noteoff 0 79 0
noteoff 15 45 0
sleep 1.742
noteoff 1 67 0
noteon 11 69 102
noteon 11 79 102
sleep 6.968
noteoff 12 64 0
sleep 8.71
noteoff 13 64 0
sleep 1.742
noteoff 3 64 0
sleep 1.742
noteon 15 45 81
sleep 1.742
noteoff 14 52 0
sleep 62.717
noteoff 10 81 0
noteoff 10 69 0
sleep 3.484
noteoff 11 79 0
noteoff 11 69 0
sleep 13.937
echo "46800 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
noteon 10 69 102
sleep 1.612
noteoff 15 45 0
noteon 0 85 101
sleep 1.612
noteon 1 73 100
noteon 11 69 102
noteon 11 79 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteon 13 61 104
sleep 1.612
noteon 3 61 100
sleep 1.612
noteon 15 45 81
sleep 1.612
noteon 14 49 106
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
noteoff 11 69 0
sleep 12.903
noteon 10 69 102
noteon 10 81 102
sleep 1.612
noteoff 0 85 0
noteoff 15 45 0
sleep 1.612
noteoff 1 73 0
noteon 11 79 102
noteon 11 69 102
sleep 6.451
noteoff 12 61 0
sleep 8.064
noteoff 13 61 0
sleep 1.612
noteoff 3 61 0
sleep 1.612
noteon 15 45 81
sleep 1.612
noteoff 14 49 0
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 69 0
noteoff 11 79 0
sleep 12.903
noteon 10 81 102
noteon 10 69 102
sleep 1.612
noteoff 15 45 0
noteon 0 88 101
sleep 1.612
noteon 1 76 100
noteon 11 79 102
noteon 11 69 102
sleep 6.451
noteon 12 55 102
sleep 8.064
noteon 13 55 104
sleep 1.612
noteon 3 55 100
sleep 1.612
noteon 15 45 81
sleep 1.612
noteon 14 43 106
sleep 58.064
noteoff 10 69 0
noteoff 10 81 0
sleep 3.225
noteoff 11 69 0
noteoff 11 79 0
sleep 12.903
noteon 10 69 102
noteon 10 81 102
sleep 1.612
noteoff 0 88 0
noteoff 15 45 0
sleep 1.612
noteoff 1 76 0
noteon 11 69 102
noteon 11 79 102
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 55 0
sleep 1.612
noteoff 3 55 0
sleep 1.612
noteon 15 45 81
sleep 1.612
noteoff 14 43 0
sleep 58.064
noteoff 10 81 0
noteoff 10 69 0
sleep 3.225
noteoff 11 79 0
noteoff 11 69 0
sleep 12.903
echo "47040 tempo_s=248 tempo_l=0.25"
noteon 10 69 102
noteon 10 81 102
sleep 2.016
noteoff 15 45 0
noteon 0 91 101
sleep 2.016
noteon 1 79 100
noteon 11 79 102
noteon 11 69 102
sleep 8.064
noteoff 5 67 0
noteon 5 57 100
noteon 12 52 102
sleep 10.08
noteon 13 52 104
sleep 2.016
noteon 3 52 100
sleep 2.016
noteon 15 45 90
sleep 2.016
noteon 14 40 106
sleep 94.752
noteoff 0 91 0
sleep 2.016
noteoff 1 79 0
sleep 20.160
noteoff 3 52 0
sleep 96.768
echo "47160 tempo_s=234 tempo_l=0.25"
noteoff 10 81 0
noteoff 10 69 0
sleep 4.272
noteoff 4 69 0
noteoff 11 69 0
noteoff 11 79 0
sleep 2.136
noteoff 6 69 0
sleep 4.273
noteoff 2 69 0
noteoff 2 79 0
sleep 2.136
noteoff 5 57 0
noteoff 12 52 0
sleep 10.683
noteoff 13 52 0
sleep 2.136
noteon 3 55 100
sleep 2.136
noteoff 15 45 0
sleep 2.136
noteoff 14 40 0
sleep 98.288
echo "47220 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 21.581
noteoff 3 55 0
sleep 86.325
echo "47280 tempo_s=290 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 20.688
noteon 3 61 100
sleep 103.448
noteoff 3 61 0
sleep 82.758
noteoff 10 79 0
sleep 20.689
noteon 3 64 100
sleep 82.758
noteon 10 75 102
sleep 20.689
noteoff 3 64 0
sleep 82.758
echo "47520 tempo_s=285 tempo_l=0.25"
noteoff 10 75 0
noteon 10 76 102
sleep 21.052
noteon 3 67 100
sleep 105.263
noteoff 3 67 0
sleep 84.21
noteoff 10 76 0
sleep 21.052
noteon 3 64 100
sleep 84.21
noteon 10 72 102
sleep 21.052
noteoff 3 64 0
sleep 84.21
noteoff 10 72 0
noteon 10 73 102
sleep 21.052
noteon 3 61 100
sleep 105.263
noteoff 3 61 0
sleep 84.21
noteoff 10 73 0
sleep 21.052
noteon 3 55 100
sleep 84.21
noteon 10 68 102
sleep 21.052
noteoff 3 55 0
sleep 84.21
echo "48000 tempo_s=277 tempo_l=0.25"
noteoff 10 68 0
noteon 10 69 102
sleep 21.66
noteon 3 52 100
sleep 108.303
noteoff 3 52 0
sleep 86.642
noteoff 10 69 0
sleep 21.66
noteon 3 55 100
sleep 90.252
noteon 11 66 82
sleep 18.05
noteoff 3 55 0
sleep 90.252
noteoff 11 66 0
noteon 11 67 102
sleep 18.05
noteon 3 61 100
sleep 108.303
noteoff 3 61 0
sleep 90.252
noteoff 11 67 0
sleep 18.05
noteon 3 64 100
sleep 90.252
noteon 11 63 102
sleep 18.05
noteoff 3 64 0
sleep 86.642
echo "48480 tempo_s=273 tempo_l=0.25"
sleep 3.663
noteoff 11 63 0
noteon 11 64 102
sleep 18.315
noteon 3 67 100
sleep 109.89
noteoff 3 67 0
sleep 91.575
noteoff 11 64 0
sleep 18.315
noteon 3 64 100
sleep 91.575
noteon 11 60 102
sleep 18.315
noteoff 3 64 0
sleep 91.575
noteoff 11 60 0
noteon 11 61 102
sleep 18.315
noteon 3 61 100
sleep 109.89
noteoff 3 61 0
sleep 91.575
noteoff 11 61 0
sleep 18.315
noteon 3 55 100
sleep 91.575
noteon 11 56 102
sleep 18.315
noteoff 3 55 0
sleep 87.912
echo "48960 tempo_s=271 tempo_l=0.25"
sleep 3.69
noteoff 11 56 0
noteon 11 57 102
sleep 18.45
noteon 3 52 100
sleep 110.700
noteoff 3 52 0
sleep 92.250
noteoff 11 57 0
sleep 18.450
noteon 3 55 100
sleep 88.560
noteon 10 78 82
sleep 22.14
noteoff 3 55 0
sleep 88.560
noteoff 10 78 0
noteon 10 79 102
sleep 22.140
noteon 3 61 100
sleep 110.700
noteoff 3 61 0
sleep 88.560
noteoff 10 79 0
sleep 22.14
noteon 3 64 100
sleep 88.560
noteon 10 75 102
sleep 22.14
noteoff 3 64 0
sleep 88.560
echo "49440 tempo_s=269 tempo_l=0.25"
noteoff 10 75 0
noteon 10 76 102
sleep 22.304
noteon 3 67 100
sleep 111.522
noteoff 3 67 0
sleep 89.217
noteoff 10 76 0
sleep 3.717
noteon 11 55 102
sleep 18.587
noteon 3 64 100
sleep 89.218
noteon 10 72 102
sleep 3.717
noteoff 11 55 0
sleep 18.586
noteoff 3 64 0
sleep 89.218
noteoff 10 72 0
noteon 10 73 102
sleep 3.717
noteon 11 61 102
sleep 18.587
noteon 3 61 100
sleep 92.935
noteoff 11 61 0
sleep 18.587
noteoff 3 61 0
sleep 89.218
noteoff 10 73 0
sleep 3.717
noteon 11 64 102
sleep 18.587
noteon 3 55 100
sleep 89.217
noteon 10 68 102
sleep 3.717
noteoff 11 64 0
sleep 18.587
noteoff 3 55 0
sleep 89.218
echo "49920 tempo_s=266 tempo_l=0.25"
noteoff 10 68 0
noteon 10 69 102
sleep 3.759
noteon 11 67 102
sleep 18.796
noteon 3 52 100
sleep 93.983
noteoff 11 67 0
sleep 18.796
noteoff 3 52 0
sleep 90.224
noteoff 10 69 0
sleep 112.781
noteon 10 72 102
sleep 112.781
noteoff 10 72 0
noteon 10 73 102
sleep 225.563
noteoff 10 73 0
sleep 112.781
noteon 10 75 102
sleep 112.781
echo "50400 tempo_s=269 tempo_l=0.25"
noteoff 10 75 0
noteon 10 76 102
sleep 223.048
noteoff 10 76 0
sleep 111.524
noteon 10 78 102
sleep 111.524
noteoff 10 78 0
noteon 10 79 102
sleep 223.048
noteoff 10 79 0
sleep 111.524
noteon 10 78 102
sleep 111.524
noteoff 10 78 0
noteon 10 79 102
sleep 223.048
noteoff 10 79 0
sleep 111.524
noteon 10 78 102
sleep 111.524
echo "51120 tempo_s=264 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 227.272
noteoff 10 79 0
sleep 113.636
noteon 10 78 102
sleep 113.636
noteoff 10 78 0
noteon 10 79 102
sleep 227.272
noteoff 10 79 0
sleep 113.636
noteon 10 78 102
sleep 113.636
echo "51600 tempo_s=260 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 230.769
echo "51720 tempo_s=307 tempo_l=0.25"
noteoff 10 79 0
sleep 97.719
noteon 10 78 102
sleep 1.628
noteon 0 90 101
sleep 1.628
noteon 1 78 100
noteon 11 66 102
sleep 16.286
noteon 3 66 100
sleep 78.175
echo "51840 tempo_s=278 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteoff 11 66 0
noteon 1 79 100
noteon 11 67 102
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 86.33
noteoff 10 79 0
sleep 1.798
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
noteoff 11 67 0
sleep 17.985
noteoff 3 67 0
sleep 86.33
echo "51960 tempo_s=246 tempo_l=0.25"
sleep 121.951
noteon 10 61 102
sleep 2.032
noteon 0 73 101
sleep 2.032
noteon 1 73 100
noteon 11 61 102
sleep 8.13
noteon 12 61 102
sleep 10.162
noteon 13 49 104
sleep 2.032
noteon 3 49 100
sleep 4.065
noteon 14 37 106
sleep 73.17
noteoff 10 61 0
sleep 2.032
noteoff 0 73 0
sleep 2.032
noteoff 1 73 0
noteoff 11 61 0
sleep 8.13
noteoff 12 61 0
sleep 8.13
echo "52080 tempo_s=310 tempo_l=0.25"
noteon 10 62 102
sleep 1.612
noteoff 13 49 0
noteon 0 74 101
sleep 1.612
noteoff 3 49 0
noteon 1 74 100
noteon 11 62 102
sleep 3.225
noteoff 14 37 0
sleep 3.224
noteon 12 62 102
sleep 8.063
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 3.224
noteon 14 38 106
sleep 4.837
noteoff 0 74 0
noteon 0 73 101
sleep 4.836
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 6.448
noteoff 12 62 0
noteon 12 61 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 1.612
noteoff 0 73 0
noteon 0 74 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.450
noteoff 12 61 0
noteon 12 62 102
sleep 4.836
noteoff 0 74 0
noteon 0 73 101
sleep 8.062
noteoff 14 38 0
noteon 14 37 106
sleep 9.674
noteoff 10 62 0
noteon 10 61 102
sleep 3.224
noteoff 1 74 0
noteoff 11 61 0
noteon 1 73 100
noteon 11 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 8.062
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 12.898
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 1 73 0
noteon 1 74 100
sleep 6.448
noteoff 12 61 0
noteon 12 62 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 4.836
noteoff 0 73 0
noteon 0 74 101
sleep 4.838
noteoff 10 62 0
noteon 10 61 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 74 0
noteoff 3 50 0
noteon 1 73 100
noteon 3 49 100
sleep 6.450
noteoff 12 62 0
noteon 12 61 102
sleep 11.286
noteoff 0 74 0
noteon 0 73 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 3.224
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 6.449
noteoff 12 61 0
noteon 12 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 3.224
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.224
noteoff 14 38 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 4.838
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 9.677
noteoff 11 62 0
noteon 11 61 102
sleep 12.903
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 73 0
noteoff 13 50 0
noteon 0 74 101
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.451
noteoff 12 61 0
noteon 12 62 102
sleep 12.903
noteoff 14 37 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 59 102
sleep 1.612
noteoff 0 74 0
noteon 0 71 101
sleep 1.612
noteoff 1 74 0
noteoff 11 61 0
noteon 1 71 100
noteon 11 59 102
sleep 3.225
noteoff 14 37 0
sleep 3.225
noteoff 12 62 0
noteon 12 59 102
sleep 8.064
noteoff 13 49 0
noteon 13 47 104
sleep 1.612
noteoff 3 49 0
noteon 3 47 100
sleep 3.225
noteon 14 35 106
sleep 25.806
noteoff 10 59 0
noteon 10 61 102
sleep 1.612
noteoff 0 71 0
noteon 0 73 101
sleep 1.612
noteoff 1 71 0
noteoff 11 59 0
noteon 1 73 100
noteon 11 61 102
sleep 6.451
noteoff 12 59 0
noteon 12 61 102
sleep 8.064
noteoff 13 47 0
noteon 13 49 104
sleep 1.612
noteoff 3 47 0
noteon 3 49 100
sleep 3.225
noteoff 14 35 0
noteon 14 37 106
sleep 25.806
echo "52320 tempo_s=278 tempo_l=0.25"
noteoff 10 61 0
noteon 10 64 102
sleep 1.798
noteoff 0 73 0
noteon 0 76 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 76 100
noteon 11 64 102
sleep 7.194
noteoff 12 61 0
noteon 12 64 102
sleep 8.992
noteoff 13 49 0
noteon 13 52 104
sleep 1.798
noteoff 3 49 0
noteon 3 52 100
sleep 3.597
noteoff 14 37 0
noteon 14 40 106
sleep 82.733
noteoff 10 64 0
sleep 1.798
noteoff 0 76 0
sleep 1.798
noteoff 1 76 0
noteoff 11 64 0
sleep 7.194
noteoff 12 64 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 3.597
noteoff 14 40 0
sleep 82.733
noteon 10 57 102
sleep 1.798
noteon 0 69 101
sleep 1.798
noteon 1 69 100
noteon 11 57 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
sleep 3.597
noteon 14 33 106
sleep 82.733
noteoff 10 57 0
sleep 1.798
noteoff 0 69 0
sleep 1.798
noteoff 1 69 0
noteoff 11 57 0
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
sleep 3.597
noteoff 14 33 0
sleep 82.733
echo "52560 tempo_s=310 tempo_l=0.25"
sleep 193.548
noteon 10 69 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteon 12 67 102
sleep 8.064
noteon 13 67 104
sleep 30.645
noteoff 10 69 0
sleep 48.387
noteon 10 69 102
sleep 48.387
noteoff 10 69 0
sleep 48.387
echo "52800 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 10.791
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 67 0
noteon 13 66 104
sleep 88.129
noteoff 10 74 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 17.985
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
echo "53040 tempo_s=310 tempo_l=0.25"
noteon 10 76 102
sleep 9.677
noteoff 12 66 0
noteon 12 61 102
sleep 8.064
noteoff 13 66 0
noteon 13 61 104
sleep 79.032
noteoff 10 76 0
noteon 10 69 102
sleep 80.645
noteoff 10 69 0
sleep 16.129
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 48.387
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 41.935
noteoff 12 61 0
sleep 6.451
echo "53280 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 1.798
noteoff 13 61 0
sleep 8.992
noteon 12 62 102
sleep 8.992
noteon 13 62 104
sleep 88.129
noteoff 10 78 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 17.985
noteon 10 81 102
sleep 3.597
noteoff 11 69 0
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
echo "53520 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 3.225
noteon 11 71 102
sleep 6.451
noteoff 12 62 0
noteon 12 67 102
sleep 8.064
noteoff 13 62 0
noteon 13 55 104
sleep 4.838
noteon 14 31 106
sleep 74.193
noteoff 10 81 0
noteon 10 79 102
sleep 80.645
noteoff 10 79 0
sleep 16.129
noteon 10 78 102
sleep 22.58
noteoff 14 31 0
sleep 25.806
noteoff 10 78 0
sleep 48.387
noteon 10 76 102
sleep 48.387
noteoff 10 76 0
sleep 48.387
echo "53760 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 3.597
noteoff 11 71 0
noteon 11 69 102
sleep 7.194
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 55 0
noteon 13 57 104
sleep 5.395
noteon 14 33 106
sleep 82.733
noteoff 10 76 0
noteon 10 74 102
sleep 89.928
noteoff 10 74 0
sleep 17.985
noteon 10 74 102
sleep 25.179
noteoff 14 33 0
sleep 28.776
noteoff 10 74 0
sleep 53.956
noteon 10 74 102
sleep 53.956
noteoff 10 74 0
sleep 53.956
echo "54000 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 3.225
noteoff 11 69 0
noteon 4 64 115
noteon 11 67 102
sleep 6.451
noteoff 12 66 0
noteon 5 57 115
noteon 12 64 102
sleep 8.064
noteoff 13 57 0
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 74.193
noteoff 10 74 0
noteon 10 73 102
sleep 80.645
noteoff 10 73 0
sleep 16.129
noteon 10 71 102
sleep 22.58
noteoff 14 33 0
sleep 25.806
noteoff 10 71 0
sleep 48.387
noteon 10 73 102
sleep 48.387
noteoff 10 73 0
sleep 35.483
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
echo "54240 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteoff 13 45 0
noteon 0 86 101
sleep 1.798
noteoff 4 64 0
noteon 1 74 100
noteon 1 66 100
noteon 11 66 102
noteon 4 62 100
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 74 101
noteon 2 66 101
sleep 1.798
noteoff 5 57 0
noteon 12 62 102
noteon 5 54 100
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 62 100
noteon 3 50 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 82.733
noteoff 10 74 0
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteoff 1 66 0
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 66 0
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 3.597
noteoff 2 66 0
noteoff 2 74 0
sleep 1.798
noteoff 5 54 0
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
noteoff 3 62 0
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 82.733
noteon 10 76 102
noteon 10 67 102
sleep 1.798
noteon 0 85 101
noteon 0 88 101
sleep 1.798
noteon 1 76 100
noteon 1 69 100
noteon 4 64 100
noteon 11 61 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 69 101
noteon 2 76 101
sleep 1.798
noteon 5 57 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 57 100
noteon 3 45 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 82.733
noteoff 10 67 0
noteoff 10 76 0
sleep 1.798
noteoff 0 88 0
noteoff 0 85 0
sleep 1.798
noteoff 1 69 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 11 61 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 76 0
noteoff 2 69 0
sleep 1.798
noteoff 5 57 0
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
noteoff 3 57 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 82.733
echo "54480 tempo_s=257 tempo_l=0.25"
sleep 233.463
echo "54600 tempo_s=310 tempo_l=0.25"
sleep 96.774
noteon 10 78 102
sleep 1.612
noteon 0 90 101
sleep 1.612
noteon 1 78 100
noteon 11 66 102
sleep 16.127
noteon 3 66 100
sleep 77.418
echo "54720 tempo_s=278 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteoff 11 66 0
noteon 1 79 100
noteon 11 67 102
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 86.33
noteoff 10 79 0
sleep 1.798
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
noteoff 11 67 0
sleep 17.985
noteoff 3 67 0
sleep 86.33
echo "54840 tempo_s=246 tempo_l=0.25"
sleep 121.951
noteon 10 61 102
sleep 2.032
noteon 0 73 101
sleep 2.032
noteon 1 73 100
noteon 11 61 102
sleep 8.13
noteon 12 61 102
sleep 10.162
noteon 13 49 104
sleep 2.032
noteon 3 49 100
sleep 4.065
noteon 14 37 106
sleep 73.17
noteoff 10 61 0
sleep 2.032
noteoff 0 73 0
sleep 2.032
noteoff 1 73 0
noteoff 11 61 0
sleep 8.13
noteoff 12 61 0
sleep 8.13
echo "54960 tempo_s=310 tempo_l=0.25"
noteon 10 62 102
sleep 1.612
noteoff 13 49 0
noteon 0 74 101
sleep 1.612
noteoff 3 49 0
noteon 1 74 100
noteon 11 62 102
sleep 3.225
noteoff 14 37 0
sleep 3.224
noteon 12 62 102
sleep 8.063
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 3.224
noteon 14 38 106
sleep 4.837
noteoff 0 74 0
noteon 0 73 101
sleep 4.836
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 6.448
noteoff 12 62 0
noteon 12 61 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 1.612
noteoff 0 73 0
noteon 0 74 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.450
noteoff 12 61 0
noteon 12 62 102
sleep 4.836
noteoff 0 74 0
noteon 0 73 101
sleep 8.062
noteoff 14 38 0
noteon 14 37 106
sleep 9.674
noteoff 10 62 0
noteon 10 61 102
sleep 3.224
noteoff 1 74 0
noteoff 11 61 0
noteon 1 73 100
noteon 11 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 8.062
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 12.898
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 1 73 0
noteon 1 74 100
sleep 6.448
noteoff 12 61 0
noteon 12 62 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 4.836
noteoff 0 73 0
noteon 0 74 101
sleep 4.838
noteoff 10 62 0
noteon 10 61 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 74 0
noteoff 3 50 0
noteon 1 73 100
noteon 3 49 100
sleep 6.450
noteoff 12 62 0
noteon 12 61 102
sleep 11.286
noteoff 0 74 0
noteon 0 73 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 3.224
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 6.449
noteoff 12 61 0
noteon 12 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 3.224
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.224
noteoff 14 38 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 4.838
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 9.677
noteoff 11 62 0
noteon 11 61 102
sleep 12.903
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 73 0
noteoff 13 50 0
noteon 0 74 101
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.451
noteoff 12 61 0
noteon 12 62 102
sleep 12.903
noteoff 14 37 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 59 102
sleep 1.612
noteoff 0 74 0
noteon 0 71 101
sleep 1.612
noteoff 1 74 0
noteoff 11 61 0
noteon 1 71 100
noteon 11 59 102
sleep 3.225
noteoff 14 37 0
sleep 3.225
noteoff 12 62 0
noteon 12 59 102
sleep 8.064
noteoff 13 49 0
noteon 13 47 104
sleep 1.612
noteoff 3 49 0
noteon 3 47 100
sleep 3.225
noteon 14 35 106
sleep 25.806
noteoff 10 59 0
noteon 10 61 102
sleep 1.612
noteoff 0 71 0
noteon 0 73 101
sleep 1.612
noteoff 1 71 0
noteoff 11 59 0
noteon 1 73 100
noteon 11 61 102
sleep 6.451
noteoff 12 59 0
noteon 12 61 102
sleep 8.064
noteoff 13 47 0
noteon 13 49 104
sleep 1.612
noteoff 3 47 0
noteon 3 49 100
sleep 3.225
noteoff 14 35 0
noteon 14 37 106
sleep 25.806
echo "55200 tempo_s=278 tempo_l=0.25"
noteoff 10 61 0
noteon 10 64 102
sleep 1.798
noteoff 0 73 0
noteon 0 76 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 76 100
noteon 4 57 100
noteon 11 64 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 64 101
noteon 2 76 101
sleep 1.798
noteoff 12 61 0
noteon 5 45 100
noteon 12 64 102
sleep 8.992
noteoff 13 49 0
noteon 13 52 104
sleep 1.798
noteoff 3 49 0
noteon 3 52 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteoff 14 37 0
noteon 14 40 106
sleep 82.733
noteoff 10 64 0
sleep 1.798
noteoff 0 76 0
sleep 1.798
noteoff 1 76 0
noteoff 4 57 0
noteoff 11 64 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 76 0
noteoff 2 64 0
sleep 1.798
noteoff 5 45 0
noteoff 12 64 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 40 0
sleep 82.733
noteon 10 57 102
sleep 1.798
noteon 0 69 101
sleep 1.798
noteon 1 69 100
noteon 4 57 100
noteon 11 57 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 57 101
noteon 2 69 101
sleep 1.798
noteon 5 45 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 82.733
noteoff 10 57 0
sleep 1.798
noteoff 0 69 0
sleep 1.798
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 57 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 69 0
noteoff 2 57 0
sleep 1.798
noteoff 5 45 0
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 82.733
echo "55440 tempo_s=310 tempo_l=0.25"
sleep 193.548
noteon 10 69 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteon 12 67 102
sleep 8.064
noteon 13 67 104
sleep 30.645
noteoff 10 69 0
sleep 48.387
noteon 10 69 102
sleep 48.387
noteoff 10 69 0
sleep 48.387
echo "55680 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 10.791
noteoff 12 67 0
noteon 12 65 102
sleep 8.992
noteoff 13 67 0
noteon 13 65 104
sleep 88.129
noteoff 10 74 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 17.985
noteon 10 77 102
sleep 53.956
noteoff 10 77 0
sleep 53.956
noteon 10 77 102
sleep 53.956
noteoff 10 77 0
sleep 53.956
echo "55920 tempo_s=310 tempo_l=0.25"
noteon 10 76 102
sleep 9.677
noteoff 12 65 0
noteon 12 61 102
sleep 8.064
noteoff 13 65 0
noteon 13 61 104
sleep 79.032
noteoff 10 76 0
noteon 10 69 102
sleep 80.645
noteoff 10 69 0
sleep 16.129
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 48.387
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 48.387
echo "56160 tempo_s=278 tempo_l=0.25"
noteon 10 77 102
sleep 10.791
noteoff 12 61 0
noteon 12 62 102
sleep 8.992
noteoff 13 61 0
noteon 13 62 104
sleep 88.129
noteoff 10 77 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 3.597
noteoff 11 69 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 65 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 65 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 65 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 65 0
sleep 14.388
echo "56400 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 3.225
noteon 11 70 102
sleep 6.451
noteoff 12 62 0
noteon 12 67 102
sleep 8.064
noteoff 13 62 0
noteon 13 55 104
sleep 4.838
noteon 14 31 106
sleep 74.193
noteoff 10 81 0
noteon 10 79 102
sleep 80.645
noteoff 10 79 0
sleep 16.129
noteon 10 77 102
sleep 22.58
noteoff 14 31 0
sleep 25.806
noteoff 10 77 0
sleep 48.387
noteon 10 76 102
sleep 48.387
noteoff 10 76 0
sleep 48.387
echo "56640 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 3.597
noteoff 11 70 0
noteon 11 69 102
sleep 7.194
noteoff 12 67 0
noteon 12 65 102
sleep 8.992
noteoff 13 55 0
noteon 13 57 104
sleep 5.395
noteon 14 33 106
sleep 82.733
noteoff 10 76 0
noteon 10 74 102
sleep 89.928
noteoff 10 74 0
sleep 17.985
noteon 10 74 102
sleep 25.179
noteoff 14 33 0
sleep 28.776
noteoff 10 74 0
sleep 53.956
noteon 10 74 102
sleep 53.956
noteoff 10 74 0
sleep 53.956
echo "56880 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 3.225
noteoff 11 69 0
noteon 4 64 115
noteon 11 67 102
sleep 6.451
noteoff 12 65 0
noteon 5 57 115
noteon 12 64 102
sleep 8.064
noteoff 13 57 0
noteon 13 57 104
sleep 4.838
noteon 14 33 106
sleep 74.193
noteoff 10 74 0
noteon 10 73 102
sleep 80.645
noteoff 10 73 0
sleep 16.129
noteon 10 71 102
sleep 22.58
noteoff 14 33 0
sleep 25.806
noteoff 10 71 0
sleep 48.387
noteon 10 73 102
sleep 48.387
noteoff 10 73 0
sleep 48.387
echo "57120 tempo_s=278 tempo_l=0.25"
noteon 10 62 102
noteon 10 74 102
sleep 3.596
noteoff 4 64 0
noteoff 11 67 0
noteon 4 62 100
noteon 11 65 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 5.394
noteoff 5 57 0
noteoff 12 64 0
noteon 12 62 102
noteon 5 50 100
sleep 8.990
noteoff 13 57 0
noteon 13 50 104
sleep 3.596
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 50.344
noteoff 11 65 0
sleep 7.193
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 5.395
noteoff 14 38 0
sleep 14.388
noteon 11 57 102
sleep 7.194
noteon 12 53 102
sleep 8.992
noteon 13 50 104
sleep 5.395
noteon 14 38 106
sleep 50.359
noteoff 11 57 0
sleep 7.194
noteoff 12 53 0
sleep 8.992
noteoff 13 50 0
sleep 5.395
noteoff 14 38 0
sleep 10.791
noteoff 10 74 0
noteoff 10 62 0
sleep 3.597
noteoff 4 62 0
noteon 11 57 102
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 5.395
noteoff 5 50 0
noteon 12 53 102
sleep 8.992
noteon 13 50 104
sleep 3.597
noteoff 15 50 0
sleep 1.798
noteon 14 38 106
sleep 50.359
noteoff 11 57 0
sleep 7.194
noteoff 12 53 0
sleep 8.992
noteoff 13 50 0
sleep 5.395
noteoff 14 38 0
sleep 10.791
noteon 10 77 102
sleep 3.597
noteon 11 57 102
sleep 7.194
noteon 12 53 102
sleep 8.992
noteon 13 50 104
sleep 5.395
noteon 14 38 106
sleep 50.359
noteoff 11 57 0
sleep 5.395
noteoff 10 77 0
sleep 1.798
noteoff 12 53 0
sleep 8.992
noteoff 13 50 0
sleep 5.395
noteoff 14 38 0
sleep 10.791
echo "57360 tempo_s=310 tempo_l=0.25"
noteon 10 79 102
sleep 3.225
noteon 11 57 102
sleep 6.450
noteon 12 53 102
sleep 8.062
noteon 13 50 104
sleep 4.837
noteon 14 38 106
sleep 9.675
noteoff 10 79 0
noteon 10 77 102
sleep 32.257
noteoff 10 77 0
noteon 10 79 102
sleep 3.225
noteoff 11 57 0
sleep 6.450
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 9.677
noteoff 10 79 0
noteon 10 77 102
sleep 3.225
noteon 11 57 102
sleep 6.451
noteon 12 53 102
sleep 8.063
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 9.676
noteoff 10 77 0
noteon 10 79 102
sleep 32.257
noteoff 10 79 0
noteon 10 77 102
sleep 3.225
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.063
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.676
noteoff 10 77 0
noteon 10 79 102
sleep 3.225
noteon 11 57 102
sleep 6.450
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 4.837
noteon 14 38 106
sleep 9.677
noteoff 10 79 0
noteon 10 77 102
sleep 32.257
noteoff 10 77 0
noteon 10 79 102
sleep 3.225
noteoff 11 57 0
sleep 6.450
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 9.677
noteoff 10 79 0
noteon 10 76 102
sleep 3.225
noteon 11 57 102
sleep 6.451
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 25.806
noteoff 10 76 0
noteon 10 77 102
sleep 19.354
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.677
echo "57600 tempo_s=269 tempo_l=0.25"
noteoff 10 77 0
noteon 10 81 102
sleep 3.717
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 81 0
sleep 3.717
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteon 10 74 102
sleep 3.717
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 74 0
sleep 3.717
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
echo "57840 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 86 101
noteon 0 81 101
sleep 1.612
noteon 1 77 100
noteon 1 74 100
noteon 4 62 100
noteon 11 57 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 69 101
noteon 2 65 101
sleep 1.612
noteon 5 50 100
noteon 12 53 102
sleep 8.062
noteon 13 50 104
sleep 1.612
noteon 3 53 100
noteon 3 57 100
sleep 1.612
noteon 15 50 80
sleep 1.612
noteon 14 38 106
sleep 45.143
noteoff 11 57 0
sleep 6.449
noteoff 12 53 0
sleep 8.060
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 12.898
noteon 11 57 102
sleep 6.449
noteon 12 53 102
sleep 8.062
noteon 13 50 104
sleep 4.836
noteon 14 38 106
sleep 45.143
noteoff 11 57 0
sleep 6.449
noteoff 12 53 0
sleep 8.060
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 12.898
noteon 11 57 102
sleep 6.449
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 3.224
noteoff 15 50 0
sleep 1.612
noteon 14 38 106
sleep 45.154
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.061
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 12.900
noteon 11 57 102
sleep 6.450
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 4.836
noteon 14 38 106
sleep 45.154
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.061
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 9.676
echo "58080 tempo_s=269 tempo_l=0.25"
noteon 10 73 102
noteon 10 62 102
sleep 1.858
noteoff 0 81 0
noteoff 0 86 0
noteon 0 82 101
noteon 0 85 101
sleep 1.858
noteoff 1 74 0
noteoff 1 77 0
noteon 1 73 100
noteon 1 79 100
noteon 11 58 102
sleep 5.575
noteoff 2 65 0
noteoff 2 69 0
noteon 2 67 101
noteon 2 70 101
sleep 1.858
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 1.858
noteoff 3 57 0
noteoff 3 53 0
noteon 3 58 100
noteon 3 55 100
sleep 1.858
noteon 15 50 80
sleep 1.858
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 14.869
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 62 0
noteoff 10 73 0
sleep 1.858
noteoff 0 85 0
noteoff 0 82 0
sleep 1.858
noteoff 1 79 0
noteoff 1 73 0
noteoff 4 62 0
noteon 11 58 102
sleep 1.858
noteoff 6 74 0
noteoff 6 62 0
sleep 3.717
noteoff 2 70 0
noteoff 2 67 0
sleep 1.858
noteoff 5 50 0
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 1.858
noteoff 3 55 0
noteoff 3 58 0
sleep 1.858
noteoff 15 50 0
sleep 1.858
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteon 10 79 102
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 5.576
noteoff 10 79 0
sleep 1.858
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
echo "58320 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 3.225
noteon 11 58 102
sleep 6.450
noteon 12 55 102
sleep 8.062
noteon 13 50 104
sleep 4.837
noteon 14 38 106
sleep 9.675
noteoff 10 81 0
noteon 10 79 102
sleep 32.257
noteoff 10 79 0
noteon 10 81 102
sleep 3.225
noteoff 11 58 0
sleep 6.450
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 9.677
noteoff 10 81 0
noteon 10 79 102
sleep 3.225
noteon 11 58 102
sleep 6.451
noteon 12 55 102
sleep 8.063
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 9.676
noteoff 10 79 0
noteon 10 81 102
sleep 32.257
noteoff 10 81 0
noteon 10 79 102
sleep 3.225
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.063
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.676
noteoff 10 79 0
noteon 10 81 102
sleep 3.225
noteon 11 58 102
sleep 6.450
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 4.837
noteon 14 38 106
sleep 9.677
noteoff 10 81 0
noteon 10 79 102
sleep 32.257
noteoff 10 79 0
noteon 10 81 102
sleep 3.225
noteoff 11 58 0
sleep 6.450
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 9.677
noteoff 10 81 0
noteon 10 78 102
sleep 3.225
noteon 11 58 102
sleep 6.451
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 25.806
noteoff 10 78 0
noteon 10 79 102
sleep 19.354
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.677
echo "58560 tempo_s=269 tempo_l=0.25"
noteoff 10 79 0
noteon 10 82 102
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 82 0
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteon 10 73 102
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 73 0
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
echo "58800 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 85 101
noteon 0 82 101
sleep 1.612
noteon 1 79 100
noteon 1 73 100
noteon 4 62 100
noteon 11 58 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 70 101
noteon 2 67 101
sleep 1.612
noteon 5 50 100
noteon 12 55 102
sleep 8.062
noteon 13 50 104
sleep 1.612
noteon 3 58 100
noteon 3 55 100
sleep 1.612
noteon 15 50 80
sleep 1.612
noteon 14 38 106
sleep 45.143
noteoff 11 58 0
sleep 6.449
noteoff 12 55 0
sleep 8.060
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 12.898
noteon 11 58 102
sleep 6.449
noteon 12 55 102
sleep 8.062
noteon 13 50 104
sleep 4.836
noteon 14 38 106
sleep 45.143
noteoff 11 58 0
sleep 6.449
noteoff 12 55 0
sleep 8.060
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 12.898
noteon 11 58 102
sleep 6.449
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 3.224
noteoff 15 50 0
sleep 1.612
noteon 14 38 106
sleep 45.154
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.061
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 12.900
noteon 11 58 102
sleep 6.450
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 4.836
noteon 14 38 106
sleep 45.154
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.061
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 9.676
echo "59040 tempo_s=269 tempo_l=0.25"
noteon 10 62 102
noteon 10 74 102
sleep 1.858
noteoff 0 82 0
noteoff 0 85 0
noteon 0 81 101
noteon 0 86 101
sleep 1.858
noteoff 1 73 0
noteoff 1 79 0
noteon 1 74 100
noteon 1 77 100
noteon 11 62 102
noteon 11 57 102
sleep 5.575
noteoff 2 67 0
noteoff 2 70 0
noteon 2 65 101
noteon 2 69 101
sleep 1.858
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 1.858
noteoff 3 55 0
noteoff 3 58 0
noteon 3 53 100
noteon 3 57 100
sleep 1.858
noteon 15 50 80
sleep 1.858
noteon 14 38 106
sleep 52.044
noteoff 11 57 0
noteoff 11 62 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 14.869
noteon 11 57 102
noteon 11 62 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 62 0
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 74 0
noteoff 10 62 0
sleep 1.858
noteoff 0 86 0
noteoff 0 81 0
sleep 1.858
noteoff 1 77 0
noteoff 1 74 0
noteoff 4 62 0
noteon 11 62 102
noteon 11 57 102
sleep 1.858
noteoff 6 74 0
noteoff 6 62 0
sleep 3.717
noteoff 2 69 0
noteoff 2 65 0
sleep 1.858
noteoff 5 50 0
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 1.858
noteoff 3 57 0
noteoff 3 53 0
sleep 1.858
noteoff 15 50 0
sleep 1.858
noteon 14 38 106
sleep 52.044
noteoff 11 57 0
noteoff 11 62 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteon 10 77 102
sleep 3.717
noteon 11 57 102
noteon 11 62 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 62 0
noteoff 11 57 0
sleep 5.576
noteoff 10 77 0
sleep 1.858
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
echo "59280 tempo_s=310 tempo_l=0.25"
noteon 10 79 102
sleep 3.225
noteon 11 57 102
noteon 11 62 102
sleep 6.450
noteon 12 53 102
sleep 8.062
noteon 13 50 104
sleep 4.837
noteon 14 38 106
sleep 9.675
noteoff 10 79 0
noteon 10 77 102
sleep 32.257
noteoff 10 77 0
noteon 10 79 102
sleep 3.225
noteoff 11 62 0
noteoff 11 57 0
sleep 6.450
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 9.677
noteoff 10 79 0
noteon 10 77 102
sleep 3.225
noteon 11 62 102
noteon 11 57 102
sleep 6.451
noteon 12 53 102
sleep 8.063
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 9.676
noteoff 10 77 0
noteon 10 79 102
sleep 32.257
noteoff 10 79 0
noteon 10 77 102
sleep 3.225
noteoff 11 57 0
noteoff 11 62 0
sleep 6.451
noteoff 12 53 0
sleep 8.063
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.676
noteoff 10 77 0
noteon 10 79 102
sleep 3.225
noteon 11 57 102
noteon 11 62 102
sleep 6.450
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 4.837
noteon 14 38 106
sleep 9.677
noteoff 10 79 0
noteon 10 77 102
sleep 32.257
noteoff 10 77 0
noteon 10 79 102
sleep 3.225
noteoff 11 62 0
noteoff 11 57 0
sleep 6.450
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 9.677
noteoff 10 79 0
noteon 10 76 102
sleep 3.225
noteon 11 62 102
noteon 11 57 102
sleep 6.451
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 25.806
noteoff 10 76 0
noteon 10 77 102
sleep 19.354
noteoff 11 57 0
noteoff 11 62 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.677
echo "59520 tempo_s=269 tempo_l=0.25"
noteoff 10 77 0
noteon 10 81 102
sleep 3.717
noteon 11 57 102
noteon 11 62 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 62 0
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 81 0
sleep 3.717
noteon 11 62 102
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 57 0
noteoff 11 62 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteon 10 74 102
sleep 3.717
noteon 11 57 102
noteon 11 62 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 62 0
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 74 0
sleep 3.717
noteon 11 62 102
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 57 0
noteoff 11 62 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
echo "59760 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 86 101
noteon 0 81 101
sleep 1.612
noteon 1 77 100
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
noteon 11 57 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 74 101
noteon 2 65 101
sleep 1.612
noteon 5 50 100
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 53 100
noteon 3 57 100
sleep 1.612
noteon 15 50 80
sleep 1.612
noteon 14 38 106
sleep 45.161
noteoff 11 57 0
noteoff 11 62 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteoff 0 81 0
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
noteoff 1 77 0
noteoff 4 62 0
noteon 11 57 102
noteon 11 62 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
noteoff 2 65 0
noteoff 2 74 0
sleep 1.612
noteoff 5 50 0
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteoff 3 57 0
noteoff 3 53 0
sleep 1.612
noteoff 15 50 0
sleep 1.612
noteon 14 38 106
sleep 45.161
noteoff 11 62 0
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteon 0 86 101
noteon 0 81 101
sleep 1.612
noteon 1 77 100
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
noteon 11 57 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 74 101
noteon 2 65 101
sleep 1.612
noteon 5 50 100
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 57 100
noteon 3 53 100
sleep 1.612
noteon 15 50 80
sleep 1.612
noteon 14 38 106
sleep 45.161
noteoff 11 57 0
noteoff 11 62 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteoff 0 81 0
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
noteoff 1 77 0
noteoff 4 62 0
noteon 11 57 102
noteon 11 62 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
noteoff 2 65 0
noteoff 2 74 0
sleep 1.612
noteoff 5 50 0
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteoff 3 53 0
noteoff 3 57 0
sleep 1.612
noteoff 15 50 0
sleep 1.612
noteon 14 38 106
sleep 45.161
noteoff 11 62 0
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.677
echo "60000 tempo_s=269 tempo_l=0.25"
noteon 10 62 102
noteon 10 73 102
sleep 3.717
noteon 11 64 102
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 3.717
noteon 15 50 80
sleep 1.858
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
noteoff 11 64 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 14.869
noteon 11 58 102
noteon 11 64 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 64 0
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 73 0
noteoff 10 62 0
sleep 3.717
noteon 11 64 102
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 3.717
noteoff 15 50 0
sleep 1.858
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
noteoff 11 64 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteon 10 79 102
sleep 3.717
noteon 11 64 102
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
noteoff 11 64 0
sleep 5.576
noteoff 10 79 0
sleep 1.858
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
echo "60240 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 3.225
noteon 11 64 102
noteon 11 58 102
sleep 6.450
noteon 12 55 102
sleep 8.062
noteon 13 50 104
sleep 4.837
noteon 14 38 106
sleep 9.675
noteoff 10 81 0
noteon 10 79 102
sleep 32.257
noteoff 10 79 0
noteon 10 81 102
sleep 3.225
noteoff 11 58 0
noteoff 11 64 0
sleep 6.450
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 9.677
noteoff 10 81 0
noteon 10 79 102
sleep 3.225
noteon 11 64 102
noteon 11 58 102
sleep 6.451
noteon 12 55 102
sleep 8.063
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 9.676
noteoff 10 79 0
noteon 10 81 102
sleep 32.257
noteoff 10 81 0
noteon 10 79 102
sleep 3.225
noteoff 11 58 0
noteoff 11 64 0
sleep 6.451
noteoff 12 55 0
sleep 8.063
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.676
noteoff 10 79 0
noteon 10 81 102
sleep 3.225
noteon 11 58 102
noteon 11 64 102
sleep 6.450
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 4.837
noteon 14 38 106
sleep 9.677
noteoff 10 81 0
noteon 10 79 102
sleep 32.257
noteoff 10 79 0
noteon 10 81 102
sleep 3.225
noteoff 11 64 0
noteoff 11 58 0
sleep 6.450
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.837
noteoff 14 38 0
sleep 9.677
noteoff 10 81 0
noteon 10 78 102
sleep 3.225
noteon 11 64 102
noteon 11 58 102
sleep 6.451
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 25.806
noteoff 10 78 0
noteon 10 79 102
sleep 19.354
noteoff 11 58 0
noteoff 11 64 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.677
echo "60480 tempo_s=269 tempo_l=0.25"
noteoff 10 79 0
noteon 10 82 102
sleep 3.717
noteon 11 64 102
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.575
noteon 14 38 106
sleep 52.038
noteoff 11 58 0
noteoff 11 64 0
sleep 7.434
noteoff 12 55 0
sleep 9.291
noteoff 13 50 0
sleep 5.575
noteoff 14 38 0
sleep 11.151
noteoff 10 82 0
sleep 3.717
noteon 11 64 102
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.575
noteon 14 38 106
sleep 52.038
noteoff 11 58 0
noteoff 11 64 0
sleep 7.434
noteoff 12 55 0
sleep 9.291
noteoff 13 50 0
sleep 5.575
noteoff 14 38 0
sleep 11.151
noteon 10 73 102
sleep 3.717
noteon 11 64 102
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
noteoff 11 64 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 73 0
sleep 3.717
noteon 11 64 102
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
noteoff 11 64 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
echo "60720 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 85 101
noteon 0 82 101
sleep 1.612
noteon 1 73 100
noteon 1 79 100
noteon 4 62 100
noteon 11 58 102
noteon 11 64 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 73 101
noteon 2 67 101
sleep 1.612
noteon 5 50 100
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 55 100
noteon 3 58 100
sleep 1.612
noteon 15 50 80
sleep 1.612
noteon 14 38 106
sleep 45.161
noteoff 11 64 0
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteoff 0 82 0
noteoff 0 85 0
sleep 1.612
noteoff 1 79 0
noteoff 1 73 0
noteoff 4 62 0
noteon 11 58 102
noteon 11 64 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
noteoff 2 67 0
noteoff 2 73 0
sleep 1.612
noteoff 5 50 0
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteoff 3 58 0
noteoff 3 55 0
sleep 1.612
noteoff 15 50 0
sleep 1.612
noteon 14 38 106
sleep 45.161
noteoff 11 64 0
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteon 0 85 101
noteon 0 82 101
sleep 1.612
noteon 1 73 100
noteon 1 79 100
noteon 4 62 100
noteon 11 64 102
noteon 11 58 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 67 101
noteon 2 73 101
sleep 1.612
noteon 5 50 100
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 55 100
noteon 3 58 100
sleep 1.612
noteon 15 50 80
sleep 1.612
noteon 14 38 106
sleep 45.161
noteoff 11 58 0
noteoff 11 64 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteoff 0 82 0
noteoff 0 85 0
sleep 1.612
noteoff 1 79 0
noteoff 1 73 0
noteoff 4 62 0
noteon 11 58 102
noteon 11 64 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
noteoff 2 73 0
noteoff 2 67 0
sleep 1.612
noteoff 5 50 0
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteoff 3 58 0
noteoff 3 55 0
sleep 1.612
noteoff 15 50 0
sleep 1.612
noteon 14 38 106
sleep 45.161
noteoff 11 64 0
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.677
echo "60960 tempo_s=269 tempo_l=0.25"
noteon 10 81 102
sleep 3.717
noteon 11 62 102
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 3.716
noteon 15 50 80
sleep 1.858
noteon 14 38 106
sleep 52.038
noteoff 11 57 0
noteoff 11 62 0
sleep 7.434
noteoff 12 53 0
sleep 9.291
noteoff 13 50 0
sleep 5.575
noteoff 14 38 0
sleep 11.151
noteoff 10 81 0
sleep 3.717
noteon 11 57 102
noteon 11 62 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.575
noteon 14 38 106
sleep 52.038
noteoff 11 62 0
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.291
noteoff 13 50 0
sleep 5.575
noteoff 14 38 0
sleep 11.151
noteon 10 74 102
sleep 3.717
noteon 11 62 102
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 3.717
noteoff 15 50 0
sleep 1.858
noteon 14 38 106
sleep 52.044
noteoff 11 57 0
noteoff 11 62 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 74 0
sleep 3.717
noteon 11 57 102
noteon 11 62 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 62 0
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
echo "61200 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 86 101
noteon 0 81 101
sleep 1.612
noteon 1 77 100
noteon 1 74 100
noteon 4 62 100
noteon 11 57 102
noteon 11 62 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 74 101
noteon 2 65 101
sleep 1.612
noteon 5 50 100
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 57 100
noteon 3 53 100
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 62 0
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteoff 0 81 0
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
noteoff 1 77 0
noteoff 4 62 0
noteon 11 62 102
noteon 11 57 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
noteoff 2 65 0
noteoff 2 74 0
sleep 1.612
noteoff 5 50 0
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteoff 3 53 0
noteoff 3 57 0
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 57 0
noteoff 11 62 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteon 0 86 101
noteon 0 81 101
sleep 1.612
noteon 1 77 100
noteon 1 74 100
noteon 4 62 100
noteon 11 57 102
noteon 11 62 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 74 101
noteon 2 65 101
sleep 1.612
noteon 5 50 100
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 53 100
noteon 3 57 100
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 62 0
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteoff 0 81 0
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
noteoff 1 77 0
noteoff 4 62 0
noteon 11 62 102
noteon 11 57 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
noteoff 2 65 0
noteoff 2 74 0
sleep 1.612
noteoff 5 50 0
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteoff 3 57 0
noteoff 3 53 0
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 57 0
noteoff 11 62 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.677
echo "61440 tempo_s=269 tempo_l=0.25"
noteon 10 82 102
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 3.716
noteon 15 50 80
sleep 1.858
noteon 14 38 106
sleep 52.038
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.291
noteoff 13 50 0
sleep 5.575
noteoff 14 38 0
sleep 11.151
noteoff 10 82 0
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.575
noteon 14 38 106
sleep 52.038
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.291
noteoff 13 50 0
sleep 5.575
noteoff 14 38 0
sleep 11.151
noteon 10 73 102
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 3.717
noteoff 15 50 0
sleep 1.858
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 73 0
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
echo "61680 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 85 101
noteon 0 82 101
sleep 1.612
noteon 1 73 100
noteon 1 79 100
noteon 4 62 100
noteon 11 58 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 67 101
noteon 2 73 101
sleep 1.612
noteon 5 50 100
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 55 100
noteon 3 58 100
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteoff 0 82 0
noteoff 0 85 0
sleep 1.612
noteoff 1 79 0
noteoff 1 73 0
noteoff 4 62 0
noteon 11 58 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
noteoff 2 73 0
noteoff 2 67 0
sleep 1.612
noteoff 5 50 0
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteoff 3 58 0
noteoff 3 55 0
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteon 0 85 101
noteon 0 82 101
sleep 1.612
noteon 1 73 100
noteon 1 79 100
noteon 4 62 100
noteon 11 58 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 67 101
noteon 2 73 101
sleep 1.612
noteon 5 50 100
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 58 100
noteon 3 55 100
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteoff 0 82 0
noteoff 0 85 0
sleep 1.612
noteoff 1 79 0
noteoff 1 73 0
noteoff 4 62 0
noteon 11 58 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
noteoff 2 73 0
noteoff 2 67 0
sleep 1.612
noteoff 5 50 0
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteoff 3 55 0
noteoff 3 58 0
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.677
echo "61920 tempo_s=269 tempo_l=0.25"
noteon 10 81 102
sleep 3.717
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 3.716
noteon 15 50 80
sleep 1.858
noteon 14 38 106
sleep 52.038
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.291
noteoff 13 50 0
sleep 5.575
noteoff 14 38 0
sleep 11.151
noteoff 10 81 0
sleep 3.717
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.575
noteon 14 38 106
sleep 52.038
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.291
noteoff 13 50 0
sleep 5.575
noteoff 14 38 0
sleep 11.151
noteon 10 74 102
sleep 3.717
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 3.717
noteoff 15 50 0
sleep 1.858
noteon 14 38 106
sleep 52.044
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 74 0
sleep 3.717
noteon 11 57 102
sleep 7.434
noteon 12 53 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 57 0
sleep 7.434
noteoff 12 53 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
echo "62160 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 86 101
noteon 0 81 101
sleep 1.612
noteon 1 77 100
noteon 1 74 100
noteon 4 62 100
noteon 11 57 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 74 101
noteon 2 65 101
sleep 1.612
noteon 5 50 100
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 53 100
noteon 3 57 100
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteoff 0 81 0
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
noteoff 1 77 0
noteoff 4 62 0
noteon 11 57 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
noteoff 2 65 0
noteoff 2 74 0
sleep 1.612
noteoff 5 50 0
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteoff 3 57 0
noteoff 3 53 0
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteon 0 86 101
noteon 0 81 101
sleep 1.612
noteon 1 77 100
noteon 1 74 100
noteon 4 62 100
noteon 11 57 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 65 101
noteon 2 74 101
sleep 1.612
noteon 5 50 100
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 53 100
noteon 3 57 100
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteoff 0 81 0
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
noteoff 1 77 0
noteoff 4 62 0
noteon 11 57 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
noteoff 2 74 0
noteoff 2 65 0
sleep 1.612
noteoff 5 50 0
noteon 12 53 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteoff 3 57 0
noteoff 3 53 0
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 57 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.677
echo "62400 tempo_s=269 tempo_l=0.25"
noteon 10 82 102
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 3.716
noteon 15 50 80
sleep 1.858
noteon 14 38 106
sleep 52.038
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.291
noteoff 13 50 0
sleep 5.575
noteoff 14 38 0
sleep 11.151
noteoff 10 82 0
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.575
noteon 14 38 106
sleep 52.038
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.291
noteoff 13 50 0
sleep 5.575
noteoff 14 38 0
sleep 11.151
noteon 10 73 102
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 3.717
noteoff 15 50 0
sleep 1.858
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
noteoff 10 73 0
sleep 3.717
noteon 11 58 102
sleep 7.434
noteon 12 55 102
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 52.044
noteoff 11 58 0
sleep 7.434
noteoff 12 55 0
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 11.152
echo "62640 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 85 101
noteon 0 82 101
sleep 1.612
noteon 1 73 100
noteon 1 79 100
noteon 4 62 100
noteon 11 58 102
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 3.225
noteon 2 67 101
noteon 2 73 101
sleep 1.612
noteon 5 50 100
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 58 100
noteon 3 55 100
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 11.29
noteoff 0 82 0
noteoff 0 85 0
sleep 1.612
noteoff 1 79 0
noteoff 1 73 0
noteoff 4 62 0
noteon 11 58 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
noteoff 2 73 0
noteoff 2 67 0
sleep 1.612
noteoff 5 50 0
noteon 12 55 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteoff 3 55 0
noteoff 3 58 0
sleep 3.225
noteon 14 38 106
sleep 45.161
noteoff 11 58 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.677
echo "62760 tempo_s=299 tempo_l=0.25"
sleep 1.672
noteon 0 85 101
noteon 0 82 101
sleep 1.672
noteon 1 73 100
noteon 1 79 100
noteon 4 62 100
noteon 11 58 102
sleep 1.672
noteon 6 62 108
noteon 6 74 108
sleep 3.344
noteon 2 67 101
noteon 2 73 101
sleep 1.672
noteon 5 50 100
noteon 12 55 102
sleep 8.361
noteon 13 50 104
sleep 1.672
noteon 3 58 100
noteon 3 55 100
sleep 3.344
noteon 14 38 106
sleep 46.822
noteoff 11 58 0
sleep 6.688
noteoff 12 55 0
sleep 8.361
noteoff 13 50 0
sleep 5.016
noteoff 14 38 0
sleep 11.705
noteoff 0 82 0
noteoff 0 85 0
sleep 1.672
noteoff 1 79 0
noteoff 1 73 0
noteoff 4 62 0
noteon 11 58 102
sleep 1.672
noteoff 6 74 0
noteoff 6 62 0
sleep 3.344
noteoff 2 73 0
noteoff 2 67 0
sleep 1.672
noteoff 5 50 0
noteon 12 55 102
sleep 8.361
noteon 13 50 104
sleep 1.672
noteoff 3 55 0
noteoff 3 58 0
sleep 3.344
noteon 14 38 106
sleep 46.822
noteoff 11 58 0
sleep 6.688
noteoff 12 55 0
sleep 8.361
noteoff 13 50 0
sleep 5.016
noteoff 14 38 0
sleep 10.033
echo "62880 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteon 0 86 86
noteon 0 81 86
sleep 1.798
noteon 1 77 85
noteon 1 74 85
noteon 4 62 85
noteon 11 57 102
sleep 1.798
noteon 6 62 93
noteon 6 74 93
sleep 3.597
noteon 2 74 86
noteon 2 65 86
sleep 1.798
noteon 5 50 85
noteon 12 53 102
sleep 8.992
noteon 13 62 104
sleep 1.798
noteon 3 57 85
noteon 3 53 85
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 176.237
noteoff 11 57 0
sleep 16.184
noteoff 0 81 0
noteoff 0 86 0
sleep 1.798
noteoff 1 74 0
noteoff 1 77 0
noteoff 4 62 0
noteon 11 69 102
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 3.597
noteoff 2 65 0
noteoff 2 74 0
sleep 1.798
noteoff 5 50 0
noteoff 12 53 0
sleep 10.790
noteoff 3 53 0
noteoff 3 57 0
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 32.370
noteoff 11 69 0
sleep 53.950
noteon 11 69 102
sleep 53.950
noteoff 11 69 0
sleep 50.353
echo "63120 tempo_s=297 tempo_l=0.25"
sleep 3.367
noteon 11 66 102
sleep 6.733
noteon 12 60 102
sleep 8.417
noteoff 13 62 0
noteon 13 60 104
sleep 85.858
noteoff 11 66 0
noteon 11 62 102
sleep 84.175
noteoff 11 62 0
sleep 16.835
noteon 11 69 102
sleep 50.505
noteoff 11 69 0
sleep 50.505
noteon 11 69 102
sleep 50.505
noteoff 11 69 0
sleep 47.138
echo "63360 tempo_s=278 tempo_l=0.25"
sleep 3.597
noteon 11 67 102
sleep 7.194
noteoff 12 60 0
noteon 12 58 102
sleep 8.992
noteoff 13 60 0
noteon 13 58 104
sleep 91.726
noteoff 11 67 0
noteon 11 64 102
sleep 89.928
noteoff 11 64 0
sleep 17.985
noteon 11 70 102
sleep 53.956
noteoff 11 70 0
sleep 53.956
noteon 11 70 102
sleep 53.956
noteoff 11 70 0
sleep 50.359
echo "63600 tempo_s=297 tempo_l=0.25"
sleep 3.367
noteon 11 69 102
sleep 6.734
noteoff 12 58 0
noteon 12 54 102
sleep 8.417
noteoff 13 58 0
noteon 13 54 104
sleep 85.858
noteoff 11 69 0
noteon 11 62 102
sleep 84.175
noteoff 11 62 0
sleep 16.835
noteon 11 72 102
sleep 50.505
noteoff 11 72 0
sleep 50.505
noteon 11 72 102
sleep 50.505
noteoff 11 72 0
sleep 47.138
echo "63840 tempo_s=278 tempo_l=0.25"
sleep 3.597
noteon 1 79 100
noteon 11 70 102
sleep 7.194
noteoff 12 54 0
noteon 12 55 102
sleep 8.992
noteoff 13 54 0
noteon 13 55 104
sleep 1.798
noteon 3 58 100
sleep 3.597
noteon 14 31 106
sleep 190.641
noteoff 10 74 0
noteon 10 79 102
sleep 3.597
noteoff 11 70 0
sleep 7.194
noteoff 12 55 0
sleep 8.992
noteoff 13 55 0
sleep 5.395
noteoff 14 31 0
sleep 28.776
noteoff 10 79 0
sleep 53.955
noteon 10 79 102
sleep 53.955
noteoff 10 79 0
sleep 53.955
echo "64080 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 3.225
noteoff 1 79 0
noteon 11 67 102
noteon 1 77 100
sleep 16.127
noteoff 3 58 0
noteon 3 59 100
sleep 77.415
noteoff 10 74 0
noteon 10 71 102
sleep 80.641
noteoff 10 71 0
sleep 16.128
noteon 10 79 102
sleep 48.384
noteoff 10 79 0
sleep 48.384
noteon 10 79 102
sleep 48.384
noteoff 10 79 0
sleep 48.384
echo "64320 tempo_s=278 tempo_l=0.25"
noteon 10 75 102
sleep 3.597
noteoff 1 77 0
noteon 1 75 100
sleep 17.985
noteoff 3 59 0
noteon 3 60 100
sleep 86.33
noteoff 10 75 0
noteon 10 72 102
sleep 89.928
noteoff 10 72 0
sleep 17.985
noteon 10 79 102
sleep 53.956
noteoff 10 79 0
sleep 53.956
noteon 10 79 102
sleep 53.956
noteoff 10 79 0
sleep 53.956
echo "64560 tempo_s=300 tempo_l=0.25"
noteon 10 77 102
sleep 3.333
noteoff 1 75 0
noteon 1 71 100
sleep 16.666
noteoff 3 60 0
noteon 3 65 100
sleep 80.0
noteoff 10 77 0
noteon 10 74 102
sleep 83.333
noteoff 10 74 0
sleep 16.666
noteon 10 79 102
sleep 50.0
noteoff 10 79 0
sleep 50.0
noteon 10 79 102
sleep 50.0
noteoff 10 79 0
sleep 50.0
echo "64800 tempo_s=278 tempo_l=0.25"
noteon 10 75 102
sleep 3.597
noteoff 1 71 0
noteon 1 72 100
sleep 7.194
noteon 12 60 102
sleep 8.992
noteon 13 60 104
sleep 1.798
noteoff 3 65 0
noteon 3 63 100
sleep 3.597
noteon 14 36 106
sleep 190.623
noteoff 10 75 0
sleep 3.597
noteoff 1 72 0
noteoff 11 67 0
noteon 11 67 102
sleep 17.983
noteoff 3 63 0
sleep 3.597
noteoff 14 36 0
sleep 32.370
noteoff 11 67 0
sleep 53.950
noteon 11 67 102
sleep 53.950
noteoff 11 67 0
sleep 50.353
echo "65040 tempo_s=310 tempo_l=0.25"
noteon 10 72 102
sleep 3.225
noteon 11 64 102
sleep 6.450
noteoff 12 60 0
noteon 12 58 102
sleep 8.063
noteoff 13 60 0
noteon 13 58 104
sleep 82.252
noteoff 11 64 0
noteon 11 60 102
sleep 80.641
noteoff 11 60 0
sleep 16.127
noteon 11 67 102
sleep 48.384
noteoff 11 67 0
sleep 48.384
noteon 11 67 102
sleep 48.384
noteoff 11 67 0
sleep 45.159
echo "65280 tempo_s=278 tempo_l=0.25"
sleep 3.597
noteon 11 65 102
sleep 7.194
noteoff 12 58 0
noteon 12 57 102
sleep 8.992
noteoff 13 58 0
noteon 13 57 104
sleep 91.726
noteoff 11 65 0
noteon 11 60 102
sleep 89.928
noteoff 11 60 0
sleep 17.985
noteon 11 69 102
sleep 53.956
noteoff 11 69 0
sleep 53.956
noteon 11 69 102
sleep 53.956
noteoff 11 69 0
sleep 50.359
echo "65520 tempo_s=300 tempo_l=0.25"
sleep 3.333
noteon 11 67 102
sleep 6.666
noteoff 12 57 0
noteon 12 52 102
sleep 8.333
noteoff 13 57 0
noteon 13 52 104
sleep 85.0
noteoff 11 67 0
noteon 11 60 102
sleep 83.333
noteoff 11 60 0
sleep 16.666
noteon 11 70 102
sleep 50.0
noteoff 11 70 0
sleep 50.0
noteon 11 70 102
sleep 50.0
noteoff 11 70 0
sleep 46.666
echo "65760 tempo_s=278 tempo_l=0.25"
sleep 3.597
noteon 1 77 100
noteon 11 68 102
sleep 7.194
noteoff 12 52 0
noteon 12 53 102
sleep 8.992
noteoff 13 52 0
noteon 13 53 104
sleep 1.798
noteon 3 56 100
sleep 3.597
noteon 14 41 106
sleep 190.641
noteoff 10 72 0
noteon 10 77 102
sleep 3.597
noteoff 11 68 0
sleep 7.194
noteoff 12 53 0
sleep 8.992
noteoff 13 53 0
sleep 5.395
noteoff 14 41 0
sleep 28.776
noteoff 10 77 0
sleep 53.955
noteon 10 77 102
sleep 53.955
noteoff 10 77 0
sleep 53.955
echo "66000 tempo_s=310 tempo_l=0.25"
noteon 10 72 102
sleep 3.225
noteoff 1 77 0
noteon 11 65 102
noteon 1 75 100
sleep 16.127
noteoff 3 56 0
noteon 3 57 100
sleep 77.415
noteoff 10 72 0
noteon 10 69 102
sleep 80.641
noteoff 10 69 0
sleep 16.128
noteon 10 77 102
sleep 48.384
noteoff 10 77 0
sleep 48.384
noteon 10 77 102
sleep 48.384
noteoff 10 77 0
sleep 48.384
echo "66240 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 3.597
noteoff 1 75 0
noteon 1 74 100
sleep 17.985
noteoff 3 57 0
noteon 3 58 100
sleep 86.33
noteoff 10 74 0
noteon 10 70 102
sleep 89.928
noteoff 10 70 0
sleep 17.985
noteon 10 77 102
sleep 53.956
noteoff 10 77 0
sleep 53.956
noteon 10 77 102
sleep 53.956
noteoff 10 77 0
sleep 53.956
echo "66480 tempo_s=300 tempo_l=0.25"
noteon 10 75 102
sleep 3.333
noteoff 1 74 0
noteon 1 69 100
sleep 16.666
noteoff 3 58 0
noteon 3 63 100
sleep 80.0
noteoff 10 75 0
noteon 10 72 102
sleep 83.333
noteoff 10 72 0
sleep 16.666
noteon 10 77 102
sleep 50.0
noteoff 10 77 0
sleep 50.0
noteon 10 77 102
sleep 50.0
noteoff 10 77 0
sleep 50.0
echo "66720 tempo_s=278 tempo_l=0.25"
noteon 10 65 102
noteon 10 74 102
sleep 3.596
noteoff 1 69 0
noteoff 11 65 0
noteon 4 62 100
noteon 1 70 100
noteon 11 65 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 5.394
noteon 5 50 100
noteon 12 58 102
sleep 8.990
noteon 13 46 104
sleep 1.798
noteoff 3 63 0
noteon 3 46 100
noteon 3 62 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 34 106
sleep 55.741
noteoff 10 74 0
noteoff 10 65 0
sleep 3.597
noteoff 11 65 0
sleep 23.381
noteon 10 74 102
noteon 10 65 102
sleep 3.597
noteon 11 70 102
noteon 11 62 102
sleep 77.338
noteoff 10 65 0
noteoff 10 74 0
sleep 3.597
noteoff 11 62 0
noteoff 11 70 0
sleep 23.381
noteon 10 65 102
noteon 10 74 102
sleep 3.597
noteoff 1 70 0
noteon 11 70 102
noteon 11 62 102
sleep 7.194
noteoff 12 58 0
sleep 8.992
noteoff 13 46 0
sleep 1.798
noteoff 3 62 0
noteoff 3 46 0
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 34 0
sleep 55.755
noteoff 10 74 0
noteoff 10 65 0
sleep 3.597
noteoff 11 62 0
noteoff 11 70 0
sleep 23.381
noteon 10 74 102
noteon 10 65 102
sleep 3.597
noteon 11 70 102
noteon 11 62 102
sleep 7.194
noteon 12 62 102
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 50 100
sleep 3.597
noteon 14 38 106
sleep 55.755
noteoff 10 65 0
noteoff 10 74 0
sleep 3.597
noteoff 11 62 0
noteoff 11 70 0
sleep 16.187
noteoff 12 62 0
sleep 7.194
echo "66960 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
noteon 10 65 102
sleep 1.612
noteoff 13 50 0
sleep 1.612
noteoff 3 50 0
noteon 11 70 102
noteon 11 62 102
sleep 3.225
noteoff 14 38 0
sleep 3.225
noteon 12 63 102
sleep 8.062
noteon 13 51 104
sleep 1.612
noteon 3 51 100
sleep 3.224
noteon 14 39 106
sleep 19.344
noteoff 12 63 0
noteon 12 62 102
sleep 24.185
noteoff 13 51 0
noteon 13 50 104
sleep 1.612
noteoff 3 51 0
noteon 3 50 100
sleep 4.838
noteoff 10 65 0
noteoff 10 74 0
sleep 1.612
noteoff 12 62 0
noteon 12 63 102
sleep 1.612
noteoff 11 62 0
noteoff 11 70 0
sleep 11.288
noteoff 14 39 0
noteon 14 38 106
sleep 9.674
noteon 10 74 102
noteon 10 65 102
sleep 3.225
noteon 11 70 102
noteon 11 62 102
sleep 6.449
noteoff 12 63 0
noteon 12 62 102
sleep 8.064
noteoff 13 50 0
noteon 13 51 104
sleep 1.612
noteoff 3 50 0
noteon 3 51 100
sleep 22.577
noteoff 12 62 0
noteon 12 63 102
sleep 12.900
noteoff 14 38 0
noteon 14 39 106
sleep 11.287
noteoff 13 51 0
noteon 13 50 104
sleep 1.612
noteoff 3 51 0
noteon 3 50 100
sleep 4.836
noteoff 10 65 0
noteoff 10 74 0
sleep 1.612
noteoff 12 63 0
noteon 12 62 102
sleep 1.612
noteoff 11 62 0
noteoff 11 70 0
sleep 20.964
noteon 10 74 102
noteon 10 65 102
sleep 3.225
noteon 11 70 102
noteon 11 62 102
sleep 6.451
noteoff 12 62 0
noteon 12 63 102
sleep 8.061
noteoff 13 50 0
noteon 13 51 104
sleep 1.612
noteoff 3 50 0
noteon 3 51 100
sleep 3.225
noteoff 14 39 0
noteon 14 38 106
sleep 19.350
noteoff 12 63 0
noteon 12 62 102
sleep 24.189
noteoff 13 51 0
noteon 13 50 104
sleep 1.612
noteoff 3 51 0
noteon 3 50 100
sleep 4.838
noteoff 10 65 0
noteoff 10 74 0
sleep 1.612
noteoff 12 62 0
noteon 12 63 102
sleep 1.612
noteoff 11 62 0
noteoff 11 70 0
sleep 11.288
noteoff 14 38 0
noteon 14 39 106
sleep 9.674
noteon 10 65 102
noteon 10 74 102
sleep 3.225
noteon 11 70 102
noteon 11 62 102
sleep 3.224
noteoff 14 39 0
sleep 3.225
noteoff 12 63 0
noteon 12 61 102
sleep 8.064
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 3 50 0
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 35.483
noteoff 12 61 0
noteon 12 62 102
sleep 8.064
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 1.612
noteoff 10 74 0
noteoff 10 65 0
sleep 3.225
noteoff 11 62 0
noteoff 11 70 0
sleep 20.967
echo "67200 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
noteon 10 65 102
sleep 1.798
noteon 0 82 101
sleep 1.798
noteoff 4 62 0
noteon 1 70 100
noteon 11 62 102
noteon 11 70 102
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 5.395
noteoff 5 50 0
noteoff 12 62 0
noteon 12 65 102
sleep 8.992
noteoff 13 50 0
noteon 13 53 104
sleep 1.798
noteoff 3 50 0
noteon 3 58 100
noteon 3 53 100
sleep 3.597
noteoff 14 38 0
noteon 14 41 106
sleep 55.755
noteoff 10 65 0
noteoff 10 74 0
sleep 3.597
noteoff 11 70 0
noteoff 11 62 0
sleep 23.381
noteon 10 74 102
noteon 10 65 102
sleep 1.798
noteoff 0 82 0
sleep 1.798
noteoff 1 70 0
noteon 11 62 102
noteon 11 70 102
sleep 7.194
noteoff 12 65 0
sleep 8.992
noteoff 13 53 0
sleep 1.798
noteoff 3 53 0
noteoff 3 58 0
sleep 3.597
noteoff 14 41 0
sleep 55.755
noteoff 10 65 0
noteoff 10 74 0
sleep 3.597
noteoff 11 70 0
noteoff 11 62 0
sleep 23.381
noteon 10 65 102
noteon 10 74 102
sleep 1.798
noteon 0 86 101
sleep 1.798
noteon 1 74 100
noteon 11 70 102
noteon 11 62 102
sleep 7.194
noteon 12 58 102
sleep 8.992
noteon 13 46 104
sleep 1.798
noteon 3 62 100
noteon 3 46 100
sleep 3.597
noteon 14 34 106
sleep 55.755
noteoff 10 74 0
noteoff 10 65 0
sleep 3.597
noteoff 11 62 0
noteoff 11 70 0
sleep 23.381
noteon 10 65 102
noteon 10 74 102
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteoff 1 74 0
noteon 11 62 102
noteon 11 70 102
sleep 7.194
noteoff 12 58 0
sleep 8.992
noteoff 13 46 0
sleep 1.798
noteoff 3 46 0
noteoff 3 62 0
sleep 3.597
noteoff 14 34 0
sleep 55.755
noteoff 10 74 0
noteoff 10 65 0
sleep 3.597
noteoff 11 70 0
noteoff 11 62 0
sleep 23.381
echo "67440 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
noteon 10 65 102
sleep 1.612
noteon 0 89 101
sleep 1.612
noteon 1 77 100
noteon 11 62 102
noteon 11 70 102
sleep 16.129
noteon 3 65 100
sleep 53.225
noteoff 10 65 0
noteoff 10 74 0
sleep 3.225
noteoff 11 70 0
noteoff 11 62 0
sleep 20.967
noteon 10 74 102
noteon 10 65 102
sleep 1.612
noteoff 0 89 0
sleep 1.612
noteoff 1 77 0
noteon 11 70 102
noteon 11 62 102
sleep 16.129
noteoff 3 65 0
sleep 53.225
noteoff 10 65 0
noteoff 10 74 0
sleep 3.225
noteoff 11 62 0
noteoff 11 70 0
sleep 20.967
noteon 10 65 102
noteon 10 74 102
sleep 1.612
noteon 0 82 101
sleep 1.612
noteon 1 70 100
noteon 11 62 102
noteon 11 70 102
sleep 16.129
noteon 3 58 100
sleep 53.225
noteoff 10 74 0
noteoff 10 65 0
sleep 3.225
noteoff 11 70 0
noteoff 11 62 0
sleep 20.967
noteon 10 74 102
noteon 10 65 102
sleep 1.612
noteoff 0 82 0
sleep 1.612
noteoff 1 70 0
noteon 11 62 102
noteon 11 70 102
sleep 16.129
noteoff 3 58 0
sleep 53.225
noteoff 10 65 0
noteoff 10 74 0
sleep 3.225
noteoff 11 70 0
noteoff 11 62 0
sleep 20.967
echo "67680 tempo_s=278 tempo_l=0.25"
noteon 10 65 102
noteon 10 74 102
sleep 3.597
noteon 4 62 100
noteon 11 62 102
noteon 11 69 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 5.395
noteon 5 50 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 33 106
sleep 55.755
noteoff 10 74 0
noteoff 10 65 0
sleep 3.597
noteoff 11 69 0
noteoff 11 62 0
sleep 23.381
noteon 10 74 102
noteon 10 65 102
sleep 3.597
noteon 11 62 102
noteon 11 69 102
sleep 77.338
noteoff 10 65 0
noteoff 10 74 0
sleep 3.597
noteoff 11 69 0
noteoff 11 62 0
sleep 23.381
noteon 10 65 102
noteon 10 74 102
sleep 3.597
noteon 11 69 102
noteon 11 62 102
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 33 0
sleep 55.755
noteoff 10 74 0
noteoff 10 65 0
sleep 3.597
noteoff 11 62 0
noteoff 11 69 0
sleep 23.381
noteon 10 74 102
noteon 10 65 102
sleep 3.597
noteon 11 62 102
noteon 11 69 102
sleep 7.194
noteon 12 62 102
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 50 100
sleep 3.597
noteon 14 38 106
sleep 55.755
noteoff 10 65 0
noteoff 10 74 0
sleep 3.597
noteoff 11 69 0
noteoff 11 62 0
sleep 16.187
noteoff 12 62 0
sleep 7.194
echo "67920 tempo_s=310 tempo_l=0.25"
noteon 10 65 102
noteon 10 74 102
sleep 1.612
noteoff 13 50 0
sleep 1.612
noteoff 3 50 0
noteon 11 62 102
noteon 11 69 102
sleep 3.225
noteoff 14 38 0
sleep 3.225
noteon 12 64 102
sleep 8.062
noteon 13 52 104
sleep 1.612
noteon 3 52 100
sleep 3.224
noteon 14 40 106
sleep 19.344
noteoff 12 64 0
noteon 12 62 102
sleep 24.185
noteoff 13 52 0
noteon 13 50 104
sleep 1.612
noteoff 3 52 0
noteon 3 50 100
sleep 4.838
noteoff 10 74 0
noteoff 10 65 0
sleep 1.612
noteoff 12 62 0
noteon 12 64 102
sleep 1.612
noteoff 11 69 0
noteoff 11 62 0
sleep 11.288
noteoff 14 40 0
noteon 14 38 106
sleep 9.674
noteon 10 65 102
noteon 10 74 102
sleep 3.225
noteon 11 69 102
noteon 11 62 102
sleep 6.449
noteoff 12 64 0
noteon 12 62 102
sleep 8.064
noteoff 13 50 0
noteon 13 52 104
sleep 1.612
noteoff 3 50 0
noteon 3 52 100
sleep 22.577
noteoff 12 62 0
noteon 12 64 102
sleep 12.900
noteoff 14 38 0
noteon 14 40 106
sleep 11.287
noteoff 13 52 0
noteon 13 50 104
sleep 1.612
noteoff 3 52 0
noteon 3 50 100
sleep 4.836
noteoff 10 74 0
noteoff 10 65 0
sleep 1.612
noteoff 12 64 0
noteon 12 62 102
sleep 1.612
noteoff 11 62 0
noteoff 11 69 0
sleep 20.964
noteon 10 65 102
noteon 10 74 102
sleep 3.225
noteon 11 62 102
noteon 11 69 102
sleep 6.451
noteoff 12 62 0
noteon 12 64 102
sleep 8.061
noteoff 13 50 0
noteon 13 52 104
sleep 1.612
noteoff 3 50 0
noteon 3 52 100
sleep 3.225
noteoff 14 40 0
noteon 14 38 106
sleep 19.350
noteoff 12 64 0
noteon 12 62 102
sleep 24.189
noteoff 13 52 0
noteon 13 50 104
sleep 1.612
noteoff 3 52 0
noteon 3 50 100
sleep 4.838
noteoff 10 74 0
noteoff 10 65 0
sleep 1.612
noteoff 12 62 0
noteon 12 64 102
sleep 1.612
noteoff 11 69 0
noteoff 11 62 0
sleep 11.288
noteoff 14 38 0
noteon 14 40 106
sleep 9.674
noteon 10 74 102
noteon 10 65 102
sleep 3.225
noteon 11 62 102
noteon 11 69 102
sleep 3.224
noteoff 14 40 0
sleep 3.225
noteoff 12 64 0
noteon 12 61 102
sleep 8.064
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 3 50 0
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 35.483
noteoff 12 61 0
noteon 12 62 102
sleep 8.064
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 1.612
noteoff 10 65 0
noteoff 10 74 0
sleep 3.225
noteoff 11 69 0
noteoff 11 62 0
sleep 20.967
echo "68160 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
noteon 10 65 102
sleep 1.798
noteon 0 81 101
sleep 1.798
noteoff 4 62 0
noteon 11 62 102
noteon 11 69 102
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 5.395
noteoff 5 50 0
noteoff 12 62 0
noteon 12 65 102
sleep 8.992
noteoff 13 50 0
noteon 13 53 104
sleep 1.798
noteoff 3 50 0
noteon 3 57 100
noteon 3 53 100
sleep 3.597
noteoff 14 38 0
noteon 14 41 106
sleep 55.755
noteoff 10 65 0
noteoff 10 74 0
sleep 3.597
noteoff 11 69 0
noteoff 11 62 0
sleep 23.381
noteon 10 74 102
noteon 10 65 102
sleep 1.798
noteoff 0 81 0
sleep 1.798
noteon 11 62 102
noteon 11 69 102
sleep 7.194
noteoff 12 65 0
sleep 8.992
noteoff 13 53 0
sleep 1.798
noteoff 3 53 0
noteoff 3 57 0
sleep 3.597
noteoff 14 41 0
sleep 55.755
noteoff 10 65 0
noteoff 10 74 0
sleep 3.597
noteoff 11 69 0
noteoff 11 62 0
sleep 23.381
noteon 10 65 102
noteon 10 74 102
sleep 1.798
noteon 0 86 101
sleep 1.798
noteon 11 69 102
noteon 11 62 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 62 100
noteon 3 45 100
sleep 3.597
noteon 14 33 106
sleep 55.755
noteoff 10 74 0
noteoff 10 65 0
sleep 3.597
noteoff 11 62 0
noteoff 11 69 0
sleep 23.381
noteon 10 74 102
noteon 10 65 102
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteon 11 62 102
noteon 11 69 102
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
noteoff 3 62 0
sleep 3.597
noteoff 14 33 0
sleep 55.755
noteoff 10 65 0
noteoff 10 74 0
sleep 3.597
noteoff 11 69 0
noteoff 11 62 0
sleep 23.381
echo "68400 tempo_s=310 tempo_l=0.25"
noteon 10 65 102
noteon 10 74 102
sleep 1.612
noteon 0 89 101
sleep 1.612
noteon 11 62 102
noteon 11 69 102
sleep 16.129
noteon 3 65 100
sleep 53.225
noteoff 10 74 0
noteoff 10 65 0
sleep 3.225
noteoff 11 69 0
noteoff 11 62 0
sleep 20.967
noteon 10 65 102
noteon 10 74 102
sleep 1.612
noteoff 0 89 0
sleep 1.612
noteon 11 69 102
noteon 11 62 102
sleep 16.129
noteoff 3 65 0
sleep 53.225
noteoff 10 74 0
noteoff 10 65 0
sleep 3.225
noteoff 11 62 0
noteoff 11 69 0
sleep 20.967
noteon 10 65 102
noteon 10 74 102
sleep 1.612
noteon 0 81 101
sleep 1.612
noteon 11 62 102
noteon 11 69 102
sleep 16.129
noteon 3 57 100
sleep 53.225
noteoff 10 74 0
noteoff 10 65 0
sleep 3.225
noteoff 11 69 0
noteoff 11 62 0
sleep 20.967
noteon 10 74 102
noteon 10 65 102
sleep 1.612
noteoff 0 81 0
sleep 1.612
noteon 11 62 102
noteon 11 69 102
sleep 16.129
noteoff 3 57 0
sleep 53.225
noteoff 10 65 0
noteoff 10 74 0
sleep 3.225
noteoff 11 69 0
noteoff 11 62 0
sleep 20.967
echo "68640 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 3.597
noteon 11 62 102
noteon 11 72 102
sleep 7.194
noteon 12 56 102
sleep 8.992
noteon 13 44 104
sleep 1.798
noteon 3 44 100
sleep 3.596
noteon 14 32 106
sleep 55.738
noteoff 10 74 0
sleep 3.596
noteoff 11 72 0
noteoff 11 62 0
sleep 23.374
noteon 10 74 102
sleep 3.596
noteon 11 62 102
noteon 11 72 102
sleep 77.332
noteoff 10 74 0
sleep 3.597
noteoff 11 72 0
noteoff 11 62 0
sleep 23.381
noteon 10 74 102
sleep 3.597
noteon 1 74 100
noteon 11 72 102
noteon 11 62 102
sleep 7.194
noteoff 12 56 0
sleep 8.992
noteoff 13 44 0
sleep 1.798
noteoff 3 44 0
noteon 3 62 100
sleep 3.597
noteoff 14 32 0
sleep 55.755
noteoff 10 74 0
sleep 3.597
noteoff 11 62 0
noteoff 11 72 0
sleep 23.381
noteon 10 74 102
sleep 3.597
noteoff 1 74 0
noteon 11 62 102
noteon 11 72 102
sleep 17.985
noteoff 3 62 0
sleep 59.352
noteoff 10 74 0
sleep 3.597
noteoff 11 72 0
noteoff 11 62 0
sleep 23.381
echo "68880 tempo_s=300 tempo_l=0.25"
noteon 10 74 102
sleep 3.333
noteon 1 77 100
noteon 11 62 102
noteon 11 72 102
sleep 16.666
noteon 3 65 100
sleep 55.0
noteoff 10 74 0
sleep 3.333
noteoff 11 72 0
noteoff 11 62 0
sleep 21.666
noteon 10 74 102
sleep 3.333
noteoff 1 77 0
noteon 11 72 102
noteon 11 62 102
sleep 16.666
noteoff 3 65 0
sleep 55.0
noteoff 10 74 0
sleep 3.333
noteoff 11 62 0
noteoff 11 72 0
sleep 21.666
noteon 10 74 102
sleep 1.666
noteon 0 86 101
sleep 1.666
noteon 1 68 100
noteon 11 62 102
noteon 11 72 102
sleep 16.666
noteon 3 56 100
sleep 55.0
noteoff 10 74 0
sleep 3.333
noteoff 11 72 0
noteoff 11 62 0
sleep 21.666
noteon 10 74 102
sleep 1.666
noteoff 0 86 0
sleep 1.666
noteoff 1 68 0
noteon 11 62 102
noteon 11 72 102
sleep 16.666
noteoff 3 56 0
sleep 55.0
noteoff 10 74 0
sleep 3.333
noteoff 11 72 0
noteoff 11 62 0
sleep 21.666
echo "69120 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteon 0 89 101
sleep 1.798
noteon 11 71 102
noteon 11 62 102
sleep 7.194
noteon 12 55 102
sleep 8.992
noteon 13 43 104
sleep 1.798
noteon 3 43 100
sleep 3.597
noteon 14 31 106
sleep 55.755
noteoff 10 74 0
sleep 3.597
noteoff 11 62 0
noteoff 11 71 0
sleep 23.381
noteon 10 74 102
sleep 1.798
noteoff 0 89 0
sleep 1.798
noteon 11 71 102
noteon 11 62 102
sleep 77.338
noteoff 10 74 0
sleep 3.597
noteoff 11 62 0
noteoff 11 71 0
sleep 23.381
noteon 10 74 102
sleep 1.798
noteon 0 79 101
sleep 1.798
noteon 1 74 100
noteon 11 62 102
noteon 11 71 102
sleep 7.194
noteoff 12 55 0
sleep 8.992
noteoff 13 43 0
sleep 1.798
noteoff 3 43 0
noteon 3 62 100
sleep 3.597
noteoff 14 31 0
sleep 55.755
noteoff 10 74 0
sleep 3.597
noteoff 11 71 0
noteoff 11 62 0
sleep 23.381
noteon 10 74 102
sleep 1.798
noteoff 0 79 0
sleep 1.798
noteoff 1 74 0
noteon 11 71 102
noteon 11 62 102
sleep 17.985
noteoff 3 62 0
sleep 59.352
noteoff 10 74 0
sleep 3.597
noteoff 11 62 0
noteoff 11 71 0
sleep 23.381
echo "69360 tempo_s=300 tempo_l=0.25"
noteon 10 74 102
sleep 3.333
noteon 1 77 100
noteon 11 71 102
noteon 11 62 102
sleep 16.666
noteon 3 65 100
sleep 55.0
noteoff 10 74 0
sleep 3.333
noteoff 11 62 0
noteoff 11 71 0
sleep 21.666
noteon 10 74 102
sleep 3.333
noteoff 1 77 0
noteon 11 62 102
noteon 11 71 102
sleep 16.666
noteoff 3 65 0
sleep 55.0
noteoff 10 74 0
sleep 3.333
noteoff 11 71 0
noteoff 11 62 0
sleep 21.666
noteon 10 74 102
sleep 1.666
noteon 0 79 101
sleep 1.666
noteon 1 67 100
noteon 11 71 102
noteon 11 62 102
sleep 16.666
noteon 3 55 100
sleep 55.0
noteoff 10 74 0
sleep 3.333
noteoff 11 62 0
noteoff 11 71 0
sleep 21.666
noteon 10 74 102
sleep 1.666
noteoff 0 79 0
sleep 1.666
noteoff 1 67 0
noteon 11 71 102
noteon 11 62 102
sleep 16.666
noteoff 3 55 0
sleep 55.0
noteoff 10 74 0
sleep 3.333
noteoff 11 62 0
noteoff 11 71 0
sleep 21.666
echo "69600 tempo_s=278 tempo_l=0.25"
noteon 10 67 102
noteon 10 76 102
sleep 1.798
noteon 0 84 101
sleep 1.798
noteon 4 64 100
noteon 11 64 102
noteon 11 72 102
sleep 1.798
noteon 6 76 108
sleep 5.394
noteon 5 64 100
noteon 12 60 102
sleep 8.990
noteon 13 48 104
sleep 1.798
noteon 3 48 100
sleep 3.596
noteon 14 36 106
sleep 55.739
noteoff 10 76 0
noteoff 10 67 0
sleep 3.597
noteoff 11 72 0
noteoff 11 64 0
sleep 23.381
noteon 10 76 102
noteon 10 67 102
sleep 1.798
noteoff 0 84 0
sleep 1.798
noteon 11 64 102
noteon 11 72 102
sleep 77.338
noteoff 10 67 0
noteoff 10 76 0
sleep 3.597
noteoff 11 72 0
noteoff 11 64 0
sleep 23.381
noteon 10 76 102
noteon 10 67 102
sleep 3.597
noteon 11 64 102
noteon 11 72 102
sleep 7.194
noteoff 12 60 0
sleep 8.992
noteoff 13 48 0
sleep 1.798
noteoff 3 48 0
sleep 3.597
noteoff 14 36 0
sleep 55.755
noteoff 10 67 0
noteoff 10 76 0
sleep 3.597
noteoff 11 72 0
noteoff 11 64 0
sleep 23.381
noteon 10 67 102
noteon 10 76 102
sleep 3.597
noteon 11 72 102
noteon 11 64 102
sleep 7.194
noteon 12 64 102
sleep 8.992
noteon 13 52 104
sleep 1.798
noteon 3 52 100
sleep 3.597
noteon 14 40 106
sleep 55.755
noteoff 10 76 0
noteoff 10 67 0
sleep 3.597
noteoff 11 64 0
noteoff 11 72 0
sleep 16.187
noteoff 12 64 0
sleep 7.194
echo "69840 tempo_s=310 tempo_l=0.25"
noteon 10 76 102
noteon 10 67 102
sleep 1.612
noteoff 13 52 0
sleep 1.612
noteoff 3 52 0
noteon 11 72 102
noteon 11 64 102
sleep 3.225
noteoff 14 40 0
sleep 3.225
noteon 12 65 102
sleep 8.062
noteon 13 53 104
sleep 1.612
noteon 3 53 100
sleep 3.224
noteon 14 41 106
sleep 19.344
noteoff 12 65 0
noteon 12 64 102
sleep 24.185
noteoff 13 53 0
noteon 13 52 104
sleep 1.612
noteoff 3 53 0
noteon 3 52 100
sleep 4.838
noteoff 10 67 0
noteoff 10 76 0
sleep 1.612
noteoff 12 64 0
noteon 12 65 102
sleep 1.612
noteoff 11 64 0
noteoff 11 72 0
sleep 11.288
noteoff 14 41 0
noteon 14 40 106
sleep 9.674
noteon 10 67 102
noteon 10 76 102
sleep 3.225
noteon 11 72 102
noteon 11 64 102
sleep 6.449
noteoff 12 65 0
noteon 12 64 102
sleep 8.064
noteoff 13 52 0
noteon 13 53 104
sleep 1.612
noteoff 3 52 0
noteon 3 53 100
sleep 22.577
noteoff 12 64 0
noteon 12 65 102
sleep 12.900
noteoff 14 40 0
noteon 14 41 106
sleep 11.287
noteoff 13 53 0
noteon 13 52 104
sleep 1.612
noteoff 3 53 0
noteon 3 52 100
sleep 4.836
noteoff 10 76 0
noteoff 10 67 0
sleep 1.612
noteoff 12 65 0
noteon 12 64 102
sleep 1.612
noteoff 11 64 0
noteoff 11 72 0
sleep 20.964
noteon 10 67 102
noteon 10 76 102
sleep 3.225
noteon 11 64 102
noteon 11 72 102
sleep 6.451
noteoff 12 64 0
noteon 12 65 102
sleep 8.061
noteoff 13 52 0
noteon 13 53 104
sleep 1.612
noteoff 3 52 0
noteon 3 53 100
sleep 3.225
noteoff 14 41 0
noteon 14 40 106
sleep 19.350
noteoff 12 65 0
noteon 12 64 102
sleep 24.189
noteoff 13 53 0
noteon 13 52 104
sleep 1.612
noteoff 3 53 0
noteon 3 52 100
sleep 4.838
noteoff 10 76 0
noteoff 10 67 0
sleep 1.612
noteoff 12 64 0
noteon 12 65 102
sleep 1.612
noteoff 11 72 0
noteoff 11 64 0
sleep 11.288
noteoff 14 40 0
noteon 14 41 106
sleep 9.674
noteon 10 67 102
noteon 10 76 102
sleep 3.225
noteon 11 72 102
noteon 11 64 102
sleep 3.224
noteoff 14 41 0
sleep 3.225
noteoff 12 65 0
noteon 12 62 102
sleep 8.064
noteoff 13 52 0
noteon 13 50 104
sleep 1.612
noteoff 3 52 0
noteon 3 50 100
sleep 3.225
noteon 14 38 106
sleep 35.483
noteoff 12 62 0
noteon 12 64 102
sleep 8.064
noteoff 13 50 0
noteon 13 52 104
sleep 1.612
noteoff 3 50 0
noteon 3 52 100
sleep 3.225
noteoff 14 38 0
noteon 14 40 106
sleep 1.612
noteoff 10 76 0
noteoff 10 67 0
sleep 3.225
noteoff 11 64 0
noteoff 11 72 0
sleep 20.967
echo "70080 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
noteon 10 67 102
sleep 1.798
noteon 0 84 101
sleep 1.798
noteoff 4 64 0
noteon 1 72 100
noteon 11 64 102
noteon 11 72 102
sleep 1.798
noteoff 6 76 0
sleep 5.395
noteoff 5 64 0
noteoff 12 64 0
noteon 12 67 102
sleep 8.992
noteoff 13 52 0
noteon 13 55 104
sleep 1.798
noteoff 3 52 0
noteon 3 60 100
noteon 3 55 100
sleep 3.597
noteoff 14 40 0
noteon 14 43 106
sleep 55.755
noteoff 10 67 0
noteoff 10 76 0
sleep 3.597
noteoff 11 72 0
noteoff 11 64 0
sleep 23.381
noteon 10 76 102
noteon 10 67 102
sleep 1.798
noteoff 0 84 0
sleep 1.798
noteoff 1 72 0
noteon 11 64 102
noteon 11 72 102
sleep 7.194
noteoff 12 67 0
sleep 8.992
noteoff 13 55 0
sleep 1.798
noteoff 3 55 0
noteoff 3 60 0
sleep 3.597
noteoff 14 43 0
sleep 55.755
noteoff 10 67 0
noteoff 10 76 0
sleep 3.597
noteoff 11 72 0
noteoff 11 64 0
sleep 23.381
noteon 10 76 102
noteon 10 67 102
sleep 1.798
noteon 0 88 101
sleep 1.798
noteon 1 76 100
noteon 11 72 102
noteon 11 64 102
sleep 7.194
noteon 12 60 102
sleep 8.992
noteon 13 48 104
sleep 1.798
noteon 3 64 100
noteon 3 48 100
sleep 3.597
noteon 14 36 106
sleep 55.755
noteoff 10 67 0
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
noteoff 11 72 0
sleep 23.381
noteon 10 76 102
noteon 10 67 102
sleep 1.798
noteoff 0 88 0
sleep 1.798
noteoff 1 76 0
noteon 11 72 102
noteon 11 64 102
sleep 7.194
noteoff 12 60 0
sleep 8.992
noteoff 13 48 0
sleep 1.798
noteoff 3 48 0
noteoff 3 64 0
sleep 3.597
noteoff 14 36 0
sleep 55.755
noteoff 10 67 0
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
noteoff 11 72 0
sleep 23.381
echo "70320 tempo_s=310 tempo_l=0.25"
noteon 10 67 102
noteon 10 76 102
sleep 1.612
noteon 0 91 101
sleep 1.612
noteon 1 79 100
noteon 11 72 102
noteon 11 64 102
sleep 16.129
noteon 3 67 100
sleep 53.225
noteoff 10 76 0
noteoff 10 67 0
sleep 3.225
noteoff 11 64 0
noteoff 11 72 0
sleep 20.967
noteon 10 76 102
noteon 10 67 102
sleep 1.612
noteoff 0 91 0
sleep 1.612
noteoff 1 79 0
noteon 11 64 102
noteon 11 72 102
sleep 16.129
noteoff 3 67 0
sleep 53.225
noteoff 10 67 0
noteoff 10 76 0
sleep 3.225
noteoff 11 72 0
noteoff 11 64 0
sleep 20.967
noteon 10 67 102
noteon 10 76 102
sleep 1.612
noteon 0 84 101
sleep 1.612
noteon 1 72 100
noteon 11 72 102
noteon 11 64 102
sleep 16.129
noteon 3 60 100
sleep 53.225
noteoff 10 76 0
noteoff 10 67 0
sleep 3.225
noteoff 11 64 0
noteoff 11 72 0
sleep 20.967
noteon 10 67 102
noteon 10 76 102
sleep 1.612
noteoff 0 84 0
sleep 1.612
noteoff 1 72 0
noteon 11 72 102
noteon 11 64 102
sleep 16.129
noteoff 3 60 0
sleep 53.225
noteoff 10 76 0
noteoff 10 67 0
sleep 3.225
noteoff 11 64 0
noteoff 11 72 0
sleep 20.967
echo "70560 tempo_s=278 tempo_l=0.25"
noteon 10 67 102
noteon 10 76 102
sleep 3.597
noteon 4 64 100
noteon 11 64 102
noteon 11 71 102
sleep 1.798
noteon 6 76 108
sleep 5.395
noteon 5 64 100
noteon 12 59 102
sleep 8.992
noteon 13 47 104
sleep 1.798
noteon 3 47 100
sleep 3.597
noteon 14 35 106
sleep 55.755
noteoff 10 76 0
noteoff 10 67 0
sleep 3.597
noteoff 11 71 0
noteoff 11 64 0
sleep 23.381
noteon 10 76 102
noteon 10 67 102
sleep 3.597
noteon 11 64 102
noteon 11 71 102
sleep 77.338
noteoff 10 67 0
noteoff 10 76 0
sleep 3.597
noteoff 11 71 0
noteoff 11 64 0
sleep 23.381
noteon 10 76 102
noteon 10 67 102
sleep 3.597
noteon 11 71 102
noteon 11 64 102
sleep 7.194
noteoff 12 59 0
sleep 8.992
noteoff 13 47 0
sleep 1.798
noteoff 3 47 0
sleep 3.597
noteoff 14 35 0
sleep 55.755
noteoff 10 67 0
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
noteoff 11 71 0
sleep 23.381
noteon 10 76 102
noteon 10 67 102
sleep 3.597
noteon 11 64 102
noteon 11 71 102
sleep 7.194
noteon 12 64 102
sleep 8.992
noteon 13 52 104
sleep 1.798
noteon 3 52 100
sleep 3.597
noteon 14 40 106
sleep 55.755
noteoff 10 67 0
noteoff 10 76 0
sleep 3.597
noteoff 11 71 0
noteoff 11 64 0
sleep 16.187
noteoff 12 64 0
sleep 7.194
echo "70800 tempo_s=310 tempo_l=0.25"
noteon 10 67 102
noteon 10 76 102
sleep 1.612
noteoff 13 52 0
sleep 1.612
noteoff 3 52 0
noteon 11 64 102
noteon 11 71 102
sleep 3.225
noteoff 14 40 0
sleep 3.225
noteon 12 66 102
sleep 8.062
noteon 13 54 104
sleep 1.612
noteon 3 54 100
sleep 3.224
noteon 14 42 106
sleep 19.344
noteoff 12 66 0
noteon 12 64 102
sleep 24.185
noteoff 13 54 0
noteon 13 52 104
sleep 1.612
noteoff 3 54 0
noteon 3 52 100
sleep 4.838
noteoff 10 76 0
noteoff 10 67 0
sleep 1.612
noteoff 12 64 0
noteon 12 66 102
sleep 1.612
noteoff 11 71 0
noteoff 11 64 0
sleep 11.288
noteoff 14 42 0
noteon 14 40 106
sleep 9.674
noteon 10 76 102
noteon 10 67 102
sleep 3.225
noteon 11 71 102
noteon 11 64 102
sleep 6.449
noteoff 12 66 0
noteon 12 64 102
sleep 8.064
noteoff 13 52 0
noteon 13 54 104
sleep 1.612
noteoff 3 52 0
noteon 3 54 100
sleep 22.577
noteoff 12 64 0
noteon 12 66 102
sleep 12.900
noteoff 14 40 0
noteon 14 42 106
sleep 11.287
noteoff 13 54 0
noteon 13 52 104
sleep 1.612
noteoff 3 54 0
noteon 3 52 100
sleep 4.836
noteoff 10 67 0
noteoff 10 76 0
sleep 1.612
noteoff 12 66 0
noteon 12 64 102
sleep 1.612
noteoff 11 64 0
noteoff 11 71 0
sleep 20.964
noteon 10 67 102
noteon 10 76 102
sleep 3.225
noteon 11 64 102
noteon 11 71 102
sleep 6.451
noteoff 12 64 0
noteon 12 66 102
sleep 8.061
noteoff 13 52 0
noteon 13 54 104
sleep 1.612
noteoff 3 52 0
noteon 3 54 100
sleep 3.225
noteoff 14 42 0
noteon 14 40 106
sleep 19.350
noteoff 12 66 0
noteon 12 64 102
sleep 24.189
noteoff 13 54 0
noteon 13 52 104
sleep 1.612
noteoff 3 54 0
noteon 3 52 100
sleep 4.838
noteoff 10 76 0
noteoff 10 67 0
sleep 1.612
noteoff 12 64 0
noteon 12 66 102
sleep 1.612
noteoff 11 71 0
noteoff 11 64 0
sleep 11.288
noteoff 14 40 0
noteon 14 42 106
sleep 9.674
noteon 10 67 102
noteon 10 76 102
sleep 3.225
noteon 11 64 102
noteon 11 71 102
sleep 3.224
noteoff 14 42 0
sleep 3.225
noteoff 12 66 0
noteon 12 63 102
sleep 8.064
noteoff 13 52 0
noteon 13 51 104
sleep 1.612
noteoff 3 52 0
noteon 3 51 100
sleep 3.225
noteon 14 39 106
sleep 35.483
noteoff 12 63 0
noteon 12 64 102
sleep 8.064
noteoff 13 51 0
noteon 13 52 104
sleep 1.612
noteoff 3 51 0
noteon 3 52 100
sleep 3.225
noteoff 14 39 0
noteon 14 40 106
sleep 1.612
noteoff 10 76 0
noteoff 10 67 0
sleep 3.225
noteoff 11 71 0
noteoff 11 64 0
sleep 20.967
echo "71040 tempo_s=278 tempo_l=0.25"
noteon 10 67 102
noteon 10 76 102
sleep 1.798
noteon 0 83 101
sleep 1.798
noteoff 4 64 0
noteon 1 71 100
noteon 11 71 102
noteon 11 64 102
sleep 1.798
noteoff 6 76 0
sleep 5.395
noteoff 5 64 0
noteoff 12 64 0
noteon 12 67 102
sleep 8.992
noteoff 13 52 0
noteon 13 55 104
sleep 1.798
noteoff 3 52 0
noteon 3 55 100
noteon 3 59 100
sleep 3.597
noteoff 14 40 0
noteon 14 43 106
sleep 55.755
noteoff 10 76 0
noteoff 10 67 0
sleep 3.597
noteoff 11 64 0
noteoff 11 71 0
sleep 23.381
noteon 10 76 102
noteon 10 67 102
sleep 1.798
noteoff 0 83 0
sleep 1.798
noteoff 1 71 0
noteon 11 71 102
noteon 11 64 102
sleep 7.194
noteoff 12 67 0
sleep 8.992
noteoff 13 55 0
sleep 1.798
noteoff 3 59 0
noteoff 3 55 0
sleep 3.597
noteoff 14 43 0
sleep 55.755
noteoff 10 67 0
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
noteoff 11 71 0
sleep 23.381
noteon 10 67 102
noteon 10 76 102
sleep 1.798
noteon 0 88 101
sleep 1.798
noteon 1 76 100
noteon 11 64 102
noteon 11 71 102
sleep 7.194
noteon 12 59 102
sleep 8.992
noteon 13 47 104
sleep 1.798
noteon 3 64 100
noteon 3 47 100
sleep 3.597
noteon 14 35 106
sleep 55.755
noteoff 10 76 0
noteoff 10 67 0
sleep 3.597
noteoff 11 71 0
noteoff 11 64 0
sleep 23.381
noteon 10 76 102
noteon 10 67 102
sleep 1.798
noteoff 0 88 0
sleep 1.798
noteoff 1 76 0
noteon 11 71 102
noteon 11 64 102
sleep 7.194
noteoff 12 59 0
sleep 8.992
noteoff 13 47 0
sleep 1.798
noteoff 3 47 0
noteoff 3 64 0
sleep 3.597
noteoff 14 35 0
sleep 55.755
noteoff 10 67 0
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
noteoff 11 71 0
sleep 23.381
echo "71280 tempo_s=310 tempo_l=0.25"
noteon 10 67 102
noteon 10 76 102
sleep 1.612
noteon 0 91 101
sleep 1.612
noteon 1 79 100
noteon 11 71 102
noteon 11 64 102
sleep 16.129
noteon 3 67 100
sleep 53.225
noteoff 10 76 0
noteoff 10 67 0
sleep 3.225
noteoff 11 64 0
noteoff 11 71 0
sleep 20.967
noteon 10 67 102
noteon 10 76 102
sleep 1.612
noteoff 0 91 0
sleep 1.612
noteoff 1 79 0
noteon 11 64 102
noteon 11 71 102
sleep 16.129
noteoff 3 67 0
sleep 53.225
noteoff 10 76 0
noteoff 10 67 0
sleep 3.225
noteoff 11 71 0
noteoff 11 64 0
sleep 20.967
noteon 10 76 102
noteon 10 67 102
sleep 1.612
noteon 0 83 101
sleep 1.612
noteon 1 71 100
noteon 11 71 102
noteon 11 64 102
sleep 16.129
noteon 3 59 100
sleep 53.225
noteoff 10 67 0
noteoff 10 76 0
sleep 3.225
noteoff 11 64 0
noteoff 11 71 0
sleep 20.967
noteon 10 76 102
noteon 10 67 102
sleep 1.612
noteoff 0 83 0
sleep 1.612
noteoff 1 71 0
noteon 11 71 102
noteon 11 64 102
sleep 16.129
noteoff 3 59 0
sleep 53.225
noteoff 10 67 0
noteoff 10 76 0
sleep 3.225
noteoff 11 64 0
noteoff 11 71 0
sleep 20.967
echo "71520 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 3.597
noteon 11 74 102
sleep 7.194
noteon 12 58 102
sleep 8.992
noteon 13 46 104
sleep 1.798
noteon 3 46 100
sleep 3.597
noteon 14 34 106
sleep 55.745
noteoff 10 76 0
sleep 3.596
noteoff 11 74 0
sleep 23.377
noteon 10 76 102
sleep 3.597
noteon 11 74 102
sleep 77.324
noteoff 10 76 0
sleep 3.596
noteoff 11 74 0
sleep 23.377
noteon 10 76 102
sleep 3.597
noteon 1 76 100
noteon 11 74 102
sleep 7.193
noteoff 12 58 0
sleep 8.991
noteoff 13 46 0
sleep 1.798
noteoff 3 46 0
noteon 3 64 100
sleep 3.597
noteoff 14 34 0
sleep 55.755
noteoff 10 76 0
sleep 3.597
noteoff 11 74 0
sleep 23.381
noteon 10 76 102
sleep 3.597
noteoff 1 76 0
noteon 11 74 102
sleep 17.985
noteoff 3 64 0
sleep 59.352
noteoff 10 76 0
sleep 3.597
noteoff 11 74 0
sleep 23.381
echo "71760 tempo_s=300 tempo_l=0.25"
noteon 10 76 102
sleep 3.333
noteon 1 79 100
noteon 11 74 102
sleep 16.666
noteon 3 67 100
sleep 55.0
noteoff 10 76 0
sleep 3.333
noteoff 11 74 0
sleep 21.666
noteon 10 76 102
sleep 3.333
noteoff 1 79 0
noteon 11 74 102
sleep 16.666
noteoff 3 67 0
sleep 55.0
noteoff 10 76 0
sleep 3.333
noteoff 11 74 0
sleep 21.666
noteon 10 76 102
sleep 1.666
noteon 0 88 101
sleep 1.666
noteon 1 70 100
noteon 11 74 102
sleep 16.666
noteon 3 58 100
sleep 55.0
noteoff 10 76 0
sleep 3.333
noteoff 11 74 0
sleep 21.666
noteon 10 76 102
sleep 1.666
noteoff 0 88 0
sleep 1.666
noteoff 1 70 0
noteon 11 74 102
sleep 16.666
noteoff 3 58 0
sleep 55.0
noteoff 10 76 0
sleep 3.333
noteoff 11 74 0
sleep 21.666
echo "72000 tempo_s=272 tempo_l=0.25"
noteon 10 76 102
sleep 1.838
noteon 0 91 101
sleep 1.838
noteon 11 73 102
sleep 7.352
noteon 12 57 102
sleep 9.191
noteon 13 45 104
sleep 1.838
noteon 3 45 100
sleep 3.676
noteon 14 33 106
sleep 56.985
noteoff 10 76 0
sleep 3.676
noteoff 11 73 0
sleep 23.897
noteon 10 76 102
sleep 1.838
noteoff 0 91 0
sleep 1.838
noteon 11 73 102
sleep 79.044
noteoff 10 76 0
sleep 3.676
noteoff 11 73 0
sleep 23.897
noteon 10 76 102
sleep 1.838
noteon 0 81 101
sleep 1.838
noteon 1 76 100
noteon 11 73 102
sleep 7.352
noteoff 12 57 0
sleep 9.191
noteoff 13 45 0
sleep 1.838
noteoff 3 45 0
noteon 3 64 100
sleep 3.676
noteoff 14 33 0
sleep 56.985
noteoff 10 76 0
sleep 3.676
noteoff 11 73 0
sleep 23.897
noteon 10 76 102
sleep 1.838
noteoff 0 81 0
sleep 1.838
noteoff 1 76 0
noteon 11 73 102
sleep 18.382
noteoff 3 64 0
sleep 60.661
noteoff 10 76 0
sleep 3.676
noteoff 11 73 0
sleep 23.897
echo "72240 tempo_s=302 tempo_l=0.25"
noteon 10 76 102
sleep 3.311
noteon 1 79 100
noteon 11 73 102
sleep 16.556
noteon 3 67 100
sleep 54.635
noteoff 10 76 0
sleep 3.311
noteoff 11 73 0
sleep 21.523
noteon 10 76 102
sleep 3.311
noteoff 1 79 0
noteon 11 73 102
sleep 16.556
noteoff 3 67 0
sleep 54.635
noteoff 10 76 0
sleep 3.311
noteoff 11 73 0
sleep 21.523
noteon 10 76 102
sleep 1.655
noteon 0 81 101
sleep 1.655
noteon 1 69 100
noteon 11 73 102
sleep 16.556
noteon 3 57 100
sleep 54.635
noteoff 10 76 0
sleep 3.311
noteoff 11 73 0
sleep 21.523
noteon 10 76 102
sleep 1.655
noteoff 0 81 0
sleep 1.655
noteoff 1 69 0
noteon 11 73 102
sleep 16.556
noteoff 3 57 0
sleep 54.635
noteoff 10 76 0
sleep 3.311
noteoff 11 73 0
sleep 21.523
echo "72480 tempo_s=274 tempo_l=0.25"
noteon 10 77 102
sleep 1.824
noteon 0 86 101
sleep 1.824
noteon 11 74 102
sleep 7.299
noteon 12 62 102
sleep 9.124
noteon 13 50 104
sleep 1.824
noteon 3 50 100
sleep 3.649
noteon 14 38 106
sleep 56.569
noteoff 10 77 0
sleep 3.649
noteoff 11 74 0
sleep 7.299
noteoff 12 62 0
sleep 16.423
noteon 10 77 102
sleep 1.824
noteoff 0 86 0
sleep 1.824
noteon 11 74 102
sleep 7.299
noteon 12 69 102
sleep 71.163
noteoff 10 77 0
sleep 3.649
noteoff 11 74 0
sleep 7.299
noteoff 12 69 0
sleep 16.423
noteon 10 77 102
sleep 1.824
noteon 0 74 101
sleep 1.824
noteon 1 74 100
noteon 11 74 102
sleep 7.299
noteon 12 69 102
sleep 9.124
noteoff 13 50 0
sleep 1.824
noteoff 3 50 0
noteon 3 62 100
sleep 3.649
noteoff 14 38 0
sleep 56.567
noteoff 10 77 0
sleep 3.648
noteoff 11 74 0
sleep 7.299
noteoff 12 69 0
sleep 16.423
noteon 10 77 102
sleep 1.824
noteoff 0 74 0
sleep 1.824
noteoff 1 74 0
noteon 11 74 102
sleep 7.299
noteon 12 69 102
sleep 10.948
noteoff 3 62 0
sleep 60.217
noteoff 10 77 0
sleep 3.649
noteoff 11 74 0
sleep 7.298
noteoff 12 69 0
sleep 16.423
echo "72720 tempo_s=304 tempo_l=0.25"
noteon 10 77 102
sleep 3.289
noteon 1 77 100
noteon 11 74 102
sleep 6.578
noteon 12 69 102
sleep 9.866
noteon 3 65 100
sleep 54.275
noteoff 10 77 0
sleep 3.289
noteoff 11 74 0
sleep 6.577
noteoff 12 69 0
sleep 14.801
noteon 10 77 102
sleep 3.289
noteoff 1 77 0
noteon 11 74 102
sleep 6.578
noteon 12 69 102
sleep 9.868
noteoff 3 65 0
sleep 54.274
noteoff 10 77 0
sleep 3.289
noteoff 11 74 0
sleep 6.578
noteoff 12 69 0
sleep 14.802
noteon 10 77 102
sleep 1.644
noteon 0 86 101
sleep 1.644
noteon 1 69 100
noteon 11 74 102
sleep 6.578
noteon 12 69 102
sleep 9.867
noteon 3 57 100
sleep 54.275
noteoff 10 77 0
sleep 3.289
noteoff 11 74 0
sleep 6.578
noteoff 12 69 0
sleep 14.801
noteon 10 77 102
sleep 1.644
noteoff 0 86 0
sleep 1.644
noteoff 1 69 0
noteon 11 74 102
sleep 6.578
noteon 12 69 102
sleep 9.868
noteoff 3 57 0
sleep 54.274
noteoff 10 77 0
sleep 3.289
noteoff 11 74 0
sleep 6.578
noteoff 12 69 0
sleep 14.802
echo "72960 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 1.798
noteon 0 87 101
sleep 1.798
noteon 11 75 102
sleep 7.194
noteon 12 69 102
sleep 8.992
noteon 13 47 104
sleep 1.798
noteon 3 47 100
sleep 3.597
noteon 14 35 106
sleep 55.751
noteoff 10 78 0
sleep 3.597
noteoff 11 75 0
sleep 7.194
noteoff 12 69 0
sleep 16.187
noteon 10 78 102
sleep 1.798
noteoff 0 87 0
sleep 1.798
noteon 11 75 102
sleep 7.194
noteon 12 69 102
sleep 70.141
noteoff 10 78 0
sleep 3.597
noteoff 11 75 0
sleep 7.193
noteoff 12 69 0
sleep 16.187
noteon 10 78 102
sleep 1.798
noteon 0 83 101
sleep 1.798
noteon 1 75 100
noteon 11 75 102
sleep 7.194
noteon 12 69 102
sleep 8.992
noteoff 13 47 0
sleep 1.798
noteoff 3 47 0
noteon 3 63 100
sleep 3.597
noteoff 14 35 0
sleep 55.755
noteoff 10 78 0
sleep 3.597
noteoff 11 75 0
sleep 7.193
noteoff 12 69 0
sleep 16.185
noteon 10 78 102
sleep 1.798
noteoff 0 83 0
sleep 1.798
noteoff 1 75 0
noteon 11 75 102
sleep 7.194
noteon 12 69 102
sleep 10.791
noteoff 3 63 0
sleep 59.352
noteoff 10 78 0
sleep 3.597
noteoff 11 75 0
sleep 7.194
noteoff 12 69 0
sleep 16.187
echo "73200 tempo_s=308 tempo_l=0.25"
noteon 10 78 102
sleep 3.246
noteon 1 78 100
noteon 11 75 102
sleep 6.493
noteon 12 69 102
sleep 9.739
noteon 3 66 100
sleep 53.568
noteoff 10 78 0
sleep 3.246
noteoff 11 75 0
sleep 6.493
noteoff 12 69 0
sleep 14.61
noteon 10 78 102
sleep 3.246
noteoff 1 78 0
noteon 11 75 102
sleep 6.493
noteon 12 69 102
sleep 9.74
noteoff 3 66 0
sleep 53.568
noteoff 10 78 0
sleep 3.246
noteoff 11 75 0
sleep 6.493
noteoff 12 69 0
sleep 14.610
noteon 10 78 102
sleep 1.623
noteon 0 87 101
sleep 1.623
noteon 1 71 100
noteon 11 75 102
sleep 6.493
noteon 12 69 102
sleep 9.74
noteon 3 59 100
sleep 53.568
noteoff 10 78 0
sleep 3.246
noteoff 11 75 0
sleep 6.493
noteoff 12 69 0
sleep 14.61
noteon 10 78 102
sleep 1.623
noteoff 0 87 0
sleep 1.623
noteoff 1 71 0
noteon 11 75 102
sleep 6.493
noteon 12 69 102
sleep 9.739
noteoff 3 59 0
sleep 53.569
noteoff 10 78 0
sleep 3.246
noteoff 11 75 0
sleep 6.493
noteoff 12 69 0
sleep 14.61
echo "73440 tempo_s=278 tempo_l=0.25"
noteon 10 79 102
sleep 1.798
noteon 0 88 101
sleep 1.798
noteon 11 76 102
sleep 7.194
noteon 12 67 102
sleep 8.992
noteon 13 52 104
sleep 1.798
noteon 3 52 100
sleep 3.597
noteon 14 40 106
sleep 55.753
noteoff 10 79 0
sleep 3.597
noteoff 11 76 0
sleep 7.193
noteoff 12 67 0
sleep 16.185
noteon 10 79 102
sleep 1.798
noteoff 0 88 0
sleep 1.798
noteon 11 76 102
sleep 7.194
noteon 12 71 102
sleep 70.143
noteoff 10 79 0
sleep 3.597
noteoff 11 76 0
sleep 7.193
noteoff 12 71 0
sleep 16.187
noteon 10 79 102
sleep 1.798
noteon 0 76 101
sleep 1.798
noteon 1 76 100
noteon 11 76 102
sleep 7.194
noteon 12 71 102
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
noteon 3 64 100
sleep 3.597
noteoff 14 40 0
sleep 55.755
noteoff 10 79 0
sleep 3.597
noteoff 11 76 0
sleep 7.194
noteoff 12 71 0
sleep 16.187
noteon 10 79 102
sleep 1.798
noteoff 0 76 0
sleep 1.798
noteoff 1 76 0
noteon 11 76 102
sleep 7.194
noteon 12 71 102
sleep 10.791
noteoff 3 64 0
sleep 59.349
noteoff 10 79 0
sleep 3.597
noteoff 11 76 0
sleep 7.194
noteoff 12 71 0
sleep 16.187
echo "73680 tempo_s=310 tempo_l=0.25"
noteon 10 79 102
sleep 3.225
noteon 1 79 100
noteon 11 76 102
sleep 6.451
noteon 12 71 102
sleep 9.677
noteon 3 67 100
sleep 53.221
noteoff 10 79 0
sleep 3.224
noteoff 11 76 0
sleep 6.451
noteoff 12 71 0
sleep 14.516
noteon 10 79 102
sleep 3.225
noteoff 1 79 0
noteon 11 76 102
sleep 6.451
noteon 12 71 102
sleep 9.677
noteoff 3 67 0
sleep 53.223
noteoff 10 79 0
sleep 3.225
noteoff 11 76 0
sleep 6.450
noteoff 12 71 0
sleep 14.514
noteon 10 79 102
sleep 1.612
noteon 0 83 101
sleep 1.612
noteon 1 71 100
noteon 11 76 102
sleep 6.451
noteon 12 71 102
sleep 9.676
noteon 3 59 100
sleep 53.225
noteoff 10 79 0
sleep 3.225
noteoff 11 76 0
sleep 6.449
noteoff 12 71 0
sleep 14.514
noteon 10 79 102
sleep 1.612
noteoff 0 83 0
sleep 1.612
noteoff 1 71 0
noteon 11 76 102
sleep 6.451
noteon 12 71 102
sleep 9.677
noteoff 3 59 0
sleep 53.224
noteoff 10 79 0
sleep 3.225
noteoff 11 76 0
sleep 6.451
noteoff 12 71 0
sleep 14.516
echo "73920 tempo_s=278 tempo_l=0.25"
noteon 10 80 102
sleep 1.798
noteon 0 86 101
sleep 1.798
noteon 11 77 102
sleep 7.194
noteon 12 71 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteon 3 49 100
sleep 3.597
noteon 14 37 106
sleep 55.754
noteoff 10 80 0
sleep 3.597
noteoff 11 77 0
sleep 7.194
noteoff 12 71 0
sleep 16.186
noteon 10 80 102
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteon 11 77 102
sleep 7.194
noteon 12 71 102
sleep 70.141
noteoff 10 80 0
sleep 3.597
noteoff 11 77 0
sleep 7.194
noteoff 12 71 0
sleep 16.187
noteon 10 80 102
sleep 1.798
noteon 0 83 101
sleep 1.798
noteon 1 71 100
noteon 11 77 102
sleep 7.193
noteon 12 71 102
sleep 8.992
noteoff 13 49 0
sleep 1.798
noteoff 3 49 0
noteon 3 59 100
sleep 3.597
noteoff 14 37 0
sleep 55.752
noteoff 10 80 0
sleep 3.597
noteoff 11 77 0
sleep 7.194
noteoff 12 71 0
sleep 16.187
noteon 10 80 102
sleep 1.798
noteoff 0 83 0
sleep 1.798
noteoff 1 71 0
noteon 11 77 102
sleep 7.194
noteon 12 71 102
sleep 10.791
noteoff 3 59 0
sleep 59.349
noteoff 10 80 0
sleep 3.597
noteoff 11 77 0
sleep 7.193
noteoff 12 71 0
sleep 16.187
echo "74160 tempo_s=310 tempo_l=0.25"
noteon 10 80 102
sleep 3.225
noteon 1 73 100
noteon 11 77 102
sleep 6.451
noteon 12 71 102
sleep 9.677
noteon 3 61 100
sleep 53.225
noteoff 10 80 0
sleep 3.225
noteoff 11 77 0
sleep 6.448
noteoff 12 71 0
sleep 14.514
noteon 10 80 102
sleep 3.225
noteoff 1 73 0
noteon 11 77 102
sleep 6.451
noteon 12 71 102
sleep 9.677
noteoff 3 61 0
sleep 53.225
noteoff 10 80 0
sleep 3.225
noteoff 11 77 0
sleep 6.451
noteoff 12 71 0
sleep 14.516
noteon 10 80 102
sleep 1.612
noteon 0 83 101
sleep 1.612
noteon 1 71 100
noteon 11 77 102
sleep 6.451
noteon 12 71 102
sleep 9.676
noteon 3 59 100
sleep 53.224
noteoff 10 80 0
sleep 3.225
noteoff 11 77 0
sleep 6.451
noteoff 12 71 0
sleep 14.516
noteon 10 80 102
sleep 1.612
noteoff 0 83 0
sleep 1.612
noteoff 1 71 0
noteon 11 77 102
sleep 6.451
noteon 12 71 102
sleep 9.677
noteoff 3 59 0
sleep 53.221
noteoff 10 80 0
sleep 3.225
noteoff 11 77 0
sleep 6.451
noteoff 12 71 0
sleep 14.516
echo "74400 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
sleep 1.798
noteon 0 81 101
sleep 1.798
noteon 11 78 102
sleep 7.194
noteon 12 69 102
sleep 8.992
noteon 13 54 104
sleep 1.798
noteon 3 54 100
sleep 3.597
noteon 14 42 106
sleep 55.752
noteoff 10 81 0
sleep 26.977
noteon 10 81 102
sleep 1.798
noteoff 0 81 0
sleep 79.134
noteoff 10 81 0
sleep 3.597
noteoff 11 78 0
sleep 23.380
noteon 10 80 102
sleep 1.798
noteon 0 85 101
sleep 1.798
noteon 1 69 100
noteon 11 68 102
sleep 7.194
noteoff 12 69 0
sleep 8.992
noteoff 13 54 0
sleep 1.798
noteoff 3 54 0
noteon 3 57 100
sleep 3.597
noteoff 14 42 0
sleep 55.753
noteoff 10 80 0
sleep 3.597
noteoff 11 68 0
sleep 23.379
noteon 10 80 102
sleep 1.798
noteoff 0 85 0
sleep 1.798
noteoff 1 69 0
noteon 11 68 102
sleep 17.985
noteoff 3 57 0
sleep 59.351
noteoff 10 80 0
sleep 3.597
noteoff 11 68 0
sleep 23.381
echo "74640 tempo_s=310 tempo_l=0.25"
noteon 10 78 102
sleep 3.225
noteon 1 73 100
noteon 11 66 102
sleep 16.127
noteon 3 61 100
sleep 53.225
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 20.967
noteon 10 78 102
sleep 3.225
noteoff 1 73 0
noteon 11 66 102
sleep 16.128
noteoff 3 61 0
sleep 53.222
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 20.967
noteon 10 77 102
sleep 1.612
noteon 0 85 101
sleep 1.612
noteon 1 69 100
noteon 11 65 102
sleep 16.129
noteon 3 57 100
sleep 53.220
noteoff 10 77 0
sleep 3.225
noteoff 11 65 0
sleep 20.967
noteon 10 77 102
sleep 1.612
noteoff 0 85 0
sleep 1.612
noteoff 1 69 0
noteon 11 65 102
sleep 16.129
noteoff 3 57 0
sleep 53.223
noteoff 10 77 0
sleep 3.225
noteoff 11 65 0
sleep 20.965
echo "74880 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 1.798
noteon 0 90 101
sleep 1.798
noteon 11 66 102
sleep 7.194
noteon 12 62 102
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 50 100
sleep 3.597
noteon 14 38 106
sleep 55.755
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 23.379
noteon 10 78 102
sleep 1.798
noteoff 0 90 0
sleep 1.798
noteon 11 66 102
sleep 77.336
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 23.381
noteon 10 80 102
sleep 1.798
noteon 0 78 101
sleep 1.798
noteon 1 66 100
noteon 11 68 102
sleep 7.194
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
noteon 3 54 100
sleep 3.597
noteoff 14 38 0
sleep 55.753
noteoff 10 80 0
sleep 3.597
noteoff 11 68 0
sleep 23.381
noteon 10 80 102
sleep 1.798
noteoff 0 78 0
sleep 1.798
noteoff 1 66 0
noteon 11 68 102
sleep 17.985
noteoff 3 54 0
sleep 59.350
noteoff 10 80 0
sleep 3.597
noteoff 11 68 0
sleep 23.379
echo "75120 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 3.225
noteon 1 78 100
noteon 11 69 102
sleep 16.128
noteon 3 66 100
sleep 53.222
noteoff 10 81 0
sleep 3.225
noteoff 11 69 0
sleep 20.967
noteon 10 81 102
sleep 3.225
noteoff 1 78 0
noteon 11 69 102
sleep 16.129
noteoff 3 66 0
sleep 53.222
noteoff 10 81 0
sleep 3.225
noteoff 11 69 0
sleep 20.967
noteon 10 83 102
sleep 1.612
noteon 0 90 101
sleep 1.612
noteon 1 78 100
noteon 11 71 102
sleep 16.129
noteon 3 66 100
sleep 53.224
noteoff 10 83 0
sleep 3.225
noteoff 11 71 0
sleep 20.963
noteon 10 83 102
sleep 1.612
noteoff 0 90 0
sleep 1.612
noteoff 1 78 0
noteon 11 71 102
sleep 16.129
noteoff 3 66 0
sleep 53.225
noteoff 10 83 0
sleep 3.225
noteoff 11 71 0
sleep 20.967
echo "75360 tempo_s=278 tempo_l=0.25"
noteon 10 85 102
sleep 1.798
noteon 0 89 101
sleep 1.798
noteon 1 77 100
noteon 11 73 102
sleep 7.194
noteon 12 73 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteon 3 49 100
noteon 3 65 100
sleep 3.597
noteon 14 37 106
sleep 192.446
noteoff 0 89 0
noteoff 13 49 0
sleep 1.798
noteoff 1 77 0
sleep 3.597
noteoff 14 37 0
sleep 12.589
noteon 13 61 104
sleep 1.798
noteoff 3 65 0
noteoff 3 49 0
sleep 3.597
noteon 14 49 106
sleep 190.647
echo "75600 tempo_s=310 tempo_l=0.25"
sleep 193.548
noteoff 10 85 0
noteon 10 84 102
sleep 3.225
noteoff 11 73 0
noteon 11 72 102
sleep 6.451
noteoff 12 73 0
noteon 12 72 102
sleep 8.064
noteoff 13 61 0
noteon 13 60 104
sleep 4.838
noteoff 14 49 0
noteon 14 48 106
sleep 154.838
noteoff 10 84 0
sleep 3.225
noteoff 11 72 0
sleep 6.451
noteoff 12 72 0
sleep 6.451
echo "75840 tempo_s=278 tempo_l=0.25"
noteon 10 85 102
sleep 1.798
noteoff 13 60 0
sleep 1.798
noteon 11 73 102
sleep 3.597
noteoff 14 48 0
sleep 3.597
noteon 12 73 102
sleep 8.992
noteon 13 61 104
sleep 5.395
noteon 14 49 106
sleep 190.647
noteoff 10 85 0
noteon 10 86 102
sleep 3.597
noteoff 11 73 0
noteon 11 74 102
sleep 7.194
noteoff 12 73 0
noteon 12 74 102
sleep 8.992
noteoff 13 61 0
noteon 13 62 104
sleep 5.395
noteoff 14 49 0
noteon 14 50 106
sleep 190.647
echo "76080 tempo_s=310 tempo_l=0.25"
noteoff 10 86 0
noteon 10 85 102
sleep 3.225
noteoff 11 74 0
noteon 11 73 102
sleep 6.451
noteoff 12 74 0
noteon 12 73 102
sleep 8.064
noteoff 13 62 0
noteon 13 61 104
sleep 4.838
noteoff 14 50 0
noteon 14 49 106
sleep 170.967
noteoff 10 85 0
noteon 10 83 102
sleep 3.225
noteoff 11 73 0
noteon 11 71 102
sleep 6.451
noteoff 12 73 0
noteon 12 71 102
sleep 8.064
noteoff 13 61 0
noteon 13 59 104
sleep 4.838
noteoff 14 49 0
noteon 14 47 106
sleep 170.967
echo "76320 tempo_s=278 tempo_l=0.25"
noteoff 10 83 0
noteon 10 81 102
sleep 1.798
noteoff 13 59 0
sleep 1.798
noteoff 11 71 0
noteon 11 69 102
sleep 3.597
noteoff 14 47 0
sleep 3.597
noteoff 12 71 0
noteon 12 69 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 190.647
noteoff 10 81 0
noteon 10 80 102
sleep 3.597
noteoff 11 69 0
noteon 11 68 102
sleep 7.194
noteoff 12 69 0
noteon 12 68 102
sleep 8.992
noteoff 13 57 0
noteon 13 56 104
sleep 5.395
noteoff 14 45 0
noteon 14 44 106
sleep 190.647
echo "76560 tempo_s=310 tempo_l=0.25"
noteoff 10 80 0
noteon 10 78 102
sleep 3.225
noteoff 11 68 0
noteon 11 66 102
sleep 6.451
noteoff 12 68 0
noteon 12 66 102
sleep 8.064
noteoff 13 56 0
noteon 13 54 104
sleep 4.838
noteoff 14 44 0
noteon 14 42 106
sleep 170.967
noteoff 10 78 0
noteon 10 77 102
sleep 3.225
noteoff 11 66 0
noteon 11 65 102
sleep 6.451
noteoff 12 66 0
noteon 12 65 102
sleep 8.064
noteoff 13 54 0
noteon 13 53 104
sleep 4.838
noteoff 14 42 0
noteon 14 41 106
sleep 170.967
echo "76800 tempo_s=278 tempo_l=0.25"
noteoff 10 77 0
noteon 10 78 102
sleep 3.597
noteoff 11 65 0
noteon 11 66 102
sleep 7.194
noteoff 12 65 0
noteon 12 66 102
sleep 8.992
noteoff 13 53 0
noteon 13 54 104
sleep 5.395
noteoff 14 41 0
noteon 14 42 106
sleep 190.647
noteoff 10 78 0
noteon 10 74 102
sleep 3.597
noteoff 11 66 0
noteon 11 62 102
sleep 7.194
noteoff 12 66 0
noteon 12 62 102
sleep 8.992
noteoff 13 54 0
noteon 13 50 104
sleep 5.395
noteoff 14 42 0
noteon 14 38 106
sleep 190.647
echo "77040 tempo_s=310 tempo_l=0.25"
noteoff 10 74 0
noteon 10 73 102
sleep 3.225
noteoff 11 62 0
noteon 11 61 102
sleep 6.451
noteoff 12 62 0
noteon 12 61 102
sleep 8.064
noteoff 13 50 0
noteon 13 49 104
sleep 4.838
noteoff 14 38 0
noteon 14 37 106
sleep 170.967
noteoff 10 73 0
noteon 10 72 102
sleep 3.225
noteoff 11 61 0
noteon 11 60 102
sleep 6.451
noteoff 12 61 0
noteon 12 60 102
sleep 8.064
noteoff 13 49 0
noteon 13 48 104
sleep 4.838
noteoff 14 37 0
noteon 14 36 106
sleep 154.838
noteoff 10 72 0
sleep 3.225
noteoff 11 60 0
sleep 6.451
noteoff 12 60 0
sleep 6.451
echo "77280 tempo_s=278 tempo_l=0.25"
noteon 10 73 102
sleep 1.798
noteoff 13 48 0
noteon 0 85 101
sleep 1.798
noteon 1 73 100
noteon 11 61 102
sleep 3.597
noteoff 14 36 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteon 3 61 100
noteon 3 49 100
sleep 3.597
noteon 14 37 106
sleep 64.748
noteoff 10 73 0
sleep 3.597
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 73 102
sleep 3.597
noteon 11 73 102
sleep 7.194
noteon 12 61 102
sleep 79.136
noteoff 10 73 0
sleep 3.597
noteoff 11 73 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 73 102
sleep 3.597
noteon 11 73 102
sleep 7.194
noteon 12 61 102
sleep 79.136
noteoff 10 73 0
sleep 3.597
noteoff 11 73 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 73 102
sleep 3.597
noteon 11 73 102
sleep 7.194
noteon 12 61 102
sleep 79.136
noteoff 10 73 0
sleep 3.597
noteoff 11 73 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
echo "77520 tempo_s=310 tempo_l=0.25"
noteon 10 73 102
sleep 3.225
noteon 11 73 102
sleep 6.451
noteon 12 61 102
sleep 70.967
noteoff 10 73 0
sleep 3.225
noteoff 11 73 0
sleep 6.451
noteoff 12 61 0
sleep 6.451
noteon 10 73 102
sleep 3.225
noteon 11 73 102
sleep 6.451
noteon 12 61 102
sleep 70.967
noteoff 10 73 0
sleep 3.225
noteoff 11 73 0
sleep 6.451
noteoff 12 61 0
sleep 6.451
noteon 10 72 102
sleep 1.612
noteoff 0 85 0
noteon 0 84 101
sleep 1.612
noteoff 1 73 0
noteon 1 72 100
noteon 11 72 102
sleep 6.451
noteon 12 60 102
sleep 8.064
noteoff 13 49 0
noteon 13 48 104
sleep 1.612
noteoff 3 49 0
noteoff 3 61 0
noteon 3 48 100
noteon 3 60 100
sleep 3.225
noteoff 14 37 0
noteon 14 36 106
sleep 58.064
noteoff 10 72 0
sleep 3.225
noteoff 11 72 0
sleep 6.451
noteoff 12 60 0
sleep 6.451
noteon 10 72 102
sleep 3.225
noteon 11 72 102
sleep 6.451
noteon 12 60 102
sleep 70.967
noteoff 10 72 0
sleep 1.612
noteoff 0 84 0
sleep 1.612
noteoff 1 72 0
noteoff 11 72 0
sleep 6.451
noteoff 12 60 0
sleep 6.451
echo "77760 tempo_s=278 tempo_l=0.25"
noteon 10 73 102
sleep 1.798
noteoff 13 48 0
noteon 0 85 101
sleep 1.798
noteoff 3 60 0
noteoff 3 48 0
noteon 1 73 100
noteon 11 73 102
sleep 3.597
noteoff 14 36 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteon 3 49 100
noteon 3 61 100
sleep 3.597
noteon 14 37 106
sleep 64.748
noteoff 10 73 0
sleep 3.597
noteoff 11 73 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 73 102
sleep 3.597
noteon 11 73 102
sleep 7.194
noteon 12 61 102
sleep 79.136
noteoff 10 73 0
sleep 3.597
noteoff 11 73 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 74 102
sleep 1.798
noteoff 0 85 0
noteon 0 86 101
sleep 1.798
noteoff 1 73 0
noteon 1 74 100
noteon 11 74 102
sleep 7.194
noteon 12 62 102
sleep 8.992
noteoff 13 49 0
noteon 13 50 104
sleep 1.798
noteoff 3 61 0
noteoff 3 49 0
noteon 3 50 100
noteon 3 62 100
sleep 3.597
noteoff 14 37 0
noteon 14 38 106
sleep 64.748
noteoff 10 74 0
sleep 3.597
noteoff 11 74 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 74 102
sleep 3.597
noteon 11 74 102
sleep 7.194
noteon 12 62 102
sleep 79.136
noteoff 10 74 0
sleep 3.597
noteoff 11 74 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
echo "78000 tempo_s=310 tempo_l=0.25"
noteon 10 73 102
sleep 1.612
noteoff 0 86 0
noteon 0 85 101
sleep 1.612
noteoff 1 74 0
noteon 1 73 100
noteon 11 73 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 3 62 0
noteoff 3 50 0
noteon 3 61 100
noteon 3 49 100
sleep 3.225
noteoff 14 38 0
noteon 14 37 106
sleep 58.064
noteoff 10 73 0
sleep 3.225
noteoff 11 73 0
sleep 6.451
noteoff 12 61 0
sleep 6.451
noteon 10 73 102
sleep 3.225
noteon 11 73 102
sleep 6.451
noteon 12 61 102
sleep 70.967
noteoff 10 73 0
sleep 3.225
noteoff 11 73 0
sleep 6.451
noteoff 12 61 0
sleep 6.451
noteon 10 71 102
sleep 1.612
noteoff 0 85 0
noteon 0 83 101
sleep 1.612
noteoff 1 73 0
noteon 1 71 100
noteon 11 71 102
sleep 6.451
noteon 12 59 102
sleep 8.064
noteoff 13 49 0
noteon 13 47 104
sleep 1.612
noteoff 3 49 0
noteoff 3 61 0
noteon 3 59 100
noteon 3 47 100
sleep 3.225
noteoff 14 37 0
noteon 14 35 106
sleep 58.064
noteoff 10 71 0
sleep 3.225
noteoff 11 71 0
sleep 6.451
noteoff 12 59 0
sleep 6.451
noteon 10 71 102
sleep 3.225
noteon 11 71 102
sleep 6.451
noteon 12 59 102
sleep 70.967
noteoff 10 71 0
sleep 3.225
noteoff 11 71 0
sleep 6.451
noteoff 12 59 0
sleep 6.451
echo "78240 tempo_s=278 tempo_l=0.25"
noteon 10 69 102
sleep 1.798
noteoff 0 83 0
noteoff 13 47 0
noteon 0 81 101
sleep 1.798
noteoff 1 71 0
noteon 1 69 100
noteon 11 69 102
sleep 3.597
noteoff 14 35 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteoff 3 47 0
noteoff 3 59 0
noteon 3 57 100
noteon 3 45 100
sleep 3.597
noteon 14 33 106
sleep 64.748
noteoff 10 69 0
sleep 3.597
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
noteon 10 69 102
sleep 3.597
noteon 11 69 102
sleep 7.194
noteon 12 57 102
sleep 79.136
noteoff 10 69 0
sleep 3.597
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
noteon 10 68 102
sleep 1.798
noteoff 0 81 0
noteon 0 80 101
sleep 1.798
noteoff 1 69 0
noteon 1 80 100
noteon 11 68 102
sleep 7.194
noteon 12 56 102
sleep 8.992
noteoff 13 45 0
noteon 13 44 104
sleep 1.798
noteoff 3 45 0
noteoff 3 57 0
noteon 3 56 100
sleep 3.597
noteoff 14 33 0
noteon 14 32 106
sleep 64.748
noteoff 10 68 0
sleep 3.597
noteoff 11 68 0
sleep 7.194
noteoff 12 56 0
sleep 7.194
noteon 10 68 102
sleep 3.597
noteon 11 68 102
sleep 7.194
noteon 12 56 102
sleep 79.136
noteoff 10 68 0
sleep 3.597
noteoff 11 68 0
sleep 7.194
noteoff 12 56 0
sleep 7.194
echo "78480 tempo_s=310 tempo_l=0.25"
noteon 10 66 102
sleep 1.612
noteoff 0 80 0
noteon 0 78 101
sleep 1.612
noteoff 1 80 0
noteon 1 78 100
noteon 11 66 102
sleep 6.451
noteon 12 54 102
sleep 8.064
noteoff 13 44 0
noteon 13 42 104
sleep 1.612
noteoff 3 56 0
noteon 3 54 100
sleep 3.225
noteoff 14 32 0
noteon 14 30 106
sleep 58.064
noteoff 10 66 0
sleep 3.225
noteoff 11 66 0
sleep 6.451
noteoff 12 54 0
sleep 6.451
noteon 10 66 102
sleep 3.225
noteon 11 66 102
sleep 6.451
noteon 12 54 102
sleep 70.967
noteoff 10 66 0
sleep 3.225
noteoff 11 66 0
sleep 6.451
noteoff 12 54 0
sleep 6.451
noteon 10 65 102
sleep 1.612
noteoff 0 78 0
noteon 0 77 101
sleep 1.612
noteoff 1 78 0
noteon 1 77 100
noteon 11 65 102
sleep 6.451
noteon 12 53 102
sleep 8.064
noteoff 13 42 0
noteon 13 53 104
sleep 1.612
noteoff 3 54 0
noteon 3 53 100
sleep 3.225
noteoff 14 30 0
noteon 14 41 106
sleep 58.064
noteoff 10 65 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 53 0
sleep 6.451
noteon 10 65 102
sleep 3.225
noteon 11 65 102
sleep 6.451
noteon 12 53 102
sleep 70.967
noteoff 10 65 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 53 0
sleep 6.451
echo "78720 tempo_s=278 tempo_l=0.25"
noteon 10 66 102
sleep 1.798
noteoff 0 77 0
noteoff 13 53 0
noteon 0 78 101
sleep 1.798
noteoff 1 77 0
noteon 1 78 100
noteon 11 66 102
sleep 3.597
noteoff 14 41 0
sleep 3.597
noteon 12 54 102
sleep 8.992
noteon 13 54 104
sleep 1.798
noteoff 3 53 0
noteon 3 66 100
noteon 3 54 100
sleep 3.597
noteon 14 42 106
sleep 64.748
noteoff 10 66 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
noteon 10 66 102
sleep 3.597
noteon 11 66 102
sleep 7.194
noteon 12 54 102
sleep 79.136
noteoff 10 66 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
noteon 10 62 102
sleep 1.798
noteoff 0 78 0
noteon 0 86 101
sleep 1.798
noteoff 1 78 0
noteon 1 74 100
noteon 11 62 102
sleep 7.194
noteon 12 62 102
sleep 8.992
noteoff 13 54 0
noteon 13 50 104
sleep 1.798
noteoff 3 54 0
noteoff 3 66 0
noteon 3 62 100
noteon 3 50 100
sleep 3.597
noteoff 14 42 0
noteon 14 38 106
sleep 64.748
noteoff 10 62 0
sleep 3.597
noteoff 11 62 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 62 102
sleep 3.597
noteon 11 62 102
sleep 7.194
noteon 12 62 102
sleep 79.136
noteoff 10 62 0
sleep 3.597
noteoff 11 62 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
echo "78960 tempo_s=310 tempo_l=0.25"
noteon 10 61 102
sleep 1.612
noteoff 0 86 0
noteon 0 85 101
sleep 1.612
noteoff 1 74 0
noteon 1 73 100
noteon 11 61 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 3 50 0
noteoff 3 62 0
noteon 3 49 100
noteon 3 61 100
sleep 3.225
noteoff 14 38 0
noteon 14 37 106
sleep 58.064
noteoff 10 61 0
sleep 3.225
noteoff 11 61 0
sleep 6.451
noteoff 12 61 0
sleep 6.451
noteon 10 61 102
sleep 3.225
noteon 11 61 102
sleep 6.451
noteon 12 61 102
sleep 70.967
noteoff 10 61 0
sleep 3.225
noteoff 11 61 0
sleep 6.451
noteoff 12 61 0
sleep 6.451
noteon 10 59 102
sleep 1.612
noteoff 0 85 0
noteon 0 83 101
sleep 1.612
noteoff 1 73 0
noteon 1 71 100
noteon 11 59 102
sleep 6.451
noteon 12 59 102
sleep 8.064
noteoff 13 49 0
noteon 13 47 104
sleep 1.612
noteoff 3 61 0
noteoff 3 49 0
noteon 3 47 100
noteon 3 59 100
sleep 3.225
noteoff 14 37 0
noteon 14 35 106
sleep 58.064
noteoff 10 59 0
sleep 3.225
noteoff 11 59 0
sleep 6.451
noteoff 12 59 0
sleep 6.451
noteon 10 59 102
sleep 3.225
noteon 11 59 102
sleep 6.451
noteon 12 59 102
sleep 70.967
noteoff 10 59 0
sleep 1.612
noteoff 0 83 0
sleep 1.612
noteoff 1 71 0
noteoff 11 59 0
sleep 6.451
noteoff 12 59 0
sleep 6.451
echo "79200 tempo_s=278 tempo_l=0.25"
noteon 10 57 102
sleep 1.798
noteoff 13 47 0
noteon 0 81 101
sleep 1.798
noteoff 3 59 0
noteoff 3 47 0
noteon 1 69 100
noteon 4 66 100
noteon 11 57 102
sleep 3.597
noteoff 14 35 0
sleep 3.597
noteon 5 54 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
sleep 3.597
noteon 14 33 106
sleep 190.647
noteoff 10 57 0
sleep 1.798
noteoff 0 81 0
sleep 1.798
noteoff 1 69 0
noteoff 11 57 0
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
sleep 3.597
noteoff 14 33 0
sleep 84.532
noteon 0 89 101
sleep 1.798
noteon 1 77 100
sleep 17.985
noteon 3 65 100
sleep 86.33
echo "79440 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 0 89 0
noteon 0 90 101
sleep 1.612
noteoff 1 77 0
noteon 1 78 100
sleep 16.129
noteoff 3 65 0
noteon 3 66 100
sleep 79.032
noteoff 0 90 0
sleep 1.612
noteoff 1 78 0
sleep 16.129
noteoff 3 66 0
sleep 174.193
noteon 10 57 102
sleep 3.225
noteon 11 57 102
sleep 6.451
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 1.612
noteon 3 45 100
sleep 3.225
noteon 14 33 106
sleep 37.096
noteoff 4 66 0
sleep 6.451
noteoff 5 54 0
sleep 30.645
echo "79680 tempo_s=278 tempo_l=0.25"
noteoff 10 57 0
noteon 10 59 102
sleep 3.597
noteoff 11 57 0
noteon 4 66 100
noteon 11 59 102
sleep 7.194
noteoff 12 57 0
noteon 5 54 100
noteon 12 59 102
sleep 8.992
noteoff 13 45 0
noteon 13 47 104
sleep 1.798
noteoff 3 45 0
noteon 3 47 100
sleep 3.597
noteoff 14 33 0
noteon 14 35 106
sleep 82.731
noteoff 10 59 0
sleep 3.597
noteoff 11 59 0
sleep 7.194
noteoff 12 59 0
sleep 8.992
noteoff 13 47 0
sleep 1.798
noteoff 3 47 0
sleep 3.597
noteoff 14 35 0
sleep 192.442
noteon 0 89 101
sleep 1.798
noteon 1 77 100
sleep 17.985
noteon 3 65 100
sleep 86.33
echo "79920 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 0 89 0
noteon 0 90 101
sleep 1.612
noteoff 1 77 0
noteon 1 78 100
sleep 16.129
noteoff 3 65 0
noteon 3 66 100
sleep 79.032
noteoff 0 90 0
sleep 1.612
noteoff 1 78 0
sleep 16.129
noteoff 3 66 0
sleep 174.193
noteon 10 59 102
sleep 3.225
noteon 11 59 102
sleep 6.451
noteon 12 59 102
sleep 8.064
noteon 13 47 104
sleep 1.612
noteon 3 47 100
sleep 3.225
noteon 14 35 106
sleep 37.096
noteoff 4 66 0
sleep 6.451
noteoff 5 54 0
sleep 30.645
echo "80160 tempo_s=278 tempo_l=0.25"
noteoff 10 59 0
noteon 10 61 102
sleep 3.597
noteoff 11 59 0
noteon 4 66 100
noteon 11 61 102
sleep 7.194
noteoff 12 59 0
noteon 5 54 100
noteon 12 61 102
sleep 8.992
noteoff 13 47 0
noteon 13 49 104
sleep 1.798
noteoff 3 47 0
noteon 3 49 100
sleep 3.597
noteoff 14 35 0
noteon 14 37 106
sleep 82.731
noteoff 10 61 0
sleep 3.597
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 8.992
noteoff 13 49 0
sleep 1.798
noteoff 3 49 0
sleep 3.597
noteoff 14 37 0
sleep 192.442
noteon 0 89 101
sleep 1.798
noteon 1 77 100
sleep 17.985
noteon 3 65 100
sleep 86.33
echo "80400 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 0 89 0
noteon 0 90 101
sleep 1.612
noteoff 1 77 0
noteon 1 78 100
sleep 16.129
noteoff 3 65 0
noteon 3 66 100
sleep 79.032
noteoff 0 90 0
sleep 1.612
noteoff 1 78 0
sleep 16.129
noteoff 3 66 0
sleep 174.193
noteon 10 61 102
sleep 3.225
noteon 11 61 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteon 13 49 104
sleep 1.612
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 37.096
noteoff 4 66 0
sleep 6.451
noteoff 5 54 0
sleep 30.645
echo "80640 tempo_s=278 tempo_l=0.25"
noteoff 10 61 0
noteon 10 62 102
sleep 3.597
noteoff 11 61 0
noteon 4 66 100
noteon 11 62 102
sleep 7.194
noteoff 12 61 0
noteon 5 54 100
noteon 12 62 102
sleep 8.992
noteoff 13 49 0
noteon 13 50 104
sleep 1.798
noteoff 3 49 0
noteon 3 50 100
sleep 3.597
noteoff 14 37 0
noteon 14 38 106
sleep 82.731
noteoff 10 62 0
sleep 3.597
noteoff 11 62 0
sleep 7.194
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
sleep 3.597
noteoff 14 38 0
sleep 84.529
noteon 0 90 101
sleep 1.798
noteoff 4 66 0
noteon 1 78 100
sleep 7.194
noteoff 5 54 0
sleep 10.791
noteon 3 66 100
sleep 86.33
noteon 10 62 102
sleep 3.597
noteon 11 62 102
sleep 7.194
noteon 12 62 102
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 50 100
sleep 3.597
noteon 14 38 106
sleep 82.733
echo "80880 tempo_s=310 tempo_l=0.25"
noteoff 10 62 0
noteon 10 60 102
sleep 3.225
noteoff 11 62 0
noteon 4 66 100
noteon 11 60 102
sleep 6.451
noteoff 12 62 0
noteon 5 54 100
noteon 12 60 102
sleep 8.063
noteoff 13 50 0
noteon 13 48 104
sleep 1.612
noteoff 3 50 0
noteon 3 48 100
sleep 3.225
noteoff 14 38 0
noteon 14 36 106
sleep 74.183
noteoff 10 60 0
sleep 3.225
noteoff 11 60 0
sleep 6.451
noteoff 12 60 0
sleep 8.063
noteoff 13 48 0
sleep 1.612
noteoff 3 48 0
sleep 3.225
noteoff 14 36 0
sleep 35.479
noteoff 0 90 0
sleep 1.612
noteoff 1 78 0
sleep 16.126
noteoff 3 66 0
sleep 22.576
noteon 0 90 101
sleep 1.612
noteoff 4 66 0
noteon 1 78 100
sleep 6.451
noteoff 5 54 0
sleep 9.677
noteon 3 66 100
sleep 77.419
noteon 10 60 102
sleep 3.225
noteon 11 60 102
sleep 6.451
noteon 12 60 102
sleep 8.064
noteon 13 48 104
sleep 1.612
noteon 3 48 100
sleep 3.225
noteon 14 36 106
sleep 74.193
echo "81120 tempo_s=278 tempo_l=0.25"
noteoff 10 60 0
noteon 10 61 102
sleep 3.597
noteoff 11 60 0
noteon 4 66 100
noteon 11 61 102
sleep 7.194
noteoff 12 60 0
noteon 5 54 100
noteon 12 61 102
sleep 8.992
noteoff 13 48 0
noteon 13 49 104
sleep 1.798
noteoff 3 48 0
noteon 3 49 100
sleep 3.597
noteoff 14 36 0
noteon 14 37 106
sleep 68.345
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteoff 10 61 0
sleep 1.798
noteoff 13 49 0
sleep 1.798
noteon 11 69 102
sleep 3.597
noteoff 14 37 0
sleep 3.597
noteon 12 66 102
sleep 8.992
noteon 13 61 104
sleep 1.798
noteoff 3 49 0
sleep 3.597
noteon 14 49 106
sleep 39.568
noteoff 0 90 0
sleep 1.798
noteoff 1 78 0
sleep 17.985
noteoff 3 66 0
sleep 8.992
noteoff 11 69 0
sleep 7.194
noteoff 12 66 0
sleep 7.194
noteon 10 85 102
sleep 1.798
noteoff 13 61 0
sleep 1.798
noteoff 4 66 0
noteon 11 69 102
sleep 3.597
noteoff 14 49 0
sleep 3.597
noteoff 5 54 0
noteon 12 66 102
sleep 8.992
noteon 13 61 104
sleep 5.395
noteon 14 49 106
sleep 68.343
noteoff 11 69 0
sleep 7.194
noteoff 12 66 0
sleep 8.992
noteoff 13 61 0
sleep 1.798
noteon 11 69 102
sleep 3.597
noteoff 14 49 0
sleep 3.597
noteon 12 66 102
sleep 8.992
noteon 13 61 104
sleep 5.395
noteon 14 49 106
sleep 68.343
noteoff 11 69 0
sleep 7.194
noteoff 12 66 0
sleep 7.194
echo "81360 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 13 61 0
sleep 1.612
noteon 11 69 102
sleep 3.225
noteoff 14 49 0
sleep 3.225
noteon 12 66 102
sleep 8.064
noteon 13 61 104
sleep 4.837
noteon 14 49 106
sleep 61.288
noteoff 11 69 0
sleep 6.451
noteoff 12 66 0
sleep 8.063
noteoff 13 61 0
sleep 1.612
noteon 11 69 102
sleep 3.225
noteoff 14 49 0
sleep 3.225
noteon 12 66 102
sleep 8.064
noteon 13 61 104
sleep 4.837
noteon 14 49 106
sleep 58.062
noteoff 10 85 0
sleep 3.225
noteoff 11 69 0
sleep 6.451
noteoff 12 66 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 61 0
sleep 1.612
noteon 11 69 102
sleep 3.225
noteoff 14 49 0
sleep 3.225
noteon 12 66 102
sleep 8.064
noteon 13 61 104
sleep 4.838
noteon 14 49 106
sleep 61.29
noteoff 11 69 0
sleep 6.451
noteoff 12 66 0
sleep 6.451
noteoff 10 81 0
noteon 10 78 102
sleep 1.612
noteoff 13 61 0
sleep 1.612
noteon 11 69 102
sleep 3.225
noteoff 14 49 0
sleep 3.225
noteon 12 66 102
sleep 8.064
noteon 13 61 104
sleep 4.838
noteon 14 49 106
sleep 58.064
noteoff 10 78 0
sleep 3.225
noteoff 11 69 0
sleep 6.451
noteoff 12 66 0
sleep 6.451
echo "81600 tempo_s=278 tempo_l=0.25"
noteon 10 73 102
sleep 1.798
noteoff 13 61 0
sleep 1.798
noteon 11 68 102
sleep 3.597
noteoff 14 49 0
sleep 3.597
noteon 12 66 102
sleep 8.992
noteon 13 49 104
sleep 5.395
noteon 14 37 106
sleep 68.345
noteoff 11 68 0
sleep 7.194
noteoff 12 66 0
sleep 8.992
noteoff 13 49 0
sleep 1.798
noteon 11 68 102
sleep 3.597
noteoff 14 37 0
sleep 3.597
noteon 12 66 102
sleep 8.992
noteon 13 61 104
sleep 5.395
noteon 14 49 106
sleep 68.345
noteoff 11 68 0
sleep 7.194
noteoff 12 66 0
sleep 8.992
noteoff 13 61 0
sleep 1.798
noteon 11 68 102
sleep 3.597
noteoff 14 49 0
sleep 3.597
noteon 12 66 102
sleep 8.992
noteon 13 61 104
sleep 5.395
noteon 14 49 106
sleep 68.345
noteoff 11 68 0
sleep 7.194
noteoff 12 66 0
sleep 8.992
noteoff 13 61 0
sleep 1.798
noteon 11 68 102
sleep 3.597
noteoff 14 49 0
sleep 3.597
noteon 12 66 102
sleep 8.992
noteon 13 61 104
sleep 5.395
noteon 14 49 106
sleep 64.748
noteoff 10 73 0
sleep 3.597
noteoff 11 68 0
sleep 7.194
noteoff 12 66 0
sleep 7.194
echo "81840 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 1.612
noteoff 13 61 0
sleep 1.612
noteon 11 68 102
sleep 3.225
noteoff 14 49 0
sleep 3.225
noteon 12 65 102
sleep 8.064
noteon 13 49 104
sleep 4.838
noteon 14 37 106
sleep 9.677
noteoff 10 81 0
noteon 10 80 102
sleep 32.258
noteoff 10 80 0
noteon 10 81 102
sleep 19.354
noteoff 11 68 0
sleep 6.451
noteoff 12 65 0
sleep 6.451
noteoff 10 81 0
noteon 10 80 102
sleep 1.612
noteoff 13 49 0
sleep 1.612
noteon 11 68 102
sleep 3.225
noteoff 14 37 0
sleep 3.225
noteon 12 65 102
sleep 8.064
noteon 13 61 104
sleep 4.838
noteon 14 49 106
sleep 9.677
noteoff 10 80 0
noteon 10 81 102
sleep 32.258
noteoff 10 81 0
noteon 10 80 102
sleep 19.354
noteoff 11 68 0
sleep 6.451
noteoff 12 65 0
sleep 6.451
noteoff 10 80 0
noteon 10 81 102
sleep 1.612
noteoff 13 61 0
sleep 1.612
noteon 11 68 102
sleep 3.225
noteoff 14 49 0
sleep 3.225
noteon 12 65 102
sleep 8.064
noteon 13 61 104
sleep 4.838
noteon 14 49 106
sleep 9.677
noteoff 10 81 0
noteon 10 80 102
sleep 32.258
noteoff 10 80 0
noteon 10 81 102
sleep 19.354
noteoff 11 68 0
sleep 6.451
noteoff 12 65 0
sleep 6.451
noteoff 10 81 0
noteon 10 80 102
sleep 1.612
noteoff 13 61 0
sleep 1.612
noteon 11 68 102
sleep 3.225
noteoff 14 49 0
sleep 3.225
noteon 12 65 102
sleep 8.064
noteon 13 61 104
sleep 4.838
noteon 14 49 106
sleep 9.677
noteoff 10 80 0
noteon 10 81 102
sleep 32.258
noteoff 10 81 0
noteon 10 80 102
sleep 19.354
noteoff 11 68 0
sleep 6.451
noteoff 12 65 0
sleep 6.451
echo "82080 tempo_s=278 tempo_l=0.25"
noteoff 10 80 0
noteon 10 78 102
sleep 1.798
noteoff 13 61 0
noteon 0 81 101
sleep 1.798
noteon 1 69 100
noteon 4 66 100
noteon 11 66 102
sleep 1.798
noteon 6 66 108
sleep 1.798
noteoff 14 49 0
sleep 3.597
noteon 5 54 100
noteon 12 66 102
sleep 8.992
noteon 13 54 104
sleep 1.798
noteon 3 54 100
sleep 3.597
noteon 14 42 106
sleep 190.647
noteoff 10 78 0
sleep 1.798
noteoff 0 81 0
sleep 1.798
noteoff 1 69 0
noteoff 11 66 0
sleep 7.194
noteoff 12 66 0
sleep 8.992
noteoff 13 54 0
sleep 1.798
noteoff 3 54 0
sleep 3.597
noteoff 14 42 0
sleep 84.532
noteon 0 89 101
sleep 1.798
noteon 1 77 100
sleep 17.985
noteon 3 65 100
sleep 86.33
echo "82320 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 0 89 0
noteon 0 90 101
sleep 1.612
noteoff 1 77 0
noteon 1 78 100
sleep 16.129
noteoff 3 65 0
noteon 3 66 100
sleep 79.032
noteoff 0 90 0
sleep 1.612
noteoff 1 78 0
sleep 16.129
noteoff 3 66 0
sleep 174.193
noteon 10 69 102
sleep 1.612
noteon 0 81 101
sleep 1.612
noteon 1 69 100
noteon 11 57 102
sleep 6.451
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 1.612
noteon 3 45 100
sleep 3.225
noteon 14 33 106
sleep 37.096
noteoff 4 66 0
sleep 1.612
noteoff 6 66 0
sleep 4.838
noteoff 5 54 0
sleep 30.645
echo "82560 tempo_s=278 tempo_l=0.25"
noteoff 10 69 0
noteon 10 71 102
sleep 1.798
noteoff 0 81 0
noteon 0 83 101
sleep 1.798
noteoff 1 69 0
noteoff 11 57 0
noteon 1 71 100
noteon 4 66 100
noteon 11 59 102
sleep 1.798
noteon 6 66 108
sleep 5.395
noteoff 12 57 0
noteon 5 54 100
noteon 12 59 102
sleep 8.992
noteoff 13 45 0
noteon 13 47 104
sleep 1.798
noteoff 3 45 0
noteon 3 47 100
sleep 3.597
noteoff 14 33 0
noteon 14 35 106
sleep 82.723
noteoff 10 71 0
sleep 1.798
noteoff 0 83 0
sleep 1.798
noteoff 1 71 0
noteoff 11 59 0
sleep 7.193
noteoff 12 59 0
sleep 8.992
noteoff 13 47 0
sleep 1.798
noteoff 3 47 0
sleep 3.597
noteoff 14 35 0
sleep 192.433
noteon 0 89 101
sleep 1.798
noteon 1 77 100
sleep 17.985
noteon 3 65 100
sleep 86.33
echo "82800 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 0 89 0
noteon 0 90 101
sleep 1.612
noteoff 1 77 0
noteon 1 78 100
sleep 16.129
noteoff 3 65 0
noteon 3 66 100
sleep 79.032
noteoff 0 90 0
sleep 1.612
noteoff 1 78 0
sleep 16.129
noteoff 3 66 0
sleep 174.193
noteon 10 71 102
sleep 1.612
noteon 0 83 101
sleep 1.612
noteon 1 71 100
noteon 11 59 102
sleep 6.451
noteon 12 59 102
sleep 8.064
noteon 13 47 104
sleep 1.612
noteon 3 47 100
sleep 3.225
noteon 14 35 106
sleep 37.096
noteoff 4 66 0
sleep 1.612
noteoff 6 66 0
sleep 4.838
noteoff 5 54 0
sleep 30.645
echo "83040 tempo_s=278 tempo_l=0.25"
noteoff 10 71 0
noteon 10 73 102
sleep 1.798
noteoff 0 83 0
noteon 0 85 101
sleep 1.798
noteoff 1 71 0
noteoff 11 59 0
noteon 1 73 100
noteon 4 66 100
noteon 11 61 102
sleep 1.798
noteon 6 66 108
sleep 5.395
noteoff 12 59 0
noteon 5 54 100
noteon 12 61 102
sleep 8.992
noteoff 13 47 0
noteon 13 49 104
sleep 1.798
noteoff 3 47 0
noteon 3 49 100
sleep 3.597
noteoff 14 35 0
noteon 14 37 106
sleep 82.723
noteoff 10 73 0
sleep 1.798
noteoff 0 85 0
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
sleep 7.193
noteoff 12 61 0
sleep 8.992
noteoff 13 49 0
sleep 1.798
noteoff 3 49 0
sleep 3.597
noteoff 14 37 0
sleep 192.433
noteon 0 89 101
sleep 1.798
noteon 1 77 100
sleep 17.985
noteon 3 65 100
sleep 86.33
echo "83280 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 0 89 0
noteon 0 90 101
sleep 1.612
noteoff 1 77 0
noteon 1 78 100
sleep 16.129
noteoff 3 65 0
noteon 3 66 100
sleep 79.032
noteoff 0 90 0
sleep 1.612
noteoff 1 78 0
sleep 16.129
noteoff 3 66 0
sleep 174.193
noteon 10 73 102
sleep 1.612
noteon 0 85 101
sleep 1.612
noteon 1 73 100
noteon 11 61 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteon 13 49 104
sleep 1.612
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 74.193
echo "83520 tempo_s=278 tempo_l=0.25"
noteoff 10 73 0
noteon 10 74 102
sleep 1.798
noteoff 0 85 0
noteon 0 86 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 7.194
noteoff 12 61 0
noteon 12 62 102
sleep 8.992
noteoff 13 49 0
noteon 13 50 104
sleep 1.798
noteoff 3 49 0
noteon 3 50 100
sleep 3.597
noteoff 14 37 0
noteon 14 38 106
sleep 82.733
noteoff 10 74 0
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteoff 1 74 0
noteoff 11 62 0
sleep 7.194
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
sleep 3.597
noteoff 14 38 0
sleep 190.647
noteon 10 74 102
sleep 1.798
noteon 0 86 101
sleep 1.798
noteon 1 74 100
noteon 11 62 102
sleep 7.194
noteon 12 62 102
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 50 100
sleep 3.597
noteon 14 38 106
sleep 82.733
echo "83760 tempo_s=310 tempo_l=0.25"
noteoff 10 74 0
noteon 10 72 102
sleep 1.612
noteoff 0 86 0
noteon 0 84 101
sleep 1.612
noteoff 1 74 0
noteoff 11 62 0
noteon 1 72 100
noteon 11 60 102
sleep 6.451
noteoff 12 62 0
noteon 12 60 102
sleep 8.064
noteoff 13 50 0
noteon 13 48 104
sleep 1.612
noteoff 3 50 0
noteon 3 48 100
sleep 3.225
noteoff 14 38 0
noteon 14 36 106
sleep 74.193
noteoff 10 72 0
sleep 1.612
noteoff 0 84 0
sleep 1.612
noteoff 1 72 0
noteoff 11 60 0
sleep 6.451
noteoff 12 60 0
sleep 8.064
noteoff 13 48 0
sleep 1.612
noteoff 3 48 0
sleep 3.225
noteoff 14 36 0
sleep 170.967
noteon 10 72 102
sleep 1.612
noteon 0 84 101
sleep 1.612
noteon 1 72 100
noteon 11 60 102
sleep 6.451
noteon 12 60 102
sleep 8.064
noteon 13 48 104
sleep 1.612
noteon 3 48 100
sleep 3.225
noteon 14 36 106
sleep 37.096
noteoff 4 66 0
sleep 1.612
noteoff 6 66 0
sleep 4.838
noteoff 5 54 0
sleep 30.645
echo "84000 tempo_s=278 tempo_l=0.25"
noteoff 10 72 0
noteon 10 73 102
sleep 1.798
noteoff 0 84 0
noteon 0 85 101
sleep 1.798
noteoff 1 72 0
noteoff 11 60 0
noteon 4 66 100
noteon 11 61 102
noteon 1 73 100
sleep 1.798
noteon 6 66 108
sleep 5.395
noteoff 12 60 0
noteon 5 54 100
noteon 12 61 102
sleep 8.991
noteoff 13 48 0
noteon 13 49 104
sleep 1.798
noteoff 3 48 0
noteon 3 49 100
sleep 3.596
noteoff 14 36 0
noteon 14 37 106
sleep 82.716
noteoff 10 73 0
sleep 1.798
noteoff 0 85 0
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
sleep 7.193
noteoff 12 61 0
sleep 8.991
noteoff 13 49 0
sleep 1.798
noteoff 3 49 0
sleep 3.596
noteoff 14 37 0
sleep 192.425
noteon 0 89 101
sleep 1.798
noteon 1 77 100
sleep 17.985
noteon 3 65 100
sleep 86.33
echo "84240 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 0 89 0
noteon 0 90 101
sleep 1.612
noteoff 1 77 0
noteon 1 78 100
sleep 16.129
noteoff 3 65 0
noteon 3 66 100
sleep 79.032
noteoff 0 90 0
sleep 1.612
noteoff 1 78 0
sleep 16.129
noteoff 3 66 0
sleep 174.193
noteon 10 73 102
sleep 1.612
noteon 0 85 101
sleep 1.612
noteon 1 73 100
noteon 11 61 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteon 13 49 104
sleep 1.612
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 74.193
echo "84480 tempo_s=278 tempo_l=0.25"
noteoff 10 73 0
noteon 10 74 102
sleep 1.798
noteoff 0 85 0
noteon 0 86 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 7.194
noteoff 12 61 0
noteon 12 62 102
sleep 8.992
noteoff 13 49 0
noteon 13 50 104
sleep 1.798
noteoff 3 49 0
noteon 3 50 100
sleep 3.597
noteoff 14 37 0
noteon 14 38 106
sleep 82.733
noteoff 10 74 0
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteoff 1 74 0
noteoff 11 62 0
sleep 7.194
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
sleep 3.597
noteoff 14 38 0
sleep 190.647
noteon 10 74 102
sleep 1.798
noteon 0 86 101
sleep 1.798
noteon 1 74 100
noteon 11 62 102
sleep 7.194
noteon 12 62 102
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 50 100
sleep 3.597
noteon 14 38 106
sleep 82.733
echo "84720 tempo_s=310 tempo_l=0.25"
noteoff 10 74 0
noteon 10 72 102
sleep 1.612
noteoff 0 86 0
noteon 0 84 101
sleep 1.612
noteoff 1 74 0
noteoff 11 62 0
noteon 1 72 100
noteon 11 60 102
sleep 6.451
noteoff 12 62 0
noteon 12 60 102
sleep 8.064
noteoff 13 50 0
noteon 13 48 104
sleep 1.612
noteoff 3 50 0
noteon 3 48 100
sleep 3.225
noteoff 14 38 0
noteon 14 36 106
sleep 74.193
noteoff 10 72 0
sleep 1.612
noteoff 0 84 0
sleep 1.612
noteoff 1 72 0
noteoff 11 60 0
sleep 6.451
noteoff 12 60 0
sleep 8.064
noteoff 13 48 0
sleep 1.612
noteoff 3 48 0
sleep 3.225
noteoff 14 36 0
sleep 170.967
noteon 10 72 102
sleep 1.612
noteon 0 84 101
sleep 1.612
noteon 1 72 100
noteon 11 60 102
sleep 6.451
noteon 12 60 102
sleep 8.064
noteon 13 48 104
sleep 1.612
noteon 3 48 100
sleep 3.225
noteon 14 36 106
sleep 38.709
noteoff 6 66 0
sleep 35.483
echo "84960 tempo_s=278 tempo_l=0.25"
noteoff 10 72 0
noteon 10 73 102
sleep 1.798
noteoff 0 84 0
noteon 0 85 101
sleep 1.798
noteoff 1 72 0
noteoff 11 60 0
noteon 1 73 100
noteon 11 61 102
sleep 1.798
noteon 6 66 108
sleep 5.395
noteoff 12 60 0
noteon 12 61 102
sleep 8.992
noteoff 13 48 0
noteon 13 49 104
sleep 1.798
noteoff 3 48 0
noteon 3 49 100
sleep 3.597
noteoff 14 36 0
noteon 14 37 106
sleep 82.733
noteoff 10 73 0
sleep 1.798
noteoff 0 85 0
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 8.992
noteoff 13 49 0
sleep 1.798
noteoff 3 49 0
sleep 3.597
noteoff 14 37 0
sleep 41.366
noteoff 4 66 0
sleep 7.194
noteoff 5 54 0
sleep 35.971
noteon 0 90 101
sleep 1.798
noteon 1 78 100
noteon 4 66 100
sleep 7.194
noteon 5 54 100
sleep 10.791
noteon 3 66 100
sleep 86.33
noteon 10 73 102
sleep 1.798
noteon 0 85 101
sleep 1.798
noteon 1 73 100
noteon 11 61 102
sleep 7.194
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteon 3 49 100
sleep 3.597
noteon 14 37 106
sleep 82.733
echo "85200 tempo_s=310 tempo_l=0.25"
noteoff 10 73 0
noteon 10 74 102
sleep 1.612
noteoff 0 85 0
noteon 0 86 101
sleep 1.612
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 6.451
noteoff 12 61 0
noteon 12 62 102
sleep 8.064
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 74.193
noteoff 10 74 0
sleep 1.612
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
noteoff 11 62 0
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 50 0
sleep 1.612
noteoff 3 50 0
sleep 3.225
noteoff 14 38 0
sleep 37.096
noteoff 4 66 0
sleep 6.451
noteoff 5 54 0
sleep 16.129
noteoff 0 90 0
sleep 1.612
noteoff 1 78 0
sleep 14.516
noteon 0 90 101
sleep 1.612
noteoff 3 66 0
noteon 1 78 100
noteon 4 66 100
sleep 6.451
noteon 5 54 100
sleep 9.677
noteon 3 66 100
sleep 77.419
noteon 10 72 102
sleep 1.612
noteon 0 84 101
sleep 1.612
noteon 1 72 100
noteon 11 60 102
sleep 6.451
noteon 12 60 102
sleep 8.064
noteon 13 48 104
sleep 1.612
noteon 3 48 100
sleep 3.225
noteon 14 36 106
sleep 74.193
echo "85440 tempo_s=278 tempo_l=0.25"
noteoff 10 72 0
noteon 10 73 102
sleep 1.798
noteoff 0 84 0
noteon 0 85 116
sleep 1.798
noteoff 1 72 0
noteoff 11 60 0
noteon 1 73 115
noteon 11 61 102
sleep 7.194
noteoff 12 60 0
noteon 12 61 102
sleep 8.992
noteoff 13 48 0
noteon 13 49 104
sleep 1.798
noteoff 3 48 0
noteon 3 49 115
sleep 3.597
noteoff 14 36 0
noteon 14 37 106
sleep 82.733
noteoff 10 73 0
sleep 1.798
noteoff 0 85 0
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 8.992
noteoff 13 49 0
sleep 1.798
noteoff 3 49 0
sleep 3.597
noteoff 14 37 0
sleep 41.366
noteoff 4 66 0
sleep 7.194
noteoff 5 54 0
sleep 17.985
noteoff 0 90 0
sleep 1.798
noteoff 1 78 0
sleep 16.187
noteon 0 90 101
sleep 1.798
noteoff 3 66 0
noteon 1 78 100
noteon 4 66 100
sleep 7.194
noteon 5 54 100
sleep 10.791
noteon 3 66 100
sleep 86.33
noteon 10 73 102
sleep 1.798
noteon 0 85 101
sleep 1.798
noteon 1 73 100
noteon 11 61 102
sleep 7.194
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteon 3 49 100
sleep 3.597
noteon 14 37 106
sleep 82.733
echo "85680 tempo_s=310 tempo_l=0.25"
noteoff 10 73 0
noteon 10 74 102
sleep 1.612
noteoff 0 85 0
noteon 0 86 101
sleep 1.612
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 6.451
noteoff 12 61 0
noteon 12 62 102
sleep 8.064
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 74.193
noteoff 10 74 0
sleep 1.612
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
noteoff 11 62 0
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 50 0
sleep 1.612
noteoff 3 50 0
sleep 3.225
noteoff 14 38 0
sleep 37.096
noteoff 4 66 0
sleep 6.451
noteoff 5 54 0
sleep 16.129
noteoff 0 90 0
sleep 1.612
noteoff 1 78 0
sleep 14.516
noteon 0 90 101
sleep 1.612
noteoff 3 66 0
noteon 1 78 100
noteon 4 66 100
sleep 6.451
noteon 5 54 100
sleep 9.677
noteon 3 66 100
sleep 77.419
noteon 10 72 102
sleep 1.612
noteon 0 84 101
sleep 1.612
noteon 1 72 100
noteon 11 60 102
sleep 6.451
noteon 12 60 102
sleep 8.064
noteon 13 48 104
sleep 1.612
noteon 3 48 100
sleep 3.225
noteon 14 36 106
sleep 74.193
echo "85920 tempo_s=278 tempo_l=0.25"
noteoff 10 72 0
noteon 10 73 102
sleep 1.798
noteoff 0 84 0
noteon 0 85 101
sleep 1.798
noteoff 1 72 0
noteoff 11 60 0
noteon 1 73 100
noteon 11 61 102
sleep 7.194
noteoff 12 60 0
noteon 12 61 102
sleep 8.992
noteoff 13 48 0
noteon 13 49 104
sleep 1.798
noteoff 3 48 0
noteon 3 49 100
sleep 3.597
noteoff 14 36 0
noteon 14 37 106
sleep 46.757
noteoff 10 73 0
sleep 3.596
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 8.991
noteoff 13 49 0
sleep 5.395
noteoff 14 37 0
sleep 10.790
noteon 10 73 102
sleep 1.798
noteoff 0 85 0
sleep 1.798
noteoff 1 73 0
noteon 11 61 102
sleep 7.193
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteoff 3 49 0
sleep 3.597
noteon 14 37 106
sleep 41.361
noteoff 4 66 0
sleep 5.395
noteoff 10 73 0
sleep 1.798
noteoff 5 54 0
sleep 1.798
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 8.991
noteoff 0 90 0
noteoff 13 49 0
sleep 1.798
noteoff 1 78 0
sleep 3.596
noteoff 14 37 0
sleep 10.790
noteon 10 73 102
sleep 1.798
noteon 0 93 101
noteon 0 90 101
sleep 1.798
noteoff 3 66 0
noteon 1 81 100
noteon 1 78 100
noteon 4 66 100
noteon 11 61 102
sleep 7.193
noteon 5 54 100
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteon 3 66 100
noteon 3 57 100
sleep 3.597
noteon 14 37 106
sleep 46.762
noteoff 10 73 0
sleep 3.597
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 8.992
noteoff 13 49 0
sleep 5.395
noteoff 14 37 0
sleep 10.791
noteon 10 73 102
sleep 3.597
noteon 11 61 102
sleep 7.194
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 5.395
noteon 14 37 106
sleep 46.762
noteoff 10 73 0
sleep 3.597
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 8.992
noteoff 13 49 0
sleep 5.395
noteoff 14 37 0
sleep 10.791
echo "86160 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 3.225
noteon 11 62 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 41.935
noteoff 10 74 0
sleep 3.225
noteoff 11 62 0
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 9.677
noteon 10 74 102
sleep 3.225
noteon 11 62 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 37.096
noteoff 4 66 0
sleep 4.838
noteoff 10 74 0
sleep 1.612
noteoff 5 54 0
sleep 1.612
noteoff 11 62 0
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 0 90 0
noteoff 0 93 0
noteoff 13 50 0
sleep 1.612
noteoff 1 78 0
noteoff 1 81 0
sleep 3.225
noteoff 14 38 0
sleep 9.677
noteon 10 72 102
sleep 1.612
noteon 0 90 101
noteon 0 93 101
sleep 1.612
noteoff 3 57 0
noteoff 3 66 0
noteon 1 78 100
noteon 1 81 100
noteon 4 66 100
noteon 11 60 102
sleep 6.451
noteon 5 54 100
noteon 12 60 102
sleep 8.064
noteon 13 48 104
sleep 1.612
noteon 3 57 100
noteon 3 66 100
sleep 3.225
noteon 14 36 106
sleep 41.935
noteoff 10 72 0
sleep 3.225
noteoff 11 60 0
sleep 6.451
noteoff 12 60 0
sleep 8.064
noteoff 13 48 0
sleep 4.838
noteoff 14 36 0
sleep 9.677
noteon 10 72 102
sleep 3.225
noteon 11 60 102
sleep 6.451
noteon 12 60 102
sleep 8.064
noteon 13 48 104
sleep 4.838
noteon 14 36 106
sleep 41.935
noteoff 10 72 0
sleep 3.225
noteoff 11 60 0
sleep 6.451
noteoff 12 60 0
sleep 8.064
noteoff 13 48 0
sleep 4.838
noteoff 14 36 0
sleep 9.677
echo "86400 tempo_s=278 tempo_l=0.25"
noteon 10 73 102
sleep 3.597
noteon 11 61 102
sleep 7.194
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 5.395
noteon 14 37 106
sleep 46.757
noteoff 10 73 0
sleep 3.596
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 8.991
noteoff 13 49 0
sleep 5.395
noteoff 14 37 0
sleep 10.790
noteon 10 73 102
sleep 3.597
noteon 11 61 102
sleep 7.193
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 5.395
noteon 14 37 106
sleep 41.361
noteoff 4 66 0
sleep 5.395
noteoff 10 73 0
sleep 1.798
noteoff 5 54 0
sleep 1.798
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 8.991
noteoff 0 93 0
noteoff 0 90 0
noteoff 13 49 0
sleep 1.798
noteoff 1 81 0
noteoff 1 78 0
sleep 3.596
noteoff 14 37 0
sleep 10.790
noteon 10 73 102
sleep 1.798
noteon 0 93 101
noteon 0 90 101
sleep 1.798
noteoff 3 66 0
noteoff 3 57 0
noteon 1 81 100
noteon 1 78 100
noteon 4 66 100
noteon 11 61 102
sleep 7.193
noteon 5 54 100
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteon 3 57 100
noteon 3 66 100
sleep 3.597
noteon 14 37 106
sleep 46.762
noteoff 10 73 0
sleep 3.597
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 8.992
noteoff 13 49 0
sleep 5.395
noteoff 14 37 0
sleep 10.791
noteon 10 73 102
sleep 3.597
noteon 11 61 102
sleep 7.194
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 5.395
noteon 14 37 106
sleep 46.762
noteoff 10 73 0
sleep 3.597
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 8.992
noteoff 13 49 0
sleep 5.395
noteoff 14 37 0
sleep 10.791
echo "86640 tempo_s=287 tempo_l=0.25"
noteon 10 74 102
sleep 3.484
noteon 11 62 102
sleep 6.968
noteon 12 62 102
sleep 8.71
noteon 13 50 104
sleep 5.226
noteon 14 38 106
sleep 45.296
noteoff 10 74 0
sleep 3.484
noteoff 11 62 0
sleep 6.968
noteoff 12 62 0
sleep 8.71
noteoff 13 50 0
sleep 5.226
noteoff 14 38 0
sleep 10.452
noteon 10 74 102
sleep 3.484
noteon 11 62 102
sleep 6.968
noteon 12 62 102
sleep 8.71
noteon 13 50 104
sleep 5.226
noteon 14 38 106
sleep 40.069
noteoff 4 66 0
sleep 5.226
noteoff 10 74 0
sleep 1.742
noteoff 5 54 0
sleep 1.742
noteoff 11 62 0
sleep 6.968
noteoff 12 62 0
sleep 8.71
noteoff 0 90 0
noteoff 0 93 0
noteoff 13 50 0
sleep 1.742
noteoff 1 78 0
noteoff 1 81 0
sleep 3.484
noteoff 14 38 0
sleep 10.452
noteon 10 72 102
sleep 1.742
noteon 0 90 101
noteon 0 93 101
sleep 1.742
noteoff 3 66 0
noteoff 3 57 0
noteon 1 78 100
noteon 1 81 100
noteon 4 66 100
noteon 11 60 102
sleep 6.968
noteon 5 54 100
noteon 12 60 102
sleep 8.71
noteon 13 48 104
sleep 1.742
noteon 3 66 100
noteon 3 57 100
sleep 3.484
noteon 14 36 106
sleep 45.296
noteoff 10 72 0
sleep 3.484
noteoff 11 60 0
sleep 6.968
noteoff 12 60 0
sleep 8.71
noteoff 13 48 0
sleep 5.226
noteoff 14 36 0
sleep 10.452
noteon 10 72 102
sleep 3.484
noteon 11 60 102
sleep 6.968
noteon 12 60 102
sleep 8.71
noteon 13 48 104
sleep 5.226
noteon 14 36 106
sleep 45.296
noteoff 10 72 0
sleep 3.484
noteoff 11 60 0
sleep 6.968
noteoff 12 60 0
sleep 8.71
noteoff 13 48 0
sleep 5.226
noteoff 14 36 0
sleep 10.452
echo "86880 tempo_s=250 tempo_l=0.25"
noteon 10 73 102
sleep 3.999
noteon 11 61 102
sleep 7.999
noteon 12 61 102
sleep 9.999
noteon 13 49 104
sleep 5.998
noteon 14 37 106
sleep 193.967
noteoff 0 93 0
noteoff 0 90 0
sleep 1.999
noteoff 1 81 0
noteoff 1 78 0
sleep 15.996
echo "87000 tempo_s=214 tempo_l=0.25"
noteoff 10 73 0
sleep 4.672
noteoff 3 57 0
noteoff 3 66 0
noteoff 4 66 0
noteoff 11 61 0
sleep 2.336
noteoff 6 66 0
sleep 7.008
noteoff 5 54 0
noteoff 12 61 0
sleep 11.681
noteoff 13 49 0
sleep 7.009
noteoff 14 37 0
sleep 107.476
echo "87060 tempo_s=257 tempo_l=0.25"
noteon 10 77 102
sleep 116.720
echo "87120 tempo_s=271 tempo_l=0.25"
noteoff 10 77 0
noteon 10 78 102
sleep 221.402
noteoff 10 78 0
sleep 221.402
echo "87360 tempo_s=241 tempo_l=0.25"
sleep 497.925
echo "87600 tempo_s=265 tempo_l=0.25"
sleep 452.83
echo "87840 tempo_s=255 tempo_l=0.25"
sleep 3.921
noteon 11 69 74
sleep 7.843
noteon 12 64 74
sleep 9.803
noteon 13 61 76
sleep 5.881
noteon 14 49 78
sleep 211.748
noteoff 11 69 0
sleep 7.843
noteoff 12 64 0
sleep 9.803
noteoff 13 61 0
sleep 5.882
noteoff 14 49 0
sleep 90.196
noteon 10 78 87
sleep 117.647
echo "88080 tempo_s=266 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 87
sleep 225.563
noteoff 10 79 0
sleep 225.563
echo "88320 tempo_s=254 tempo_l=0.25"
sleep 472.44
echo "88560 tempo_s=288 tempo_l=0.25"
sleep 208.333
echo "88680 tempo_s=307 tempo_l=0.25"
sleep 97.719
noteon 10 78 102
sleep 1.628
noteon 0 90 101
sleep 1.628
noteon 1 78 100
noteon 11 66 102
sleep 16.286
noteon 3 66 100
sleep 78.175
echo "88800 tempo_s=278 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteoff 11 66 0
noteon 1 79 100
noteon 11 67 102
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 86.33
noteoff 10 79 0
sleep 1.798
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
noteoff 11 67 0
sleep 17.985
noteoff 3 67 0
sleep 86.33
echo "88920 tempo_s=246 tempo_l=0.25"
sleep 121.951
noteon 10 61 102
sleep 2.032
noteon 0 73 101
sleep 2.032
noteon 1 73 100
noteon 11 61 102
sleep 8.13
noteon 12 61 102
sleep 10.162
noteon 13 49 104
sleep 2.032
noteon 3 49 100
sleep 4.065
noteon 14 37 106
sleep 73.17
noteoff 10 61 0
sleep 2.032
noteoff 0 73 0
sleep 2.032
noteoff 1 73 0
noteoff 11 61 0
sleep 8.13
noteoff 12 61 0
sleep 8.13
echo "89040 tempo_s=310 tempo_l=0.25"
noteon 10 62 102
sleep 1.612
noteoff 13 49 0
noteon 0 74 101
sleep 1.612
noteoff 3 49 0
noteon 1 74 100
noteon 11 62 102
sleep 3.225
noteoff 14 37 0
sleep 3.224
noteon 12 62 102
sleep 8.063
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 3.224
noteon 14 38 106
sleep 4.837
noteoff 0 74 0
noteon 0 73 101
sleep 4.836
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 6.448
noteoff 12 62 0
noteon 12 61 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 1.612
noteoff 0 73 0
noteon 0 74 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.450
noteoff 12 61 0
noteon 12 62 102
sleep 4.836
noteoff 0 74 0
noteon 0 73 101
sleep 8.062
noteoff 14 38 0
noteon 14 37 106
sleep 9.674
noteoff 10 62 0
noteon 10 61 102
sleep 3.224
noteoff 1 74 0
noteoff 11 61 0
noteon 1 73 100
noteon 11 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 8.062
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 12.898
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 1 73 0
noteon 1 74 100
sleep 6.448
noteoff 12 61 0
noteon 12 62 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 4.836
noteoff 0 73 0
noteon 0 74 101
sleep 4.838
noteoff 10 62 0
noteon 10 61 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 74 0
noteoff 3 50 0
noteon 1 73 100
noteon 3 49 100
sleep 6.450
noteoff 12 62 0
noteon 12 61 102
sleep 11.286
noteoff 0 74 0
noteon 0 73 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 3.224
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 6.449
noteoff 12 61 0
noteon 12 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 3.224
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.224
noteoff 14 38 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 4.838
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 9.677
noteoff 11 62 0
noteon 11 61 102
sleep 12.903
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 73 0
noteoff 13 50 0
noteon 0 74 101
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.451
noteoff 12 61 0
noteon 12 62 102
sleep 12.903
noteoff 14 37 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 59 102
sleep 1.612
noteoff 0 74 0
noteon 0 71 101
sleep 1.612
noteoff 1 74 0
noteoff 11 61 0
noteon 1 71 100
noteon 11 59 102
sleep 3.225
noteoff 14 37 0
sleep 3.225
noteoff 12 62 0
noteon 12 59 102
sleep 8.064
noteoff 13 49 0
noteon 13 47 104
sleep 1.612
noteoff 3 49 0
noteon 3 47 100
sleep 3.225
noteon 14 35 106
sleep 25.806
noteoff 10 59 0
noteon 10 61 102
sleep 1.612
noteoff 0 71 0
noteon 0 73 101
sleep 1.612
noteoff 1 71 0
noteoff 11 59 0
noteon 1 73 100
noteon 11 61 102
sleep 6.451
noteoff 12 59 0
noteon 12 61 102
sleep 8.064
noteoff 13 47 0
noteon 13 49 104
sleep 1.612
noteoff 3 47 0
noteon 3 49 100
sleep 3.225
noteoff 14 35 0
noteon 14 37 106
sleep 25.806
echo "89280 tempo_s=278 tempo_l=0.25"
noteoff 10 61 0
noteon 10 64 102
sleep 1.798
noteoff 0 73 0
noteon 0 76 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 76 100
noteon 11 64 102
sleep 7.194
noteoff 12 61 0
noteon 12 64 102
sleep 8.992
noteoff 13 49 0
noteon 13 52 104
sleep 1.798
noteoff 3 49 0
noteon 3 52 100
sleep 3.597
noteoff 14 37 0
noteon 14 40 106
sleep 82.733
noteoff 10 64 0
sleep 1.798
noteoff 0 76 0
sleep 1.798
noteoff 1 76 0
noteoff 11 64 0
sleep 7.194
noteoff 12 64 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 3.597
noteoff 14 40 0
sleep 82.733
noteon 10 57 102
sleep 1.798
noteon 0 69 101
sleep 1.798
noteon 1 69 100
noteon 11 57 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
sleep 3.597
noteon 14 33 106
sleep 82.733
noteoff 10 57 0
sleep 1.798
noteoff 0 69 0
sleep 1.798
noteoff 1 69 0
noteoff 11 57 0
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
sleep 3.597
noteoff 14 33 0
sleep 82.733
echo "89520 tempo_s=310 tempo_l=0.25"
sleep 193.548
noteon 10 69 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteon 12 67 102
sleep 8.064
noteon 13 67 104
sleep 30.645
noteoff 10 69 0
sleep 48.387
noteon 10 69 102
sleep 48.387
noteoff 10 69 0
sleep 48.387
echo "89760 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 10.791
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 67 0
noteon 13 66 104
sleep 88.129
noteoff 10 74 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 17.985
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
echo "90000 tempo_s=310 tempo_l=0.25"
noteon 10 76 102
sleep 9.677
noteoff 12 66 0
noteon 12 61 102
sleep 8.064
noteoff 13 66 0
noteon 13 61 104
sleep 79.032
noteoff 10 76 0
noteon 10 69 102
sleep 80.645
noteoff 10 69 0
sleep 16.129
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 48.387
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 41.935
noteoff 12 61 0
sleep 6.451
echo "90240 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 1.798
noteoff 13 61 0
sleep 8.992
noteon 12 62 102
sleep 8.992
noteon 13 62 104
sleep 88.129
noteoff 10 78 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 17.985
noteon 10 81 102
sleep 3.597
noteoff 11 69 0
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
echo "90480 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 3.225
noteon 11 71 102
sleep 6.451
noteoff 12 62 0
noteon 12 67 102
sleep 8.064
noteoff 13 62 0
noteon 13 55 104
sleep 4.838
noteon 14 31 106
sleep 74.193
noteoff 10 81 0
noteon 10 79 102
sleep 80.645
noteoff 10 79 0
sleep 16.129
noteon 10 78 102
sleep 22.58
noteoff 14 31 0
sleep 25.806
noteoff 10 78 0
sleep 48.387
noteon 10 76 102
sleep 48.387
noteoff 10 76 0
sleep 48.387
echo "90720 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 3.597
noteoff 11 71 0
noteon 11 69 102
sleep 7.194
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 55 0
noteon 13 57 104
sleep 5.395
noteon 14 33 106
sleep 82.733
noteoff 10 76 0
noteon 10 74 102
sleep 89.928
noteoff 10 74 0
sleep 17.985
noteon 10 74 102
sleep 25.179
noteoff 14 33 0
sleep 28.776
noteoff 10 74 0
sleep 53.956
noteon 10 74 102
sleep 53.956
noteoff 10 74 0
sleep 53.956
echo "90960 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 3.225
noteoff 11 69 0
noteon 4 64 115
noteon 11 67 102
sleep 6.451
noteoff 12 66 0
noteon 5 57 115
noteon 12 64 102
sleep 8.064
noteoff 13 57 0
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 74.193
noteoff 10 74 0
noteon 10 73 102
sleep 80.645
noteoff 10 73 0
sleep 16.129
noteon 10 71 102
sleep 22.58
noteoff 14 33 0
sleep 25.806
noteoff 10 71 0
sleep 48.387
noteon 10 73 102
sleep 48.387
noteoff 10 73 0
sleep 35.483
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
echo "91200 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteoff 13 45 0
noteon 0 86 101
sleep 1.798
noteoff 4 64 0
noteon 1 74 100
noteon 1 66 100
noteon 11 66 102
noteon 4 62 100
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 66 101
noteon 2 74 101
sleep 1.798
noteoff 5 57 0
noteon 12 62 102
noteon 5 54 100
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 62 100
noteon 3 50 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 82.733
noteoff 10 74 0
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteoff 1 66 0
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 66 0
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 3.597
noteoff 2 74 0
noteoff 2 66 0
sleep 1.798
noteoff 5 54 0
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
noteoff 3 62 0
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 82.733
noteon 10 67 102
noteon 10 76 102
sleep 1.798
noteon 0 85 101
noteon 0 88 101
sleep 1.798
noteon 1 76 100
noteon 1 69 100
noteon 4 64 100
noteon 11 61 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 76 101
noteon 2 69 101
sleep 1.798
noteon 5 57 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 57 100
noteon 3 45 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 82.733
noteoff 10 76 0
noteoff 10 67 0
sleep 1.798
noteoff 0 88 0
noteoff 0 85 0
sleep 1.798
noteoff 1 69 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 11 61 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 69 0
noteoff 2 76 0
sleep 1.798
noteoff 5 57 0
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
noteoff 3 57 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 82.733
echo "91440 tempo_s=257 tempo_l=0.25"
sleep 233.463
echo "91560 tempo_s=310 tempo_l=0.25"
sleep 96.774
noteon 10 78 102
sleep 1.612
noteon 0 90 101
sleep 1.612
noteon 1 78 100
noteon 11 66 102
sleep 16.127
noteon 3 66 100
sleep 77.418
echo "91680 tempo_s=278 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteoff 11 66 0
noteon 1 79 100
noteon 11 67 102
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 86.33
noteoff 10 79 0
sleep 1.798
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
noteoff 11 67 0
sleep 17.985
noteoff 3 67 0
sleep 86.33
echo "91800 tempo_s=246 tempo_l=0.25"
sleep 121.951
noteon 10 61 102
sleep 2.032
noteon 0 73 101
sleep 2.032
noteon 1 73 100
noteon 11 61 102
sleep 8.13
noteon 12 61 102
sleep 10.162
noteon 13 49 104
sleep 2.032
noteon 3 49 100
sleep 4.065
noteon 14 37 106
sleep 73.17
noteoff 10 61 0
sleep 2.032
noteoff 0 73 0
sleep 2.032
noteoff 1 73 0
noteoff 11 61 0
sleep 8.13
noteoff 12 61 0
sleep 8.13
echo "91920 tempo_s=310 tempo_l=0.25"
noteon 10 62 102
sleep 1.612
noteoff 13 49 0
noteon 0 74 101
sleep 1.612
noteoff 3 49 0
noteon 1 74 100
noteon 11 62 102
sleep 3.225
noteoff 14 37 0
sleep 3.224
noteon 12 62 102
sleep 8.063
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 3.224
noteon 14 38 106
sleep 4.837
noteoff 0 74 0
noteon 0 73 101
sleep 4.836
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 6.448
noteoff 12 62 0
noteon 12 61 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 1.612
noteoff 0 73 0
noteon 0 74 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.450
noteoff 12 61 0
noteon 12 62 102
sleep 4.836
noteoff 0 74 0
noteon 0 73 101
sleep 8.062
noteoff 14 38 0
noteon 14 37 106
sleep 9.674
noteoff 10 62 0
noteon 10 61 102
sleep 3.224
noteoff 1 74 0
noteoff 11 61 0
noteon 1 73 100
noteon 11 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 8.062
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 12.898
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 1 73 0
noteon 1 74 100
sleep 6.448
noteoff 12 61 0
noteon 12 62 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 4.836
noteoff 0 73 0
noteon 0 74 101
sleep 4.838
noteoff 10 62 0
noteon 10 61 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 74 0
noteoff 3 50 0
noteon 1 73 100
noteon 3 49 100
sleep 6.450
noteoff 12 62 0
noteon 12 61 102
sleep 11.286
noteoff 0 74 0
noteon 0 73 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 3.224
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 6.449
noteoff 12 61 0
noteon 12 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 3.224
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.224
noteoff 14 38 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 4.838
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 9.677
noteoff 11 62 0
noteon 11 61 102
sleep 12.903
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 73 0
noteoff 13 50 0
noteon 0 74 101
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.451
noteoff 12 61 0
noteon 12 62 102
sleep 12.903
noteoff 14 37 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 59 102
sleep 1.612
noteoff 0 74 0
noteon 0 71 101
sleep 1.612
noteoff 1 74 0
noteoff 11 61 0
noteon 1 71 100
noteon 11 59 102
sleep 3.225
noteoff 14 37 0
sleep 3.225
noteoff 12 62 0
noteon 12 59 102
sleep 8.064
noteoff 13 49 0
noteon 13 47 104
sleep 1.612
noteoff 3 49 0
noteon 3 47 100
sleep 3.225
noteon 14 35 106
sleep 25.806
noteoff 10 59 0
noteon 10 61 102
sleep 1.612
noteoff 0 71 0
noteon 0 73 101
sleep 1.612
noteoff 1 71 0
noteoff 11 59 0
noteon 1 73 100
noteon 11 61 102
sleep 6.451
noteoff 12 59 0
noteon 12 61 102
sleep 8.064
noteoff 13 47 0
noteon 13 49 104
sleep 1.612
noteoff 3 47 0
noteon 3 49 100
sleep 3.225
noteoff 14 35 0
noteon 14 37 106
sleep 25.806
echo "92160 tempo_s=278 tempo_l=0.25"
noteoff 10 61 0
noteon 10 64 102
sleep 1.798
noteoff 0 73 0
noteon 0 76 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 76 100
noteon 4 57 100
noteon 11 64 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 64 101
noteon 2 76 101
sleep 1.798
noteoff 12 61 0
noteon 5 45 100
noteon 12 64 102
sleep 8.992
noteoff 13 49 0
noteon 13 52 104
sleep 1.798
noteoff 3 49 0
noteon 3 52 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteoff 14 37 0
noteon 14 40 106
sleep 82.733
noteoff 10 64 0
sleep 1.798
noteoff 0 76 0
sleep 1.798
noteoff 1 76 0
noteoff 4 57 0
noteoff 11 64 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 76 0
noteoff 2 64 0
sleep 1.798
noteoff 5 45 0
noteoff 12 64 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 40 0
sleep 82.733
noteon 10 57 102
sleep 1.798
noteon 0 69 101
sleep 1.798
noteon 1 69 100
noteon 4 57 100
noteon 11 57 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 69 101
noteon 2 57 101
sleep 1.798
noteon 5 45 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 82.733
noteoff 10 57 0
sleep 1.798
noteoff 0 69 0
sleep 1.798
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 57 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 57 0
noteoff 2 69 0
sleep 1.798
noteoff 5 45 0
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 82.733
echo "92400 tempo_s=310 tempo_l=0.25"
sleep 193.548
noteon 10 69 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteon 12 67 102
sleep 8.064
noteon 13 67 104
sleep 30.645
noteoff 10 69 0
sleep 48.387
noteon 10 69 102
sleep 48.387
noteoff 10 69 0
sleep 48.387
echo "92640 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 10.791
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 67 0
noteon 13 66 104
sleep 88.129
noteoff 10 74 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 17.985
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
echo "92880 tempo_s=310 tempo_l=0.25"
noteon 10 76 102
sleep 9.677
noteoff 12 66 0
noteon 12 61 102
sleep 8.064
noteoff 13 66 0
noteon 13 61 104
sleep 79.032
noteoff 10 76 0
noteon 10 69 102
sleep 80.645
noteoff 10 69 0
sleep 16.129
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 48.387
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 41.935
noteoff 12 61 0
sleep 6.451
echo "93120 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 1.798
noteoff 13 61 0
sleep 8.992
noteon 12 62 102
sleep 8.992
noteon 13 62 104
sleep 88.129
noteoff 10 78 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 3.597
noteoff 11 69 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
echo "93360 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 3.225
noteon 11 71 102
sleep 6.451
noteoff 12 62 0
noteon 12 67 102
sleep 8.064
noteoff 13 62 0
noteon 13 55 104
sleep 4.838
noteon 14 31 106
sleep 74.193
noteoff 10 81 0
noteon 10 79 102
sleep 80.645
noteoff 10 79 0
sleep 16.129
noteon 10 78 102
sleep 22.58
noteoff 14 31 0
sleep 25.806
noteoff 10 78 0
sleep 48.387
noteon 10 76 102
sleep 48.387
noteoff 10 76 0
sleep 48.387
echo "93600 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 3.597
noteoff 11 71 0
noteon 11 69 102
sleep 7.194
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 55 0
noteon 13 57 104
sleep 5.395
noteon 14 33 106
sleep 82.733
noteoff 10 76 0
noteon 10 74 102
sleep 89.928
noteoff 10 74 0
sleep 17.985
noteon 10 74 102
sleep 25.179
noteoff 14 33 0
sleep 28.776
noteoff 10 74 0
sleep 53.956
noteon 10 74 102
sleep 53.956
noteoff 10 74 0
sleep 53.956
echo "93840 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 3.225
noteoff 11 69 0
noteon 4 64 115
noteon 11 67 102
sleep 4.838
noteon 2 67 116
noteon 2 73 116
sleep 1.612
noteoff 12 66 0
noteon 5 57 115
noteon 12 64 102
sleep 8.064
noteoff 13 57 0
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 74.193
noteoff 10 74 0
noteon 10 73 102
sleep 80.645
noteoff 10 73 0
sleep 16.129
noteon 10 71 102
sleep 22.58
noteoff 14 33 0
sleep 25.806
noteoff 10 71 0
sleep 48.387
echo "94020 tempo_s=235 tempo_l=0.25"
noteon 10 73 102
sleep 63.829
noteoff 10 73 0
sleep 46.808
noteoff 11 67 0
sleep 8.51
noteoff 12 64 0
sleep 8.51
echo "94080 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
noteon 10 62 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteoff 4 64 0
noteon 11 66 102
noteon 4 62 100
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteoff 2 73 0
noteoff 2 67 0
noteon 2 74 101
noteon 2 66 101
sleep 1.798
noteoff 5 57 0
noteon 12 62 102
noteon 5 50 100
sleep 8.992
noteon 13 50 104
sleep 3.597
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 190.647
noteoff 10 62 0
noteoff 10 74 0
sleep 3.597
noteoff 11 66 0
sleep 5.395
noteoff 2 66 0
noteoff 2 74 0
sleep 1.798
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 3.597
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 84.532
noteon 0 85 101
sleep 1.798
noteon 1 73 100
sleep 17.985
noteon 3 61 100
sleep 86.33
echo "94320 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 0 85 0
noteon 0 86 101
sleep 1.612
noteoff 1 73 0
noteon 1 74 100
sleep 16.129
noteoff 3 61 0
noteon 3 62 100
sleep 79.032
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
sleep 16.129
noteoff 3 62 0
sleep 174.193
noteon 10 62 102
sleep 3.225
noteon 11 62 102
sleep 6.451
noteon 12 54 102
sleep 8.064
noteon 13 54 104
sleep 4.838
noteon 14 42 106
sleep 25.806
noteoff 10 62 0
sleep 3.225
noteoff 11 62 0
sleep 6.451
noteoff 12 54 0
sleep 8.064
noteoff 13 54 0
sleep 4.838
noteoff 14 42 0
sleep 25.806
echo "94560 tempo_s=278 tempo_l=0.25"
noteon 10 64 102
sleep 3.597
noteon 11 64 102
sleep 7.194
noteon 12 55 102
sleep 8.992
noteon 13 55 104
sleep 5.395
noteon 14 43 106
sleep 82.733
noteoff 10 64 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 55 0
sleep 8.992
noteoff 13 55 0
sleep 5.395
noteoff 14 43 0
sleep 82.733
noteon 10 66 102
sleep 3.597
noteon 11 66 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 82.733
noteoff 10 66 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 5.395
noteoff 14 45 0
sleep 82.733
echo "94800 tempo_s=310 tempo_l=0.25"
noteon 10 67 102
sleep 3.225
noteon 11 67 102
sleep 6.451
noteon 12 59 102
sleep 8.064
noteon 13 59 104
sleep 4.838
noteon 14 47 106
sleep 74.193
noteoff 10 67 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 59 0
sleep 8.064
noteoff 13 59 0
sleep 4.838
noteoff 14 47 0
sleep 74.193
noteon 10 65 102
sleep 3.225
noteon 11 65 102
sleep 6.451
noteon 12 56 102
sleep 8.064
noteon 13 56 104
sleep 4.838
noteon 14 44 106
sleep 74.193
noteoff 10 65 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 56 0
sleep 8.064
noteoff 13 56 0
sleep 4.838
noteoff 14 44 0
sleep 74.193
echo "95040 tempo_s=278 tempo_l=0.25"
noteon 10 66 102
sleep 3.597
noteoff 4 62 0
noteon 11 66 102
noteon 4 62 100
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 5.395
noteoff 5 50 0
noteon 12 57 102
noteon 5 50 100
sleep 8.992
noteon 13 57 104
sleep 3.597
noteon 15 50 90
sleep 1.798
noteon 14 45 106
sleep 190.605
noteoff 10 66 0
sleep 3.596
noteoff 11 66 0
sleep 7.192
noteoff 12 57 0
sleep 8.991
noteoff 13 57 0
sleep 3.597
noteoff 15 50 0
sleep 1.798
noteoff 14 45 0
sleep 84.525
noteon 0 85 101
sleep 1.798
noteon 1 73 100
sleep 17.984
noteon 3 61 100
sleep 86.324
echo "95280 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteoff 0 85 0
noteon 0 86 101
sleep 1.612
noteoff 1 73 0
noteon 1 74 100
sleep 16.127
noteoff 3 61 0
noteon 3 62 100
sleep 79.032
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
sleep 16.129
noteoff 3 62 0
sleep 174.193
noteon 10 66 102
sleep 3.225
noteon 11 66 102
sleep 6.451
noteon 12 58 102
sleep 8.064
noteon 13 58 104
sleep 4.838
noteon 14 46 106
sleep 25.806
noteoff 10 66 0
sleep 3.225
noteoff 11 66 0
sleep 6.451
noteoff 12 58 0
sleep 8.064
noteoff 13 58 0
sleep 4.838
noteoff 14 46 0
sleep 25.806
echo "95520 tempo_s=278 tempo_l=0.25"
noteon 10 67 102
sleep 3.597
noteon 11 67 102
sleep 7.194
noteon 12 59 102
sleep 8.992
noteon 13 59 104
sleep 5.395
noteon 14 47 106
sleep 82.733
noteoff 10 67 0
sleep 3.597
noteoff 11 67 0
sleep 7.194
noteoff 12 59 0
sleep 8.992
noteoff 13 59 0
sleep 5.395
noteoff 14 47 0
sleep 82.733
noteon 10 69 102
sleep 3.597
noteon 11 57 102
sleep 7.194
noteon 12 54 102
sleep 8.992
noteon 13 54 104
sleep 5.395
noteon 14 42 106
sleep 82.733
noteoff 10 69 0
sleep 3.597
noteoff 11 57 0
sleep 7.194
noteoff 12 54 0
sleep 8.992
noteoff 13 54 0
sleep 5.395
noteoff 14 42 0
sleep 82.733
echo "95760 tempo_s=310 tempo_l=0.25"
noteon 10 71 102
sleep 3.225
noteon 11 59 102
sleep 6.451
noteon 12 55 102
sleep 8.064
noteon 13 55 104
sleep 4.838
noteon 14 43 106
sleep 74.193
noteoff 10 71 0
sleep 3.225
noteoff 11 59 0
sleep 6.451
noteoff 12 55 0
sleep 8.064
noteoff 13 55 0
sleep 4.838
noteoff 14 43 0
sleep 74.193
noteon 10 73 102
sleep 3.225
noteon 11 61 102
sleep 6.451
noteon 12 52 102
sleep 8.064
noteon 13 52 104
sleep 4.838
noteon 14 40 106
sleep 74.193
noteoff 10 73 0
sleep 3.225
noteoff 11 61 0
sleep 6.451
noteoff 12 52 0
sleep 8.064
noteoff 13 52 0
sleep 4.838
noteoff 14 40 0
sleep 74.193
echo "96000 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteon 0 86 101
sleep 1.798
noteoff 4 62 0
noteon 1 74 100
noteon 11 62 102
noteon 4 62 100
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 5.395
noteoff 5 50 0
noteon 12 50 102
noteon 5 50 100
sleep 8.991
noteon 13 50 104
sleep 1.798
noteon 3 62 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteon 14 38 106
sleep 84.510
noteoff 0 86 0
noteon 0 85 101
sleep 1.798
noteoff 1 74 0
noteon 1 73 100
sleep 17.981
noteoff 3 62 0
noteon 3 61 100
sleep 86.308
noteoff 10 74 0
sleep 1.798
noteoff 0 85 0
noteon 0 86 101
sleep 1.798
noteoff 1 73 0
noteoff 11 62 0
noteon 1 74 100
sleep 7.192
noteoff 12 50 0
sleep 8.991
noteoff 13 50 0
sleep 1.798
noteoff 3 61 0
noteon 3 62 100
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 84.525
noteoff 0 86 0
noteon 0 85 101
sleep 1.798
noteoff 1 74 0
noteon 1 73 100
sleep 17.984
noteoff 3 62 0
noteon 3 61 100
sleep 79.130
noteoff 0 85 0
sleep 1.798
noteoff 1 73 0
sleep 5.395
echo "96240 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 86 101
sleep 1.612
noteon 1 74 100
sleep 8.062
noteoff 3 61 0
sleep 8.064
noteon 3 62 100
sleep 79.032
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
sleep 16.129
noteoff 3 62 0
sleep 77.419
noteon 10 74 102
sleep 3.225
noteon 11 62 102
sleep 6.451
noteon 12 54 102
sleep 8.064
noteon 13 54 104
sleep 4.838
noteon 14 42 106
sleep 58.064
noteoff 10 74 0
sleep 3.225
noteoff 11 62 0
sleep 12.903
noteon 10 74 102
sleep 3.225
noteon 11 62 102
sleep 6.451
noteoff 12 54 0
sleep 8.064
noteoff 13 54 0
sleep 4.838
noteoff 14 42 0
sleep 58.064
noteoff 10 74 0
sleep 3.225
noteoff 11 62 0
sleep 12.903
echo "96480 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 3.597
noteon 11 64 102
sleep 7.194
noteon 12 55 102
sleep 8.992
noteon 13 55 104
sleep 5.395
noteon 14 43 106
sleep 64.748
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
sleep 14.388
noteon 10 76 102
sleep 3.597
noteon 11 64 102
sleep 7.194
noteoff 12 55 0
sleep 8.992
noteoff 13 55 0
sleep 5.395
noteoff 14 43 0
sleep 64.748
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
sleep 14.388
noteon 10 78 102
sleep 3.597
noteon 11 66 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 14.388
noteon 10 78 102
sleep 3.597
noteon 11 66 102
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 5.395
noteoff 14 45 0
sleep 64.748
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 14.388
echo "96720 tempo_s=310 tempo_l=0.25"
noteon 10 79 102
sleep 3.225
noteon 11 67 102
sleep 6.451
noteon 12 59 102
sleep 8.064
noteon 13 59 104
sleep 4.838
noteon 14 47 106
sleep 58.064
noteoff 10 79 0
sleep 3.225
noteoff 11 67 0
sleep 12.903
noteon 10 79 102
sleep 3.225
noteon 11 67 102
sleep 6.451
noteoff 12 59 0
sleep 8.064
noteoff 13 59 0
sleep 4.838
noteoff 14 47 0
sleep 58.064
noteoff 10 79 0
sleep 3.225
noteoff 11 67 0
sleep 12.903
noteon 10 77 102
sleep 3.225
noteon 11 65 102
sleep 6.451
noteon 12 56 102
sleep 8.064
noteon 13 56 104
sleep 4.838
noteon 14 44 106
sleep 58.064
noteoff 10 77 0
sleep 3.225
noteoff 11 65 0
sleep 12.903
noteon 10 77 102
sleep 3.225
noteon 11 65 102
sleep 6.451
noteoff 12 56 0
sleep 8.064
noteoff 13 56 0
sleep 4.838
noteoff 14 44 0
sleep 58.064
noteoff 10 77 0
sleep 3.225
noteoff 11 65 0
sleep 12.903
echo "96960 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 1.798
noteon 0 86 101
sleep 1.798
noteoff 4 62 0
noteon 1 74 100
noteon 11 66 102
noteon 4 62 100
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 5.395
noteoff 5 50 0
noteon 12 57 102
noteon 5 50 100
sleep 8.991
noteon 13 57 104
sleep 1.798
noteon 3 62 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteon 14 45 106
sleep 84.510
noteoff 0 86 0
noteon 0 85 101
sleep 1.798
noteoff 1 74 0
noteon 1 73 100
sleep 17.981
noteoff 3 62 0
noteon 3 61 100
sleep 86.308
noteoff 10 78 0
sleep 1.798
noteoff 0 85 0
noteon 0 86 101
sleep 1.798
noteoff 1 73 0
noteoff 11 66 0
noteon 1 74 100
sleep 7.192
noteoff 12 57 0
sleep 8.991
noteoff 13 57 0
sleep 1.798
noteoff 3 61 0
noteon 3 62 100
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 45 0
sleep 84.525
noteoff 0 86 0
noteon 0 85 101
sleep 1.798
noteoff 1 74 0
noteon 1 73 100
sleep 17.984
noteoff 3 62 0
noteon 3 61 100
sleep 79.130
noteoff 0 85 0
sleep 1.798
noteoff 1 73 0
sleep 5.395
echo "97200 tempo_s=310 tempo_l=0.25"
sleep 1.612
noteon 0 86 101
sleep 1.612
noteon 1 74 100
sleep 8.062
noteoff 3 61 0
sleep 8.064
noteon 3 62 100
sleep 79.032
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
sleep 16.129
noteoff 3 62 0
sleep 77.419
noteon 10 78 102
sleep 1.612
noteon 0 86 101
noteon 0 78 101
sleep 1.612
noteoff 4 62 0
noteon 1 78 100
noteon 1 74 100
noteon 4 62 100
noteon 11 66 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
noteon 6 74 108
noteon 6 62 108
sleep 3.225
noteon 2 74 101
noteon 2 66 101
sleep 1.612
noteoff 5 50 0
noteon 5 50 100
noteon 12 60 102
sleep 8.064
noteon 13 60 104
sleep 1.612
noteon 3 48 100
noteon 3 60 100
sleep 1.612
noteon 15 50 80
sleep 1.612
noteon 14 48 106
sleep 58.064
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 6.451
noteoff 12 60 0
sleep 6.451
noteon 10 78 102
sleep 1.612
noteoff 0 78 0
noteoff 0 86 0
noteoff 15 50 0
sleep 1.612
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 62 0
noteon 11 66 102
sleep 1.612
noteoff 6 62 0
noteoff 6 74 0
sleep 3.225
noteoff 2 66 0
noteoff 2 74 0
sleep 1.612
noteoff 5 50 0
noteon 12 60 102
sleep 8.064
noteoff 13 60 0
sleep 1.612
noteoff 3 60 0
noteoff 3 48 0
sleep 1.612
noteon 15 50 90
sleep 1.612
noteoff 14 48 0
sleep 58.064
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 6.451
noteoff 12 60 0
sleep 6.451
echo "97440 tempo_s=278 tempo_l=0.25"
noteon 10 79 102
sleep 1.798
noteoff 15 50 0
noteon 0 86 101
noteon 0 79 101
sleep 1.798
noteon 1 79 100
noteon 1 74 100
noteon 4 62 100
noteon 11 67 102
sleep 1.798
noteon 6 74 108
noteon 6 62 108
sleep 3.597
noteon 2 74 101
noteon 2 67 101
sleep 1.798
noteon 5 50 100
noteon 12 59 102
sleep 8.992
noteon 13 59 104
sleep 1.798
noteon 3 59 100
noteon 3 47 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteon 14 47 106
sleep 64.748
noteoff 10 79 0
sleep 3.597
noteoff 11 67 0
sleep 7.194
noteoff 12 59 0
sleep 7.194
noteon 10 79 102
sleep 1.798
noteoff 0 79 0
noteoff 0 86 0
noteoff 15 50 0
sleep 1.798
noteoff 1 74 0
noteoff 1 79 0
noteoff 4 62 0
noteon 11 67 102
sleep 1.798
noteoff 6 62 0
noteoff 6 74 0
sleep 3.597
noteoff 2 67 0
noteoff 2 74 0
sleep 1.798
noteoff 5 50 0
noteon 12 59 102
sleep 8.992
noteoff 13 59 0
sleep 1.798
noteoff 3 47 0
noteoff 3 59 0
sleep 1.798
noteon 15 50 90
sleep 1.798
noteoff 14 47 0
sleep 64.748
noteoff 10 79 0
sleep 3.597
noteoff 11 67 0
sleep 7.194
noteoff 12 59 0
sleep 7.194
noteon 10 78 102
sleep 1.798
noteoff 15 50 0
noteon 0 86 101
noteon 0 78 101
sleep 1.798
noteon 1 78 100
noteon 1 74 100
noteon 4 62 100
noteon 11 66 102
sleep 1.798
noteon 6 74 108
noteon 6 62 108
sleep 3.597
noteon 2 66 101
noteon 2 74 101
sleep 1.798
noteon 5 50 100
noteon 12 60 102
sleep 8.992
noteon 13 60 104
sleep 1.798
noteon 3 60 100
noteon 3 48 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteon 14 48 106
sleep 64.748
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 60 0
sleep 7.194
noteon 10 78 102
sleep 1.798
noteoff 0 78 0
noteoff 0 86 0
noteoff 15 50 0
sleep 1.798
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 62 0
noteon 11 66 102
sleep 1.798
noteoff 6 62 0
noteoff 6 74 0
sleep 3.597
noteoff 2 74 0
noteoff 2 66 0
sleep 1.798
noteoff 5 50 0
noteon 12 60 102
sleep 8.992
noteoff 13 60 0
sleep 1.798
noteoff 3 48 0
noteoff 3 60 0
sleep 1.798
noteon 15 50 90
sleep 1.798
noteoff 14 48 0
sleep 64.748
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 60 0
sleep 7.194
echo "97680 tempo_s=310 tempo_l=0.25"
noteon 10 79 102
sleep 1.612
noteoff 15 50 0
noteon 0 86 101
noteon 0 79 101
sleep 1.612
noteon 1 79 100
noteon 1 74 100
noteon 4 62 100
noteon 11 67 102
sleep 1.612
noteon 6 74 108
noteon 6 62 108
sleep 3.225
noteon 2 74 101
noteon 2 67 101
sleep 1.612
noteon 5 50 100
noteon 12 59 102
sleep 8.064
noteon 13 59 104
sleep 1.612
noteon 3 47 100
noteon 3 59 100
sleep 1.612
noteon 15 50 90
sleep 1.612
noteon 14 47 106
sleep 58.064
noteoff 10 79 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 59 0
sleep 6.451
noteon 10 79 102
sleep 1.612
noteoff 0 79 0
noteoff 0 86 0
noteoff 15 50 0
sleep 1.612
noteoff 1 74 0
noteoff 1 79 0
noteoff 4 62 0
noteon 11 67 102
sleep 1.612
noteoff 6 62 0
noteoff 6 74 0
sleep 3.225
noteoff 2 67 0
noteoff 2 74 0
sleep 1.612
noteoff 5 50 0
noteon 12 59 102
sleep 8.064
noteoff 13 59 0
sleep 1.612
noteoff 3 59 0
noteoff 3 47 0
sleep 1.612
noteon 15 50 90
sleep 1.612
noteoff 14 47 0
sleep 58.064
noteoff 10 79 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 59 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 15 50 0
noteon 0 86 101
noteon 0 80 101
sleep 1.612
noteon 1 80 100
noteon 1 74 100
noteon 4 62 100
noteon 11 68 102
sleep 1.612
noteon 6 74 108
noteon 6 62 108
sleep 3.225
noteon 2 74 101
noteon 2 68 101
sleep 1.612
noteon 5 50 100
noteon 12 58 102
sleep 8.064
noteon 13 58 104
sleep 1.612
noteon 3 46 100
noteon 3 58 100
sleep 1.612
noteon 15 50 90
sleep 1.612
noteon 14 46 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 68 0
sleep 6.451
noteoff 12 58 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 0 80 0
noteoff 0 86 0
noteoff 15 50 0
sleep 1.612
noteoff 1 74 0
noteoff 1 80 0
noteoff 4 62 0
noteon 11 68 102
sleep 1.612
noteoff 6 62 0
noteoff 6 74 0
sleep 3.224
noteoff 2 68 0
noteoff 2 74 0
sleep 1.612
noteoff 5 50 0
noteon 12 58 102
sleep 8.062
noteoff 13 58 0
sleep 1.612
noteoff 3 58 0
noteoff 3 46 0
sleep 1.612
noteon 15 50 90
sleep 1.612
noteoff 14 46 0
sleep 58.041
noteoff 10 80 0
sleep 3.224
noteoff 11 68 0
sleep 6.449
noteoff 12 58 0
sleep 6.449
echo "97920 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
sleep 1.798
noteoff 15 50 0
noteon 0 81 101
noteon 0 85 101
sleep 1.798
noteon 1 81 100
noteon 1 73 100
noteon 4 69 100
noteon 11 69 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 73 101
noteon 2 69 101
sleep 1.798
noteon 5 57 100
noteon 12 57 102
sleep 8.991
noteon 13 45 104
sleep 1.798
noteon 3 45 100
noteon 3 57 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 33 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 64 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 0 85 0
noteoff 0 81 0
noteoff 13 57 0
sleep 1.798
noteoff 1 73 0
noteoff 1 81 0
noteoff 4 69 0
noteon 11 64 102
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 1.798
noteoff 14 45 0
sleep 1.798
noteoff 2 69 0
noteoff 2 73 0
sleep 1.798
noteoff 5 57 0
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 57 0
noteoff 3 45 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 64 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
echo "98160 tempo_s=310 tempo_l=0.25"
noteon 10 85 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 1 79 100
noteon 1 76 100
noteon 11 67 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 1.612
noteon 3 67 100
noteon 3 64 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 85 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 67 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 67 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 67 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
echo "98400 tempo_s=278 tempo_l=0.25"
noteon 10 86 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteoff 1 76 0
noteoff 1 79 0
noteon 1 74 100
noteon 1 78 100
noteon 11 66 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 64 0
noteoff 3 67 0
noteon 3 62 100
noteon 3 66 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 86 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 66 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 66 102
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 3.597
noteoff 15 45 0
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 66 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 66 0
sleep 7.194
noteoff 12 62 0
sleep 1.798
noteoff 1 74 0
sleep 5.395
echo "98640 tempo_s=310 tempo_l=0.25"
noteon 10 86 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteoff 1 78 0
noteon 1 77 100
noteon 1 74 100
noteon 11 65 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 1.612
noteoff 3 62 0
sleep 6.451
noteon 13 57 104
sleep 1.612
noteoff 3 66 0
noteon 3 62 100
noteon 3 65 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 86 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 65 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 65 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 65 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
echo "98880 tempo_s=278 tempo_l=0.25"
noteon 10 83 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteoff 1 74 0
noteoff 1 77 0
noteon 1 73 100
noteon 1 76 100
noteon 4 69 100
noteon 11 64 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 5 57 100
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 65 0
noteoff 3 62 0
noteon 3 61 100
noteon 3 64 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 83 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 76 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 76 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 76 102
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 3.597
noteoff 15 45 0
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 76 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 76 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 61 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 76 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
echo "99120 tempo_s=310 tempo_l=0.25"
noteon 10 85 102
sleep 1.612
noteoff 13 57 0
noteon 0 85 101
noteon 0 81 101
sleep 1.612
noteoff 1 76 0
noteoff 1 73 0
noteon 1 79 100
noteon 1 76 100
noteon 11 79 102
sleep 3.225
noteoff 14 45 0
sleep 1.612
noteon 2 81 101
noteon 2 69 101
sleep 1.612
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 1.612
noteoff 3 64 0
noteoff 3 61 0
noteon 3 67 100
noteon 3 64 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 85 0
sleep 3.225
noteoff 11 79 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 79 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 79 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
noteon 10 81 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 79 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 64 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 79 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
echo "99360 tempo_s=278 tempo_l=0.25"
noteon 10 86 102
sleep 1.798
noteoff 0 85 0
noteoff 13 57 0
noteon 0 86 101
sleep 1.798
noteoff 1 76 0
noteoff 1 79 0
noteon 1 78 100
noteon 1 74 100
noteon 11 78 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 64 0
noteoff 3 67 0
noteon 3 62 100
noteon 3 66 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 86 0
sleep 3.597
noteoff 11 78 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 78 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 78 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 78 102
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 3.597
noteoff 15 45 0
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 78 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 11 78 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 62 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 78 0
sleep 7.194
noteoff 0 86 0
noteoff 12 62 0
sleep 1.798
noteoff 1 74 0
sleep 5.395
echo "99600 tempo_s=310 tempo_l=0.25"
noteon 10 86 102
sleep 1.612
noteoff 0 81 0
noteoff 13 57 0
noteon 0 80 101
noteon 0 86 101
sleep 1.612
noteoff 1 78 0
noteon 1 74 100
noteon 1 77 100
noteon 11 77 102
sleep 3.225
noteoff 14 45 0
sleep 1.612
noteoff 2 69 0
noteoff 2 81 0
noteon 2 68 101
noteon 2 80 101
sleep 1.612
noteon 12 62 102
sleep 1.612
noteoff 3 62 0
sleep 6.451
noteon 13 57 104
sleep 1.612
noteoff 3 66 0
noteon 3 62 100
noteon 3 65 100
sleep 3.225
noteon 14 45 106
sleep 58.064
noteoff 10 86 0
sleep 3.225
noteoff 11 77 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 77 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 77 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 77 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 77 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
noteon 10 80 102
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteon 11 77 102
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteon 12 62 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 80 0
sleep 3.225
noteoff 11 77 0
sleep 6.451
noteoff 12 62 0
sleep 6.451
echo "99840 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
sleep 1.798
noteoff 0 86 0
noteoff 0 80 0
noteoff 13 57 0
noteon 0 85 101
noteon 0 81 101
sleep 1.798
noteoff 1 77 0
noteoff 1 74 0
noteon 1 73 100
noteon 1 76 100
noteon 11 76 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 1.798
noteoff 14 45 0
sleep 1.798
noteoff 2 80 0
noteoff 2 68 0
noteon 2 81 101
noteon 2 69 101
sleep 1.798
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 65 0
noteoff 3 62 0
noteon 3 61 100
noteon 3 64 100
sleep 1.798
noteon 15 45 68
sleep 1.798
noteon 14 45 106
sleep 111.51
noteoff 10 81 0
sleep 1.798
noteoff 0 81 0
noteoff 0 85 0
sleep 1.798
noteoff 1 76 0
noteoff 1 73 0
noteoff 11 76 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 69 0
noteoff 2 81 0
sleep 1.798
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 1.798
noteoff 3 64 0
noteoff 3 61 0
sleep 3.597
noteoff 14 45 0
sleep 12.589
noteoff 4 69 0
sleep 7.194
noteoff 5 57 0
sleep 14.388
noteoff 15 45 0
sleep 19.784
noteon 10 57 102
sleep 1.798
noteon 0 69 101
noteon 0 81 101
sleep 1.798
noteon 1 69 100
noteon 4 57 100
noteon 11 57 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 57 101
noteon 2 69 101
sleep 1.798
noteon 5 57 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
noteon 3 57 100
sleep 1.798
noteon 15 45 68
sleep 1.798
noteon 14 33 106
sleep 111.51
noteoff 10 57 0
sleep 1.798
noteoff 0 81 0
noteoff 0 69 0
sleep 1.798
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 57 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 69 0
noteoff 2 57 0
sleep 1.798
noteoff 5 57 0
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 57 0
noteoff 3 45 0
sleep 3.597
noteoff 14 33 0
sleep 34.172
noteoff 15 45 0
sleep 19.784
echo "100080 tempo_s=299 tempo_l=0.25"
noteon 10 57 102
sleep 1.672
noteon 0 69 101
noteon 0 81 101
sleep 1.672
noteon 1 69 100
noteon 4 57 100
noteon 11 57 102
sleep 1.672
noteon 6 57 108
noteon 6 69 108
sleep 3.344
noteon 2 57 101
noteon 2 69 101
sleep 1.672
noteon 5 57 100
noteon 12 57 102
sleep 8.361
noteon 13 45 104
sleep 1.672
noteon 3 45 100
noteon 3 57 100
sleep 1.672
noteon 15 45 68
sleep 1.672
noteon 14 33 106
sleep 103.678
noteoff 10 57 0
sleep 1.672
noteoff 0 81 0
noteoff 0 69 0
sleep 1.672
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 57 0
sleep 1.672
noteoff 6 69 0
noteoff 6 57 0
sleep 3.344
noteoff 2 69 0
noteoff 2 57 0
sleep 1.672
noteoff 5 57 0
noteoff 12 57 0
sleep 8.361
noteoff 13 45 0
sleep 1.672
noteoff 3 57 0
noteoff 3 45 0
sleep 3.344
noteoff 14 33 0
sleep 31.772
noteoff 15 45 0
sleep 18.394
echo "100200 tempo_s=278 tempo_l=0.25"
noteon 10 57 102
sleep 1.798
noteon 0 69 101
noteon 0 81 101
sleep 1.798
noteon 1 69 100
noteon 4 57 100
noteon 11 57 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 69 101
noteon 2 57 101
sleep 1.798
noteon 5 57 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
noteon 3 57 100
sleep 1.798
noteon 15 45 68
sleep 1.798
noteon 14 33 106
sleep 111.51
noteoff 10 57 0
sleep 1.798
noteoff 0 81 0
noteoff 0 69 0
sleep 1.798
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 57 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 57 0
noteoff 2 69 0
sleep 1.798
noteoff 5 57 0
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 57 0
noteoff 3 45 0
sleep 3.597
noteoff 14 33 0
sleep 34.172
noteoff 15 45 0
sleep 19.784
echo "100320 tempo_s=257 tempo_l=0.25"
noteon 10 57 102
sleep 1.945
noteon 0 81 101
noteon 0 69 101
sleep 1.945
noteon 1 69 100
noteon 4 57 100
noteon 11 57 102
sleep 1.945
noteon 6 57 108
noteon 6 69 108
sleep 3.891
noteon 2 69 101
noteon 2 57 101
sleep 1.945
noteon 5 57 100
noteon 12 57 102
sleep 9.727
noteon 13 45 104
sleep 1.945
noteon 3 57 100
noteon 3 45 100
sleep 1.945
noteon 15 45 68
sleep 1.945
noteon 14 33 106
sleep 206.225
noteoff 10 57 0
sleep 1.945
noteoff 0 69 0
noteoff 0 81 0
sleep 1.945
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 57 0
sleep 1.945
noteoff 6 69 0
noteoff 6 57 0
sleep 3.891
noteoff 2 57 0
noteoff 2 69 0
sleep 1.945
noteoff 5 57 0
noteoff 12 57 0
sleep 9.727
noteoff 13 45 0
sleep 1.945
noteoff 3 45 0
noteoff 3 57 0
sleep 1.945
noteoff 15 45 0
sleep 1.945
noteoff 14 33 0
sleep 694.552
noteon 13 50 104
sleep 3.891
select 14 1 0 45
sleep 1.945
noteon 14 38 39
sleep 233.459
noteoff 14 38 0
sleep 206.225
echo "101040 tempo_s=283 tempo_l=0.25"
sleep 19.434
noteoff 13 50 0
noteon 13 52 104
sleep 212.014
noteoff 13 52 0
noteon 13 54 104
sleep 192.579
echo "101280 tempo_s=268 tempo_l=0.25"
noteon 10 69 102
sleep 1.865
noteoff 13 54 0
sleep 1.865
noteon 11 61 102
sleep 7.462
noteon 12 57 102
sleep 9.328
noteon 13 55 104
sleep 5.597
noteon 14 33 46
sleep 223.868
noteoff 14 33 0
sleep 197.751
echo "101520 tempo_s=281 tempo_l=0.25"
sleep 3.558
noteoff 11 61 0
noteon 11 62 102
sleep 7.117
noteoff 12 57 0
noteon 12 59 102
sleep 204.622
noteoff 13 55 0
sleep 1.779
noteoff 11 62 0
noteon 11 64 102
sleep 7.116
noteoff 12 59 0
noteon 12 61 102
sleep 8.896
noteon 13 55 104
sleep 179.711
noteoff 11 64 0
sleep 7.117
noteoff 12 61 0
sleep 7.117
echo "101760 tempo_s=268 tempo_l=0.25"
sleep 1.865
noteoff 13 55 0
sleep 1.865
noteon 11 66 102
sleep 7.462
noteon 12 62 102
sleep 9.328
noteon 13 55 104
sleep 5.597
noteon 14 38 46
sleep 218.283
noteoff 13 55 0
noteon 13 54 104
sleep 5.597
noteoff 14 38 0
sleep 197.761
echo "102000 tempo_s=281 tempo_l=0.25"
sleep 19.572
noteoff 13 54 0
noteon 13 52 104
sleep 213.523
noteoff 13 52 0
noteon 13 50 104
sleep 179.715
noteoff 11 66 0
sleep 7.117
noteoff 12 62 0
sleep 7.117
echo "102240 tempo_s=268 tempo_l=0.25"
sleep 1.865
noteoff 13 50 0
sleep 1.865
noteon 11 61 102
sleep 7.462
noteon 12 57 102
sleep 9.328
noteon 13 55 104
sleep 5.597
noteon 14 33 46
sleep 223.88
noteoff 14 33 0
sleep 197.761
echo "102480 tempo_s=281 tempo_l=0.25"
sleep 3.558
noteoff 11 61 0
noteon 11 62 102
sleep 7.117
noteoff 12 57 0
noteon 12 59 102
sleep 204.626
noteoff 13 55 0
sleep 1.779
noteoff 11 62 0
noteon 11 64 102
sleep 7.117
noteoff 12 59 0
noteon 12 61 102
sleep 8.896
noteon 13 55 104
sleep 179.715
noteoff 11 64 0
sleep 7.117
noteoff 12 61 0
sleep 7.117
echo "102720 tempo_s=268 tempo_l=0.25"
sleep 1.865
noteoff 13 55 0
sleep 1.865
noteon 11 66 102
sleep 7.462
noteon 12 62 102
sleep 9.328
noteon 13 54 104
sleep 5.597
noteon 14 38 46
sleep 223.88
noteoff 14 38 0
sleep 182.835
noteoff 11 66 0
sleep 7.462
noteoff 12 62 0
sleep 7.462
echo "102960 tempo_s=281 tempo_l=0.25"
noteoff 10 69 0
noteon 10 67 102
sleep 3.558
noteon 11 74 102
sleep 7.117
noteon 12 62 102
sleep 8.896
noteoff 13 54 0
noteon 13 52 104
sleep 193.95
noteoff 10 67 0
noteon 10 66 102
sleep 19.572
noteoff 13 52 0
noteon 13 50 104
sleep 176.156
noteoff 10 66 0
sleep 17.793
echo "103200 tempo_s=268 tempo_l=0.25"
noteon 10 71 102
sleep 1.865
noteoff 13 50 0
sleep 18.656
noteon 13 56 104
sleep 412.313
noteoff 11 74 0
sleep 7.462
noteoff 12 62 0
sleep 7.462
echo "103440 tempo_s=281 tempo_l=0.25"
noteoff 10 71 0
noteon 10 69 102
sleep 3.558
noteon 11 74 102
sleep 7.117
noteon 12 62 102
sleep 8.896
noteoff 13 56 0
noteon 13 54 104
sleep 193.95
noteoff 10 69 0
noteon 10 68 102
sleep 19.572
noteoff 13 54 0
noteon 13 52 104
sleep 176.156
noteoff 10 68 0
sleep 17.793
echo "103680 tempo_s=268 tempo_l=0.25"
noteon 10 73 102
sleep 1.865
noteoff 13 52 0
sleep 1.865
noteon 4 64 100
sleep 5.597
noteon 2 69 101
sleep 1.865
noteon 5 64 100
sleep 9.328
noteon 13 57 104
sleep 1.865
noteon 3 57 100
sleep 3.731
noteon 14 33 46
sleep 197.755
noteoff 10 73 0
sleep 3.731
noteoff 11 74 0
noteon 11 73 102
sleep 7.462
noteoff 12 62 0
noteon 12 61 102
sleep 14.924
noteoff 14 33 0
sleep 197.756
echo "103920 tempo_s=281 tempo_l=0.25"
sleep 3.558
noteoff 11 73 0
noteon 11 71 102
sleep 5.338
noteoff 2 69 0
noteon 2 71 101
sleep 1.779
noteoff 12 61 0
noteon 12 59 102
sleep 10.676
noteoff 3 57 0
noteon 3 59 100
sleep 193.95
noteoff 13 57 0
sleep 1.779
noteoff 11 71 0
noteon 11 69 102
sleep 5.338
noteoff 2 71 0
noteon 2 73 101
sleep 1.779
noteoff 12 59 0
noteon 12 57 102
sleep 8.896
noteon 13 57 104
sleep 1.779
noteoff 3 59 0
noteon 3 61 100
sleep 177.935
noteoff 11 69 0
sleep 7.117
noteoff 12 57 0
sleep 7.117
echo "104160 tempo_s=268 tempo_l=0.25"
noteoff 2 73 0
noteon 10 76 102
sleep 1.865
noteoff 13 57 0
sleep 1.865
noteon 11 69 102
sleep 5.597
noteon 2 74 101
sleep 1.865
noteon 12 52 102
sleep 1.865
noteoff 3 61 0
sleep 7.462
noteon 13 57 104
sleep 1.865
noteon 3 62 100
sleep 205.223
noteoff 11 69 0
noteon 11 68 102
sleep 16.791
noteoff 13 57 0
noteon 13 56 104
sleep 203.358
echo "104400 tempo_s=281 tempo_l=0.25"
noteoff 10 76 0
noteon 10 78 102
sleep 3.558
noteoff 11 68 0
noteon 11 66 102
sleep 16.014
noteoff 13 56 0
noteon 13 54 104
sleep 193.95
noteoff 2 74 0
noteoff 10 78 0
noteon 10 80 102
sleep 3.558
noteoff 11 66 0
noteon 11 64 102
sleep 5.338
noteon 2 74 101
sleep 3.558
noteoff 3 62 0
sleep 7.117
noteoff 13 54 0
noteon 13 52 104
sleep 1.779
noteon 3 62 100
sleep 174.377
noteoff 10 80 0
sleep 3.558
noteoff 11 64 0
sleep 14.234
echo "104640 tempo_s=268 tempo_l=0.25"
noteoff 2 74 0
noteon 10 81 87
sleep 1.865
noteoff 13 52 0
sleep 1.865
noteon 11 69 102
sleep 5.597
noteon 2 74 101
sleep 3.731
noteoff 3 62 0
sleep 7.462
noteon 13 57 104
sleep 1.865
noteon 3 62 100
sleep 3.731
noteon 14 33 46
sleep 85.82
noteoff 10 81 0
sleep 121.268
noteoff 2 74 0
noteon 2 72 101
sleep 13.059
noteoff 3 62 0
noteon 3 60 100
sleep 3.731
noteoff 14 33 0
sleep 197.761
echo "104880 tempo_s=281 tempo_l=0.25"
sleep 8.896
noteoff 2 72 0
noteon 2 71 101
sleep 12.455
noteoff 3 60 0
noteon 3 59 100
sleep 177.935
noteoff 11 69 0
sleep 16.014
noteoff 13 57 0
sleep 1.779
noteon 11 69 102
sleep 5.338
noteoff 2 71 0
noteon 2 69 101
sleep 10.676
noteon 13 57 104
sleep 1.779
noteoff 3 59 0
noteon 3 57 100
sleep 177.935
noteoff 11 69 0
sleep 14.234
echo "105120 tempo_s=270 tempo_l=0.25"
noteoff 2 69 0
noteon 10 76 87
sleep 1.851
noteoff 13 57 0
sleep 1.851
noteon 11 69 102
sleep 5.555
noteon 2 74 101
sleep 3.703
noteoff 3 57 0
sleep 7.407
noteon 13 57 104
sleep 1.851
noteon 3 62 100
sleep 203.703
noteoff 11 69 0
noteon 11 68 102
sleep 16.666
noteoff 13 57 0
noteon 13 56 104
sleep 201.851
echo "105360 tempo_s=285 tempo_l=0.25"
noteoff 10 76 0
noteon 10 78 87
sleep 3.508
noteoff 11 68 0
noteon 11 66 102
sleep 15.789
noteoff 13 56 0
noteon 13 54 104
sleep 191.228
noteoff 2 74 0
noteoff 10 78 0
noteon 10 80 87
sleep 3.508
noteoff 11 66 0
noteon 11 64 102
sleep 5.263
noteon 2 74 101
sleep 3.508
noteoff 3 62 0
sleep 7.017
noteoff 13 54 0
noteon 13 52 104
sleep 1.754
noteon 3 62 100
sleep 171.929
noteoff 10 80 0
sleep 3.508
noteoff 11 64 0
sleep 14.035
echo "105600 tempo_s=269 tempo_l=0.25"
noteoff 2 74 0
noteon 10 81 74
sleep 1.858
noteoff 13 52 0
sleep 1.858
noteon 1 81 100
noteon 11 69 102
sleep 5.576
noteon 2 72 101
sleep 3.717
noteoff 3 62 0
sleep 7.434
noteon 13 57 104
sleep 1.858
noteon 3 62 100
sleep 3.717
noteon 14 33 46
sleep 85.499
noteoff 10 81 0
sleep 104.086
noteoff 12 52 0
sleep 7.434
echo "105720 tempo_s=256 tempo_l=0.25"
sleep 3.906
noteoff 4 64 0
noteoff 11 69 0
sleep 5.859
noteoff 2 72 0
sleep 1.953
noteoff 5 64 0
noteon 12 57 102
sleep 11.718
noteoff 3 62 0
noteon 3 60 100
sleep 3.906
noteoff 14 33 0
sleep 207.027
echo "105840 tempo_s=288 tempo_l=0.25"
sleep 3.472
noteon 1 69 100
sleep 6.944
noteoff 12 57 0
noteon 12 60 102
sleep 8.68
noteoff 13 57 0
noteon 13 60 104
sleep 1.736
noteoff 3 60 0
noteon 3 60 100
noteon 3 64 100
sleep 190.969
noteoff 1 69 0
noteoff 1 81 0
noteon 1 71 100
noteon 1 79 100
sleep 6.944
noteoff 12 60 0
noteon 12 57 102
sleep 8.68
noteoff 13 60 0
noteon 13 59 104
sleep 1.736
noteoff 3 64 0
noteoff 3 60 0
noteon 3 59 100
noteon 3 62 100
sleep 187.495
echo "106080 tempo_s=263 tempo_l=0.25"
sleep 3.802
noteoff 1 79 0
noteoff 1 71 0
noteon 1 72 100
noteon 1 78 100
sleep 7.604
noteoff 12 57 0
noteon 12 57 102
sleep 9.505
noteoff 13 59 0
noteon 13 57 104
sleep 1.901
noteoff 3 62 0
noteoff 3 59 0
noteon 3 60 100
noteon 3 57 100
sleep 209.120
noteoff 1 78 0
noteoff 1 72 0
noteon 1 73 100
noteon 1 76 100
sleep 7.604
noteoff 12 57 0
noteon 12 55 102
sleep 9.505
noteoff 13 57 0
noteon 13 55 104
sleep 1.901
noteoff 3 57 0
noteoff 3 60 0
noteon 3 55 100
noteon 3 58 100
sleep 205.319
echo "106320 tempo_s=288 tempo_l=0.25"
noteon 10 74 102
sleep 1.736
noteon 0 86 101
noteon 0 74 101
sleep 1.736
noteoff 1 76 0
noteoff 1 73 0
noteon 4 62 100
noteon 11 62 102
noteon 1 74 100
sleep 6.944
noteoff 12 55 0
noteon 5 50 100
noteon 12 54 102
sleep 8.680
noteoff 13 55 0
noteon 13 54 104
sleep 1.736
noteoff 3 58 0
noteoff 3 55 0
noteon 3 57 100
noteon 3 54 100
sleep 190.960
noteoff 1 74 0
noteon 1 74 100
noteon 1 72 100
sleep 6.944
noteoff 12 54 0
noteon 12 50 102
sleep 8.680
noteoff 13 54 0
noteon 13 50 104
sleep 1.736
noteoff 3 54 0
noteoff 3 57 0
noteon 3 50 100
noteon 3 54 100
sleep 3.472
noteon 14 38 46
sleep 177.075
noteoff 12 50 0
sleep 1.736
noteoff 1 72 0
noteoff 1 74 0
sleep 5.208
echo "106560 tempo_s=269 tempo_l=0.25"
sleep 1.858
noteoff 13 50 0
sleep 1.858
noteon 1 71 100
noteon 1 74 100
sleep 5.574
noteon 2 74 101
noteon 2 62 101
sleep 1.858
noteon 12 55 102
sleep 1.858
noteoff 3 54 0
noteoff 3 50 0
sleep 7.432
noteon 13 55 104
sleep 1.858
noteon 3 59 100
noteon 3 55 100
sleep 1.858
select 14 1 0 48
noteon 15 50 36
sleep 1.858
noteoff 14 38 0
noteon 14 43 106
sleep 196.988
noteoff 10 74 0
sleep 3.716
noteoff 1 74 0
noteoff 1 71 0
noteoff 11 62 0
noteon 1 79 100
noteon 1 67 100
sleep 9.293
noteoff 3 55 0
noteoff 3 59 0
sleep 9.293
noteon 3 55 100
sleep 1.858
noteoff 15 50 0
sleep 198.884
echo "106800 tempo_s=289 tempo_l=0.25"
sleep 3.46
noteoff 1 67 0
noteoff 1 79 0
noteon 1 81 100
noteon 1 69 100
sleep 17.301
noteoff 3 55 0
noteon 3 57 100
sleep 179.93
noteoff 12 55 0
sleep 8.65
noteoff 13 55 0
sleep 1.73
noteoff 1 69 0
noteoff 1 81 0
noteon 1 83 100
noteon 1 71 100
sleep 3.46
noteoff 14 43 0
sleep 3.46
noteon 12 55 102
sleep 8.65
noteon 13 55 104
sleep 1.73
noteoff 3 57 0
noteon 3 59 100
sleep 3.46
noteon 14 43 106
sleep 176.47
noteoff 12 55 0
sleep 1.73
noteoff 1 71 0
noteoff 1 83 0
sleep 5.19
echo "107040 tempo_s=269 tempo_l=0.25"
noteon 10 72 74
noteon 10 62 74
sleep 1.858
noteoff 0 74 0
noteoff 0 86 0
noteoff 13 55 0
noteon 0 86 101
noteon 0 74 101
sleep 1.858
noteon 1 84 100
noteon 1 72 100
noteon 11 69 74
sleep 3.717
noteoff 14 43 0
sleep 3.717
noteon 12 55 102
sleep 1.858
noteoff 3 59 0
sleep 7.433
noteon 13 55 104
sleep 1.858
noteon 3 60 100
sleep 1.858
noteon 15 50 36
sleep 1.858
noteon 14 43 106
sleep 197.021
noteoff 10 62 0
noteoff 10 72 0
sleep 3.717
noteoff 11 69 0
sleep 7.434
noteoff 12 55 0
noteon 12 54 102
sleep 9.293
noteoff 13 55 0
noteon 13 54 104
sleep 3.717
noteoff 15 50 0
sleep 1.858
noteoff 14 43 0
noteon 14 42 106
sleep 197.026
echo "107280 tempo_s=289 tempo_l=0.25"
sleep 1.73
noteoff 0 74 0
noteoff 0 86 0
noteon 0 88 101
noteon 0 76 101
sleep 8.65
noteoff 12 54 0
noteon 12 52 102
sleep 8.65
noteoff 13 54 0
noteon 13 52 104
sleep 5.19
noteoff 14 42 0
noteon 14 40 106
sleep 178.2
noteoff 1 72 0
noteoff 1 84 0
sleep 6.92
noteoff 0 76 0
noteoff 0 88 0
noteon 0 90 101
noteon 0 78 101
sleep 1.73
noteon 1 72 100
noteon 1 84 100
sleep 6.92
noteoff 12 52 0
noteon 12 50 102
sleep 1.73
noteoff 3 60 0
sleep 6.92
noteoff 13 52 0
noteon 13 50 104
sleep 1.73
noteon 3 60 100
sleep 3.46
noteoff 14 40 0
noteon 14 38 106
sleep 176.47
noteoff 0 78 0
noteoff 0 90 0
noteoff 12 50 0
sleep 1.73
noteoff 1 84 0
noteoff 1 72 0
sleep 5.19
echo "107520 tempo_s=269 tempo_l=0.25"
noteon 10 62 74
noteon 10 71 74
sleep 1.858
noteoff 13 50 0
noteon 0 91 101
noteon 0 79 101
sleep 1.858
noteon 1 72 100
noteon 1 84 100
noteon 11 67 74
sleep 3.717
noteoff 14 38 0
sleep 3.717
noteon 12 55 102
sleep 1.858
noteoff 3 60 0
sleep 7.433
noteon 13 55 104
sleep 1.858
noteon 3 60 100
sleep 1.858
noteon 15 50 36
sleep 1.858
noteon 14 43 106
sleep 197.021
noteoff 10 71 0
noteoff 10 62 0
sleep 1.858
noteoff 0 79 0
noteoff 0 91 0
sleep 1.858
noteoff 1 84 0
noteoff 1 72 0
noteoff 11 67 0
noteon 1 83 100
noteon 1 71 100
sleep 18.587
noteoff 3 60 0
noteon 3 59 100
sleep 1.858
noteoff 15 50 0
sleep 198.884
echo "107760 tempo_s=289 tempo_l=0.25"
sleep 3.46
noteoff 1 71 0
noteoff 1 83 0
noteon 1 81 100
noteon 1 69 100
sleep 17.301
noteoff 3 59 0
noteon 3 57 100
sleep 179.93
noteoff 12 55 0
sleep 8.65
noteoff 13 55 0
sleep 1.73
noteoff 1 69 0
noteoff 1 81 0
noteon 1 79 100
noteon 1 67 100
sleep 3.46
noteoff 14 43 0
sleep 3.46
noteon 12 55 102
sleep 8.65
noteon 13 55 104
sleep 1.73
noteoff 3 57 0
noteon 3 55 100
sleep 3.46
noteon 14 43 106
sleep 176.47
noteoff 12 55 0
sleep 1.73
noteoff 1 67 0
noteoff 1 79 0
sleep 5.19
echo "108000 tempo_s=269 tempo_l=0.25"
noteon 10 62 74
noteon 10 72 74
sleep 1.858
noteoff 13 55 0
noteon 0 86 101
noteon 0 74 101
sleep 1.858
noteon 1 72 100
noteon 1 84 100
noteon 11 69 74
sleep 3.717
noteoff 14 43 0
sleep 3.717
noteon 12 55 102
sleep 1.858
noteoff 3 55 0
sleep 7.433
noteon 13 55 104
sleep 1.858
noteon 3 60 100
sleep 1.858
noteon 15 50 36
sleep 1.858
noteon 14 43 106
sleep 197.021
noteoff 10 72 0
noteoff 10 62 0
sleep 3.717
noteoff 11 69 0
sleep 7.434
noteoff 12 55 0
noteon 12 54 102
sleep 9.293
noteoff 13 55 0
noteon 13 54 104
sleep 3.717
noteoff 15 50 0
sleep 1.858
noteoff 14 43 0
noteon 14 42 106
sleep 197.026
echo "108240 tempo_s=289 tempo_l=0.25"
sleep 1.73
noteoff 0 74 0
noteoff 0 86 0
noteon 0 88 101
noteon 0 76 101
sleep 6.92
noteoff 2 62 0
noteoff 2 74 0
noteon 2 76 101
noteon 2 64 101
sleep 1.73
noteoff 12 54 0
noteon 12 52 102
sleep 8.65
noteoff 13 54 0
noteon 13 52 104
sleep 5.19
noteoff 14 42 0
noteon 14 40 106
sleep 178.2
noteoff 1 84 0
noteoff 1 72 0
sleep 6.92
noteoff 0 76 0
noteoff 0 88 0
noteon 0 90 101
noteon 0 78 101
sleep 1.73
noteon 1 72 100
noteon 1 84 100
sleep 5.19
noteoff 2 64 0
noteoff 2 76 0
noteon 2 66 101
noteon 2 78 101
sleep 1.73
noteoff 12 52 0
noteon 12 50 102
sleep 1.73
noteoff 3 60 0
sleep 6.92
noteoff 13 52 0
noteon 13 50 104
sleep 1.73
noteon 3 60 100
sleep 3.46
noteoff 14 40 0
noteon 14 38 106
sleep 176.47
noteoff 0 78 0
noteoff 0 90 0
noteoff 12 50 0
sleep 1.73
noteoff 1 84 0
noteoff 1 72 0
sleep 5.19
echo "108480 tempo_s=269 tempo_l=0.25"
noteoff 2 78 0
noteoff 2 66 0
noteon 10 62 102
noteon 10 71 102
sleep 1.858
noteoff 13 50 0
noteon 0 91 101
noteon 0 79 101
sleep 1.858
noteon 1 71 100
noteon 1 83 100
noteon 11 67 102
sleep 3.717
noteoff 14 38 0
sleep 1.858
noteon 2 79 101
noteon 2 67 101
sleep 1.858
noteon 12 55 102
sleep 1.858
noteoff 3 60 0
sleep 7.434
noteon 13 55 104
sleep 1.858
noteon 3 59 100
sleep 1.858
noteon 15 50 36
sleep 1.858
noteon 14 43 106
sleep 178.433
noteoff 10 71 0
noteoff 10 62 0
sleep 3.716
noteoff 11 67 0
sleep 14.869
noteon 10 74 102
sleep 1.858
noteoff 0 79 0
noteoff 0 91 0
sleep 1.858
noteoff 1 83 0
noteoff 1 71 0
noteoff 4 62 0
noteon 1 72 100
noteon 1 84 100
noteon 11 62 102
sleep 5.576
noteoff 2 67 0
noteoff 2 79 0
sleep 1.858
noteoff 5 50 0
sleep 11.151
noteoff 3 59 0
noteon 3 60 100
sleep 1.858
noteoff 15 50 0
sleep 198.863
echo "108720 tempo_s=293 tempo_l=0.25"
sleep 3.412
noteoff 1 84 0
noteoff 1 72 0
noteon 1 71 100
noteon 1 83 100
sleep 17.062
noteoff 3 60 0
noteon 3 59 100
sleep 184.275
noteoff 10 74 0
noteon 10 75 102
sleep 3.412
noteoff 1 83 0
noteoff 1 71 0
noteoff 11 62 0
noteon 1 69 100
noteon 1 81 100
noteon 11 63 102
sleep 5.119
noteon 2 71 101
sleep 1.706
noteoff 12 55 0
noteon 12 54 102
sleep 8.531
noteoff 13 55 0
noteon 13 54 104
sleep 1.706
noteoff 3 59 0
noteon 3 57 100
sleep 3.412
noteoff 14 43 0
noteon 14 42 106
sleep 180.862
echo "108960 tempo_s=269 tempo_l=0.25"
noteoff 10 75 0
noteon 10 76 102
sleep 3.717
noteoff 1 81 0
noteoff 1 69 0
noteoff 11 63 0
noteon 1 67 100
noteon 1 79 100
noteon 11 64 102
sleep 7.434
noteoff 12 54 0
noteon 12 59 102
sleep 9.293
noteoff 13 54 0
noteon 13 59 104
sleep 1.858
noteoff 3 57 0
noteon 3 55 100
sleep 3.717
noteoff 14 42 0
noteon 14 47 106
sleep 197.004
noteoff 10 76 0
noteon 10 78 102
sleep 1.858
noteon 0 90 101
noteon 0 78 101
sleep 1.858
noteoff 1 79 0
noteoff 1 67 0
noteoff 11 64 0
noteon 1 75 100
noteon 1 78 100
noteon 11 66 102
sleep 5.574
noteoff 2 71 0
noteon 2 72 101
sleep 1.858
noteoff 12 59 0
noteon 12 57 102
sleep 9.292
noteoff 13 59 0
noteon 13 57 104
sleep 1.858
noteoff 3 55 0
noteon 3 60 100
noteon 3 66 100
sleep 3.717
noteoff 14 47 0
noteon 14 45 106
sleep 197.006
echo "109200 tempo_s=293 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 1.706
noteoff 0 78 0
noteoff 0 90 0
noteon 0 88 101
noteon 0 76 101
sleep 1.706
noteoff 1 78 0
noteoff 1 75 0
noteoff 11 66 0
noteon 1 76 100
noteon 11 67 102
sleep 5.119
noteoff 2 72 0
noteon 2 71 101
sleep 1.706
noteoff 12 57 0
noteon 12 55 102
sleep 8.531
noteoff 13 57 0
noteon 13 55 104
sleep 1.706
noteoff 3 66 0
noteoff 3 60 0
noteon 3 59 100
noteon 3 64 100
sleep 3.412
noteoff 14 45 0
noteon 14 43 106
sleep 180.863
noteoff 10 79 0
noteon 10 83 102
sleep 1.706
noteoff 0 76 0
noteoff 0 88 0
noteon 0 86 101
noteon 0 74 101
sleep 1.706
noteoff 1 76 0
noteoff 11 67 0
noteon 1 74 100
noteon 1 76 100
noteon 11 71 102
sleep 5.119
noteoff 2 71 0
noteon 2 71 101
sleep 1.706
noteoff 12 55 0
noteon 12 56 102
sleep 8.531
noteoff 13 55 0
noteon 13 56 104
sleep 1.706
noteoff 3 64 0
noteoff 3 59 0
noteon 3 62 100
noteon 3 59 100
sleep 3.412
noteoff 14 43 0
noteon 14 44 106
sleep 78.488
echo "109380 tempo_s=244 tempo_l=0.25"
sleep 102.451
noteoff 10 83 0
sleep 4.098
noteoff 11 71 0
sleep 8.196
noteoff 0 74 0
noteoff 0 86 0
noteoff 12 56 0
sleep 2.049
noteoff 1 76 0
noteoff 1 74 0
sleep 6.147
echo "109440 tempo_s=269 tempo_l=0.25"
noteoff 2 71 0
noteon 10 57 102
sleep 1.858
noteoff 13 56 0
noteon 0 85 101
noteon 0 73 101
sleep 1.858
noteon 1 76 100
noteon 1 73 100
noteon 11 64 92
sleep 3.717
noteoff 14 44 0
sleep 1.858
noteon 2 69 101
sleep 1.858
noteon 12 57 102
sleep 1.858
noteoff 3 59 0
noteoff 3 62 0
sleep 7.434
noteon 13 45 104
sleep 1.858
noteon 3 61 100
noteon 3 57 100
sleep 3.717
noteon 14 33 106
sleep 66.914
noteoff 10 57 0
sleep 11.152
noteoff 12 57 0
sleep 7.434
noteon 10 57 102
sleep 1.858
noteoff 13 45 0
sleep 5.576
noteoff 14 33 0
sleep 3.717
noteon 12 57 102
sleep 9.293
noteon 13 57 104
sleep 5.576
noteon 14 45 106
sleep 66.914
noteoff 10 57 0
sleep 11.152
noteoff 12 57 0
sleep 7.434
noteon 10 61 102
sleep 1.858
noteoff 0 73 0
noteoff 0 85 0
noteoff 13 57 0
sleep 1.858
noteoff 1 73 0
noteoff 1 76 0
noteoff 11 64 0
sleep 3.717
noteoff 14 45 0
sleep 1.858
noteoff 2 69 0
sleep 1.858
noteon 12 57 102
sleep 9.293
noteon 13 57 104
sleep 1.858
noteoff 3 57 0
noteoff 3 61 0
sleep 3.717
noteon 14 45 106
sleep 66.914
noteoff 10 61 0
sleep 11.152
noteoff 12 57 0
sleep 7.434
noteon 10 61 102
sleep 1.858
noteoff 13 57 0
sleep 5.576
noteoff 14 45 0
sleep 3.717
noteon 12 57 102
sleep 9.293
noteon 13 57 104
sleep 5.576
noteon 14 45 106
sleep 66.914
noteoff 10 61 0
sleep 11.152
noteoff 12 57 0
sleep 7.434
echo "109680 tempo_s=301 tempo_l=0.25"
noteon 10 64 102
sleep 1.661
noteoff 13 57 0
sleep 4.983
noteoff 14 45 0
sleep 3.322
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 4.983
noteon 14 45 106
sleep 59.8
noteoff 10 64 0
sleep 9.966
noteoff 12 57 0
sleep 6.644
noteon 10 64 102
sleep 1.661
noteoff 13 57 0
sleep 4.983
noteoff 14 45 0
sleep 3.322
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 4.983
noteon 14 45 106
sleep 59.8
noteoff 10 64 0
sleep 9.966
noteoff 12 57 0
sleep 6.644
noteon 10 69 102
sleep 1.661
noteoff 13 57 0
sleep 4.983
noteoff 14 45 0
sleep 3.322
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 4.983
noteon 14 45 106
sleep 59.8
noteoff 10 69 0
sleep 9.966
noteoff 12 57 0
sleep 6.644
noteon 10 69 102
sleep 1.661
noteoff 13 57 0
sleep 4.983
noteoff 14 45 0
sleep 3.322
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 4.983
noteon 14 45 106
sleep 59.8
noteoff 10 69 0
sleep 9.966
noteoff 12 57 0
sleep 6.644
echo "109920 tempo_s=274 tempo_l=0.25"
noteon 10 73 102
sleep 1.824
noteoff 13 57 0
noteon 0 85 101
noteon 0 81 101
sleep 1.824
noteon 1 73 100
noteon 1 81 100
noteon 4 64 100
noteon 11 61 102
sleep 1.824
noteon 6 57 108
noteon 6 69 108
sleep 1.824
noteoff 14 45 0
sleep 1.824
noteon 2 69 101
noteon 2 73 101
sleep 1.824
noteon 5 57 100
noteon 12 57 102
sleep 9.120
noteon 13 57 104
sleep 1.824
noteon 3 61 100
noteon 3 57 100
sleep 1.824
noteon 15 45 82
sleep 1.824
noteon 14 45 106
sleep 69.312
noteoff 11 61 0
sleep 7.296
noteoff 12 57 0
sleep 9.120
noteoff 13 57 0
sleep 1.824
noteon 11 61 102
sleep 3.648
noteoff 14 45 0
sleep 3.648
noteon 12 57 102
sleep 9.120
noteon 13 57 104
sleep 5.472
noteon 14 45 106
sleep 69.312
noteoff 11 61 0
sleep 7.296
noteoff 12 57 0
sleep 9.120
noteoff 13 57 0
sleep 1.824
noteon 11 64 102
sleep 3.648
noteoff 14 45 0
sleep 3.648
noteon 12 57 102
sleep 9.122
noteon 13 57 104
sleep 3.649
noteoff 15 45 0
sleep 1.824
noteon 14 45 106
sleep 69.343
noteoff 11 64 0
sleep 7.299
noteoff 12 57 0
sleep 9.124
noteoff 13 57 0
sleep 1.824
noteon 11 64 102
sleep 3.649
noteoff 14 45 0
sleep 3.649
noteon 12 57 102
sleep 9.124
noteon 13 57 104
sleep 5.474
noteon 14 45 106
sleep 69.343
noteoff 11 64 0
sleep 7.299
noteoff 12 57 0
sleep 7.299
echo "110160 tempo_s=302 tempo_l=0.25"
sleep 1.655
noteoff 13 57 0
sleep 1.655
noteon 11 69 102
sleep 3.311
noteoff 14 45 0
sleep 3.311
noteon 12 57 102
sleep 8.278
noteon 13 57 104
sleep 4.966
noteon 14 45 106
sleep 62.913
noteoff 11 69 0
sleep 6.622
noteoff 12 57 0
sleep 8.278
noteoff 13 57 0
sleep 1.655
noteon 11 69 102
sleep 3.311
noteoff 14 45 0
sleep 3.311
noteon 12 57 102
sleep 8.278
noteon 13 57 104
sleep 4.966
noteon 14 45 106
sleep 59.602
noteoff 10 73 0
sleep 3.311
noteoff 11 69 0
sleep 6.622
noteoff 12 57 0
sleep 6.622
noteon 10 57 102
sleep 1.655
noteoff 13 57 0
sleep 1.655
noteon 11 73 102
sleep 3.311
noteoff 14 45 0
sleep 3.311
noteon 12 57 102
sleep 8.278
noteon 13 57 104
sleep 4.966
noteon 14 45 106
sleep 59.602
noteoff 10 57 0
sleep 3.311
noteoff 11 73 0
sleep 6.622
noteoff 12 57 0
sleep 6.622
noteon 10 57 102
sleep 1.655
noteoff 13 57 0
sleep 1.655
noteon 11 73 102
sleep 3.311
noteoff 14 45 0
sleep 3.311
noteon 12 57 102
sleep 8.278
noteon 13 57 104
sleep 4.966
noteon 14 45 106
sleep 59.602
noteoff 10 57 0
sleep 3.311
noteoff 11 73 0
sleep 6.622
noteoff 12 57 0
sleep 6.622
echo "110400 tempo_s=277 tempo_l=0.25"
noteon 10 59 102
sleep 1.805
noteoff 0 81 0
noteoff 0 85 0
noteoff 13 57 0
sleep 1.805
noteoff 1 81 0
noteoff 1 73 0
noteoff 4 64 0
noteon 11 74 102
sleep 1.805
noteoff 6 69 0
noteoff 6 57 0
sleep 1.805
noteoff 14 45 0
sleep 1.805
noteoff 2 73 0
noteoff 2 69 0
sleep 1.805
noteoff 5 57 0
noteon 12 56 102
sleep 9.025
noteon 13 45 104
sleep 1.805
noteoff 3 57 0
noteoff 3 61 0
sleep 3.61
noteon 14 33 106
sleep 64.981
noteoff 10 59 0
sleep 10.83
noteoff 12 56 0
sleep 7.22
noteon 10 59 102
sleep 1.805
noteoff 13 45 0
sleep 5.415
noteoff 14 33 0
sleep 3.61
noteon 12 56 102
sleep 9.025
noteon 13 45 104
sleep 5.415
noteon 14 33 106
sleep 64.981
noteoff 10 59 0
sleep 10.83
noteoff 12 56 0
sleep 7.22
noteon 10 62 102
sleep 1.805
noteoff 13 45 0
sleep 1.805
noteoff 11 74 0
sleep 3.61
noteoff 14 33 0
sleep 3.61
noteon 12 56 102
sleep 9.025
noteon 13 45 104
sleep 5.415
noteon 14 33 106
sleep 64.981
noteoff 10 62 0
sleep 10.83
noteoff 12 56 0
sleep 7.22
noteon 10 62 102
sleep 1.805
noteoff 13 45 0
sleep 5.415
noteoff 14 33 0
sleep 3.61
noteon 12 56 102
sleep 9.025
noteon 13 45 104
sleep 5.415
noteon 14 33 106
sleep 64.981
noteoff 10 62 0
sleep 10.83
noteoff 12 56 0
sleep 7.22
echo "110640 tempo_s=306 tempo_l=0.25"
noteon 10 68 102
sleep 1.633
noteoff 13 45 0
sleep 4.901
noteoff 14 33 0
sleep 3.267
noteon 12 56 102
sleep 8.169
noteon 13 45 104
sleep 4.901
noteon 14 33 106
sleep 58.823
noteoff 10 68 0
sleep 9.803
noteoff 12 56 0
sleep 6.535
noteon 10 68 102
sleep 1.633
noteoff 13 45 0
sleep 4.901
noteoff 14 33 0
sleep 3.267
noteon 12 56 102
sleep 8.169
noteon 13 45 104
sleep 4.901
noteon 14 33 106
sleep 58.823
noteoff 10 68 0
sleep 9.803
noteoff 12 56 0
sleep 6.535
noteon 10 71 102
sleep 1.633
noteoff 13 45 0
sleep 4.901
noteoff 14 33 0
sleep 3.267
noteon 12 56 102
sleep 8.169
noteon 13 45 104
sleep 4.901
noteon 14 33 106
sleep 58.823
noteoff 10 71 0
sleep 9.803
noteoff 12 56 0
sleep 6.535
noteon 10 71 102
sleep 1.633
noteoff 13 45 0
sleep 4.901
noteoff 14 33 0
sleep 3.267
noteon 12 56 102
sleep 8.169
noteon 13 45 104
sleep 4.901
noteon 14 33 106
sleep 58.823
noteoff 10 71 0
sleep 9.803
noteoff 12 56 0
sleep 6.535
echo "110880 tempo_s=277 tempo_l=0.25"
noteon 10 74 102
sleep 1.805
noteoff 13 45 0
noteon 0 86 101
noteon 0 83 101
sleep 1.805
noteon 1 80 100
noteon 1 74 100
noteon 4 69 100
noteon 11 62 102
sleep 1.805
noteon 6 57 108
noteon 6 69 108
sleep 1.805
noteoff 14 33 0
sleep 1.805
noteon 2 71 101
noteon 2 74 101
sleep 1.805
noteon 5 57 100
noteon 12 56 102
sleep 9.025
noteon 13 45 104
sleep 1.805
noteon 3 62 100
noteon 3 56 100
sleep 1.805
noteon 15 45 82
sleep 1.805
noteon 14 33 106
sleep 68.590
noteoff 11 62 0
sleep 7.220
noteoff 12 56 0
sleep 9.025
noteoff 13 45 0
sleep 1.805
noteon 11 62 102
sleep 3.610
noteoff 14 33 0
sleep 3.610
noteon 12 56 102
sleep 9.025
noteon 13 45 104
sleep 5.415
noteon 14 33 106
sleep 68.590
noteoff 11 62 0
sleep 7.220
noteoff 12 56 0
sleep 9.025
noteoff 13 45 0
sleep 1.805
noteon 11 68 102
sleep 1.805
noteoff 6 69 0
noteoff 6 57 0
sleep 1.805
noteoff 14 33 0
sleep 3.610
noteon 12 56 102
sleep 9.025
noteon 13 45 104
sleep 3.61
noteoff 15 45 0
sleep 1.805
noteon 14 33 106
sleep 68.592
noteoff 11 68 0
sleep 7.22
noteoff 12 56 0
sleep 9.025
noteoff 13 45 0
sleep 1.805
noteon 11 68 102
sleep 3.61
noteoff 14 33 0
sleep 3.61
noteon 12 56 102
sleep 9.025
noteon 13 45 104
sleep 5.415
noteon 14 33 106
sleep 68.592
noteoff 11 68 0
sleep 7.22
noteoff 12 56 0
sleep 7.22
echo "111120 tempo_s=307 tempo_l=0.25"
sleep 1.628
noteoff 13 45 0
sleep 1.628
noteon 11 71 102
sleep 3.257
noteoff 14 33 0
sleep 3.257
noteon 12 56 102
sleep 8.143
noteon 13 45 104
sleep 4.885
noteon 14 33 106
sleep 61.889
noteoff 11 71 0
sleep 6.514
noteoff 12 56 0
sleep 8.143
noteoff 13 45 0
sleep 1.628
noteon 11 71 102
sleep 3.257
noteoff 14 33 0
sleep 3.257
noteon 12 56 102
sleep 8.143
noteon 13 45 104
sleep 4.885
noteon 14 33 106
sleep 58.631
noteoff 10 74 0
sleep 3.257
noteoff 11 71 0
sleep 6.514
noteoff 12 56 0
sleep 6.514
noteon 10 59 102
sleep 1.628
noteoff 13 45 0
sleep 1.628
noteon 11 74 102
sleep 3.257
noteoff 14 33 0
sleep 3.257
noteon 12 56 102
sleep 8.143
noteon 13 45 104
sleep 4.885
noteon 14 33 106
sleep 58.631
noteoff 10 59 0
sleep 3.257
noteoff 11 74 0
sleep 6.514
noteoff 12 56 0
sleep 6.514
noteon 10 59 102
sleep 1.628
noteoff 13 45 0
sleep 1.628
noteon 11 74 102
sleep 3.257
noteoff 14 33 0
sleep 3.257
noteon 12 56 102
sleep 8.143
noteon 13 45 104
sleep 4.885
noteon 14 33 106
sleep 58.631
noteoff 10 59 0
sleep 3.257
noteoff 11 74 0
sleep 6.514
noteoff 12 56 0
sleep 6.514
echo "111360 tempo_s=278 tempo_l=0.25"
noteon 10 61 102
sleep 1.798
noteoff 0 83 0
noteoff 0 86 0
noteoff 13 45 0
sleep 1.798
noteoff 1 74 0
noteoff 1 80 0
noteoff 4 69 0
noteon 11 76 102
sleep 3.597
noteoff 14 33 0
sleep 1.798
noteoff 2 74 0
noteoff 2 71 0
sleep 1.798
noteoff 5 57 0
noteon 12 55 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteoff 3 56 0
noteoff 3 62 0
sleep 3.597
noteon 14 33 106
sleep 64.748
noteoff 10 61 0
sleep 10.791
noteoff 12 55 0
sleep 7.194
noteon 10 61 102
sleep 1.798
noteoff 13 45 0
sleep 5.395
noteoff 14 33 0
sleep 3.597
noteon 12 55 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 64.748
noteoff 10 61 0
sleep 3.597
noteoff 11 76 0
sleep 7.194
noteoff 12 55 0
sleep 7.194
noteon 10 76 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 61 102
noteon 11 64 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 55 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 64.748
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
noteoff 11 61 0
sleep 7.194
noteoff 12 55 0
sleep 7.194
noteon 10 76 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 61 102
noteon 11 64 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 55 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 64.748
noteoff 10 76 0
sleep 3.597
noteoff 11 64 0
noteoff 11 61 0
sleep 7.194
noteoff 12 55 0
sleep 7.194
echo "111600 tempo_s=310 tempo_l=0.25"
noteon 10 76 117
sleep 1.612
noteoff 13 45 0
noteon 0 88 101
noteon 0 85 101
sleep 1.612
noteon 1 73 100
noteon 1 79 100
noteon 4 64 100
noteon 11 61 117
noteon 11 64 117
sleep 1.612
noteon 6 57 108
noteon 6 69 108
sleep 1.612
noteoff 14 33 0
sleep 1.612
noteon 2 67 101
noteon 2 73 101
sleep 1.612
noteon 5 57 100
noteon 12 55 117
sleep 8.060
noteon 13 45 119
sleep 1.612
noteon 3 61 100
noteon 3 55 100
sleep 1.612
noteon 15 45 82
sleep 1.612
noteon 14 33 121
sleep 58.032
noteoff 10 76 0
sleep 3.224
noteoff 11 64 0
noteoff 11 61 0
sleep 6.448
noteoff 12 55 0
sleep 6.448
noteon 10 76 117
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 61 117
noteon 11 64 117
sleep 3.224
noteoff 14 33 0
sleep 3.224
noteon 12 55 117
sleep 8.060
noteon 13 45 119
sleep 4.836
noteon 14 33 121
sleep 58.032
noteoff 10 76 0
sleep 3.224
noteoff 11 64 0
noteoff 11 61 0
sleep 6.448
noteoff 12 55 0
sleep 6.448
noteon 10 61 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 61 102
noteon 11 64 102
sleep 3.224
noteoff 14 33 0
sleep 3.224
noteon 12 55 102
sleep 8.062
noteon 13 45 104
sleep 3.225
noteoff 15 45 0
sleep 1.612
noteon 14 33 106
sleep 58.064
noteoff 10 61 0
sleep 3.225
noteoff 11 64 0
noteoff 11 61 0
sleep 6.451
noteoff 12 55 0
sleep 6.451
noteon 10 61 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 61 102
noteon 11 64 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 55 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 58.064
noteoff 10 61 0
sleep 3.225
noteoff 11 64 0
noteoff 11 61 0
sleep 6.451
noteoff 12 55 0
sleep 6.451
echo "111840 tempo_s=278 tempo_l=0.25"
noteon 10 62 102
sleep 1.798
noteoff 0 85 0
noteoff 0 88 0
noteoff 13 45 0
sleep 1.798
noteoff 1 79 0
noteoff 1 73 0
noteoff 4 64 0
noteon 11 62 102
noteon 11 66 102
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 1.798
noteoff 14 33 0
sleep 1.798
noteoff 2 73 0
noteoff 2 67 0
sleep 1.798
noteoff 5 57 0
noteon 12 54 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteoff 3 55 0
noteoff 3 61 0
sleep 3.597
noteon 14 33 106
sleep 64.748
noteoff 10 62 0
sleep 3.597
noteoff 11 66 0
noteoff 11 62 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
noteon 10 62 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 66 102
noteon 11 62 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 54 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 64.748
noteoff 10 62 0
sleep 3.597
noteoff 11 62 0
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
noteon 10 78 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 66 102
noteon 11 62 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 54 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 64.748
noteoff 10 78 0
sleep 3.597
noteoff 11 62 0
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
noteon 10 78 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 66 102
noteon 11 62 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 54 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 64.748
noteoff 10 78 0
sleep 3.597
noteoff 11 62 0
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
echo "112080 tempo_s=310 tempo_l=0.25"
noteon 10 78 117
sleep 1.612
noteoff 13 45 0
noteon 0 90 101
noteon 0 86 101
sleep 1.612
noteon 1 78 100
noteon 1 74 100
noteon 4 66 100
noteon 11 66 117
noteon 11 62 117
sleep 1.612
noteon 6 57 108
noteon 6 69 108
sleep 1.612
noteoff 14 33 0
sleep 1.612
noteon 2 74 101
noteon 2 66 101
sleep 1.612
noteon 5 62 100
noteon 12 54 117
sleep 8.060
noteon 13 45 119
sleep 1.612
noteon 3 62 100
noteon 3 54 100
sleep 3.224
noteon 14 33 121
sleep 58.032
noteoff 10 78 0
sleep 3.224
noteoff 11 62 0
noteoff 11 66 0
sleep 6.448
noteoff 12 54 0
sleep 6.448
noteon 10 78 117
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 66 117
noteon 11 62 117
sleep 3.224
noteoff 14 33 0
sleep 3.224
noteon 12 54 117
sleep 8.060
noteon 13 45 119
sleep 4.836
noteon 14 33 121
sleep 58.032
noteoff 10 78 0
sleep 3.224
noteoff 11 62 0
noteoff 11 66 0
sleep 6.448
noteoff 12 54 0
sleep 6.448
noteon 10 62 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 66 102
noteon 11 62 102
sleep 3.224
noteoff 14 33 0
sleep 3.224
noteon 12 54 102
sleep 8.062
noteon 13 45 104
sleep 3.225
noteon 15 45 82
sleep 1.612
noteon 14 33 106
sleep 58.064
noteoff 10 62 0
sleep 3.225
noteoff 11 62 0
noteoff 11 66 0
sleep 6.451
noteoff 12 54 0
sleep 6.451
echo "112260 tempo_s=249 tempo_l=0.25"
noteon 10 62 102
sleep 2.008
noteoff 13 45 0
sleep 2.008
noteon 11 62 102
noteon 11 66 102
sleep 4.016
noteoff 14 33 0
sleep 4.016
noteon 12 54 102
sleep 10.04
noteon 13 45 104
sleep 6.024
noteon 14 33 106
sleep 72.289
noteoff 10 62 0
sleep 4.016
noteoff 11 66 0
noteoff 11 62 0
sleep 8.032
noteoff 12 54 0
sleep 2.008
noteoff 4 66 0
sleep 2.008
noteoff 6 69 0
noteoff 6 57 0
sleep 4.016
echo "112320 tempo_s=278 tempo_l=0.25"
noteon 10 64 102
sleep 1.798
noteoff 0 86 0
noteoff 0 90 0
noteoff 5 62 0
noteoff 13 45 0
noteon 0 91 101
noteon 0 85 101
sleep 1.798
noteoff 1 74 0
noteoff 1 78 0
noteon 1 73 100
noteon 1 79 100
noteon 4 64 100
noteon 11 67 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 1.798
noteoff 14 33 0
sleep 1.798
noteoff 2 66 0
noteoff 2 74 0
noteon 2 67 101
noteon 2 73 101
sleep 1.798
noteon 5 57 100
noteon 12 49 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteoff 3 54 0
noteoff 3 62 0
noteon 3 52 100
noteon 3 61 100
sleep 1.798
noteoff 15 45 0
noteon 15 45 82
sleep 1.798
noteon 14 33 106
sleep 64.748
noteoff 10 64 0
sleep 17.985
noteon 10 64 102
sleep 89.928
noteoff 10 64 0
sleep 17.985
noteon 10 79 102
sleep 1.798
noteoff 0 85 0
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 11 67 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 73 0
noteoff 2 67 0
sleep 1.798
noteoff 5 57 0
noteoff 12 49 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 61 0
noteoff 3 52 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 64.748
noteoff 10 79 0
sleep 17.985
noteon 10 79 102
sleep 89.928
noteoff 10 79 0
sleep 17.985
echo "112560 tempo_s=292 tempo_l=0.25"
noteon 10 79 117
sleep 85.608
noteoff 10 79 0
sleep 17.122
noteon 10 79 117
sleep 85.608
noteoff 10 79 0
sleep 17.122
noteon 10 76 102
sleep 85.616
noteoff 10 76 0
sleep 17.123
noteon 10 76 102
sleep 85.616
noteoff 10 76 0
sleep 17.123
echo "112800 tempo_s=257 tempo_l=0.25"
noteon 10 73 102
sleep 97.275
noteoff 10 73 0
sleep 19.455
noteon 10 73 102
sleep 97.275
noteoff 10 73 0
sleep 19.455
noteon 10 69 102
sleep 97.275
noteoff 10 69 0
sleep 19.455
noteon 10 69 102
sleep 97.275
noteoff 10 69 0
sleep 19.455
echo "113040 tempo_s=291 tempo_l=0.25"
noteon 10 67 102
sleep 85.908
noteoff 10 67 0
sleep 17.182
noteon 10 67 102
sleep 85.908
noteoff 10 67 0
sleep 17.182
noteon 10 64 102
sleep 85.908
noteoff 10 64 0
sleep 17.182
echo "113220 tempo_s=242 tempo_l=0.25"
noteon 10 64 102
sleep 103.304
noteoff 10 64 0
sleep 20.661
echo "113280 tempo_s=269 tempo_l=0.25"
noteon 10 66 92
sleep 3.717
noteon 4 69 100
sleep 7.434
noteon 5 57 100
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 85.496
noteoff 10 66 0
sleep 111.517
noteon 10 69 92
sleep 3.717
noteon 11 66 92
sleep 7.434
noteon 12 54 92
noteon 12 50 92
sleep 9.292
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 85.497
noteoff 10 69 0
sleep 3.717
noteoff 11 66 0
sleep 7.434
noteoff 12 50 0
noteoff 12 54 0
sleep 100.366
echo "113520 tempo_s=287 tempo_l=0.25"
noteon 10 66 92
sleep 3.484
noteon 11 62 92
sleep 6.968
noteon 12 54 92
noteon 12 57 92
sleep 94.076
noteoff 10 66 0
sleep 3.484
noteoff 11 62 0
sleep 6.968
noteoff 12 57 0
noteoff 12 54 0
sleep 94.076
noteon 10 69 92
sleep 3.484
noteon 11 66 92
sleep 6.968
noteon 12 54 92
noteon 12 50 92
sleep 94.076
noteoff 10 69 0
sleep 3.484
noteoff 11 66 0
sleep 6.968
noteoff 12 50 0
noteoff 12 54 0
sleep 94.076
echo "113760 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteoff 4 69 0
noteon 4 66 100
sleep 7.434
noteoff 5 57 0
noteon 5 54 100
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 197.026
noteon 10 69 92
sleep 3.717
noteon 11 66 92
sleep 7.434
noteon 12 50 92
noteon 12 54 92
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 85.501
noteoff 10 69 0
sleep 3.717
noteoff 11 66 0
sleep 7.434
noteoff 12 54 0
noteoff 12 50 0
sleep 100.371
echo "114000 tempo_s=287 tempo_l=0.25"
noteon 10 66 92
sleep 3.484
noteoff 4 66 0
noteon 4 62 100
noteon 11 62 92
sleep 6.968
noteoff 5 54 0
noteon 5 50 100
noteon 12 57 92
noteon 12 54 92
sleep 94.076
noteoff 10 66 0
sleep 3.484
noteoff 11 62 0
sleep 6.968
noteoff 12 54 0
noteoff 12 57 0
sleep 94.076
noteon 10 69 92
sleep 3.484
noteon 11 66 92
sleep 6.968
noteon 12 54 92
noteon 12 50 92
sleep 94.076
noteoff 10 69 0
sleep 3.484
noteoff 11 66 0
sleep 6.968
noteoff 12 50 0
noteoff 12 54 0
sleep 94.076
echo "114240 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteoff 4 62 0
noteon 1 81 100
noteon 4 57 100
sleep 7.434
noteoff 5 50 0
noteon 5 45 100
sleep 9.293
noteon 13 49 104
sleep 5.576
noteon 14 37 106
sleep 197.019
noteon 10 69 92
sleep 3.717
noteoff 4 57 0
noteon 11 67 92
sleep 7.434
noteoff 5 45 0
noteon 12 52 92
noteon 12 55 92
sleep 9.293
noteoff 13 49 0
sleep 5.576
noteoff 14 37 0
sleep 85.501
noteoff 10 69 0
sleep 3.717
noteoff 11 67 0
sleep 7.434
noteoff 12 55 0
noteoff 12 52 0
sleep 100.371
echo "114480 tempo_s=287 tempo_l=0.25"
noteon 10 67 92
sleep 3.484
noteon 11 64 92
sleep 6.968
noteon 12 57 92
noteon 12 55 92
sleep 94.076
noteoff 10 67 0
sleep 3.484
noteoff 11 64 0
sleep 6.968
noteoff 12 55 0
noteoff 12 57 0
sleep 94.076
noteon 10 69 92
sleep 3.484
noteon 11 67 92
sleep 6.968
noteon 12 55 92
noteon 12 52 92
sleep 94.076
noteoff 10 69 0
sleep 3.484
noteoff 11 67 0
sleep 6.968
noteoff 12 52 0
noteoff 12 55 0
sleep 94.076
echo "114720 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteoff 1 81 0
noteon 1 79 100
sleep 16.728
noteon 13 45 104
sleep 5.576
noteon 14 33 106
sleep 197.017
noteon 10 76 102
sleep 1.858
noteon 0 88 101
sleep 1.858
noteon 11 67 92
sleep 7.432
noteon 12 55 92
noteon 12 52 92
sleep 9.290
noteoff 13 45 0
sleep 5.574
noteoff 14 33 0
sleep 85.468
noteoff 10 76 0
noteon 10 74 102
sleep 1.858
noteoff 0 88 0
noteon 0 86 101
sleep 1.858
noteoff 11 67 0
sleep 7.432
noteoff 12 52 0
noteoff 12 55 0
sleep 100.332
echo "114960 tempo_s=287 tempo_l=0.25"
noteoff 10 74 0
noteon 10 73 102
sleep 1.742
noteoff 0 86 0
noteon 0 85 101
sleep 1.742
noteoff 1 79 0
noteon 1 76 100
noteon 11 64 92
sleep 6.968
noteon 12 55 92
noteon 12 57 92
sleep 94.076
noteoff 10 73 0
noteon 10 71 102
sleep 1.742
noteoff 0 85 0
noteon 0 83 101
sleep 1.742
noteoff 11 64 0
sleep 6.968
noteoff 12 57 0
noteoff 12 55 0
sleep 94.076
noteoff 10 71 0
noteon 10 69 102
sleep 1.742
noteoff 0 83 0
noteon 0 81 101
sleep 1.742
noteon 11 67 92
sleep 6.968
noteon 12 55 92
noteon 12 52 92
sleep 94.076
noteoff 10 69 0
noteon 10 73 102
sleep 1.742
noteoff 0 81 0
noteon 0 85 101
sleep 1.742
noteoff 11 67 0
sleep 6.968
noteoff 12 52 0
noteoff 12 55 0
sleep 76.655
noteoff 10 73 0
sleep 1.742
noteoff 0 85 0
sleep 10.452
noteoff 1 76 0
sleep 5.226
echo "115200 tempo_s=269 tempo_l=0.25"
noteon 10 74 92
sleep 1.858
noteon 0 86 91
sleep 1.858
noteon 1 78 100
noteon 4 69 100
noteon 11 66 92
sleep 7.434
noteon 5 57 100
noteon 12 54 92
noteon 12 50 92
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 85.495
noteoff 10 74 0
sleep 3.717
noteoff 11 66 0
sleep 7.434
noteoff 12 50 0
noteoff 12 54 0
sleep 100.364
noteon 10 69 92
sleep 1.858
noteoff 0 86 0
sleep 1.858
noteoff 1 78 0
noteon 11 66 92
sleep 7.434
noteon 12 57 92
noteon 12 54 92
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 85.501
noteoff 10 69 0
sleep 3.717
noteoff 11 66 0
sleep 7.434
noteoff 12 54 0
noteoff 12 57 0
sleep 100.371
echo "115440 tempo_s=287 tempo_l=0.25"
noteon 10 66 92
sleep 3.484
noteon 11 62 92
sleep 6.968
noteon 12 54 92
noteon 12 50 92
sleep 94.076
noteoff 10 66 0
sleep 3.484
noteoff 11 62 0
sleep 6.968
noteoff 12 50 0
noteoff 12 54 0
sleep 94.076
noteon 10 69 92
sleep 3.484
noteon 11 66 92
sleep 6.968
noteon 12 57 92
noteon 12 54 92
sleep 94.076
noteoff 10 69 0
sleep 3.484
noteoff 11 66 0
sleep 6.968
noteoff 12 54 0
noteoff 12 57 0
sleep 94.076
echo "115680 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteoff 4 69 0
noteon 4 66 100
sleep 7.434
noteoff 5 57 0
noteon 5 54 100
sleep 9.293
noteon 13 50 104
sleep 5.576
noteon 14 38 106
sleep 197.026
noteon 10 69 92
sleep 3.717
noteon 11 66 92
sleep 7.434
noteon 12 54 92
noteon 12 50 92
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 85.501
noteoff 10 69 0
sleep 3.717
noteoff 11 66 0
sleep 7.434
noteoff 12 50 0
noteoff 12 54 0
sleep 100.371
echo "115920 tempo_s=287 tempo_l=0.25"
noteon 10 66 92
sleep 3.484
noteoff 4 66 0
noteon 4 62 100
noteon 11 62 92
sleep 6.968
noteoff 5 54 0
noteon 5 50 100
noteon 12 54 92
noteon 12 57 92
sleep 94.076
noteoff 10 66 0
sleep 3.484
noteoff 11 62 0
sleep 6.968
noteoff 12 57 0
noteoff 12 54 0
sleep 94.076
noteon 10 69 92
sleep 3.484
noteon 11 66 92
sleep 6.968
noteon 12 54 92
noteon 12 50 92
sleep 94.076
noteoff 10 69 0
sleep 3.484
noteoff 11 66 0
sleep 6.968
noteoff 12 50 0
noteoff 12 54 0
sleep 94.076
echo "116160 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteoff 4 62 0
noteon 1 81 100
noteon 4 57 100
sleep 7.434
noteoff 5 50 0
noteon 5 45 100
sleep 9.293
noteon 13 49 104
sleep 5.576
noteon 14 37 106
sleep 197.019
noteon 10 69 92
sleep 3.717
noteoff 4 57 0
noteon 11 67 92
sleep 7.434
noteoff 5 45 0
noteon 12 52 92
noteon 12 55 92
sleep 9.293
noteoff 13 49 0
sleep 5.576
noteoff 14 37 0
sleep 85.501
noteoff 10 69 0
sleep 3.717
noteoff 11 67 0
sleep 7.434
noteoff 12 55 0
noteoff 12 52 0
sleep 100.371
echo "116400 tempo_s=287 tempo_l=0.25"
noteon 10 67 92
sleep 3.484
noteon 11 64 92
sleep 6.968
noteon 12 55 92
noteon 12 57 92
sleep 94.076
noteoff 10 67 0
sleep 3.484
noteoff 11 64 0
sleep 6.968
noteoff 12 57 0
noteoff 12 55 0
sleep 94.076
noteon 10 69 92
sleep 3.484
noteon 11 67 92
sleep 6.968
noteon 12 55 92
noteon 12 52 92
sleep 94.076
noteoff 10 69 0
sleep 3.484
noteoff 11 67 0
sleep 6.968
noteoff 12 52 0
noteoff 12 55 0
sleep 94.076
echo "116640 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteoff 1 81 0
noteon 1 79 100
sleep 16.728
noteon 13 45 104
sleep 1.858
noteon 3 57 100
sleep 3.717
noteon 14 33 106
sleep 197.009
noteon 10 76 102
sleep 1.858
noteon 0 88 101
sleep 1.858
noteon 11 67 92
sleep 7.432
noteon 12 64 92
noteon 12 57 92
sleep 9.290
noteoff 13 45 0
sleep 5.574
noteoff 14 33 0
sleep 85.468
noteoff 10 76 0
noteon 10 74 102
sleep 1.858
noteoff 0 88 0
noteon 0 86 101
sleep 1.858
noteoff 11 67 0
sleep 7.432
noteoff 12 57 0
noteoff 12 64 0
sleep 100.332
echo "116880 tempo_s=287 tempo_l=0.25"
noteoff 10 74 0
noteon 10 73 102
sleep 1.742
noteoff 0 86 0
noteon 0 85 101
sleep 1.742
noteoff 1 79 0
noteon 1 76 100
noteon 11 69 92
sleep 6.968
noteon 12 67 92
noteon 12 64 92
sleep 94.076
noteoff 10 73 0
noteon 10 71 102
sleep 1.742
noteoff 0 85 0
noteon 0 83 101
sleep 1.742
noteoff 11 69 0
sleep 6.968
noteoff 12 64 0
noteoff 12 67 0
sleep 94.076
noteoff 10 71 0
noteon 10 69 102
sleep 1.742
noteoff 0 83 0
noteon 0 81 101
sleep 1.742
noteon 11 67 92
sleep 6.968
noteon 12 57 92
noteon 12 64 92
sleep 94.076
noteoff 10 69 0
noteon 10 73 102
sleep 1.742
noteoff 0 81 0
noteon 0 85 101
sleep 1.742
noteoff 11 67 0
sleep 6.968
noteoff 12 64 0
noteoff 12 57 0
sleep 76.655
noteoff 10 73 0
sleep 1.742
noteoff 0 85 0
sleep 10.452
noteoff 1 76 0
sleep 5.226
echo "117120 tempo_s=269 tempo_l=0.25"
noteon 10 74 92
sleep 1.858
noteon 0 86 91
sleep 1.858
noteon 1 78 100
noteon 11 66 92
sleep 7.434
noteon 12 57 92
noteon 12 62 92
sleep 9.293
noteon 13 50 104
sleep 5.575
noteon 14 38 106
sleep 85.501
noteoff 10 74 0
sleep 1.858
noteoff 0 86 0
sleep 1.858
noteoff 11 66 0
sleep 7.434
noteoff 12 62 0
noteoff 12 57 0
sleep 100.370
noteon 10 78 92
sleep 3.717
noteon 11 74 92
sleep 7.434
noteon 12 62 92
noteon 12 66 92
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 85.498
noteoff 10 78 0
sleep 3.717
noteoff 11 74 0
sleep 7.434
noteoff 12 66 0
noteoff 12 62 0
sleep 100.368
echo "117360 tempo_s=287 tempo_l=0.25"
noteon 10 74 92
sleep 3.484
noteon 11 69 92
sleep 6.968
noteon 12 57 92
noteon 12 62 92
sleep 94.074
noteoff 10 74 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 62 0
noteoff 12 57 0
sleep 94.074
noteon 10 78 92
sleep 3.484
noteon 11 74 92
sleep 6.968
noteon 12 66 92
noteon 12 62 92
sleep 94.072
noteoff 10 78 0
sleep 3.484
noteoff 11 74 0
sleep 6.968
noteoff 12 62 0
noteoff 12 66 0
sleep 94.073
echo "117600 tempo_s=269 tempo_l=0.25"
sleep 20.445
noteon 13 54 104
sleep 1.858
noteoff 3 57 0
noteon 3 58 100
sleep 3.717
noteon 14 42 106
sleep 197.020
noteon 10 78 92
sleep 3.716
noteon 11 73 92
sleep 7.434
noteon 12 61 92
noteon 12 66 92
sleep 9.293
noteoff 13 54 0
sleep 5.576
noteoff 14 42 0
sleep 85.499
noteoff 10 78 0
sleep 3.717
noteoff 11 73 0
sleep 7.434
noteoff 12 66 0
noteoff 12 61 0
sleep 100.369
echo "117840 tempo_s=287 tempo_l=0.25"
noteon 10 73 92
sleep 3.484
noteon 11 70 92
sleep 6.968
noteon 12 58 92
noteon 12 61 92
sleep 94.073
noteoff 10 73 0
sleep 3.484
noteoff 11 70 0
sleep 6.968
noteoff 12 61 0
noteoff 12 58 0
sleep 94.073
noteon 10 78 92
sleep 3.484
noteon 11 73 92
sleep 6.968
noteon 12 61 92
noteon 12 66 92
sleep 94.073
noteoff 10 78 0
sleep 3.484
noteoff 11 73 0
sleep 6.968
noteoff 12 66 0
noteoff 12 61 0
sleep 94.071
echo "118080 tempo_s=269 tempo_l=0.25"
sleep 20.443
noteon 13 59 104
sleep 1.858
noteoff 3 58 0
noteon 3 59 100
sleep 3.717
noteon 14 47 106
sleep 197.019
noteon 10 78 92
sleep 3.716
noteon 11 74 92
sleep 7.434
noteon 12 66 92
noteon 12 62 92
sleep 9.293
noteoff 13 59 0
sleep 5.575
noteoff 14 47 0
sleep 85.499
noteoff 10 78 0
sleep 3.717
noteoff 11 74 0
sleep 7.434
noteoff 12 62 0
noteoff 12 66 0
sleep 100.367
echo "118320 tempo_s=293 tempo_l=0.25"
noteon 10 74 92
sleep 3.412
noteon 11 71 92
sleep 6.825
noteon 12 62 92
noteon 12 59 92
sleep 92.144
noteoff 10 74 0
sleep 3.412
noteoff 11 71 0
sleep 6.825
noteoff 12 59 0
noteoff 12 62 0
sleep 92.145
noteon 10 78 92
sleep 3.412
noteon 11 74 92
sleep 6.825
noteon 12 62 92
noteon 12 66 92
sleep 92.145
noteoff 10 78 0
sleep 3.412
noteoff 11 74 0
sleep 6.825
noteoff 12 66 0
noteoff 12 62 0
sleep 92.146
echo "118560 tempo_s=278 tempo_l=0.25"
sleep 3.597
noteoff 1 78 0
noteon 4 69 100
noteon 1 81 100
sleep 7.194
noteon 5 57 100
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 59 0
noteon 3 61 100
sleep 3.596
noteon 14 45 106
sleep 190.622
noteon 10 81 92
sleep 3.596
noteon 11 76 92
sleep 7.192
noteon 12 64 92
noteon 12 69 92
sleep 8.990
noteoff 13 57 0
sleep 5.394
noteoff 14 45 0
sleep 82.726
noteoff 10 81 0
sleep 3.596
noteoff 11 76 0
sleep 7.193
noteoff 12 69 0
noteoff 12 64 0
sleep 97.115
echo "118800 tempo_s=297 tempo_l=0.25"
noteon 10 76 92
sleep 3.366
noteon 11 73 92
sleep 6.733
noteon 12 61 92
noteon 12 64 92
sleep 90.909
noteoff 10 76 0
sleep 3.367
noteoff 11 73 0
sleep 6.734
noteoff 12 64 0
noteoff 12 61 0
sleep 90.908
noteon 10 81 92
sleep 3.366
noteon 11 76 92
sleep 6.734
noteon 12 64 92
noteon 12 69 92
sleep 90.908
noteoff 10 81 0
sleep 3.367
noteoff 11 76 0
sleep 6.734
noteoff 12 69 0
noteoff 12 64 0
sleep 90.909
echo "119040 tempo_s=280 tempo_l=0.25"
sleep 19.641
noteon 13 62 104
sleep 1.785
noteoff 3 61 0
noteon 3 62 100
sleep 3.571
noteon 14 50 106
sleep 189.268
noteon 10 81 92
sleep 3.570
noteon 11 78 92
sleep 7.142
noteon 12 62 92
noteon 12 66 92
sleep 8.928
noteoff 13 62 0
sleep 5.357
noteoff 14 50 0
sleep 82.134
noteoff 10 81 0
sleep 3.571
noteoff 11 78 0
sleep 7.142
noteoff 12 66 0
noteoff 12 62 0
sleep 96.420
echo "119280 tempo_s=296 tempo_l=0.25"
noteon 10 78 92
sleep 3.378
noteon 11 74 92
sleep 6.756
noteon 12 66 92
noteon 12 69 92
sleep 91.206
noteoff 10 78 0
sleep 3.378
noteoff 11 74 0
sleep 6.756
noteoff 12 69 0
noteoff 12 66 0
sleep 91.206
noteon 10 81 92
sleep 3.378
noteon 11 78 92
sleep 6.756
noteon 12 66 92
noteon 12 62 92
sleep 91.206
noteoff 10 81 0
sleep 3.378
noteoff 11 78 0
sleep 6.756
noteoff 12 62 0
noteoff 12 66 0
sleep 86.139
noteoff 4 69 0
sleep 5.067
echo "119520 tempo_s=277 tempo_l=0.25"
sleep 1.805
noteoff 5 57 0
sleep 1.805
noteon 4 67 100
sleep 7.22
noteon 5 67 100
sleep 9.025
noteon 13 55 104
sleep 1.805
noteoff 3 62 0
noteon 3 59 100
sleep 3.61
noteon 14 43 106
sleep 191.335
noteon 10 79 92
sleep 3.61
noteoff 4 67 0
noteon 11 71 92
sleep 7.22
noteoff 5 67 0
noteon 12 55 92
noteon 12 64 92
sleep 9.025
noteoff 13 55 0
sleep 5.415
noteoff 14 43 0
sleep 83.032
noteoff 10 79 0
sleep 3.61
noteoff 11 71 0
sleep 7.22
noteoff 12 64 0
noteoff 12 55 0
sleep 97.472
echo "119760 tempo_s=293 tempo_l=0.25"
noteon 10 76 92
sleep 3.412
noteoff 1 81 0
noteon 1 79 100
noteon 4 67 100
noteon 11 67 92
sleep 6.825
noteon 5 67 100
noteon 12 59 92
noteon 12 67 92
sleep 92.15
noteoff 10 76 0
sleep 3.412
noteoff 11 67 0
sleep 6.825
noteoff 12 67 0
noteoff 12 59 0
sleep 92.15
noteon 10 79 92
sleep 3.412
noteoff 1 79 0
noteoff 4 67 0
noteon 1 76 100
noteon 11 71 92
sleep 6.825
noteoff 5 67 0
noteon 12 55 92
noteon 12 64 92
sleep 10.238
noteoff 3 59 0
noteon 3 55 100
sleep 81.911
noteoff 10 79 0
sleep 3.412
noteoff 11 71 0
sleep 6.825
noteoff 12 64 0
noteoff 12 55 0
sleep 87.03
noteoff 1 76 0
sleep 5.119
echo "120000 tempo_s=272 tempo_l=0.25"
sleep 3.676
noteon 1 76 100
sleep 9.191
noteoff 3 55 0
sleep 7.352
noteon 13 57 104
sleep 1.838
noteon 3 54 100
sleep 3.676
noteon 14 45 106
sleep 194.852
noteon 10 78 92
sleep 3.676
noteon 11 69 92
sleep 7.352
noteon 12 54 92
noteon 12 62 92
sleep 9.191
noteoff 13 57 0
sleep 5.514
noteoff 14 45 0
sleep 84.558
noteoff 10 78 0
sleep 3.676
noteoff 11 69 0
sleep 7.352
noteoff 12 62 0
noteoff 12 54 0
sleep 99.264
echo "120240 tempo_s=291 tempo_l=0.25"
noteon 10 74 92
sleep 3.436
noteoff 1 76 0
noteon 1 74 100
noteon 4 69 100
noteon 11 66 92
sleep 6.872
noteon 5 69 100
noteon 12 57 92
noteon 12 66 92
sleep 92.783
noteoff 10 74 0
sleep 3.436
noteoff 11 66 0
sleep 6.872
noteoff 12 66 0
noteoff 12 57 0
sleep 92.783
noteon 10 78 92
sleep 3.436
noteoff 4 69 0
noteon 11 69 92
sleep 6.872
noteoff 5 69 0
noteon 12 62 92
noteon 12 54 92
sleep 92.783
noteoff 10 78 0
sleep 3.436
noteoff 11 69 0
sleep 6.872
noteoff 12 54 0
noteoff 12 62 0
sleep 92.783
echo "120480 tempo_s=269 tempo_l=0.25"
sleep 20.446
noteon 13 45 104
sleep 1.858
noteoff 3 54 0
noteon 3 55 100
sleep 3.717
noteon 14 33 106
sleep 197.026
noteon 10 76 92
sleep 3.717
noteon 11 67 92
sleep 7.434
noteon 12 52 92
noteon 12 61 92
sleep 9.293
noteoff 13 45 0
sleep 5.576
noteoff 14 33 0
sleep 85.501
noteoff 10 76 0
sleep 3.717
noteoff 11 67 0
sleep 7.434
noteoff 12 61 0
noteoff 12 52 0
sleep 100.371
echo "120720 tempo_s=291 tempo_l=0.25"
noteon 10 73 92
sleep 3.436
noteoff 1 74 0
noteon 1 76 100
noteon 11 64 92
sleep 6.872
noteon 12 64 92
noteon 12 57 92
sleep 92.783
noteoff 10 73 0
sleep 3.436
noteoff 11 64 0
sleep 6.872
noteoff 12 57 0
noteoff 12 64 0
sleep 92.783
noteon 10 76 92
sleep 3.436
noteon 4 57 100
noteon 11 67 92
sleep 6.872
noteon 5 57 100
noteon 12 61 92
noteon 12 52 92
sleep 92.783
echo "120900 tempo_s=210 tempo_l=0.25"
noteoff 10 76 0
sleep 4.761
noteoff 1 76 0
noteoff 11 67 0
noteon 1 73 100
sleep 9.523
noteoff 12 52 0
noteoff 12 61 0
sleep 121.428
noteoff 1 73 0
noteoff 4 57 0
sleep 7.142
echo "120960 tempo_s=269 tempo_l=0.25"
noteon 10 74 92
sleep 1.858
noteoff 5 57 0
sleep 1.858
noteon 1 74 100
noteon 4 57 100
noteon 11 65 92
sleep 1.858
noteon 6 57 108
noteon 6 69 108
sleep 3.717
noteon 2 69 101
sleep 1.858
noteon 5 45 100
noteon 12 62 92
noteon 12 53 92
sleep 1.858
noteoff 3 55 0
sleep 7.434
noteon 13 50 104
sleep 1.858
noteon 3 53 100
sleep 1.858
noteon 15 45 31
sleep 1.858
noteon 14 38 106
sleep 85.499
noteoff 10 74 0
sleep 3.717
noteoff 11 65 0
sleep 7.434
noteoff 12 53 0
noteoff 12 62 0
sleep 100.368
noteon 10 77 92
sleep 3.717
noteoff 1 74 0
noteon 11 74 92
sleep 7.434
noteon 12 62 92
noteon 12 57 92
sleep 9.293
noteoff 13 50 0
sleep 1.858
noteoff 3 53 0
sleep 1.858
noteoff 15 45 0
sleep 1.858
noteoff 14 38 0
sleep 85.501
noteoff 10 77 0
sleep 3.717
noteoff 11 74 0
sleep 7.434
noteoff 12 57 0
noteoff 12 62 0
sleep 100.369
echo "121200 tempo_s=287 tempo_l=0.25"
noteon 10 74 92
sleep 3.484
noteon 11 69 92
sleep 6.968
noteon 12 62 92
noteon 12 65 92
sleep 10.452
noteon 3 57 100
sleep 83.621
noteoff 10 74 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 65 0
noteoff 12 62 0
sleep 94.076
noteon 10 77 92
sleep 3.484
noteon 11 74 92
sleep 6.968
noteon 12 57 92
noteon 12 62 92
sleep 94.073
noteoff 10 77 0
sleep 3.484
noteoff 11 74 0
sleep 6.968
noteoff 12 62 0
noteoff 12 57 0
sleep 94.076
echo "121440 tempo_s=269 tempo_l=0.25"
sleep 9.293
noteoff 2 69 0
noteon 2 65 101
sleep 11.151
noteon 13 50 104
sleep 1.858
noteoff 3 57 0
noteon 3 53 100
sleep 1.858
noteon 15 45 31
sleep 1.858
noteon 14 38 106
sleep 197.026
noteon 10 77 92
sleep 3.717
noteon 11 74 92
sleep 7.434
noteon 12 57 92
noteon 12 62 92
sleep 9.293
noteoff 13 50 0
sleep 3.717
noteoff 15 45 0
sleep 1.858
noteoff 14 38 0
sleep 85.501
noteoff 10 77 0
sleep 3.717
noteoff 11 74 0
sleep 7.434
noteoff 12 62 0
noteoff 12 57 0
sleep 100.371
echo "121680 tempo_s=287 tempo_l=0.25"
noteon 10 74 92
sleep 3.484
noteon 11 69 92
sleep 5.226
noteoff 2 65 0
noteon 2 62 101
sleep 1.742
noteon 12 62 92
noteon 12 65 92
sleep 10.452
noteoff 3 53 0
noteon 3 50 100
sleep 83.623
noteoff 10 74 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 65 0
noteoff 12 62 0
sleep 94.076
noteon 10 77 92
sleep 3.484
noteon 11 74 92
sleep 6.968
noteon 12 57 92
noteon 12 62 92
sleep 94.076
noteoff 10 77 0
sleep 3.484
noteoff 11 74 0
sleep 6.968
noteoff 12 62 0
noteoff 12 57 0
sleep 94.076
echo "121920 tempo_s=269 tempo_l=0.25"
noteoff 2 62 0
sleep 3.717
noteon 1 81 100
sleep 5.576
noteon 2 57 101
sleep 3.717
noteoff 3 50 0
sleep 7.434
noteon 13 49 104
sleep 1.858
noteon 3 45 100
sleep 1.858
noteon 15 45 31
sleep 1.858
noteon 14 37 106
sleep 197.026
noteon 10 79 92
sleep 3.717
noteon 11 76 92
sleep 5.576
noteoff 2 57 0
sleep 1.858
noteon 12 64 92
noteon 12 57 92
sleep 9.293
noteoff 13 49 0
sleep 1.858
noteoff 3 45 0
sleep 1.858
noteoff 15 45 0
sleep 1.858
noteoff 14 37 0
sleep 85.501
noteoff 10 79 0
sleep 3.717
noteoff 11 76 0
sleep 7.434
noteoff 12 57 0
noteoff 12 64 0
sleep 100.371
echo "122160 tempo_s=287 tempo_l=0.25"
noteon 10 76 92
sleep 3.484
noteon 11 69 92
sleep 6.968
noteon 12 67 92
noteon 12 64 92
sleep 94.076
noteoff 10 76 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 64 0
noteoff 12 67 0
sleep 94.076
noteon 10 79 92
sleep 3.484
noteon 11 76 92
sleep 6.968
noteon 12 64 92
noteon 12 57 92
sleep 94.076
noteoff 10 79 0
sleep 3.484
noteoff 11 76 0
sleep 6.968
noteoff 12 57 0
noteoff 12 64 0
sleep 94.076
echo "122400 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteoff 1 81 0
noteon 1 79 100
sleep 16.728
noteon 13 45 104
sleep 3.717
noteon 15 45 31
sleep 1.858
noteon 14 33 106
sleep 197.017
noteon 10 76 102
sleep 1.858
noteon 0 88 101
sleep 1.858
noteon 11 76 92
sleep 7.432
noteon 12 64 92
noteon 12 57 92
sleep 9.290
noteoff 13 45 0
sleep 3.716
noteoff 15 45 0
sleep 1.858
noteoff 14 33 0
sleep 85.468
noteoff 10 76 0
noteon 10 74 102
sleep 1.858
noteoff 0 88 0
noteon 0 86 101
sleep 1.858
noteoff 11 76 0
sleep 7.432
noteoff 12 57 0
noteoff 12 64 0
sleep 100.332
echo "122640 tempo_s=287 tempo_l=0.25"
noteoff 10 74 0
noteon 10 73 102
sleep 1.742
noteoff 0 86 0
noteon 0 85 101
sleep 1.742
noteoff 1 79 0
noteon 1 76 100
noteon 11 69 92
sleep 6.968
noteon 12 67 92
noteon 12 64 92
sleep 94.076
noteoff 10 73 0
noteon 10 71 102
sleep 1.742
noteoff 0 85 0
noteon 0 83 101
sleep 1.742
noteoff 11 69 0
sleep 6.968
noteoff 12 64 0
noteoff 12 67 0
sleep 94.076
noteoff 10 71 0
noteon 10 69 102
sleep 1.742
noteoff 0 83 0
noteon 0 81 101
sleep 1.742
noteon 11 76 92
sleep 6.968
noteon 12 64 92
noteon 12 57 92
sleep 94.076
noteoff 10 69 0
noteon 10 73 102
sleep 1.742
noteoff 0 81 0
noteon 0 85 101
sleep 1.742
noteoff 11 76 0
sleep 6.968
noteoff 12 57 0
noteoff 12 64 0
sleep 76.655
noteoff 10 73 0
sleep 1.742
noteoff 0 85 0
sleep 10.452
noteoff 1 76 0
sleep 5.226
echo "122880 tempo_s=269 tempo_l=0.25"
noteon 10 74 92
sleep 1.858
noteon 0 86 91
sleep 1.858
noteon 1 77 100
noteon 11 74 92
sleep 16.728
noteon 13 50 104
sleep 1.858
noteon 3 57 100
sleep 1.858
noteon 15 45 31
sleep 1.858
noteon 14 38 106
sleep 85.501
noteoff 10 74 0
sleep 1.858
noteoff 0 86 0
sleep 1.858
noteoff 11 74 0
sleep 107.805
noteon 10 69 92
sleep 3.717
noteoff 1 77 0
noteoff 4 57 0
noteon 11 65 92
sleep 1.858
noteoff 6 69 0
noteoff 6 57 0
sleep 5.576
noteoff 5 45 0
noteon 12 53 92
noteon 12 50 92
sleep 9.293
noteoff 13 50 0
sleep 3.717
noteoff 15 45 0
sleep 1.858
noteoff 14 38 0
sleep 85.501
noteoff 10 69 0
sleep 3.717
noteoff 11 65 0
sleep 7.434
noteoff 12 50 0
noteoff 12 53 0
sleep 100.368
echo "123120 tempo_s=287 tempo_l=0.25"
noteon 10 65 92
sleep 3.484
noteon 11 62 92
sleep 6.968
noteon 12 53 92
noteon 12 57 92
sleep 94.072
noteoff 10 65 0
sleep 3.484
noteoff 11 62 0
sleep 6.968
noteoff 12 57 0
noteoff 12 53 0
sleep 94.073
noteon 10 69 92
sleep 3.484
noteon 11 65 92
sleep 6.968
noteon 12 50 92
noteon 12 53 92
sleep 94.073
noteoff 10 69 0
sleep 3.484
noteoff 11 65 0
sleep 6.968
noteoff 12 53 0
noteoff 12 50 0
sleep 94.075
echo "123360 tempo_s=269 tempo_l=0.25"
sleep 20.444
noteon 13 50 104
sleep 5.575
noteon 14 38 106
sleep 197.021
noteon 10 69 92
sleep 3.717
noteon 11 65 92
sleep 7.434
noteon 12 50 92
noteon 12 53 92
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 85.498
noteoff 10 69 0
sleep 3.717
noteoff 11 65 0
sleep 7.434
noteoff 12 53 0
noteoff 12 50 0
sleep 100.368
echo "123600 tempo_s=287 tempo_l=0.25"
noteon 10 65 92
sleep 3.484
noteon 11 62 92
sleep 6.968
noteon 12 53 92
noteon 12 57 92
sleep 10.452
noteoff 3 57 0
noteon 3 55 100
sleep 83.621
noteoff 10 65 0
sleep 3.484
noteoff 11 62 0
sleep 6.968
noteoff 12 57 0
noteoff 12 53 0
sleep 94.075
noteon 10 69 92
sleep 3.484
noteon 11 65 92
sleep 6.968
noteon 12 53 92
noteon 12 50 92
sleep 10.452
noteoff 3 55 0
noteon 3 53 100
sleep 83.622
noteoff 10 69 0
sleep 3.484
noteoff 11 65 0
sleep 6.968
noteoff 12 50 0
noteoff 12 53 0
sleep 94.073
echo "123840 tempo_s=269 tempo_l=0.25"
sleep 13.011
noteoff 3 53 0
sleep 7.434
noteon 13 48 104
sleep 1.858
noteon 3 60 100
sleep 3.717
noteon 14 36 106
sleep 197.016
noteon 10 72 92
sleep 3.717
noteon 11 67 92
sleep 7.434
noteon 12 52 92
noteon 12 55 92
sleep 9.293
noteoff 13 48 0
sleep 5.576
noteoff 14 36 0
sleep 85.497
noteoff 10 72 0
sleep 3.717
noteoff 11 67 0
sleep 7.434
noteoff 12 55 0
noteoff 12 52 0
sleep 100.368
echo "124080 tempo_s=287 tempo_l=0.25"
noteon 10 67 92
sleep 3.484
noteon 11 64 92
sleep 6.968
noteon 12 55 92
noteon 12 60 92
sleep 94.075
noteoff 10 67 0
sleep 3.484
noteoff 11 64 0
sleep 6.968
noteoff 12 60 0
noteoff 12 55 0
sleep 94.073
noteon 10 72 92
sleep 3.484
noteon 11 67 92
sleep 6.968
noteon 12 55 92
noteon 12 52 92
sleep 94.073
noteoff 10 72 0
sleep 3.484
noteoff 11 67 0
sleep 6.968
noteoff 12 52 0
noteoff 12 55 0
sleep 94.073
echo "124320 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteon 1 72 100
sleep 16.728
noteon 13 46 104
sleep 5.575
noteon 14 34 106
sleep 197.022
noteon 10 72 92
sleep 3.717
noteon 11 67 92
sleep 7.434
noteon 12 55 92
noteon 12 52 92
sleep 9.292
noteoff 13 46 0
sleep 5.576
noteoff 14 34 0
sleep 85.501
noteoff 10 72 0
sleep 3.717
noteoff 11 67 0
sleep 7.434
noteoff 12 52 0
noteoff 12 55 0
sleep 100.368
echo "124560 tempo_s=287 tempo_l=0.25"
noteon 10 67 92
sleep 3.484
noteon 11 64 92
sleep 6.968
noteon 12 60 92
noteon 12 55 92
sleep 94.072
noteoff 10 67 0
sleep 3.484
noteoff 11 64 0
sleep 6.968
noteoff 12 55 0
noteoff 12 60 0
sleep 94.072
noteon 10 72 92
sleep 3.484
noteon 11 67 92
sleep 6.968
noteon 12 52 92
noteon 12 55 92
sleep 94.073
noteoff 10 72 0
sleep 3.484
noteoff 11 67 0
sleep 6.968
noteoff 12 55 0
noteoff 12 52 0
sleep 94.075
echo "124800 tempo_s=269 tempo_l=0.25"
sleep 20.444
noteon 13 45 104
sleep 5.576
noteon 14 33 106
sleep 197.008
noteon 10 72 92
sleep 3.716
noteon 11 69 92
sleep 7.434
noteon 12 53 92
noteon 12 57 92
sleep 9.293
noteoff 13 45 0
sleep 5.576
noteoff 14 33 0
sleep 85.494
noteoff 10 72 0
sleep 3.717
noteoff 11 69 0
sleep 7.434
noteoff 12 57 0
noteoff 12 53 0
sleep 100.362
echo "125040 tempo_s=287 tempo_l=0.25"
noteon 10 69 92
sleep 3.484
noteon 11 65 92
sleep 6.968
noteon 12 57 92
noteon 12 60 92
sleep 94.068
noteoff 10 69 0
sleep 3.484
noteoff 11 65 0
sleep 6.968
noteoff 12 60 0
noteoff 12 57 0
sleep 94.068
noteon 10 72 92
sleep 3.484
noteon 11 69 92
sleep 6.968
noteon 12 53 92
noteon 12 57 92
sleep 94.068
noteoff 10 72 0
sleep 3.484
noteoff 11 69 0
sleep 6.968
noteoff 12 57 0
noteoff 12 53 0
sleep 88.842
noteoff 1 72 0
sleep 5.226
echo "125280 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteon 1 70 100
sleep 9.292
noteoff 3 60 0
sleep 7.434
noteon 13 46 104
sleep 1.858
noteon 3 58 100
sleep 3.717
noteon 14 34 106
sleep 197.026
noteon 10 70 92
sleep 3.717
noteon 11 67 92
sleep 7.434
noteon 12 50 92
noteon 12 55 92
sleep 9.293
noteoff 13 46 0
sleep 5.576
noteoff 14 34 0
sleep 85.501
noteoff 10 70 0
sleep 3.717
noteoff 11 67 0
sleep 7.434
noteoff 12 55 0
noteoff 12 50 0
sleep 100.371
echo "125520 tempo_s=287 tempo_l=0.25"
noteon 10 67 92
sleep 3.484
noteoff 1 70 0
noteon 1 72 100
noteon 11 62 92
sleep 6.968
noteon 12 55 92
noteon 12 58 92
sleep 10.452
noteoff 3 58 0
noteon 3 60 100
sleep 83.623
noteoff 10 67 0
sleep 3.484
noteoff 11 62 0
sleep 6.968
noteoff 12 58 0
noteoff 12 55 0
sleep 94.076
noteon 10 70 92
sleep 3.484
noteoff 1 72 0
noteon 1 74 100
noteon 11 67 92
sleep 6.968
noteon 12 55 92
noteon 12 50 92
sleep 10.452
noteoff 3 60 0
noteon 3 62 100
sleep 83.623
noteoff 10 70 0
sleep 3.484
noteoff 11 67 0
sleep 6.968
noteoff 12 50 0
noteoff 12 55 0
sleep 88.85
noteoff 1 74 0
sleep 5.226
echo "125760 tempo_s=269 tempo_l=0.25"
sleep 3.717
noteon 1 65 100
sleep 9.293
noteoff 3 62 0
sleep 7.434
noteon 13 48 104
sleep 1.858
noteon 3 53 100
sleep 3.717
noteon 14 36 106
sleep 197.026
noteon 10 69 92
sleep 3.717
noteon 11 65 92
sleep 7.434
noteon 12 48 92
noteon 12 53 92
sleep 9.293
noteoff 13 48 0
sleep 5.576
noteoff 14 36 0
sleep 85.501
noteoff 10 69 0
sleep 3.717
noteoff 11 65 0
sleep 7.434
noteoff 12 53 0
noteoff 12 48 0
sleep 100.371
echo "126000 tempo_s=287 tempo_l=0.25"
noteon 10 65 92
sleep 3.484
noteon 11 60 92
sleep 6.968
noteon 12 57 92
noteon 12 53 92
sleep 94.076
noteoff 10 65 0
sleep 3.484
noteoff 11 60 0
sleep 6.968
noteoff 12 53 0
noteoff 12 57 0
sleep 94.076
noteon 10 69 92
sleep 3.484
noteon 11 65 92
sleep 6.968
noteon 12 48 92
noteon 12 53 92
sleep 94.076
noteoff 10 69 0
sleep 3.484
noteoff 11 65 0
sleep 6.968
noteoff 12 53 0
noteoff 12 48 0
sleep 94.076
echo "126240 tempo_s=269 tempo_l=0.25"
sleep 1.858
noteon 0 81 101
sleep 1.858
noteoff 1 65 0
noteon 1 64 100
sleep 16.728
noteon 13 48 104
sleep 1.858
noteoff 3 53 0
noteon 3 52 100
sleep 3.717
noteon 14 36 106
sleep 197.021
noteon 10 69 92
sleep 3.717
noteon 11 67 92
sleep 7.434
noteon 12 52 92
noteon 12 55 92
sleep 9.293
noteoff 13 48 0
sleep 5.576
noteoff 14 36 0
sleep 85.500
noteoff 10 69 0
sleep 3.717
noteoff 11 67 0
sleep 7.434
noteoff 12 55 0
noteoff 12 52 0
sleep 100.370
echo "126480 tempo_s=291 tempo_l=0.25"
noteon 10 67 92
sleep 3.436
noteon 11 64 92
sleep 6.872
noteon 12 55 92
noteon 12 57 92
sleep 92.783
noteoff 10 67 0
sleep 3.436
noteoff 11 64 0
sleep 6.872
noteoff 12 57 0
noteoff 12 55 0
sleep 92.783
noteon 10 69 92
sleep 3.436
noteon 11 67 92
sleep 6.872
noteon 12 55 92
noteon 12 52 92
sleep 92.783
echo "126660 tempo_s=235 tempo_l=0.25"
noteoff 10 69 0
sleep 4.255
noteoff 11 67 0
sleep 8.51
noteoff 12 52 0
noteoff 12 55 0
sleep 108.51
noteoff 1 64 0
sleep 6.382
echo "126720 tempo_s=269 tempo_l=0.25"
sleep 3.716
noteon 1 69 100
sleep 9.292
noteoff 3 52 0
sleep 7.434
noteon 13 50 104
sleep 1.858
noteon 3 57 100
sleep 3.717
noteon 14 38 106
sleep 197.016
noteon 10 77 92
sleep 3.716
noteon 11 69 92
sleep 7.434
noteon 12 53 92
noteon 12 62 92
sleep 9.293
noteoff 13 50 0
sleep 5.576
noteoff 14 38 0
sleep 85.498
noteoff 10 77 0
sleep 3.717
noteoff 11 69 0
sleep 7.434
noteoff 12 62 0
noteoff 12 53 0
sleep 100.364
echo "126960 tempo_s=291 tempo_l=0.25"
noteon 10 74 92
sleep 3.436
noteon 11 65 92
sleep 6.872
noteon 12 57 92
noteon 12 65 92
sleep 92.780
noteoff 10 74 0
sleep 3.436
noteoff 11 65 0
sleep 6.872
noteoff 12 65 0
noteoff 12 57 0
sleep 92.778
noteon 10 77 92
sleep 3.436
noteon 11 69 92
sleep 6.872
noteon 12 53 92
noteon 12 62 92
sleep 92.780
noteoff 10 77 0
sleep 3.436
noteoff 11 69 0
sleep 6.872
noteoff 12 62 0
noteoff 12 53 0
sleep 85.906
noteoff 0 81 0
sleep 6.872
echo "127200 tempo_s=269 tempo_l=0.25"
sleep 1.858
noteon 0 79 101
sleep 1.858
noteoff 1 69 0
noteon 1 67 100
sleep 16.728
noteon 13 43 104
sleep 1.858
noteoff 3 57 0
noteon 3 55 100
sleep 3.717
noteon 14 31 106
sleep 197.014
noteon 10 75 92
sleep 3.717
noteon 11 70 92
sleep 7.434
noteon 12 51 92
noteon 12 58 92
sleep 9.293
noteoff 13 43 0
sleep 5.576
noteoff 14 31 0
sleep 85.495
noteoff 10 75 0
sleep 3.717
noteoff 11 70 0
sleep 7.434
noteoff 12 58 0
noteoff 12 51 0
sleep 100.368
echo "127440 tempo_s=294 tempo_l=0.25"
noteon 10 70 92
sleep 1.7
noteoff 0 79 0
noteon 0 81 101
sleep 1.7
noteoff 1 67 0
noteon 11 67 92
noteon 1 69 100
sleep 6.802
noteon 12 58 92
noteon 12 63 92
sleep 10.203
noteoff 3 55 0
noteon 3 57 100
sleep 81.629
noteoff 10 70 0
sleep 3.401
noteoff 11 67 0
sleep 6.802
noteoff 12 63 0
noteoff 12 58 0
sleep 91.833
noteon 10 75 92
sleep 1.7
noteoff 0 81 0
noteon 0 82 101
sleep 1.7
noteoff 1 69 0
noteon 1 70 100
noteon 11 70 92
sleep 6.802
noteon 12 51 92
noteon 12 58 92
sleep 10.203
noteoff 3 57 0
noteon 3 58 100
sleep 81.628
noteoff 10 75 0
sleep 3.401
noteoff 11 70 0
sleep 6.802
noteoff 12 58 0
noteoff 12 51 0
sleep 85.030
noteoff 0 82 0
sleep 1.7
noteoff 1 70 0
sleep 5.102
echo "127680 tempo_s=276 tempo_l=0.25"
sleep 1.811
noteon 0 74 101
sleep 1.811
noteon 1 62 100
sleep 9.057
noteoff 3 58 0
sleep 7.246
noteon 13 45 104
sleep 1.811
noteon 3 50 100
sleep 3.623
noteon 14 33 106
sleep 192.020
noteon 10 74 92
sleep 3.622
noteon 11 69 92
sleep 7.246
noteon 12 57 92
noteon 12 53 92
sleep 9.057
noteoff 13 45 0
sleep 5.434
noteoff 14 33 0
sleep 83.328
noteoff 10 74 0
sleep 3.623
noteoff 11 69 0
sleep 7.246
noteoff 12 53 0
noteoff 12 57 0
sleep 97.821
echo "127920 tempo_s=296 tempo_l=0.25"
noteon 10 69 92
sleep 3.378
noteon 11 65 92
sleep 6.756
noteon 12 57 92
noteon 12 62 92
sleep 91.212
noteoff 10 69 0
sleep 3.378
noteoff 11 65 0
sleep 6.756
noteoff 12 62 0
noteoff 12 57 0
sleep 91.210
noteon 10 74 92
sleep 3.378
noteon 11 69 92
sleep 6.756
noteon 12 57 92
noteon 12 53 92
sleep 91.213
noteoff 10 74 0
sleep 3.378
noteoff 11 69 0
sleep 6.756
noteoff 12 53 0
noteoff 12 57 0
sleep 91.211
echo "128160 tempo_s=282 tempo_l=0.25"
sleep 1.773
noteoff 0 74 0
noteon 0 73 101
sleep 1.773
noteoff 1 62 0
noteon 1 61 100
sleep 15.957
noteon 13 45 104
sleep 1.773
noteoff 3 50 0
noteon 3 49 100
sleep 3.546
noteon 14 33 106
sleep 187.939
noteon 10 73 92
sleep 3.546
noteon 11 69 92
sleep 7.092
noteon 12 57 92
noteon 12 52 92
sleep 8.865
noteoff 13 45 0
sleep 5.319
noteoff 14 33 0
sleep 81.558
noteoff 10 73 0
sleep 3.546
noteoff 11 69 0
sleep 7.092
noteoff 12 52 0
noteoff 12 57 0
sleep 95.742
echo "128400 tempo_s=297 tempo_l=0.25"
noteon 10 69 92
sleep 3.366
noteon 11 64 92
sleep 6.734
noteon 12 61 92
noteon 12 57 92
sleep 90.904
noteoff 10 69 0
sleep 3.367
noteoff 11 64 0
sleep 6.734
noteoff 12 57 0
noteoff 12 61 0
sleep 90.908
noteon 10 73 92
sleep 3.367
noteon 11 69 92
sleep 6.734
noteon 12 52 92
noteon 12 57 92
sleep 82.488
echo "128575 tempo_s=242 tempo_l=0.25"
sleep 10.330
noteoff 10 73 0
sleep 4.132
noteoff 11 69 0
sleep 8.264
noteoff 12 57 0
noteoff 12 52 0
sleep 103.302
noteoff 0 73 0
sleep 2.066
noteoff 1 61 0
sleep 6.198
echo "128640 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
sleep 1.798
noteon 0 86 101
noteon 0 81 101
sleep 1.798
noteon 1 74 100
noteon 1 81 100
noteon 4 62 100
noteon 11 69 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 69 101
sleep 1.798
noteon 5 50 100
noteon 12 54 102
sleep 1.798
noteoff 3 49 0
sleep 7.194
noteon 13 42 104
sleep 1.798
noteon 3 42 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 30 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 69 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 69 102
sleep 7.194
noteoff 12 54 0
sleep 8.992
noteoff 13 42 0
sleep 1.798
noteoff 3 42 0
sleep 3.597
noteoff 14 30 0
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 69 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 69 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteon 14 33 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 69 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 69 102
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
sleep 3.597
noteoff 14 33 0
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 69 0
sleep 14.388
echo "128880 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 3.225
noteon 14 38 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 69 0
sleep 12.903
noteon 10 81 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 50 0
sleep 1.612
noteoff 3 50 0
sleep 3.225
noteoff 14 38 0
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 69 0
sleep 12.903
noteon 10 81 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteon 12 66 102
sleep 8.064
noteon 13 54 104
sleep 1.612
noteon 3 54 100
sleep 3.225
noteon 14 42 106
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 69 0
sleep 12.903
noteon 10 81 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteoff 12 66 0
sleep 8.064
noteoff 13 54 0
sleep 1.612
noteoff 3 54 0
sleep 3.225
noteoff 14 42 0
sleep 58.064
noteoff 10 81 0
sleep 3.225
noteoff 11 69 0
sleep 6.451
noteoff 0 81 0
sleep 1.612
noteoff 1 81 0
sleep 4.838
echo "129120 tempo_s=278 tempo_l=0.25"
noteoff 2 69 0
noteon 10 78 102
sleep 1.798
noteon 0 78 101
sleep 1.798
noteon 1 78 100
noteon 11 66 102
sleep 5.395
noteon 2 66 101
sleep 1.798
noteon 12 69 102
sleep 8.991
noteon 13 57 104
sleep 1.798
noteon 3 57 100
sleep 3.596
noteon 14 45 106
sleep 64.729
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 14.385
noteon 10 78 102
sleep 3.596
noteon 11 66 102
sleep 86.309
noteoff 10 78 0
sleep 3.596
noteoff 11 66 0
sleep 14.384
noteon 10 78 102
sleep 3.596
noteon 11 66 102
sleep 86.328
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 14.388
noteon 10 78 102
sleep 3.597
noteon 11 66 102
sleep 86.33
noteoff 10 78 0
sleep 3.597
noteoff 11 66 0
sleep 14.388
echo "129360 tempo_s=310 tempo_l=0.25"
noteon 10 78 102
sleep 3.225
noteon 11 66 102
sleep 77.419
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 12.903
noteon 10 78 102
sleep 3.225
noteon 11 66 102
sleep 70.967
noteoff 12 69 0
sleep 6.451
noteoff 10 78 0
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteoff 3 57 0
noteoff 11 66 0
sleep 3.225
noteoff 14 45 0
sleep 3.225
noteoff 0 78 0
sleep 1.612
noteoff 1 78 0
sleep 4.838
noteoff 2 66 0
noteon 10 74 102
sleep 1.612
noteon 0 74 101
sleep 1.612
noteon 1 74 100
noteon 11 62 102
sleep 4.838
noteon 2 62 101
sleep 1.612
noteon 12 54 102
sleep 8.064
noteon 13 42 104
sleep 1.612
noteon 3 42 100
sleep 3.225
noteon 14 30 106
sleep 58.064
noteoff 10 74 0
sleep 3.225
noteoff 11 62 0
sleep 12.903
noteon 10 74 102
sleep 3.225
noteon 11 62 102
sleep 6.451
noteoff 12 54 0
sleep 8.064
noteoff 13 42 0
sleep 1.612
noteoff 3 42 0
sleep 3.225
noteoff 14 30 0
sleep 58.064
noteoff 10 74 0
sleep 3.225
noteoff 11 62 0
sleep 6.451
noteoff 0 74 0
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
noteoff 4 62 0
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
echo "129600 tempo_s=278 tempo_l=0.25"
noteoff 2 62 0
noteon 10 83 102
sleep 1.798
noteoff 5 50 0
noteon 0 83 101
noteon 0 86 101
sleep 1.798
noteon 1 74 100
noteon 1 83 100
noteon 4 64 100
noteon 11 71 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 71 101
sleep 1.798
noteon 5 62 100
noteon 12 55 102
sleep 8.992
noteon 13 43 104
sleep 1.798
noteon 3 43 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 31 106
sleep 64.742
noteoff 10 83 0
sleep 3.597
noteoff 11 71 0
sleep 14.387
noteon 10 83 102
sleep 3.596
noteon 11 71 102
sleep 7.193
noteoff 12 55 0
sleep 8.992
noteoff 13 43 0
sleep 1.798
noteoff 3 43 0
sleep 3.597
noteoff 14 31 0
sleep 64.744
noteoff 10 83 0
sleep 3.596
noteoff 11 71 0
sleep 14.387
noteon 10 83 102
sleep 3.597
noteon 11 71 102
sleep 7.194
noteon 12 59 102
sleep 8.992
noteon 13 47 104
sleep 1.798
noteon 3 47 100
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteon 14 35 106
sleep 64.748
noteoff 10 83 0
sleep 3.597
noteoff 11 71 0
sleep 14.388
noteon 10 83 102
sleep 3.597
noteon 11 71 102
sleep 7.194
noteoff 12 59 0
sleep 8.992
noteoff 13 47 0
sleep 1.798
noteoff 3 47 0
sleep 3.597
noteoff 14 35 0
sleep 64.748
noteoff 10 83 0
sleep 3.597
noteoff 11 71 0
sleep 14.388
echo "129840 tempo_s=310 tempo_l=0.25"
noteon 10 83 102
sleep 3.225
noteon 11 71 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 3.225
noteon 14 38 106
sleep 58.064
noteoff 10 83 0
sleep 3.225
noteoff 11 71 0
sleep 12.903
noteon 10 83 102
sleep 3.225
noteon 11 71 102
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 50 0
sleep 1.612
noteoff 3 50 0
sleep 3.225
noteoff 14 38 0
sleep 58.064
noteoff 10 83 0
sleep 3.225
noteoff 11 71 0
sleep 12.903
noteon 10 83 102
sleep 3.225
noteon 11 71 102
sleep 6.451
noteon 12 67 102
sleep 8.064
noteon 13 55 104
sleep 1.612
noteon 3 55 100
sleep 3.225
noteon 14 43 106
sleep 58.064
noteoff 10 83 0
sleep 3.225
noteoff 11 71 0
sleep 12.903
noteon 10 83 102
sleep 3.225
noteon 11 71 102
sleep 6.451
noteoff 12 67 0
sleep 8.064
noteoff 13 55 0
sleep 1.612
noteoff 3 55 0
sleep 3.225
noteoff 14 43 0
sleep 58.064
noteoff 10 83 0
sleep 3.225
noteoff 11 71 0
sleep 6.451
noteoff 0 83 0
sleep 1.612
noteoff 1 83 0
sleep 4.838
echo "130080 tempo_s=278 tempo_l=0.25"
noteoff 2 71 0
noteon 10 79 102
sleep 1.798
noteon 0 79 101
sleep 1.798
noteon 1 79 100
noteon 11 67 102
sleep 5.395
noteon 2 67 101
sleep 1.798
noteon 12 71 102
sleep 8.991
noteon 13 59 104
sleep 1.798
noteon 3 59 100
sleep 3.596
noteon 14 47 106
sleep 64.729
noteoff 10 79 0
sleep 3.597
noteoff 11 67 0
sleep 14.385
noteon 10 79 102
sleep 3.596
noteon 11 67 102
sleep 86.309
noteoff 10 79 0
sleep 3.596
noteoff 11 67 0
sleep 14.384
noteon 10 79 102
sleep 3.596
noteon 11 67 102
sleep 86.328
noteoff 10 79 0
sleep 3.597
noteoff 11 67 0
sleep 14.388
noteon 10 79 102
sleep 3.597
noteon 11 67 102
sleep 86.33
noteoff 10 79 0
sleep 3.597
noteoff 11 67 0
sleep 14.388
echo "130320 tempo_s=310 tempo_l=0.25"
noteon 10 79 102
sleep 3.225
noteon 11 67 102
sleep 77.419
noteoff 10 79 0
sleep 3.225
noteoff 11 67 0
sleep 12.903
noteon 10 79 102
sleep 3.225
noteon 11 67 102
sleep 70.967
noteoff 12 71 0
sleep 6.451
noteoff 10 79 0
sleep 1.612
noteoff 13 59 0
sleep 1.612
noteoff 3 59 0
noteoff 11 67 0
sleep 3.225
noteoff 14 47 0
sleep 3.225
noteoff 0 79 0
sleep 1.612
noteoff 1 79 0
sleep 4.838
noteoff 2 67 0
noteon 10 76 102
sleep 1.612
noteon 0 76 101
sleep 1.612
noteon 1 76 100
noteon 11 64 102
sleep 4.838
noteon 2 64 101
sleep 1.612
noteon 12 56 102
sleep 8.064
noteon 13 44 104
sleep 1.612
noteon 3 44 100
sleep 3.225
noteon 14 32 106
sleep 58.064
noteoff 10 76 0
sleep 3.225
noteoff 11 64 0
sleep 12.903
noteon 10 76 102
sleep 3.225
noteon 11 64 102
sleep 6.451
noteoff 12 56 0
sleep 8.064
noteoff 13 44 0
sleep 1.612
noteoff 3 44 0
sleep 3.225
noteoff 14 32 0
sleep 58.064
noteoff 10 76 0
sleep 3.225
noteoff 11 64 0
sleep 6.451
noteoff 0 76 0
noteoff 0 86 0
sleep 1.612
noteoff 1 76 0
noteoff 1 74 0
noteoff 4 64 0
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
echo "130560 tempo_s=278 tempo_l=0.25"
noteoff 2 64 0
noteon 10 81 102
sleep 1.798
noteoff 5 62 0
noteon 0 81 101
noteon 0 86 101
sleep 1.798
noteon 1 74 100
noteon 1 81 100
noteon 4 66 100
noteon 11 69 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 69 101
sleep 1.798
noteon 5 62 100
noteon 12 57 102
sleep 8.990
noteon 13 45 104
sleep 1.798
noteon 3 45 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 64.728
noteoff 10 81 0
sleep 3.596
noteoff 11 69 0
sleep 14.384
noteon 10 81 102
sleep 3.596
noteon 11 69 102
sleep 7.192
noteoff 12 57 0
sleep 8.990
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
sleep 3.596
noteoff 14 33 0
sleep 64.728
noteoff 10 81 0
sleep 3.596
noteoff 11 69 0
sleep 14.384
noteon 10 81 102
sleep 3.596
noteon 11 69 102
sleep 7.192
noteon 12 69 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteon 3 57 100
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 14 45 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 69 0
sleep 7.194
noteoff 12 69 0
sleep 7.194
noteon 10 81 102
sleep 3.597
noteon 11 69 102
sleep 16.187
noteoff 13 57 0
sleep 1.798
noteoff 3 57 0
sleep 3.597
noteoff 14 45 0
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 69 0
sleep 7.194
noteoff 0 81 0
sleep 1.798
noteoff 1 81 0
sleep 5.395
echo "130800 tempo_s=310 tempo_l=0.25"
noteoff 2 69 0
noteon 10 78 102
sleep 1.612
noteon 0 78 101
sleep 1.612
noteon 1 78 100
noteon 11 66 102
sleep 4.838
noteon 2 66 101
sleep 72.58
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 12.903
noteon 10 78 102
sleep 3.225
noteon 11 66 102
sleep 77.419
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 6.451
noteoff 0 78 0
sleep 1.612
noteoff 1 78 0
sleep 4.838
noteoff 2 66 0
noteon 10 74 102
sleep 1.612
noteon 0 74 101
sleep 1.612
noteon 1 74 100
noteon 11 62 102
sleep 4.838
noteon 2 62 101
sleep 1.612
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 1.612
noteon 3 45 100
sleep 3.225
noteon 14 33 106
sleep 58.064
noteoff 10 74 0
sleep 3.225
noteoff 11 62 0
sleep 12.903
noteon 10 74 102
sleep 3.225
noteon 11 62 102
sleep 6.451
noteoff 12 57 0
sleep 8.064
noteoff 13 45 0
sleep 1.612
noteoff 3 45 0
sleep 3.225
noteoff 14 33 0
sleep 58.064
noteoff 10 74 0
sleep 3.225
noteoff 11 62 0
sleep 6.451
noteoff 0 74 0
noteoff 0 86 0
sleep 1.612
noteoff 1 74 0
noteoff 4 66 0
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
echo "131040 tempo_s=278 tempo_l=0.25"
noteoff 2 62 0
noteon 10 81 102
sleep 1.798
noteoff 5 62 0
noteon 0 81 101
noteon 0 85 101
sleep 1.798
noteon 1 73 100
noteon 1 81 100
noteon 4 64 100
noteon 11 69 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 69 101
sleep 1.798
noteon 5 57 100
noteon 12 55 102
sleep 8.990
noteon 13 43 104
sleep 1.798
noteon 3 43 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 31 106
sleep 64.728
noteoff 10 81 0
sleep 3.596
noteoff 11 69 0
sleep 14.384
noteon 10 81 102
sleep 3.596
noteon 11 69 102
sleep 7.192
noteoff 12 55 0
sleep 8.990
noteoff 13 43 0
sleep 1.798
noteoff 3 43 0
sleep 3.596
noteoff 14 31 0
sleep 64.728
noteoff 10 81 0
sleep 3.596
noteoff 11 69 0
sleep 14.384
noteon 10 81 102
sleep 3.596
noteon 11 69 102
sleep 7.192
noteon 12 67 102
sleep 8.992
noteon 13 55 104
sleep 1.798
noteon 3 55 100
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 14 43 106
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 69 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 69 102
sleep 7.194
noteoff 12 67 0
sleep 8.992
noteoff 13 55 0
sleep 1.798
noteoff 3 55 0
sleep 3.597
noteoff 14 43 0
sleep 64.748
noteoff 10 81 0
sleep 3.597
noteoff 11 69 0
sleep 7.194
noteoff 0 81 0
sleep 1.798
noteoff 1 81 0
sleep 5.395
echo "131280 tempo_s=310 tempo_l=0.25"
noteoff 2 69 0
noteon 10 76 102
sleep 1.612
noteon 0 76 101
sleep 1.612
noteon 1 76 100
noteon 11 64 102
sleep 4.838
noteon 2 64 101
sleep 72.58
noteoff 10 76 0
sleep 3.225
noteoff 11 64 0
sleep 12.903
noteon 10 76 102
sleep 3.225
noteon 11 64 102
sleep 77.419
noteoff 10 76 0
sleep 3.225
noteoff 11 64 0
sleep 6.451
noteoff 0 76 0
sleep 1.612
noteoff 1 76 0
sleep 4.838
noteoff 2 64 0
noteon 10 73 102
sleep 1.612
noteon 0 73 101
sleep 1.612
noteon 1 73 100
noteon 11 61 102
sleep 4.838
noteon 2 61 101
sleep 1.612
noteon 12 55 102
sleep 8.064
noteon 13 43 104
sleep 1.612
noteon 3 43 100
sleep 3.225
noteon 14 31 106
sleep 58.064
noteoff 10 73 0
sleep 3.225
noteoff 11 61 0
sleep 12.903
echo "131460 tempo_s=255 tempo_l=0.25"
noteon 10 73 102
sleep 3.921
noteon 11 61 102
sleep 7.843
noteoff 12 55 0
sleep 9.803
noteoff 13 43 0
sleep 1.96
noteoff 3 43 0
sleep 3.921
noteoff 14 31 0
sleep 70.588
noteoff 10 73 0
sleep 3.921
noteoff 11 61 0
sleep 7.843
noteoff 0 73 0
noteoff 0 85 0
sleep 1.96
noteoff 1 73 0
noteoff 4 64 0
sleep 1.96
noteoff 6 69 0
noteoff 6 57 0
sleep 3.921
echo "131520 tempo_s=278 tempo_l=0.25"
noteoff 2 61 0
noteon 10 86 102
noteon 10 74 102
sleep 1.798
noteoff 5 57 0
noteon 0 86 101
noteon 0 81 101
sleep 1.798
noteon 1 69 100
noteon 1 81 100
noteon 4 69 100
noteon 11 62 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 81 101
noteon 2 69 101
sleep 1.798
noteon 5 57 100
noteon 12 54 102
sleep 8.992
noteon 13 42 104
sleep 1.798
noteon 3 54 100
noteon 3 57 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 30 106
sleep 64.748
noteoff 10 74 0
noteoff 10 86 0
sleep 3.597
noteoff 11 62 0
sleep 14.388
noteon 10 86 102
noteon 10 74 102
sleep 3.597
noteon 11 81 102
sleep 7.194
noteoff 12 54 0
sleep 8.992
noteoff 13 42 0
sleep 5.395
noteoff 14 30 0
sleep 64.748
noteoff 10 74 0
noteoff 10 86 0
sleep 3.597
noteoff 11 81 0
sleep 14.388
noteon 10 74 102
noteon 10 86 102
sleep 3.597
noteon 11 81 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 3.597
noteoff 15 50 0
sleep 1.798
noteon 14 33 106
sleep 64.748
noteoff 10 86 0
noteoff 10 74 0
sleep 3.597
noteoff 11 81 0
sleep 14.388
noteon 10 86 102
noteon 10 74 102
sleep 3.597
noteon 11 81 102
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 5.395
noteoff 14 33 0
sleep 64.748
noteoff 10 74 0
noteoff 10 86 0
sleep 3.597
noteoff 11 81 0
sleep 14.388
echo "131760 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteon 11 81 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 81 0
sleep 12.903
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteon 11 81 102
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 81 0
sleep 12.903
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteon 11 81 102
sleep 6.451
noteon 12 66 102
sleep 8.064
noteon 13 54 104
sleep 4.838
noteon 14 42 106
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 81 0
sleep 12.903
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteon 11 81 102
sleep 6.451
noteoff 12 66 0
sleep 8.064
noteoff 13 54 0
sleep 4.838
noteoff 14 42 0
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 81 0
sleep 8.064
noteoff 1 81 0
noteoff 1 69 0
noteoff 4 69 0
sleep 4.838
echo "132000 tempo_s=278 tempo_l=0.25"
noteoff 2 69 0
noteoff 2 81 0
noteon 10 86 102
noteon 10 74 102
sleep 1.798
noteoff 5 57 0
sleep 1.798
noteon 1 78 100
noteon 1 66 100
noteon 4 66 100
noteon 11 78 102
sleep 5.395
noteon 2 78 101
noteon 2 66 101
sleep 1.798
noteon 5 54 100
noteon 12 69 102
sleep 8.992
noteon 13 57 104
sleep 5.395
noteon 14 45 106
sleep 64.736
noteoff 10 74 0
noteoff 10 86 0
sleep 3.597
noteoff 11 78 0
sleep 14.385
noteon 10 86 102
noteon 10 74 102
sleep 3.596
noteon 11 78 102
sleep 86.313
noteoff 10 74 0
noteoff 10 86 0
sleep 3.596
noteoff 11 78 0
sleep 14.385
noteon 10 74 102
noteon 10 86 102
sleep 3.596
noteon 11 78 102
sleep 86.328
noteoff 10 86 0
noteoff 10 74 0
sleep 3.597
noteoff 11 78 0
sleep 14.388
noteon 10 86 102
noteon 10 74 102
sleep 3.597
noteon 11 78 102
sleep 86.33
noteoff 10 74 0
noteoff 10 86 0
sleep 3.597
noteoff 11 78 0
sleep 14.388
echo "132240 tempo_s=310 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 11 78 102
sleep 77.419
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 78 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 11 78 102
sleep 70.967
noteoff 12 69 0
sleep 6.451
noteoff 10 74 0
noteoff 10 86 0
sleep 1.612
noteoff 13 57 0
sleep 1.612
noteoff 11 78 0
sleep 3.225
noteoff 14 45 0
sleep 4.838
noteoff 1 66 0
noteoff 1 78 0
noteoff 4 66 0
sleep 4.838
noteoff 2 66 0
noteoff 2 78 0
noteon 10 86 102
noteon 10 74 102
sleep 1.612
noteoff 5 54 0
sleep 1.612
noteon 1 74 100
noteon 1 62 100
noteon 4 62 100
noteon 11 74 102
sleep 4.838
noteon 2 62 101
noteon 2 74 101
sleep 1.612
noteon 5 50 100
noteon 12 54 102
sleep 8.064
noteon 13 42 104
sleep 4.838
noteon 14 30 106
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 74 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 11 74 102
sleep 6.451
noteoff 12 54 0
sleep 8.064
noteoff 13 42 0
sleep 4.838
noteoff 14 30 0
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 74 0
sleep 6.451
noteoff 0 81 0
noteoff 0 86 0
sleep 1.612
noteoff 1 62 0
noteoff 1 74 0
noteoff 4 62 0
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
echo "132480 tempo_s=278 tempo_l=0.25"
noteoff 2 74 0
noteoff 2 62 0
noteon 10 74 102
noteon 10 86 102
sleep 1.798
noteoff 5 50 0
noteon 0 86 101
noteon 0 83 101
sleep 1.798
noteon 1 83 100
noteon 1 71 100
noteon 4 62 100
noteon 11 83 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 71 101
sleep 1.798
noteon 5 50 100
noteon 12 55 102
sleep 8.990
noteon 13 43 104
sleep 1.798
noteoff 3 57 0
noteoff 3 54 0
noteon 3 59 100
noteon 3 47 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 31 106
sleep 64.728
noteoff 10 86 0
noteoff 10 74 0
sleep 3.596
noteoff 11 83 0
sleep 14.384
noteon 10 74 102
noteon 10 86 102
sleep 3.596
noteon 11 83 102
sleep 7.192
noteoff 12 55 0
sleep 8.990
noteoff 13 43 0
sleep 5.394
noteoff 14 31 0
sleep 64.728
noteoff 10 86 0
noteoff 10 74 0
sleep 3.596
noteoff 11 83 0
sleep 14.384
noteon 10 86 102
noteon 10 74 102
sleep 3.596
noteon 11 83 102
sleep 7.192
noteon 12 59 102
sleep 8.992
noteon 13 47 104
sleep 3.597
noteoff 15 50 0
sleep 1.798
noteon 14 35 106
sleep 64.748
noteoff 10 74 0
noteoff 10 86 0
sleep 3.597
noteoff 11 83 0
sleep 14.388
noteon 10 86 102
noteon 10 74 102
sleep 3.597
noteon 11 83 102
sleep 7.194
noteoff 12 59 0
sleep 8.992
noteoff 13 47 0
sleep 5.395
noteoff 14 35 0
sleep 64.748
noteoff 10 74 0
noteoff 10 86 0
sleep 3.597
noteoff 11 83 0
sleep 14.388
echo "132720 tempo_s=310 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 11 83 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 50 104
sleep 4.838
noteon 14 38 106
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 83 0
sleep 12.903
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteon 11 83 102
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 83 0
sleep 12.903
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteon 11 83 102
sleep 6.451
noteon 12 67 102
sleep 8.064
noteon 13 55 104
sleep 4.838
noteon 14 43 106
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 83 0
sleep 12.903
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteon 11 83 102
sleep 6.451
noteoff 12 67 0
sleep 8.064
noteoff 13 55 0
sleep 4.838
noteoff 14 43 0
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 83 0
sleep 8.064
noteoff 1 71 0
noteoff 1 83 0
sleep 4.838
echo "132960 tempo_s=278 tempo_l=0.25"
noteoff 2 71 0
noteon 10 86 102
noteon 10 74 102
sleep 3.597
noteon 1 79 100
noteon 1 67 100
noteon 11 79 102
sleep 5.395
noteon 2 67 101
sleep 1.798
noteon 12 71 102
sleep 8.992
noteon 13 59 104
sleep 1.798
noteoff 3 47 0
noteoff 3 59 0
noteon 3 47 100
noteon 3 55 100
sleep 3.597
noteon 14 47 106
sleep 64.736
noteoff 10 74 0
noteoff 10 86 0
sleep 3.597
noteoff 11 79 0
sleep 14.385
noteon 10 86 102
noteon 10 74 102
sleep 3.596
noteon 11 79 102
sleep 86.313
noteoff 10 74 0
noteoff 10 86 0
sleep 3.596
noteoff 11 79 0
sleep 14.385
noteon 10 74 102
noteon 10 86 102
sleep 3.596
noteon 11 79 102
sleep 86.328
noteoff 10 86 0
noteoff 10 74 0
sleep 3.597
noteoff 11 79 0
sleep 14.388
noteon 10 86 102
noteon 10 74 102
sleep 3.597
noteon 11 79 102
sleep 86.33
noteoff 10 74 0
noteoff 10 86 0
sleep 3.597
noteoff 11 79 0
sleep 14.388
echo "133200 tempo_s=310 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 11 79 102
sleep 77.419
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 79 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 11 79 102
sleep 70.967
noteoff 12 71 0
sleep 6.451
noteoff 10 74 0
noteoff 10 86 0
sleep 1.612
noteoff 13 59 0
sleep 1.612
noteoff 11 79 0
sleep 3.225
noteoff 14 47 0
sleep 4.838
noteoff 1 67 0
noteoff 1 79 0
sleep 4.838
noteoff 2 67 0
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteon 1 74 100
noteon 1 62 100
noteon 11 74 102
sleep 4.838
noteon 2 62 101
sleep 1.612
noteon 12 56 102
sleep 8.064
noteon 13 44 104
sleep 1.612
noteoff 3 55 0
noteoff 3 47 0
noteon 3 50 100
noteon 3 44 100
sleep 3.225
noteon 14 32 106
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 74 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 11 74 102
sleep 6.451
noteoff 12 56 0
sleep 8.064
noteoff 13 44 0
sleep 4.838
noteoff 14 32 0
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 74 0
sleep 6.451
noteoff 0 83 0
noteoff 0 86 0
sleep 1.612
noteoff 1 62 0
noteoff 1 74 0
noteoff 4 62 0
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 3.225
echo "133440 tempo_s=287 tempo_l=0.25"
noteoff 2 62 0
noteon 10 74 102
noteon 10 86 102
sleep 1.742
noteoff 5 50 0
noteon 0 86 101
noteon 0 84 101
sleep 1.742
noteon 1 84 100
noteon 1 74 100
noteon 4 62 100
noteon 11 74 102
noteon 11 84 102
sleep 1.742
noteon 6 62 108
noteon 6 74 108
sleep 3.484
noteon 2 72 101
noteon 2 62 101
sleep 1.742
noteon 5 50 100
noteon 12 57 102
sleep 8.71
noteon 13 45 104
sleep 1.742
noteoff 3 44 0
noteoff 3 50 0
noteon 3 60 100
noteon 3 57 100
sleep 1.742
noteon 15 50 80
sleep 1.742
noteon 14 33 106
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 84 0
noteoff 11 74 0
sleep 13.937
noteon 10 86 102
noteon 10 74 102
sleep 3.484
noteon 11 74 102
noteon 11 84 102
sleep 6.968
noteoff 12 57 0
sleep 8.71
noteoff 13 45 0
sleep 5.226
noteoff 14 33 0
sleep 62.717
noteoff 10 74 0
noteoff 10 86 0
sleep 3.484
noteoff 11 84 0
noteoff 11 74 0
sleep 13.937
noteon 10 74 102
noteon 10 86 102
sleep 3.484
noteon 11 74 102
noteon 11 84 102
sleep 6.968
noteon 12 60 102
sleep 8.71
noteon 13 48 104
sleep 3.484
noteoff 15 50 0
sleep 1.742
noteon 14 36 106
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 84 0
noteoff 11 74 0
sleep 13.937
noteon 10 86 102
noteon 10 74 102
sleep 3.484
noteon 11 84 102
noteon 11 74 102
sleep 6.968
noteoff 12 60 0
sleep 8.71
noteoff 13 48 0
sleep 5.226
noteoff 14 36 0
sleep 62.717
noteoff 10 74 0
noteoff 10 86 0
sleep 3.484
noteoff 11 74 0
noteoff 11 84 0
sleep 13.937
echo "133680 tempo_s=310 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 11 84 102
noteon 11 74 102
sleep 6.451
noteon 12 66 102
sleep 8.064
noteon 13 54 104
sleep 4.838
noteon 14 42 106
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 74 0
noteoff 11 84 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 11 74 102
noteon 11 84 102
sleep 6.451
noteoff 12 66 0
sleep 8.064
noteoff 13 54 0
sleep 4.838
noteoff 14 42 0
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 84 0
noteoff 11 74 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 11 74 102
noteon 11 84 102
sleep 6.451
noteon 12 69 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 84 0
noteoff 11 74 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 11 84 102
noteon 11 74 102
sleep 6.451
noteoff 12 69 0
sleep 8.064
noteoff 13 57 0
sleep 4.838
noteoff 14 45 0
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 74 0
noteoff 11 84 0
sleep 8.064
noteoff 1 74 0
noteoff 1 84 0
sleep 4.838
echo "133920 tempo_s=287 tempo_l=0.25"
noteon 10 74 102
noteon 10 86 102
sleep 3.484
noteon 1 69 100
noteon 11 74 102
noteon 11 84 102
sleep 6.968
noteon 12 72 102
sleep 8.71
noteon 13 60 104
sleep 3.484
noteon 15 50 80
sleep 1.742
noteon 14 48 106
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 84 0
noteoff 11 74 0
sleep 13.937
noteon 10 74 102
noteon 10 86 102
sleep 3.484
noteoff 1 69 0
noteon 11 84 102
noteon 11 74 102
sleep 6.968
noteoff 12 72 0
sleep 8.71
noteoff 13 60 0
sleep 5.226
noteoff 14 48 0
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 74 0
noteoff 11 84 0
sleep 13.937
noteon 10 74 102
noteon 10 86 102
sleep 3.484
noteon 1 72 100
noteon 11 84 102
noteon 11 74 102
sleep 6.968
noteon 12 69 102
sleep 8.71
noteon 13 57 104
sleep 3.484
noteoff 15 50 0
sleep 1.742
noteon 14 45 106
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 74 0
noteoff 11 84 0
sleep 13.937
noteon 10 74 102
noteon 10 86 102
sleep 3.484
noteoff 1 72 0
noteon 11 74 102
noteon 11 84 102
sleep 6.968
noteoff 12 69 0
sleep 8.71
noteoff 13 57 0
sleep 5.226
noteoff 14 45 0
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 84 0
noteoff 11 74 0
sleep 13.937
echo "134160 tempo_s=310 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 1 78 100
noteon 11 74 102
noteon 11 84 102
sleep 6.451
noteon 12 66 102
sleep 8.064
noteon 13 54 104
sleep 4.838
noteon 14 42 106
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 84 0
noteoff 11 74 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteoff 1 78 0
noteon 11 74 102
noteon 11 84 102
sleep 6.451
noteoff 12 66 0
sleep 8.064
noteoff 13 54 0
sleep 4.838
noteoff 14 42 0
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 84 0
noteoff 11 74 0
sleep 12.903
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteon 1 81 100
noteon 11 74 102
noteon 11 84 102
sleep 6.451
noteon 12 60 102
sleep 8.064
noteon 13 48 104
sleep 4.838
noteon 14 36 106
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 84 0
noteoff 11 74 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteoff 1 81 0
noteon 11 74 102
noteon 11 84 102
sleep 6.451
noteoff 12 60 0
sleep 8.064
noteoff 13 48 0
sleep 4.838
noteoff 14 36 0
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 84 0
noteoff 11 74 0
sleep 12.903
echo "134400 tempo_s=287 tempo_l=0.25"
noteon 10 74 102
noteon 10 86 102
sleep 3.484
noteon 1 84 100
noteon 11 74 102
noteon 11 84 102
sleep 6.968
noteon 12 57 102
sleep 8.71
noteon 13 45 104
sleep 3.484
noteon 15 50 80
sleep 1.742
noteon 14 33 106
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 84 0
noteoff 11 74 0
sleep 13.937
noteon 10 86 102
noteon 10 74 102
sleep 3.484
noteoff 1 84 0
noteon 11 74 102
noteon 11 84 102
sleep 6.968
noteoff 12 57 0
sleep 8.71
noteoff 13 45 0
sleep 5.226
noteoff 14 33 0
sleep 62.717
noteoff 10 74 0
noteoff 10 86 0
sleep 3.484
noteoff 11 84 0
noteoff 11 74 0
sleep 13.937
noteon 10 74 102
noteon 10 86 102
sleep 3.484
noteon 1 81 100
noteon 11 84 102
noteon 11 74 102
sleep 6.968
noteon 12 60 102
sleep 8.71
noteon 13 48 104
sleep 3.484
noteoff 15 50 0
sleep 1.742
noteon 14 36 106
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 74 0
noteoff 11 84 0
sleep 13.937
noteon 10 74 102
noteon 10 86 102
sleep 3.484
noteoff 1 81 0
noteon 11 74 102
noteon 11 84 102
sleep 6.968
noteoff 12 60 0
sleep 8.71
noteoff 13 48 0
sleep 5.226
noteoff 14 36 0
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 84 0
noteoff 11 74 0
sleep 13.937
echo "134640 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteon 1 78 100
noteon 11 74 102
noteon 11 84 102
sleep 6.451
noteon 12 66 102
sleep 8.064
noteon 13 54 104
sleep 4.838
noteon 14 42 106
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 84 0
noteoff 11 74 0
sleep 12.903
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteoff 1 78 0
noteon 11 74 102
noteon 11 84 102
sleep 6.451
noteoff 12 66 0
sleep 8.064
noteoff 13 54 0
sleep 4.838
noteoff 14 42 0
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 84 0
noteoff 11 74 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 1 72 100
noteon 11 74 102
noteon 11 84 102
sleep 6.451
noteon 12 69 102
sleep 8.064
noteon 13 57 104
sleep 4.838
noteon 14 45 106
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 84 0
noteoff 11 74 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteoff 1 72 0
noteon 11 84 102
noteon 11 74 102
sleep 6.451
noteoff 12 69 0
sleep 8.064
noteoff 13 57 0
sleep 4.838
noteoff 14 45 0
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 74 0
noteoff 11 84 0
sleep 12.903
echo "134880 tempo_s=287 tempo_l=0.25"
noteon 10 74 102
noteon 10 86 102
sleep 3.484
noteon 1 69 100
noteon 11 74 102
noteon 11 84 102
sleep 6.968
noteon 12 72 102
sleep 8.71
noteon 13 60 104
sleep 3.484
noteon 15 50 80
sleep 1.742
noteon 14 48 106
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 84 0
noteoff 11 74 0
sleep 13.937
noteon 10 74 102
noteon 10 86 102
sleep 3.484
noteoff 1 69 0
noteon 11 84 102
noteon 11 74 102
sleep 6.968
noteoff 12 72 0
sleep 8.71
noteoff 13 60 0
sleep 5.226
noteoff 14 48 0
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 74 0
noteoff 11 84 0
sleep 13.937
noteon 10 74 102
noteon 10 86 102
sleep 3.484
noteon 1 72 100
noteon 11 74 102
noteon 11 84 102
sleep 6.968
noteon 12 69 102
sleep 8.71
noteon 13 57 104
sleep 3.484
noteoff 15 50 0
sleep 1.742
noteon 14 45 106
sleep 62.717
noteoff 10 86 0
noteoff 10 74 0
sleep 3.484
noteoff 11 84 0
noteoff 11 74 0
sleep 13.937
noteon 10 86 102
noteon 10 74 102
sleep 3.484
noteoff 1 72 0
noteon 11 74 102
noteon 11 84 102
sleep 6.968
noteoff 12 69 0
sleep 8.71
noteoff 13 57 0
sleep 5.226
noteoff 14 45 0
sleep 62.717
noteoff 10 74 0
noteoff 10 86 0
sleep 3.484
noteoff 11 84 0
noteoff 11 74 0
sleep 13.937
echo "135120 tempo_s=310 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 1 78 100
noteon 11 74 102
noteon 11 84 102
sleep 6.451
noteon 12 66 102
sleep 8.064
noteon 13 54 104
sleep 4.838
noteon 14 42 106
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 84 0
noteoff 11 74 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteoff 1 78 0
noteon 11 84 102
noteon 11 74 102
sleep 6.451
noteoff 12 66 0
sleep 8.064
noteoff 13 54 0
sleep 4.838
noteoff 14 42 0
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 74 0
noteoff 11 84 0
sleep 12.903
noteon 10 86 102
noteon 10 74 102
sleep 3.225
noteon 1 81 100
noteon 11 84 102
noteon 11 74 102
sleep 6.451
noteon 12 60 102
sleep 8.064
noteon 13 48 104
sleep 4.838
noteon 14 36 106
sleep 58.064
noteoff 10 74 0
noteoff 10 86 0
sleep 3.225
noteoff 11 74 0
noteoff 11 84 0
sleep 12.903
noteon 10 74 102
noteon 10 86 102
sleep 3.225
noteoff 1 81 0
noteon 11 74 102
noteon 11 84 102
sleep 6.451
noteoff 12 60 0
sleep 8.064
noteoff 13 48 0
sleep 4.838
noteoff 14 36 0
sleep 58.064
noteoff 10 86 0
noteoff 10 74 0
sleep 3.225
noteoff 11 84 0
noteoff 11 74 0
sleep 12.903
echo "135360 tempo_s=272 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 3.676
noteon 1 84 100
noteon 11 84 102
noteon 11 74 102
sleep 7.352
noteon 12 57 102
sleep 9.191
noteon 13 45 104
sleep 1.838
noteoff 3 57 0
noteoff 3 60 0
noteon 3 45 100
sleep 1.838
noteon 15 50 80
sleep 1.838
noteon 14 33 106
sleep 88.234
noteoff 1 84 0
sleep 18.381
noteoff 3 45 0
sleep 80.881
noteoff 0 84 0
noteoff 0 86 0
sleep 1.838
noteoff 4 62 0
sleep 1.838
noteoff 6 74 0
noteoff 6 62 0
sleep 3.676
echo "135480 tempo_s=234 tempo_l=0.25"
noteoff 2 62 0
noteoff 2 72 0
noteoff 10 74 0
noteoff 10 86 0
sleep 2.136
noteoff 5 50 0
sleep 2.136
noteoff 11 74 0
noteoff 11 84 0
sleep 8.547
noteoff 12 57 0
sleep 10.683
noteoff 13 45 0
sleep 2.136
noteon 3 48 100
sleep 2.136
noteoff 15 50 0
sleep 2.136
noteoff 14 33 0
sleep 98.287
echo "135540 tempo_s=278 tempo_l=0.25"
noteon 10 83 102
sleep 21.582
noteoff 3 48 0
sleep 86.330
echo "135600 tempo_s=290 tempo_l=0.25"
noteoff 10 83 0
noteon 10 84 102
sleep 20.688
noteon 3 54 100
sleep 103.446
noteoff 3 54 0
sleep 82.756
noteoff 10 84 0
sleep 20.689
noteon 3 57 100
sleep 82.758
noteon 10 80 102
sleep 20.689
noteoff 3 57 0
sleep 82.758
echo "135840 tempo_s=285 tempo_l=0.25"
noteoff 10 80 0
noteon 10 81 102
sleep 21.052
noteon 3 60 100
sleep 105.263
noteoff 3 60 0
sleep 84.21
noteoff 10 81 0
sleep 21.052
noteon 3 57 100
sleep 84.21
noteon 10 77 102
sleep 21.052
noteoff 3 57 0
sleep 84.21
noteoff 10 77 0
noteon 10 78 102
sleep 21.052
noteon 3 54 100
sleep 105.263
noteoff 3 54 0
sleep 84.21
noteoff 10 78 0
sleep 21.052
noteon 3 48 100
sleep 84.21
noteon 10 73 102
sleep 21.052
noteoff 3 48 0
sleep 84.21
echo "136320 tempo_s=277 tempo_l=0.25"
noteoff 10 73 0
noteon 10 74 102
sleep 21.66
noteon 3 45 100
sleep 108.303
noteoff 3 45 0
sleep 86.642
noteoff 10 74 0
sleep 21.66
noteon 3 48 100
sleep 86.642
noteon 10 71 102
sleep 21.66
noteoff 3 48 0
sleep 86.642
noteoff 10 71 0
noteon 10 72 102
sleep 21.66
noteon 3 54 100
sleep 108.303
noteoff 3 54 0
sleep 86.642
noteoff 10 72 0
sleep 21.66
noteon 3 57 100
sleep 86.642
noteon 10 68 102
sleep 21.66
noteoff 3 57 0
sleep 86.642
echo "136800 tempo_s=273 tempo_l=0.25"
noteoff 10 68 0
noteon 10 69 102
sleep 21.978
noteon 3 60 100
sleep 109.89
noteoff 3 60 0
sleep 87.912
noteoff 10 69 0
sleep 3.663
noteon 11 60 102
sleep 18.315
noteon 3 57 100
sleep 87.912
noteon 10 65 102
sleep 3.663
noteoff 11 60 0
sleep 18.315
noteoff 3 57 0
sleep 87.912
noteoff 10 65 0
noteon 10 66 102
sleep 3.663
noteon 11 66 102
sleep 18.315
noteon 3 54 100
sleep 91.575
noteoff 11 66 0
sleep 18.315
noteoff 3 54 0
sleep 87.912
noteoff 10 66 0
sleep 3.663
noteon 11 69 102
sleep 18.315
noteon 3 48 100
sleep 87.912
noteon 10 61 102
sleep 3.663
noteoff 11 69 0
sleep 18.315
noteoff 3 48 0
sleep 87.912
echo "137280 tempo_s=272 tempo_l=0.25"
noteoff 10 61 0
noteon 10 62 102
sleep 3.676
noteon 11 72 102
sleep 18.382
noteon 3 45 100
sleep 91.911
noteoff 11 72 0
sleep 18.382
noteoff 3 45 0
sleep 88.235
echo "137400 tempo_s=246 tempo_l=0.25"
noteoff 10 62 0
sleep 121.951
noteon 10 73 82
sleep 121.951
echo "137520 tempo_s=272 tempo_l=0.25"
noteoff 10 73 0
noteon 10 74 102
sleep 220.588
noteoff 10 74 0
sleep 110.294
noteon 10 77 102
sleep 110.294
echo "137760 tempo_s=271 tempo_l=0.25"
noteoff 10 77 0
noteon 10 78 102
sleep 221.402
noteoff 10 78 0
sleep 110.701
noteon 10 80 102
sleep 110.701
noteoff 10 80 0
noteon 10 81 102
sleep 221.402
noteoff 10 81 0
sleep 110.701
noteon 10 83 102
sleep 110.701
echo "138240 tempo_s=264 tempo_l=0.25"
noteoff 10 83 0
noteon 10 84 102
sleep 227.272
noteoff 10 84 0
sleep 113.636
noteon 10 83 102
sleep 113.636
echo "138480 tempo_s=259 tempo_l=0.25"
noteoff 10 83 0
noteon 10 84 102
sleep 231.66
noteoff 10 84 0
sleep 115.83
noteon 10 83 102
sleep 115.83
noteoff 10 83 0
noteon 10 84 102
sleep 231.66
echo "138840 tempo_s=251 tempo_l=0.25"
noteoff 10 84 0
sleep 119.521
noteon 10 83 102
sleep 119.521
noteoff 10 83 0
noteon 10 84 102
sleep 239.043
echo "139080 tempo_s=235 tempo_l=0.25"
noteoff 10 84 0
sleep 127.659
echo "139140 tempo_s=265 tempo_l=0.25"
noteon 10 83 102
sleep 113.207
noteoff 10 83 0
noteon 10 84 102
sleep 226.415
noteoff 10 84 0
sleep 113.207
noteon 10 66 102
sleep 94.339
noteoff 10 66 0
sleep 18.867
noteon 10 67 102
sleep 37.735
noteoff 10 67 0
noteon 10 66 102
sleep 37.735
noteoff 10 66 0
noteon 10 67 102
sleep 37.735
noteoff 10 67 0
noteon 10 66 102
sleep 37.735
noteoff 10 66 0
noteon 10 67 102
sleep 37.735
noteoff 10 67 0
noteon 10 66 102
sleep 37.735
noteoff 10 66 0
noteon 10 67 102
sleep 37.735
noteoff 10 67 0
noteon 10 66 102
sleep 37.735
noteoff 10 66 0
noteon 10 67 102
sleep 37.735
noteoff 10 67 0
noteon 10 64 102
sleep 56.603
noteoff 10 64 0
noteon 10 66 102
sleep 56.603
noteoff 10 66 0
noteon 10 69 102
sleep 3.773
noteon 1 78 100
noteon 1 81 100
noteon 4 62 100
noteon 11 66 102
sleep 7.547
noteon 5 50 100
noteon 12 62 102
sleep 9.433
noteon 13 60 104
sleep 5.66
noteon 14 48 106
sleep 86.792
noteoff 10 69 0
sleep 113.207
noteon 10 62 102
sleep 3.773
noteoff 1 81 0
noteoff 1 78 0
noteoff 4 62 0
noteoff 11 66 0
sleep 7.547
noteoff 5 50 0
noteoff 12 62 0
sleep 9.433
noteoff 13 60 0
sleep 5.66
noteoff 14 48 0
sleep 86.792
noteoff 10 62 0
sleep 113.207
echo "139920 tempo_s=180 tempo_l=0.25"
sleep 500.0
echo "140100 tempo_s=264 tempo_l=0.25"
noteon 10 85 102
sleep 113.636
noteoff 10 85 0
noteon 10 86 102
sleep 227.272
noteoff 10 86 0
sleep 113.636
noteon 10 67 102
sleep 94.696
noteoff 10 67 0
sleep 18.939
noteon 10 69 102
sleep 37.878
noteoff 10 69 0
noteon 10 67 102
sleep 37.878
noteoff 10 67 0
noteon 10 69 102
sleep 37.878
noteoff 10 69 0
noteon 10 67 102
sleep 37.878
noteoff 10 67 0
noteon 10 69 102
sleep 37.878
noteoff 10 69 0
noteon 10 67 102
sleep 37.878
noteoff 10 67 0
noteon 10 69 102
sleep 37.878
noteoff 10 69 0
noteon 10 67 102
sleep 37.878
noteoff 10 67 0
noteon 10 69 102
sleep 37.878
noteoff 10 69 0
noteon 10 66 102
sleep 56.818
noteoff 10 66 0
noteon 10 67 102
sleep 56.818
noteoff 10 67 0
noteon 10 71 102
sleep 3.787
noteon 1 79 100
noteon 1 83 100
noteon 4 62 100
noteon 11 67 102
sleep 7.575
noteon 5 50 100
noteon 12 62 102
sleep 9.469
noteon 13 59 104
sleep 5.681
noteon 14 47 106
sleep 87.121
noteoff 10 71 0
sleep 113.636
noteon 10 62 102
sleep 3.787
noteoff 1 83 0
noteoff 1 79 0
noteoff 4 62 0
noteoff 11 67 0
sleep 7.575
noteoff 5 50 0
noteoff 12 62 0
sleep 9.469
noteoff 13 59 0
sleep 5.681
noteoff 14 47 0
sleep 87.121
noteoff 10 62 0
sleep 113.636
echo "140880 tempo_s=190 tempo_l=0.25"
sleep 473.684
echo "141060 tempo_s=284 tempo_l=0.25"
noteon 10 78 102
sleep 1.76
noteon 0 90 101
sleep 1.76
noteon 1 78 100
noteon 11 66 102
sleep 17.605
noteon 3 66 100
sleep 84.507
echo "141120 tempo_s=278 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteoff 11 66 0
noteon 1 79 100
noteon 11 67 102
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 86.33
noteoff 10 79 0
sleep 1.798
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
noteoff 11 67 0
sleep 17.985
noteoff 3 67 0
sleep 194.244
noteon 10 61 102
sleep 1.798
noteon 0 73 101
sleep 1.798
noteon 1 73 100
noteon 11 61 102
sleep 7.194
noteon 12 61 102
sleep 8.992
noteon 13 49 104
sleep 1.798
noteon 3 49 100
sleep 3.597
noteon 14 37 106
sleep 64.748
noteoff 10 61 0
sleep 1.798
noteoff 0 73 0
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
sleep 7.194
noteoff 12 61 0
sleep 7.194
echo "141360 tempo_s=310 tempo_l=0.25"
noteon 10 62 102
sleep 1.612
noteoff 13 49 0
noteon 0 74 101
sleep 1.612
noteoff 3 49 0
noteon 1 74 100
noteon 11 62 102
sleep 3.225
noteoff 14 37 0
sleep 3.224
noteon 12 62 102
sleep 8.063
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 3.224
noteon 14 38 106
sleep 4.837
noteoff 0 74 0
noteon 0 73 101
sleep 4.836
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 6.448
noteoff 12 62 0
noteon 12 61 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 1.612
noteoff 0 73 0
noteon 0 74 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.450
noteoff 12 61 0
noteon 12 62 102
sleep 4.836
noteoff 0 74 0
noteon 0 73 101
sleep 8.062
noteoff 14 38 0
noteon 14 37 106
sleep 9.674
noteoff 10 62 0
noteon 10 61 102
sleep 3.224
noteoff 1 74 0
noteoff 11 61 0
noteon 1 73 100
noteon 11 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 8.062
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 12.898
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 1 73 0
noteon 1 74 100
sleep 6.448
noteoff 12 61 0
noteon 12 62 102
sleep 9.673
noteoff 11 62 0
noteon 11 61 102
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 4.836
noteoff 0 73 0
noteon 0 74 101
sleep 4.838
noteoff 10 62 0
noteon 10 61 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 1 74 0
noteoff 3 50 0
noteon 1 73 100
noteon 3 49 100
sleep 6.450
noteoff 12 62 0
noteon 12 61 102
sleep 11.286
noteoff 0 74 0
noteon 0 73 101
sleep 11.286
noteoff 10 61 0
noteon 10 62 102
sleep 3.224
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 6.449
noteoff 12 61 0
noteon 12 62 102
sleep 4.837
noteoff 0 73 0
noteon 0 74 101
sleep 3.224
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.224
noteoff 14 38 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 61 102
sleep 3.225
noteoff 1 74 0
noteon 1 73 100
sleep 4.838
noteoff 0 74 0
noteon 0 73 101
sleep 1.612
noteoff 12 62 0
noteon 12 61 102
sleep 9.677
noteoff 11 62 0
noteon 11 61 102
sleep 12.903
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 0 73 0
noteoff 13 50 0
noteon 0 74 101
noteon 13 49 104
sleep 1.612
noteoff 1 73 0
noteoff 3 50 0
noteon 1 74 100
noteon 3 49 100
sleep 6.451
noteoff 12 61 0
noteon 12 62 102
sleep 12.903
noteoff 14 37 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 59 102
sleep 1.612
noteoff 0 74 0
noteon 0 71 101
sleep 1.612
noteoff 1 74 0
noteoff 11 61 0
noteon 1 71 100
noteon 11 59 102
sleep 3.225
noteoff 14 37 0
sleep 3.225
noteoff 12 62 0
noteon 12 59 102
sleep 8.064
noteoff 13 49 0
noteon 13 47 104
sleep 1.612
noteoff 3 49 0
noteon 3 47 100
sleep 3.225
noteon 14 35 106
sleep 25.806
noteoff 10 59 0
noteon 10 61 102
sleep 1.612
noteoff 0 71 0
noteon 0 73 101
sleep 1.612
noteoff 1 71 0
noteoff 11 59 0
noteon 1 73 100
noteon 11 61 102
sleep 6.451
noteoff 12 59 0
noteon 12 61 102
sleep 8.064
noteoff 13 47 0
noteon 13 49 104
sleep 1.612
noteoff 3 47 0
noteon 3 49 100
sleep 3.225
noteoff 14 35 0
noteon 14 37 106
sleep 25.806
echo "141600 tempo_s=278 tempo_l=0.25"
noteoff 10 61 0
noteon 10 64 102
sleep 1.798
noteoff 0 73 0
noteon 0 76 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 76 100
noteon 11 64 102
sleep 7.194
noteoff 12 61 0
noteon 12 64 102
sleep 8.992
noteoff 13 49 0
noteon 13 52 104
sleep 1.798
noteoff 3 49 0
noteon 3 52 100
sleep 3.597
noteoff 14 37 0
noteon 14 40 106
sleep 82.733
noteoff 10 64 0
sleep 1.798
noteoff 0 76 0
sleep 1.798
noteoff 1 76 0
noteoff 11 64 0
sleep 7.194
noteoff 12 64 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 3.597
noteoff 14 40 0
sleep 82.733
echo "141720 tempo_s=246 tempo_l=0.25"
noteon 10 57 102
sleep 2.032
noteon 0 69 101
sleep 2.032
noteon 1 69 100
noteon 11 57 102
sleep 8.13
noteon 12 57 102
sleep 10.162
noteon 13 45 104
sleep 2.032
noteon 3 45 100
sleep 4.065
noteon 14 33 106
sleep 93.495
noteoff 10 57 0
sleep 2.032
noteoff 0 69 0
sleep 2.032
noteoff 1 69 0
noteoff 11 57 0
sleep 8.13
noteoff 12 57 0
sleep 10.162
noteoff 13 45 0
sleep 2.032
noteoff 3 45 0
sleep 4.065
noteoff 14 33 0
sleep 93.495
echo "141840 tempo_s=310 tempo_l=0.25"
sleep 193.548
noteon 10 69 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteon 12 67 102
sleep 8.064
noteon 13 67 104
sleep 30.645
noteoff 10 69 0
sleep 48.387
noteon 10 69 102
sleep 48.387
noteoff 10 69 0
sleep 48.387
echo "142080 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 10.791
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 67 0
noteon 13 66 104
sleep 88.129
noteoff 10 74 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 17.985
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
noteon 10 78 102
sleep 53.956
noteoff 10 78 0
sleep 53.956
echo "142320 tempo_s=310 tempo_l=0.25"
noteon 10 76 102
sleep 9.677
noteoff 12 66 0
noteon 12 61 102
sleep 8.064
noteoff 13 66 0
noteon 13 61 104
sleep 79.032
noteoff 10 76 0
noteon 10 69 102
sleep 80.645
noteoff 10 69 0
sleep 16.129
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 48.387
noteon 10 79 102
sleep 48.387
noteoff 10 79 0
sleep 41.935
noteoff 12 61 0
sleep 6.451
echo "142560 tempo_s=278 tempo_l=0.25"
noteon 10 78 102
sleep 1.798
noteoff 13 61 0
sleep 8.992
noteon 12 62 102
sleep 8.992
noteon 13 62 104
sleep 88.129
noteoff 10 78 0
noteon 10 69 102
sleep 89.928
noteoff 10 69 0
sleep 17.985
noteon 10 81 102
sleep 3.597
noteoff 11 69 0
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
noteon 10 81 102
sleep 3.597
noteon 11 66 102
sleep 50.359
noteoff 10 81 0
sleep 39.568
noteoff 11 66 0
sleep 14.388
echo "142800 tempo_s=310 tempo_l=0.25"
noteon 10 81 102
sleep 3.225
noteon 11 71 102
sleep 6.451
noteoff 12 62 0
noteon 12 67 102
sleep 8.064
noteoff 13 62 0
noteon 13 55 104
sleep 4.838
noteon 14 31 106
sleep 74.193
noteoff 10 81 0
noteon 10 79 102
sleep 80.645
noteoff 10 79 0
sleep 16.129
noteon 10 78 102
sleep 22.58
noteoff 14 31 0
sleep 25.806
noteoff 10 78 0
sleep 48.387
noteon 10 76 102
sleep 48.387
noteoff 10 76 0
sleep 48.387
echo "143040 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 3.597
noteoff 11 71 0
noteon 11 69 102
sleep 7.194
noteoff 12 67 0
noteon 12 66 102
sleep 8.992
noteoff 13 55 0
noteon 13 57 104
sleep 5.395
noteon 14 33 106
sleep 82.733
noteoff 10 76 0
noteon 10 74 102
sleep 89.928
noteoff 10 74 0
sleep 17.985
noteon 10 74 102
sleep 25.179
noteoff 14 33 0
sleep 28.776
noteoff 10 74 0
sleep 53.956
noteon 10 74 102
sleep 53.956
noteoff 10 74 0
sleep 53.956
echo "143280 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 3.225
noteoff 11 69 0
noteon 4 64 115
noteon 11 67 102
sleep 6.451
noteoff 12 66 0
noteon 5 57 115
noteon 12 64 102
sleep 8.064
noteoff 13 57 0
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 74.193
noteoff 10 74 0
noteon 10 73 102
sleep 80.645
noteoff 10 73 0
sleep 16.129
noteon 10 71 102
sleep 22.58
noteoff 14 33 0
sleep 25.806
noteoff 10 71 0
sleep 48.387
noteon 10 73 102
sleep 48.387
noteoff 10 73 0
sleep 35.483
noteoff 11 67 0
sleep 6.451
noteoff 12 64 0
sleep 6.451
echo "143520 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteoff 13 45 0
noteon 0 86 101
sleep 1.798
noteoff 4 64 0
noteon 1 74 100
noteon 1 66 100
noteon 11 66 102
noteon 4 62 100
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 74 101
noteon 2 66 101
sleep 1.798
noteoff 5 57 0
noteon 12 62 102
noteon 5 54 100
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 62 100
noteon 3 50 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 82.733
noteoff 10 74 0
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteoff 1 66 0
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 66 0
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 3.597
noteoff 2 66 0
noteoff 2 74 0
sleep 1.798
noteoff 5 54 0
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
noteoff 3 62 0
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 82.733
noteon 10 67 102
noteon 10 76 102
sleep 1.798
noteon 0 85 101
noteon 0 88 101
sleep 1.798
noteon 1 76 100
noteon 1 69 100
noteon 4 64 100
noteon 11 61 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 76 101
noteon 2 69 101
sleep 1.798
noteon 5 57 100
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 57 100
noteon 3 45 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 82.733
noteoff 10 76 0
noteoff 10 67 0
sleep 1.798
noteoff 0 88 0
noteoff 0 85 0
sleep 1.798
noteoff 1 69 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 11 61 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.597
noteoff 2 69 0
noteoff 2 76 0
sleep 1.798
noteoff 5 57 0
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
noteoff 3 57 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 82.733
echo "143760 tempo_s=257 tempo_l=0.25"
sleep 233.463
echo "143880 tempo_s=310 tempo_l=0.25"
sleep 96.774
noteon 10 78 102
sleep 1.612
noteon 0 90 101
sleep 1.612
noteon 1 78 100
noteon 11 66 102
sleep 16.127
noteon 3 66 100
sleep 77.418
echo "144000 tempo_s=278 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteoff 11 66 0
noteon 1 79 100
noteon 11 67 102
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 86.33
noteoff 10 79 0
sleep 1.798
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
noteoff 11 67 0
sleep 17.985
noteoff 3 67 0
sleep 86.33
echo "144120 tempo_s=246 tempo_l=0.25"
sleep 121.951
noteon 10 61 102
sleep 4.065
noteon 11 61 102
sleep 8.13
noteon 12 61 102
sleep 10.162
noteon 13 49 104
sleep 2.032
noteon 3 49 100
sleep 4.065
noteon 14 37 106
sleep 73.17
noteoff 10 61 0
sleep 4.065
noteoff 11 61 0
sleep 8.13
noteoff 12 61 0
sleep 8.13
echo "144240 tempo_s=310 tempo_l=0.25"
noteon 10 62 102
sleep 1.612
noteoff 13 49 0
sleep 1.612
noteoff 3 49 0
noteon 11 62 102
sleep 3.225
noteoff 14 37 0
sleep 3.224
noteon 12 62 102
sleep 8.063
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 3.224
noteon 14 38 106
sleep 9.674
noteoff 10 62 0
noteon 10 61 102
sleep 9.674
noteoff 12 62 0
noteon 12 61 102
sleep 9.674
noteoff 11 62 0
noteon 11 61 102
sleep 12.900
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 3 50 0
noteon 3 49 100
sleep 6.450
noteoff 12 61 0
noteon 12 62 102
sleep 12.899
noteoff 14 38 0
noteon 14 37 106
sleep 9.674
noteoff 10 62 0
noteon 10 61 102
sleep 3.224
noteoff 11 61 0
noteon 11 62 102
sleep 6.449
noteoff 12 62 0
noteon 12 61 102
sleep 8.062
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 12.899
noteoff 10 61 0
noteon 10 62 102
sleep 9.674
noteoff 12 61 0
noteon 12 62 102
sleep 9.674
noteoff 11 62 0
noteon 11 61 102
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 9.675
noteoff 10 62 0
noteon 10 61 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 3 50 0
noteon 3 49 100
sleep 6.450
noteoff 12 62 0
noteon 12 61 102
sleep 22.574
noteoff 10 61 0
noteon 10 62 102
sleep 3.224
noteoff 11 61 0
noteon 11 62 102
sleep 6.449
noteoff 12 61 0
noteon 12 62 102
sleep 8.062
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.224
noteoff 14 38 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 61 102
sleep 9.677
noteoff 12 62 0
noteon 12 61 102
sleep 9.677
noteoff 11 62 0
noteon 11 61 102
sleep 12.903
noteoff 10 61 0
noteon 10 62 102
sleep 1.612
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 3 50 0
noteon 3 49 100
sleep 6.451
noteoff 12 61 0
noteon 12 62 102
sleep 12.903
noteoff 14 37 0
noteon 14 37 106
sleep 9.677
noteoff 10 62 0
noteon 10 59 102
sleep 3.225
noteoff 11 61 0
noteon 11 59 102
sleep 3.225
noteoff 14 37 0
sleep 3.225
noteoff 12 62 0
noteon 12 59 102
sleep 8.064
noteoff 13 49 0
noteon 13 47 104
sleep 1.612
noteoff 3 49 0
noteon 3 47 100
sleep 3.225
noteon 14 35 106
sleep 25.806
noteoff 10 59 0
noteon 10 61 102
sleep 3.225
noteoff 11 59 0
noteon 11 61 102
sleep 6.451
noteoff 12 59 0
noteon 12 61 102
sleep 8.064
noteoff 13 47 0
noteon 13 49 104
sleep 1.612
noteoff 3 47 0
noteon 3 49 100
sleep 3.225
noteoff 14 35 0
noteon 14 37 106
sleep 25.806
echo "144480 tempo_s=278 tempo_l=0.25"
noteoff 10 61 0
noteon 10 64 102
sleep 3.597
noteoff 11 61 0
noteon 11 64 102
sleep 5.395
noteon 2 64 101
noteon 2 76 101
sleep 1.798
noteoff 12 61 0
noteon 12 64 102
sleep 8.992
noteoff 13 49 0
noteon 13 52 104
sleep 1.798
noteoff 3 49 0
noteon 3 52 100
sleep 3.597
noteoff 14 37 0
noteon 14 40 106
sleep 82.733
noteoff 10 64 0
sleep 3.597
noteoff 11 64 0
sleep 5.395
noteoff 2 76 0
noteoff 2 64 0
sleep 1.798
noteoff 12 64 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 3.597
noteoff 14 40 0
sleep 82.733
noteon 10 57 102
sleep 3.597
noteon 11 57 102
sleep 5.395
noteon 2 57 101
noteon 2 69 101
sleep 1.798
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
sleep 3.597
noteon 14 33 106
sleep 82.733
noteoff 10 57 0
sleep 3.597
noteoff 11 57 0
sleep 5.395
noteoff 2 69 0
noteoff 2 57 0
sleep 1.798
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 45 0
sleep 3.597
noteoff 14 33 0
sleep 82.733
echo "144720 tempo_s=257 tempo_l=0.25"
sleep 233.463
echo "144840 tempo_s=309 tempo_l=0.25"
sleep 97.087
noteon 10 78 102
sleep 1.618
noteon 0 90 101
sleep 1.618
noteon 1 78 100
noteon 11 66 102
sleep 16.181
noteon 3 66 100
sleep 77.669
echo "144960 tempo_s=278 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteoff 11 66 0
noteon 1 79 100
noteon 11 67 102
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 86.33
noteoff 10 79 0
sleep 1.798
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
noteoff 11 67 0
sleep 17.985
noteoff 3 67 0
sleep 86.33
echo "145080 tempo_s=250 tempo_l=0.25"
sleep 119.999
noteon 10 62 102
sleep 3.999
noteon 11 62 102
sleep 7.999
noteon 12 62 102
sleep 10.0
noteon 13 50 104
sleep 1.999
noteon 3 50 100
sleep 3.999
noteon 14 38 106
sleep 72.0
noteoff 10 62 0
sleep 3.999
noteoff 11 62 0
sleep 7.999
noteoff 12 62 0
sleep 7.999
echo "145200 tempo_s=310 tempo_l=0.25"
noteon 10 64 102
sleep 1.612
noteoff 13 50 0
sleep 1.612
noteoff 3 50 0
noteon 11 64 102
sleep 3.225
noteoff 14 38 0
sleep 3.224
noteon 12 64 102
sleep 8.063
noteon 13 52 104
sleep 1.612
noteon 3 52 100
sleep 3.224
noteon 14 40 106
sleep 9.674
noteoff 10 64 0
noteon 10 62 102
sleep 9.674
noteoff 12 64 0
noteon 12 62 102
sleep 9.674
noteoff 11 64 0
noteon 11 62 102
sleep 12.900
noteoff 10 62 0
noteon 10 64 102
sleep 1.612
noteoff 13 52 0
noteon 13 50 104
sleep 1.612
noteoff 3 52 0
noteon 3 50 100
sleep 6.450
noteoff 12 62 0
noteon 12 64 102
sleep 12.899
noteoff 14 40 0
noteon 14 38 106
sleep 9.674
noteoff 10 64 0
noteon 10 62 102
sleep 3.224
noteoff 11 62 0
noteon 11 64 102
sleep 6.449
noteoff 12 64 0
noteon 12 62 102
sleep 8.062
noteoff 13 50 0
noteon 13 52 104
sleep 1.612
noteoff 3 50 0
noteon 3 52 100
sleep 12.899
noteoff 10 62 0
noteon 10 64 102
sleep 9.674
noteoff 12 62 0
noteon 12 64 102
sleep 9.674
noteoff 11 64 0
noteon 11 62 102
sleep 3.225
noteoff 14 38 0
noteon 14 40 106
sleep 9.675
noteoff 10 64 0
noteon 10 62 102
sleep 1.612
noteoff 13 52 0
noteon 13 50 104
sleep 1.612
noteoff 3 52 0
noteon 3 50 100
sleep 6.450
noteoff 12 64 0
noteon 12 62 102
sleep 22.574
noteoff 10 62 0
noteon 10 64 102
sleep 3.224
noteoff 11 62 0
noteon 11 64 102
sleep 6.449
noteoff 12 62 0
noteon 12 64 102
sleep 8.062
noteoff 13 50 0
noteon 13 52 104
sleep 1.612
noteoff 3 50 0
noteon 3 52 100
sleep 3.224
noteoff 14 40 0
noteon 14 38 106
sleep 9.677
noteoff 10 64 0
noteon 10 62 102
sleep 9.677
noteoff 12 64 0
noteon 12 62 102
sleep 9.677
noteoff 11 64 0
noteon 11 62 102
sleep 12.903
noteoff 10 62 0
noteon 10 64 102
sleep 1.612
noteoff 13 52 0
noteon 13 50 104
sleep 1.612
noteoff 3 52 0
noteon 3 50 100
sleep 6.451
noteoff 12 62 0
noteon 12 64 102
sleep 12.903
noteoff 14 38 0
noteon 14 38 106
sleep 9.677
noteoff 10 64 0
noteon 10 61 102
sleep 3.225
noteoff 11 62 0
noteon 11 61 102
sleep 3.225
noteoff 14 38 0
sleep 3.225
noteoff 12 64 0
noteon 12 61 102
sleep 8.064
noteoff 13 50 0
noteon 13 49 104
sleep 1.612
noteoff 3 50 0
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 25.806
noteoff 10 61 0
noteon 10 62 102
sleep 3.225
noteoff 11 61 0
noteon 11 62 102
sleep 6.451
noteoff 12 61 0
noteon 12 62 102
sleep 8.064
noteoff 13 49 0
noteon 13 50 104
sleep 1.612
noteoff 3 49 0
noteon 3 50 100
sleep 3.225
noteoff 14 37 0
noteon 14 38 106
sleep 25.806
echo "145440 tempo_s=278 tempo_l=0.25"
noteoff 10 62 0
noteon 10 67 102
sleep 3.597
noteoff 11 62 0
noteon 11 67 102
sleep 7.194
noteoff 12 62 0
noteon 12 67 102
sleep 8.992
noteoff 13 50 0
noteon 13 55 104
sleep 1.798
noteoff 3 50 0
noteon 3 55 100
sleep 3.597
noteoff 14 38 0
noteon 14 43 106
sleep 82.733
noteoff 10 67 0
sleep 3.597
noteoff 11 67 0
sleep 7.194
noteoff 12 67 0
sleep 8.992
noteoff 13 55 0
sleep 1.798
noteoff 3 55 0
sleep 3.597
noteoff 14 43 0
sleep 82.733
noteon 10 58 102
sleep 3.597
noteon 11 58 102
sleep 7.194
noteon 12 58 102
sleep 8.992
noteon 13 46 104
sleep 1.798
noteon 3 46 100
sleep 3.597
noteon 14 34 106
sleep 82.733
noteoff 10 58 0
sleep 3.597
noteoff 11 58 0
sleep 7.194
noteoff 12 58 0
sleep 8.992
noteoff 13 46 0
sleep 1.798
noteoff 3 46 0
sleep 3.597
noteoff 14 34 0
sleep 82.733
echo "145680 tempo_s=257 tempo_l=0.25"
sleep 233.463
echo "145800 tempo_s=310 tempo_l=0.25"
sleep 96.774
noteon 10 78 102
sleep 1.612
noteon 0 90 101
sleep 1.612
noteon 1 78 100
noteon 11 66 102
sleep 16.129
noteon 3 66 100
sleep 77.419
echo "145920 tempo_s=278 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 102
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteoff 11 66 0
noteon 1 79 100
noteon 11 67 102
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 86.33
noteoff 10 79 0
sleep 1.798
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
noteoff 11 67 0
sleep 17.985
noteoff 3 67 0
sleep 86.33
echo "146040 tempo_s=263 tempo_l=0.25"
noteon 10 62 102
sleep 3.802
noteon 11 62 102
sleep 7.604
noteon 12 62 102
sleep 9.505
noteon 13 50 104
sleep 1.901
noteon 3 50 100
sleep 3.802
noteon 14 38 106
sleep 87.452
noteoff 10 62 0
sleep 3.802
noteoff 11 62 0
sleep 7.604
noteoff 12 62 0
sleep 9.505
noteoff 13 50 0
sleep 1.901
noteoff 3 50 0
sleep 3.802
noteoff 14 38 0
sleep 87.452
echo "146160 tempo_s=310 tempo_l=0.25"
noteon 10 67 102
sleep 3.225
noteon 11 67 102
sleep 6.451
noteon 12 67 102
sleep 8.064
noteon 13 55 104
sleep 1.612
noteon 3 55 100
sleep 3.225
noteon 14 43 106
sleep 74.193
noteoff 10 67 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 67 0
sleep 8.064
noteoff 13 55 0
sleep 1.612
noteoff 3 55 0
sleep 3.225
noteoff 14 43 0
sleep 74.193
noteon 10 59 102
sleep 3.225
noteon 11 59 102
sleep 6.451
noteon 12 59 102
sleep 8.064
noteon 13 47 104
sleep 1.612
noteon 3 47 100
sleep 3.225
noteon 14 35 106
sleep 74.193
noteoff 10 59 0
sleep 1.612
noteon 0 90 101
sleep 1.612
noteoff 11 59 0
noteon 1 78 100
sleep 6.451
noteoff 12 59 0
sleep 8.064
noteoff 13 47 0
sleep 1.612
noteoff 3 47 0
noteon 3 66 100
sleep 3.225
noteoff 14 35 0
sleep 74.193
echo "146400 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteon 1 79 100
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 88.129
noteoff 0 91 0
sleep 1.798
noteoff 1 79 0
sleep 17.985
noteoff 3 67 0
sleep 86.33
noteon 10 64 102
sleep 3.597
noteon 11 64 102
sleep 7.194
noteon 12 64 102
sleep 8.992
noteon 13 52 104
sleep 1.798
noteon 3 50 100
sleep 3.597
noteon 14 40 106
sleep 82.733
noteoff 10 64 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 64 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 50 0
sleep 3.597
noteoff 14 40 0
sleep 82.733
echo "146640 tempo_s=310 tempo_l=0.25"
noteon 10 67 102
sleep 3.225
noteon 11 67 102
sleep 6.451
noteon 12 67 102
sleep 8.064
noteon 13 55 104
sleep 1.612
noteon 3 55 100
sleep 3.225
noteon 14 43 106
sleep 74.193
noteoff 10 67 0
sleep 3.225
noteoff 11 67 0
sleep 6.451
noteoff 12 67 0
sleep 8.064
noteoff 13 55 0
sleep 1.612
noteoff 3 55 0
sleep 3.225
noteoff 14 43 0
sleep 74.193
noteon 10 60 102
sleep 3.225
noteon 11 60 102
sleep 6.451
noteon 12 60 102
sleep 8.064
noteon 13 48 104
sleep 1.612
noteon 3 48 100
sleep 3.225
noteon 14 36 106
sleep 74.193
noteoff 10 60 0
sleep 1.612
noteon 0 90 101
sleep 1.612
noteoff 11 60 0
noteon 1 78 100
sleep 6.451
noteoff 12 60 0
sleep 8.064
noteoff 13 48 0
sleep 1.612
noteoff 3 48 0
noteon 3 66 100
sleep 3.225
noteoff 14 36 0
sleep 74.193
echo "146880 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 0 90 0
noteon 0 91 101
sleep 1.798
noteoff 1 78 0
noteon 1 79 100
sleep 17.985
noteoff 3 66 0
noteon 3 67 100
sleep 194.244
noteon 10 67 102
sleep 3.597
noteon 11 67 102
sleep 7.194
noteon 12 55 102
sleep 8.992
noteon 13 55 104
sleep 1.798
noteon 3 55 100
sleep 3.597
noteon 14 43 106
sleep 82.733
noteoff 10 67 0
sleep 3.597
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 8.992
noteoff 13 55 0
sleep 1.798
noteoff 3 55 0
sleep 3.597
noteoff 14 43 0
sleep 82.733
echo "147120 tempo_s=310 tempo_l=0.25"
noteon 10 70 102
sleep 3.225
noteon 11 70 102
sleep 6.451
noteon 12 58 102
sleep 8.064
noteon 13 58 104
sleep 1.612
noteon 3 58 100
sleep 3.225
noteon 14 46 106
sleep 74.193
noteoff 10 70 0
sleep 3.225
noteoff 11 70 0
sleep 6.451
noteoff 12 58 0
sleep 8.064
noteoff 13 58 0
sleep 1.612
noteoff 3 58 0
sleep 3.225
noteoff 14 46 0
sleep 74.193
noteon 10 61 102
sleep 3.225
noteon 11 61 102
sleep 6.451
noteon 12 49 102
sleep 8.064
noteon 13 49 104
sleep 1.612
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 74.193
noteoff 10 61 0
sleep 3.225
noteoff 11 61 0
sleep 6.451
noteoff 12 49 0
sleep 8.064
noteoff 13 49 0
sleep 1.612
noteoff 3 49 0
sleep 3.225
noteoff 14 37 0
sleep 74.193
echo "147360 tempo_s=278 tempo_l=0.25"
noteon 10 62 102
sleep 3.597
noteon 11 62 102
sleep 7.194
noteon 12 50 102
sleep 8.992
noteon 13 50 104
sleep 1.798
noteon 3 50 100
sleep 3.597
noteon 14 38 106
sleep 82.733
noteoff 10 62 0
sleep 3.597
noteoff 11 62 0
sleep 7.194
noteoff 12 50 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
sleep 3.597
noteoff 14 38 0
sleep 82.733
noteon 10 71 102
sleep 3.597
noteon 11 71 102
sleep 7.194
noteon 12 59 102
sleep 8.992
noteon 13 59 104
sleep 1.798
noteon 3 59 100
sleep 3.597
noteon 14 47 106
sleep 82.733
noteoff 10 71 0
sleep 3.597
noteoff 11 71 0
sleep 7.194
noteoff 12 59 0
sleep 8.992
noteoff 13 59 0
sleep 1.798
noteoff 3 59 0
sleep 3.597
noteoff 14 47 0
sleep 82.733
echo "147600 tempo_s=310 tempo_l=0.25"
noteon 10 63 102
sleep 3.225
noteon 11 63 102
sleep 6.451
noteon 12 51 102
sleep 8.064
noteon 13 51 104
sleep 1.612
noteon 3 51 100
sleep 3.225
noteon 14 39 106
sleep 74.193
noteoff 10 63 0
sleep 3.225
noteoff 11 63 0
sleep 6.451
noteoff 12 51 0
sleep 8.064
noteoff 13 51 0
sleep 1.612
noteoff 3 51 0
sleep 3.225
noteoff 14 39 0
sleep 74.193
noteon 10 72 102
sleep 3.225
noteon 11 72 102
sleep 6.451
noteon 12 60 102
sleep 8.064
noteon 13 60 104
sleep 1.612
noteon 3 60 100
sleep 3.225
noteon 14 48 106
sleep 74.193
noteoff 10 72 0
sleep 3.225
noteoff 11 72 0
sleep 6.451
noteoff 12 60 0
sleep 8.064
noteoff 13 60 0
sleep 1.612
noteoff 3 60 0
sleep 3.225
noteoff 14 48 0
sleep 74.193
echo "147840 tempo_s=278 tempo_l=0.25"
noteon 10 64 102
sleep 3.597
noteon 11 64 102
sleep 7.194
noteon 12 52 102
sleep 8.992
noteon 13 52 104
sleep 1.798
noteon 3 52 100
sleep 3.597
noteon 14 40 106
sleep 82.733
noteoff 10 64 0
sleep 3.597
noteoff 11 64 0
sleep 7.194
noteoff 12 52 0
sleep 8.992
noteoff 13 52 0
sleep 1.798
noteoff 3 52 0
sleep 3.597
noteoff 14 40 0
sleep 82.733
noteon 10 73 102
sleep 3.597
noteon 11 73 102
sleep 7.194
noteon 12 61 102
sleep 8.992
noteon 13 61 104
sleep 1.798
noteon 3 61 100
sleep 3.597
noteon 14 49 106
sleep 82.733
noteoff 10 73 0
sleep 3.597
noteoff 11 73 0
sleep 7.194
noteoff 12 61 0
sleep 8.992
noteoff 13 61 0
sleep 1.798
noteoff 3 61 0
sleep 3.597
noteoff 14 49 0
sleep 82.733
echo "148080 tempo_s=310 tempo_l=0.25"
noteon 10 65 102
sleep 1.612
noteoff 0 91 0
noteon 0 89 101
sleep 1.612
noteoff 1 79 0
noteon 1 77 100
noteon 11 65 102
sleep 6.451
noteon 12 53 102
sleep 8.064
noteon 13 53 104
sleep 1.612
noteoff 3 67 0
noteon 3 53 100
noteon 3 65 100
sleep 3.225
noteon 14 41 106
sleep 74.193
noteoff 10 65 0
sleep 3.225
noteoff 11 65 0
sleep 6.451
noteoff 12 53 0
sleep 8.064
noteoff 13 53 0
sleep 1.612
noteoff 3 53 0
sleep 3.225
noteoff 14 41 0
sleep 74.193
noteon 10 74 102
sleep 3.225
noteon 11 74 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 62 104
sleep 1.612
noteon 3 62 100
sleep 3.225
noteon 14 50 106
sleep 74.193
noteoff 10 74 0
sleep 3.225
noteoff 11 74 0
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 62 0
sleep 1.612
noteoff 3 62 0
sleep 3.225
noteoff 14 50 0
sleep 74.193
echo "148320 tempo_s=278 tempo_l=0.25"
noteon 10 65 102
sleep 3.597
noteon 11 65 102
sleep 7.194
noteon 12 53 102
sleep 8.992
noteon 13 53 104
sleep 1.798
noteon 3 53 100
sleep 3.597
noteon 14 41 106
sleep 82.733
noteoff 10 65 0
sleep 3.597
noteoff 11 65 0
sleep 7.194
noteoff 12 53 0
sleep 8.992
noteoff 13 53 0
sleep 1.798
noteoff 3 53 0
sleep 3.597
noteoff 14 41 0
sleep 82.733
noteon 10 74 102
sleep 1.798
noteoff 0 89 0
noteon 0 88 101
sleep 1.798
noteoff 1 77 0
noteon 1 76 100
noteon 11 74 102
sleep 7.194
noteon 12 62 102
sleep 8.992
noteon 13 62 104
sleep 1.798
noteoff 3 65 0
noteon 3 64 100
sleep 3.597
noteon 14 50 106
sleep 82.733
noteoff 10 74 0
sleep 3.597
noteoff 11 74 0
sleep 7.194
noteoff 12 62 0
sleep 8.992
noteoff 13 62 0
sleep 5.395
noteoff 14 50 0
sleep 82.733
echo "148560 tempo_s=310 tempo_l=0.25"
noteon 10 66 102
sleep 1.612
noteoff 0 88 0
noteon 0 86 101
sleep 1.612
noteoff 1 76 0
noteon 1 74 100
noteon 11 66 102
sleep 6.451
noteon 12 54 102
sleep 8.064
noteon 13 54 104
sleep 1.612
noteoff 3 64 0
noteon 3 62 100
sleep 3.225
noteon 14 42 106
sleep 74.193
noteoff 10 66 0
sleep 3.225
noteoff 11 66 0
sleep 6.451
noteoff 12 54 0
sleep 8.064
noteoff 13 54 0
sleep 4.838
noteoff 14 42 0
sleep 74.193
noteon 10 74 102
sleep 1.612
noteoff 0 86 0
noteon 0 84 101
sleep 1.612
noteoff 1 74 0
noteon 1 72 100
noteon 11 74 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 62 104
sleep 1.612
noteoff 3 62 0
noteon 3 60 100
sleep 3.225
noteon 14 50 106
sleep 74.193
noteoff 10 74 0
sleep 3.225
noteoff 11 74 0
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 62 0
sleep 4.838
noteoff 14 50 0
sleep 74.193
echo "148800 tempo_s=278 tempo_l=0.25"
noteon 10 67 102
sleep 1.798
noteoff 0 84 0
noteon 0 82 101
sleep 1.798
noteoff 1 72 0
noteon 1 70 100
noteon 4 62 100
noteon 11 67 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 5.395
noteon 5 50 100
noteon 12 55 102
sleep 8.992
noteon 13 55 104
sleep 1.798
noteoff 3 60 0
noteon 3 58 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 43 106
sleep 82.733
noteoff 10 67 0
sleep 3.597
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 8.992
noteoff 13 55 0
sleep 5.395
noteoff 14 43 0
sleep 82.733
noteon 10 74 102
sleep 3.597
noteon 11 74 102
sleep 7.194
noteon 12 62 102
sleep 8.992
noteon 13 62 104
sleep 3.597
noteoff 15 50 0
sleep 1.798
noteon 14 50 106
sleep 82.733
noteoff 10 74 0
sleep 3.597
noteoff 11 74 0
sleep 7.194
noteoff 12 62 0
sleep 8.992
noteoff 13 62 0
sleep 5.395
noteoff 14 50 0
sleep 82.733
echo "149040 tempo_s=310 tempo_l=0.25"
noteon 10 69 102
sleep 3.225
noteon 11 69 102
sleep 6.451
noteon 12 57 102
sleep 8.064
noteon 13 57 104
sleep 3.225
noteon 15 50 80
sleep 1.612
noteon 14 45 106
sleep 74.193
noteoff 10 69 0
sleep 3.225
noteoff 11 69 0
sleep 6.451
noteoff 12 57 0
sleep 8.064
noteoff 13 57 0
sleep 4.838
noteoff 14 45 0
sleep 74.193
noteon 10 74 102
sleep 1.612
noteoff 0 82 0
noteon 0 81 101
sleep 1.612
noteoff 1 70 0
noteon 1 69 100
noteon 11 74 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 62 104
sleep 1.612
noteoff 3 58 0
noteon 3 57 100
sleep 1.612
noteoff 15 50 0
sleep 1.612
noteon 14 50 106
sleep 74.193
noteoff 10 74 0
sleep 3.225
noteoff 11 74 0
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 62 0
sleep 4.838
noteoff 14 50 0
sleep 74.193
echo "149280 tempo_s=278 tempo_l=0.25"
noteon 10 70 102
sleep 1.798
noteoff 0 81 0
noteon 0 79 101
sleep 1.798
noteoff 1 69 0
noteon 1 67 100
noteon 11 70 102
sleep 7.194
noteon 12 58 102
sleep 8.992
noteon 13 58 104
sleep 1.798
noteoff 3 57 0
noteon 3 55 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 46 106
sleep 82.733
noteoff 10 70 0
sleep 3.597
noteoff 11 70 0
sleep 7.194
noteoff 12 58 0
sleep 8.992
noteoff 13 58 0
sleep 5.395
noteoff 14 46 0
sleep 82.733
noteon 10 74 102
sleep 1.798
noteoff 0 79 0
noteon 0 77 101
sleep 1.798
noteoff 1 67 0
noteon 1 65 100
noteon 11 74 102
sleep 7.194
noteon 12 62 102
sleep 8.992
noteon 13 62 104
sleep 1.798
noteoff 3 55 0
noteon 3 53 100
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteon 14 50 106
sleep 82.733
noteoff 10 74 0
sleep 3.597
noteoff 11 74 0
sleep 7.194
noteoff 12 62 0
sleep 8.992
noteoff 13 62 0
sleep 5.395
noteoff 14 50 0
sleep 82.733
echo "149520 tempo_s=310 tempo_l=0.25"
noteon 10 68 102
sleep 1.612
noteoff 0 77 0
noteon 0 76 101
sleep 1.612
noteoff 1 65 0
noteon 1 64 100
noteon 11 68 102
sleep 6.451
noteon 12 56 102
sleep 8.064
noteon 13 56 104
sleep 1.612
noteoff 3 53 0
noteon 3 52 100
sleep 1.612
noteon 15 50 80
sleep 1.612
noteon 14 44 106
sleep 74.193
noteoff 10 68 0
sleep 3.225
noteoff 11 68 0
sleep 6.451
noteoff 12 56 0
sleep 8.064
noteoff 13 56 0
sleep 4.838
noteoff 14 44 0
sleep 74.193
noteon 10 74 102
sleep 1.612
noteoff 0 76 0
noteon 0 74 101
sleep 1.612
noteoff 1 64 0
noteon 1 62 100
noteon 11 74 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 62 104
sleep 1.612
noteoff 3 52 0
noteon 3 50 100
sleep 1.612
noteoff 15 50 0
sleep 1.612
noteon 14 50 106
sleep 74.193
noteoff 10 74 0
sleep 3.225
noteoff 11 74 0
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 62 0
sleep 4.838
noteoff 14 50 0
sleep 74.193
echo "149760 tempo_s=278 tempo_l=0.25"
noteon 10 69 102
sleep 1.798
noteoff 0 74 0
noteon 0 81 101
sleep 1.798
noteoff 1 62 0
noteoff 4 62 0
noteon 1 69 100
noteon 4 57 100
noteon 11 69 102
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
noteon 6 57 108
noteon 6 69 108
sleep 5.395
noteoff 5 50 0
noteon 5 45 100
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 50 0
noteon 3 57 100
noteon 3 45 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 75.530
noteoff 12 57 0
sleep 8.991
noteoff 13 57 0
sleep 8.992
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 80.925
noteoff 12 57 0
sleep 7.193
noteoff 10 69 0
sleep 1.798
noteoff 0 81 0
noteoff 13 57 0
sleep 1.798
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 69 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 5.395
noteoff 5 45 0
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 45 0
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 75.530
noteoff 12 57 0
sleep 8.991
noteoff 13 57 0
sleep 8.992
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 80.925
noteoff 12 57 0
sleep 7.193
echo "150000 tempo_s=301 tempo_l=0.25"
sleep 1.661
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 1.661
noteoff 3 57 0
noteon 3 59 100
sleep 73.089
noteoff 12 57 0
sleep 8.305
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 74.75
noteoff 12 57 0
sleep 8.305
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 1.661
noteoff 3 59 0
noteon 3 61 100
sleep 73.089
noteoff 12 57 0
sleep 8.305
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 74.75
noteoff 12 57 0
sleep 6.644
echo "150240 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 13 57 0
sleep 8.992
noteon 12 57 102
sleep 1.798
noteoff 3 61 0
sleep 7.194
noteon 13 57 104
sleep 1.798
noteon 3 62 100
sleep 79.136
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 8.992
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 80.935
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 8.992
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 80.935
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 8.992
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 80.935
noteoff 12 57 0
sleep 7.194
echo "150480 tempo_s=301 tempo_l=0.25"
sleep 1.661
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 74.75
noteoff 12 57 0
sleep 8.305
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 74.75
noteoff 12 57 0
sleep 8.305
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 1.661
noteoff 3 62 0
sleep 6.644
noteon 13 57 104
sleep 1.661
noteon 3 62 100
sleep 73.089
noteoff 12 57 0
sleep 8.305
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 74.75
noteoff 12 57 0
sleep 6.644
echo "150720 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
noteon 10 69 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 1 73 100
noteon 1 69 100
noteon 4 69 100
noteon 11 81 102
noteon 11 69 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 5.395
noteon 5 57 100
noteon 12 57 102
sleep 1.798
noteoff 3 62 0
sleep 7.193
noteon 13 57 104
sleep 1.798
noteon 3 61 100
noteon 3 57 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 75.529
noteoff 12 57 0
sleep 8.991
noteoff 13 57 0
sleep 8.992
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 80.924
noteoff 12 57 0
sleep 7.193
noteoff 10 69 0
noteoff 10 81 0
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteoff 4 69 0
noteoff 11 69 0
noteoff 11 81 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 5.395
noteoff 5 57 0
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 3.596
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 75.529
noteoff 12 57 0
sleep 8.991
noteoff 13 57 0
sleep 8.992
noteon 12 57 102
sleep 8.991
noteon 13 57 104
sleep 80.924
noteoff 12 57 0
sleep 7.193
echo "150960 tempo_s=301 tempo_l=0.25"
sleep 1.661
noteoff 13 57 0
sleep 1.661
noteoff 1 69 0
noteoff 1 73 0
noteon 1 74 100
noteon 1 71 100
sleep 6.644
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 1.661
noteoff 3 57 0
noteoff 3 61 0
noteon 3 62 100
noteon 3 59 100
sleep 73.089
noteoff 12 57 0
sleep 8.305
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 74.75
noteoff 12 57 0
sleep 8.305
noteoff 13 57 0
sleep 1.661
noteoff 1 71 0
noteoff 1 74 0
noteon 1 76 100
noteon 1 73 100
sleep 6.644
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 1.661
noteoff 3 59 0
noteoff 3 62 0
noteon 3 64 100
noteon 3 61 100
sleep 73.089
noteoff 12 57 0
sleep 8.305
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 74.75
noteoff 12 57 0
sleep 1.661
noteoff 1 73 0
noteoff 1 76 0
sleep 4.983
echo "151200 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 1 74 100
noteon 1 77 100
sleep 7.194
noteon 12 57 102
sleep 1.798
noteoff 3 61 0
noteoff 3 64 0
sleep 7.194
noteon 13 57 104
sleep 1.798
noteon 3 62 100
noteon 3 65 100
sleep 79.136
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 8.992
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 80.935
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 8.992
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 80.935
noteoff 12 57 0
sleep 8.992
noteoff 13 57 0
sleep 8.992
noteon 12 57 102
sleep 8.992
noteon 13 57 104
sleep 80.935
noteoff 12 57 0
sleep 7.194
echo "151440 tempo_s=301 tempo_l=0.25"
sleep 1.661
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 74.75
noteoff 12 57 0
sleep 8.305
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 74.75
noteoff 12 57 0
sleep 1.661
noteoff 1 77 0
noteoff 1 74 0
sleep 6.644
noteoff 13 57 0
sleep 1.661
noteon 1 74 100
noteon 1 77 100
sleep 6.644
noteon 12 57 102
sleep 1.661
noteoff 3 65 0
noteoff 3 62 0
sleep 6.644
noteon 13 57 104
sleep 1.661
noteon 3 65 100
noteon 3 62 100
sleep 73.089
noteoff 12 57 0
sleep 8.305
noteoff 13 57 0
sleep 8.305
noteon 12 57 102
sleep 8.305
noteon 13 57 104
sleep 74.75
noteoff 12 57 0
sleep 1.661
noteoff 1 77 0
noteoff 1 74 0
sleep 4.983
echo "151680 tempo_s=278 tempo_l=0.25"
noteon 10 69 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteon 1 73 100
noteon 1 76 100
noteon 4 69 100
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 5.395
noteon 5 57 100
noteon 12 57 102
sleep 1.798
noteoff 3 62 0
noteoff 3 65 0
sleep 7.193
noteon 13 57 104
sleep 1.798
noteon 3 61 100
noteon 3 64 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 64.740
noteoff 10 69 0
sleep 17.983
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 17.983
noteon 13 57 104
sleep 70.135
noteoff 10 81 0
sleep 17.983
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteoff 4 69 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 5.395
noteoff 5 57 0
noteoff 12 57 0
sleep 8.992
noteon 13 57 104
sleep 3.596
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 64.740
noteoff 10 81 0
sleep 17.983
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 17.984
noteon 13 57 104
sleep 70.135
noteoff 10 81 0
sleep 17.983
echo "151920 tempo_s=301 tempo_l=0.25"
noteon 10 81 102
sleep 1.661
noteoff 13 57 0
sleep 1.661
noteoff 1 76 0
noteoff 1 73 0
noteon 1 71 100
noteon 1 74 100
sleep 14.949
noteon 13 57 104
sleep 1.661
noteoff 3 64 0
noteoff 3 61 0
noteon 3 59 100
noteon 3 62 100
sleep 63.122
noteoff 10 81 0
sleep 16.611
noteon 10 81 102
sleep 1.661
noteoff 13 57 0
sleep 16.611
noteon 13 57 104
sleep 64.784
noteoff 10 81 0
sleep 16.611
noteon 10 81 102
sleep 1.661
noteoff 13 57 0
sleep 1.661
noteoff 1 74 0
noteoff 1 71 0
noteon 1 69 100
noteon 1 73 100
sleep 14.95
noteon 13 57 104
sleep 1.661
noteoff 3 62 0
noteoff 3 59 0
noteon 3 57 100
noteon 3 61 100
sleep 63.122
noteoff 10 81 0
sleep 16.611
noteon 10 81 102
sleep 1.661
noteoff 13 57 0
sleep 16.611
noteon 13 57 104
sleep 64.784
noteoff 10 81 0
sleep 11.627
noteoff 1 73 0
noteoff 1 69 0
sleep 4.983
echo "152160 tempo_s=278 tempo_l=0.25"
noteon 10 69 102
sleep 1.798
noteoff 13 57 0
noteon 0 86 101
noteon 0 83 101
sleep 1.798
noteon 1 77 100
noteon 1 74 100
noteon 4 69 100
noteon 11 74 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 62 101
noteon 2 71 101
sleep 1.798
noteon 5 57 100
noteon 12 59 102
sleep 1.798
noteoff 3 61 0
noteoff 3 57 0
sleep 7.193
noteon 13 57 104
sleep 1.798
noteon 3 62 100
noteon 3 65 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 64.734
noteoff 10 69 0
sleep 17.982
noteon 10 80 102
sleep 1.798
noteoff 13 57 0
sleep 17.982
noteon 13 57 104
sleep 70.128
noteoff 10 80 0
sleep 17.982
noteon 10 80 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteoff 4 69 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 5.394
noteoff 5 57 0
sleep 8.992
noteon 13 57 104
sleep 3.596
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 64.740
noteoff 10 80 0
sleep 17.983
noteon 10 80 102
sleep 1.798
noteoff 13 57 0
sleep 17.983
noteon 13 57 104
sleep 70.135
noteoff 10 80 0
sleep 17.983
echo "152400 tempo_s=301 tempo_l=0.25"
noteon 10 80 102
sleep 1.661
noteoff 0 83 0
noteoff 0 86 0
noteoff 13 57 0
noteon 0 88 101
noteon 0 85 101
sleep 1.661
noteoff 1 74 0
noteoff 1 77 0
noteoff 11 74 0
noteon 1 73 100
noteon 1 76 100
noteon 11 76 102
sleep 4.983
noteoff 2 71 0
noteoff 2 62 0
noteon 2 73 101
noteon 2 64 101
sleep 1.661
noteoff 12 59 0
noteon 12 61 102
sleep 8.305
noteon 13 57 104
sleep 1.661
noteoff 3 65 0
noteoff 3 62 0
noteon 3 61 100
noteon 3 64 100
sleep 63.122
noteoff 10 80 0
sleep 16.611
noteon 10 80 102
sleep 1.661
noteoff 13 57 0
sleep 16.611
noteon 13 57 104
sleep 64.784
noteoff 10 80 0
sleep 16.611
noteon 10 80 102
sleep 1.661
noteoff 0 85 0
noteoff 0 88 0
noteoff 13 57 0
noteon 0 89 101
noteon 0 86 101
sleep 1.661
noteoff 1 76 0
noteoff 1 73 0
noteoff 11 76 0
noteon 1 71 100
noteon 1 74 100
noteon 11 77 102
sleep 4.983
noteoff 2 64 0
noteoff 2 73 0
noteon 2 74 101
noteon 2 65 101
sleep 1.661
noteoff 12 61 0
noteon 12 62 102
sleep 8.305
noteon 13 57 104
sleep 1.661
noteoff 3 64 0
noteoff 3 61 0
noteon 3 59 100
noteon 3 62 100
sleep 63.122
noteoff 10 80 0
sleep 16.611
noteon 10 80 102
sleep 1.661
noteoff 13 57 0
sleep 16.611
noteon 13 57 104
sleep 64.784
noteoff 10 80 0
sleep 3.322
noteoff 11 77 0
sleep 6.644
noteoff 0 86 0
noteoff 0 89 0
noteoff 12 62 0
sleep 1.661
noteoff 1 74 0
noteoff 1 71 0
sleep 4.983
echo "152640 tempo_s=278 tempo_l=0.25"
noteoff 2 65 0
noteoff 2 74 0
noteon 10 69 102
sleep 1.798
noteoff 13 57 0
noteon 0 85 101
noteon 0 88 101
sleep 1.798
noteon 1 73 100
noteon 1 76 100
noteon 4 69 100
noteon 11 76 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 73 101
noteon 2 64 101
sleep 1.798
noteon 5 57 100
noteon 12 61 102
sleep 1.798
noteoff 3 62 0
noteoff 3 59 0
sleep 7.193
noteon 13 57 104
sleep 1.798
noteon 3 61 100
noteon 3 64 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 64.734
noteoff 10 69 0
sleep 17.982
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 17.982
noteon 13 57 104
sleep 70.128
noteoff 10 81 0
sleep 17.982
noteon 10 81 102
sleep 1.798
noteoff 0 88 0
noteoff 0 85 0
noteoff 13 57 0
sleep 1.798
noteoff 4 69 0
noteoff 11 76 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 3.596
noteoff 2 64 0
noteoff 2 73 0
sleep 1.798
noteoff 5 57 0
noteoff 12 61 0
sleep 8.992
noteon 13 57 104
sleep 3.596
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 64.740
noteoff 10 81 0
sleep 17.983
noteon 10 81 102
sleep 1.798
noteoff 13 57 0
sleep 17.984
noteon 13 57 104
sleep 70.135
noteoff 10 81 0
sleep 17.983
echo "152880 tempo_s=301 tempo_l=0.25"
noteon 10 81 102
sleep 1.661
noteoff 13 57 0
sleep 1.661
noteoff 1 76 0
noteoff 1 73 0
noteon 1 71 100
noteon 1 74 100
sleep 14.949
noteon 13 57 104
sleep 1.661
noteoff 3 64 0
noteoff 3 61 0
noteon 3 62 100
noteon 3 59 100
sleep 63.122
noteoff 10 81 0
sleep 16.611
noteon 10 81 102
sleep 1.661
noteoff 13 57 0
sleep 16.611
noteon 13 57 104
sleep 64.784
noteoff 10 81 0
sleep 16.611
noteon 10 81 102
sleep 1.661
noteoff 13 57 0
sleep 1.661
noteoff 1 74 0
noteoff 1 71 0
noteon 1 69 100
noteon 1 73 100
sleep 14.95
noteon 13 57 104
sleep 1.661
noteoff 3 59 0
noteoff 3 62 0
noteon 3 57 100
noteon 3 61 100
sleep 63.122
noteoff 10 81 0
sleep 16.611
noteon 10 81 102
sleep 1.661
noteoff 13 57 0
sleep 16.611
noteon 13 57 104
sleep 64.784
noteoff 10 81 0
sleep 11.627
noteoff 1 73 0
noteoff 1 69 0
sleep 4.983
echo "153120 tempo_s=278 tempo_l=0.25"
noteon 10 69 102
sleep 1.798
noteoff 13 57 0
noteon 0 86 101
noteon 0 83 101
sleep 1.798
noteon 1 77 100
noteon 1 74 100
noteon 4 69 100
noteon 11 74 102
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 3.597
noteon 2 71 101
noteon 2 62 101
sleep 1.798
noteon 5 57 100
noteon 12 59 102
sleep 1.798
noteoff 3 61 0
noteoff 3 57 0
sleep 7.193
noteon 13 57 104
sleep 1.798
noteon 3 65 100
noteon 3 62 100
sleep 1.798
noteon 15 45 80
sleep 1.798
noteon 14 33 106
sleep 64.734
noteoff 10 69 0
sleep 17.982
noteon 10 80 102
sleep 1.798
noteoff 13 57 0
sleep 17.982
noteon 13 57 104
sleep 70.128
noteoff 10 80 0
sleep 17.982
noteon 10 80 102
sleep 1.798
noteoff 13 57 0
sleep 1.798
noteoff 4 69 0
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
sleep 5.394
noteoff 5 57 0
sleep 8.992
noteon 13 57 104
sleep 3.596
noteoff 15 45 0
sleep 1.798
noteoff 14 33 0
sleep 64.740
noteoff 10 80 0
sleep 17.983
noteon 10 80 102
sleep 1.798
noteoff 13 57 0
sleep 17.983
noteon 13 57 104
sleep 70.135
noteoff 10 80 0
sleep 17.983
echo "153360 tempo_s=301 tempo_l=0.25"
noteon 10 80 102
sleep 1.661
noteoff 0 83 0
noteoff 0 86 0
noteoff 13 57 0
noteon 0 88 101
noteon 0 85 101
sleep 1.661
noteoff 1 74 0
noteoff 1 77 0
noteoff 11 74 0
noteon 1 73 100
noteon 1 76 100
noteon 11 76 102
sleep 4.983
noteoff 2 62 0
noteoff 2 71 0
noteon 2 64 101
noteon 2 73 101
sleep 1.661
noteoff 12 59 0
noteon 12 61 102
sleep 8.305
noteon 13 57 104
sleep 1.661
noteoff 3 62 0
noteoff 3 65 0
noteon 3 61 100
noteon 3 64 100
sleep 63.122
noteoff 10 80 0
sleep 16.611
noteon 10 80 102
sleep 1.661
noteoff 13 57 0
sleep 16.611
noteon 13 57 104
sleep 64.784
noteoff 10 80 0
sleep 16.611
noteon 10 80 102
sleep 1.661
noteoff 0 85 0
noteoff 0 88 0
noteoff 13 57 0
noteon 0 89 101
noteon 0 86 101
sleep 1.661
noteoff 1 76 0
noteoff 1 73 0
noteoff 11 76 0
noteon 1 71 100
noteon 1 74 100
noteon 11 77 102
sleep 4.983
noteoff 2 73 0
noteoff 2 64 0
noteon 2 65 101
noteon 2 74 101
sleep 1.661
noteoff 12 61 0
noteon 12 62 102
sleep 8.305
noteon 13 57 104
sleep 1.661
noteoff 3 64 0
noteoff 3 61 0
noteon 3 62 100
noteon 3 59 100
sleep 63.122
noteoff 10 80 0
sleep 16.611
noteon 10 80 102
sleep 1.661
noteoff 13 57 0
sleep 16.611
noteon 13 57 104
sleep 64.784
noteoff 10 80 0
sleep 3.322
noteoff 11 77 0
sleep 6.644
noteoff 0 86 0
noteoff 0 89 0
noteoff 12 62 0
sleep 1.661
noteoff 1 74 0
noteoff 1 71 0
sleep 4.983
echo "153600 tempo_s=273 tempo_l=0.25"
noteoff 2 74 0
noteoff 2 65 0
noteon 10 69 102
sleep 1.831
noteoff 13 57 0
noteon 0 85 101
noteon 0 88 101
sleep 1.831
noteon 1 76 100
noteon 1 73 100
noteon 4 69 100
noteon 11 76 102
sleep 5.494
noteon 2 73 101
noteon 2 64 101
sleep 1.831
noteon 5 57 100
noteon 12 61 102
sleep 1.831
noteoff 3 59 0
noteoff 3 62 0
sleep 7.326
noteon 13 57 104
sleep 1.831
noteon 3 64 100
noteon 3 61 100
sleep 1.831
noteon 15 45 35
sleep 1.831
noteon 14 33 106
sleep 65.933
noteoff 10 69 0
sleep 3.663
noteoff 11 76 0
sleep 7.326
noteoff 12 61 0
sleep 7.325
noteon 10 81 102
sleep 1.831
noteoff 15 45 0
noteoff 13 57 0
sleep 1.831
noteon 11 76 102
sleep 3.663
noteoff 14 33 0
sleep 3.663
noteon 12 73 102
sleep 9.157
noteon 13 57 104
sleep 3.662
noteon 15 45 37
sleep 1.831
noteon 14 33 106
sleep 65.932
noteoff 10 81 0
sleep 3.663
noteoff 11 76 0
sleep 7.326
noteoff 12 73 0
sleep 7.326
noteon 10 81 102
sleep 1.831
noteoff 0 88 0
noteoff 0 85 0
noteoff 15 45 0
noteoff 13 57 0
sleep 1.831
noteoff 1 73 0
noteoff 1 76 0
noteon 11 76 102
sleep 3.663
noteoff 14 33 0
sleep 1.831
noteoff 2 64 0
noteoff 2 73 0
sleep 1.831
noteon 12 73 102
sleep 9.157
noteon 13 57 104
sleep 1.831
noteoff 3 61 0
noteoff 3 64 0
sleep 1.831
noteon 15 45 40
sleep 1.831
noteon 14 33 106
sleep 65.933
noteoff 10 81 0
sleep 3.663
noteoff 11 76 0
sleep 7.326
noteoff 12 73 0
sleep 7.325
noteon 10 81 102
sleep 1.831
noteoff 15 45 0
noteoff 13 57 0
sleep 1.831
noteon 11 76 102
sleep 3.663
noteoff 14 33 0
sleep 3.663
noteon 12 73 102
sleep 9.157
noteon 13 57 104
sleep 3.662
noteon 15 45 43
sleep 1.831
noteon 14 33 106
sleep 65.932
noteoff 10 81 0
sleep 3.663
noteoff 11 76 0
sleep 7.326
noteoff 12 73 0
sleep 7.326
echo "153840 tempo_s=294 tempo_l=0.25"
noteon 10 79 102
sleep 1.7
noteoff 15 45 0
noteoff 13 57 0
sleep 1.7
noteon 11 74 102
sleep 3.401
noteoff 14 33 0
sleep 3.401
noteon 12 71 102
sleep 8.503
noteon 13 57 104
sleep 3.401
noteon 15 45 46
sleep 1.7
noteon 14 33 106
sleep 61.221
noteoff 10 79 0
sleep 3.401
noteoff 11 74 0
sleep 6.802
noteoff 12 71 0
sleep 6.802
noteon 10 79 102
sleep 1.7
noteoff 15 45 0
noteoff 13 57 0
sleep 1.7
noteon 11 74 102
sleep 3.401
noteoff 14 33 0
sleep 3.401
noteon 12 71 102
sleep 8.503
noteon 13 57 104
sleep 3.400
noteon 15 45 49
sleep 1.7
noteon 14 33 106
sleep 61.221
noteoff 10 79 0
sleep 3.401
noteoff 11 74 0
sleep 6.802
noteoff 12 71 0
sleep 6.802
noteon 10 79 102
sleep 1.7
noteoff 15 45 0
noteoff 13 57 0
sleep 1.7
noteon 11 74 102
sleep 3.401
noteoff 14 33 0
sleep 3.401
noteon 12 71 102
sleep 8.503
noteon 13 57 104
sleep 3.401
noteon 15 45 51
sleep 1.7
noteon 14 33 106
sleep 61.221
noteoff 10 79 0
sleep 3.401
noteoff 11 74 0
sleep 6.802
noteoff 12 71 0
sleep 6.802
noteon 10 79 102
sleep 1.7
noteoff 15 45 0
noteoff 13 57 0
sleep 1.7
noteon 11 74 102
sleep 3.401
noteoff 14 33 0
sleep 3.401
noteon 12 71 102
sleep 8.503
noteon 13 57 104
sleep 3.400
noteon 15 45 54
sleep 1.7
noteon 14 33 106
sleep 61.221
noteoff 10 79 0
sleep 3.401
noteoff 11 74 0
sleep 6.802
noteoff 12 71 0
sleep 6.802
echo "154080 tempo_s=273 tempo_l=0.25"
noteon 10 78 102
sleep 1.831
noteoff 15 45 0
noteoff 13 57 0
sleep 1.831
noteon 11 74 102
sleep 3.663
noteoff 14 33 0
sleep 3.663
noteon 12 69 102
sleep 9.157
noteon 13 57 104
sleep 3.663
noteon 15 45 57
sleep 1.831
noteon 14 33 106
sleep 65.930
noteoff 10 78 0
sleep 3.663
noteoff 11 74 0
sleep 7.326
noteoff 12 69 0
sleep 7.325
noteon 10 78 102
sleep 1.831
noteoff 15 45 0
noteoff 13 57 0
sleep 1.831
noteon 11 74 102
sleep 3.663
noteoff 14 33 0
sleep 3.663
noteon 12 69 102
sleep 9.157
noteon 13 57 104
sleep 3.662
noteon 15 45 60
sleep 1.831
noteon 14 33 106
sleep 65.932
noteoff 10 78 0
sleep 3.662
noteoff 11 74 0
sleep 7.325
noteoff 12 69 0
sleep 7.326
noteon 10 78 102
sleep 1.831
noteoff 15 45 0
noteoff 13 57 0
noteon 0 81 101
sleep 1.831
noteon 1 69 100
noteon 11 74 102
sleep 3.663
noteoff 14 33 0
sleep 3.662
noteon 12 69 102
sleep 9.157
noteon 13 57 104
sleep 1.831
noteon 3 57 100
sleep 1.831
noteon 15 45 63
sleep 1.831
noteon 14 33 106
sleep 65.929
noteoff 10 78 0
sleep 3.663
noteoff 11 74 0
sleep 7.325
noteoff 12 69 0
sleep 7.325
noteon 10 78 102
sleep 1.831
noteoff 15 45 0
noteoff 13 57 0
sleep 1.831
noteon 11 74 102
sleep 3.662
noteoff 14 33 0
sleep 3.663
noteon 12 69 102
sleep 9.157
noteon 13 57 104
sleep 3.662
noteon 15 45 65
sleep 1.831
noteon 14 33 106
sleep 65.929
noteoff 10 78 0
sleep 3.663
noteoff 11 74 0
sleep 7.325
noteoff 12 69 0
sleep 7.325
echo "154320 tempo_s=291 tempo_l=0.25"
noteon 10 76 102
sleep 1.718
noteoff 0 81 0
noteoff 15 45 0
noteoff 13 57 0
noteon 0 82 101
sleep 1.718
noteoff 1 69 0
noteon 1 70 100
noteon 11 73 102
sleep 3.436
noteoff 14 33 0
sleep 3.436
noteon 12 70 102
noteon 12 67 102
sleep 8.590
noteon 13 57 104
sleep 1.718
noteoff 3 57 0
noteon 3 58 100
sleep 1.718
noteon 15 45 68
sleep 1.718
noteon 14 33 106
sleep 61.851
noteoff 10 76 0
sleep 3.436
noteoff 11 73 0
sleep 6.872
noteoff 12 67 0
noteoff 12 70 0
sleep 6.872
noteon 10 76 102
sleep 1.718
noteoff 15 45 0
noteoff 13 57 0
sleep 1.718
noteon 11 73 102
sleep 3.436
noteoff 14 33 0
sleep 3.436
noteon 12 67 102
noteon 12 70 102
sleep 8.590
noteon 13 57 104
sleep 3.436
noteon 15 45 71
sleep 1.718
noteon 14 33 106
sleep 61.848
noteoff 10 76 0
sleep 3.436
noteoff 11 73 0
sleep 6.872
noteoff 12 70 0
noteoff 12 67 0
sleep 6.872
noteon 10 76 102
sleep 1.718
noteoff 0 82 0
noteoff 15 45 0
noteoff 13 57 0
noteon 0 81 101
sleep 1.718
noteoff 1 70 0
noteon 1 69 100
noteon 11 73 102
sleep 3.436
noteoff 14 33 0
sleep 3.436
noteon 12 67 102
noteon 12 69 102
sleep 8.590
noteon 13 57 104
sleep 1.718
noteoff 3 58 0
noteon 3 57 100
sleep 1.718
noteon 15 45 74
sleep 1.718
noteon 14 33 106
sleep 61.851
noteoff 10 76 0
sleep 3.436
noteoff 11 73 0
sleep 6.872
noteoff 12 69 0
noteoff 12 67 0
sleep 6.872
echo "154500 tempo_s=240 tempo_l=0.25"
noteon 10 76 102
sleep 2.083
noteoff 15 45 0
noteoff 13 57 0
sleep 2.083
noteon 11 73 102
sleep 4.166
noteoff 14 33 0
sleep 4.166
noteon 12 67 102
noteon 12 69 102
sleep 10.415
noteon 13 57 104
sleep 4.166
noteon 15 45 77
sleep 2.083
noteon 14 33 106
sleep 74.993
noteoff 10 76 0
sleep 4.166
noteoff 11 73 0
sleep 8.333
noteoff 0 81 0
noteoff 12 69 0
noteoff 12 67 0
sleep 2.083
noteoff 1 69 0
sleep 6.25
echo "154560 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
noteon 10 62 102
sleep 1.798
noteoff 15 45 0
noteoff 13 57 0
noteon 0 86 101
sleep 1.798
noteoff 4 69 0
noteon 1 74 100
noteon 11 62 102
noteon 4 62 100
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 1.798
noteoff 14 33 0
sleep 3.597
noteoff 5 57 0
noteon 12 69 102
noteon 12 66 102
noteon 5 50 100
sleep 1.798
noteoff 3 57 0
sleep 7.193
noteon 13 62 104
sleep 1.798
noteon 3 50 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 84.521
noteoff 13 62 0
sleep 17.983
noteon 13 50 104
sleep 88.118
noteoff 10 62 0
noteoff 10 74 0
sleep 1.798
noteoff 0 86 0
noteoff 13 50 0
sleep 1.798
noteoff 1 74 0
noteoff 4 62 0
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 5.395
noteoff 5 50 0
noteoff 12 66 0
noteoff 12 69 0
sleep 8.992
noteon 13 50 104
sleep 3.597
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 84.522
noteoff 13 50 0
sleep 17.983
noteon 13 50 104
sleep 88.118
echo "154800 tempo_s=301 tempo_l=0.25"
sleep 1.661
noteoff 13 50 0
sleep 1.661
noteoff 11 62 0
noteon 11 64 102
sleep 14.949
noteon 13 50 104
sleep 1.661
noteoff 3 50 0
noteon 3 52 100
sleep 81.395
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 83.056
noteoff 13 50 0
sleep 1.661
noteoff 11 64 0
noteon 11 66 102
sleep 14.95
noteon 13 50 104
sleep 1.661
noteoff 3 52 0
noteon 3 54 100
sleep 81.395
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 68.106
noteoff 11 66 0
sleep 13.289
echo "155040 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 13 50 0
sleep 1.798
noteon 11 67 102
sleep 8.992
noteoff 3 54 0
sleep 7.194
noteon 13 50 104
sleep 1.798
noteon 3 55 100
sleep 88.129
noteoff 13 50 0
sleep 17.985
noteon 13 50 104
sleep 89.928
noteoff 13 50 0
sleep 17.985
noteon 13 50 104
sleep 89.928
noteoff 13 50 0
sleep 17.985
noteon 13 50 104
sleep 88.129
echo "155280 tempo_s=301 tempo_l=0.25"
sleep 1.661
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 83.056
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 68.106
noteoff 11 67 0
sleep 14.95
noteoff 13 50 0
sleep 1.661
noteon 11 67 102
sleep 8.305
noteoff 3 55 0
sleep 6.644
noteon 13 50 104
sleep 1.661
noteon 3 55 100
sleep 81.395
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 68.106
noteoff 11 67 0
sleep 13.289
echo "155520 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
noteon 10 62 102
sleep 1.798
noteoff 13 50 0
sleep 1.798
noteon 4 62 100
noteon 11 66 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 5.395
noteon 5 50 100
noteon 12 62 102
sleep 1.798
noteoff 3 55 0
sleep 7.193
noteon 13 50 104
sleep 1.798
noteon 3 50 100
noteon 3 54 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 84.520
noteoff 13 50 0
sleep 17.983
noteon 13 50 104
sleep 88.117
noteoff 10 62 0
noteoff 10 74 0
sleep 1.798
noteoff 13 50 0
sleep 1.798
noteoff 4 62 0
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 5.395
noteoff 5 50 0
sleep 8.992
noteon 13 50 104
sleep 3.596
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 84.520
noteoff 13 50 0
sleep 17.983
noteon 13 50 104
sleep 88.117
echo "155760 tempo_s=301 tempo_l=0.25"
sleep 1.661
noteoff 13 50 0
sleep 1.661
noteoff 11 66 0
noteon 11 67 102
sleep 6.644
noteoff 12 62 0
noteon 12 64 102
sleep 8.305
noteon 13 50 104
sleep 1.661
noteoff 3 54 0
noteoff 3 50 0
noteon 3 55 100
noteon 3 52 100
sleep 81.395
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 83.056
noteoff 13 50 0
sleep 1.661
noteoff 11 67 0
noteon 11 69 102
sleep 6.644
noteoff 12 64 0
noteon 12 66 102
sleep 8.305
noteon 13 50 104
sleep 1.661
noteoff 3 52 0
noteoff 3 55 0
noteon 3 54 100
noteon 3 57 100
sleep 81.395
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 68.106
noteoff 11 69 0
sleep 6.644
noteoff 12 66 0
sleep 6.644
echo "156000 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 13 50 0
sleep 1.798
noteon 11 70 102
sleep 7.194
noteon 12 67 102
sleep 1.798
noteoff 3 57 0
noteoff 3 54 0
sleep 7.194
noteon 13 50 104
sleep 1.798
noteon 3 55 100
noteon 3 58 100
sleep 88.129
noteoff 13 50 0
sleep 17.985
noteon 13 50 104
sleep 89.928
noteoff 13 50 0
sleep 17.985
noteon 13 50 104
sleep 89.928
noteoff 13 50 0
sleep 17.985
noteon 13 50 104
sleep 88.129
echo "156240 tempo_s=301 tempo_l=0.25"
sleep 1.661
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 83.056
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 68.106
noteoff 11 70 0
sleep 6.644
noteoff 12 67 0
sleep 8.305
noteoff 13 50 0
sleep 1.661
noteon 11 70 102
sleep 6.644
noteon 12 67 102
sleep 1.661
noteoff 3 58 0
noteoff 3 55 0
sleep 6.644
noteon 13 50 104
sleep 1.661
noteon 3 58 100
noteon 3 55 100
sleep 81.395
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 68.106
noteoff 11 70 0
sleep 6.644
noteoff 12 67 0
sleep 6.644
echo "156480 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteoff 13 50 0
sleep 1.798
noteon 4 62 100
noteon 11 69 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 5.395
noteon 5 50 100
noteon 12 66 102
sleep 1.798
noteoff 3 55 0
noteoff 3 58 0
sleep 7.193
noteon 13 50 104
sleep 1.798
noteon 3 54 100
noteon 3 57 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 64.739
noteoff 10 74 0
sleep 17.983
noteon 10 86 102
sleep 1.798
noteoff 13 50 0
sleep 17.983
noteon 13 50 104
sleep 70.134
noteoff 10 86 0
sleep 17.983
noteon 10 86 102
sleep 1.798
noteoff 13 50 0
sleep 1.798
noteoff 4 62 0
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 5.395
noteoff 5 50 0
sleep 8.992
noteon 13 50 104
sleep 3.596
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 64.739
noteoff 10 86 0
sleep 17.983
noteon 10 86 102
sleep 1.798
noteoff 13 50 0
sleep 17.983
noteon 13 50 104
sleep 70.134
noteoff 10 86 0
sleep 17.983
echo "156720 tempo_s=301 tempo_l=0.25"
noteon 10 86 102
sleep 1.661
noteoff 13 50 0
sleep 1.661
noteoff 11 69 0
noteon 11 67 102
sleep 6.644
noteoff 12 66 0
noteon 12 64 102
sleep 8.305
noteon 13 50 104
sleep 1.661
noteoff 3 57 0
noteoff 3 54 0
noteon 3 52 100
noteon 3 55 100
sleep 63.122
noteoff 10 86 0
sleep 16.611
noteon 10 86 102
sleep 1.661
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 64.784
noteoff 10 86 0
sleep 16.611
noteon 10 86 102
sleep 1.661
noteoff 13 50 0
sleep 1.661
noteoff 11 67 0
noteon 11 66 102
sleep 6.644
noteoff 12 64 0
noteon 12 62 102
sleep 8.305
noteon 13 50 104
sleep 1.661
noteoff 3 55 0
noteoff 3 52 0
noteon 3 50 100
noteon 3 54 100
sleep 63.122
noteoff 10 86 0
sleep 16.611
noteon 10 86 102
sleep 1.661
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 64.784
noteoff 10 86 0
sleep 3.322
noteoff 11 66 0
sleep 6.644
noteoff 12 62 0
sleep 6.644
echo "156960 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteoff 13 50 0
noteon 0 88 101
noteon 0 85 101
sleep 1.798
noteon 1 76 100
noteon 1 73 100
noteon 4 62 100
noteon 11 70 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 64 101
noteon 2 73 101
sleep 1.798
noteon 5 50 100
noteon 12 67 102
sleep 1.798
noteoff 3 54 0
noteoff 3 50 0
sleep 7.193
noteon 13 50 104
sleep 1.798
noteon 3 58 100
noteon 3 55 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 64.736
noteoff 10 74 0
sleep 17.982
noteon 10 85 102
sleep 1.798
noteoff 13 50 0
sleep 17.982
noteon 13 50 104
sleep 70.132
noteoff 10 85 0
sleep 17.982
noteon 10 85 102
sleep 1.798
noteoff 13 50 0
sleep 1.798
noteoff 4 62 0
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 5.394
noteoff 5 50 0
sleep 8.991
noteon 13 50 104
sleep 3.596
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 64.737
noteoff 10 85 0
sleep 17.981
noteon 10 85 102
sleep 1.798
noteoff 13 50 0
sleep 17.983
noteon 13 50 104
sleep 70.130
noteoff 10 85 0
sleep 17.983
echo "157200 tempo_s=301 tempo_l=0.25"
noteon 10 85 102
sleep 1.661
noteoff 0 85 0
noteoff 0 88 0
noteoff 13 50 0
noteon 0 90 101
noteon 0 86 101
sleep 1.661
noteoff 1 73 0
noteoff 1 76 0
noteoff 11 70 0
noteon 1 78 100
noteon 1 74 100
noteon 11 69 102
sleep 4.983
noteoff 2 73 0
noteoff 2 64 0
noteon 2 74 101
noteon 2 66 101
sleep 1.661
noteoff 12 67 0
noteon 12 66 102
sleep 8.305
noteon 13 50 104
sleep 1.661
noteoff 3 55 0
noteoff 3 58 0
noteon 3 54 100
noteon 3 57 100
sleep 63.122
noteoff 10 85 0
sleep 16.611
noteon 10 85 102
sleep 1.661
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 64.784
noteoff 10 85 0
sleep 16.611
noteon 10 85 102
sleep 1.661
noteoff 0 86 0
noteoff 0 90 0
noteoff 13 50 0
noteon 0 91 101
noteon 0 88 101
sleep 1.661
noteoff 1 74 0
noteoff 1 78 0
noteoff 11 69 0
noteon 1 79 100
noteon 1 76 100
noteon 11 67 102
sleep 4.983
noteoff 2 66 0
noteoff 2 74 0
noteon 2 67 101
noteon 2 76 101
sleep 1.661
noteoff 12 66 0
noteon 12 64 102
sleep 8.305
noteon 13 50 104
sleep 1.661
noteoff 3 57 0
noteoff 3 54 0
noteon 3 55 100
noteon 3 52 100
sleep 63.122
noteoff 10 85 0
sleep 16.611
noteon 10 85 102
sleep 1.661
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 64.784
noteoff 10 85 0
sleep 3.322
noteoff 11 67 0
sleep 6.644
noteoff 0 88 0
noteoff 0 91 0
noteoff 12 64 0
sleep 1.661
noteoff 1 76 0
noteoff 1 79 0
sleep 4.983
echo "157440 tempo_s=278 tempo_l=0.25"
noteoff 2 76 0
noteoff 2 67 0
noteon 10 74 102
sleep 1.798
noteoff 13 50 0
noteon 0 86 101
noteon 0 90 101
sleep 1.798
noteon 1 74 100
noteon 1 78 100
noteon 4 62 100
noteon 11 69 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 66 101
noteon 2 74 101
sleep 1.798
noteon 5 50 100
noteon 12 66 102
sleep 1.798
noteoff 3 52 0
noteoff 3 55 0
sleep 7.193
noteon 13 50 104
sleep 1.798
noteon 3 54 100
noteon 3 57 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 64.739
noteoff 10 74 0
sleep 17.983
noteon 10 86 102
sleep 1.798
noteoff 13 50 0
sleep 17.983
noteon 13 50 104
sleep 70.134
noteoff 10 86 0
sleep 17.983
noteon 10 86 102
sleep 1.798
noteoff 0 90 0
noteoff 0 86 0
noteoff 13 50 0
sleep 1.798
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 62 0
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 3.597
noteoff 2 74 0
noteoff 2 66 0
sleep 1.798
noteoff 5 50 0
sleep 8.992
noteon 13 50 104
sleep 3.596
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 64.739
noteoff 10 86 0
sleep 17.983
noteon 10 86 102
sleep 1.798
noteoff 13 50 0
sleep 17.983
noteon 13 50 104
sleep 70.134
noteoff 10 86 0
sleep 17.983
echo "157680 tempo_s=301 tempo_l=0.25"
noteon 10 86 102
sleep 1.661
noteoff 13 50 0
sleep 1.661
noteoff 11 69 0
noteon 11 67 102
sleep 6.644
noteoff 12 66 0
noteon 12 64 102
sleep 8.305
noteon 13 50 104
sleep 1.661
noteoff 3 57 0
noteoff 3 54 0
noteon 3 52 100
noteon 3 55 100
sleep 63.122
noteoff 10 86 0
sleep 16.611
noteon 10 86 102
sleep 1.661
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 64.784
noteoff 10 86 0
sleep 16.611
noteon 10 86 102
sleep 1.661
noteoff 13 50 0
sleep 1.661
noteoff 11 67 0
noteon 11 66 102
sleep 6.644
noteoff 12 64 0
noteon 12 62 102
sleep 8.305
noteon 13 50 104
sleep 1.661
noteoff 3 55 0
noteoff 3 52 0
noteon 3 50 100
noteon 3 54 100
sleep 63.122
noteoff 10 86 0
sleep 16.611
noteon 10 86 102
sleep 1.661
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 64.784
noteoff 10 86 0
sleep 3.322
noteoff 11 66 0
sleep 6.644
noteoff 12 62 0
sleep 6.644
echo "157920 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteoff 13 50 0
noteon 0 88 101
noteon 0 85 101
sleep 1.798
noteon 1 76 100
noteon 1 73 100
noteon 4 62 100
noteon 11 70 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 73 101
noteon 2 64 101
sleep 1.798
noteon 5 50 100
noteon 12 67 102
sleep 8.991
noteon 13 50 104
sleep 1.798
noteoff 3 54 0
noteoff 3 50 0
noteon 3 58 100
noteon 3 55 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 64.736
noteoff 10 74 0
sleep 17.982
noteon 10 85 102
sleep 1.798
noteoff 13 50 0
sleep 17.982
noteon 13 50 104
sleep 70.132
noteoff 10 85 0
sleep 17.982
noteon 10 85 102
sleep 1.798
noteoff 13 50 0
sleep 1.798
noteoff 4 62 0
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 5.394
noteoff 5 50 0
sleep 8.991
noteon 13 50 104
sleep 3.596
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 64.737
noteoff 10 85 0
sleep 17.981
noteon 10 85 102
sleep 1.798
noteoff 13 50 0
sleep 17.983
noteon 13 50 104
sleep 70.130
noteoff 10 85 0
sleep 17.983
echo "158160 tempo_s=301 tempo_l=0.25"
noteon 10 85 102
sleep 1.661
noteoff 0 85 0
noteoff 0 88 0
noteoff 13 50 0
noteon 0 90 101
noteon 0 86 101
sleep 1.661
noteoff 1 73 0
noteoff 1 76 0
noteoff 11 70 0
noteon 1 78 100
noteon 1 74 100
noteon 11 69 102
sleep 4.983
noteoff 2 64 0
noteoff 2 73 0
noteon 2 66 101
noteon 2 74 101
sleep 1.661
noteoff 12 67 0
noteon 12 66 102
sleep 8.305
noteon 13 50 104
sleep 1.661
noteoff 3 55 0
noteoff 3 58 0
noteon 3 54 100
noteon 3 57 100
sleep 63.122
noteoff 10 85 0
sleep 16.611
noteon 10 85 102
sleep 1.661
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 64.784
noteoff 10 85 0
sleep 16.611
noteon 10 85 102
sleep 1.661
noteoff 0 86 0
noteoff 0 90 0
noteoff 13 50 0
noteon 0 91 101
noteon 0 88 101
sleep 1.661
noteoff 1 74 0
noteoff 1 78 0
noteoff 11 69 0
noteon 1 79 100
noteon 1 76 100
noteon 11 67 102
sleep 4.983
noteoff 2 74 0
noteoff 2 66 0
noteon 2 76 101
noteon 2 67 101
sleep 1.661
noteoff 12 66 0
noteon 12 64 102
sleep 8.305
noteon 13 50 104
sleep 1.661
noteoff 3 57 0
noteoff 3 54 0
noteon 3 52 100
noteon 3 55 100
sleep 63.122
noteoff 10 85 0
sleep 16.611
noteon 10 85 102
sleep 1.661
noteoff 13 50 0
sleep 16.611
noteon 13 50 104
sleep 64.784
noteoff 10 85 0
sleep 3.322
noteoff 11 67 0
sleep 6.644
noteoff 0 88 0
noteoff 0 91 0
noteoff 12 64 0
sleep 1.661
noteoff 1 76 0
noteoff 1 79 0
sleep 4.983
echo "158400 tempo_s=278 tempo_l=0.25"
noteoff 2 67 0
noteoff 2 76 0
noteon 10 86 102
sleep 1.798
noteoff 13 50 0
noteon 0 86 101
noteon 0 90 101
sleep 1.798
noteon 1 74 100
noteon 1 78 100
noteon 4 62 100
noteon 11 69 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteon 2 66 101
noteon 2 74 101
sleep 1.798
noteon 5 50 100
noteon 12 66 102
sleep 1.798
noteoff 3 55 0
noteoff 3 52 0
sleep 7.194
noteon 13 50 104
sleep 1.798
noteon 3 54 100
noteon 3 57 100
sleep 1.798
noteon 15 50 80
sleep 1.798
noteon 14 38 106
sleep 64.748
noteoff 10 86 0
sleep 3.597
noteoff 11 69 0
sleep 14.388
noteon 10 74 102
sleep 3.597
noteon 11 69 102
sleep 86.33
noteoff 10 74 0
sleep 3.597
noteoff 11 69 0
sleep 7.194
noteoff 12 66 0
sleep 7.194
noteon 10 74 102
sleep 1.798
noteoff 0 90 0
noteoff 0 86 0
noteon 0 78 101
sleep 1.798
noteoff 1 78 0
noteoff 1 74 0
noteon 1 66 100
noteon 11 66 102
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 3.597
noteoff 2 74 0
noteoff 2 66 0
sleep 1.798
noteon 12 54 102
sleep 10.791
noteoff 3 57 0
noteoff 3 54 0
noteon 3 54 100
noteon 3 50 100
sleep 1.798
noteoff 15 50 0
sleep 66.546
noteoff 10 74 0
sleep 3.597
noteoff 11 66 0
sleep 14.388
noteon 10 74 102
sleep 1.798
noteoff 0 78 0
sleep 1.798
noteoff 1 66 0
noteon 11 66 102
sleep 86.33
noteoff 10 74 0
sleep 3.597
noteoff 11 66 0
sleep 14.388
echo "158640 tempo_s=301 tempo_l=0.25"
noteon 10 74 102
sleep 1.661
noteon 0 79 101
noteon 0 86 101
sleep 1.661
noteon 1 67 100
noteon 1 74 100
noteon 11 67 102
sleep 4.983
noteon 2 74 101
noteon 2 62 101
sleep 1.661
noteoff 12 54 0
noteon 12 55 102
sleep 8.305
noteoff 13 50 0
noteon 13 52 104
sleep 1.661
noteoff 3 50 0
noteoff 3 54 0
noteon 3 52 100
noteon 3 55 100
sleep 3.322
noteoff 14 38 0
noteon 14 40 106
sleep 59.796
noteoff 10 74 0
sleep 3.322
noteoff 11 67 0
sleep 13.288
noteon 10 74 102
sleep 3.322
noteon 11 67 102
sleep 79.728
noteoff 10 74 0
sleep 3.322
noteoff 11 67 0
sleep 13.288
noteon 10 74 102
sleep 1.661
noteoff 0 79 0
noteon 0 81 101
sleep 1.661
noteoff 1 67 0
noteon 11 69 102
noteon 1 69 100
sleep 6.644
noteoff 12 55 0
noteon 12 57 102
sleep 8.305
noteoff 13 52 0
noteon 13 54 104
sleep 1.661
noteoff 3 55 0
noteoff 3 52 0
noteon 3 57 100
noteon 3 54 100
sleep 3.322
noteoff 14 40 0
noteon 14 42 106
sleep 59.8
noteoff 10 74 0
sleep 3.322
noteoff 11 69 0
sleep 13.289
noteon 10 74 102
sleep 3.322
noteon 11 69 102
sleep 79.734
noteoff 10 74 0
sleep 3.322
noteoff 11 69 0
sleep 6.644
noteoff 0 81 0
noteoff 12 57 0
sleep 1.661
noteoff 1 69 0
sleep 4.983
echo "158880 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteoff 13 54 0
noteon 0 83 101
sleep 1.798
noteon 1 71 100
noteon 11 71 102
sleep 3.597
noteoff 14 42 0
sleep 3.597
noteon 12 59 102
sleep 1.798
noteoff 3 54 0
noteoff 3 57 0
sleep 7.194
noteon 13 55 104
sleep 1.798
noteon 3 59 100
noteon 3 55 100
sleep 3.597
noteon 14 43 106
sleep 64.739
noteoff 10 74 0
sleep 3.596
noteoff 11 71 0
sleep 14.386
noteon 10 71 102
sleep 3.597
noteon 11 71 102
sleep 86.319
noteoff 10 71 0
sleep 3.597
noteoff 11 71 0
sleep 7.193
noteoff 0 86 0
sleep 1.798
noteoff 1 74 0
sleep 5.395
noteoff 2 62 0
noteoff 2 74 0
noteon 10 76 102
sleep 1.798
noteon 0 83 101
sleep 1.798
noteon 1 71 100
noteon 11 71 102
sleep 5.395
noteon 2 71 101
noteon 2 59 101
sleep 80.924
noteoff 10 76 0
sleep 3.596
noteoff 11 71 0
sleep 14.386
noteon 10 76 102
sleep 1.798
noteoff 0 83 0
sleep 1.798
noteoff 1 71 0
noteon 11 71 102
sleep 5.395
noteoff 2 59 0
noteoff 2 71 0
sleep 80.929
noteoff 10 76 0
sleep 3.596
noteoff 11 71 0
sleep 14.388
echo "159120 tempo_s=301 tempo_l=0.25"
noteon 10 76 102
sleep 1.661
noteoff 0 83 0
noteon 0 81 101
noteon 0 88 101
sleep 1.661
noteoff 1 71 0
noteoff 4 62 0
noteon 11 69 102
noteon 1 69 100
noteon 1 76 100
noteon 4 64 100
sleep 4.983
noteon 2 76 101
noteon 2 64 101
sleep 1.661
noteoff 5 50 0
noteoff 12 59 0
noteon 12 57 102
noteon 5 64 100
sleep 8.305
noteoff 13 55 0
noteon 13 54 104
sleep 1.661
noteoff 3 55 0
noteoff 3 59 0
noteon 3 54 100
noteon 3 57 100
sleep 3.322
noteoff 14 43 0
noteon 14 42 106
sleep 59.796
noteoff 10 76 0
sleep 3.322
noteoff 11 69 0
sleep 13.288
noteon 10 76 102
sleep 3.322
noteon 11 69 102
sleep 79.728
noteoff 10 76 0
sleep 3.322
noteoff 11 69 0
sleep 13.288
noteon 10 76 102
sleep 1.661
noteoff 0 81 0
noteon 0 79 101
sleep 1.661
noteoff 1 69 0
noteon 11 67 102
noteon 1 67 100
sleep 6.644
noteoff 12 57 0
noteon 12 55 102
sleep 8.305
noteoff 13 54 0
noteon 13 52 104
sleep 1.661
noteoff 3 57 0
noteoff 3 54 0
noteon 3 55 100
noteon 3 52 100
sleep 3.322
noteoff 14 42 0
noteon 14 40 106
sleep 59.799
noteoff 10 76 0
sleep 3.322
noteoff 11 67 0
sleep 13.288
noteon 10 76 102
sleep 3.322
noteon 11 67 102
sleep 79.732
noteoff 10 76 0
sleep 3.322
noteoff 11 67 0
sleep 6.644
noteoff 0 79 0
noteoff 12 55 0
sleep 1.661
noteoff 1 67 0
sleep 4.983
echo "159360 tempo_s=278 tempo_l=0.25"
noteon 10 76 102
sleep 1.798
noteoff 13 52 0
noteon 0 85 101
sleep 1.798
noteon 1 73 100
noteon 11 73 102
sleep 3.597
noteoff 14 40 0
sleep 3.597
noteon 12 61 102
sleep 1.798
noteoff 3 52 0
noteoff 3 55 0
sleep 7.194
noteon 13 57 104
sleep 1.798
noteon 3 61 100
noteon 3 57 100
sleep 3.597
noteon 14 45 106
sleep 64.739
noteoff 10 76 0
sleep 3.596
noteoff 11 73 0
sleep 14.386
noteon 10 73 102
sleep 3.597
noteon 11 73 102
sleep 86.319
noteoff 10 73 0
sleep 3.597
noteoff 11 73 0
sleep 7.193
noteoff 0 88 0
sleep 1.798
noteoff 1 76 0
sleep 5.395
noteoff 2 64 0
noteoff 2 76 0
noteon 10 78 102
sleep 1.798
noteon 0 85 101
sleep 1.798
noteon 1 73 100
noteon 11 73 102
sleep 5.395
noteon 2 61 101
noteon 2 73 101
sleep 80.924
noteoff 10 78 0
sleep 3.596
noteoff 11 73 0
sleep 14.386
noteon 10 78 102
sleep 1.798
noteoff 0 85 0
sleep 1.798
noteoff 1 73 0
noteon 11 73 102
sleep 5.395
noteoff 2 73 0
noteoff 2 61 0
sleep 80.929
noteoff 10 78 0
sleep 3.596
noteoff 11 73 0
sleep 14.388
echo "159600 tempo_s=293 tempo_l=0.25"
noteon 10 78 102
sleep 1.706
noteoff 0 85 0
noteon 0 83 101
noteon 0 90 101
sleep 1.706
noteoff 1 73 0
noteoff 4 64 0
noteon 11 71 102
noteon 1 71 100
noteon 1 78 100
noteon 4 66 100
sleep 5.118
noteon 2 78 101
noteon 2 66 101
sleep 1.706
noteoff 5 64 0
noteoff 12 61 0
noteon 12 59 102
noteon 5 66 100
sleep 8.531
noteoff 13 57 0
noteon 13 55 104
sleep 1.706
noteoff 3 57 0
noteoff 3 61 0
noteon 3 59 100
noteon 3 55 100
sleep 3.412
noteoff 14 45 0
noteon 14 43 106
sleep 61.421
noteoff 10 78 0
sleep 3.412
noteoff 11 71 0
sleep 13.649
noteon 10 78 102
sleep 3.412
noteon 11 71 102
sleep 81.894
noteoff 10 78 0
sleep 3.412
noteoff 11 71 0
sleep 13.648
noteon 10 78 102
sleep 1.706
noteoff 0 83 0
noteon 0 81 101
sleep 1.706
noteoff 1 71 0
noteon 11 69 102
noteon 1 69 100
sleep 6.825
noteoff 12 59 0
noteon 12 57 102
sleep 8.532
noteoff 13 55 0
noteon 13 54 104
sleep 1.706
noteoff 3 55 0
noteoff 3 59 0
noteon 3 57 100
noteon 3 54 100
sleep 3.412
noteoff 14 43 0
noteon 14 42 106
sleep 61.428
noteoff 10 78 0
sleep 3.412
noteoff 11 69 0
sleep 13.650
noteon 10 78 102
sleep 3.412
noteon 11 69 102
sleep 81.904
noteoff 10 78 0
sleep 3.412
noteoff 11 69 0
sleep 6.824
noteoff 0 81 0
noteoff 12 57 0
sleep 1.706
noteoff 1 69 0
sleep 5.119
echo "159840 tempo_s=271 tempo_l=0.25"
noteon 10 78 102
sleep 1.845
noteoff 13 54 0
noteon 0 86 101
sleep 1.845
noteon 1 74 100
noteon 11 74 102
sleep 3.69
noteoff 14 42 0
sleep 3.69
noteon 12 62 102
sleep 1.845
noteoff 3 54 0
noteoff 3 57 0
sleep 7.380
noteon 13 59 104
sleep 1.845
noteon 3 62 100
noteon 3 59 100
sleep 3.69
noteon 14 47 106
sleep 66.420
noteoff 10 78 0
sleep 3.690
noteoff 11 74 0
sleep 14.760
noteon 10 74 102
sleep 3.69
noteon 11 74 102
sleep 88.560
noteoff 10 74 0
sleep 3.69
noteoff 11 74 0
sleep 7.380
noteoff 0 90 0
sleep 1.845
noteoff 1 78 0
sleep 5.535
noteoff 2 66 0
noteoff 2 78 0
noteon 10 79 102
sleep 1.845
noteon 0 86 101
sleep 1.845
noteon 1 74 100
noteon 11 74 102
sleep 5.535
noteon 2 62 101
noteon 2 74 101
sleep 83.025
noteoff 10 79 0
sleep 3.690
noteoff 11 74 0
sleep 14.760
noteon 10 79 102
sleep 1.845
noteoff 0 86 0
sleep 1.845
noteoff 1 74 0
noteon 11 74 102
sleep 5.535
noteoff 2 74 0
noteoff 2 62 0
sleep 83.025
noteoff 10 79 0
sleep 3.690
noteoff 11 74 0
sleep 14.760
echo "160080 tempo_s=288 tempo_l=0.25"
noteon 10 79 102
sleep 1.736
noteoff 0 86 0
noteon 0 85 101
noteon 0 91 101
sleep 1.736
noteoff 1 74 0
noteoff 4 66 0
noteon 4 67 100
noteon 11 73 102
noteon 1 73 100
noteon 1 79 100
sleep 5.208
noteon 2 76 101
noteon 2 64 101
sleep 1.736
noteoff 5 66 0
noteoff 12 62 0
noteon 5 67 100
noteon 12 61 102
sleep 8.680
noteoff 13 59 0
noteon 13 57 104
sleep 1.736
noteoff 3 59 0
noteoff 3 62 0
noteon 3 57 100
noteon 3 61 100
sleep 3.472
noteoff 14 47 0
noteon 14 45 106
sleep 62.496
noteoff 10 79 0
sleep 3.472
noteoff 11 73 0
sleep 13.888
noteon 10 79 102
sleep 3.472
noteon 11 73 102
sleep 17.360
noteoff 1 73 0
sleep 65.968
noteoff 10 79 0
sleep 3.472
noteoff 11 73 0
sleep 13.888
noteon 10 79 102
sleep 1.736
noteoff 0 85 0
noteon 0 83 101
sleep 1.736
noteon 1 71 100
noteon 11 71 102
sleep 6.944
noteoff 12 61 0
noteon 12 59 102
sleep 8.680
noteoff 13 57 0
noteon 13 55 104
sleep 1.736
noteoff 3 61 0
noteoff 3 57 0
noteon 3 55 100
noteon 3 59 100
sleep 3.472
noteoff 14 45 0
noteon 14 43 106
sleep 62.496
noteoff 10 79 0
sleep 3.472
noteoff 11 71 0
sleep 13.888
echo "160260 tempo_s=166 tempo_l=0.25"
noteon 10 79 102
sleep 6.024
noteon 11 71 102
sleep 144.576
noteoff 10 79 0
sleep 6.024
noteoff 11 71 0
sleep 12.048
noteoff 0 83 0
noteoff 12 59 0
sleep 3.012
noteoff 1 71 0
sleep 9.036
echo "160320 tempo_s=261 tempo_l=0.25"
noteon 10 69 102
noteon 10 79 102
sleep 1.915
noteoff 13 55 0
noteon 0 88 101
sleep 1.915
noteoff 4 67 0
noteon 1 76 100
noteon 11 76 102
noteon 11 69 102
noteon 4 64 100
sleep 1.915
noteon 6 69 108
noteon 6 76 108
sleep 1.915
noteoff 14 43 0
sleep 3.831
noteoff 5 67 0
noteon 12 64 102
noteon 5 57 100
sleep 1.915
noteoff 3 59 0
noteoff 3 55 0
sleep 7.662
noteon 13 61 104
sleep 1.915
noteon 3 61 100
noteon 3 64 100
sleep 1.915
noteon 15 45 90
sleep 1.915
noteon 14 49 106
sleep 59.386
noteoff 15 45 0
sleep 15.325
noteon 15 45 90
sleep 61.302
noteoff 15 45 0
sleep 15.325
noteon 15 45 90
sleep 51.724
echo "160440 tempo_s=256 tempo_l=0.25"
sleep 9.765
noteoff 15 45 0
sleep 15.625
noteon 15 45 90
sleep 62.5
noteoff 15 45 0
sleep 15.625
noteon 15 45 90
sleep 62.5
noteoff 15 45 0
sleep 15.625
noteon 15 45 90
sleep 52.734
echo "160560 tempo_s=245 tempo_l=0.25"
sleep 10.204
noteoff 15 45 0
sleep 16.326
noteon 15 45 90
sleep 65.306
noteoff 15 45 0
sleep 16.326
noteon 15 45 90
sleep 65.306
noteoff 15 45 0
sleep 16.326
noteon 15 45 90
sleep 55.102
echo "160680 tempo_s=229 tempo_l=0.25"
sleep 10.917
noteoff 15 45 0
sleep 17.467
noteon 15 45 90
sleep 69.868
noteoff 15 45 0
sleep 17.467
noteon 15 45 90
sleep 69.868
noteoff 15 45 0
sleep 17.467
noteon 15 45 90
sleep 58.951
echo "160800 tempo_s=212 tempo_l=0.25"
sleep 11.792
noteoff 15 45 0
sleep 18.867
noteon 15 45 90
sleep 75.471
noteoff 15 45 0
sleep 18.867
noteon 15 45 90
sleep 75.471
noteoff 15 45 0
sleep 18.867
noteon 15 45 90
sleep 63.679
echo "160920 tempo_s=190 tempo_l=0.25"
sleep 13.157
noteoff 15 45 0
sleep 21.052
noteon 15 45 90
sleep 84.21
noteoff 15 45 0
sleep 21.052
noteon 15 45 90
sleep 84.21
noteoff 15 45 0
sleep 21.052
noteon 15 45 90
sleep 71.052
echo "161040 tempo_s=173 tempo_l=0.25"
sleep 14.45
noteoff 15 45 0
sleep 23.121
noteon 15 45 90
sleep 92.485
noteoff 15 45 0
sleep 23.121
noteon 15 45 95
sleep 92.480
noteoff 15 45 0
sleep 23.120
noteon 15 45 101
sleep 78.030
echo "161160 tempo_s=140 tempo_l=0.25"
sleep 17.855
noteoff 15 45 0
sleep 28.568
noteon 15 45 95
sleep 203.547
noteoff 10 79 0
noteoff 10 69 0
sleep 3.571
noteoff 0 88 0
noteoff 0 91 0
sleep 3.571
noteoff 1 76 0
noteoff 1 79 0
noteoff 4 64 0
noteoff 11 69 0
noteoff 11 76 0
sleep 3.571
noteoff 6 76 0
noteoff 6 69 0
noteoff 15 45 0
sleep 7.142
noteoff 2 64 0
noteoff 2 76 0
sleep 3.571
noteoff 5 57 0
noteoff 12 64 0
sleep 14.284
echo "161240 tempo_s=17 tempo_l=0.25"
sleep 29.411
noteoff 13 61 0
sleep 29.411
noteoff 3 64 0
noteoff 3 61 0
sleep 58.823
noteoff 14 49 0
sleep 1058.823
echo "161280 tempo_s=180 tempo_l=0.25"
noteon 10 66 74
sleep 5.555
noteon 11 61 74
sleep 11.111
noteon 12 54 74
sleep 13.887
noteon 13 46 76
sleep 8.333
noteon 14 34 78
sleep 2502.673
noteoff 10 66 0
sleep 5.555
noteoff 11 61 0
sleep 8.333
echo "162200 tempo_s=28 tempo_l=0.25"
sleep 17.857
noteoff 12 54 0
sleep 89.285
noteoff 13 46 0
sleep 53.571
noteoff 14 34 0
sleep 553.571
echo "162240 tempo_s=263 tempo_l=0.25"
noteon 10 66 72
sleep 3.802
noteon 4 66 85
noteon 11 61 72
sleep 7.604
noteon 5 54 85
noteon 12 58 72
sleep 9.505
noteon 13 54 74
sleep 5.703
noteon 14 42 76
sleep 155.882
noteoff 10 66 0
sleep 3.802
noteoff 11 61 0
sleep 7.604
noteoff 12 58 0
sleep 9.505
noteoff 13 54 0
sleep 5.703
noteoff 14 42 0
sleep 19.010
noteon 10 66 82
sleep 3.802
noteon 11 61 82
sleep 7.604
noteon 12 58 82
sleep 171.094
noteoff 10 66 0
sleep 3.802
noteoff 11 61 0
sleep 7.604
noteoff 12 58 0
sleep 34.22
echo "162480 tempo_s=301 tempo_l=0.25"
sleep 18.272
noteon 13 52 94
sleep 4.983
noteon 14 40 96
sleep 154.485
noteoff 13 52 0
sleep 4.983
noteoff 14 40 0
sleep 16.611
noteon 10 66 102
sleep 3.322
noteon 11 61 102
sleep 6.644
noteon 12 58 102
sleep 149.501
noteoff 10 66 0
sleep 3.322
noteoff 11 61 0
sleep 6.644
noteoff 12 58 0
sleep 29.9
echo "162720 tempo_s=263 tempo_l=0.25"
sleep 20.912
noteon 13 50 104
sleep 5.703
noteon 14 38 106
sleep 176.806
noteoff 13 50 0
sleep 5.703
noteoff 14 38 0
sleep 19.011
noteon 10 66 102
sleep 3.802
noteon 11 62 102
sleep 7.604
noteon 12 59 102
sleep 171.102
noteoff 10 66 0
sleep 3.802
noteoff 11 62 0
sleep 7.604
noteoff 12 59 0
sleep 34.22
echo "162960 tempo_s=301 tempo_l=0.25"
sleep 18.272
noteon 13 49 104
sleep 4.983
noteon 14 37 106
sleep 154.485
noteoff 13 49 0
sleep 4.983
noteoff 14 37 0
sleep 16.611
noteon 10 66 102
sleep 3.322
noteon 11 64 102
sleep 6.644
noteon 12 58 102
sleep 149.501
noteoff 10 66 0
sleep 3.322
noteoff 11 64 0
sleep 6.644
noteoff 12 58 0
sleep 29.9
echo "163200 tempo_s=263 tempo_l=0.25"
noteon 10 66 102
sleep 3.802
noteon 11 62 102
sleep 5.703
noteon 2 71 101
sleep 1.901
noteon 12 59 102
sleep 9.505
noteon 13 47 104
sleep 1.901
noteon 3 59 100
sleep 3.802
noteon 14 35 106
sleep 155.882
noteoff 10 66 0
sleep 3.802
noteoff 11 62 0
sleep 7.604
noteoff 12 59 0
sleep 9.505
noteoff 13 47 0
sleep 5.703
noteoff 14 35 0
sleep 19.010
noteon 10 71 102
sleep 3.802
noteoff 4 66 0
noteon 11 66 102
sleep 7.604
noteoff 5 54 0
noteon 12 62 102
sleep 171.101
noteoff 10 71 0
sleep 3.802
noteoff 11 66 0
sleep 7.604
noteoff 12 62 0
sleep 34.22
echo "163440 tempo_s=301 tempo_l=0.25"
sleep 18.272
noteon 13 57 104
sleep 4.983
noteon 14 45 106
sleep 154.485
noteoff 13 57 0
sleep 4.983
noteoff 14 45 0
sleep 16.611
noteon 10 71 102
sleep 3.322
noteon 11 66 102
sleep 6.644
noteon 12 63 102
sleep 149.501
noteoff 10 71 0
sleep 3.322
noteoff 11 66 0
sleep 6.644
noteoff 12 63 0
sleep 29.9
echo "163680 tempo_s=263 tempo_l=0.25"
sleep 20.912
noteon 13 55 104
sleep 5.703
noteon 14 43 106
sleep 176.806
noteoff 13 55 0
sleep 5.703
noteoff 14 43 0
sleep 19.011
noteon 10 71 102
sleep 3.802
noteon 11 67 102
sleep 7.604
noteon 12 64 102
sleep 171.102
noteoff 10 71 0
sleep 3.802
noteoff 11 67 0
sleep 7.604
noteoff 12 64 0
sleep 34.22
echo "163920 tempo_s=301 tempo_l=0.25"
sleep 18.272
noteon 13 54 104
sleep 4.983
noteon 14 42 106
sleep 154.485
noteoff 13 54 0
sleep 4.983
noteoff 14 42 0
sleep 16.611
noteon 10 71 102
sleep 3.322
noteon 11 69 102
sleep 6.644
noteon 12 63 102
sleep 149.501
noteoff 10 71 0
sleep 3.322
noteoff 11 69 0
sleep 6.644
noteoff 12 63 0
sleep 29.9
echo "164160 tempo_s=263 tempo_l=0.25"
noteon 10 71 102
sleep 3.802
noteon 1 76 100
noteon 11 67 102
sleep 7.604
noteon 12 64 102
sleep 9.505
noteon 13 52 104
sleep 5.703
noteon 14 40 106
sleep 155.882
noteoff 10 71 0
sleep 3.802
noteoff 11 67 0
sleep 7.604
noteoff 12 64 0
sleep 9.505
noteoff 13 52 0
sleep 5.703
noteoff 14 40 0
sleep 19.010
noteon 10 76 102
sleep 3.802
noteon 11 67 102
sleep 5.703
noteoff 2 71 0
sleep 1.901
noteon 12 59 102
sleep 11.406
noteoff 3 59 0
sleep 159.695
noteoff 10 76 0
sleep 3.802
noteoff 11 67 0
sleep 7.604
noteoff 12 59 0
sleep 34.22
echo "164400 tempo_s=301 tempo_l=0.25"
sleep 18.272
noteon 13 50 104
sleep 4.983
noteon 14 38 106
sleep 154.485
noteoff 13 50 0
sleep 4.983
noteoff 14 38 0
sleep 16.611
noteon 10 76 102
sleep 3.322
noteon 11 68 102
sleep 6.644
noteon 12 59 102
sleep 149.501
noteoff 10 76 0
sleep 3.322
noteoff 11 68 0
sleep 6.644
noteoff 12 59 0
sleep 29.9
echo "164640 tempo_s=263 tempo_l=0.25"
sleep 20.912
noteon 13 49 104
sleep 5.703
noteon 14 37 106
sleep 176.806
noteoff 13 49 0
sleep 5.703
noteoff 14 37 0
sleep 19.011
noteon 10 76 102
sleep 3.802
noteon 11 69 102
sleep 7.604
noteon 12 61 102
sleep 171.102
noteoff 10 76 0
sleep 3.802
noteoff 11 69 0
sleep 7.604
noteoff 12 61 0
sleep 34.22
echo "164880 tempo_s=301 tempo_l=0.25"
sleep 18.272
noteon 13 47 104
sleep 4.983
noteon 14 35 106
sleep 154.485
noteoff 13 47 0
sleep 4.983
noteoff 14 35 0
sleep 16.611
noteon 10 76 102
sleep 3.322
noteon 11 68 102
sleep 6.644
noteon 12 62 102
sleep 149.501
noteoff 10 76 0
sleep 3.322
noteoff 11 68 0
sleep 6.644
noteoff 12 62 0
sleep 29.9
echo "165120 tempo_s=263 tempo_l=0.25"
noteon 10 76 102
sleep 3.802
noteoff 1 76 0
noteon 4 64 100
noteon 11 67 102
noteon 1 73 100
noteon 1 76 100
sleep 7.604
noteon 5 57 100
noteon 12 61 102
sleep 9.505
noteon 13 45 104
sleep 5.703
noteon 14 33 106
sleep 155.885
noteoff 10 76 0
sleep 3.802
noteoff 11 67 0
sleep 7.604
noteoff 12 61 0
sleep 9.505
noteoff 13 45 0
sleep 5.703
noteoff 14 33 0
sleep 19.010
noteon 10 64 102
sleep 3.802
noteon 11 61 102
sleep 7.604
noteon 12 57 102
sleep 171.094
noteoff 10 64 0
sleep 3.802
noteoff 11 61 0
sleep 7.604
noteoff 12 57 0
sleep 34.219
echo "165360 tempo_s=301 tempo_l=0.25"
sleep 3.322
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteon 1 78 100
noteon 1 74 100
noteon 4 66 100
sleep 6.644
noteoff 5 57 0
noteon 5 62 100
sleep 8.305
noteon 13 50 104
sleep 4.983
noteon 14 38 106
sleep 154.477
noteoff 13 50 0
sleep 4.983
noteoff 14 38 0
sleep 16.610
noteon 10 66 102
sleep 3.322
noteon 11 62 102
sleep 6.644
noteon 12 57 102
sleep 149.494
noteoff 10 66 0
sleep 3.322
noteoff 11 62 0
sleep 6.644
noteoff 12 57 0
sleep 29.898
echo "165600 tempo_s=263 tempo_l=0.25"
sleep 3.802
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 66 0
noteon 1 76 100
noteon 1 79 100
noteon 4 67 100
sleep 7.604
noteoff 5 62 0
noteon 5 64 100
sleep 9.505
noteon 13 43 104
sleep 5.703
noteon 14 31 106
sleep 176.797
noteoff 13 43 0
sleep 5.703
noteoff 14 31 0
sleep 19.011
noteon 10 67 102
sleep 3.802
noteon 11 64 102
sleep 7.604
noteon 12 59 102
sleep 171.093
noteoff 10 67 0
sleep 3.802
noteoff 11 64 0
sleep 7.604
noteoff 12 59 0
sleep 34.220
echo "165840 tempo_s=301 tempo_l=0.25"
sleep 3.322
noteoff 1 79 0
noteoff 1 76 0
noteoff 4 67 0
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
sleep 6.644
noteoff 5 64 0
noteon 5 57 100
sleep 8.305
noteon 13 45 104
sleep 4.983
noteon 14 33 106
sleep 154.479
noteoff 13 45 0
sleep 4.983
noteoff 14 33 0
sleep 16.611
noteon 10 64 102
sleep 3.322
noteon 11 61 102
sleep 6.644
noteon 12 57 102
sleep 89.697
echo "166020 tempo_s=200 tempo_l=0.25"
sleep 90.000
noteoff 10 64 0
sleep 5.0
noteoff 11 61 0
sleep 10.0
noteoff 12 57 0
sleep 45.0
echo "166080 tempo_s=278 tempo_l=0.25"
noteon 10 66 102
sleep 3.597
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
sleep 5.395
select 12 1 0 45
sleep 1.798
noteoff 5 57 0
noteon 12 66 27
sleep 7.194
select 13 1 0 45
sleep 1.798
noteon 13 54 29
sleep 3.597
select 14 1 0 45
sleep 1.798
noteon 14 42 31
sleep 190.647
noteoff 10 66 0
sleep 3.597
noteon 11 66 102
sleep 7.194
noteoff 12 66 0
sleep 8.992
noteoff 13 54 0
sleep 5.395
noteoff 14 42 0
sleep 82.733
noteon 10 77 102
sleep 3.597
noteoff 11 66 0
sleep 104.316
echo "166320 tempo_s=305 tempo_l=0.25"
noteoff 10 77 0
noteon 10 78 102
sleep 3.278
noteon 11 58 102
sleep 6.557
noteon 12 64 27
sleep 8.196
noteon 13 52 29
sleep 1.639
noteon 3 58 100
sleep 3.278
noteon 14 40 31
sleep 78.688
noteoff 11 58 0
sleep 16.393
noteoff 3 58 0
sleep 78.688
noteoff 10 78 0
sleep 3.278
noteon 11 66 102
sleep 6.557
noteoff 12 64 0
sleep 8.196
noteoff 13 52 0
sleep 4.918
noteoff 14 40 0
sleep 75.409
noteon 10 77 102
sleep 3.278
noteoff 11 66 0
sleep 95.081
echo "166560 tempo_s=278 tempo_l=0.25"
noteoff 10 77 0
noteon 10 78 102
sleep 3.597
noteon 11 59 102
sleep 7.194
noteon 12 62 27
sleep 8.992
noteon 13 50 29
sleep 1.798
noteon 3 59 100
sleep 3.597
noteon 14 38 31
sleep 86.33
noteoff 11 59 0
sleep 17.985
noteoff 3 59 0
sleep 86.33
noteoff 10 78 0
sleep 3.597
noteon 11 66 102
sleep 7.194
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 5.395
noteoff 14 38 0
sleep 82.733
noteon 10 77 102
sleep 3.597
noteoff 11 66 0
sleep 104.316
echo "166800 tempo_s=305 tempo_l=0.25"
noteoff 10 77 0
noteon 10 78 102
sleep 3.278
noteon 11 61 102
sleep 6.557
noteon 12 61 27
sleep 8.196
noteon 13 49 29
sleep 1.639
noteon 3 61 100
sleep 3.278
noteon 14 37 31
sleep 78.688
noteoff 11 61 0
sleep 16.393
noteoff 3 61 0
sleep 78.688
noteoff 10 78 0
sleep 3.278
noteon 11 66 102
sleep 6.557
noteoff 12 61 0
sleep 8.196
noteoff 13 49 0
sleep 4.918
noteoff 14 37 0
sleep 75.409
noteon 10 77 102
sleep 3.278
noteoff 11 66 0
sleep 95.081
echo "167040 tempo_s=278 tempo_l=0.25"
noteoff 10 77 0
noteon 10 78 102
sleep 3.597
noteon 11 62 102
sleep 7.194
noteon 12 59 27
sleep 8.992
noteon 13 47 29
sleep 1.798
noteon 3 62 100
sleep 3.597
noteon 14 35 31
sleep 86.33
noteoff 11 62 0
sleep 17.985
noteoff 3 62 0
sleep 86.33
noteoff 10 78 0
noteon 10 59 102
sleep 10.791
noteoff 12 59 0
sleep 8.992
noteoff 13 47 0
sleep 5.395
noteoff 14 35 0
sleep 82.733
noteoff 10 59 0
sleep 1.798
noteon 0 82 101
sleep 1.798
noteon 11 70 102
sleep 104.316
echo "167280 tempo_s=305 tempo_l=0.25"
noteon 10 63 102
sleep 1.639
noteoff 0 82 0
noteon 0 83 101
sleep 1.639
noteoff 11 70 0
noteon 1 75 100
noteon 11 71 102
sleep 6.557
noteon 12 69 27
sleep 8.196
noteon 13 57 29
sleep 4.918
noteon 14 45 31
sleep 75.409
noteoff 10 63 0
sleep 3.278
noteoff 1 75 0
sleep 95.081
noteon 10 59 102
sleep 1.639
noteoff 0 83 0
sleep 1.639
noteoff 11 71 0
sleep 6.557
noteoff 12 69 0
sleep 8.196
noteoff 13 57 0
sleep 4.918
noteoff 14 45 0
sleep 75.409
noteoff 10 59 0
sleep 1.639
noteon 0 82 101
sleep 1.639
noteon 11 70 102
sleep 95.081
echo "167520 tempo_s=278 tempo_l=0.25"
noteon 10 64 102
sleep 1.798
noteoff 0 82 0
noteon 0 83 101
sleep 1.798
noteoff 11 70 0
noteon 1 76 100
noteon 11 71 102
sleep 7.194
noteon 12 67 27
sleep 8.992
noteon 13 55 29
sleep 5.395
noteon 14 43 31
sleep 82.733
noteoff 10 64 0
sleep 3.597
noteoff 1 76 0
sleep 104.316
noteon 10 59 102
sleep 1.798
noteoff 0 83 0
sleep 1.798
noteoff 11 71 0
sleep 7.194
noteoff 12 67 0
sleep 8.992
noteoff 13 55 0
sleep 5.395
noteoff 14 43 0
sleep 82.733
noteoff 10 59 0
sleep 1.798
noteon 0 82 101
sleep 1.798
noteon 11 70 102
sleep 104.316
echo "167760 tempo_s=305 tempo_l=0.25"
noteon 10 66 102
sleep 1.639
noteoff 0 82 0
noteon 0 83 101
sleep 1.639
noteoff 11 70 0
noteon 1 78 100
noteon 11 71 102
sleep 6.557
noteon 12 66 27
sleep 8.196
noteon 13 54 29
sleep 4.918
noteon 14 42 31
sleep 75.409
noteoff 10 66 0
sleep 3.278
noteoff 1 78 0
sleep 95.081
noteon 10 59 102
sleep 1.639
noteoff 0 83 0
sleep 1.639
noteoff 11 71 0
sleep 6.557
noteoff 12 66 0
sleep 8.196
noteoff 13 54 0
sleep 4.918
noteoff 14 42 0
sleep 75.409
noteoff 10 59 0
sleep 1.639
noteon 0 82 101
sleep 1.639
noteon 11 70 102
sleep 95.081
echo "168000 tempo_s=278 tempo_l=0.25"
noteon 10 67 102
sleep 1.798
noteoff 0 82 0
noteon 0 83 101
sleep 1.798
noteoff 11 70 0
noteon 1 79 100
noteon 11 71 102
sleep 7.194
noteon 12 64 27
sleep 8.992
noteon 13 52 29
sleep 5.395
noteon 14 40 31
sleep 82.733
noteoff 10 67 0
sleep 3.597
noteoff 1 79 0
sleep 106.115
noteoff 0 83 0
sleep 1.798
noteoff 11 71 0
noteon 11 64 102
sleep 7.194
noteoff 12 64 0
sleep 8.992
noteoff 13 52 0
sleep 5.395
noteoff 14 40 0
sleep 82.733
noteon 10 75 102
sleep 3.597
noteoff 11 64 0
sleep 104.316
echo "168240 tempo_s=305 tempo_l=0.25"
noteoff 10 75 0
noteon 10 76 102
sleep 3.278
noteon 11 56 102
sleep 6.557
noteon 12 62 27
sleep 8.196
noteon 13 50 29
sleep 1.639
noteon 3 56 100
sleep 3.278
noteon 14 38 31
sleep 78.688
noteoff 11 56 0
sleep 16.393
noteoff 3 56 0
sleep 78.688
noteoff 10 76 0
sleep 3.278
noteon 11 64 102
sleep 6.557
noteoff 12 62 0
sleep 8.196
noteoff 13 50 0
sleep 4.918
noteoff 14 38 0
sleep 75.409
noteon 10 75 102
sleep 3.278
noteoff 11 64 0
sleep 95.081
echo "168480 tempo_s=278 tempo_l=0.25"
noteoff 10 75 0
noteon 10 76 102
sleep 3.597
noteon 11 57 102
sleep 7.194
noteon 12 61 27
sleep 8.992
noteon 13 49 29
sleep 1.798
noteon 3 57 100
sleep 3.597
noteon 14 37 31
sleep 86.33
noteoff 11 57 0
sleep 17.985
noteoff 3 57 0
sleep 86.33
noteoff 10 76 0
sleep 3.597
noteon 11 64 102
sleep 7.194
noteoff 12 61 0
sleep 8.992
noteoff 13 49 0
sleep 5.395
noteoff 14 37 0
sleep 82.733
noteon 10 75 102
sleep 3.597
noteoff 11 64 0
sleep 104.316
echo "168720 tempo_s=305 tempo_l=0.25"
noteoff 10 75 0
noteon 10 76 102
sleep 3.278
noteon 11 59 102
sleep 6.557
noteon 12 59 27
sleep 8.196
noteon 13 47 29
sleep 1.639
noteon 3 59 100
sleep 3.278
noteon 14 35 31
sleep 78.688
noteoff 11 59 0
sleep 16.393
noteoff 3 59 0
sleep 78.688
noteoff 10 76 0
sleep 3.278
noteon 11 64 102
sleep 6.557
noteoff 12 59 0
sleep 8.196
noteoff 13 47 0
sleep 4.918
noteoff 14 35 0
sleep 75.409
noteon 10 75 102
sleep 3.278
noteoff 11 64 0
sleep 95.081
echo "168960 tempo_s=278 tempo_l=0.25"
noteoff 10 75 0
noteon 10 76 102
sleep 3.597
noteon 11 61 102
sleep 7.194
noteon 12 57 27
sleep 8.992
noteon 13 45 29
sleep 1.798
noteon 3 61 100
sleep 3.597
noteon 14 33 31
sleep 86.33
noteoff 11 61 0
sleep 17.985
noteoff 3 61 0
sleep 86.33
noteoff 10 76 0
noteon 10 57 102
sleep 10.791
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 5.395
noteoff 14 33 0
sleep 82.733
noteoff 10 57 0
sleep 1.798
noteon 0 80 101
sleep 1.798
noteon 11 68 102
sleep 104.316
echo "169200 tempo_s=305 tempo_l=0.25"
noteon 10 61 102
sleep 1.639
noteoff 0 80 0
noteon 0 81 101
sleep 1.639
noteoff 11 68 0
noteon 1 73 100
noteon 11 69 102
sleep 6.557
noteon 12 67 27
sleep 8.196
noteon 13 55 29
sleep 4.918
noteon 14 43 31
sleep 75.409
noteoff 10 61 0
sleep 3.278
noteoff 1 73 0
sleep 95.081
noteon 10 57 102
sleep 1.639
noteoff 0 81 0
sleep 1.639
noteoff 11 69 0
sleep 6.557
noteoff 12 67 0
sleep 8.196
noteoff 13 55 0
sleep 4.918
noteoff 14 43 0
sleep 75.409
noteoff 10 57 0
sleep 1.639
noteon 0 80 101
sleep 1.639
noteon 11 68 102
sleep 95.081
echo "169440 tempo_s=278 tempo_l=0.25"
noteon 10 62 102
sleep 1.798
noteoff 0 80 0
noteon 0 81 101
sleep 1.798
noteoff 11 68 0
noteon 1 74 100
noteon 11 69 102
sleep 7.194
noteon 12 66 27
sleep 8.992
noteon 13 54 29
sleep 5.395
noteon 14 42 31
sleep 82.733
noteoff 10 62 0
sleep 3.597
noteoff 1 74 0
sleep 104.316
noteon 10 57 102
sleep 1.798
noteoff 0 81 0
sleep 1.798
noteoff 11 69 0
sleep 7.194
noteoff 12 66 0
sleep 8.992
noteoff 13 54 0
sleep 5.395
noteoff 14 42 0
sleep 82.733
noteoff 10 57 0
sleep 1.798
noteon 0 80 101
sleep 1.798
noteon 11 68 102
sleep 104.316
echo "169680 tempo_s=305 tempo_l=0.25"
noteon 10 64 102
sleep 1.639
noteoff 0 80 0
noteon 0 81 101
sleep 1.639
noteoff 11 68 0
noteon 1 76 100
noteon 11 69 102
sleep 6.557
noteon 12 64 27
sleep 8.196
noteon 13 52 29
sleep 4.918
noteon 14 40 31
sleep 75.409
noteoff 10 64 0
sleep 3.278
noteoff 1 76 0
sleep 95.081
noteon 10 57 102
sleep 1.639
noteoff 0 81 0
sleep 1.639
noteoff 11 69 0
sleep 6.557
noteoff 12 64 0
sleep 8.196
noteoff 13 52 0
sleep 4.918
noteoff 14 40 0
sleep 75.409
noteoff 10 57 0
sleep 1.639
noteon 0 80 101
sleep 1.639
noteon 11 68 102
sleep 95.081
echo "169920 tempo_s=278 tempo_l=0.25"
noteon 10 66 102
sleep 1.798
noteoff 0 80 0
noteon 0 81 101
sleep 1.798
noteoff 11 68 0
noteon 1 78 100
noteon 11 69 102
sleep 7.194
noteon 12 62 27
sleep 8.992
noteon 13 50 29
sleep 1.798
noteon 3 66 100
sleep 3.597
noteon 14 38 31
sleep 82.733
noteoff 10 66 0
sleep 3.597
noteoff 1 78 0
sleep 17.985
noteoff 3 66 0
sleep 88.129
noteoff 0 81 0
sleep 1.798
noteoff 11 69 0
noteon 11 62 102
sleep 7.194
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 5.395
noteoff 14 38 0
sleep 82.733
noteon 10 85 102
sleep 3.597
noteoff 11 62 0
sleep 104.316
echo "170160 tempo_s=300 tempo_l=0.25"
noteoff 10 85 0
noteon 10 86 102
sleep 3.333
noteon 11 66 102
sleep 6.666
noteon 12 60 27
sleep 8.333
noteon 13 48 29
sleep 1.666
noteon 3 54 100
sleep 3.333
noteon 14 36 31
sleep 80.0
noteoff 11 66 0
sleep 16.666
noteoff 3 54 0
sleep 80.0
noteoff 10 86 0
sleep 3.333
noteon 11 62 102
sleep 6.666
noteoff 12 60 0
sleep 8.333
noteoff 13 48 0
sleep 5.0
noteoff 14 36 0
sleep 76.666
noteon 10 85 102
sleep 3.333
noteoff 11 62 0
sleep 96.666
echo "170400 tempo_s=278 tempo_l=0.25"
noteoff 10 85 0
noteon 10 86 102
sleep 3.597
noteon 11 67 102
sleep 7.194
noteon 12 59 27
sleep 8.992
noteon 13 47 29
sleep 1.798
noteon 3 55 100
sleep 3.597
noteon 14 35 31
sleep 86.33
noteoff 11 67 0
sleep 17.985
noteoff 3 55 0
sleep 86.33
noteoff 10 86 0
sleep 3.597
noteon 11 62 102
sleep 7.194
noteoff 12 59 0
sleep 8.992
noteoff 13 47 0
sleep 5.395
noteoff 14 35 0
sleep 82.733
noteon 10 85 102
sleep 3.597
noteoff 11 62 0
sleep 104.316
echo "170640 tempo_s=296 tempo_l=0.25"
noteoff 10 85 0
noteon 10 86 102
sleep 3.378
noteon 11 69 102
sleep 6.756
noteon 12 57 27
sleep 8.445
noteon 13 45 29
sleep 1.689
noteon 3 57 100
sleep 3.378
noteon 14 33 31
sleep 81.081
noteoff 11 69 0
sleep 16.891
noteoff 3 57 0
sleep 81.081
noteoff 10 86 0
sleep 3.378
noteon 11 62 102
sleep 6.756
noteoff 12 57 0
sleep 8.445
noteoff 13 45 0
sleep 5.067
noteoff 14 33 0
sleep 77.702
noteon 10 85 102
sleep 3.378
noteoff 11 62 0
sleep 97.972
echo "170880 tempo_s=276 tempo_l=0.25"
noteoff 10 85 0
noteon 10 86 102
sleep 3.623
noteon 11 71 102
sleep 7.246
noteon 12 55 27
sleep 9.057
noteon 13 43 29
sleep 1.811
noteon 3 59 100
sleep 3.623
noteon 14 31 31
sleep 86.956
noteoff 11 71 0
sleep 18.115
noteoff 3 59 0
sleep 86.956
noteoff 10 86 0
sleep 3.623
noteon 11 62 102
sleep 7.246
noteoff 12 55 0
sleep 9.057
noteoff 13 43 0
sleep 5.434
noteoff 14 31 0
sleep 83.333
noteon 10 85 102
sleep 3.623
noteoff 11 62 0
sleep 105.072
echo "171120 tempo_s=295 tempo_l=0.25"
noteoff 10 85 0
noteon 10 86 102
sleep 3.389
noteon 1 66 100
noteon 11 66 102
sleep 6.779
noteon 12 60 27
sleep 8.474
noteon 13 48 29
sleep 1.694
noteon 3 54 100
sleep 3.389
noteon 14 36 31
sleep 81.355
noteoff 1 66 0
noteoff 11 66 0
sleep 16.949
noteoff 3 54 0
sleep 81.355
noteoff 10 86 0
sleep 3.389
noteon 11 62 102
sleep 6.779
noteoff 12 60 0
sleep 8.474
noteoff 13 48 0
sleep 5.084
noteoff 14 36 0
sleep 77.966
noteon 10 85 102
sleep 3.389
noteoff 11 62 0
sleep 98.305
echo "171360 tempo_s=275 tempo_l=0.25"
noteoff 10 85 0
noteon 10 86 102
sleep 3.636
noteon 1 67 100
noteon 11 67 102
sleep 7.272
noteon 12 59 27
sleep 9.09
noteon 13 47 29
sleep 1.818
noteon 3 55 100
sleep 3.636
noteon 14 35 31
sleep 87.272
noteoff 1 67 0
noteoff 11 67 0
sleep 18.181
noteoff 3 55 0
sleep 87.272
noteoff 10 86 0
sleep 3.636
noteon 11 62 102
sleep 7.272
noteoff 12 59 0
sleep 9.09
noteoff 13 47 0
sleep 5.454
noteoff 14 35 0
sleep 83.636
noteon 10 85 102
sleep 3.636
noteoff 11 62 0
sleep 105.454
echo "171600 tempo_s=293 tempo_l=0.25"
noteoff 10 85 0
noteon 10 86 102
sleep 3.412
noteon 1 69 100
noteon 11 69 102
sleep 6.825
noteon 12 57 27
sleep 8.532
noteon 13 45 29
sleep 1.706
noteon 3 57 100
sleep 3.412
noteon 14 33 31
sleep 81.911
noteoff 1 69 0
noteoff 11 69 0
sleep 17.064
noteoff 3 57 0
sleep 81.911
noteoff 10 86 0
sleep 3.412
noteon 11 62 102
sleep 6.825
noteoff 12 57 0
sleep 8.532
noteoff 13 45 0
sleep 5.119
noteoff 14 33 0
sleep 78.498
noteon 10 85 102
sleep 3.412
noteoff 11 62 0
sleep 98.976
echo "171840 tempo_s=274 tempo_l=0.25"
noteoff 10 85 0
noteon 10 74 102
noteon 10 86 102
sleep 1.824
noteon 0 83 101
sleep 1.824
noteon 1 71 100
noteon 4 50 85
noteon 11 71 102
sleep 5.474
select 12 1 0 48
sleep 1.824
noteon 5 38 85
noteon 12 55 102
sleep 7.298
select 13 1 0 48
sleep 1.824
noteon 13 43 104
sleep 1.824
noteon 3 59 100
sleep 1.824
select 14 1 0 48
sleep 1.824
noteon 14 31 106
sleep 85.749
noteoff 0 83 0
sleep 1.824
noteoff 1 71 0
noteoff 11 71 0
sleep 18.245
noteoff 3 59 0
sleep 306.560
echo "172080 tempo_s=257 tempo_l=0.25"
sleep 1.945
noteon 0 78 101
sleep 1.945
noteon 1 66 100
noteon 11 74 102
noteon 11 62 102
sleep 7.782
noteoff 12 55 0
noteon 12 60 102
sleep 9.727
noteoff 13 43 0
noteon 13 48 104
sleep 1.945
noteon 3 54 100
sleep 3.891
noteoff 14 31 0
noteon 14 36 106
sleep 441.626
noteoff 0 78 0
noteon 0 79 101
sleep 1.945
noteoff 1 66 0
noteon 1 67 100
sleep 7.782
noteoff 12 60 0
noteon 12 59 102
sleep 9.727
noteoff 13 48 0
noteon 13 47 104
sleep 1.945
noteoff 3 54 0
noteon 3 55 100
sleep 3.891
noteoff 14 36 0
noteon 14 35 106
sleep 439.686
echo "172560 tempo_s=252 tempo_l=0.25"
sleep 1.984
noteoff 0 79 0
noteon 0 81 101
sleep 1.984
noteoff 1 67 0
noteon 1 69 100
sleep 7.936
noteoff 12 59 0
noteon 12 57 102
sleep 9.92
noteoff 13 47 0
noteon 13 45 104
sleep 1.984
noteoff 3 55 0
noteon 3 57 100
sleep 3.968
noteoff 14 35 0
noteon 14 33 106
sleep 450.392
noteoff 0 81 0
noteon 0 83 101
sleep 1.984
noteoff 1 69 0
noteon 1 71 100
sleep 7.936
noteoff 12 57 0
noteon 12 55 102
sleep 9.92
noteoff 13 45 0
noteon 13 43 104
sleep 1.984
noteoff 3 57 0
noteon 3 59 100
sleep 3.968
noteoff 14 33 0
noteon 14 31 106
sleep 448.409
echo "173040 tempo_s=248 tempo_l=0.25"
sleep 2.016
noteoff 0 83 0
noteon 0 78 101
sleep 2.016
noteoff 1 71 0
noteon 1 66 100
sleep 8.064
noteoff 12 55 0
noteon 12 60 102
sleep 10.08
noteoff 13 43 0
noteon 13 48 104
sleep 2.016
noteoff 3 59 0
noteon 3 54 100
sleep 4.032
noteoff 14 31 0
noteon 14 36 106
sleep 457.656
noteoff 0 78 0
noteon 0 79 101
sleep 2.016
noteoff 1 66 0
noteon 1 67 100
sleep 8.064
noteoff 12 60 0
noteon 12 59 102
sleep 10.08
noteoff 13 48 0
noteon 13 47 104
sleep 2.016
noteoff 3 54 0
noteon 3 55 100
sleep 4.032
noteoff 14 36 0
noteon 14 35 106
sleep 455.638
echo "173520 tempo_s=243 tempo_l=0.25"
sleep 2.057
noteoff 0 79 0
noteon 0 81 101
sleep 2.057
noteoff 1 67 0
noteon 1 69 100
sleep 8.23
noteoff 12 59 0
noteon 12 57 102
sleep 10.288
noteoff 13 47 0
noteon 13 45 104
sleep 2.057
noteoff 3 55 0
noteon 3 57 100
sleep 4.115
noteoff 14 35 0
noteon 14 33 106
sleep 465.012
echo "173760 tempo_s=280 tempo_l=0.25"
sleep 1.785
noteoff 0 81 0
noteon 0 83 101
sleep 1.785
noteoff 1 69 0
noteon 1 71 100
sleep 7.142
noteoff 12 57 0
noteon 12 55 102
sleep 8.928
noteoff 13 45 0
noteon 13 43 104
sleep 1.785
noteoff 3 57 0
noteon 3 59 100
sleep 3.571
noteoff 14 33 0
noteon 14 31 106
sleep 833.928
noteoff 0 83 0
noteon 0 78 101
sleep 1.785
noteoff 1 71 0
noteon 1 66 100
sleep 7.142
noteoff 12 55 0
noteon 12 60 102
sleep 8.928
noteoff 13 43 0
noteon 13 48 104
sleep 1.785
noteoff 3 59 0
noteon 3 54 100
sleep 3.571
noteoff 14 31 0
noteon 14 36 106
sleep 832.142
echo "174720 tempo_s=277 tempo_l=0.25"
sleep 1.805
noteoff 0 78 0
noteon 0 79 101
sleep 1.805
noteoff 1 66 0
noteon 1 67 100
sleep 7.22
noteoff 12 60 0
noteon 12 59 102
sleep 9.025
noteoff 13 48 0
noteon 13 47 104
sleep 1.805
noteoff 3 54 0
noteon 3 55 100
sleep 3.61
noteoff 14 36 0
noteon 14 35 106
sleep 842.96
noteoff 0 79 0
noteon 0 81 101
sleep 1.805
noteoff 1 67 0
noteon 1 69 100
sleep 7.22
noteoff 12 59 0
noteon 12 57 102
sleep 9.025
noteoff 13 47 0
noteon 13 45 104
sleep 1.805
noteoff 3 55 0
noteon 3 57 100
sleep 3.61
noteoff 14 35 0
noteon 14 33 106
sleep 841.142
echo "175680 tempo_s=273 tempo_l=0.25"
noteoff 10 86 0
noteoff 10 74 0
noteon 10 86 102
sleep 1.831
noteoff 0 81 0
noteon 0 83 69
sleep 1.831
noteoff 1 69 0
noteoff 11 62 0
noteoff 11 74 0
noteon 1 71 68
noteon 11 71 102
sleep 7.325
noteoff 12 57 0
noteon 12 55 102
sleep 9.157
noteoff 13 45 0
noteon 13 43 104
sleep 1.831
noteoff 3 57 0
noteon 3 59 68
sleep 3.663
noteoff 14 33 0
noteon 14 31 106
sleep 65.933
noteoff 10 86 0
sleep 3.663
noteoff 11 71 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.493
noteon 14 31 106
sleep 65.927
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.324
noteoff 12 55 0
sleep 7.325
noteon 10 86 102
sleep 1.831
noteoff 0 83 0
noteoff 13 43 0
sleep 1.831
noteoff 1 71 0
noteoff 4 50 0
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.662
noteoff 5 38 0
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 1.831
noteoff 3 59 0
sleep 3.663
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
noteon 10 86 102
sleep 1.831
noteoff 13 43 0
sleep 1.831
noteon 11 83 102
sleep 3.663
noteoff 14 31 0
sleep 3.663
noteon 12 55 102
sleep 9.157
noteon 13 43 104
sleep 5.494
noteon 14 31 106
sleep 65.934
noteoff 10 86 0
sleep 3.663
noteoff 11 83 0
sleep 7.326
noteoff 12 55 0
sleep 7.326
echo "176640 tempo_s=270 tempo_l=0.25"
noteon 10 83 102
sleep 1.851
noteoff 13 43 0
sleep 1.851
noteon 11 79 102
sleep 3.703
noteoff 14 31 0
sleep 3.703
noteon 12 55 102
sleep 9.259
noteon 13 43 104
sleep 5.555
noteon 14 31 106
sleep 66.666
noteoff 10 83 0
sleep 3.703
noteoff 11 79 0
sleep 7.407
noteoff 12 55 0
sleep 7.407
noteon 10 83 102
sleep 1.851
noteoff 13 43 0
sleep 1.851
noteon 11 79 102
sleep 3.703
noteoff 14 31 0
sleep 3.703
noteon 12 55 102
sleep 9.259
noteon 13 43 104
sleep 5.555
noteon 14 31 106
sleep 66.666
noteoff 10 83 0
sleep 3.703
noteoff 11 79 0
sleep 7.407
noteoff 12 55 0
sleep 7.407
noteon 10 83 102
sleep 1.851
noteoff 13 43 0
sleep 1.851
noteon 11 79 102
sleep 3.703
noteoff 14 31 0
sleep 3.703
noteon 12 55 102
sleep 9.259
noteon 13 43 104
sleep 5.555
noteon 14 31 106
sleep 66.666
noteoff 10 83 0
sleep 3.703
noteoff 11 79 0
sleep 7.407
noteoff 12 55 0
sleep 7.407
noteon 10 83 102
sleep 1.851
noteoff 13 43 0
sleep 1.851
noteon 11 79 102
sleep 3.703
noteoff 14 31 0
sleep 3.703
noteon 12 55 102
sleep 9.259
noteon 13 43 104
sleep 5.555
noteon 14 31 106
sleep 66.666
noteoff 10 83 0
sleep 3.703
noteoff 11 79 0
sleep 7.407
noteoff 12 55 0
sleep 7.407
noteon 10 83 102
sleep 1.851
noteoff 13 43 0
sleep 1.851
noteon 11 79 102
sleep 3.703
noteoff 14 31 0
sleep 3.703
noteon 12 55 102
sleep 9.259
noteon 13 43 104
sleep 5.555
noteon 14 31 106
sleep 66.666
noteoff 10 83 0
sleep 3.703
noteoff 11 79 0
sleep 7.407
noteoff 12 55 0
sleep 7.407
noteon 10 83 102
sleep 1.851
noteoff 13 43 0
sleep 1.851
noteon 11 79 102
sleep 3.703
noteoff 14 31 0
sleep 3.703
noteon 12 55 102
sleep 9.259
noteon 13 43 104
sleep 5.555
noteon 14 31 106
sleep 66.666
noteoff 10 83 0
sleep 3.703
noteoff 11 79 0
sleep 7.407
noteoff 12 55 0
sleep 7.407
noteon 10 83 102
sleep 1.851
noteoff 13 43 0
sleep 1.851
noteon 11 79 102
sleep 3.703
noteoff 14 31 0
sleep 3.703
noteon 12 55 102
sleep 9.259
noteon 13 43 104
sleep 5.555
noteon 14 31 106
sleep 66.666
noteoff 10 83 0
sleep 3.703
noteoff 11 79 0
sleep 7.407
noteoff 12 55 0
sleep 7.407
noteon 10 83 102
sleep 1.851
noteoff 13 43 0
sleep 1.851
noteon 11 79 102
sleep 3.703
noteoff 14 31 0
sleep 3.703
noteon 12 55 102
sleep 9.259
noteon 13 43 104
sleep 5.555
noteon 14 31 106
sleep 66.666
noteoff 10 83 0
sleep 3.703
noteoff 11 79 0
sleep 7.407
noteoff 12 55 0
sleep 7.407
echo "177120 tempo_s=265 tempo_l=0.25"
noteon 10 79 102
sleep 1.886
noteoff 13 43 0
sleep 1.886
noteon 11 74 102
sleep 3.773
noteoff 14 31 0
sleep 3.773
noteon 12 55 102
sleep 9.433
noteon 13 43 104
sleep 5.66
noteon 14 31 106
sleep 67.924
noteoff 10 79 0
sleep 3.773
noteoff 11 74 0
sleep 7.547
noteoff 12 55 0
sleep 7.547
noteon 10 79 102
sleep 1.886
noteoff 13 43 0
sleep 1.886
noteon 11 74 102
sleep 3.773
noteoff 14 31 0
sleep 3.773
noteon 12 55 102
sleep 9.433
noteon 13 43 104
sleep 5.66
noteon 14 31 106
sleep 67.924
noteoff 10 79 0
sleep 3.773
noteoff 11 74 0
sleep 7.547
noteoff 12 55 0
sleep 7.547
noteon 10 79 102
sleep 1.886
noteoff 13 43 0
sleep 1.886
noteon 11 74 102
sleep 3.773
noteoff 14 31 0
sleep 3.773
noteon 12 55 102
sleep 9.433
noteon 13 43 104
sleep 5.66
noteon 14 31 106
sleep 67.924
noteoff 10 79 0
sleep 3.773
noteoff 11 74 0
sleep 7.547
noteoff 12 55 0
sleep 7.547
noteon 10 79 102
sleep 1.886
noteoff 13 43 0
sleep 1.886
noteon 11 74 102
sleep 3.773
noteoff 14 31 0
sleep 3.773
noteon 12 55 102
sleep 9.433
noteon 13 43 104
sleep 5.66
noteon 14 31 106
sleep 67.924
noteoff 10 79 0
sleep 3.773
noteoff 11 74 0
sleep 7.547
noteoff 12 55 0
sleep 7.547
noteon 10 79 102
sleep 1.886
noteoff 13 43 0
sleep 1.886
noteon 11 74 102
sleep 3.773
noteoff 14 31 0
sleep 3.773
noteon 12 55 102
sleep 9.433
noteon 13 43 104
sleep 5.66
noteon 14 31 106
sleep 67.924
noteoff 10 79 0
sleep 3.773
noteoff 11 74 0
sleep 7.547
noteoff 12 55 0
sleep 7.547
noteon 10 79 102
sleep 1.886
noteoff 13 43 0
sleep 1.886
noteon 11 74 102
sleep 3.773
noteoff 14 31 0
sleep 3.773
noteon 12 55 102
sleep 9.433
noteon 13 43 104
sleep 5.66
noteon 14 31 106
sleep 67.924
noteoff 10 79 0
sleep 3.773
noteoff 11 74 0
sleep 7.547
noteoff 12 55 0
sleep 7.547
noteon 10 79 102
sleep 1.886
noteoff 13 43 0
sleep 1.886
noteon 11 74 102
sleep 3.773
noteoff 14 31 0
sleep 3.773
noteon 12 55 102
sleep 9.433
noteon 13 43 104
sleep 5.66
noteon 14 31 106
sleep 67.924
noteoff 10 79 0
sleep 3.773
noteoff 11 74 0
sleep 7.547
noteoff 12 55 0
sleep 7.547
noteon 10 79 102
sleep 1.886
noteoff 13 43 0
sleep 1.886
noteon 11 74 102
sleep 3.773
noteoff 14 31 0
sleep 3.773
noteon 12 55 102
sleep 9.433
noteon 13 43 104
sleep 5.66
noteon 14 31 106
sleep 67.924
noteoff 10 79 0
sleep 3.773
noteoff 11 74 0
sleep 7.547
noteoff 12 55 0
sleep 7.547
echo "177600 tempo_s=260 tempo_l=0.25"
noteon 10 74 102
sleep 1.923
noteoff 13 43 0
sleep 1.923
noteon 11 71 102
sleep 3.846
noteoff 14 31 0
sleep 3.846
noteon 12 55 102
sleep 9.615
noteon 13 43 104
sleep 5.769
noteon 14 31 106
sleep 69.23
noteoff 10 74 0
sleep 3.846
noteoff 11 71 0
sleep 7.692
noteoff 12 55 0
sleep 7.692
noteon 10 74 102
sleep 1.923
noteoff 13 43 0
sleep 1.923
noteon 11 71 102
sleep 3.846
noteoff 14 31 0
sleep 3.846
noteon 12 55 102
sleep 9.615
noteon 13 43 104
sleep 5.769
noteon 14 31 106
sleep 69.23
noteoff 10 74 0
sleep 3.846
noteoff 11 71 0
sleep 7.692
noteoff 12 55 0
sleep 7.692
noteon 10 74 102
sleep 1.923
noteoff 13 43 0
sleep 1.923
noteon 11 71 102
sleep 3.846
noteoff 14 31 0
sleep 3.846
noteon 12 55 102
sleep 9.615
noteon 13 43 104
sleep 5.769
noteon 14 31 106
sleep 69.23
noteoff 10 74 0
sleep 3.846
noteoff 11 71 0
sleep 7.692
noteoff 12 55 0
sleep 7.692
noteon 10 74 102
sleep 1.923
noteoff 13 43 0
sleep 1.923
noteon 11 71 102
sleep 3.846
noteoff 14 31 0
sleep 3.846
noteon 12 55 102
sleep 9.615
noteon 13 43 104
sleep 5.769
noteon 14 31 106
sleep 69.23
noteoff 10 74 0
sleep 3.846
noteoff 11 71 0
sleep 7.692
noteoff 12 55 0
sleep 7.692
noteon 10 74 102
sleep 1.923
noteoff 13 43 0
sleep 1.923
noteon 11 71 102
sleep 3.846
noteoff 14 31 0
sleep 3.846
noteon 12 55 102
sleep 9.615
noteon 13 43 104
sleep 5.769
noteon 14 31 106
sleep 69.23
noteoff 10 74 0
sleep 3.846
noteoff 11 71 0
sleep 7.692
noteoff 12 55 0
sleep 7.692
noteon 10 74 102
sleep 1.923
noteoff 13 43 0
sleep 1.923
noteon 11 71 102
sleep 3.846
noteoff 14 31 0
sleep 3.846
noteon 12 55 102
sleep 9.615
noteon 13 43 104
sleep 5.769
noteon 14 31 106
sleep 69.23
noteoff 10 74 0
sleep 3.846
noteoff 11 71 0
sleep 7.692
noteoff 12 55 0
sleep 7.692
noteon 10 74 102
sleep 1.923
noteoff 13 43 0
sleep 1.923
noteon 11 71 102
sleep 3.846
noteoff 14 31 0
sleep 3.846
noteon 12 55 102
sleep 9.615
noteon 13 43 104
sleep 5.769
noteon 14 31 106
sleep 69.23
noteoff 10 74 0
sleep 3.846
noteoff 11 71 0
sleep 7.692
noteoff 12 55 0
sleep 7.692
noteon 10 74 102
sleep 1.923
noteoff 13 43 0
sleep 1.923
noteon 11 71 102
sleep 3.846
noteoff 14 31 0
sleep 3.846
noteon 12 55 102
sleep 9.615
noteon 13 43 104
sleep 5.769
noteon 14 31 106
sleep 69.23
noteoff 10 74 0
sleep 3.846
noteoff 11 71 0
sleep 7.692
noteoff 12 55 0
sleep 7.692
echo "178080 tempo_s=247 tempo_l=0.25"
noteon 10 71 102
sleep 2.024
noteoff 13 43 0
sleep 2.024
noteon 11 67 102
sleep 4.048
noteoff 14 31 0
sleep 4.048
noteon 12 55 102
sleep 10.121
noteon 13 43 104
sleep 6.072
noteon 14 31 106
sleep 72.874
noteoff 10 71 0
sleep 4.048
noteoff 11 67 0
sleep 8.097
noteoff 12 55 0
sleep 8.097
noteon 10 71 102
sleep 2.024
noteoff 13 43 0
sleep 2.024
noteon 11 67 102
sleep 4.048
noteoff 14 31 0
sleep 4.048
noteon 12 55 102
sleep 10.121
noteon 13 43 104
sleep 6.072
noteon 14 31 106
sleep 72.874
noteoff 10 71 0
sleep 4.048
noteoff 11 67 0
sleep 8.097
noteoff 12 55 0
sleep 8.097
noteon 10 71 102
sleep 2.024
noteoff 13 43 0
sleep 2.024
noteon 11 67 102
sleep 4.048
noteoff 14 31 0
sleep 4.048
noteon 12 55 102
sleep 10.121
noteon 13 43 104
sleep 6.072
noteon 14 31 106
sleep 72.874
noteoff 10 71 0
sleep 4.048
noteoff 11 67 0
sleep 8.097
noteoff 12 55 0
sleep 8.097
noteon 10 71 102
sleep 2.024
noteoff 13 43 0
sleep 2.024
noteon 11 67 102
sleep 4.048
noteoff 14 31 0
sleep 4.048
noteon 12 55 102
sleep 10.121
noteon 13 43 104
sleep 6.072
noteon 14 31 106
sleep 72.874
noteoff 10 71 0
sleep 4.048
noteoff 11 67 0
sleep 8.097
noteoff 12 55 0
sleep 8.097
noteon 10 71 102
sleep 2.024
noteoff 13 43 0
sleep 2.024
noteon 11 67 102
sleep 4.048
noteoff 14 31 0
sleep 4.048
noteon 12 55 102
sleep 10.121
noteon 13 43 104
sleep 6.072
noteon 14 31 106
sleep 72.874
noteoff 10 71 0
sleep 4.048
noteoff 11 67 0
sleep 8.097
noteoff 12 55 0
sleep 8.097
noteon 10 71 102
sleep 2.024
noteoff 13 43 0
sleep 2.024
noteon 11 67 102
sleep 4.048
noteoff 14 31 0
sleep 4.048
noteon 12 55 102
sleep 10.121
noteon 13 43 104
sleep 6.072
noteon 14 31 106
sleep 72.874
noteoff 10 71 0
sleep 4.048
noteoff 11 67 0
sleep 8.097
noteoff 12 55 0
sleep 8.097
noteon 10 71 102
sleep 2.024
noteoff 13 43 0
sleep 2.024
noteon 11 67 102
sleep 4.048
noteoff 14 31 0
sleep 4.048
noteon 12 55 102
sleep 10.121
noteon 13 43 104
sleep 6.072
noteon 14 31 106
sleep 72.874
noteoff 10 71 0
sleep 4.048
noteoff 11 67 0
sleep 8.097
noteoff 12 55 0
sleep 8.097
noteon 10 71 102
sleep 2.024
noteoff 13 43 0
sleep 2.024
noteon 11 67 102
sleep 4.048
noteoff 14 31 0
sleep 4.048
noteon 12 55 102
sleep 10.121
noteon 13 43 104
sleep 6.072
noteon 14 31 106
sleep 32.388
echo "178530 tempo_s=107 tempo_l=0.25"
sleep 93.457
noteoff 10 71 0
sleep 9.344
noteoff 11 67 0
sleep 18.688
noteoff 12 55 0
sleep 18.688
echo "178560 tempo_s=278 tempo_l=0.25"
noteon 10 70 102
noteon 10 62 102
sleep 1.798
noteoff 13 43 0
noteon 0 89 101
noteon 0 86 101
sleep 1.798
noteon 1 70 100
noteon 4 62 100
noteon 11 58 102
noteon 11 65 102
noteon 1 77 100
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 1.798
noteoff 14 31 0
sleep 1.798
noteon 2 65 101
noteon 2 74 101
sleep 1.798
noteon 5 50 100
noteon 12 56 102
sleep 8.990
noteon 13 44 104
sleep 1.798
noteon 3 44 100
noteon 3 56 100
sleep 1.798
noteon 15 50 96
sleep 1.798
noteon 14 32 106
sleep 55.749
noteoff 15 50 0
sleep 14.386
noteon 15 50 94
sleep 57.547
noteoff 15 50 0
sleep 14.387
noteon 15 50 91
sleep 57.546
noteoff 15 50 0
sleep 14.387
noteon 15 50 88
sleep 57.547
noteoff 15 50 0
sleep 14.386
noteon 15 50 88
sleep 57.547
noteoff 15 50 0
sleep 14.387
noteon 15 50 88
sleep 48.555
echo "178800 tempo_s=310 tempo_l=0.25"
sleep 8.062
noteoff 15 50 0
sleep 12.902
noteon 15 50 88
sleep 51.612
noteoff 15 50 0
sleep 12.903
noteon 15 50 88
sleep 51.612
noteoff 15 50 0
sleep 12.903
noteon 15 50 88
sleep 51.612
noteoff 15 50 0
sleep 12.903
noteon 15 50 88
sleep 51.612
noteoff 15 50 0
sleep 12.903
noteon 15 50 88
sleep 51.612
noteoff 15 50 0
sleep 12.903
noteon 15 50 88
sleep 43.548
echo "179040 tempo_s=278 tempo_l=0.25"
sleep 8.992
noteoff 15 50 0
sleep 14.388
noteon 15 50 88
sleep 57.553
noteoff 15 50 0
sleep 14.388
noteon 15 50 88
sleep 57.553
noteoff 15 50 0
sleep 14.388
noteon 15 50 88
sleep 57.553
noteoff 15 50 0
sleep 14.388
noteon 15 50 88
sleep 57.553
noteoff 15 50 0
sleep 14.388
noteon 15 50 88
sleep 57.553
noteoff 15 50 0
sleep 14.388
noteon 15 50 88
sleep 48.561
echo "179280 tempo_s=310 tempo_l=0.25"
sleep 8.064
noteoff 15 50 0
sleep 12.903
noteon 15 50 88
sleep 51.612
noteoff 15 50 0
sleep 12.903
noteon 15 50 88
sleep 51.612
noteoff 15 50 0
sleep 12.903
noteon 15 50 88
sleep 51.612
noteoff 15 50 0
sleep 12.903
noteon 15 50 88
sleep 51.612
noteoff 15 50 0
sleep 12.903
noteon 15 50 88
sleep 51.612
noteoff 15 50 0
sleep 12.903
noteon 15 50 88
sleep 43.548
echo "179520 tempo_s=278 tempo_l=0.25"
noteoff 10 62 0
noteoff 10 70 0
noteon 10 69 102
sleep 1.798
noteoff 0 86 0
noteoff 0 89 0
noteon 0 86 101
noteon 0 90 101
sleep 1.798
noteoff 1 77 0
noteoff 1 70 0
noteoff 4 62 0
noteoff 11 65 0
noteoff 11 58 0
noteon 4 66 100
noteon 11 57 102
noteon 11 66 102
noteon 1 78 100
noteon 1 74 100
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteoff 2 74 0
noteoff 2 65 0
noteoff 15 50 0
noteon 2 74 101
noteon 2 66 101
sleep 1.798
noteoff 5 50 0
noteoff 12 56 0
noteon 5 62 100
noteon 12 57 102
sleep 8.992
noteoff 13 44 0
noteon 13 45 104
sleep 1.798
noteoff 3 56 0
noteoff 3 44 0
noteon 3 45 100
noteon 3 57 100
sleep 1.798
noteon 15 45 88
sleep 1.798
noteoff 14 32 0
noteon 14 33 106
sleep 68.337
noteoff 11 66 0
noteoff 11 57 0
sleep 7.194
noteoff 12 57 0
sleep 7.193
noteoff 10 69 0
noteon 10 68 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 74 102
noteon 11 66 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.394
noteon 14 33 106
sleep 68.337
noteoff 11 66 0
noteoff 11 74 0
sleep 7.194
noteoff 12 57 0
sleep 7.193
noteoff 10 68 0
noteon 10 69 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 74 102
noteon 11 66 102
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 1.798
noteoff 14 33 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 3.596
noteoff 15 45 0
sleep 1.798
noteon 14 33 106
sleep 68.337
noteoff 11 66 0
noteoff 11 74 0
sleep 7.194
noteoff 12 57 0
sleep 7.193
noteoff 10 69 0
noteon 10 68 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 74 102
noteon 11 66 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.394
noteon 14 33 106
sleep 68.337
noteoff 11 66 0
noteoff 11 74 0
sleep 7.194
noteoff 12 57 0
sleep 7.193
echo "179760 tempo_s=310 tempo_l=0.25"
noteoff 10 68 0
noteon 10 69 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 74 102
noteon 11 66 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 61.29
noteoff 11 66 0
noteoff 11 74 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteoff 10 69 0
noteon 10 68 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 74 102
noteon 11 66 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 61.29
noteoff 11 66 0
noteoff 11 74 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteoff 10 68 0
noteon 10 69 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 74 102
noteon 11 66 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 61.29
noteoff 11 66 0
noteoff 11 74 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteoff 10 69 0
noteon 10 68 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 74 102
noteon 11 66 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 58.064
noteoff 10 68 0
sleep 3.225
noteoff 11 66 0
noteoff 11 74 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
echo "180000 tempo_s=278 tempo_l=0.25"
noteon 10 69 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 66 102
noteon 11 74 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 68.345
noteoff 11 74 0
noteoff 11 66 0
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteon 11 66 102
noteon 11 74 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 64.748
noteoff 10 69 0
sleep 3.597
noteoff 11 74 0
noteoff 11 66 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
noteon 10 71 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 66 102
noteon 11 74 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 28.776
noteoff 10 71 0
sleep 39.568
noteoff 11 74 0
noteoff 11 66 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
noteon 10 73 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 66 102
noteon 11 74 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 28.776
noteoff 10 73 0
sleep 39.568
noteoff 11 74 0
noteoff 11 66 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
echo "180240 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 25.806
noteoff 10 74 0
sleep 35.483
noteoff 11 74 0
noteoff 11 66 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteon 10 76 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 25.806
noteoff 10 76 0
sleep 35.483
noteoff 11 74 0
noteoff 11 66 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteon 10 78 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 25.806
noteoff 10 78 0
sleep 35.483
noteoff 11 74 0
noteoff 11 66 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteon 10 79 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 25.806
noteoff 10 79 0
sleep 35.483
noteoff 11 74 0
noteoff 11 66 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
echo "180480 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
sleep 1.798
noteoff 0 90 0
noteoff 0 86 0
noteoff 13 45 0
noteon 0 81 101
noteon 0 93 101
sleep 1.798
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 66 0
noteon 4 66 100
noteon 11 66 102
noteon 11 74 102
noteon 1 78 100
noteon 1 74 100
sleep 1.798
noteon 6 74 108
noteon 6 62 108
sleep 1.798
noteoff 14 33 0
sleep 1.798
noteoff 2 66 0
noteoff 2 74 0
noteon 2 66 101
noteon 2 74 101
sleep 1.798
noteoff 5 62 0
noteon 5 62 100
noteon 12 69 102
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 57 0
noteoff 3 45 0
noteon 3 57 100
sleep 1.798
noteon 15 45 108
sleep 1.798
noteon 14 45 106
sleep 68.331
noteoff 11 74 0
noteoff 11 66 0
sleep 14.387
noteoff 10 81 0
noteon 10 80 102
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 11 66 102
noteon 11 74 102
sleep 19.780
noteon 15 45 104
sleep 70.129
noteoff 11 74 0
noteoff 11 66 0
sleep 14.386
noteoff 10 80 0
noteon 10 81 102
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 11 66 102
noteon 11 74 102
sleep 1.798
noteoff 6 62 0
noteoff 6 74 0
noteon 6 62 108
noteon 6 74 108
sleep 17.983
noteon 15 45 98
sleep 70.132
noteoff 11 74 0
noteoff 11 66 0
sleep 14.386
noteoff 10 81 0
noteon 10 80 102
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 11 66 102
noteon 11 74 102
sleep 19.781
noteon 15 45 94
sleep 70.135
noteoff 11 74 0
noteoff 11 66 0
sleep 14.387
echo "180720 tempo_s=310 tempo_l=0.25"
noteoff 10 80 0
noteon 10 81 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 16.127
noteon 15 45 90
sleep 62.903
noteoff 11 74 0
noteoff 11 66 0
sleep 12.903
noteoff 10 81 0
noteon 10 80 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 17.741
noteon 15 45 90
sleep 62.903
noteoff 11 74 0
noteoff 11 66 0
sleep 12.903
noteoff 10 80 0
noteon 10 81 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 16.129
noteon 15 45 90
sleep 62.903
noteoff 11 74 0
noteoff 11 66 0
sleep 12.903
noteoff 10 81 0
noteon 10 80 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 17.741
noteon 15 45 90
sleep 59.677
noteoff 10 80 0
sleep 3.225
noteoff 11 74 0
noteoff 11 66 0
sleep 12.903
echo "180960 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 11 66 102
noteon 11 74 102
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 17.985
noteon 15 45 90
sleep 70.143
noteoff 11 74 0
noteoff 11 66 0
sleep 16.187
noteoff 15 45 0
sleep 1.798
noteon 11 66 102
noteon 11 74 102
sleep 19.784
noteon 15 45 90
sleep 66.546
noteoff 10 81 0
sleep 3.597
noteoff 11 74 0
noteoff 11 66 0
sleep 7.194
noteoff 12 69 0
sleep 7.194
noteon 10 79 102
sleep 1.798
noteoff 0 93 0
noteoff 0 81 0
noteoff 15 45 0
noteoff 13 57 0
noteon 0 91 101
noteon 0 79 101
sleep 1.798
noteon 11 66 102
noteon 11 74 102
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
noteon 6 74 108
noteon 6 62 108
sleep 1.798
noteoff 14 45 0
sleep 3.597
noteon 12 67 102
sleep 8.992
noteon 13 55 104
sleep 1.798
noteoff 3 57 0
noteon 3 55 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 43 106
sleep 28.776
noteoff 10 79 0
sleep 10.791
noteoff 12 67 0
sleep 8.992
noteoff 13 55 0
sleep 5.395
noteoff 14 43 0
sleep 3.597
noteoff 0 79 0
noteoff 0 91 0
sleep 10.791
noteoff 11 74 0
noteoff 11 66 0
sleep 8.992
noteoff 3 55 0
sleep 5.395
noteon 10 78 102
sleep 1.798
noteoff 15 45 0
noteon 0 90 101
noteon 0 78 101
sleep 1.798
noteon 11 66 102
noteon 11 74 102
sleep 7.194
noteon 12 66 102
sleep 8.992
noteon 13 54 104
sleep 1.798
noteon 3 54 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 42 106
sleep 28.776
noteoff 10 78 0
sleep 10.791
noteoff 12 66 0
sleep 8.992
noteoff 13 54 0
sleep 5.395
noteoff 14 42 0
sleep 3.597
noteoff 0 78 0
noteoff 0 90 0
sleep 10.791
noteoff 11 74 0
noteoff 11 66 0
sleep 8.992
noteoff 3 54 0
sleep 5.395
echo "181200 tempo_s=310 tempo_l=0.25"
noteon 10 76 102
sleep 1.612
noteoff 15 45 0
noteon 0 88 101
noteon 0 76 101
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 1.612
noteoff 6 62 0
noteoff 6 74 0
noteon 6 74 108
noteon 6 62 108
sleep 4.838
noteon 12 64 102
sleep 8.064
noteon 13 52 104
sleep 1.612
noteon 3 52 100
sleep 1.612
noteon 15 45 90
sleep 1.612
noteon 14 40 106
sleep 25.806
noteoff 10 76 0
sleep 9.677
noteoff 12 64 0
sleep 8.064
noteoff 13 52 0
sleep 4.838
noteoff 14 40 0
sleep 3.225
noteoff 0 76 0
noteoff 0 88 0
sleep 9.677
noteoff 11 74 0
noteoff 11 66 0
sleep 8.064
noteoff 3 52 0
sleep 4.838
noteon 10 74 102
sleep 1.612
noteoff 15 45 0
noteon 0 86 101
noteon 0 74 101
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 6.451
noteon 12 62 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 1.612
noteon 15 45 90
sleep 1.612
noteon 14 38 106
sleep 25.806
noteoff 10 74 0
sleep 9.677
noteoff 12 62 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 3.225
noteoff 0 74 0
noteoff 0 86 0
sleep 9.677
noteoff 11 74 0
noteoff 11 66 0
sleep 8.064
noteoff 3 50 0
sleep 4.838
noteon 10 73 102
sleep 1.612
noteoff 15 45 0
noteon 0 85 101
noteon 0 73 101
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 1.612
noteoff 6 62 0
noteoff 6 74 0
noteon 6 74 108
noteon 6 62 108
sleep 4.838
noteon 12 61 102
sleep 8.064
noteon 13 49 104
sleep 1.612
noteon 3 49 100
sleep 1.612
noteon 15 45 90
sleep 1.612
noteon 14 37 106
sleep 25.806
noteoff 10 73 0
sleep 9.677
noteoff 12 61 0
sleep 8.064
noteoff 13 49 0
sleep 4.838
noteoff 14 37 0
sleep 3.225
noteoff 0 73 0
noteoff 0 85 0
sleep 9.677
noteoff 11 74 0
noteoff 11 66 0
sleep 8.064
noteoff 3 49 0
sleep 4.838
noteon 10 71 102
sleep 1.612
noteoff 15 45 0
noteon 0 83 101
noteon 0 71 101
sleep 1.612
noteon 11 66 102
noteon 11 74 102
sleep 6.451
noteon 12 59 102
sleep 8.064
noteon 13 47 104
sleep 1.612
noteon 3 47 100
sleep 1.612
noteon 15 45 90
sleep 1.612
noteon 14 35 106
sleep 25.806
noteoff 10 71 0
sleep 9.677
noteoff 12 59 0
sleep 8.064
noteoff 13 47 0
sleep 4.838
noteoff 14 35 0
sleep 3.225
noteoff 0 71 0
noteoff 0 83 0
sleep 9.677
noteoff 11 74 0
noteoff 11 66 0
sleep 8.064
noteoff 3 47 0
sleep 4.838
echo "181440 tempo_s=278 tempo_l=0.25"
noteon 10 69 102
sleep 1.798
noteoff 15 45 0
noteon 0 81 101
noteon 0 69 101
sleep 1.798
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 66 0
noteon 1 79 100
noteon 11 67 102
noteon 11 73 102
noteon 1 73 100
noteon 4 64 100
sleep 1.798
noteoff 6 62 0
noteoff 6 74 0
noteon 6 69 108
noteon 6 57 108
sleep 3.597
noteoff 2 74 0
noteoff 2 66 0
noteon 2 67 101
noteon 2 73 101
sleep 1.798
noteoff 5 62 0
noteon 12 57 102
noteon 5 57 100
sleep 8.992
noteon 13 45 104
sleep 1.798
noteon 3 45 100
noteon 3 57 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 33 106
sleep 68.336
noteoff 11 73 0
noteoff 11 67 0
sleep 7.193
noteoff 12 57 0
sleep 7.193
noteoff 10 69 0
noteon 10 68 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 73 102
noteon 11 67 102
sleep 3.596
noteoff 14 33 0
sleep 3.596
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 68.337
noteoff 11 67 0
noteoff 11 73 0
sleep 7.193
noteoff 12 57 0
sleep 7.193
noteoff 10 68 0
noteon 10 69 102
sleep 1.798
noteoff 0 69 0
noteoff 0 81 0
noteoff 13 45 0
sleep 1.798
noteon 11 67 102
noteon 11 73 102
sleep 1.798
noteoff 6 57 0
noteoff 6 69 0
sleep 1.798
noteoff 14 33 0
sleep 3.596
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 3.596
noteoff 15 45 0
sleep 1.798
noteon 14 33 106
sleep 68.336
noteoff 11 73 0
noteoff 11 67 0
sleep 7.193
noteoff 12 57 0
sleep 7.193
noteoff 10 69 0
noteon 10 68 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 73 102
noteon 11 67 102
sleep 3.597
noteoff 14 33 0
sleep 3.596
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 68.340
noteoff 11 67 0
noteoff 11 73 0
sleep 7.193
noteoff 12 57 0
sleep 7.194
echo "181680 tempo_s=310 tempo_l=0.25"
noteoff 10 68 0
noteon 10 69 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 67 102
noteon 11 73 102
sleep 3.225
noteoff 14 33 0
sleep 3.224
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.837
noteon 14 33 106
sleep 61.29
noteoff 11 73 0
noteoff 11 67 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteoff 10 69 0
noteon 10 68 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 67 102
noteon 11 73 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 61.29
noteoff 11 73 0
noteoff 11 67 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteoff 10 68 0
noteon 10 69 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 67 102
noteon 11 73 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 61.29
noteoff 11 73 0
noteoff 11 67 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteoff 10 69 0
noteon 10 68 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 73 102
noteon 11 67 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 58.064
noteoff 10 68 0
sleep 3.225
noteoff 11 67 0
noteoff 11 73 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
echo "181920 tempo_s=278 tempo_l=0.25"
noteon 10 69 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 67 102
noteon 11 73 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 68.345
noteoff 11 73 0
noteoff 11 67 0
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteon 11 73 102
noteon 11 67 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 64.748
noteoff 10 69 0
sleep 3.597
noteoff 11 67 0
noteoff 11 73 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
noteon 10 71 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 73 102
noteon 11 67 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 28.776
noteoff 10 71 0
sleep 39.568
noteoff 11 67 0
noteoff 11 73 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
noteon 10 73 102
sleep 1.798
noteoff 13 45 0
sleep 1.798
noteon 11 73 102
noteon 11 67 102
sleep 3.597
noteoff 14 33 0
sleep 3.597
noteon 12 57 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 28.776
noteoff 10 73 0
sleep 39.568
noteoff 11 67 0
noteoff 11 73 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
echo "182160 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 73 102
noteon 11 67 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 25.806
noteoff 10 74 0
sleep 35.483
noteoff 11 67 0
noteoff 11 73 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteon 10 76 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 73 102
noteon 11 67 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 25.806
noteoff 10 76 0
sleep 35.483
noteoff 11 67 0
noteoff 11 73 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteon 10 78 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 73 102
noteon 11 67 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 25.806
noteoff 10 78 0
sleep 35.483
noteoff 11 67 0
noteoff 11 73 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
noteon 10 79 102
sleep 1.612
noteoff 13 45 0
sleep 1.612
noteon 11 67 102
noteon 11 73 102
sleep 3.225
noteoff 14 33 0
sleep 3.225
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 4.838
noteon 14 33 106
sleep 25.806
noteoff 10 79 0
sleep 35.483
noteoff 11 73 0
noteoff 11 67 0
sleep 6.451
noteoff 12 57 0
sleep 6.451
echo "182400 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
sleep 1.798
noteoff 13 45 0
noteon 0 81 101
noteon 0 93 101
sleep 1.798
noteoff 1 73 0
noteoff 1 79 0
noteoff 4 64 0
noteon 1 79 100
noteon 11 67 102
noteon 11 73 102
noteon 1 73 100
noteon 4 64 100
sleep 1.798
noteon 6 57 108
noteon 6 69 108
sleep 1.798
noteoff 14 33 0
sleep 1.798
noteoff 2 73 0
noteoff 2 67 0
noteon 2 73 101
noteon 2 67 101
sleep 1.798
noteoff 5 57 0
noteon 12 69 102
noteon 5 57 100
sleep 8.992
noteon 13 57 104
sleep 1.798
noteoff 3 57 0
noteoff 3 45 0
noteon 3 57 100
sleep 1.798
noteon 15 45 108
sleep 1.798
noteon 14 45 106
sleep 68.331
noteoff 11 73 0
noteoff 11 67 0
sleep 14.387
noteoff 10 81 0
noteon 10 80 102
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 11 73 102
noteon 11 67 102
sleep 19.780
noteon 15 45 104
sleep 70.129
noteoff 11 67 0
noteoff 11 73 0
sleep 14.386
noteoff 10 80 0
noteon 10 81 102
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 11 73 102
noteon 11 67 102
sleep 1.798
noteoff 6 69 0
noteoff 6 57 0
noteon 6 69 108
noteon 6 57 108
sleep 17.983
noteon 15 45 98
sleep 70.132
noteoff 11 67 0
noteoff 11 73 0
sleep 14.386
noteoff 10 81 0
noteon 10 80 102
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 11 73 102
noteon 11 67 102
sleep 19.781
noteon 15 45 94
sleep 70.135
noteoff 11 67 0
noteoff 11 73 0
sleep 14.387
echo "182640 tempo_s=310 tempo_l=0.25"
noteoff 10 80 0
noteon 10 81 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 73 102
noteon 11 67 102
sleep 1.612
noteoff 6 57 0
noteoff 6 69 0
noteon 6 69 108
noteon 6 57 108
sleep 16.127
noteon 15 45 90
sleep 62.903
noteoff 11 67 0
noteoff 11 73 0
sleep 12.903
noteoff 10 81 0
noteon 10 80 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 73 102
noteon 11 67 102
sleep 17.741
noteon 15 45 90
sleep 62.903
noteoff 11 67 0
noteoff 11 73 0
sleep 12.903
noteoff 10 80 0
noteon 10 81 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 73 102
noteon 11 67 102
sleep 1.612
noteoff 6 57 0
noteoff 6 69 0
noteon 6 69 108
noteon 6 57 108
sleep 16.129
noteon 15 45 90
sleep 62.903
noteoff 11 67 0
noteoff 11 73 0
sleep 12.903
noteoff 10 81 0
noteon 10 80 102
sleep 1.612
noteoff 15 45 0
sleep 1.612
noteon 11 67 102
noteon 11 73 102
sleep 17.741
noteon 15 45 90
sleep 59.677
noteoff 10 80 0
sleep 3.225
noteoff 11 73 0
noteoff 11 67 0
sleep 12.903
echo "182880 tempo_s=278 tempo_l=0.25"
noteon 10 81 102
sleep 1.798
noteoff 15 45 0
sleep 1.798
noteon 11 81 102
sleep 1.798
noteoff 6 57 0
noteoff 6 69 0
noteon 6 69 108
noteon 6 57 108
sleep 17.985
noteon 15 45 90
sleep 30.575
noteoff 10 81 0
sleep 3.597
noteoff 11 81 0
sleep 43.165
noteoff 12 69 0
sleep 7.194
noteon 10 79 102
sleep 1.798
noteoff 0 93 0
noteoff 0 81 0
noteoff 15 45 0
noteoff 13 57 0
noteon 0 79 101
noteon 0 91 101
sleep 1.798
noteon 11 79 102
sleep 3.597
noteoff 14 45 0
sleep 3.597
noteon 12 67 102
sleep 8.992
noteon 13 55 104
sleep 1.798
noteoff 3 57 0
noteon 3 55 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 43 106
sleep 28.776
noteoff 10 79 0
sleep 3.597
noteoff 11 79 0
sleep 7.194
noteoff 12 67 0
sleep 8.992
noteoff 13 55 0
sleep 5.395
noteoff 14 43 0
sleep 3.597
noteoff 0 91 0
noteoff 0 79 0
sleep 19.784
noteoff 3 55 0
sleep 5.395
noteon 10 78 102
sleep 1.798
noteoff 15 45 0
noteon 0 78 101
noteon 0 90 101
sleep 1.798
noteon 11 78 102
sleep 1.798
noteoff 6 57 0
noteoff 6 69 0
noteon 6 69 108
noteon 6 57 108
sleep 5.395
noteon 12 66 102
sleep 8.992
noteon 13 54 104
sleep 1.798
noteon 3 54 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 42 106
sleep 28.776
noteoff 10 78 0
sleep 3.597
noteoff 11 78 0
sleep 7.194
noteoff 12 66 0
sleep 8.992
noteoff 13 54 0
sleep 5.395
noteoff 14 42 0
sleep 3.597
noteoff 0 90 0
noteoff 0 78 0
sleep 19.784
noteoff 3 54 0
sleep 5.395
noteon 10 76 102
sleep 1.798
noteoff 15 45 0
noteon 0 76 101
noteon 0 88 101
sleep 1.798
noteon 11 76 102
sleep 7.194
noteon 12 64 102
sleep 8.992
noteon 13 52 104
sleep 1.798
noteon 3 52 100
sleep 1.798
noteon 15 45 90
sleep 1.798
noteon 14 40 106
sleep 28.776
noteoff 10 76 0
sleep 3.597
noteoff 11 76 0
sleep 7.194
noteoff 12 64 0
sleep 8.992
noteoff 13 52 0
sleep 5.395
noteoff 14 40 0
sleep 3.597
noteoff 0 88 0
noteoff 0 76 0
sleep 19.784
noteoff 3 52 0
sleep 5.395
echo "183120 tempo_s=310 tempo_l=0.25"
noteon 10 74 102
sleep 1.612
noteoff 15 45 0
noteon 0 74 101
noteon 0 86 101
sleep 1.612
noteon 11 74 102
sleep 1.612
noteoff 6 57 0
noteoff 6 69 0
noteon 6 57 108
noteon 6 69 108
sleep 4.838
noteon 12 62 102
sleep 8.064
noteon 13 50 104
sleep 1.612
noteon 3 50 100
sleep 1.612
noteon 15 45 90
sleep 1.612
noteon 14 38 106
sleep 25.806
noteoff 10 74 0
sleep 3.225
noteoff 11 74 0
sleep 6.451
noteoff 12 62 0
sleep 8.064
noteoff 13 50 0
sleep 4.838
noteoff 14 38 0
sleep 3.225
noteoff 0 86 0
noteoff 0 74 0
sleep 17.741
noteoff 3 50 0
sleep 4.838
noteon 10 73 102
sleep 1.612
noteoff 15 45 0
noteon 0 73 101
noteon 0 85 101
sleep 1.612
noteon 11 73 102
sleep 6.451
noteon 12 61 102
sleep 8.064
noteon 13 49 104
sleep 1.612
noteon 3 49 100
sleep 1.612
noteon 15 45 90
sleep 1.612
noteon 14 37 106
sleep 25.806
noteoff 10 73 0
sleep 3.225
noteoff 11 73 0
sleep 6.451
noteoff 12 61 0
sleep 8.064
noteoff 13 49 0
sleep 4.838
noteoff 14 37 0
sleep 3.225
noteoff 0 85 0
noteoff 0 73 0
sleep 17.741
noteoff 3 49 0
sleep 4.838
noteon 10 71 102
sleep 1.612
noteoff 15 45 0
noteon 0 71 101
noteon 0 83 101
sleep 1.612
noteon 11 71 102
sleep 1.612
noteoff 6 69 0
noteoff 6 57 0
noteon 6 57 108
noteon 6 69 108
sleep 4.838
noteon 12 59 102
sleep 8.064
noteon 13 47 104
sleep 1.612
noteon 3 47 100
sleep 1.612
noteon 15 45 90
sleep 1.612
noteon 14 35 106
sleep 25.806
noteoff 10 71 0
sleep 3.225
noteoff 11 71 0
sleep 6.451
noteoff 12 59 0
sleep 8.064
noteoff 13 47 0
sleep 4.838
noteoff 14 35 0
sleep 3.225
noteoff 0 83 0
noteoff 0 71 0
sleep 17.741
noteoff 3 47 0
sleep 4.838
noteon 10 69 102
sleep 1.612
noteoff 15 45 0
noteon 0 81 101
noteon 0 69 101
sleep 1.612
noteon 11 69 102
sleep 6.451
noteon 12 57 102
sleep 8.064
noteon 13 45 104
sleep 1.612
noteon 3 45 100
sleep 1.612
noteon 15 45 90
sleep 1.612
noteon 14 33 106
sleep 25.806
noteoff 10 69 0
sleep 3.225
noteoff 11 69 0
sleep 6.451
noteoff 12 57 0
sleep 8.064
noteoff 13 45 0
sleep 4.838
noteoff 14 33 0
sleep 3.225
noteoff 0 69 0
noteoff 0 81 0
sleep 17.741
noteoff 3 45 0
sleep 4.838
echo "183360 tempo_s=305 tempo_l=0.25"
noteon 10 74 102
sleep 1.639
noteoff 15 45 0
noteon 0 86 101
noteon 0 74 101
sleep 1.639
noteoff 1 73 0
noteoff 1 79 0
noteoff 4 64 0
noteon 1 78 100
noteon 1 74 100
noteon 4 62 100
noteon 11 74 102
sleep 1.639
noteoff 6 69 0
noteoff 6 57 0
noteon 6 74 108
noteon 6 62 108
sleep 3.278
noteoff 2 67 0
noteoff 2 73 0
noteon 2 66 101
noteon 2 74 101
sleep 1.639
noteoff 5 57 0
noteon 5 50 100
noteon 12 62 102
sleep 8.196
noteon 13 50 104
sleep 1.639
noteon 3 50 100
sleep 1.639
noteon 15 50 90
sleep 1.639
noteon 14 38 106
sleep 173.77
echo "183480 tempo_s=277 tempo_l=0.25"
noteoff 10 74 0
sleep 1.805
noteoff 0 74 0
noteoff 0 86 0
sleep 1.805
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 62 0
noteoff 11 74 0
sleep 1.805
noteoff 6 62 0
noteoff 6 74 0
sleep 3.61
noteoff 2 74 0
noteoff 2 66 0
sleep 1.805
noteoff 5 50 0
noteoff 12 62 0
sleep 9.025
noteoff 13 50 0
sleep 1.805
noteoff 3 50 0
sleep 1.805
noteoff 15 50 0
sleep 1.805
noteoff 14 38 0
sleep 83.032
noteon 10 85 102
sleep 108.302
echo "183600 tempo_s=310 tempo_l=0.25"
noteoff 10 85 0
noteon 10 86 102
sleep 3.225
noteon 4 62 100
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 4.838
noteon 5 50 100
sleep 11.288
noteon 15 50 45
sleep 172.573
noteoff 10 86 0
sleep 3.225
noteoff 4 62 0
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 4.838
noteoff 5 50 0
sleep 11.29
noteoff 15 50 0
sleep 77.414
noteon 0 78 101
sleep 1.612
noteon 1 78 100
noteon 1 66 100
noteon 11 66 102
sleep 4.838
noteon 2 66 101
sleep 1.612
noteon 12 66 102
sleep 8.064
noteon 13 54 104
sleep 1.612
noteon 3 54 100
sleep 3.225
noteon 14 42 106
sleep 74.188
echo "183840 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 0 78 0
noteon 0 79 101
sleep 1.798
noteoff 1 66 0
noteoff 1 78 0
noteoff 11 66 0
noteon 1 67 100
noteon 11 67 102
noteon 1 79 100
sleep 5.395
noteoff 2 66 0
noteon 2 67 101
sleep 1.798
noteoff 12 66 0
noteon 12 67 102
sleep 8.992
noteoff 13 54 0
noteon 13 55 104
sleep 1.798
noteoff 3 54 0
noteon 3 55 100
sleep 3.597
noteoff 14 42 0
noteon 14 43 106
sleep 192.437
noteoff 0 79 0
sleep 1.798
noteoff 1 79 0
noteoff 1 67 0
noteoff 11 67 0
sleep 5.395
noteoff 2 67 0
sleep 1.798
noteoff 12 67 0
sleep 8.992
noteoff 13 55 0
sleep 1.798
noteoff 3 55 0
sleep 3.597
noteoff 14 43 0
sleep 82.732
noteon 10 85 102
sleep 107.907
echo "184080 tempo_s=310 tempo_l=0.25"
noteoff 10 85 0
noteon 10 86 102
sleep 3.224
noteon 4 62 100
sleep 1.612
noteon 6 74 108
noteon 6 62 108
sleep 4.837
noteon 5 50 100
sleep 11.289
noteon 15 50 57
sleep 172.569
noteoff 10 86 0
sleep 3.224
noteoff 4 62 0
sleep 1.612
noteoff 6 62 0
noteoff 6 74 0
sleep 4.837
noteoff 5 50 0
sleep 11.287
noteoff 15 50 0
sleep 77.415
noteon 0 80 101
sleep 1.612
noteon 1 80 100
noteon 1 68 100
noteon 11 68 102
sleep 4.838
noteon 2 68 101
sleep 1.612
noteon 12 68 102
sleep 8.064
noteon 13 56 104
sleep 1.612
noteon 3 56 100
sleep 3.225
noteon 14 44 106
sleep 74.188
echo "184320 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 0 80 0
noteon 0 81 101
sleep 1.798
noteoff 1 68 0
noteoff 1 80 0
noteoff 11 68 0
noteon 1 81 100
noteon 11 69 102
noteon 1 69 100
sleep 5.395
noteoff 2 68 0
noteon 2 69 101
sleep 1.798
noteoff 12 68 0
noteon 12 69 102
sleep 8.992
noteoff 13 56 0
noteon 13 57 104
sleep 1.798
noteoff 3 56 0
noteon 3 57 100
sleep 3.597
noteoff 14 44 0
noteon 14 45 106
sleep 192.437
noteoff 0 81 0
sleep 1.798
noteoff 1 69 0
noteoff 1 81 0
noteoff 11 69 0
sleep 5.395
noteoff 2 69 0
sleep 1.798
noteoff 12 69 0
sleep 8.992
noteoff 13 57 0
sleep 1.798
noteoff 3 57 0
sleep 3.597
noteoff 14 45 0
sleep 82.730
noteon 10 85 102
sleep 107.906
echo "184560 tempo_s=310 tempo_l=0.25"
noteoff 10 85 0
noteon 10 86 102
sleep 3.224
noteon 4 62 100
sleep 1.612
noteon 6 62 108
noteon 6 74 108
sleep 4.837
noteon 5 50 100
sleep 11.289
noteon 15 50 70
sleep 172.567
noteoff 10 86 0
sleep 3.224
noteoff 4 62 0
sleep 1.612
noteoff 6 74 0
noteoff 6 62 0
sleep 4.837
noteoff 5 50 0
sleep 11.289
noteoff 15 50 0
sleep 77.416
noteon 0 82 101
sleep 1.612
noteon 1 70 100
noteon 1 82 100
noteon 11 70 102
sleep 4.837
noteon 2 70 101
sleep 1.612
noteon 12 70 102
sleep 8.063
noteon 13 58 104
sleep 1.612
noteon 3 58 100
sleep 3.225
noteon 14 46 106
sleep 74.191
echo "184800 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 0 82 0
noteon 0 83 101
sleep 1.798
noteoff 1 82 0
noteoff 1 70 0
noteoff 11 70 0
noteon 11 71 102
noteon 1 71 100
noteon 1 83 100
sleep 5.395
noteoff 2 70 0
noteon 2 71 101
sleep 1.798
noteoff 12 70 0
noteon 12 71 102
sleep 8.992
noteoff 13 58 0
noteon 13 59 104
sleep 1.798
noteoff 3 58 0
noteon 3 59 100
sleep 3.597
noteoff 14 46 0
noteon 14 47 106
sleep 192.439
noteoff 0 83 0
sleep 1.798
noteoff 1 83 0
noteoff 1 71 0
noteoff 11 71 0
sleep 5.395
noteoff 2 71 0
sleep 1.798
noteoff 12 71 0
sleep 8.992
noteoff 13 59 0
sleep 1.798
noteoff 3 59 0
sleep 3.597
noteoff 14 47 0
sleep 82.730
noteon 10 85 102
sleep 107.907
echo "185040 tempo_s=310 tempo_l=0.25"
noteoff 10 85 0
noteon 10 86 102
sleep 3.224
noteon 4 62 100
sleep 1.612
noteon 6 74 108
noteon 6 62 108
sleep 4.837
noteon 5 50 100
sleep 11.289
noteon 15 50 82
sleep 172.567
noteoff 10 86 0
sleep 3.224
noteoff 4 62 0
sleep 1.612
noteoff 6 62 0
noteoff 6 74 0
sleep 4.837
noteoff 5 50 0
sleep 11.289
noteoff 15 50 0
sleep 77.414
noteon 0 83 101
sleep 1.612
noteon 1 83 100
noteon 11 71 102
noteon 1 71 100
sleep 4.837
noteon 2 71 101
sleep 1.612
noteon 12 71 102
sleep 8.064
noteon 13 59 104
sleep 1.612
noteon 3 59 100
sleep 3.225
noteon 14 47 106
sleep 74.193
echo "185280 tempo_s=292 tempo_l=0.25"
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 0 83 0
noteon 0 84 101
sleep 1.712
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
noteon 4 62 100
noteon 1 84 100
noteon 1 72 100
noteon 11 72 102
sleep 1.712
noteon 6 62 108
noteon 6 74 108
sleep 3.424
noteoff 2 71 0
noteon 2 72 101
sleep 1.712
noteoff 12 71 0
noteon 5 50 100
noteon 12 72 102
sleep 8.561
noteoff 13 59 0
noteon 13 60 104
sleep 1.712
noteoff 3 59 0
noteon 3 60 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteoff 14 47 0
noteon 14 48 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 84 0
sleep 1.712
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 5.136
noteoff 2 72 0
sleep 1.712
noteoff 12 72 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.712
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.424
noteoff 14 48 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 81 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 69 100
noteon 1 81 100
noteon 4 62 100
noteon 11 69 102
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 69 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 1.712
noteon 3 57 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 45 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 81 0
sleep 1.712
noteoff 1 81 0
noteoff 1 69 0
noteoff 11 69 0
sleep 5.136
noteoff 2 69 0
sleep 1.712
noteoff 12 69 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 57 0
noteon 0 83 101
sleep 1.712
noteoff 3 57 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.424
noteoff 14 45 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
echo "185520 tempo_s=314 tempo_l=0.25"
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
echo "185760 tempo_s=292 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 74 108
noteon 6 62 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 72 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 8.561
noteon 13 60 104
sleep 1.712
noteon 3 60 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 48 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 84 0
sleep 1.712
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 5.136
noteoff 2 72 0
sleep 1.712
noteoff 12 72 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.712
noteoff 3 60 0
noteon 1 71 100
noteon 1 83 100
noteon 11 71 102
sleep 3.424
noteoff 14 48 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 83 0
noteoff 1 71 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 81 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 81 100
noteon 1 69 100
noteon 4 62 100
noteon 11 69 102
sleep 1.712
noteoff 6 62 0
noteoff 6 74 0
noteon 6 74 108
noteon 6 62 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 69 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 1.712
noteon 3 57 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 45 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 81 0
sleep 1.712
noteoff 1 69 0
noteoff 1 81 0
noteoff 11 69 0
sleep 5.136
noteoff 2 69 0
sleep 1.712
noteoff 12 69 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 57 0
noteon 0 83 101
sleep 1.712
noteoff 3 57 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.424
noteoff 14 45 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
echo "186000 tempo_s=314 tempo_l=0.25"
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 72 100
noteon 1 84 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 62 0
noteoff 6 74 0
noteon 6 74 108
noteon 6 62 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 84 0
noteoff 1 72 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 62 0
noteoff 6 74 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
echo "186240 tempo_s=292 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 72 100
noteon 1 84 100
noteon 4 62 100
noteon 11 72 102
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 72 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 8.561
noteon 13 60 104
sleep 1.712
noteon 3 60 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 48 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 84 0
sleep 1.712
noteoff 1 84 0
noteoff 1 72 0
noteoff 11 72 0
sleep 5.136
noteoff 2 72 0
sleep 1.712
noteoff 12 72 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.712
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.424
noteoff 14 48 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 72 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 8.561
noteon 13 60 104
sleep 1.712
noteon 3 60 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 48 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 84 0
sleep 1.712
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 5.136
noteoff 2 72 0
sleep 1.712
noteoff 12 72 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.712
noteoff 3 60 0
noteon 1 71 100
noteon 1 83 100
noteon 11 71 102
sleep 3.424
noteoff 14 48 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 83 0
noteoff 1 71 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
echo "186480 tempo_s=314 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 72 100
noteon 1 84 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 84 0
noteoff 1 72 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
echo "186720 tempo_s=292 tempo_l=0.25"
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 72 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 8.561
noteon 13 60 104
sleep 1.712
noteon 3 60 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 48 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 84 0
sleep 1.712
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 5.136
noteoff 2 72 0
sleep 1.712
noteoff 12 72 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.712
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.424
noteoff 14 48 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 81 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 81 100
noteon 1 69 100
noteon 4 62 100
noteon 11 69 102
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 69 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 1.712
noteon 3 57 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 45 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 81 0
sleep 1.712
noteoff 1 69 0
noteoff 1 81 0
noteoff 11 69 0
sleep 5.136
noteoff 2 69 0
sleep 1.712
noteoff 12 69 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 57 0
noteon 0 79 101
sleep 1.712
noteoff 3 57 0
noteon 1 79 100
noteon 1 67 100
noteon 11 67 102
sleep 3.424
noteoff 14 45 0
sleep 1.712
noteon 2 67 101
sleep 1.712
noteon 12 67 102
sleep 8.561
noteon 13 55 104
sleep 1.712
noteon 3 55 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 43 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 79 0
sleep 1.712
noteoff 1 67 0
noteoff 1 79 0
noteoff 11 67 0
sleep 5.136
noteoff 2 67 0
sleep 1.712
noteoff 12 67 0
sleep 6.849
echo "186960 tempo_s=314 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 55 0
noteon 0 78 101
sleep 1.592
noteoff 3 55 0
noteoff 4 62 0
noteon 1 66 100
noteon 1 78 100
noteon 4 62 100
noteon 11 66 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 43 0
sleep 1.592
noteon 2 66 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 66 102
sleep 7.961
noteon 13 54 104
sleep 1.592
noteon 3 54 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 42 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 78 0
sleep 1.592
noteoff 1 78 0
noteoff 1 66 0
noteoff 11 66 0
sleep 4.777
noteoff 2 66 0
sleep 1.592
noteoff 12 66 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 54 0
noteon 0 76 101
sleep 1.592
noteoff 3 54 0
noteon 1 76 100
noteon 1 64 100
noteon 11 64 102
sleep 3.184
noteoff 14 42 0
sleep 1.592
noteon 2 64 101
sleep 1.592
noteon 12 64 102
sleep 7.961
noteon 13 52 104
sleep 1.592
noteon 3 52 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 40 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 76 0
sleep 1.592
noteoff 1 64 0
noteoff 1 76 0
noteoff 11 64 0
sleep 4.777
noteoff 2 64 0
sleep 1.592
noteoff 12 64 0
sleep 6.369
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 52 0
noteon 0 74 101
sleep 1.592
noteoff 3 52 0
noteoff 4 62 0
noteon 1 74 100
noteon 1 62 100
noteon 4 62 100
noteon 11 62 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 40 0
sleep 1.592
noteon 2 62 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 62 102
sleep 7.961
noteon 13 50 104
sleep 1.592
noteon 3 50 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 38 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 74 0
sleep 1.592
noteoff 1 62 0
noteoff 1 74 0
noteoff 11 62 0
sleep 4.777
noteoff 2 62 0
sleep 1.592
noteoff 12 62 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 50 0
noteon 0 72 101
sleep 1.592
noteoff 3 50 0
noteon 1 60 100
noteon 1 72 100
noteon 11 60 102
sleep 3.184
noteoff 14 38 0
sleep 1.592
noteon 2 60 101
sleep 1.592
noteon 12 60 102
sleep 7.961
noteon 13 48 104
sleep 1.592
noteon 3 48 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 36 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 72 0
sleep 1.592
noteoff 1 72 0
noteoff 1 60 0
noteoff 11 60 0
sleep 4.777
noteoff 2 60 0
sleep 1.592
noteoff 12 60 0
sleep 6.369
echo "187200 tempo_s=292 tempo_l=0.25"
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 48 0
noteon 0 71 101
sleep 1.712
noteoff 3 48 0
noteoff 4 62 0
noteon 1 74 100
noteon 1 86 100
noteon 11 59 102
noteon 4 62 100
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 74 108
noteon 6 62 108
sleep 1.712
noteoff 14 36 0
sleep 1.712
noteon 2 59 101
sleep 1.712
noteoff 5 50 0
noteon 12 59 102
noteon 5 50 100
sleep 8.560
noteon 13 47 104
sleep 1.712
noteon 3 47 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 35 106
sleep 44.512
noteoff 10 86 0
sleep 20.544
noteoff 11 59 0
sleep 6.848
noteoff 12 59 0
sleep 6.848
noteon 10 85 102
sleep 1.712
noteoff 13 47 0
sleep 1.712
noteon 11 59 102
sleep 3.424
noteoff 14 35 0
sleep 3.424
noteon 12 59 102
sleep 8.560
noteon 13 47 104
sleep 5.136
noteon 14 35 106
sleep 44.512
noteoff 10 85 0
sleep 20.544
noteoff 11 59 0
sleep 6.848
noteoff 12 59 0
sleep 6.848
noteon 10 86 102
sleep 1.712
noteoff 0 71 0
noteoff 13 47 0
sleep 1.712
noteon 11 62 102
sleep 3.424
noteoff 14 35 0
sleep 1.712
noteoff 2 59 0
sleep 1.712
noteon 12 59 102
sleep 8.560
noteon 13 47 104
sleep 3.424
noteoff 15 50 0
sleep 1.712
noteon 14 35 106
sleep 44.512
noteoff 10 86 0
sleep 20.544
noteoff 11 62 0
sleep 6.848
noteoff 12 59 0
sleep 6.848
noteon 10 85 102
sleep 1.712
noteoff 13 47 0
sleep 1.712
noteon 11 62 102
sleep 3.424
noteoff 14 35 0
sleep 3.424
noteon 12 59 102
sleep 8.561
noteon 13 47 104
sleep 5.136
noteon 14 35 106
sleep 44.518
noteoff 10 85 0
sleep 20.546
noteoff 11 62 0
sleep 6.849
noteoff 12 59 0
sleep 6.849
echo "187440 tempo_s=314 tempo_l=0.25"
noteon 10 86 102
sleep 1.592
noteoff 13 47 0
noteon 0 91 101
noteon 0 86 101
sleep 1.592
noteon 11 67 102
sleep 3.184
noteoff 14 35 0
sleep 3.184
noteon 12 59 102
sleep 7.961
noteon 13 47 104
sleep 4.776
noteon 14 35 106
sleep 41.401
noteoff 10 86 0
sleep 19.108
noteoff 11 67 0
sleep 6.369
noteoff 12 59 0
sleep 6.369
noteon 10 85 102
sleep 1.592
noteoff 13 47 0
sleep 1.592
noteon 11 67 102
sleep 3.184
noteoff 14 35 0
sleep 3.184
noteon 12 59 102
sleep 7.961
noteon 13 47 104
sleep 4.777
noteon 14 35 106
sleep 41.401
noteoff 10 85 0
sleep 19.108
noteoff 11 67 0
sleep 6.369
noteoff 12 59 0
sleep 6.369
noteon 10 86 102
sleep 1.592
noteoff 13 47 0
sleep 1.592
noteon 11 71 102
sleep 3.184
noteoff 14 35 0
sleep 3.184
noteon 12 59 102
sleep 7.961
noteon 13 47 104
sleep 4.777
noteon 14 35 106
sleep 41.401
noteoff 10 86 0
sleep 19.108
noteoff 11 71 0
sleep 6.369
noteoff 12 59 0
sleep 6.369
noteon 10 85 102
sleep 1.592
noteoff 13 47 0
sleep 1.592
noteon 11 71 102
sleep 3.184
noteoff 14 35 0
sleep 3.184
noteon 12 59 102
sleep 7.961
noteon 13 47 104
sleep 4.777
noteon 14 35 106
sleep 41.401
noteoff 10 85 0
sleep 19.108
noteoff 11 71 0
sleep 6.369
noteoff 12 59 0
sleep 6.369
echo "187680 tempo_s=292 tempo_l=0.25"
noteon 10 86 102
sleep 1.712
noteoff 13 47 0
sleep 1.712
noteoff 1 86 0
noteon 1 79 100
noteon 11 74 102
sleep 3.424
noteoff 14 35 0
sleep 1.712
noteon 2 67 101
noteon 2 74 101
sleep 1.712
noteon 12 58 102
sleep 8.561
noteon 13 46 104
sleep 1.712
noteoff 3 47 0
noteon 3 58 100
noteon 3 46 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 34 106
sleep 27.397
noteoff 10 86 0
sleep 37.671
noteoff 11 74 0
sleep 6.849
noteoff 12 58 0
sleep 6.849
noteon 10 84 102
sleep 1.712
noteoff 13 46 0
sleep 1.712
noteon 11 74 102
sleep 3.424
noteoff 14 34 0
sleep 3.424
noteon 12 58 102
sleep 8.561
noteon 13 46 104
sleep 5.136
noteon 14 34 106
sleep 27.397
noteoff 10 84 0
sleep 37.671
noteoff 11 74 0
sleep 6.849
noteoff 12 58 0
sleep 6.849
noteon 10 82 102
sleep 1.712
noteoff 13 46 0
sleep 1.712
noteon 11 79 102
sleep 3.424
noteoff 14 34 0
sleep 3.424
noteon 12 58 102
sleep 8.561
noteon 13 46 104
sleep 3.424
noteoff 15 50 0
sleep 1.712
noteon 14 34 106
sleep 27.397
noteoff 10 82 0
sleep 37.671
noteoff 11 79 0
sleep 6.849
noteoff 12 58 0
sleep 6.849
noteon 10 81 102
sleep 1.712
noteoff 13 46 0
sleep 1.712
noteon 11 79 102
sleep 3.424
noteoff 14 34 0
sleep 3.424
noteon 12 58 102
sleep 8.561
noteon 13 46 104
sleep 5.136
noteon 14 34 106
sleep 27.397
noteoff 10 81 0
sleep 37.671
noteoff 11 79 0
sleep 6.849
noteoff 12 58 0
sleep 6.849
echo "187920 tempo_s=314 tempo_l=0.25"
noteon 10 79 102
sleep 1.592
noteoff 0 91 0
noteoff 13 46 0
noteon 0 91 101
sleep 1.592
noteon 11 82 102
sleep 3.184
noteoff 14 34 0
sleep 3.184
noteon 12 58 102
sleep 7.961
noteon 13 46 104
sleep 4.777
noteon 14 34 106
sleep 25.477
noteoff 10 79 0
sleep 35.031
noteoff 11 82 0
sleep 6.369
noteoff 12 58 0
sleep 6.369
noteon 10 77 102
sleep 1.592
noteoff 13 46 0
sleep 1.592
noteon 11 82 102
sleep 3.184
noteoff 14 34 0
sleep 3.184
noteon 12 58 102
sleep 7.961
noteon 13 46 104
sleep 4.777
noteon 14 34 106
sleep 25.477
noteoff 10 77 0
sleep 35.031
noteoff 11 82 0
sleep 6.369
noteoff 12 58 0
sleep 6.369
noteon 10 76 102
sleep 1.592
noteoff 13 46 0
sleep 1.592
noteon 11 86 102
sleep 3.184
noteoff 14 34 0
sleep 3.184
noteon 12 58 102
sleep 7.961
noteon 13 46 104
sleep 4.777
noteon 14 34 106
sleep 25.477
noteoff 10 76 0
sleep 35.031
noteoff 11 86 0
sleep 6.369
noteoff 12 58 0
sleep 6.369
noteon 10 74 102
sleep 1.592
noteoff 13 46 0
sleep 1.592
noteon 11 86 102
sleep 3.184
noteoff 14 34 0
sleep 3.184
noteon 12 58 102
sleep 7.961
noteon 13 46 104
sleep 4.777
noteon 14 34 106
sleep 25.477
noteoff 10 74 0
sleep 35.031
noteoff 11 86 0
sleep 6.369
noteoff 12 58 0
sleep 6.369
echo "188160 tempo_s=292 tempo_l=0.25"
noteon 10 81 102
sleep 1.712
noteoff 0 91 0
noteoff 0 86 0
noteoff 13 46 0
noteon 0 86 101
noteon 0 90 101
sleep 1.712
noteoff 1 79 0
noteoff 1 74 0
noteoff 4 62 0
noteon 1 78 100
noteon 1 74 100
noteon 11 74 102
noteon 4 62 100
noteon 11 66 102
sleep 1.712
noteoff 6 62 0
noteoff 6 74 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 34 0
sleep 1.712
noteoff 2 74 0
noteoff 2 67 0
noteon 2 66 101
noteon 2 74 101
sleep 1.712
noteoff 5 50 0
noteon 12 57 102
noteon 5 50 100
sleep 8.560
noteon 13 45 104
sleep 1.712
noteoff 3 46 0
noteoff 3 58 0
noteon 3 45 100
noteon 3 57 100
sleep 1.712
noteon 15 50 110
sleep 1.712
noteon 14 33 106
sleep 61.632
noteoff 10 81 0
sleep 3.424
noteoff 11 66 0
noteoff 11 74 0
sleep 6.848
noteoff 12 57 0
sleep 6.848
noteon 10 80 102
sleep 1.712
noteoff 13 45 0
sleep 1.712
noteon 11 66 102
noteon 11 74 102
sleep 3.424
noteoff 14 33 0
sleep 3.424
noteon 12 57 102
sleep 8.560
noteon 13 45 104
sleep 5.136
noteon 14 33 106
sleep 61.632
noteoff 10 80 0
sleep 3.424
noteoff 11 74 0
noteoff 11 66 0
sleep 6.848
noteoff 12 57 0
sleep 6.848
noteon 10 81 102
sleep 1.712
noteoff 13 45 0
sleep 1.712
noteon 11 66 102
noteon 11 74 102
sleep 3.424
noteoff 14 33 0
sleep 3.424
noteon 12 57 102
sleep 8.560
noteon 13 45 104
sleep 3.424
noteoff 15 50 0
sleep 1.712
noteon 14 33 106
sleep 61.632
noteoff 10 81 0
sleep 3.424
noteoff 11 74 0
noteoff 11 66 0
sleep 6.848
noteoff 12 57 0
sleep 6.848
noteon 10 80 102
sleep 1.712
noteoff 13 45 0
sleep 1.712
noteon 11 66 102
noteon 11 74 102
sleep 3.424
noteoff 14 33 0
sleep 3.424
noteon 12 57 102
sleep 8.561
noteon 13 45 104
sleep 5.136
noteon 14 33 106
sleep 61.638
noteoff 10 80 0
sleep 3.424
noteoff 11 74 0
noteoff 11 66 0
sleep 6.849
noteoff 12 57 0
sleep 6.848
echo "188400 tempo_s=314 tempo_l=0.25"
noteon 10 81 102
sleep 1.592
noteoff 13 45 0
sleep 1.592
noteon 11 66 102
noteon 11 74 102
sleep 3.184
noteoff 14 33 0
sleep 3.184
noteon 12 57 102
sleep 7.961
noteon 13 45 104
sleep 4.777
noteon 14 33 106
sleep 57.324
noteoff 10 81 0
sleep 3.184
noteoff 11 74 0
noteoff 11 66 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 78 102
sleep 1.592
noteoff 13 45 0
sleep 1.592
noteon 11 66 102
noteon 11 74 102
sleep 3.184
noteoff 14 33 0
sleep 3.184
noteon 12 57 102
sleep 7.961
noteon 13 45 104
sleep 4.777
noteon 14 33 106
sleep 57.324
noteoff 10 78 0
sleep 3.184
noteoff 11 74 0
noteoff 11 66 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 76 102
sleep 1.592
noteoff 13 45 0
sleep 1.592
noteon 11 66 102
noteon 11 74 102
sleep 3.184
noteoff 14 33 0
sleep 3.184
noteon 12 57 102
sleep 7.961
noteon 13 45 104
sleep 4.777
noteon 14 33 106
sleep 57.324
noteoff 10 76 0
sleep 3.184
noteoff 11 74 0
noteoff 11 66 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 74 102
sleep 1.592
noteoff 13 45 0
sleep 1.592
noteon 11 66 102
noteon 11 74 102
sleep 3.184
noteoff 14 33 0
sleep 3.184
noteon 12 57 102
sleep 7.961
noteon 13 45 104
sleep 4.777
noteon 14 33 106
sleep 57.324
noteoff 10 74 0
sleep 3.184
noteoff 11 74 0
noteoff 11 66 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
echo "188640 tempo_s=292 tempo_l=0.25"
noteon 10 79 102
sleep 1.712
noteoff 0 90 0
noteoff 0 86 0
noteoff 13 45 0
noteon 0 85 101
noteon 0 88 101
sleep 1.712
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 62 0
noteon 1 73 100
noteon 1 76 100
noteon 11 64 102
noteon 11 73 102
noteon 4 64 100
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 57 108
noteon 6 69 108
sleep 1.712
noteoff 14 33 0
sleep 1.712
noteoff 2 74 0
noteoff 2 66 0
noteon 2 73 101
noteon 2 64 101
sleep 1.712
noteoff 5 50 0
noteon 12 57 102
noteon 5 57 100
sleep 8.560
noteon 13 45 104
sleep 1.712
noteoff 3 57 0
noteoff 3 45 0
noteon 3 57 100
noteon 3 45 100
sleep 1.712
noteon 15 45 110
sleep 1.712
noteon 14 33 106
sleep 61.632
noteoff 10 79 0
sleep 3.424
noteoff 11 73 0
noteoff 11 64 0
sleep 6.848
noteoff 12 57 0
sleep 6.848
noteon 10 78 102
sleep 1.712
noteoff 13 45 0
sleep 1.712
noteon 11 64 102
noteon 11 73 102
sleep 3.424
noteoff 14 33 0
sleep 3.424
noteon 12 57 102
sleep 8.560
noteon 13 45 104
sleep 5.136
noteon 14 33 106
sleep 61.632
noteoff 10 78 0
sleep 3.424
noteoff 11 73 0
noteoff 11 64 0
sleep 6.848
noteoff 12 57 0
sleep 6.848
noteon 10 79 102
sleep 1.712
noteoff 13 45 0
sleep 1.712
noteon 11 64 102
noteon 11 73 102
sleep 3.424
noteoff 14 33 0
sleep 3.424
noteon 12 57 102
sleep 8.560
noteon 13 45 104
sleep 3.424
noteoff 15 45 0
sleep 1.712
noteon 14 33 106
sleep 61.632
noteoff 10 79 0
sleep 3.424
noteoff 11 73 0
noteoff 11 64 0
sleep 6.848
noteoff 12 57 0
sleep 6.848
noteon 10 78 102
sleep 1.712
noteoff 13 45 0
sleep 1.712
noteon 11 64 102
noteon 11 73 102
sleep 3.424
noteoff 14 33 0
sleep 3.424
noteon 12 57 102
sleep 8.561
noteon 13 45 104
sleep 5.136
noteon 14 33 106
sleep 61.638
noteoff 10 78 0
sleep 3.424
noteoff 11 73 0
noteoff 11 64 0
sleep 6.849
noteoff 12 57 0
sleep 6.848
echo "188880 tempo_s=314 tempo_l=0.25"
noteon 10 79 102
sleep 1.592
noteoff 13 45 0
sleep 1.592
noteon 11 64 102
noteon 11 73 102
sleep 3.184
noteoff 14 33 0
sleep 3.184
noteon 12 57 102
sleep 7.961
noteon 13 45 104
sleep 4.777
noteon 14 33 106
sleep 57.324
noteoff 10 79 0
sleep 3.184
noteoff 11 73 0
noteoff 11 64 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 76 102
sleep 1.592
noteoff 13 45 0
sleep 1.592
noteon 11 64 102
noteon 11 73 102
sleep 3.184
noteoff 14 33 0
sleep 3.184
noteon 12 57 102
sleep 7.961
noteon 13 45 104
sleep 4.777
noteon 14 33 106
sleep 57.324
noteoff 10 76 0
sleep 3.184
noteoff 11 73 0
noteoff 11 64 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 74 102
sleep 1.592
noteoff 13 45 0
sleep 1.592
noteon 11 64 102
noteon 11 73 102
sleep 3.184
noteoff 14 33 0
sleep 3.184
noteon 12 57 102
sleep 7.961
noteon 13 45 104
sleep 4.777
noteon 14 33 106
sleep 57.324
noteoff 10 74 0
sleep 3.184
noteoff 11 73 0
noteoff 11 64 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 73 102
sleep 1.592
noteoff 13 45 0
sleep 1.592
noteon 11 64 102
noteon 11 73 102
sleep 3.184
noteoff 14 33 0
sleep 3.184
noteon 12 57 102
sleep 7.961
noteon 13 45 104
sleep 4.777
noteon 14 33 106
sleep 25.477
echo "189090 tempo_s=158 tempo_l=0.25"
sleep 63.291
noteoff 10 73 0
sleep 6.329
noteoff 11 73 0
noteoff 11 64 0
sleep 12.658
noteoff 12 57 0
sleep 12.658
echo "189120 tempo_s=292 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 0 88 0
noteoff 0 85 0
noteoff 13 45 0
noteon 0 84 101
sleep 1.712
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteon 1 72 100
noteon 1 84 100
noteon 4 62 100
noteon 11 72 102
sleep 1.712
noteoff 6 69 0
noteoff 6 57 0
noteon 6 74 108
noteon 6 62 108
sleep 1.712
noteoff 14 33 0
sleep 1.712
noteoff 2 64 0
noteoff 2 73 0
noteon 2 72 101
sleep 1.712
noteoff 5 57 0
noteon 5 50 100
noteon 12 72 102
sleep 8.561
noteon 13 60 104
sleep 1.712
noteoff 3 45 0
noteoff 3 57 0
noteon 3 60 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 48 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 84 0
sleep 1.712
noteoff 1 84 0
noteoff 1 72 0
noteoff 11 72 0
sleep 5.136
noteoff 2 72 0
sleep 1.712
noteoff 12 72 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.712
noteoff 3 60 0
noteon 1 71 100
noteon 1 83 100
noteon 11 71 102
sleep 3.424
noteoff 14 48 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 83 0
noteoff 1 71 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 81 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 69 100
noteon 1 81 100
noteon 4 62 100
noteon 11 69 102
sleep 1.712
noteoff 6 62 0
noteoff 6 74 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 69 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 1.712
noteon 3 57 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 45 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 81 0
sleep 1.712
noteoff 1 81 0
noteoff 1 69 0
noteoff 11 69 0
sleep 5.136
noteoff 2 69 0
sleep 1.712
noteoff 12 69 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 57 0
noteon 0 83 101
sleep 1.712
noteoff 3 57 0
noteon 1 71 100
noteon 1 83 100
noteon 11 71 102
sleep 3.424
noteoff 14 45 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 83 0
noteoff 1 71 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
echo "189360 tempo_s=314 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 71 100
noteon 1 83 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 83 0
noteoff 1 71 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
echo "189600 tempo_s=292 tempo_l=0.25"
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 72 100
noteon 1 84 100
noteon 4 62 100
noteon 11 72 102
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 72 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 8.561
noteon 13 60 104
sleep 1.712
noteon 3 60 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 48 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 84 0
sleep 1.712
noteoff 1 84 0
noteoff 1 72 0
noteoff 11 72 0
sleep 5.136
noteoff 2 72 0
sleep 1.712
noteoff 12 72 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.712
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.424
noteoff 14 48 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 81 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 69 100
noteon 1 81 100
noteon 4 62 100
noteon 11 69 102
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 69 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 1.712
noteon 3 57 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 45 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 81 0
sleep 1.712
noteoff 1 81 0
noteoff 1 69 0
noteoff 11 69 0
sleep 5.136
noteoff 2 69 0
sleep 1.712
noteoff 12 69 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 57 0
noteon 0 83 101
sleep 1.712
noteoff 3 57 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.424
noteoff 14 45 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
echo "189840 tempo_s=314 tempo_l=0.25"
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 74 108
noteon 6 62 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 72 100
noteon 1 84 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 62 0
noteoff 6 74 0
noteon 6 74 108
noteon 6 62 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 84 0
noteoff 1 72 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
echo "190080 tempo_s=292 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 72 100
noteon 1 84 100
noteon 4 62 100
noteon 11 72 102
sleep 1.712
noteoff 6 62 0
noteoff 6 74 0
noteon 6 74 108
noteon 6 62 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 72 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 8.561
noteon 13 60 104
sleep 1.712
noteon 3 60 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 48 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 84 0
sleep 1.712
noteoff 1 84 0
noteoff 1 72 0
noteoff 11 72 0
sleep 5.136
noteoff 2 72 0
sleep 1.712
noteoff 12 72 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.712
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.424
noteoff 14 48 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.712
noteoff 6 62 0
noteoff 6 74 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 72 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 8.561
noteon 13 60 104
sleep 1.712
noteon 3 60 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 48 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 84 0
sleep 1.712
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 5.136
noteoff 2 72 0
sleep 1.712
noteoff 12 72 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.712
noteoff 3 60 0
noteon 1 71 100
noteon 1 83 100
noteon 11 71 102
sleep 3.424
noteoff 14 48 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 83 0
noteoff 1 71 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
echo "190320 tempo_s=314 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.592
noteoff 3 59 0
noteoff 4 62 0
noteon 1 72 100
noteon 1 84 100
noteon 4 62 100
noteon 11 72 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 47 0
sleep 1.592
noteon 2 72 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 7.961
noteon 13 60 104
sleep 1.592
noteon 3 60 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 48 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 84 0
sleep 1.592
noteoff 1 84 0
noteoff 1 72 0
noteoff 11 72 0
sleep 4.777
noteoff 2 72 0
sleep 1.592
noteoff 12 72 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.592
noteoff 3 60 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 3.184
noteoff 14 48 0
sleep 1.592
noteon 2 71 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 1.592
noteon 3 59 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 47 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 83 0
sleep 1.592
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
sleep 4.777
noteoff 2 71 0
sleep 1.592
noteoff 12 71 0
sleep 6.369
echo "190560 tempo_s=292 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 84 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 84 100
noteon 1 72 100
noteon 4 62 100
noteon 11 72 102
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 72 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 72 102
sleep 8.561
noteon 13 60 104
sleep 1.712
noteon 3 60 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 48 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 84 0
sleep 1.712
noteoff 1 72 0
noteoff 1 84 0
noteoff 11 72 0
sleep 5.136
noteoff 2 72 0
sleep 1.712
noteoff 12 72 0
sleep 6.849
noteon 10 74 102
noteon 10 86 102
sleep 1.712
noteoff 15 50 0
noteoff 13 60 0
noteon 0 83 101
sleep 1.712
noteoff 3 60 0
noteon 1 71 100
noteon 1 83 100
noteon 11 71 102
sleep 3.424
noteoff 14 48 0
sleep 1.712
noteon 2 71 101
sleep 1.712
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 1.712
noteon 3 59 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 47 106
sleep 61.643
noteoff 10 86 0
noteoff 10 74 0
sleep 1.712
noteoff 0 83 0
sleep 1.712
noteoff 1 83 0
noteoff 1 71 0
noteoff 11 71 0
sleep 5.136
noteoff 2 71 0
sleep 1.712
noteoff 12 71 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 59 0
noteon 0 81 101
sleep 1.712
noteoff 3 59 0
noteoff 4 62 0
noteon 1 81 100
noteon 1 69 100
noteon 4 62 100
noteon 11 69 102
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.712
noteoff 14 47 0
sleep 1.712
noteon 2 69 101
sleep 1.712
noteoff 5 50 0
noteon 5 50 100
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 1.712
noteon 3 57 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 45 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 81 0
sleep 1.712
noteoff 1 69 0
noteoff 1 81 0
noteoff 11 69 0
sleep 5.136
noteoff 2 69 0
sleep 1.712
noteoff 12 69 0
sleep 6.849
noteon 10 86 102
noteon 10 74 102
sleep 1.712
noteoff 15 50 0
noteoff 13 57 0
noteon 0 79 101
sleep 1.712
noteoff 3 57 0
noteon 1 79 100
noteon 1 67 100
noteon 11 67 102
sleep 3.424
noteoff 14 45 0
sleep 1.712
noteon 2 67 101
sleep 1.712
noteon 12 67 102
sleep 8.561
noteon 13 55 104
sleep 1.712
noteon 3 55 100
sleep 1.712
noteon 15 50 90
sleep 1.712
noteon 14 43 106
sleep 61.643
noteoff 10 74 0
noteoff 10 86 0
sleep 1.712
noteoff 0 79 0
sleep 1.712
noteoff 1 67 0
noteoff 1 79 0
noteoff 11 67 0
sleep 5.136
noteoff 2 67 0
sleep 1.712
noteoff 12 67 0
sleep 6.849
echo "190800 tempo_s=314 tempo_l=0.25"
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 55 0
noteon 0 78 101
sleep 1.592
noteoff 3 55 0
noteoff 4 62 0
noteon 1 66 100
noteon 1 78 100
noteon 4 62 100
noteon 11 66 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 43 0
sleep 1.592
noteon 2 66 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 66 102
sleep 7.961
noteon 13 54 104
sleep 1.592
noteon 3 54 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 42 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 78 0
sleep 1.592
noteoff 1 78 0
noteoff 1 66 0
noteoff 11 66 0
sleep 4.777
noteoff 2 66 0
sleep 1.592
noteoff 12 66 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 54 0
noteon 0 76 101
sleep 1.592
noteoff 3 54 0
noteon 1 76 100
noteon 1 64 100
noteon 11 64 102
sleep 3.184
noteoff 14 42 0
sleep 1.592
noteon 2 64 101
sleep 1.592
noteon 12 64 102
sleep 7.961
noteon 13 52 104
sleep 1.592
noteon 3 52 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 40 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 76 0
sleep 1.592
noteoff 1 64 0
noteoff 1 76 0
noteoff 11 64 0
sleep 4.777
noteoff 2 64 0
sleep 1.592
noteoff 12 64 0
sleep 6.369
noteon 10 74 102
noteon 10 86 102
sleep 1.592
noteoff 15 50 0
noteoff 13 52 0
noteon 0 74 101
sleep 1.592
noteoff 3 52 0
noteoff 4 62 0
noteon 1 74 100
noteon 1 62 100
noteon 4 62 100
noteon 11 62 102
sleep 1.592
noteoff 6 74 0
noteoff 6 62 0
noteon 6 62 108
noteon 6 74 108
sleep 1.592
noteoff 14 40 0
sleep 1.592
noteon 2 62 101
sleep 1.592
noteoff 5 50 0
noteon 5 50 100
noteon 12 62 102
sleep 7.961
noteon 13 50 104
sleep 1.592
noteon 3 50 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 38 106
sleep 57.324
noteoff 10 86 0
noteoff 10 74 0
sleep 1.592
noteoff 0 74 0
sleep 1.592
noteoff 1 62 0
noteoff 1 74 0
noteoff 11 62 0
sleep 4.777
noteoff 2 62 0
sleep 1.592
noteoff 12 62 0
sleep 6.369
noteon 10 86 102
noteon 10 74 102
sleep 1.592
noteoff 15 50 0
noteoff 13 50 0
noteon 0 72 101
sleep 1.592
noteoff 3 50 0
noteon 1 60 100
noteon 1 72 100
noteon 11 60 102
sleep 3.184
noteoff 14 38 0
sleep 1.592
noteon 2 60 101
sleep 1.592
noteon 12 60 102
sleep 7.961
noteon 13 48 104
sleep 1.592
noteon 3 48 100
sleep 1.592
noteon 15 50 90
sleep 1.592
noteon 14 36 106
sleep 57.324
noteoff 10 74 0
noteoff 10 86 0
sleep 1.592
noteoff 0 72 0
sleep 1.592
noteoff 1 72 0
noteoff 1 60 0
noteoff 11 60 0
sleep 4.777
noteoff 2 60 0
sleep 1.592
noteoff 12 60 0
sleep 6.369
echo "191040 tempo_s=292 tempo_l=0.25"
noteon 10 87 102
sleep 1.712
noteoff 15 50 0
noteoff 13 48 0
noteon 0 71 101
sleep 1.712
noteoff 3 48 0
noteoff 4 62 0
noteon 1 78 100
noteon 1 75 100
noteon 11 59 102
noteon 4 66 110
sleep 1.712
noteoff 6 74 0
noteoff 6 62 0
sleep 1.712
noteoff 14 36 0
sleep 1.712
noteon 2 59 101
sleep 1.712
noteoff 5 50 0
noteon 12 59 102
noteon 5 66 110
sleep 8.560
noteon 13 47 104
sleep 1.712
noteon 3 47 100
noteon 3 59 100
sleep 3.424
noteon 14 35 106
sleep 61.632
noteoff 10 87 0
sleep 10.272
noteoff 12 59 0
sleep 6.848
noteon 10 87 102
sleep 1.712
noteoff 13 47 0
sleep 5.136
noteoff 14 35 0
sleep 3.424
noteon 12 71 102
sleep 8.560
noteon 13 59 104
sleep 5.136
noteon 14 47 106
sleep 61.632
noteoff 10 87 0
sleep 3.424
noteoff 11 59 0
sleep 6.848
noteoff 12 71 0
sleep 6.848
noteon 10 87 102
sleep 1.712
noteoff 0 71 0
noteoff 13 59 0
sleep 1.712
noteon 11 83 102
sleep 3.424
noteoff 14 47 0
sleep 1.712
noteoff 2 59 0
sleep 1.712
noteon 12 71 102
sleep 8.560
noteon 13 59 104
sleep 5.136
noteon 14 47 106
sleep 61.632
noteoff 10 87 0
sleep 3.424
noteoff 11 83 0
sleep 6.848
noteoff 12 71 0
sleep 6.848
noteon 10 87 102
sleep 1.712
noteoff 13 59 0
sleep 1.712
noteon 11 82 102
sleep 3.424
noteoff 14 47 0
sleep 3.424
noteon 12 71 102
sleep 8.561
noteon 13 59 104
sleep 5.136
noteon 14 47 106
sleep 61.643
noteoff 10 87 0
sleep 3.424
noteoff 11 82 0
sleep 6.849
noteoff 12 71 0
sleep 6.849
echo "191280 tempo_s=314 tempo_l=0.25"
noteon 10 87 102
sleep 1.592
noteoff 13 59 0
noteon 0 90 101
noteon 0 87 101
sleep 1.592
noteon 11 83 102
sleep 3.184
noteoff 14 47 0
sleep 1.592
noteon 2 71 101
noteon 2 66 101
sleep 1.592
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 4.777
noteon 14 47 106
sleep 57.324
noteoff 10 87 0
sleep 3.184
noteoff 11 83 0
sleep 6.369
noteoff 12 71 0
sleep 6.369
noteon 10 87 102
sleep 1.592
noteoff 13 59 0
sleep 1.592
noteon 11 82 102
sleep 3.184
noteoff 14 47 0
sleep 3.184
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 4.777
noteon 14 47 106
sleep 57.324
noteoff 10 87 0
sleep 3.184
noteoff 11 82 0
sleep 6.369
noteoff 12 71 0
sleep 6.369
noteon 10 87 102
sleep 1.592
noteoff 13 59 0
sleep 1.592
noteon 11 83 102
sleep 3.184
noteoff 14 47 0
sleep 3.184
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 4.777
noteon 14 47 106
sleep 57.324
noteoff 10 87 0
sleep 3.184
noteoff 11 83 0
sleep 6.369
noteoff 12 71 0
sleep 6.369
noteon 10 87 102
sleep 1.592
noteoff 13 59 0
sleep 1.592
noteon 11 82 102
sleep 3.184
noteoff 14 47 0
sleep 3.184
noteon 12 71 102
sleep 7.961
noteon 13 59 104
sleep 4.777
noteon 14 47 106
sleep 57.324
noteoff 10 87 0
sleep 3.184
noteoff 11 82 0
sleep 6.369
noteoff 12 71 0
sleep 6.369
echo "191520 tempo_s=292 tempo_l=0.25"
noteon 10 87 102
sleep 1.712
noteoff 13 59 0
sleep 1.712
noteon 11 83 102
sleep 3.424
noteoff 14 47 0
sleep 3.424
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 1.712
noteoff 3 59 0
noteoff 3 47 0
noteon 3 57 100
noteon 3 45 100
sleep 3.424
noteon 14 45 106
sleep 61.643
noteoff 10 87 0
sleep 3.424
noteoff 11 83 0
sleep 6.849
noteoff 12 69 0
sleep 6.849
noteon 10 87 102
sleep 1.712
noteoff 13 57 0
sleep 1.712
noteon 11 81 102
sleep 3.424
noteoff 14 45 0
sleep 3.424
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 5.136
noteon 14 45 106
sleep 61.643
noteoff 10 87 0
sleep 3.424
noteoff 11 81 0
sleep 6.849
noteoff 12 69 0
sleep 6.849
noteon 10 87 102
sleep 1.712
noteoff 13 57 0
sleep 1.712
noteon 11 79 102
sleep 3.424
noteoff 14 45 0
sleep 3.424
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 5.136
noteon 14 45 106
sleep 61.643
noteoff 10 87 0
sleep 3.424
noteoff 11 79 0
sleep 6.849
noteoff 12 69 0
sleep 6.849
noteon 10 87 102
sleep 1.712
noteoff 13 57 0
sleep 1.712
noteon 11 78 102
sleep 3.424
noteoff 14 45 0
sleep 3.424
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 5.136
noteon 14 45 106
sleep 61.643
noteoff 10 87 0
sleep 3.424
noteoff 11 78 0
sleep 6.849
noteoff 12 69 0
sleep 6.849
echo "191760 tempo_s=314 tempo_l=0.25"
noteon 10 87 102
sleep 1.592
noteoff 0 87 0
noteoff 0 90 0
noteoff 13 57 0
noteon 0 90 101
noteon 0 87 101
sleep 1.592
noteon 11 76 102
sleep 3.184
noteoff 14 45 0
sleep 3.184
noteon 12 69 102
sleep 7.961
noteon 13 57 104
sleep 4.777
noteon 14 45 106
sleep 57.324
noteoff 10 87 0
sleep 3.184
noteoff 11 76 0
sleep 6.369
noteoff 12 69 0
sleep 6.369
noteon 10 87 102
sleep 1.592
noteoff 13 57 0
sleep 1.592
noteon 11 75 102
sleep 3.184
noteoff 14 45 0
sleep 3.184
noteon 12 69 102
sleep 7.961
noteon 13 57 104
sleep 4.777
noteon 14 45 106
sleep 57.324
noteoff 10 87 0
sleep 3.184
noteoff 11 75 0
sleep 6.369
noteoff 12 69 0
sleep 6.369
noteon 10 87 102
sleep 1.592
noteoff 13 57 0
sleep 1.592
noteon 11 73 102
sleep 3.184
noteoff 14 45 0
sleep 3.184
noteon 12 69 102
sleep 7.961
noteon 13 57 104
sleep 4.777
noteon 14 45 106
sleep 57.324
noteoff 10 87 0
sleep 3.184
noteoff 11 73 0
sleep 6.369
noteoff 12 69 0
sleep 6.369
noteon 10 87 102
sleep 1.592
noteoff 13 57 0
sleep 1.592
noteon 11 71 102
sleep 3.184
noteoff 14 45 0
sleep 3.184
noteon 12 69 102
sleep 7.961
noteon 13 57 104
sleep 4.777
noteon 14 45 106
sleep 57.324
noteoff 10 87 0
sleep 3.184
noteoff 11 71 0
sleep 6.369
noteoff 12 69 0
sleep 6.369
echo "192000 tempo_s=292 tempo_l=0.25"
noteon 10 88 102
sleep 1.712
noteoff 0 87 0
noteoff 0 90 0
noteoff 13 57 0
noteon 0 88 101
noteon 0 91 101
sleep 1.712
noteoff 1 75 0
noteoff 1 78 0
noteoff 4 66 0
noteon 1 79 100
noteon 1 76 100
noteon 4 67 110
noteon 11 79 102
sleep 1.712
noteon 6 76 118
noteon 6 79 118
sleep 1.712
noteoff 14 45 0
sleep 1.712
noteoff 2 66 0
noteoff 2 71 0
noteon 2 71 101
noteon 2 67 101
sleep 1.712
noteoff 5 66 0
noteon 5 64 110
noteon 12 67 102
sleep 8.561
noteon 13 55 104
sleep 1.712
noteoff 3 45 0
noteoff 3 57 0
noteon 3 55 100
noteon 3 43 100
sleep 3.424
noteon 14 43 106
sleep 61.643
noteoff 10 88 0
sleep 3.424
noteoff 11 79 0
sleep 6.849
noteoff 12 67 0
sleep 6.849
noteon 10 88 102
sleep 1.712
noteoff 13 55 0
sleep 1.712
noteon 11 78 102
sleep 3.424
noteoff 14 43 0
sleep 3.424
noteon 12 67 102
sleep 8.561
noteon 13 55 104
sleep 5.136
noteon 14 43 106
sleep 61.643
noteoff 10 88 0
sleep 3.424
noteoff 11 78 0
sleep 6.849
noteoff 12 67 0
sleep 6.849
noteon 10 88 102
sleep 1.712
noteoff 13 55 0
sleep 1.712
noteon 11 79 102
sleep 3.424
noteoff 14 43 0
sleep 3.424
noteon 12 67 102
sleep 8.561
noteon 13 55 104
sleep 1.712
noteoff 3 43 0
noteoff 3 55 0
noteon 3 55 100
sleep 3.424
noteon 14 43 106
sleep 61.643
noteoff 10 88 0
sleep 3.424
noteoff 11 79 0
sleep 6.849
noteoff 12 67 0
sleep 6.849
noteon 10 88 102
sleep 1.712
noteoff 13 55 0
sleep 1.712
noteon 11 78 102
sleep 3.424
noteoff 14 43 0
sleep 3.424
noteon 12 67 102
sleep 8.561
noteon 13 55 104
sleep 5.136
noteon 14 43 106
sleep 61.643
noteoff 10 88 0
sleep 3.424
noteoff 11 78 0
sleep 6.849
noteoff 12 67 0
sleep 6.849
echo "192240 tempo_s=314 tempo_l=0.25"
noteon 10 91 102
sleep 1.592
noteoff 13 55 0
sleep 1.592
noteon 11 79 102
sleep 3.184
noteoff 14 43 0
sleep 3.184
noteon 12 64 102
sleep 7.961
noteon 13 52 104
sleep 1.592
noteoff 3 55 0
noteon 3 52 100
sleep 3.184
noteon 14 40 106
sleep 57.324
noteoff 10 91 0
sleep 3.184
noteoff 11 79 0
sleep 6.369
noteoff 12 64 0
sleep 6.369
noteon 10 91 102
sleep 1.592
noteoff 13 52 0
sleep 1.592
noteon 11 81 102
sleep 3.184
noteoff 14 40 0
sleep 3.184
noteon 12 64 102
sleep 7.961
noteon 13 52 104
sleep 4.777
noteon 14 40 106
sleep 57.324
noteoff 10 91 0
sleep 3.184
noteoff 11 81 0
sleep 6.369
noteoff 12 64 0
sleep 6.369
noteon 10 91 102
sleep 1.592
noteoff 13 52 0
sleep 1.592
noteon 11 83 102
sleep 3.184
noteoff 14 40 0
sleep 3.184
noteon 12 64 102
sleep 7.961
noteon 13 52 104
sleep 1.592
noteoff 3 52 0
noteon 3 52 100
sleep 3.184
noteon 14 40 106
sleep 57.324
noteoff 10 91 0
sleep 3.184
noteoff 11 83 0
sleep 6.369
noteoff 12 64 0
sleep 6.369
noteon 10 91 102
sleep 1.592
noteoff 13 52 0
sleep 1.592
noteon 11 79 102
sleep 3.184
noteoff 14 40 0
sleep 3.184
noteon 12 64 102
sleep 7.961
noteon 13 52 104
sleep 4.777
noteon 14 40 106
sleep 57.324
noteoff 10 91 0
sleep 3.184
noteoff 11 79 0
sleep 6.369
noteoff 12 64 0
sleep 6.369
echo "192480 tempo_s=292 tempo_l=0.25"
noteon 10 86 102
sleep 1.712
noteoff 0 91 0
noteoff 0 88 0
noteoff 13 52 0
noteon 0 90 101
noteon 0 86 101
sleep 1.712
noteoff 1 76 0
noteoff 1 79 0
noteoff 4 67 0
noteon 1 78 100
noteon 1 74 100
noteon 4 66 110
noteon 11 78 102
sleep 1.712
noteoff 6 79 0
noteoff 6 76 0
noteon 6 74 118
noteon 6 78 118
sleep 1.712
noteoff 14 40 0
sleep 1.712
noteoff 2 67 0
noteoff 2 71 0
noteon 2 66 101
noteon 2 69 101
sleep 1.712
noteoff 5 64 0
noteon 5 62 110
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 1.712
noteoff 3 52 0
noteon 3 57 100
sleep 1.712
noteon 15 45 90
sleep 1.712
noteon 14 45 106
sleep 61.643
noteoff 10 86 0
sleep 3.424
noteoff 11 78 0
sleep 6.849
noteoff 12 69 0
sleep 6.849
noteon 10 86 102
sleep 1.712
noteoff 13 57 0
sleep 1.712
noteon 11 79 102
sleep 3.424
noteoff 14 45 0
sleep 3.424
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 5.136
noteon 14 45 106
sleep 61.643
noteoff 10 86 0
sleep 3.424
noteoff 11 79 0
sleep 6.849
noteoff 12 69 0
sleep 6.849
noteon 10 86 102
sleep 1.712
noteoff 13 57 0
sleep 1.712
noteon 11 81 102
sleep 3.424
noteoff 14 45 0
sleep 3.424
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 1.712
noteoff 3 57 0
noteon 3 57 100
sleep 1.712
noteoff 15 45 0
sleep 1.712
noteon 14 45 106
sleep 61.643
noteoff 10 86 0
sleep 3.424
noteoff 11 81 0
sleep 6.849
noteoff 12 69 0
sleep 6.849
noteon 10 86 102
sleep 1.712
noteoff 13 57 0
sleep 1.712
noteon 11 78 102
sleep 3.424
noteoff 14 45 0
sleep 3.424
noteon 12 69 102
sleep 8.561
noteon 13 57 104
sleep 5.136
noteon 14 45 106
sleep 61.643
noteoff 10 86 0
sleep 3.424
noteoff 11 78 0
sleep 6.849
noteoff 12 69 0
sleep 6.849
echo "192720 tempo_s=314 tempo_l=0.25"
noteon 10 85 102
sleep 1.592
noteoff 0 86 0
noteoff 0 90 0
noteoff 13 57 0
noteon 0 85 101
noteon 0 88 101
sleep 1.592
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 66 0
noteon 1 76 100
noteon 1 73 100
noteon 4 64 110
noteon 11 76 102
sleep 1.592
noteoff 6 78 0
noteoff 6 74 0
noteon 6 69 118
noteon 6 76 118
sleep 1.592
noteoff 14 45 0
sleep 1.592
noteoff 2 69 0
noteoff 2 66 0
noteon 2 67 101
noteon 2 69 101
sleep 1.592
noteoff 5 62 0
noteon 5 57 110
noteon 12 57 102
sleep 7.961
noteon 13 45 104
sleep 1.592
noteoff 3 57 0
noteon 3 45 100
sleep 1.592
noteon 15 45 90
sleep 1.592
noteon 14 33 106
sleep 57.324
noteoff 10 85 0
sleep 3.184
noteoff 11 76 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 85 102
sleep 1.592
noteoff 13 45 0
sleep 1.592
noteon 11 78 102
sleep 3.184
noteoff 14 33 0
sleep 3.184
noteon 12 57 102
sleep 7.961
noteon 13 45 104
sleep 4.777
noteon 14 33 106
sleep 57.324
noteoff 10 85 0
sleep 3.184
noteoff 11 78 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 85 102
sleep 1.592
noteoff 13 45 0
sleep 1.592
noteon 11 79 102
sleep 3.184
noteoff 14 33 0
sleep 3.184
noteon 12 57 102
sleep 7.961
noteon 13 45 104
sleep 1.592
noteoff 3 45 0
noteon 3 45 100
sleep 1.592
noteoff 15 45 0
sleep 1.592
noteon 14 33 106
sleep 57.324
noteoff 10 85 0
sleep 3.184
noteoff 11 79 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
echo "192900 tempo_s=234 tempo_l=0.25"
noteon 10 85 102
sleep 2.136
noteoff 13 45 0
sleep 2.136
noteon 11 76 102
sleep 4.273
noteoff 14 33 0
sleep 4.273
noteon 12 57 102
sleep 10.683
noteon 13 45 104
sleep 6.41
noteon 14 33 106
sleep 76.923
noteoff 10 85 0
sleep 4.273
noteoff 11 76 0
sleep 8.547
noteoff 12 57 0
sleep 8.547
echo "192960 tempo_s=278 tempo_l=0.25"
noteon 10 86 102
sleep 1.798
noteoff 0 88 0
noteoff 0 85 0
noteoff 13 45 0
noteon 0 86 101
sleep 1.798
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 64 0
noteon 1 74 100
noteon 4 62 110
noteon 11 66 102
sleep 1.798
noteoff 6 76 0
noteoff 6 69 0
noteon 6 66 118
noteon 6 74 118
sleep 1.798
noteoff 14 33 0
sleep 1.798
noteoff 2 69 0
noteoff 2 67 0
noteon 2 66 101
noteon 2 69 101
sleep 1.798
noteoff 5 57 0
noteon 5 54 110
noteon 12 54 102
sleep 8.992
noteon 13 50 104
sleep 1.798
noteoff 3 45 0
noteon 3 50 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteon 14 38 106
sleep 68.340
noteoff 11 66 0
sleep 7.193
noteoff 12 54 0
sleep 10.791
noteon 11 66 102
sleep 7.193
noteon 12 54 102
sleep 82.728
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.193
noteoff 10 86 0
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteoff 1 74 0
noteoff 4 62 0
noteon 11 69 102
sleep 1.798
noteoff 6 74 0
noteoff 6 66 0
sleep 3.597
noteoff 2 69 0
noteoff 2 66 0
sleep 1.798
noteoff 5 54 0
noteon 12 57 102
sleep 8.991
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
sleep 1.798
noteoff 15 50 0
sleep 1.798
noteoff 14 38 0
sleep 68.341
noteoff 11 69 0
sleep 7.193
noteoff 12 57 0
sleep 8.992
noteon 0 92 101
sleep 1.798
noteon 1 68 100
noteon 11 69 102
noteon 1 80 100
sleep 7.194
noteon 12 57 102
sleep 10.791
noteon 3 56 100
sleep 71.942
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
echo "193200 tempo_s=314 tempo_l=0.25"
sleep 1.592
noteoff 0 92 0
noteon 0 93 101
sleep 1.592
noteoff 1 80 0
noteoff 1 68 0
noteon 1 81 100
noteon 1 69 100
noteon 11 74 102
sleep 6.369
noteon 12 62 102
sleep 9.554
noteoff 3 56 0
noteon 3 57 100
sleep 63.694
noteoff 11 74 0
sleep 6.369
noteoff 12 62 0
sleep 9.554
noteon 11 74 102
sleep 6.369
noteon 12 62 102
sleep 73.248
noteoff 11 74 0
sleep 6.369
noteoff 12 62 0
sleep 7.961
noteoff 0 93 0
sleep 1.592
noteoff 1 69 0
noteoff 1 81 0
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 9.554
noteoff 3 57 0
sleep 63.694
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 78 102
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 70.063
noteoff 10 78 0
sleep 3.184
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
echo "193440 tempo_s=278 tempo_l=0.25"
noteon 10 79 102
sleep 3.597
noteon 4 62 100
noteon 11 66 102
sleep 7.194
noteon 5 54 100
noteon 12 54 102
sleep 8.992
noteon 13 50 104
sleep 5.395
noteon 14 38 106
sleep 10.791
noteoff 10 79 0
noteon 10 78 102
sleep 35.971
noteoff 10 78 0
noteon 10 79 102
sleep 21.582
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
noteoff 10 79 0
noteon 10 78 102
sleep 3.597
noteon 11 66 102
sleep 7.194
noteon 12 54 102
sleep 25.179
noteoff 10 78 0
noteon 10 79 102
sleep 35.971
noteoff 10 79 0
noteon 10 78 102
sleep 21.582
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
noteoff 10 78 0
noteon 10 79 102
sleep 3.597
noteon 11 69 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteoff 13 50 0
sleep 5.395
noteoff 14 38 0
sleep 10.791
noteoff 10 79 0
noteon 10 78 102
sleep 35.971
noteoff 10 78 0
noteon 10 79 102
sleep 21.582
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
noteoff 10 79 0
noteon 10 76 102
sleep 3.597
noteon 11 69 102
sleep 7.194
noteon 12 57 102
sleep 43.165
noteoff 10 76 0
noteon 10 78 102
sleep 39.568
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
echo "193680 tempo_s=314 tempo_l=0.25"
noteoff 10 78 0
noteon 10 81 102
sleep 3.184
noteon 11 74 102
sleep 6.369
noteon 12 62 102
sleep 73.248
noteoff 11 74 0
sleep 6.369
noteoff 12 62 0
sleep 6.369
noteoff 10 81 0
sleep 3.184
noteon 11 74 102
sleep 6.369
noteon 12 62 102
sleep 73.248
noteoff 11 74 0
sleep 6.369
noteoff 12 62 0
sleep 6.369
noteon 10 74 102
sleep 3.184
noteon 11 66 102
sleep 6.369
noteon 12 54 102
sleep 73.248
noteoff 11 66 0
sleep 6.369
noteoff 12 54 0
sleep 6.369
noteoff 10 74 0
sleep 3.184
noteon 11 66 102
sleep 6.369
noteon 12 54 102
sleep 73.248
noteoff 11 66 0
sleep 6.369
noteoff 12 54 0
sleep 6.369
echo "193920 tempo_s=278 tempo_l=0.25"
noteon 10 73 102
sleep 3.597
noteoff 4 62 0
noteon 4 64 100
noteon 11 67 102
sleep 7.194
noteoff 5 54 0
noteon 5 57 100
noteon 12 55 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 68.345
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 10.791
noteon 11 67 102
sleep 7.194
noteon 12 55 102
sleep 82.733
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 7.194
noteoff 10 73 0
sleep 3.597
noteoff 4 64 0
noteon 11 69 102
sleep 7.194
noteoff 5 57 0
noteon 12 57 102
sleep 8.992
noteoff 13 45 0
sleep 5.395
noteoff 14 33 0
sleep 68.345
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteon 0 92 101
sleep 1.798
noteon 1 68 100
noteon 1 80 100
noteon 11 69 102
sleep 7.194
noteon 12 57 102
sleep 10.791
noteon 3 56 100
sleep 71.942
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
echo "194160 tempo_s=314 tempo_l=0.25"
sleep 1.592
noteoff 0 92 0
noteon 0 93 101
sleep 1.592
noteoff 1 80 0
noteoff 1 68 0
noteon 1 69 100
noteon 1 81 100
noteon 11 76 102
sleep 6.369
noteon 12 64 102
sleep 9.554
noteoff 3 56 0
noteon 3 57 100
sleep 63.694
noteoff 11 76 0
sleep 6.369
noteoff 12 64 0
sleep 9.554
noteon 11 76 102
sleep 6.369
noteon 12 64 102
sleep 73.248
noteoff 11 76 0
sleep 6.369
noteoff 12 64 0
sleep 7.961
noteoff 0 93 0
sleep 1.592
noteoff 1 81 0
noteoff 1 69 0
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 9.554
noteoff 3 57 0
sleep 63.694
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 73 102
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 70.063
noteoff 10 73 0
sleep 3.184
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
echo "194400 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 3.597
noteon 4 64 100
noteon 11 67 102
sleep 7.194
noteon 5 57 100
noteon 12 55 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 10.791
noteoff 10 74 0
noteon 10 73 102
sleep 35.971
noteoff 10 73 0
noteon 10 74 102
sleep 21.582
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 7.194
noteoff 10 74 0
noteon 10 73 102
sleep 3.597
noteon 11 67 102
sleep 7.194
noteon 12 55 102
sleep 25.179
noteoff 10 73 0
noteon 10 74 102
sleep 35.971
noteoff 10 74 0
noteon 10 73 102
sleep 21.582
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 7.194
noteoff 10 73 0
noteon 10 74 102
sleep 3.597
noteon 11 76 102
sleep 7.194
noteon 12 64 102
sleep 8.992
noteoff 13 45 0
sleep 5.395
noteoff 14 33 0
sleep 10.791
noteoff 10 74 0
noteon 10 73 102
sleep 35.971
noteoff 10 73 0
noteon 10 74 102
sleep 21.582
noteoff 11 76 0
sleep 7.194
noteoff 12 64 0
sleep 7.194
noteoff 10 74 0
noteon 10 71 102
sleep 3.597
noteon 11 76 102
sleep 7.194
noteon 12 64 102
sleep 43.165
noteoff 10 71 0
noteon 10 73 102
sleep 39.568
noteoff 11 76 0
sleep 7.194
noteoff 12 64 0
sleep 7.194
echo "194640 tempo_s=314 tempo_l=0.25"
noteoff 10 73 0
noteon 10 76 102
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.248
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteoff 10 76 0
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.248
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 69 102
sleep 3.184
noteon 11 67 102
sleep 6.369
noteon 12 55 102
sleep 73.248
noteoff 11 67 0
sleep 6.369
noteoff 12 55 0
sleep 6.369
noteoff 10 69 0
sleep 3.184
noteon 11 67 102
sleep 6.369
noteon 12 55 102
sleep 73.248
noteoff 11 67 0
sleep 6.369
noteoff 12 55 0
sleep 6.369
echo "194880 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 3.597
noteoff 4 64 0
noteon 4 62 100
noteon 11 66 102
sleep 7.194
noteoff 5 57 0
noteon 5 54 100
noteon 12 54 102
sleep 8.992
noteon 13 50 104
sleep 5.395
noteon 14 38 106
sleep 68.345
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 10.791
noteon 11 66 102
sleep 7.194
noteon 12 54 102
sleep 82.733
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
noteoff 10 74 0
sleep 3.597
noteoff 4 62 0
noteon 11 69 102
sleep 7.194
noteoff 5 54 0
noteon 12 57 102
sleep 8.992
noteoff 13 50 0
sleep 5.395
noteoff 14 38 0
sleep 68.345
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteon 0 92 101
sleep 1.798
noteon 1 68 100
noteon 1 80 100
noteon 11 69 102
sleep 7.194
noteon 12 57 102
sleep 10.791
noteon 3 56 100
sleep 71.942
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
echo "195120 tempo_s=314 tempo_l=0.25"
sleep 1.592
noteoff 0 92 0
noteon 0 93 101
sleep 1.592
noteoff 1 80 0
noteoff 1 68 0
noteon 1 81 100
noteon 1 69 100
noteon 11 74 102
sleep 6.369
noteon 12 62 102
sleep 9.554
noteoff 3 56 0
noteon 3 57 100
sleep 63.694
noteoff 11 74 0
sleep 6.369
noteoff 12 62 0
sleep 9.554
noteon 11 74 102
sleep 6.369
noteon 12 62 102
sleep 73.248
noteoff 11 74 0
sleep 6.369
noteoff 12 62 0
sleep 7.961
noteoff 0 93 0
sleep 1.592
noteoff 1 69 0
noteoff 1 81 0
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 9.554
noteoff 3 57 0
sleep 63.694
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 78 102
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 70.063
noteoff 10 78 0
sleep 3.184
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
echo "195360 tempo_s=278 tempo_l=0.25"
noteon 10 79 102
sleep 3.597
noteon 4 62 100
noteon 11 66 102
sleep 7.194
noteon 5 54 100
noteon 12 54 102
sleep 8.992
noteon 13 50 104
sleep 5.395
noteon 14 38 106
sleep 10.791
noteoff 10 79 0
noteon 10 78 102
sleep 35.971
noteoff 10 78 0
noteon 10 79 102
sleep 21.582
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
noteoff 10 79 0
noteon 10 78 102
sleep 3.597
noteon 11 66 102
sleep 7.194
noteon 12 54 102
sleep 25.179
noteoff 10 78 0
noteon 10 79 102
sleep 35.971
noteoff 10 79 0
noteon 10 78 102
sleep 21.582
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
noteoff 10 78 0
noteon 10 79 102
sleep 3.597
noteon 11 69 102
sleep 7.194
noteon 12 57 102
sleep 8.992
noteoff 13 50 0
sleep 5.395
noteoff 14 38 0
sleep 10.791
noteoff 10 79 0
noteon 10 78 102
sleep 35.971
noteoff 10 78 0
noteon 10 79 102
sleep 21.582
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
noteoff 10 79 0
noteon 10 76 102
sleep 3.597
noteon 11 69 102
sleep 7.194
noteon 12 57 102
sleep 43.165
noteoff 10 76 0
noteon 10 78 102
sleep 39.568
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
echo "195600 tempo_s=314 tempo_l=0.25"
noteoff 10 78 0
noteon 10 81 102
sleep 3.184
noteon 11 74 102
sleep 6.369
noteon 12 62 102
sleep 73.248
noteoff 11 74 0
sleep 6.369
noteoff 12 62 0
sleep 6.369
noteoff 10 81 0
sleep 3.184
noteon 11 74 102
sleep 6.369
noteon 12 62 102
sleep 73.248
noteoff 11 74 0
sleep 6.369
noteoff 12 62 0
sleep 6.369
noteon 10 74 102
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.248
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteoff 10 74 0
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.248
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
echo "195840 tempo_s=278 tempo_l=0.25"
noteon 10 73 102
sleep 3.597
noteoff 4 62 0
noteon 4 64 100
noteon 11 67 102
sleep 7.194
noteoff 5 54 0
noteon 5 57 100
noteon 12 55 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 68.345
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 10.791
noteon 11 67 102
sleep 7.194
noteon 12 55 102
sleep 82.733
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 7.194
noteoff 10 73 0
sleep 3.597
noteoff 4 64 0
noteon 11 69 102
sleep 7.194
noteoff 5 57 0
noteon 12 57 102
sleep 8.992
noteoff 13 45 0
sleep 5.395
noteoff 14 33 0
sleep 68.345
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 8.992
noteon 0 92 101
sleep 1.798
noteon 1 68 100
noteon 1 80 100
noteon 11 69 102
sleep 7.194
noteon 12 57 102
sleep 10.791
noteon 3 56 100
sleep 71.942
noteoff 11 69 0
sleep 7.194
noteoff 12 57 0
sleep 7.194
echo "196080 tempo_s=314 tempo_l=0.25"
sleep 1.592
noteoff 0 92 0
noteon 0 93 101
sleep 1.592
noteoff 1 80 0
noteoff 1 68 0
noteon 1 69 100
noteon 1 81 100
noteon 11 76 102
sleep 6.369
noteon 12 64 102
sleep 9.554
noteoff 3 56 0
noteon 3 57 100
sleep 63.694
noteoff 11 76 0
sleep 6.369
noteoff 12 64 0
sleep 9.554
noteon 11 76 102
sleep 6.369
noteon 12 64 102
sleep 73.248
noteoff 11 76 0
sleep 6.369
noteoff 12 64 0
sleep 7.961
noteoff 0 93 0
sleep 1.592
noteoff 1 81 0
noteoff 1 69 0
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 9.554
noteoff 3 57 0
sleep 63.694
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 73 102
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 70.063
noteoff 10 73 0
sleep 3.184
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
echo "196320 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 3.597
noteon 4 64 100
noteon 11 67 102
sleep 7.194
noteon 5 57 100
noteon 12 55 102
sleep 8.992
noteon 13 45 104
sleep 5.395
noteon 14 33 106
sleep 10.791
noteoff 10 74 0
noteon 10 73 102
sleep 35.971
noteoff 10 73 0
noteon 10 74 102
sleep 21.582
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 7.194
noteoff 10 74 0
noteon 10 73 102
sleep 3.597
noteon 11 67 102
sleep 7.194
noteon 12 55 102
sleep 25.179
noteoff 10 73 0
noteon 10 74 102
sleep 35.971
noteoff 10 74 0
noteon 10 73 102
sleep 21.582
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 7.194
noteoff 10 73 0
noteon 10 74 102
sleep 3.597
noteon 11 76 102
sleep 7.194
noteon 12 64 102
sleep 8.992
noteoff 13 45 0
sleep 5.395
noteoff 14 33 0
sleep 10.791
noteoff 10 74 0
noteon 10 73 102
sleep 35.971
noteoff 10 73 0
noteon 10 74 102
sleep 21.582
noteoff 11 76 0
sleep 7.194
noteoff 12 64 0
sleep 7.194
noteoff 10 74 0
noteon 10 71 102
sleep 3.597
noteon 11 76 102
sleep 7.194
noteon 12 64 102
sleep 43.165
noteoff 10 71 0
noteon 10 73 102
sleep 39.568
noteoff 11 76 0
sleep 7.194
noteoff 12 64 0
sleep 7.194
echo "196560 tempo_s=314 tempo_l=0.25"
noteoff 10 73 0
noteon 10 76 102
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.248
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteoff 10 76 0
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.248
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 69 102
sleep 3.184
noteon 11 67 102
sleep 6.369
noteon 12 55 102
sleep 73.248
noteoff 11 67 0
sleep 6.369
noteoff 12 55 0
sleep 6.369
noteoff 10 69 0
sleep 1.592
noteon 0 92 101
sleep 1.592
noteon 1 80 100
noteon 1 68 100
noteon 11 67 102
sleep 6.369
noteon 12 55 102
sleep 9.554
noteon 3 56 100
sleep 63.694
noteoff 11 67 0
sleep 6.369
noteoff 12 55 0
sleep 6.369
echo "196800 tempo_s=278 tempo_l=0.25"
noteon 10 74 102
sleep 1.798
noteoff 0 92 0
noteon 0 93 101
sleep 1.798
noteoff 1 68 0
noteoff 1 80 0
noteoff 4 64 0
noteon 1 69 100
noteon 11 66 102
noteon 1 81 100
noteon 4 62 100
sleep 7.194
noteoff 5 57 0
noteon 12 54 102
noteon 5 54 100
sleep 8.992
noteon 13 50 104
sleep 1.798
noteoff 3 56 0
noteon 3 57 100
sleep 3.597
noteon 14 38 106
sleep 68.345
noteoff 11 66 0
sleep 7.193
noteoff 12 54 0
sleep 7.194
noteoff 10 74 0
sleep 3.597
noteon 11 66 102
sleep 7.194
noteon 12 54 102
sleep 82.730
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.193
noteon 10 79 102
sleep 1.798
noteoff 0 93 0
sleep 1.798
noteoff 1 81 0
noteoff 1 69 0
noteoff 4 62 0
noteon 11 74 102
sleep 7.194
noteoff 5 54 0
noteon 12 62 102
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 57 0
sleep 3.597
noteoff 14 38 0
sleep 10.791
noteoff 10 79 0
noteon 10 78 102
sleep 35.969
noteoff 10 78 0
noteon 10 79 102
sleep 21.580
noteoff 11 74 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteoff 10 79 0
noteon 10 78 102
sleep 3.597
noteon 11 74 102
sleep 7.194
noteon 12 62 102
sleep 25.178
noteoff 10 78 0
noteon 10 79 102
sleep 35.968
noteoff 10 79 0
noteon 10 78 102
sleep 21.581
noteoff 11 74 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
echo "197040 tempo_s=314 tempo_l=0.25"
noteoff 10 78 0
noteon 10 81 102
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.244
noteoff 11 69 0
sleep 6.368
noteoff 12 57 0
sleep 6.368
noteoff 10 81 0
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.244
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.369
noteon 10 74 102
sleep 3.184
noteon 11 66 102
sleep 6.369
noteon 12 54 102
sleep 73.245
noteoff 11 66 0
sleep 6.369
noteoff 12 54 0
sleep 6.369
noteoff 10 74 0
sleep 1.592
noteon 0 92 101
sleep 1.592
noteon 1 68 100
noteon 1 80 100
noteon 11 66 102
sleep 6.369
noteon 12 54 102
sleep 9.554
noteon 3 56 100
sleep 63.691
noteoff 11 66 0
sleep 6.369
noteoff 12 54 0
sleep 6.368
echo "197280 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 0 92 0
noteon 0 93 101
sleep 1.798
noteoff 1 80 0
noteoff 1 68 0
noteon 1 81 100
noteon 1 69 100
noteon 4 64 100
noteon 11 67 102
sleep 7.194
noteon 5 57 100
noteon 12 55 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteoff 3 56 0
noteon 3 57 100
sleep 3.597
noteon 14 33 106
sleep 68.342
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 10.790
noteon 11 67 102
sleep 7.194
noteon 12 55 102
sleep 82.729
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 7.194
noteon 10 74 102
sleep 1.798
noteoff 0 93 0
sleep 1.798
noteoff 1 69 0
noteoff 1 81 0
noteoff 4 64 0
noteon 11 76 102
sleep 7.193
noteoff 5 57 0
noteon 12 64 102
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 57 0
sleep 3.597
noteoff 14 33 0
sleep 10.790
noteoff 10 74 0
noteon 10 73 102
sleep 35.969
noteoff 10 73 0
noteon 10 74 102
sleep 21.582
noteoff 11 76 0
sleep 7.194
noteoff 12 64 0
sleep 7.193
noteoff 10 74 0
noteon 10 73 102
sleep 3.597
noteon 11 76 102
sleep 7.194
noteon 12 64 102
sleep 25.178
noteoff 10 73 0
noteon 10 74 102
sleep 35.971
noteoff 10 74 0
noteon 10 73 102
sleep 21.582
noteoff 11 76 0
sleep 7.192
noteoff 12 64 0
sleep 7.193
echo "197520 tempo_s=314 tempo_l=0.25"
noteoff 10 73 0
noteon 10 76 102
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.245
noteoff 11 69 0
sleep 6.368
noteoff 12 57 0
sleep 6.369
noteoff 10 76 0
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.244
noteoff 11 69 0
sleep 6.368
noteoff 12 57 0
sleep 6.369
noteon 10 69 102
sleep 3.184
noteon 11 67 102
sleep 6.369
noteon 12 55 102
sleep 73.243
noteoff 11 67 0
sleep 6.369
noteoff 12 55 0
sleep 6.368
noteoff 10 69 0
sleep 1.592
noteon 0 92 101
sleep 1.592
noteon 1 68 100
noteon 1 80 100
noteon 11 67 102
sleep 6.369
noteon 12 55 102
sleep 9.553
noteon 3 56 100
sleep 63.691
noteoff 11 67 0
sleep 6.368
noteoff 12 55 0
sleep 6.369
echo "197760 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 0 92 0
noteon 0 93 101
sleep 1.798
noteoff 1 80 0
noteoff 1 68 0
noteon 1 81 100
noteon 1 69 100
noteon 4 62 100
noteon 11 66 102
sleep 7.194
noteon 5 54 100
noteon 12 54 102
sleep 8.992
noteon 13 50 104
sleep 1.798
noteoff 3 56 0
noteon 3 57 100
sleep 3.597
noteon 14 38 106
sleep 68.341
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 10.791
noteon 11 66 102
sleep 7.194
noteon 12 54 102
sleep 82.729
noteoff 11 66 0
sleep 7.194
noteoff 12 54 0
sleep 7.194
noteon 10 79 102
sleep 1.798
noteoff 0 93 0
sleep 1.798
noteoff 1 69 0
noteoff 1 81 0
noteoff 4 62 0
noteon 11 74 102
sleep 7.193
noteoff 5 54 0
noteon 12 62 102
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 57 0
sleep 3.597
noteoff 14 38 0
sleep 10.789
noteoff 10 79 0
noteon 10 78 102
sleep 35.969
noteoff 10 78 0
noteon 10 79 102
sleep 21.582
noteoff 11 74 0
sleep 7.193
noteoff 12 62 0
sleep 7.194
noteoff 10 79 0
noteon 10 78 102
sleep 3.597
noteon 11 74 102
sleep 7.193
noteon 12 62 102
sleep 25.178
noteoff 10 78 0
noteon 10 79 102
sleep 35.970
noteoff 10 79 0
noteon 10 78 102
sleep 21.581
noteoff 11 74 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
echo "198000 tempo_s=314 tempo_l=0.25"
noteoff 10 78 0
noteon 10 81 102
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.243
noteoff 11 69 0
sleep 6.368
noteoff 12 57 0
sleep 6.369
noteoff 10 81 0
sleep 3.184
noteon 11 69 102
sleep 6.369
noteon 12 57 102
sleep 73.244
noteoff 11 69 0
sleep 6.369
noteoff 12 57 0
sleep 6.368
noteon 10 74 102
sleep 3.184
noteon 11 66 102
sleep 6.369
noteon 12 54 102
sleep 73.245
noteoff 11 66 0
sleep 6.369
noteoff 12 54 0
sleep 6.369
noteoff 10 74 0
sleep 1.592
noteon 0 92 101
sleep 1.592
noteon 1 68 100
noteon 1 80 100
noteon 11 66 102
sleep 6.369
noteon 12 54 102
sleep 9.554
noteon 3 56 100
sleep 63.689
noteoff 11 66 0
sleep 6.369
noteoff 12 54 0
sleep 6.368
echo "198240 tempo_s=278 tempo_l=0.25"
sleep 1.798
noteoff 0 92 0
noteon 0 93 101
sleep 1.798
noteoff 1 80 0
noteoff 1 68 0
noteon 1 69 100
noteon 1 81 100
noteon 4 64 100
noteon 11 67 102
sleep 7.194
noteon 5 57 100
noteon 12 55 102
sleep 8.992
noteon 13 45 104
sleep 1.798
noteoff 3 56 0
noteon 3 57 100
sleep 3.597
noteon 14 33 106
sleep 68.341
noteoff 11 67 0
sleep 7.193
noteoff 12 55 0
sleep 10.790
noteon 11 67 102
sleep 7.193
noteon 12 55 102
sleep 82.730
noteoff 11 67 0
sleep 7.194
noteoff 12 55 0
sleep 7.193
noteon 10 74 102
sleep 1.798
noteoff 0 93 0
sleep 1.798
noteoff 1 81 0
noteoff 1 69 0
noteoff 4 64 0
noteon 11 76 102
sleep 7.194
noteoff 5 57 0
noteon 12 64 102
sleep 8.992
noteoff 13 45 0
sleep 1.798
noteoff 3 57 0
sleep 3.597
noteoff 14 33 0
sleep 10.791
noteoff 10 74 0
noteon 10 73 102
sleep 35.969
noteoff 10 73 0
noteon 10 74 102
sleep 21.581
noteoff 11 76 0
sleep 7.194
noteoff 12 64 0
sleep 7.194
noteoff 10 74 0
noteon 10 73 102
sleep 3.597
noteon 11 76 102
sleep 7.194
noteon 12 64 102
sleep 25.179
noteoff 10 73 0
noteon 10 74 102
sleep 35.967
noteoff 10 74 0
noteon 10 73 102
sleep 21.582
noteoff 11 76 0
sleep 7.194
noteoff 12 64 0
sleep 7.194
echo "198480 tempo_s=293 tempo_l=0.25"
noteoff 10 73 0
noteon 10 76 102
sleep 3.412
noteon 11 69 102
sleep 6.825
noteon 12 57 102
sleep 78.492
noteoff 11 69 0
sleep 6.824
noteoff 12 57 0
sleep 6.825
noteoff 10 76 0
sleep 3.412
noteon 11 69 102
sleep 6.825
noteon 12 57 102
sleep 78.491
noteoff 11 69 0
sleep 6.824
noteoff 12 57 0
sleep 6.825
noteon 10 69 102
sleep 3.412
noteon 11 67 102
sleep 6.825
noteon 12 55 102
sleep 78.492
noteoff 11 67 0
sleep 6.825
noteoff 12 55 0
sleep 6.825
noteoff 10 69 0
sleep 3.412
noteon 11 67 102
sleep 6.825
noteon 12 55 102
sleep 78.493
noteoff 11 67 0
sleep 6.825
noteoff 12 55 0
sleep 6.825
echo "198720 tempo_s=241 tempo_l=0.25"
noteon 10 74 102
sleep 2.074
noteon 0 78 101
noteon 0 86 101
sleep 2.074
noteon 1 74 100
noteon 4 62 100
sleep 8.298
noteon 5 54 100
noteon 12 54 102
sleep 10.373
noteon 13 50 104
sleep 2.074
noteon 3 62 100
noteon 3 50 100
sleep 4.149
noteon 14 38 106
sleep 219.917
noteoff 10 74 0
sleep 2.074
noteoff 0 86 0
noteoff 0 78 0
sleep 2.074
noteoff 1 74 0
noteoff 4 62 0
sleep 8.298
noteoff 5 54 0
noteoff 12 54 0
sleep 10.373
noteoff 13 50 0
sleep 2.074
noteoff 3 50 0
noteoff 3 62 0
sleep 4.149
noteoff 14 38 0
sleep 219.917
echo "198960 tempo_s=187 tempo_l=0.25"
noteon 10 66 102
sleep 2.673
noteon 0 78 101
sleep 2.673
noteon 1 66 100
noteon 4 66 120
noteon 11 66 102
noteon 1 78 100
sleep 2.673
noteon 6 66 127
sleep 5.347
noteon 2 66 101
sleep 2.673
noteon 5 54 120
noteon 12 54 102
sleep 13.368
noteon 13 42 104
sleep 2.673
noteon 3 54 100
noteon 3 42 100
sleep 5.347
noteon 14 30 106
sleep 1780.665
echo "199640 tempo_s=17 tempo_l=0.25"
noteoff 10 66 0
noteoff 12 54 0
noteoff 13 42 0
noteoff 14 30 0
sleep 29.411
noteoff 0 78 0
sleep 29.411
noteoff 1 78 0
noteoff 1 66 0
noteoff 3 42 0
noteoff 3 54 0
noteoff 4 66 0
noteoff 11 66 0
sleep 29.411
noteoff 6 66 0
sleep 58.823
noteoff 2 66 0
sleep 29.411
noteoff 5 54 0
sleep 999.996
echo "199680 tempo_s=263 tempo_l=0.25"
noteon 10 66 52
sleep 3.802
noteon 4 66 100
noteon 11 61 52
sleep 7.604
noteon 5 54 100
noteon 12 58 52
sleep 9.505
noteon 13 54 54
sleep 5.703
noteon 14 42 56
sleep 155.882
noteoff 10 66 0
sleep 3.802
noteoff 11 61 0
sleep 7.604
noteoff 12 58 0
sleep 9.505
noteoff 13 54 0
sleep 5.703
noteoff 14 42 0
sleep 19.011
noteon 10 66 59
sleep 3.802
noteon 11 61 59
sleep 7.604
noteon 12 58 59
sleep 171.102
noteoff 10 66 0
sleep 3.802
noteoff 11 61 0
sleep 7.604
noteoff 12 58 0
sleep 34.22
echo "199920 tempo_s=301 tempo_l=0.25"
sleep 18.272
noteon 13 52 68
sleep 4.983
noteon 14 40 70
sleep 154.485
noteoff 13 52 0
sleep 4.983
noteoff 14 40 0
sleep 16.611
noteon 10 66 74
sleep 3.322
noteon 11 61 74
sleep 6.644
noteon 12 58 74
sleep 149.501
noteoff 10 66 0
sleep 3.322
noteoff 11 61 0
sleep 6.644
noteoff 12 58 0
sleep 29.9
echo "200160 tempo_s=263 tempo_l=0.25"
sleep 20.912
noteon 13 50 76
sleep 5.703
noteon 14 38 78
sleep 176.806
noteoff 13 50 0
sleep 5.703
noteoff 14 38 0
sleep 19.011
noteon 10 66 74
sleep 3.802
noteon 11 62 74
sleep 7.604
noteon 12 59 74
sleep 171.102
noteoff 10 66 0
sleep 3.802
noteoff 11 62 0
sleep 7.604
noteoff 12 59 0
sleep 34.22
echo "200400 tempo_s=301 tempo_l=0.25"
sleep 18.272
noteon 13 49 76
sleep 4.983
noteon 14 37 78
sleep 154.485
noteoff 13 49 0
sleep 4.983
noteoff 14 37 0
sleep 16.611
noteon 10 66 74
sleep 3.322
noteon 11 64 74
sleep 6.644
noteon 12 58 74
sleep 149.501
noteoff 10 66 0
sleep 3.322
noteoff 11 64 0
sleep 6.644
noteoff 12 58 0
sleep 29.9
echo "200640 tempo_s=263 tempo_l=0.25"
noteon 10 66 74
sleep 11.406
noteon 12 59 74
sleep 9.505
noteon 13 47 76
sleep 5.703
noteon 14 35 78
sleep 155.893
noteoff 10 66 0
sleep 11.406
noteoff 12 59 0
sleep 9.505
noteoff 13 47 0
sleep 5.703
noteoff 14 35 0
sleep 22.813
noteoff 4 66 0
noteon 11 62 74
sleep 7.604
noteoff 5 54 0
sleep 102.661
noteon 10 77 87
sleep 72.243
noteoff 11 62 0
sleep 41.825
echo "200880 tempo_s=301 tempo_l=0.25"
noteoff 10 77 0
noteon 10 78 87
sleep 199.335
noteoff 10 78 0
sleep 199.335
echo "201120 tempo_s=263 tempo_l=0.25"
sleep 912.547
noteon 10 67 72
sleep 3.802
noteon 1 79 100
noteon 11 62 72
sleep 7.604
noteon 12 59 72
sleep 9.505
noteon 13 55 74
sleep 5.703
noteon 14 43 76
sleep 155.893
noteoff 10 67 0
sleep 3.802
noteoff 11 62 0
sleep 7.604
noteoff 12 59 0
sleep 9.505
noteoff 13 55 0
sleep 5.703
noteoff 14 43 0
sleep 19.011
noteon 10 67 82
sleep 3.802
noteon 11 62 82
sleep 7.604
noteon 12 59 82
sleep 171.101
noteoff 10 67 0
sleep 3.802
noteoff 11 62 0
sleep 7.604
noteoff 12 59 0
sleep 34.22
echo "201840 tempo_s=301 tempo_l=0.25"
sleep 18.272
noteon 13 54 94
sleep 4.983
noteon 14 42 96
sleep 154.484
noteoff 13 54 0
sleep 4.983
noteoff 14 42 0
sleep 16.611
noteon 10 67 102
sleep 3.322
noteon 11 62 102
sleep 6.644
noteon 12 59 102
sleep 149.500
noteoff 10 67 0
sleep 3.322
noteoff 11 62 0
sleep 6.644
noteoff 12 59 0
sleep 29.9
echo "202080 tempo_s=263 tempo_l=0.25"
sleep 20.912
noteon 13 52 104
sleep 5.703
noteon 14 40 106
sleep 176.805
noteoff 13 52 0
sleep 5.703
noteoff 14 40 0
sleep 19.011
noteon 10 67 102
sleep 3.802
noteon 11 61 102
sleep 7.604
noteon 12 58 102
sleep 171.101
noteoff 10 67 0
sleep 3.802
noteoff 11 61 0
sleep 7.604
noteoff 12 58 0
sleep 34.22
echo "202320 tempo_s=301 tempo_l=0.25"
sleep 18.272
noteon 13 50 104
sleep 4.983
noteon 14 38 106
sleep 154.484
noteoff 13 50 0
sleep 4.983
noteoff 14 38 0
sleep 16.611
noteon 10 67 102
sleep 3.322
noteon 11 62 102
sleep 6.644
noteon 12 58 102
sleep 149.500
noteoff 10 67 0
sleep 3.322
noteoff 11 62 0
sleep 6.644
noteoff 12 58 0
sleep 29.9
echo "202560 tempo_s=263 tempo_l=0.25"
noteon 10 67 102
sleep 3.802
noteon 11 64 102
sleep 7.604
noteon 12 57 102
sleep 9.505
noteon 13 49 104
sleep 5.703
noteon 14 37 106
sleep 155.893
noteoff 10 67 0
sleep 3.802
noteoff 11 64 0
sleep 7.604
noteoff 12 57 0
sleep 9.505
noteoff 13 49 0
sleep 5.703
noteoff 14 37 0
sleep 22.813
noteoff 1 79 0
sleep 110.266
noteon 10 78 87
sleep 114.068
echo "202800 tempo_s=301 tempo_l=0.25"
noteoff 10 78 0
noteon 10 79 87
sleep 199.335
noteoff 10 79 0
sleep 199.335
echo "203040 tempo_s=248 tempo_l=0.25"
sleep 483.87
echo "203280 tempo_s=310 tempo_l=0.25"
sleep 290.322
noteon 10 73 102
sleep 1.612
noteon 0 85 101
sleep 1.612
noteon 1 73 100
noteon 11 61 102
sleep 4.838
noteon 2 73 101
noteon 2 61 101
sleep 1.612
noteon 12 61 102
sleep 8.064
noteon 13 49 104
sleep 1.612
noteon 3 61 100
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 74.193
echo "203520 tempo_s=278 tempo_l=0.25"
noteoff 10 73 0
noteon 10 62 102
noteon 10 74 102
sleep 1.798
noteoff 0 85 0
noteon 0 86 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteoff 2 61 0
noteoff 2 73 0
noteon 2 62 101
noteon 2 74 101
sleep 1.798
noteoff 12 61 0
noteon 5 50 100
noteon 12 62 102
sleep 8.992
noteoff 13 49 0
noteon 13 50 104
sleep 1.798
noteoff 3 49 0
noteoff 3 61 0
noteon 3 50 100
noteon 3 62 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteoff 14 37 0
noteon 14 38 106
sleep 82.733
noteoff 10 74 0
noteoff 10 62 0
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteoff 1 74 0
noteoff 11 62 0
sleep 5.395
noteoff 2 74 0
noteoff 2 62 0
sleep 1.798
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 62 0
noteoff 3 50 0
sleep 3.597
noteoff 14 38 0
sleep 82.733
noteon 10 66 102
sleep 3.597
noteoff 4 62 0
noteon 11 66 102
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 5.395
noteoff 5 50 0
sleep 12.589
noteoff 15 50 0
sleep 66.546
noteoff 10 66 0
sleep 3.597
noteoff 11 66 0
sleep 14.388
noteon 10 67 102
sleep 3.597
noteon 11 67 102
sleep 32.374
noteoff 10 67 0
noteon 10 66 102
sleep 21.582
noteoff 11 67 0
noteon 11 66 102
sleep 14.388
noteoff 10 66 0
noteon 10 67 102
sleep 35.971
echo "203760 tempo_s=310 tempo_l=0.25"
noteoff 10 67 0
noteon 10 66 102
sleep 3.225
noteoff 11 66 0
noteon 11 67 102
sleep 29.032
noteoff 10 66 0
noteon 10 67 102
sleep 19.354
noteoff 11 67 0
noteon 11 66 102
sleep 12.903
noteoff 10 67 0
noteon 10 66 102
sleep 32.258
noteoff 10 66 0
noteon 10 67 102
sleep 3.225
noteoff 11 66 0
noteon 11 67 102
sleep 29.032
noteoff 10 67 0
noteon 10 66 102
sleep 19.354
noteoff 11 67 0
noteon 11 66 102
sleep 12.903
noteoff 10 66 0
noteon 10 67 102
sleep 32.258
noteoff 10 67 0
noteon 10 66 102
sleep 3.225
noteoff 11 66 0
noteon 11 67 102
sleep 29.032
noteoff 10 66 0
noteon 10 67 102
sleep 19.354
noteoff 11 67 0
noteon 11 66 102
sleep 12.903
noteoff 10 67 0
noteon 10 66 102
sleep 32.258
noteoff 10 66 0
noteon 10 64 102
sleep 3.225
noteoff 11 66 0
noteon 11 64 102
sleep 45.161
noteoff 10 64 0
noteon 10 66 102
sleep 3.225
noteoff 11 64 0
noteon 11 66 102
sleep 45.161
echo "204000 tempo_s=278 tempo_l=0.25"
noteoff 10 66 0
noteon 10 69 102
sleep 3.597
noteoff 11 66 0
noteon 11 69 102
sleep 104.316
noteoff 10 69 0
sleep 3.597
noteoff 11 69 0
sleep 104.316
noteon 10 62 102
sleep 3.597
noteon 11 62 102
sleep 104.316
noteoff 10 62 0
sleep 3.597
noteoff 11 62 0
sleep 104.316
echo "204240 tempo_s=310 tempo_l=0.25"
sleep 290.322
noteon 10 73 102
sleep 1.612
noteon 0 85 101
sleep 1.612
noteon 1 73 100
noteon 11 61 102
sleep 4.838
noteon 2 73 101
noteon 2 61 101
sleep 1.612
noteon 12 61 102
sleep 8.064
noteon 13 49 104
sleep 1.612
noteon 3 61 100
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 74.193
echo "204480 tempo_s=278 tempo_l=0.25"
noteoff 10 73 0
noteon 10 62 102
noteon 10 74 102
sleep 1.798
noteoff 0 85 0
noteon 0 86 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
sleep 1.798
noteon 6 62 108
noteon 6 74 108
sleep 3.597
noteoff 2 61 0
noteoff 2 73 0
noteon 2 62 101
noteon 2 74 101
sleep 1.798
noteoff 12 61 0
noteon 5 50 100
noteon 12 62 102
sleep 8.992
noteoff 13 49 0
noteon 13 50 104
sleep 1.798
noteoff 3 49 0
noteoff 3 61 0
noteon 3 62 100
noteon 3 50 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteoff 14 37 0
noteon 14 38 106
sleep 82.733
noteoff 10 74 0
noteoff 10 62 0
sleep 1.798
noteoff 0 86 0
sleep 1.798
noteoff 1 74 0
noteoff 11 62 0
sleep 5.395
noteoff 2 74 0
noteoff 2 62 0
sleep 1.798
noteoff 12 62 0
sleep 8.992
noteoff 13 50 0
sleep 1.798
noteoff 3 50 0
noteoff 3 62 0
sleep 3.597
noteoff 14 38 0
sleep 82.733
noteon 10 69 102
sleep 3.597
noteoff 4 62 0
noteon 11 69 102
sleep 1.798
noteoff 6 74 0
noteoff 6 62 0
sleep 5.395
noteoff 5 50 0
sleep 12.589
noteoff 15 50 0
sleep 66.546
noteoff 10 69 0
sleep 3.597
noteoff 11 69 0
sleep 14.388
noteon 10 71 102
sleep 3.597
noteon 11 71 102
sleep 32.374
noteoff 10 71 0
noteon 10 69 102
sleep 21.582
noteoff 11 71 0
noteon 11 69 102
sleep 14.388
noteoff 10 69 0
noteon 10 71 102
sleep 35.971
echo "204720 tempo_s=310 tempo_l=0.25"
noteoff 10 71 0
noteon 10 69 102
sleep 3.225
noteoff 11 69 0
noteon 11 71 102
sleep 29.032
noteoff 10 69 0
noteon 10 71 102
sleep 19.354
noteoff 11 71 0
noteon 11 69 102
sleep 12.903
noteoff 10 71 0
noteon 10 69 102
sleep 32.258
noteoff 10 69 0
noteon 10 71 102
sleep 3.225
noteoff 11 69 0
noteon 11 71 102
sleep 29.032
noteoff 10 71 0
noteon 10 69 102
sleep 19.354
noteoff 11 71 0
noteon 11 69 102
sleep 12.903
noteoff 10 69 0
noteon 10 71 102
sleep 32.258
noteoff 10 71 0
noteon 10 69 102
sleep 3.225
noteoff 11 69 0
noteon 11 71 102
sleep 29.032
noteoff 10 69 0
noteon 10 71 102
sleep 19.354
noteoff 11 71 0
noteon 11 69 102
sleep 12.903
noteoff 10 71 0
noteon 10 69 102
sleep 32.258
noteoff 10 69 0
noteon 10 68 102
sleep 3.225
noteoff 11 69 0
noteon 11 68 102
sleep 45.161
noteoff 10 68 0
noteon 10 69 102
sleep 3.225
noteoff 11 68 0
noteon 11 69 102
sleep 45.161
echo "204960 tempo_s=278 tempo_l=0.25"
noteoff 10 69 0
noteon 10 74 102
sleep 3.597
noteoff 11 69 0
noteon 11 74 102
sleep 104.316
noteoff 10 74 0
sleep 3.597
noteoff 11 74 0
sleep 104.316
noteon 10 66 102
sleep 3.597
noteon 11 66 102
sleep 104.316
noteoff 10 66 0
sleep 3.597
noteoff 11 66 0
sleep 104.316
echo "205200 tempo_s=310 tempo_l=0.25"
sleep 290.322
noteon 10 73 102
sleep 1.612
noteon 0 85 101
sleep 1.612
noteon 1 73 100
noteon 11 61 102
sleep 4.838
noteon 2 61 101
noteon 2 73 101
sleep 1.612
noteon 12 61 102
sleep 8.064
noteon 13 49 104
sleep 1.612
noteon 3 61 100
noteon 3 49 100
sleep 3.225
noteon 14 37 106
sleep 74.193
echo "205440 tempo_s=278 tempo_l=0.25"
noteoff 10 73 0
noteon 10 74 102
noteon 10 62 102
sleep 1.798
noteoff 0 85 0
noteon 0 86 101
sleep 1.798
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
sleep 1.798
noteon 6 74 108
noteon 6 62 108
sleep 3.597
noteoff 2 73 0
noteoff 2 61 0
noteon 2 74 101
noteon 2 62 101
sleep 1.798
noteoff 12 61 0
noteon 5 50 100
noteon 12 62 102
sleep 8.992
noteoff 13 49 0
noteon 13 50 104
sleep 1.798
noteoff 3 49 0
noteoff 3 61 0
noteon 3 50 100
noteon 3 62 100
sleep 1.798
noteon 15 50 90
sleep 1.798
noteoff 14 37 0
noteon 14 38 106
sleep 84.532
noteoff 0 86 0
sleep 1.798
noteoff 1 74 0
sleep 5.395
noteoff 2 62 0
noteoff 2 74 0
sleep 12.589
noteoff 3 62 0
noteoff 3 50 0
sleep 68.345
noteoff 10 62 0
noteoff 10 74 0
sleep 3.597
noteoff 11 62 0
sleep 7.194
noteoff 12 62 0
sleep 7.194
noteon 10 74 102
sleep 1.798
noteoff 13 50 0
sleep 1.798
noteoff 4 62 0
noteon 11 62 102
sleep 1.798
noteoff 6 62 0
noteoff 6 74 0
sleep 1.798
noteoff 14 38 0
sleep 3.597
noteoff 5 50 0
sleep 12.589
noteoff 15 50 0
sleep 84.532
noteoff 10 74 0
sleep 3.597
noteoff 11 62 0
sleep 104.316
echo "205680 tempo_s=310 tempo_l=0.25"
noteon 10 78 102
sleep 3.225
noteon 11 66 102
sleep 93.548
noteoff 10 78 0
sleep 3.225
noteoff 11 66 0
sleep 93.548
noteon 10 69 102
sleep 3.225
noteon 11 57 102
sleep 93.548
noteoff 10 69 0
sleep 3.225
noteoff 11 57 0
sleep 93.548
echo "205920 tempo_s=290 tempo_l=0.25"
sleep 1.724
noteon 0 86 101
sleep 1.724
noteon 1 74 100
noteon 4 62 100
sleep 1.724
noteon 6 74 108
noteon 6 62 108
sleep 3.448
noteon 2 62 101
noteon 2 74 101
sleep 1.724
noteon 5 50 100
noteon 12 62 102
sleep 8.62
noteon 13 50 104
sleep 1.724
noteon 3 50 100
noteon 3 62 100
sleep 1.724
noteon 15 50 90
sleep 1.724
noteon 14 38 106
sleep 182.758
noteon 10 78 102
sleep 1.724
noteoff 0 86 0
sleep 1.724
noteoff 1 74 0
noteoff 4 62 0
noteon 11 66 102
sleep 1.724
noteoff 6 62 0
noteoff 6 74 0
sleep 3.448
noteoff 2 74 0
noteoff 2 62 0
sleep 1.724
noteoff 5 50 0
noteoff 12 62 0
sleep 8.62
noteoff 13 50 0
sleep 1.724
noteoff 3 62 0
noteoff 3 50 0
sleep 1.724
noteoff 15 50 0
sleep 1.724
noteoff 14 38 0
sleep 79.31
noteoff 10 78 0
sleep 3.448
noteoff 11 66 0
sleep 100.0
echo "206160 tempo_s=312 tempo_l=0.25"
noteon 10 81 102
sleep 3.205
noteon 11 69 102
sleep 92.948
noteoff 10 81 0
sleep 3.205
noteoff 11 69 0
sleep 92.948
noteon 10 74 102
sleep 3.205
noteon 11 62 102
sleep 92.948
noteoff 10 74 0
sleep 3.205
noteoff 11 62 0
sleep 92.948
echo "206400 tempo_s=304 tempo_l=0.25"
sleep 1.644
noteon 0 86 101
sleep 1.644
noteon 1 74 100
noteon 4 62 100
sleep 1.644
noteon 6 74 108
noteon 6 62 108
sleep 3.289
noteon 2 74 101
noteon 2 62 101
sleep 1.644
noteon 5 50 100
noteon 12 62 102
sleep 8.223
noteon 13 50 104
sleep 1.644
noteon 3 50 100
noteon 3 62 100
sleep 1.644
noteon 15 50 90
sleep 1.644
noteon 14 38 106
sleep 174.342
noteon 10 81 102
sleep 1.644
noteoff 0 86 0
sleep 1.644
noteoff 1 74 0
noteoff 4 62 0
noteon 11 69 102
sleep 1.644
noteoff 6 62 0
noteoff 6 74 0
sleep 3.289
noteoff 2 62 0
noteoff 2 74 0
sleep 1.644
noteoff 5 50 0
noteoff 12 62 0
sleep 8.223
noteoff 13 50 0
sleep 1.644
noteoff 3 62 0
noteoff 3 50 0
sleep 1.644
noteoff 15 50 0
sleep 1.644
noteoff 14 38 0
sleep 75.657
noteoff 10 81 0
sleep 3.289
noteoff 11 69 0
sleep 95.394
echo "206640 tempo_s=321 tempo_l=0.25"
noteon 10 86 102
sleep 3.115
noteon 11 74 102
sleep 90.342
noteoff 10 86 0
sleep 3.115
noteoff 11 74 0
sleep 90.342
noteon 10 81 102
sleep 1.557
noteon 0 81 101
sleep 1.557
noteon 1 69 100
noteon 4 57 100
noteon 11 69 102
sleep 1.557
noteon 6 57 108
noteon 6 69 108
sleep 3.115
noteon 2 57 101
noteon 2 69 101
sleep 1.557
noteon 5 45 100
noteon 12 57 102
sleep 7.788
noteon 13 45 104
sleep 1.557
noteon 3 57 100
noteon 3 45 100
sleep 1.557
noteon 15 45 90
sleep 1.557
noteon 14 33 106
sleep 71.651
noteoff 10 81 0
sleep 1.557
noteoff 0 81 0
sleep 1.557
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 69 0
sleep 1.557
noteoff 6 69 0
noteoff 6 57 0
sleep 3.115
noteoff 2 69 0
noteoff 2 57 0
sleep 1.557
noteoff 5 45 0
noteoff 12 57 0
sleep 7.788
noteoff 13 45 0
sleep 1.557
noteoff 3 45 0
noteoff 3 57 0
sleep 1.557
noteoff 15 45 0
sleep 1.557
noteoff 14 33 0
sleep 71.651
echo "206880 tempo_s=304 tempo_l=0.25"
noteon 10 86 102
sleep 1.644
noteon 0 86 101
sleep 1.644
noteon 1 74 100
noteon 4 62 100
noteon 11 74 102
sleep 1.644
noteon 6 62 108
noteon 6 74 108
sleep 3.289
noteon 2 62 101
noteon 2 74 101
sleep 1.644
noteon 5 50 100
noteon 12 62 102
sleep 8.223
noteon 13 50 104
sleep 1.644
noteon 3 50 100
noteon 3 62 100
sleep 1.644
noteon 15 50 90
sleep 1.644
noteon 14 38 106
sleep 75.657
noteoff 10 86 0
sleep 1.644
noteoff 0 86 0
sleep 1.644
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 74 0
sleep 1.644
noteoff 6 74 0
noteoff 6 62 0
sleep 3.289
noteoff 2 74 0
noteoff 2 62 0
sleep 1.644
noteoff 5 50 0
noteoff 12 62 0
sleep 8.223
noteoff 13 50 0
sleep 1.644
noteoff 3 62 0
noteoff 3 50 0
sleep 1.644
noteoff 15 50 0
sleep 1.644
noteoff 14 38 0
sleep 75.657
noteon 10 81 102
sleep 1.644
noteon 0 81 101
sleep 1.644
noteon 1 69 100
noteon 4 57 100
noteon 11 69 102
sleep 1.644
noteon 6 69 108
noteon 6 57 108
sleep 3.289
noteon 2 69 101
noteon 2 57 101
sleep 1.644
noteon 5 45 100
noteon 12 57 102
sleep 8.223
noteon 13 45 104
sleep 1.644
noteon 3 57 100
noteon 3 45 100
sleep 1.644
noteon 15 45 90
sleep 1.644
noteon 14 33 106
sleep 75.657
noteoff 10 81 0
sleep 1.644
noteoff 0 81 0
sleep 1.644
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 69 0
sleep 1.644
noteoff 6 57 0
noteoff 6 69 0
sleep 3.289
noteoff 2 57 0
noteoff 2 69 0
sleep 1.644
noteoff 5 45 0
noteoff 12 57 0
sleep 8.223
noteoff 13 45 0
sleep 1.644
noteoff 3 45 0
noteoff 3 57 0
sleep 1.644
noteoff 15 45 0
sleep 1.644
noteoff 14 33 0
sleep 75.657
echo "207120 tempo_s=321 tempo_l=0.25"
noteon 10 86 102
sleep 1.557
noteon 0 86 101
sleep 1.557
noteon 1 74 100
noteon 4 62 100
noteon 11 74 102
sleep 1.557
noteon 6 62 108
noteon 6 74 108
sleep 3.115
noteon 2 74 101
noteon 2 62 101
sleep 1.557
noteon 5 50 100
noteon 12 62 102
sleep 7.788
noteon 13 50 104
sleep 1.557
noteon 3 50 100
noteon 3 62 100
sleep 1.557
noteon 15 50 90
sleep 1.557
noteon 14 38 106
sleep 71.651
noteoff 10 86 0
sleep 1.557
noteoff 0 86 0
sleep 1.557
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 74 0
sleep 1.557
noteoff 6 74 0
noteoff 6 62 0
sleep 3.115
noteoff 2 62 0
noteoff 2 74 0
sleep 1.557
noteoff 5 50 0
noteoff 12 62 0
sleep 7.788
noteoff 13 50 0
sleep 1.557
noteoff 3 62 0
noteoff 3 50 0
sleep 1.557
noteoff 15 50 0
sleep 1.557
noteoff 14 38 0
sleep 71.651
noteon 10 81 102
sleep 1.557
noteon 0 81 101
sleep 1.557
noteon 1 69 100
noteon 4 57 100
noteon 11 69 102
sleep 1.557
noteon 6 57 108
noteon 6 69 108
sleep 3.115
noteon 2 69 101
noteon 2 57 101
sleep 1.557
noteon 5 45 100
noteon 12 57 102
sleep 7.788
noteon 13 45 104
sleep 1.557
noteon 3 45 100
noteon 3 57 100
sleep 1.557
noteon 15 45 90
sleep 1.557
noteon 14 33 106
sleep 71.651
noteoff 10 81 0
sleep 1.557
noteoff 0 81 0
sleep 1.557
noteoff 1 69 0
noteoff 4 57 0
noteoff 11 69 0
sleep 1.557
noteoff 6 69 0
noteoff 6 57 0
sleep 3.115
noteoff 2 57 0
noteoff 2 69 0
sleep 1.557
noteoff 5 45 0
noteoff 12 57 0
sleep 7.788
noteoff 13 45 0
sleep 1.557
noteoff 3 57 0
noteoff 3 45 0
sleep 1.557
noteoff 15 45 0
sleep 1.557
noteoff 14 33 0
sleep 71.651
echo "207360 tempo_s=304 tempo_l=0.25"
noteon 10 86 102
sleep 1.644
noteon 0 86 101
sleep 1.644
noteon 1 74 100
noteon 4 62 100
noteon 11 74 102
sleep 1.644
noteon 6 62 108
noteon 6 74 108
sleep 3.289
noteon 2 62 101
noteon 2 74 101
sleep 1.644
noteon 5 50 100
noteon 12 62 102
sleep 8.223
noteon 13 50 104
sleep 1.644
noteon 3 62 100
noteon 3 50 100
sleep 1.644
noteon 15 50 90
sleep 1.644
noteon 14 38 106
sleep 59.21
noteoff 10 86 0
sleep 3.289
noteoff 11 74 0
sleep 6.578
noteoff 12 62 0
sleep 6.578
noteon 10 86 102
sleep 3.289
noteon 11 74 102
sleep 6.578
noteon 12 62 102
sleep 8.223
noteoff 13 50 0
sleep 4.934
noteoff 14 38 0
sleep 57.565
noteoff 0 86 0
sleep 1.644
noteoff 1 74 0
noteoff 4 62 0
noteoff 10 86 0
sleep 1.644
noteoff 6 74 0
noteoff 6 62 0
sleep 1.644
noteoff 11 74 0
sleep 1.644
noteoff 2 74 0
noteoff 2 62 0
sleep 1.644
noteoff 5 50 0
sleep 3.289
noteoff 12 62 0
sleep 6.578
noteoff 3 50 0
noteoff 3 62 0
noteon 10 81 102
sleep 1.644
noteon 0 85 101
noteon 0 88 101
sleep 1.644
noteon 1 76 100
noteon 1 73 100
noteon 4 64 100
noteon 11 69 102
sleep 1.644
noteon 6 69 108
noteon 6 57 108
sleep 3.289
noteon 2 64 101
noteon 2 73 101
sleep 1.644
noteon 5 57 100
noteon 12 69 102
sleep 8.223
noteon 13 57 104
sleep 1.644
noteon 3 57 100
sleep 1.644
noteoff 15 50 0
noteon 15 45 90
sleep 1.644
noteon 14 45 106
sleep 59.21
noteoff 10 81 0
sleep 3.289
noteoff 11 69 0
sleep 6.578
noteoff 12 69 0
sleep 6.578
noteon 10 81 102
sleep 3.289
noteon 11 69 102
sleep 6.578
noteon 12 69 102
sleep 8.223
noteoff 13 57 0
sleep 4.934
noteoff 14 45 0
sleep 57.565
noteoff 0 88 0
noteoff 0 85 0
sleep 1.644
noteoff 1 73 0
noteoff 1 76 0
noteoff 4 64 0
noteoff 10 81 0
sleep 1.644
noteoff 6 57 0
noteoff 6 69 0
sleep 1.644
noteoff 11 69 0
sleep 1.644
noteoff 2 73 0
noteoff 2 64 0
sleep 1.644
noteoff 5 57 0
sleep 3.289
noteoff 12 69 0
sleep 6.578
echo "207600 tempo_s=321 tempo_l=0.25"
noteoff 3 57 0
noteon 10 86 102
sleep 1.557
noteon 0 86 101
noteon 0 90 101
sleep 1.557
noteon 1 74 100
noteon 1 78 100
noteon 4 66 100
noteon 11 74 102
sleep 1.557
noteon 6 62 108
noteon 6 74 108
sleep 3.115
noteon 2 74 101
noteon 2 66 101
sleep 1.557
noteon 5 62 100
noteon 12 74 102
sleep 7.788
noteon 13 62 104
sleep 1.557
noteon 3 62 100
sleep 1.557
noteoff 15 45 0
noteon 15 50 90
sleep 1.557
noteon 14 50 106
sleep 56.074
noteoff 10 86 0
sleep 3.115
noteoff 11 74 0
sleep 6.23
noteoff 12 74 0
sleep 6.23
noteon 10 86 102
sleep 3.115
noteon 11 74 102
sleep 6.23
noteon 12 74 102
sleep 7.788
noteoff 13 62 0
sleep 4.672
noteoff 14 50 0
sleep 54.517
noteoff 0 90 0
noteoff 0 86 0
sleep 1.557
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
noteoff 10 86 0
sleep 1.557
noteoff 6 74 0
noteoff 6 62 0
sleep 1.557
noteoff 11 74 0
sleep 1.557
noteoff 2 66 0
noteoff 2 74 0
sleep 1.557
noteoff 5 62 0
sleep 3.115
noteoff 12 74 0
sleep 6.23
noteoff 3 62 0
noteon 10 81 102
sleep 1.557
noteon 0 85 101
noteon 0 88 101
sleep 1.557
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
noteon 11 69 102
sleep 1.557
noteon 6 57 108
noteon 6 69 108
sleep 3.115
noteon 2 73 101
noteon 2 64 101
sleep 1.557
noteon 5 57 100
noteon 12 69 102
sleep 7.788
noteon 13 57 104
sleep 1.557
noteon 3 57 100
sleep 1.557
noteoff 15 50 0
noteon 15 45 90
sleep 1.557
noteon 14 45 106
sleep 56.074
noteoff 10 81 0
sleep 3.115
noteoff 11 69 0
sleep 6.23
noteoff 12 69 0
sleep 6.23
noteon 10 81 102
sleep 3.115
noteon 11 69 102
sleep 6.23
noteon 12 69 102
sleep 7.788
noteoff 13 57 0
sleep 4.672
noteoff 14 45 0
sleep 54.517
noteoff 0 88 0
noteoff 0 85 0
sleep 1.557
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 10 81 0
sleep 1.557
noteoff 6 69 0
noteoff 6 57 0
sleep 1.557
noteoff 11 69 0
sleep 1.557
noteoff 2 64 0
noteoff 2 73 0
sleep 1.557
noteoff 5 57 0
sleep 3.115
noteoff 12 69 0
sleep 6.23
echo "207840 tempo_s=304 tempo_l=0.25"
noteoff 3 57 0
noteon 10 86 102
sleep 1.644
noteon 0 90 101
noteon 0 86 101
sleep 1.644
noteon 1 74 100
noteon 1 78 100
noteon 4 66 100
noteon 11 74 102
sleep 1.644
noteon 6 62 108
noteon 6 74 108
sleep 3.289
noteon 2 66 101
noteon 2 74 101
sleep 1.644
noteon 5 62 100
noteon 12 74 102
sleep 8.223
noteon 13 62 104
sleep 1.644
noteon 3 62 100
sleep 1.644
noteoff 15 45 0
noteon 15 50 90
sleep 1.644
noteon 14 50 106
sleep 59.21
noteoff 10 86 0
sleep 3.289
noteoff 11 74 0
sleep 6.578
noteoff 12 74 0
sleep 6.578
noteon 10 86 102
sleep 3.289
noteon 11 74 102
sleep 6.578
noteon 12 74 102
sleep 8.223
noteoff 13 62 0
sleep 4.934
noteoff 14 50 0
sleep 57.565
noteoff 0 86 0
noteoff 0 90 0
sleep 1.644
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
noteoff 10 86 0
sleep 1.644
noteoff 6 74 0
noteoff 6 62 0
sleep 1.644
noteoff 11 74 0
sleep 1.644
noteoff 2 74 0
noteoff 2 66 0
sleep 1.644
noteoff 5 62 0
sleep 3.289
noteoff 12 74 0
sleep 6.578
noteoff 3 62 0
noteon 10 81 102
sleep 1.644
noteon 0 88 101
noteon 0 85 101
sleep 1.644
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
noteon 11 69 102
sleep 1.644
noteon 6 69 108
noteon 6 57 108
sleep 3.289
noteon 2 64 101
noteon 2 73 101
sleep 1.644
noteon 5 57 100
noteon 12 69 102
sleep 8.223
noteon 13 57 104
sleep 1.644
noteon 3 57 100
sleep 1.644
noteoff 15 50 0
noteon 15 45 90
sleep 1.644
noteon 14 45 106
sleep 59.21
noteoff 10 81 0
sleep 3.289
noteoff 11 69 0
sleep 6.578
noteoff 12 69 0
sleep 6.578
noteon 10 81 102
sleep 3.289
noteon 11 69 102
sleep 6.578
noteon 12 69 102
sleep 8.223
noteoff 13 57 0
sleep 4.934
noteoff 14 45 0
sleep 57.565
noteoff 0 85 0
noteoff 0 88 0
sleep 1.644
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 10 81 0
sleep 1.644
noteoff 6 57 0
noteoff 6 69 0
sleep 1.644
noteoff 11 69 0
sleep 1.644
noteoff 2 73 0
noteoff 2 64 0
sleep 1.644
noteoff 5 57 0
sleep 3.289
noteoff 12 69 0
sleep 6.578
echo "208080 tempo_s=321 tempo_l=0.25"
noteoff 3 57 0
noteon 10 86 102
sleep 1.557
noteon 0 86 101
noteon 0 90 101
sleep 1.557
noteon 1 78 100
noteon 1 74 100
noteon 4 66 100
noteon 11 74 102
sleep 1.557
noteon 6 62 108
noteon 6 74 108
sleep 3.115
noteon 2 66 101
noteon 2 74 101
sleep 1.557
noteon 5 62 100
noteon 12 74 102
sleep 7.788
noteon 13 62 104
sleep 1.557
noteon 3 62 100
sleep 1.557
noteoff 15 45 0
noteon 15 50 90
sleep 1.557
noteon 14 50 106
sleep 56.074
noteoff 10 86 0
sleep 3.115
noteoff 11 74 0
sleep 6.23
noteoff 12 74 0
sleep 6.23
noteon 10 86 102
sleep 3.115
noteon 11 74 102
sleep 6.23
noteon 12 74 102
sleep 7.788
noteoff 13 62 0
sleep 4.672
noteoff 14 50 0
sleep 54.517
noteoff 0 90 0
noteoff 0 86 0
sleep 1.557
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 66 0
noteoff 10 86 0
sleep 1.557
noteoff 6 74 0
noteoff 6 62 0
sleep 1.557
noteoff 11 74 0
sleep 1.557
noteoff 2 74 0
noteoff 2 66 0
sleep 1.557
noteoff 5 62 0
sleep 3.115
noteoff 12 74 0
sleep 6.23
noteoff 3 62 0
noteon 10 81 102
sleep 1.557
noteon 0 85 101
noteon 0 88 101
sleep 1.557
noteon 1 73 100
noteon 1 76 100
noteon 4 64 100
noteon 11 69 102
sleep 1.557
noteon 6 57 108
noteon 6 69 108
sleep 3.115
noteon 2 73 101
noteon 2 64 101
sleep 1.557
noteon 5 57 100
noteon 12 69 102
sleep 7.788
noteon 13 57 104
sleep 1.557
noteon 3 57 100
sleep 1.557
noteoff 15 50 0
noteon 15 45 90
sleep 1.557
noteon 14 45 106
sleep 56.074
noteoff 10 81 0
sleep 3.115
noteoff 11 69 0
sleep 6.23
noteoff 12 69 0
sleep 6.23
noteon 10 81 102
sleep 3.115
noteon 11 69 102
sleep 6.23
noteon 12 69 102
sleep 7.788
noteoff 13 57 0
sleep 4.672
noteoff 14 45 0
sleep 54.517
noteoff 0 88 0
noteoff 0 85 0
sleep 1.557
noteoff 1 76 0
noteoff 1 73 0
noteoff 4 64 0
noteoff 10 81 0
sleep 1.557
noteoff 6 69 0
noteoff 6 57 0
sleep 1.557
noteoff 11 69 0
sleep 1.557
noteoff 2 64 0
noteoff 2 73 0
sleep 1.557
noteoff 5 57 0
sleep 3.115
noteoff 12 69 0
sleep 6.23
echo "208320 tempo_s=304 tempo_l=0.25"
noteoff 3 57 0
noteon 10 86 102
sleep 1.644
noteon 0 86 101
noteon 0 90 101
sleep 1.644
noteon 1 78 100
noteon 1 74 100
noteon 4 62 100
noteon 11 74 102
sleep 1.644
noteon 6 62 108
noteon 6 74 108
sleep 3.289
noteon 2 66 101
noteon 2 74 101
sleep 1.644
noteon 5 54 100
noteon 12 74 102
sleep 8.223
noteon 13 62 104
sleep 1.644
noteon 3 62 100
sleep 1.644
noteoff 15 45 0
noteon 15 50 90
sleep 1.644
noteon 14 50 106
sleep 75.657
noteoff 10 86 0
sleep 21.381
noteoff 15 50 0
sleep 77.302
noteon 10 85 102
sleep 1.644
noteoff 0 90 0
noteoff 0 86 0
sleep 1.644
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 62 0
noteoff 11 74 0
sleep 1.644
noteoff 6 74 0
noteoff 6 62 0
sleep 3.289
noteoff 2 74 0
noteoff 2 66 0
sleep 1.644
noteoff 5 54 0
noteoff 12 74 0
sleep 8.223
noteoff 13 62 0
sleep 1.644
noteoff 3 62 0
sleep 3.289
noteoff 14 50 0
sleep 75.657
noteoff 10 85 0
noteon 10 83 102
sleep 98.684
echo "208560 tempo_s=321 tempo_l=0.25"
noteoff 10 83 0
noteon 10 81 102
sleep 93.457
noteoff 10 81 0
noteon 10 79 102
sleep 93.457
noteoff 10 79 0
noteon 10 78 102
sleep 93.457
noteoff 10 78 0
noteon 10 76 102
sleep 93.457
echo "208800 tempo_s=304 tempo_l=0.25"
noteoff 10 76 0
noteon 10 74 102
sleep 1.644
noteon 0 86 101
sleep 1.644
noteon 1 74 100
noteon 1 86 100
noteon 4 62 100
noteon 11 74 102
sleep 1.644
noteon 6 62 108
noteon 6 74 108
sleep 3.289
noteon 2 74 101
sleep 1.644
noteon 5 50 100
noteon 12 74 102
sleep 8.223
noteon 13 62 104
sleep 1.644
noteon 3 62 100
sleep 1.644
noteon 15 50 90
sleep 1.644
noteon 14 50 106
sleep 75.657
noteoff 10 74 0
sleep 1.644
noteoff 0 86 0
sleep 1.644
noteoff 1 86 0
noteoff 1 74 0
noteoff 11 74 0
sleep 4.934
noteoff 2 74 0
sleep 1.644
noteoff 12 74 0
sleep 8.223
noteoff 13 62 0
sleep 1.644
noteoff 3 62 0
sleep 3.289
noteoff 14 50 0
sleep 75.657
noteon 10 73 102
sleep 1.644
noteon 0 85 101
sleep 1.644
noteoff 4 62 0
noteon 1 85 100
noteon 1 73 100
noteon 11 73 102
sleep 1.644
noteoff 6 74 0
noteoff 6 62 0
sleep 3.289
noteon 2 73 101
sleep 1.644
noteoff 5 50 0
noteon 12 73 102
sleep 8.223
noteon 13 61 104
sleep 1.644
noteon 3 61 100
sleep 1.644
noteoff 15 50 0
sleep 1.644
noteon 14 49 106
sleep 75.657
noteoff 10 73 0
noteon 10 71 102
sleep 1.644
noteoff 0 85 0
noteon 0 83 101
sleep 1.644
noteoff 1 73 0
noteoff 1 85 0
noteoff 11 73 0
noteon 1 83 100
noteon 1 71 100
noteon 11 71 102
sleep 4.934
noteoff 2 73 0
noteon 2 71 101
sleep 1.644
noteoff 12 73 0
noteon 12 71 102
sleep 8.223
noteoff 13 61 0
noteon 13 59 104
sleep 1.644
noteoff 3 61 0
noteon 3 59 100
sleep 3.289
noteoff 14 49 0
noteon 14 47 106
sleep 75.657
echo "209040 tempo_s=321 tempo_l=0.25"
noteoff 10 71 0
noteon 10 69 102
sleep 1.557
noteoff 0 83 0
noteon 0 81 101
sleep 1.557
noteoff 1 71 0
noteoff 1 83 0
noteoff 11 71 0
noteon 1 81 100
noteon 1 69 100
noteon 11 69 102
sleep 4.672
noteoff 2 71 0
noteon 2 69 101
sleep 1.557
noteoff 12 71 0
noteon 12 69 102
sleep 7.788
noteoff 13 59 0
noteon 13 57 104
sleep 1.557
noteoff 3 59 0
noteon 3 57 100
sleep 3.115
noteoff 14 47 0
noteon 14 45 106
sleep 71.651
noteoff 10 69 0
noteon 10 67 102
sleep 1.557
noteoff 0 81 0
noteon 0 79 101
sleep 1.557
noteoff 1 69 0
noteoff 1 81 0
noteoff 11 69 0
noteon 1 79 100
noteon 1 67 100
noteon 11 67 102
sleep 4.672
noteoff 2 69 0
noteon 2 67 101
sleep 1.557
noteoff 12 69 0
noteon 12 67 102
sleep 7.788
noteoff 13 57 0
noteon 13 55 104
sleep 1.557
noteoff 3 57 0
noteon 3 55 100
sleep 3.115
noteoff 14 45 0
noteon 14 43 106
sleep 71.651
noteoff 10 67 0
noteon 10 66 102
sleep 1.557
noteoff 0 79 0
noteon 0 78 101
sleep 1.557
noteoff 1 67 0
noteoff 1 79 0
noteoff 11 67 0
noteon 1 78 100
noteon 1 66 100
noteon 11 66 102
sleep 4.672
noteoff 2 67 0
noteon 2 66 101
sleep 1.557
noteoff 12 67 0
noteon 12 66 102
sleep 7.788
noteoff 13 55 0
noteon 13 54 104
sleep 1.557
noteoff 3 55 0
noteon 3 54 100
sleep 3.115
noteoff 14 43 0
noteon 14 42 106
sleep 71.651
noteoff 10 66 0
noteon 10 64 102
sleep 1.557
noteoff 0 78 0
noteon 0 76 101
sleep 1.557
noteoff 1 66 0
noteoff 1 78 0
noteoff 11 66 0
noteon 1 76 100
noteon 1 64 100
noteon 11 64 102
sleep 4.672
noteoff 2 66 0
noteon 2 64 101
sleep 1.557
noteoff 12 66 0
noteon 12 64 102
sleep 7.788
noteoff 13 54 0
noteon 13 52 104
sleep 1.557
noteoff 3 54 0
noteon 3 52 100
sleep 3.115
noteoff 14 42 0
noteon 14 40 106
sleep 56.074
noteoff 10 64 0
sleep 3.115
noteoff 11 64 0
sleep 6.23
noteoff 12 64 0
sleep 6.23
echo "209280 tempo_s=304 tempo_l=0.25"
noteon 10 62 102
sleep 1.644
noteoff 0 76 0
noteoff 13 52 0
noteon 0 74 101
sleep 1.644
noteoff 1 64 0
noteoff 1 76 0
noteon 1 74 100
noteon 1 62 100
noteon 4 62 110
noteon 11 62 102
sleep 1.644
noteon 6 62 118
sleep 1.644
noteoff 14 40 0
sleep 1.644
noteoff 2 64 0
noteon 2 62 101
sleep 1.644
noteon 5 50 110
noteon 12 62 102
sleep 8.223
noteon 13 50 104
sleep 1.644
noteoff 3 52 0
noteon 3 50 100
sleep 1.644
noteon 15 50 90
sleep 1.644
noteon 14 38 106
sleep 50.986
noteoff 15 50 0
sleep 13.157
noteon 15 50 90
sleep 11.513
noteoff 10 62 0
noteon 10 61 102
sleep 1.644
noteoff 0 74 0
noteon 0 73 101
sleep 1.644
noteoff 1 74 0
noteoff 11 62 0
noteon 1 73 100
noteon 11 61 102
sleep 4.934
noteoff 2 62 0
noteon 2 61 101
sleep 1.644
noteoff 12 62 0
noteon 12 61 102
sleep 8.223
noteoff 13 50 0
noteon 13 49 104
sleep 1.644
noteoff 3 50 0
noteon 3 49 100
sleep 3.289
noteoff 14 38 0
noteon 14 37 106
sleep 18.092
noteoff 15 50 0
sleep 13.157
noteon 15 50 90
sleep 44.407
noteoff 10 61 0
noteon 10 62 102
sleep 1.644
noteoff 0 73 0
noteon 0 74 101
sleep 1.644
noteoff 1 73 0
noteoff 1 62 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 4.934
noteoff 2 61 0
noteoff 15 50 0
noteon 2 62 101
sleep 1.644
noteoff 12 61 0
noteon 12 62 102
sleep 8.223
noteoff 13 49 0
noteon 13 50 104
sleep 1.644
noteoff 3 49 0
noteon 3 50 100
sleep 1.644
noteon 15 50 90
sleep 1.644
noteoff 14 37 0
noteon 14 38 106
sleep 50.986
noteoff 15 50 0
sleep 13.157
noteon 15 50 90
sleep 11.513
noteoff 10 62 0
noteon 10 61 102
sleep 1.644
noteoff 0 74 0
noteon 0 73 101
sleep 1.644
noteoff 1 74 0
noteoff 11 62 0
noteon 1 73 100
noteon 11 61 102
sleep 4.934
noteoff 2 62 0
noteon 2 61 101
sleep 1.644
noteoff 12 62 0
noteon 12 61 102
sleep 8.223
noteoff 13 50 0
noteon 13 49 104
sleep 1.644
noteoff 3 50 0
noteon 3 49 100
sleep 3.289
noteoff 14 38 0
noteon 14 37 106
sleep 18.092
noteoff 15 50 0
sleep 13.157
noteon 15 50 90
sleep 31.25
noteoff 4 62 0
sleep 1.644
noteoff 6 62 0
sleep 4.934
noteoff 5 50 0
sleep 6.578
echo "209520 tempo_s=321 tempo_l=0.25"
noteoff 10 61 0
noteon 10 62 102
sleep 1.557
noteoff 0 73 0
noteon 0 74 101
sleep 1.557
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 4 66 110
noteon 11 62 102
sleep 1.557
noteon 6 66 118
sleep 3.115
noteoff 2 61 0
noteoff 15 50 0
noteon 2 62 101
sleep 1.557
noteoff 12 61 0
noteon 5 54 110
noteon 12 62 102
sleep 7.788
noteoff 13 49 0
noteon 13 50 104
sleep 1.557
noteoff 3 49 0
noteon 3 50 100
sleep 1.557
noteon 15 50 90
sleep 1.557
noteoff 14 37 0
noteon 14 38 106
sleep 48.286
noteoff 15 50 0
sleep 12.461
noteon 15 50 90
sleep 10.903
noteoff 10 62 0
noteon 10 61 102
sleep 1.557
noteoff 0 74 0
noteon 0 73 101
sleep 1.557
noteoff 1 74 0
noteoff 11 62 0
noteon 1 73 100
noteon 11 61 102
sleep 4.672
noteoff 2 62 0
noteon 2 61 101
sleep 1.557
noteoff 12 62 0
noteon 12 61 102
sleep 7.788
noteoff 13 50 0
noteon 13 49 104
sleep 1.557
noteoff 3 50 0
noteon 3 49 100
sleep 3.115
noteoff 14 38 0
noteon 14 37 106
sleep 17.133
noteoff 15 50 0
sleep 12.461
noteon 15 50 90
sleep 42.056
noteoff 10 61 0
noteon 10 62 102
sleep 1.557
noteoff 0 73 0
noteon 0 74 101
sleep 1.557
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 4.672
noteoff 2 61 0
noteoff 15 50 0
noteon 2 62 101
sleep 1.557
noteoff 12 61 0
noteon 12 62 102
sleep 7.788
noteoff 13 49 0
noteon 13 50 104
sleep 1.557
noteoff 3 49 0
noteon 3 50 100
sleep 1.557
noteon 15 50 90
sleep 1.557
noteoff 14 37 0
noteon 14 38 106
sleep 48.286
noteoff 15 50 0
sleep 12.461
noteon 15 50 90
sleep 10.903
noteoff 10 62 0
noteon 10 61 102
sleep 1.557
noteoff 0 74 0
noteon 0 73 101
sleep 1.557
noteoff 1 74 0
noteoff 11 62 0
noteon 1 73 100
noteon 11 61 102
sleep 4.672
noteoff 2 62 0
noteon 2 61 101
sleep 1.557
noteoff 12 62 0
noteon 12 61 102
sleep 7.788
noteoff 13 50 0
noteon 13 49 104
sleep 1.557
noteoff 3 50 0
noteon 3 49 100
sleep 3.115
noteoff 14 38 0
noteon 14 37 106
sleep 17.133
noteoff 15 50 0
sleep 12.461
noteon 15 50 90
sleep 26.479
noteoff 10 61 0
sleep 3.115
noteoff 4 66 0
noteoff 11 61 0
sleep 1.557
noteoff 6 66 0
sleep 4.672
noteoff 5 54 0
noteoff 12 61 0
sleep 6.23
echo "209760 tempo_s=304 tempo_l=0.25"
noteon 10 62 102
sleep 1.644
noteoff 0 73 0
noteoff 13 49 0
noteon 0 74 101
sleep 1.644
noteoff 1 73 0
noteon 1 74 100
noteon 4 69 110
noteon 11 62 102
sleep 1.644
noteon 6 69 118
sleep 1.644
noteoff 14 37 0
sleep 1.644
noteoff 2 61 0
noteoff 15 50 0
noteon 2 62 101
sleep 1.644
noteon 5 57 110
noteon 12 62 102
sleep 8.223
noteon 13 50 104
sleep 1.644
noteoff 3 49 0
noteon 3 50 100
sleep 1.644
noteon 15 50 90
sleep 1.644
noteon 14 38 106
sleep 50.986
noteoff 15 50 0
sleep 13.157
noteon 15 50 90
sleep 11.513
noteoff 10 62 0
noteon 10 61 102
sleep 1.644
noteoff 0 74 0
noteon 0 73 101
sleep 1.644
noteoff 1 74 0
noteoff 11 62 0
noteon 1 73 100
noteon 11 61 102
sleep 4.934
noteoff 2 62 0
noteon 2 61 101
sleep 1.644
noteoff 12 62 0
noteon 12 61 102
sleep 8.223
noteoff 13 50 0
noteon 13 49 104
sleep 1.644
noteoff 3 50 0
noteon 3 49 100
sleep 3.289
noteoff 14 38 0
noteon 14 37 106
sleep 18.092
noteoff 15 50 0
sleep 13.157
noteon 15 50 90
sleep 44.407
noteoff 10 61 0
noteon 10 62 102
sleep 1.644
noteoff 0 73 0
noteon 0 74 101
sleep 1.644
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 4.934
noteoff 2 61 0
noteoff 15 50 0
noteon 2 62 101
sleep 1.644
noteoff 12 61 0
noteon 12 62 102
sleep 8.223
noteoff 13 49 0
noteon 13 50 104
sleep 1.644
noteoff 3 49 0
noteon 3 50 100
sleep 1.644
noteon 15 50 90
sleep 1.644
noteoff 14 37 0
noteon 14 38 106
sleep 50.986
noteoff 15 50 0
sleep 13.157
noteon 15 50 90
sleep 11.513
noteoff 10 62 0
noteon 10 61 102
sleep 1.644
noteoff 0 74 0
noteon 0 73 101
sleep 1.644
noteoff 1 74 0
noteoff 11 62 0
noteon 1 73 100
noteon 11 61 102
sleep 4.934
noteoff 2 62 0
noteon 2 61 101
sleep 1.644
noteoff 12 62 0
noteon 12 61 102
sleep 8.223
noteoff 13 50 0
noteon 13 49 104
sleep 1.644
noteoff 3 50 0
noteon 3 49 100
sleep 3.289
noteoff 14 38 0
noteon 14 37 106
sleep 18.092
noteoff 15 50 0
sleep 13.157
noteon 15 50 90
sleep 31.25
noteoff 4 69 0
sleep 1.644
noteoff 6 69 0
sleep 4.934
noteoff 5 57 0
sleep 6.578
echo "210000 tempo_s=321 tempo_l=0.25"
noteoff 10 61 0
noteon 10 62 102
sleep 1.557
noteoff 0 73 0
noteon 0 74 101
sleep 1.557
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 4 66 110
noteon 11 62 102
sleep 1.557
noteon 6 66 118
sleep 3.115
noteoff 2 61 0
noteoff 15 50 0
noteon 2 62 101
sleep 1.557
noteoff 12 61 0
noteon 5 54 110
noteon 12 62 102
sleep 7.788
noteoff 13 49 0
noteon 13 50 104
sleep 1.557
noteoff 3 49 0
noteon 3 50 100
sleep 1.557
noteon 15 50 90
sleep 1.557
noteoff 14 37 0
noteon 14 38 106
sleep 48.286
noteoff 15 50 0
sleep 12.461
noteon 15 50 90
sleep 10.903
noteoff 10 62 0
noteon 10 61 102
sleep 1.557
noteoff 0 74 0
noteon 0 73 101
sleep 1.557
noteoff 1 74 0
noteoff 11 62 0
noteon 1 73 100
noteon 11 61 102
sleep 4.672
noteoff 2 62 0
noteon 2 61 101
sleep 1.557
noteoff 12 62 0
noteon 12 61 102
sleep 7.788
noteoff 13 50 0
noteon 13 49 104
sleep 1.557
noteoff 3 50 0
noteon 3 49 100
sleep 3.115
noteoff 14 38 0
noteon 14 37 106
sleep 17.133
noteoff 15 50 0
sleep 12.461
noteon 15 50 90
sleep 42.056
noteoff 10 61 0
noteon 10 62 102
sleep 1.557
noteoff 0 73 0
noteon 0 74 101
sleep 1.557
noteoff 1 73 0
noteoff 11 61 0
noteon 1 74 100
noteon 11 62 102
sleep 4.672
noteoff 2 61 0
noteoff 15 50 0
noteon 2 62 101
sleep 1.557
noteoff 12 61 0
noteon 12 62 102
sleep 7.788
noteoff 13 49 0
noteon 13 50 104
sleep 1.557
noteoff 3 49 0
noteon 3 50 100
sleep 1.557
noteon 15 50 90
sleep 1.557
noteoff 14 37 0
noteon 14 38 106
sleep 48.286
noteoff 15 50 0
sleep 12.461
noteon 15 50 90
sleep 10.903
noteoff 10 62 0
noteon 10 61 102
sleep 1.557
noteoff 0 74 0
noteon 0 73 101
sleep 1.557
noteoff 1 74 0
noteoff 11 62 0
noteon 1 73 100
noteon 11 61 102
sleep 4.672
noteoff 2 62 0
noteon 2 61 101
sleep 1.557
noteoff 12 62 0
noteon 12 61 102
sleep 7.788
noteoff 13 50 0
noteon 13 49 104
sleep 1.557
noteoff 3 50 0
noteon 3 49 100
sleep 3.115
noteoff 14 38 0
noteon 14 37 106
sleep 17.133
noteoff 15 50 0
sleep 12.461
noteon 15 50 90
sleep 26.479
noteoff 10 61 0
sleep 3.115
noteoff 4 66 0
noteoff 11 61 0
sleep 1.557
noteoff 6 66 0
sleep 4.672
noteoff 5 54 0
noteoff 12 61 0
sleep 6.23
echo "210240 tempo_s=290 tempo_l=0.25"
noteon 10 62 102
sleep 1.724
noteoff 0 73 0
noteoff 13 49 0
noteon 0 74 101
sleep 1.724
noteoff 1 73 0
noteon 1 74 100
noteon 4 62 100
noteon 11 62 102
sleep 1.724
noteon 6 62 108
noteon 6 74 108
sleep 1.724
noteoff 14 37 0
sleep 1.724
noteoff 2 61 0
noteoff 15 50 0
noteon 2 62 101
sleep 1.724
noteon 5 50 100
noteon 12 62 102
sleep 8.62
noteon 13 50 104
sleep 1.724
noteoff 3 49 0
noteon 3 50 100
sleep 1.724
noteon 15 50 90
sleep 1.724
noteon 14 38 106
sleep 182.758
noteoff 10 62 0
sleep 1.724
noteoff 0 74 0
sleep 1.724
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 62 0
sleep 1.724
noteoff 6 74 0
noteoff 6 62 0
sleep 3.448
noteoff 2 62 0
sleep 1.724
noteoff 5 50 0
noteoff 12 62 0
sleep 8.62
noteoff 13 50 0
sleep 1.724
noteoff 3 50 0
sleep 1.724
noteoff 15 50 0
sleep 1.724
noteoff 14 38 0
sleep 182.758
echo "210480 tempo_s=309 tempo_l=0.25"
sleep 388.349
echo "210720 tempo_s=268 tempo_l=0.25"
noteon 10 86 102
noteon 10 78 102
noteon 10 62 102
sleep 1.865
noteon 0 86 101
sleep 1.865
noteon 1 74 100
noteon 1 78 100
noteon 4 66 100
noteon 11 78 102
noteon 11 69 102
noteon 11 62 102
sleep 1.865
noteon 6 66 108
noteon 6 74 108
sleep 3.731
noteon 2 66 101
noteon 2 74 101
sleep 1.865
noteon 5 62 100
noteon 12 66 102
noteon 12 57 102
noteon 12 74 102
sleep 9.328
noteon 13 50 104
sleep 1.865
noteon 3 50 100
sleep 1.865
noteon 15 50 90
sleep 1.865
noteon 14 38 106
sleep 197.761
noteoff 10 62 0
noteoff 10 78 0
noteoff 10 86 0
sleep 1.865
noteoff 0 86 0
sleep 1.865
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
noteoff 11 62 0
noteoff 11 69 0
noteoff 11 78 0
sleep 1.865
noteoff 6 74 0
noteoff 6 66 0
sleep 3.731
noteoff 2 74 0
noteoff 2 66 0
sleep 1.865
noteoff 5 62 0
noteoff 12 74 0
noteoff 12 57 0
noteoff 12 66 0
sleep 9.328
noteoff 13 50 0
sleep 1.865
noteoff 3 50 0
sleep 1.865
noteoff 15 50 0
sleep 1.865
noteoff 14 38 0
sleep 197.761
noteon 10 86 102
noteon 10 78 102
noteon 10 62 102
sleep 1.865
noteon 0 86 101
sleep 1.865
noteon 1 74 100
noteon 1 78 100
noteon 4 66 100
noteon 11 78 102
noteon 11 69 102
noteon 11 62 102
sleep 1.865
noteon 6 66 108
noteon 6 74 108
sleep 3.731
noteon 2 66 101
noteon 2 74 101
sleep 1.865
noteon 5 62 100
noteon 12 66 102
noteon 12 57 102
noteon 12 74 102
sleep 9.328
noteon 13 50 104
sleep 1.865
noteon 3 50 100
sleep 1.865
noteon 15 50 90
sleep 1.865
noteon 14 38 106
sleep 197.761
noteoff 10 62 0
noteoff 10 78 0
noteoff 10 86 0
sleep 1.865
noteoff 0 86 0
sleep 1.865
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
noteoff 11 62 0
noteoff 11 69 0
noteoff 11 78 0
sleep 1.865
noteoff 6 74 0
noteoff 6 66 0
sleep 3.731
noteoff 2 74 0
noteoff 2 66 0
sleep 1.865
noteoff 5 62 0
noteoff 12 74 0
noteoff 12 57 0
noteoff 12 66 0
sleep 9.328
noteoff 13 50 0
sleep 1.865
noteoff 3 50 0
sleep 1.865
noteoff 15 50 0
sleep 1.865
noteoff 14 38 0
sleep 197.761
noteon 10 86 102
noteon 10 78 102
noteon 10 62 102
sleep 1.865
noteon 0 86 101
sleep 1.865
noteon 1 74 100
noteon 1 78 100
noteon 4 66 100
noteon 11 78 102
noteon 11 69 102
noteon 11 62 102
sleep 1.865
noteon 6 66 108
noteon 6 74 108
sleep 3.731
noteon 2 66 101
noteon 2 74 101
sleep 1.865
noteon 5 62 100
noteon 12 74 102
noteon 12 57 102
noteon 12 66 102
sleep 9.328
noteon 13 50 104
sleep 1.865
noteon 3 50 100
sleep 1.865
noteon 15 50 90
sleep 1.865
noteon 14 38 106
sleep 197.761
noteoff 10 62 0
noteoff 10 78 0
noteoff 10 86 0
sleep 1.865
noteoff 0 86 0
sleep 1.865
noteoff 1 78 0
noteoff 1 74 0
noteoff 4 66 0
noteoff 11 62 0
noteoff 11 69 0
noteoff 11 78 0
sleep 1.865
noteoff 6 74 0
noteoff 6 66 0
sleep 3.731
noteoff 2 74 0
noteoff 2 66 0
sleep 1.865
noteoff 5 62 0
noteoff 12 66 0
noteoff 12 57 0
noteoff 12 74 0
sleep 9.328
noteoff 13 50 0
sleep 1.865
noteoff 3 50 0
sleep 1.865
noteoff 15 50 0
sleep 1.865
noteoff 14 38 0
sleep 197.761
echo "211440 tempo_s=241 tempo_l=0.25"
sleep 497.925
echo "211680 tempo_s=231 tempo_l=0.25"
noteon 10 69 102
noteon 10 78 102
noteon 10 62 102
sleep 2.164
noteon 0 86 101
sleep 2.164
noteon 1 78 100
noteon 1 74 100
noteon 4 62 100
noteon 11 74 102
noteon 11 66 102
noteon 11 57 102
sleep 2.164
noteon 6 62 108
noteon 6 74 108
sleep 4.329
noteon 2 66 101
noteon 2 74 101
sleep 2.164
noteon 5 54 100
noteon 12 66 102
noteon 12 50 102
noteon 12 57 102
sleep 10.822
noteon 13 50 104
sleep 2.164
noteon 3 50 100
sleep 2.164
noteon 15 50 90
sleep 2.164
noteon 14 38 106
sleep 229.437
noteoff 10 62 0
noteoff 10 78 0
noteoff 10 69 0
sleep 2.164
noteoff 0 86 0
sleep 2.164
noteoff 1 74 0
noteoff 1 78 0
noteoff 4 62 0
noteoff 11 57 0
noteoff 11 66 0
noteoff 11 74 0
sleep 2.164
noteoff 6 74 0
noteoff 6 62 0
sleep 4.329
noteoff 2 74 0
noteoff 2 66 0
sleep 2.164
noteoff 5 54 0
noteoff 12 57 0
noteoff 12 50 0
noteoff 12 66 0
sleep 10.822
noteoff 13 50 0
sleep 2.164
noteoff 3 50 0
sleep 2.164
noteoff 15 50 0
sleep 2.164
noteoff 14 38 0
sleep 229.437
echo "211920 tempo_s=214 tempo_l=0.25"
sleep 560.747
echo "212160 tempo_s=101 tempo_l=0.25"
noteon 10 62 104
sleep 4.95
noteon 0 74 103
sleep 4.95
noteon 1 74 102
noteon 4 62 102
noteon 11 62 104
sleep 4.95
noteon 6 62 110
sleep 9.9
noteon 2 62 103
sleep 4.95
noteon 5 50 102
noteon 12 50 104
sleep 24.752
noteon 13 50 106
sleep 4.95
noteon 3 50 102
sleep 4.95
noteon 15 50 91
sleep 4.95
noteon 14 38 108
sleep 524.737
noteoff 10 62 0
sleep 4.95
noteoff 0 74 0
sleep 4.95
noteoff 1 74 0
noteoff 4 62 0
noteoff 11 62 0
sleep 4.95
noteoff 6 62 0
sleep 9.900
noteoff 2 62 0
sleep 4.95
noteoff 5 50 0
noteoff 12 50 0
sleep 24.750
noteoff 13 50 0
sleep 4.95
noteoff 3 50 0
sleep 4.95
noteoff 15 50 0
sleep 4.95
noteoff 14 38 0
sleep 1782.173
quit
