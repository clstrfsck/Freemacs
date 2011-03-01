FREEMACS FOR DOS

WHAT'S NEW

3/7/06: Francesco Zamblera has uploaded the bitmap font editor he used
to prepare the bitmaps for Freemacs MULE. In the next release of
Freemacs MULE, the font editor will be included directly in the MULE
package.  I've mirrored FFE at ibiblio. Also at Francesco's site.
<http://utenti.lycos.it/vilnergoy/>

1/13/06: Francesco Zamblera has written a small multilingual package
("MULE") for Freemacs. Some bitmap fonts and keyboard files, for use
with XKEYB and a modified version of GNUCHCP, a program which can make
PS files with the right font encodings, and a series of MINT function
to integrate all that into Freemacs.  I've mirrored MULE at
ibiblio. Also at Francesco's site.
<http://utenti.lycos.it/vilnergoy/>

ABOUT

Freemacs is a programmable editor. The .EXE file is only ~21K because
it only contains a language interpreter and text editor
primitives. The bulk of the programming is done in MINT, which is a
string-oriented language. Freemacs is yet another GNU emacs
clone. Emacs was first written at MIT by Richard M. Stallman.

In 1994, I started to assemble a list of software for the FreeDOS
Project, an open-source effort to re-create MS-DOS
functionality. Although we didn't use the term "open-source" and the
name was still "PD-DOS" or "Free-DOS" at the time, but it was the same
project with the same goals - use open-source software GPL, public
domain, or other free licenses) to create a DOS replacement.

But I didn't want our DOS to just be a plain DOS system. DOS should be
improved where possible. I decided to include some more powerful
utilities and applications, including some work-alikes of UNIX
applications and utilities. Russ Nelson had written the freemacs
editor, a clone of GNU emacs for DOS. Unlike other emacs editors,
freemacs tried very hard to be just like GNU emacs. I like that.

In 1994, I asked Russ for permission to include his freemacs editor in
our FreeDOS Project, and he agreed. Later, we re-released the
test-release of freemacs 1.6d as 1.6g, removing all restrictions on
redistribution. For a while, this became the default text editor for
FreeDOS. Russ said he didn't have time anymore to work on freemacs, so
he gave me the role of maintainer. Unfortunately, I haven't had much
time to work on it, either.

I use freemacs for all kinds of things, including editing my C
programs and creating html pages (although we don't have an
html-mode.)

BUGS

Freemacs is quite stable, although as in any open-source project, some
bugs remain, including a fatal bug if you happen to use CGA
mode. Contact me if you want to help apply these fixes. We really
should have a freemacs 1.7.

DOWNLOAD

Current version is 1.6g ("g" = "GNU").

    * DOS binary [117k]
      http://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/edit/emacs/emacs16g.zip

    * source code [100k] (requires TASM)
      http://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/edit/emacs/emac16gs.zip

    * emacs docs [45k] (from 1.6a)
      http://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/edit/emacs/emacdocs.zip

    * MINT reference [22k]
      http://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/edit/emacs/mint.zip

DOCUMENTATION

Mini How-tos that relate specifically to freemacs:

    * Brief tutorial on modifying freemacs, by Ken R. van Wyck.
      (docs/customiz.txt)

    * Freemacs mini-HOWTO, by Jim Hall.
      (docs/mini.txt)

    * MINT Tutorial
      (docs/mint.txt)

    * Freemacs extension writer's guide to MINT functions (a MINT
    * tutorial)
      (docs/mint2.txt)

    * Emacs in 2 pages
      (docs/quickie.txt)

    * Freemacs tutorial, by Richard Stallman.
      (docs/tutorial.txt)
