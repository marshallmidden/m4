set nocompatible nosecure exrc autoindent showmatch noslowopen modeline modelines=5000
set shiftwidth=4 wm=2
set encoding=utf-8
map R1 :q
map R2 0i/* 0f;a */
map Oo 0i/* 0f;a */
map R3 0i/* 0f}a */
map Oj 0i/* 0f}a */
map RN 0i/* 0f{a */
map R1 :q
syntax off
set nohlsearch
set nowrapscan

map OP yyppoj
map OQ cwcheck_string_eoln
map OR f%cf""lcf))
map OS f,cf))f 
map Ol f"df,r&f)i, 1

