set nocompatible nosecure exrc autoindent showmatch noslowopen modeline modelines=5000
set shiftwidth=4 tabstop=8 wm=2
set encoding=utf-8
map R1 Jxx
map R2 0i/* 0f;a */
map Oo 0i/* 0f;a */
map R3 0i/* 0f}a */
map Oj 0i/* 0f}a */
map RN 0i/* 0f{a */
map R1 :q
syntax off
set nohlsearch
set nowrapscan

map [3~ :.,/^[@d]/-1s/^/#manual //^[@d]:w
map OF :.,/^[@d]/-1s/^/#advanced //^[@d]:w
map [6~ :.,/^[@d]/-1s/^/#ignore //^[@d]:w

map [3~ opr_info("%s:%u:%s Entering show_attribute",__FILE__,__LINE__,__func__);:x
map [6~ /^$/

" map [5~ 0f=c$ is not set0i# 
" map [6~ 02xf c$=y

" map OP o      ------------------bkn
" map OQ o                         ------------------bkn
" map OR o                                            ------------------bkn
" map OS o                                                               ------------------bkn

" map [15~ o      ++++++++++++++++++bkn
" map [17~ o                         ++++++++++++++++++bkn
" map [18~ o                                            ++++++++++++++++++bkn
" map [19~ o                                                               ++++++++++++++++++bkn

" map [20~ o      ******************bkn
" map [21~ o                         ******************bkn
" map [23~ o                                            ******************bkn
" map [24~ o                                                               ******************bkn


map [5~ :tn
