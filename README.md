# GD Autosave

## Usage

1. Inject DLL into game (use an autoloader, that works too)
2. Open editor
3. Become free from the shackles of random crashes

## Disabling autosave

1. The option can be found within the editor settings. That is, go into the editor, click the pause button, and then click the settings button at the top-right of the pause menu. You will find your desired checkbox on the 2nd page.

2. Click the checkbox. This setting is applied globally.

## Changing time

The default time offset is set to be 300 seconds, or 5 minutes.

This is currently defined as a constant in the `game_hooks.cpp` file. This is not subject to change anytime soon, but it should be trivial to add the ability to set this in-game.

## Credits

* Andre for [CappuccinoSDK](https://github.com/AndreNIH/CappuccinoSDK)
* Camden for theory crafting
