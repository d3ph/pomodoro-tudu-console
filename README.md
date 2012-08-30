Pomodoro TuDu (console)
=====================

TODO list + pomodoro-indicator for best time management task scheduling

Install
=======

 TuDu needs ncurses library with wide characters (in some systems is known as 
 ncursesw) for work. You will need to install it for compile it.

 For compiling the program execute:
 $ ./configure
 $ make

 For installing the program execute (as root):
 # make install


Usage
=====

Usage: tudu [options]
        -f file load tudu file
        -c file load specific config file

        -v      show version
        -h      show this usage message

The default config file is in ~/.tudurc
The file tudurc in the source code is an example of config file.


Notes
=====

* Since version 0.8 the theme config deadlineMark now is call warn, because is
not use just for the deadline is also use for the past sched.


* Some environments (like Konsole of KDE3) don't show well the tasks that have
childs. The normal way to show them is because they are bold, but in some 
shells the bold characters are indistinguishable of the normal ones. For that
cases (or others) there is the posibility of show "(+)", for the collapsed 
tasks, and "(-)", for the expanded tasks, at the left of them. This option is
activated on the config file by the option:

[ general ]
visual_tree  = yes
