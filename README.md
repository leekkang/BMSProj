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

## Sequence Diagram

## Inner Logic

## Reference

* [BMS command memo](http://hitkey.nekokan.dyndns.info/cmds.htm#CHARSET)
* [bemuse](https://github.com/bemusic/bemuse)
* [mkut/bms](https://github.com/mkut/bms/tree/master/BMSLib.Test)
* [Guide to understand BMS format](http://cosmic.mearie.org/2005/03/bmsguide/)