# Half-Rats: Parasomnia - Public source code

This is the public source code of the Half-Life 1 mod: Half-Rats: Parasomnia.

This source code was taken from the "v1.0b" build meaning that you have the same code used to produce the "v1.0b" binaries.

**By using this source code, you agree to respect the "Half-Life 1 SDK license" available in the `LICENSE` file alongside this README.**

You are free to do whatever you want with this source code as long as:

- The above license is respected.
- No commercial usage is made.

We would really appreciate that you give proper credits (see the last section in this file) should you use this source code.

## Building the source code (Windows)

Half-Rats: Parasomnia has been developed with Visual C++ 2010 Ultimate with the Service Pack 1. You have multiple options:

- You can find somewhere on Internet a download link to the free Visual C++ 2010 Express Edition (other editions should work) with it's Service Pack 1. In that case, just open the `projects.sln` solution located in `projects/vs2010` and start programming right away.
- Upgrade the projects and solution for a modern Visual Studio version, keep in mind that you will likely need to change existing code in order to build it under these modern toolsets.

It's recommended to keep the warning C4996 disabled to keep the support with Linux and Mac.

## Building the source code (Linux)

Linux developers need the latest version of GCC/G++ (x64 users must use "multilib" versions) and a terminal, just `cd` in the `linux` folder and type `make`.

Mac developers need the latest version of Xcode command-line tools and a terminal, just `cd` in the `linux` folder and type `make`, keep in mind that Half-Rats: Parasomnia wasn't designed for Macs in the first place so extra work will be required to support that platform.

## Contributing

We no longer accept bug reports for Half-Rats: Parasomnia. However, it's with pleasure that we accept pull requests that might lead to a community patch released on Steam (credits will be given of course).

## Credits

Half-Rats: Parasomnia's code has been written by Osiris and Joël "Shepard62FR" Troch with the contributions from the following people:

- abbadon from TWHL for the Armored Fucker's shield AI code.
- ts2do from The Wavelength for the dynamic NPC footsteps sounds based on materials.
- Claudio "SysOp" Ficara for the AI marker (Half-Cat's fish) from Arrangement.
- Dominik "ripieces" Tugend for the input code fixes under Linux and Mac.
- Laurie "LRC" Cheers for the fixes and some entities implemented within Spirit of Half-Life.
- Marc-Antoine "malortie" Lortie for the fixes for modern Visual Studio versions.
- Sam "Solokiller" Vanheer for the Armored Fucker's shield AI code, network fixes and minor other stuff.
- Valve Software for the GoldSrc engine.
- Id Software (now a ZeniMax company) for the QuakeWorld/id Tech 2 (Quake II) engines which GoldSrc is based upon and has fixes from respectively.
