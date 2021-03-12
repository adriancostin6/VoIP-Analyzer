# VoIP Analyser

Command-line interface for capturing and analysing VoIP traffic. Main features
include the parsing and creation of SIP signaling packets, with a side emphasis
on capturing RTP data, decoding it and producing .wav files.

## Getting Started

### Prerequisites

This project requires the [libtins](https://github.com/mfontanini/libtins) library for compilation. 
Instructions for installing this dependency are provided on the official github page.

For Windows builds, you will also need the [WinPCAP developer pack](https://www.winpcap.org/install/bin/WpdPack_4_1_2.zip).

### Building on Linux/\*NIX operating systems

#### Step 1 - Cloning the repository

To clone the repository run `git clone https://github.com/adriancostin6/VoIP-Analyzer.git`.

#### Step 2 - Installing libtins 

##### Arch Linux based systems

If you are using an Arch Linux based distribution you can install libtins directly from the AUR (Arch User Repository) using your AUR helper of choice:

Example: `paru libtins` or `yay libtins`. *Make sure to get the libtins package, not lib32-libtins.*

##### MacOS

If you are using MacOS you can install libtins using homebrew:

`brew install libtins`

##### Other \*NIX based operating systems

If you are using another *NIX based operating system you will have to manually compile and install libtins. Because libtins depends on libpcap and libcrypto you will have to install those as well:

- For Debian based systems

`apt-get install libpcap-dev libssl-dev cmake`

- For Red Hat based systems

`yum install libpcap-devel openssl-devel cmake`

##### Building libtins

After getting all the required dependencies, building the library can be done by following the steps highlighted below:

1. Go to the root project directory and create an *external* folder

```
cd VoIP-Analyzer
mkdir external
cd external
```

2. Clone the libtins repository and build the source code

```
git clone https://github.com/mfontanini/libtins.git
cd libtins
mkdir build
cd build
cmake ../ -DLIBTINS_BUILD_SHARED=0 -DLIBTINS_ENABLE_CXX11=1
make
```

3. Install the library. The shared objects will typically be installed in `/usr/local/lib` and `/usr/local/include`:

`make install` 

#### Step 3 - Building the project 

In order to build the project, go to the root directory and execute the following commands:

```
cd VoIP-Analyzer
mkdir build
cd build
cmake ..
make
```

After doing so, you should have an executable called `voip-analyzer`. *Remember to run it with elevated privileges if you want to use the packet capture mode.*

### Building on Windows

#### Step 1 - Cloning the repository

To clone the repository run `git clone https://github.com/adriancostin6/VoIP-Analyzer.git`.

#### Step 2 - Downloading libtins 

Under the Windows operating system you have two options for using the libtins library. You can either download a binary version of the library from the [appveyor platform](https://ci.appveyor.com/project/mfontanini/libtins), or you can build the source code, which is the way I recommend doing it. To do so, you should follow the steps provided below:

1. Download the libtins source code and place it into a directory called *external* inside the VoIP-Analyzer project directory.

```
cd VoIP-Analyzer
mkdir external
cd external
git clone https://github.com/mfontanini/libtins.git
```

2. Download the WinPCAP developer pack from the following [link](https://www.winpcap.org/install/bin/WpdPack_4_1_2.zip). After downloading, extract the contents of the zip archive into the *external* directory you created earlier.

3. Build libtins

```
cd libtins
mkdir build
cd build
cmake ../ -DLIBTINS_BUILD_SHARED=0 -DLIBTINS_ENABLE_CXX11=1 -DPCAP_ROOT_DIR=../../WpdPack
```

This will generate a Visual Studio solution that you can use to build the libtins library. Once you open the solution, build the *tins* subproject.

*Recommended build configuration is Release x64*.

#### Step 3 - Building the project 

If both the *libtins* and the *WinPCAP developer pack* are properly placed inside the *external* directory, buiding the project can be done using the following commands:

```
cd VoIP-Analyzer
mkdir build
cd build 
cmake ..
```

After running *CMake*, you should have a Visual Studio solution inside the *build* directory, which you can use in order to build the project. *The recommended build configuration is Release x64*. You can also build the project using:

`cmake --build --config Release .`

## Features 

1. Live capture mode
    1. Live interface
        1. Captures all incoming traffic and saves it to a PCAP file
        1. Parses the PCAP file two times to extract the SIP and RTP data
        1. Saves all of the SIP packets to separate output files located in the output folder
        1. Decodes and writes the RTP data to two separate WAV files, one for each speaker in the call.
    1, Existing PCAP file
        1. Parses an existing PCAP file for SIP and RTP data as described above
1. Packet crafting mode
    1. Reads user input either from a text file or from the command line
    1. Creates a SIP packet from the information provided
    1. Validates the packet by checking for the correct syntax and presence of mandatory headers (for SIP requests)
    1. If the packet is valid it sends it over the network to the specified address and port

## Decoding RTP packets only works on the linux version at the moment.
