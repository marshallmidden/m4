#!/bin/bash -ex
# echo .bashrc

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi
##############################################################################

# There are 3 different types of shells in bash: the login shell, normal shell
# and interactive shell. Login shells read ~/.profile and interactive shells
# read ~/.bashrc; in our setup, /etc/profile sources ~/.bashrc - thus all
# settings made here will also take effect in a login shell.
#
# NOTE: It is recommended to make language settings in ~/.profile rather than
# here, since multilingual X sessions would not work properly if LANG is over-
# ridden in every subshell.
unset LANG
export LANG
# Some applications read the EDITOR variable to determine your favourite text
# editor. So uncomment the line below and enter the editor of your choice :-)
# export EDITOR=/usr/bin/vim
# export EDITOR=/usr/bin/mcedit
##############################################################################
# If not running interactively, don't do anything
test -z "$PS1" && return
##############################################################################
test -s ~/.alias && . ~/.alias
##############################################################################
# echo PATH=$PATH
# Default is: PATH=/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/opt/X11/bin
#-----------------------------------------------------------------------------
prepath () {
    case ":$PATH:" in
      *":$1:"*) :;;			# in the middle
      "$1:"*) :;;			# at the end
      *":$1") :;;			# at the beginning
      "$1") :;;				# if only one
      *) PATH=$1:$PATH;;
    esac
}
#-----------------------------------------------------------------------------
prepath /px/bin
prepath /px/bin/cli
prepath /usr/local/libexec
prepath $HOME/bin
#--export PATH="$HOME/bin:/usr/local/libexec:/px/bin/cli:$PATH:/px/bin"
# echo PATH=$PATH
##############################################################################
BLOCKSIZE=1024
#-- CDPATH=".:$HOME:$HOME/src:/usr/src:/sys"
CDPATH=".:$HOME:$HOME/src"
unset EMACS
FIGNORE='.o:.lo:.po:.b:.rc:.save:.orig:.old:.save:.BAK:.bak:.Bak:.aux:.dvi:.toc:.lof'
HISTFILESIZE=262144
HISTSIZE=262144
IGNOREEOF=
HISTCONTROL=ignoredups
##############################################################################
export GZIP=-9
export LESS='-C -d -e -g -i -Q -s -R'
# export MAKEFLAGS=-w
export MORE='-cs'
export SIMPLE_BACKUP_SUFFIX='~'
export VERSION_CONTROL='numbered'     
##############################################################################
unset LS_COLORS
export LS_OPTIONS='--color=none'
##############################################################################
# Turn on/off shell options
# shopt -u autocd 
shopt -u cdable_vars
shopt -s cdspell
shopt -s checkhash
# shopt -s checkjobs
# Check window size after each command, update LINES and COLUMNS.
shopt -s checkwinsize
shopt -s cmdhist
# shopt -u compat31
# shopt -u compat32
# shopt -s dirspell
shopt -u dotglob
shopt -u execfail
shopt -s expand_aliases
# shopt -u extdebug
# On some systems this blows up tab expansion in the bash expansion scripts.
#--blowup-- shopt -u extglob
shopt -s extglob
# shopt -s extquote
# shopt -u failglob
# shopt -s force_fignore
# shopt -u globstar
# shopt -u gnu_errfmt
# Append to the history file, don't overwrite it.
shopt -s histappend
shopt -u histreedit
shopt -u histverify
shopt -s hostcomplete
shopt -u huponexit
shopt -s interactive_comments
shopt -u lithist
shopt -u mailwarn
shopt -u no_empty_cmd_completion
shopt -u nocaseglob
# shopt -u nocasematch
shopt -u nullglob
shopt -s progcomp
shopt -s promptvars
shopt -u shift_verbose
shopt -u sourcepath
shopt -u xpg_echo
##############################################################################
# Following for HQs/SigmaTek.
alias pq='clear && ps augxww | egrep "vir|qemu"'
alias lsblk="lsblk -i"
# blkid -- use to see what is really mounted, verses "mount" command.
##############################################################################
#- unset LS_COLORS
##############################################################################
alias sql-list='(echo ".mode line"; echo ".echo on"; echo "select * from vm_snaps;"; echo "select * from snap_schedules;"; echo "select * from vm_settings;"; echo ".exit")| sqlite3 /var/www/pw/db/development.sqlite3'
alias virsh-list='virsh list --all ; virsh snapshot-list ActiveDirectory2016; virsh snapshot-list win2016'
alias zfs-list='zfs list -t snapshot -r clouddrive/ParsecCIFS vmstorage_pool/vm-disk'
#-----------------------------------------------------------------------------
export GOROOT=/root/src/go
export GOPATH=/root/src/GoProjects
prepath $GOPATH/bin
prepath $GOROOT/bin
#-----------------------------------------------------------------------------
ulimit -c unlimited
ulimit -d unlimited
ulimit -f unlimited
ulimit -i 256534
ulimit -l unlimited
ulimit -m unlimited
ulimit -n 1048576
ulimit -q unlimited
ulimit -s unlimited
ulimit -t unlimited
ulimit -u unlimited
ulimit -v unlimited
ulimit -x unlimited
TIMEFORMAT='real %R sec  user %U sec  system %S sec - %%%P CPU usage'
#-----------------------------------------------------------------------------
# vim:ts=4:sw=4
# echo end of .bashrc
