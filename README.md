# Arduino Firmware

Plantware Team

## Project Description
This project creates a user-mode audio TX wav file player. It explores the intricacies of handling audio digital media streams in multimedia embedded systems, focusing on real-time requirements for timely processing of incoming and outgoing data streams. The specific implementation involves utilizing the AXI Streaming FIFO of the Zynq SoC for FIFO programming, a technique widely employed in various devices like portable media players, set-top boxes, and digital TVs. Code that initialized the AXI FIFO which pipes to the CODEC was implemented in the pre-lab. Code that configures the CODEC and Audio TX draining was created for this lab. The user-mode wav player implements the following:

- Automatically configure and initialize audio CODEC on Zedboard.
- Automatically enable and disable Audio TX capabilities on Zedboard.
- Automatically map AXI FIFO and use FIFO to transmit output samples.
- Play 8 bit - 32 bit wav files, mono and stereo.

## Installation / Make Instructions
### Prerequisites
In order to install and make this driver for a Northeastern University Zedboard, you will need a custom developed SDK, specifically designed for the Northeastern Zedboards. For a more detailed look at the SDK installation, please see [this](https://neu-ece-4534.github.io/EnvironmentHome.html) link. General SDK installation guidelines are listed below.  
***NOTE: Steps are only intended for Linux based installations, specifically debian installations like Ubuntu.***

Install development tools and git
```console
$ sudo apt install build-essential gdb-multiarch git libyaml-dev cmake
```

Install the Northeastern Zedboard specific SDK (includes cross-compilation and VENV) using the following commands
```console
$ SDK_INST=esl-glibc-x86_64-esl-eece4534-image-cortexa9t2hf-neon-zedboard-esl-v3-toolchain-2023.1.sh
$ wget --no-check-certificate  https://www1.coe.neu.edu/~esl/EECE4534/$SDK_INST
$ bash ./$SDK_INST -d ~/eece4534sdk
```

Allow for simple SDK activation in your shell using the `setup-xarm` shell macro. Place the following in your .bashsrc.
```console
$ echo "alias setup-xarm='source ~/eece4534sdk/environment-setup-cortexa9t2hf-neon-esl-linux-gnueabi && PS1=\"(xarm) \$PS1\"'" >> ~/.bashrc
```

<!-- Prebuild kernel libraries for kernel compilation.
```console
  $ setup-xarm
  $ cd $OECORE_TARGET_SYSROOT/lib/modules/5.10.0-esl/build/
  $ make scripts && make prepare
``` -->

### Downloading wav player
There are three ways to download the player for the Northeastern Zedboards. The easiest way is by navigating to releases, and downloading **Release 0.1** binary `player`. 

Another option is to download the source code. Once downloaded, extract the file and navigate to the './usermode-hw-player' directory. In this directory, there is a pre-compiled `player` binary that can be transferred to the Zedboard right away. However, if there are any issues with the pre-compiled release `player` binary, either from the source code or from the release, please refer to the instructions below for how to re-compile the player.

### (In case of release binary issue) Player compilation
The player can be re-made from the downloaded source code folder, or for the most up to date code, you can also clone the repository into a local folder on your machine.
```console
$ git clone git@github.com:neu-ece-4534-sp24/lab5-team-6.git <desired folder name>
```

In either the release folder or the cloned git repo, navigate to the './usermode-hw-player' directory. Clean the directory using your shell to remove the pre-compiled binary.
```console
$ make clean
```

Next, in the same directory, use your shell to re-make the player binary on your own machine.
```console
$ make
```
This will fail if the SDK is not initialized before hand (this can be done using the `setup-xarm` command).


### Uploading the binary to the Zedboard
Upload the compiled `player` binary to the Zedboard using your choice of secure file transfer. The following SCP command will work for transferring the module to the board (assuming you are in the './usermode-hw-player' directory).
```console
$ scp player root@zed<board number>.nuesl.org:  
```

## Usage Definition
In order to use the Audio TX wav player on the Zedboard, please ensure that there are existing wav files on the Zedboard. For convenience, there are two provided wav files in the './audio_samples' directory. 'hal_9000.wav' is an 8-bit, 11.025 kHz sample rate wav file, and 'butterdog.wav' is a 16-bit, 48 kHz sample rate wav file. Feel free to upload one of these to the Zedboard, or use your own.

### Playback of full wav file
In order to playback a full wav file, simply run the `player` in its default state.
```console
$ ./player <wav file name>.wav
```
This will play the file starting from sample 0 all the way to the last sample.

### Playback starting at some sample number
In order to playback the wav file from a certain starting point, use the `--start` command line argument.
```console
$ ./player <wav file name>.wav --start <sample number>
```
This will play the file starting from whatever sample number is defined. Note that it will return an error if the starting sample number is outside the range of the wav file.

### Playback some seconds of a wav file
In order to playback a user defined number of seconds of a wav file, use the `--seconds` command line argument.
```console
$ ./player <wav file name>.wav --seconds <number of seconds>
```
The number of seconds can be a floating-point number.

### Playback some seconds of a wav file starting at some sample number
Feel free to combine all of the command line arguments to play a certain number of seconds of audio starting at a desired sample number.
```console
$ ./player <wav file name>.wav --start <sample number> --seconds <number of seconds>
```

## Usage Example
### Example playing provided 'hal-9000.wav'
This example goes over playing the provided 'butterdog.wav' wav file in a number of different ways. First, navigate to the './audio_samples' folder provided in **Release 0.1** folder. Transfer it to the board using `scp` or `sftp`. Once the example wav file is on the Zedboard, and the `player` binary is on the Zedboard, the file can be played in a number of different ways.

Play the entire example wav file from start to finish. Check that there is audio coming out of the HPH Out port of the Zedboard.
```console
$ ./player butterdog.wav
```

Play only 2 seconds of audio from the example wav file. Check that the audio lasts roughly 2 seconds this time.
```console
$ ./player butterdog.wav --seconds 2
```

Play the example wav file starting from around halfway through (sample number 850425). Check that the audio sample appears to start from the middle. Please note that if an uneven sample number is chosen, the sample will start at the chosen sample number - 1. This is due to the way that the codec interprets the wav file data.
```console
$ ./player butterdog.wav --start 850425
```

Combine everything together to play 1 second starting at roughly halfway through the example wav file. Ensure this sounds as expected.
```console
$ ./player butterdog.wav --start 850425 --seconds 1
```

## Discussion
A few challenges were faced during the development of **Release 0.1**. First, the first makefile iteration did not properly link the `alsa/asoundlib.h`, and thus returned errors whenever the `make` command was used. To fix this, the -lasound flag was added to a list of tags for the overall make function to ensure that the final compilation did not have linker errors.

Another challenge faced was that when 8-bit samples were being played, the audio playback was super distorted. Upon a deep inspection, it turned out this was because 8-bit wav files are unsigned, whereas 16 and 32 bit wav files are signed. As the codec interpreted the all sample values as signed, this caused any value over 0.5 - 1 amplitude in an 8-bit wav file to be interpreted a -1 - 0 amplitude, heavily distorting the signal. The fix was this was to convert all 8-bit wav files to signed by subtracting 127 bits from each sample if the wav file was 8-bits.

Finally, we ended up using the `SND_PCM_FORMAT_S32_LE` flag for configuring the codec format, but after this changed, the codec interpreted the sent bits as big endian. The fix for this was to change the way `audio_word_from_buf` processed each audio buffer. However, changing to little endian could have hypothetically made the original shifting logic in the buffer to word conversion work as intended.

## Known Issues
A small issue is that the current **Release 0.2** release can only process up to 32 bit wav files, and returns an error if higher depth wav files are used. This should possibly be adjusted in future iterations, though it is worth noting the max codec depth is 24-bits for wav file outputs, so no higher listening depth can be achieved.

On the other hand, **RELEASE 0.2** fixed the largest issue with **Release 0.1**, where the player blocks when the AXI FIFO is full. The next paragraph highlights the previous release issue, while the updated performance of **Release 0.2** is discussed below. 

In `fifo_transmit_word`, the previous blocking function was as follows:
```c
void fifo_transmit_word(uint32_t word)
{
  // TODO implement this
  // NOTE block if full
  // printf("Transmitting word: %X\n", word);
  
  while(fifo_full() == 1) 
  {
    // Wait until FIFO is not full (block)
  }

  // Write the word to the FIFO
  *(volatile unsigned int *)REG_OFFSET(fiforegs, FIFO_TDFD) = word;
  // Transmit the word (4 bytes, 32 bits)
  *(volatile unsigned int *)REG_OFFSET(fiforegs, FIFO_TLR) = 0x00000004;
  // Read word
  // printf("FIFO_ISR: %X\n", *(volatile unsigned int *)REG_OFFSET(fiforegs, FIFO_ISR));
  // Clear transmit complete interrupt bits
  *(volatile unsigned int *)REG_OFFSET(fiforegs, FIFO_ISR) = 0x08000000;
}
```

The blocking in this function first checks if the FIFO buffer is full by calling fifo_full(). If the FIFO buffer is full (fifo_full() == 1), the function enters a while loop where it does nothing but continuously check if the FIFO buffer is still full. This constant checking consumes CPU cycles, because the CPU is actively engaged in running the loop and checking the condition. This results in up to 100% CPU usage on the Zedboard when playing an audio file as the function has to block so frequently. This is shown in the `top` output below.

```console
top - 14:30:21 up 53 min,  3 users,  load average: 0.12,  Tasks:  69 total,   2 running,  67 sleeping,   0 stopped,
%Cpu(s): 47.8 us,  2.2 sy,  0.0 ni, 49.8 id,  0.0 wa,  0. MiB Mem :    493.8 total,    415.7 free,     27.0 used,   MiB Swap:      0.0 total,      0.0 free,      0.0 used.

  PID USER      PR  NI    VIRT    RES    SHR S  %CPU
  965 root      20   0    3432   1788   1588 R 100.0
  963 root      20   0    4248   1884   1532 R   1.0          1 root      20   0   26288   6108   4660 S   0.0
    2 root      20   0       0      0      0 S   0.0
    3 root       0 -20       0      0      0 I   0.0
    4 root       0 -20       0      0      0 I   0.0
    8 root       0 -20       0      0      0 I   0.0
    9 root      20   0       0      0      0 S   0.0 

```
This issue was fixed in **Release 0.2** and the performance is discussed below.

## Release 0.2

### Release Highlights
- Implemented timed polling to only fill the FIFO when needed.
- Inserted the sleep into the code, when the FIFO is full.
- Implemented `calculate_sleep_time()` for automatic calculation of the delay at runtime based on the sampling rate defined in the WAVE file.
- Verified that we can play files with different sampling rates successfully.

### Features
- All features from **Release 0.1** are still present.
- Added timed polling to only fill the FIFO when needed, increasing efficiency and allowing for the playing of files with different sampling rates.

### Discussion

Our **Release 0.1** had a significant issue with the player blocking the CPU when the FIFO was full. This was due to the program continuously checking if the FIFO was full, and doing nothing else while it was. This was a significant issue, as it caused the CPU usage to be very high, and the program to run inefficiently.

Initially we implemented the polling system for a sample rate of 11025Hz, and the program worked as expected. However, when we tried to play a file with a different sampling rate, the program did not work as expected, and the sound was distorted/choppy. This was due to the program not being able to handle different sampling rates, and the FIFO being filled too quickly(or too slowly) for the codec to handle.

The main challenge faced in implementing timed polling was dynamically calculating the sleep time based on the sampling rate of the WAVE file. The sleep time was calculated using the formula 
```c
  double sleep_time_f = (double)502 / (double)hdr.sample_rate;
  sleep_time_f = sleep_time_f * (double)1000000;
  sleep_time = (unsigned int)sleep_time_f;
```
The sleep time was then used to sleep the program until the FIFO was ready to be filled again. This was implemented in the `fifo_transmit_word` function, which now looks like this(and sleep_time is a global variable):

```c
/* @brief Transmit a word (put into FIFO)
   @param word a 32-bit word */
void fifo_transmit_word(uint32_t word)
{
  while(fifo_full() == 1) 
  {
    usleep(sleep_time); 
  }

  // Write the word to the FIFO
  *(volatile unsigned int *)REG_OFFSET(fiforegs, FIFO_TDFD) = word;
  // Transmit the word (4 bytes, 32 bits)
  *(volatile unsigned int *)REG_OFFSET(fiforegs, FIFO_TLR) = 0x00000004;
  // Clear transmit complete interrupt bits
  *(volatile unsigned int *)REG_OFFSET(fiforegs, FIFO_ISR) = 0x08000000;
}
```
Doing this allowed us to play files with different sampling rates successfully. Furthermore, the program should run more efficiently now, as it only fills the FIFO when needed. This is shown in the `top` output below.

```console
top - 15:55:30 up 34 min,  2 users,  load average: 0.04,  Tasks:  67 total,   1 running,  66 sleeping,   0 stopped,
%Cpu(s):  9.5 us,  1.5 sy,  0.0 ni, 89.0 id,  0.0 wa,  0. MiB Mem :    493.8 total,    421.8 free,     26.5 used,   MiB Swap:      0.0 total,      0.0 free,      0.0 used.

  PID USER      PR  NI    VIRT    RES    SHR S  %CPU
  401 root      20   0    3432   1788   1588 S  19.2
    1 root      20   0   26288   6104   4660 S   0.0
    2 root      20   0       0      0      0 S   0.0
    3 root       0 -20       0      0      0 I   0.0
    4 root       0 -20       0      0      0 I   0.0
    8 root       0 -20       0      0      0 I   0.0
    9 root      20   0       0      0      0 S   0.0
   10 root      20   0       0      0      0 I   0.0
```
Observing the `top` output, we can see that the CPU usage is lower than in **Release 0.1**. This, as mentioned, is due to the program only filling the FIFO when needed, and sleeping when the FIFO is full. This is a significant design improvement over **Release 0.1**, and the cpu usage between the two releases may be even larger depending on the audio files played.


## Credits
Chris's pre-lab was used as the starting point for the user-level Audio TX wav player. This was initially to utilize a more flexible `audio_word_from_buf` converter that only used bitwise operations and shifting, but this was ultimately changed to work better with the codec and now works with if statements.
