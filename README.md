# Welcome
Welcom to the mandelbrot Operating System. 
<br>This OS is built by a humble group of teenagers over at [Discord](https://discord.gg/W523cD3Q3P). 
<br>We do this solely to have fun and to learn. 
<br>We are not organized and are here to enjoy ourselves. 
<br>Sounds apealling? Create a pull request. We'll see if we can accomodate you.
<br>![image screen](./screen/mandlebrot.png)

# About 
This project is made for fun and learning
<br>It's like tracing OS history, but with modern knowledge and not a lot of budget lol

# Where are we in the project
### In that section, I will compare our OS to other OSes from the past
### We are not saying that our OS is equal to them, we are way less talented than those guys were
<br>We have a shell, with some command.
<br>We are developping the Unix / MS-DOS of Mandelbrot.
<br>
<br>We need a Filesystem. We are planning to use USTAR.

# Running
You'll need to have a cross-compiler, build one using `./build-cross-compiler.sh`.
<br>Run `make qemu` to compile the OS and run it in `qemu` (needs `qemu-system-i386`) or just `make` to compile it.
<br>Option are available to build
<br>To build with azerty keyboard, add AZERTY=1 to the command (`make AZERTY=1 qemu` for exemple).
<br>To build with clang instead of GCC, add LLVM=1 to the command (`make LLVM=1 qemu` for exemple).

# Using
Using our OS is really simple, on boot, you'll be greated with a shell
<br>A small variety of command available:
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-cls : clear the screen
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-hello : print hello world
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-hi : print 20 time hi
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-echo : repeat the args if any
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-sysfetch : neofetch but it's in the kernel
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-reboot : reboot the system
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-bsod : kernel panic the system
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-mandlebros : show mandlebros background
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-ctheme : change the theme with the following
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;legacy : 
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;![image legacy](./screen/legacy.png)
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;dark : 
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;![image dark](./screen/dark.png)
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;light : 
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;![image light](./screen/light.png)
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;witchcraft : 
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;![image witchcraft](./screen/witchcraft.png)

## Using code
This is open source. Reuse code. Just follow the license terms and we are good. :)
