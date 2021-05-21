# life-terminal
A version of Conway's game of Life that runs in the commandline using ncurses.

This program is designed for and tested on Linux, but it should work on MacOS and other POSIX compatible OSes as well. The build script will probably work on all compatible OSes, because it assumes you have a specific compiler on your machine.

# Flags
  - -d (--board-dimensions): set the size of the board (world). Example of the format is 100x200. 100 is the width in this example and 200 is the height. Full example: `./conway-terminal -d 100x200`
  - -v (--version): Displays the version of the program and quits after.
  - -a (--about): Displays an about text and quits after.
  - -h (--help): Shows a help text and quits after.

# Controls
  - Arrow keys for navigating.
  - X/Enter: Make dead cell alive or make living cell dead.
  - T: Next generation (=tick).
  - P: Toggle automatic generation progression. (I don't know why P)
  - +/-: change tick speed. (You don't need to use 'shift' + '=' to increase, the '=' key does just fine as well)
  - Q: exit the program.
  
This version in it's current form is not very useful to design advanced things in, because it doesn't have saving functionality.



# Building
Before you can use it, you'll have to build it.
### Step one: downloading
First download the repo. You can use this command for that:
```shell
git clone https://github.com/Thijmer/life-terminal
```
### Step two: building
Then run `cd life-terminal` to enter the repository directory. 
Now you can run the following command to build the code:
```shell
./build.sh final
```
(This assumes you have gcc installed on your machine. This will be the case on most Linux systems.)

### Step three: running

After that's done, you can run the program from within this directory using:
```shell
./conway-terminal
```

# Contribute
Are there bugs anywhere? Do you see a way to improve performance? Please report it to me, I'd be happy to fix it for you. Or (if you have programming knowledge, which the audience for this program will probably have) feel free to make a clone, fix the things and create a pull request.
