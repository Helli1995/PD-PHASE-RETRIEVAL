# PD-Phase-Retrieval
Real-Time-Phase-Gradient-Heap-Integration and RTISI-LA Objects, implementing two
different algorithms for realtime phase reconstruction, based on the c-library 
libphaseret.

Recommended Installation Steps:

For MacOS:

1. Install homebrew https://brew.sh

2. Install with brew the following libraries:
"brew install cmake fftw lapack"

3. Clone libltfat(written by Peter L. Søndergaard and Zdeněk Průša.)-fork from umleaute:
https://github.com/umlaeute/phaseret

4. Open the terminal.app and move to directory phaseret/libltfat/
Create a new build-diretory with "mkdir build"
Enter the directory with "cd build"
Build the libltfat and libphaseret with the following command:
"cmake -DNOBLASLAPACK=0 -DNOFFTW=0 -DUSECPP=1 \ -DBUILD_SHARED_LIBS=1 -DDO_LIBPHASERET=1 .."

libltfat.dylib, libphaseret.dylib,... should be present now in the current directory.

5. Download the phase_retrieval library for PD from this page.
Unzip the directory, open terminal.app and move inside the unzipped directoy
and type the following command: "make"

6. In your PD-external folder create a new diretory (e.g. pd-phase_retrieval) and copy all the 
.dylib files and the created .pd_darwin and help.pd files into this directory.

7. In PD Settings: Add the paths to the .pd_darwin files to searchpaths.

If you want to use the patch GraphicsOnSound inside the examples folder additionally install:
	8. pd-windowing (repeat step 5 with this library).
	https://github.com/electrickery/pd-windowing
	and copy the .pd_darwin files into externals or into a newly created folder inside externals.
	Also you have to install a recent version of Gem via Deken (Open PD -> help -> find externals online: search for Gem and download) or download from (https://git.iem.at/pd/Gem) and install.
	(also add these paths to the PD-searchpaths)

