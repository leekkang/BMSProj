# BMSPlayer

* A program that interprets [BMS](http://hitkey.nekokan.dyndns.info/cmds.htm#MEMO-ABOUT-BMS-FORMAT-SPECIFICATION) format files and plays music using the [FMOD](https://www.fmod.com/) library.
* This program is designed to test the BMS analysis library before building the game application.
* BMS files and music files must be downloaded separately.

## Environment & External Library

* OS : Windows 10 64bit
* IDE : Visual Studio Community 2017 15.9.11
* Library
  * [FMOD Studio API 2.00.08](https://www.fmod.com/download)

## Usage

* After creating the `StreamingAssets` folder, put a folder containing BMS files in the folder.
  * Up to depth-2, BMS files can be recognized.
    * `StreamingAssets/BMSFolder` OK
    * `StreamingAssets/Folder/BMSFolder` OK
    * `StreamingAssets/Folder1/Folder2/BMSFolder` NO
* You can move folders and music using the up, down, left, and right arrow buttons on the keyboard.
  * left arrow : prev folder
  * right arrow : next folder
  * up arrow : next music
  * down arrow : prev music
* The pattern can be moved with the `[, ]` button on the keyboard.
  * `[` : prev pattern
  * `]` : next pattern
* If you are using an IDE for debugging, you need to link the FMOD Library.

![](result.png)

## Supported Option

* Encoding Type
  * [UCS-2](https://en.wikipedia.org/wiki/Universal_Coded_Character_Set) BE, LE
  * [UTF-8](https://en.wikipedia.org/wiki/UTF-8)
  * [Shift JIS](https://en.wikipedia.org/wiki/Shift_JIS)
  * [EUC-KR](https://en.wikipedia.org/wiki/Extended_Unix_Code#EUC-KR)

* [Supported Command List](http://hitkey.nekokan.dyndns.info/cmds.htm#MEMO-ABOUT-BMS-FORMAT-SPECIFICATION)
  * [PLAYER](http://hitkey.nekokan.dyndns.info/cmds.htm#PLAYER)
  * [TITLE](http://hitkey.nekokan.dyndns.info/cmds.htm#TITLE)
  * [ARTIST](http://hitkey.nekokan.dyndns.info/cmds.htm#ARTIST)
  * [GENRE](http://hitkey.nekokan.dyndns.info/cmds.htm#GENRE)
  * [PLAYLEVEL](http://hitkey.nekokan.dyndns.info/cmds.htm#PLAYLEVEL)
  * [DIFFICULTY](http://hitkey.nekokan.dyndns.info/cmds.htm#DIFFICULTY)
  * [STAGEFILE](http://hitkey.nekokan.dyndns.info/cmds.htm#STAGEFILE)
  * [BANNER](http://hitkey.nekokan.dyndns.info/cmds.htm#BANNER)
  * [BPM](http://hitkey.nekokan.dyndns.info/cmds.htm#BPM), [BPMxx](http://hitkey.nekokan.dyndns.info/cmds.htm#BPMXX)
  * [WAV](http://hitkey.nekokan.dyndns.info/cmds.htm#WAVXX)
  * [STOP](http://hitkey.nekokan.dyndns.info/cmds.htm#STOPXX)
  * [LNOBJ](http://hitkey.nekokan.dyndns.info/cmds.htm#LNOBJ)
  * [LNTYPE](http://hitkey.nekokan.dyndns.info/cmds.htm#LNTYPE-1)
  * [RANDOM](http://hitkey.nekokan.dyndns.info/cmds.htm#RANDOM) (control flow)
  * Since BMP, RANK and TOTAL are headers related to gameplay, they are defined and not implemented.

## Sequence Diagram

## Inner Logic

## Reference

* [BMS command memo](http://hitkey.nekokan.dyndns.info/cmds.htm#CHARSET)
* [bemuse](https://github.com/bemusic/bemuse)
* [mkut/bms](https://github.com/mkut/bms/tree/master/BMSLib.Test)
* [Guide to understand BMS format](http://cosmic.mearie.org/2005/03/bmsguide/)