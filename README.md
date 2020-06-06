# Bluetooth [WIP]
STM32 Bluetooth Audio Player

This repository documents my work designing, building, programming, and testing a Bluetooth audio player. Some of this log has been written retroactively, but I try to capture what my thoughts were at the time. 

## Motivation:
This project started as a request from a friend. He asked if I could make a music player that he could wear as a watch and stream to Bluetooth headphones. I had done some work before with FPGAs using SD cards so I was familiar with the protocol. I determined that this would be challenging enough to be a) fun, and b) educational, while still being possible/practical given my skills and equipment.

## Preliminary Requirements
Much of this project would be serving as a learning experience. Prior to this I only had experience with programming MIPS (PIC32) microcontrollers in class or in smaller personal projects. Additionally this work had only minor hardware integration. This would be my first project to combine significant hardware design and development (component selection, schematic design, PCB layout, board assembly, board testing) with full programming. Over time I had accumulated drivers/libraries for PIC devices but I had not worked with ARM devices yet. I should explain that I determined early on to use an STM32 processor due to the prevalence in industry.

Because this would serve as a learning experience, I did not have a firm idea of what I would be capable of for this. This led to establishing loose requirements so that way based on what I learned, I could adjust the design.

My preliminary requirements were (in no significant order):

1. Read an audio file off of a micro SD card and stream the file over Bluetooth to a speaker or headphones
1. The whole module should be able to be worn (with a 3D-printed housing or other mount) on the wrist as a watch-sized device
1. Be battery powered and have a streaming battery life of >90 minutes
1. Be able to recharge the battery via a micro USB port
1. Allow the user to:
    1. Pause/Play
    1. Increase/Decrease the volume
    1. Skip to the Next/Previous song
    1. Allow the user to choose between a "shuffle" function and sequential playback
1. Use an STM32 processor
1. Keep the total cost <$75 USD

## Component Selection
Component selection was centered on the Bluetooth radio/module and the host microcontroller. The radio selected was the SierraWireless BC127. The selection was driven by the following major factors (note that cost and size factor into all components):
1. Embedded Bluetooth Stack
1. Low (15mA) streaming current consumption
1. Ability to be powered directly from a lithium battery and integrated charging circuit 
1. Availability of Sparkfun Arduino libraries. 

I had planned to leverage the Arduino libraries to help with the programming, but it has proven easier and quicker to write my own functions rather than port the Arduino ones.

The BC127 supports the I2S protocol for digital audio and is controlled via UART. This then led to the selection criteria for the host microcontroller. The selected microcontroller was a STM32L051C8 in a 48-pin LQFP package. The major factors in this decision were:
1. Included peripherals
    1. I2S for digital audio transfer to BC127
    1. SPI for SD card communication
    1. 2x UART for communication between STM32 and BC127 and communication between SM32 and a PC for debugging and configuration
1. EEPROM for user setting storage
1. DMA for SD (SPI) reception and I2S transmission
1. Up to 32MHz clock 
1. Sufficient RAM for data buffers
1. Low current consumption
1. 48-pin LQFP is easily hand-soldered

With these two core components selected, the remaining components were chosen during the circuit design process based off of either reference circuits from the datasheets, or general design knowledge.

## Circuit Design
Schematic design and PCB layout was done in EagleCAD. The design used many common components that already had component models, but several libraries and models had to be made still. 

Testing of some circuit aspects (e.g. the power on/off circuit) were modeled in OrCad and simulated in PSPICE as a sanity check.

The design of the battery charging circuit is based off of the Application Note for the BC127. In more recent versions of the Application Note it is made clear that the charging circuit shown bypasses the internal regulator and uses an external fast charge. Had I understood this at the time then some space could have been saved on the PCB. 

### Schematic

The finalized schematic:
![Finalized Schematic](/Images/Schematic.png)


STM32CoreMX was used to simplify STM32 pin selections during the schematic design.

### PCB

The finalized PCB layout:
![Finalized PCB Layout](/Images/PCB.png)

The layout is not ideal as there are some larger loops, and my high frequency lines do alternate between layers, but when doing the layout I determined that this would likely not be an issue. This is because the majority of the traces are DC signals and the highest frequency lines are my I2S clock and SD SPI clock lines. The I2S clock runs at about 3 MHz and the SPI clock runs at about 4 MHz. 

1206 size resistors and capacitors were used for the ease of soldering. In hindsight I could have at least gone down to 0805 size and hand soldered them easily. This would have saved space as well.

A 2-layer PCB was used in order to save cost, but a 4-layer board may have resulted in a more compact design. However, this would have still been limited by the size of the battery, micro SD card slot, and buttons for control.

### Assembly and Hardware Testing

The unassembled PCB:
PCB Top | PCB Bottom
--------|-----------
![Finalized PCB Layout](/Images/PCB_TOP.jpg) | ![Finalized PCB Layout](/Images/PCB_BOTTOM.jpg)

At the time of assembly, the only lab equipment I had was a soldering iron with various tips, a set of "Helping Hands", a magnifying glass, a handheld multimeter, and flux, solder, wire, etc. (Since then I have upgraded my home lab to have a proper work area along with a 5-20x stereo microscope, 4-channel 100MHz oscilloscope, variable power supply, various fine tools etc.)

The assembled PCB:
PCB Top | PCB Bottom
--------|-----------
![Finalized PCB Layout](/Images/Assembled_PCB_Top.jpg) | ![Finalized PCB Layout](/Images/Assembled_PCB.jpg)

Given the tools available at the time I am satisfied with the quality of the assembled PCB.

Testing was done in various stages of PCB assembly. Once the USB port and 3.3V regulator were installed, the board was powered and the voltage was checked to verify 3.3V. Once this tested satisfactory then the remaining components were installed with continuity checks and voltage checks along the way to verify operation. 

A set of pads for use with the ST-Link programmer and a set of pads for UART communication with a PC were then used to begin programming.

## Programming
A significant drawback to the requirement to keep the total cost under $75 was that no development boards or discovery kits were used. This meant that programming could not begin in any significant way until the PCB design was finalized, assembled, and tested. 

As stated earlier, STM32CubeMX was used to assist with pin selection. This also allowed for easy configuration of the clock and peripherals and generation of the initial code. 

As programming began, an effort was made to utilize the HAL drivers as much as possible, however in many cases (especially early on in the program) it was easier or faster (from a code execution perspective) to interact with the registers directly. This had the added benefit of giving me a much better understanding of the environment versus if I only used the HAL drivers.

Data is stored on the SD card. It is read via SPI1 using DMA in order to reduce processor load. FatFs is used to handle the file reads. Data is read into a buffer with an upper and a lower half. This data is then sent over I2S using DMA to the BC127. The upper half of the buffer is filled while the lower half is being sent and vice versa. 

The BC127 is controlled via UART. The STM32 send and receives commands or notifications over UART2. In order to support configuration and debugging, this UART2 is mirrored to UART1. UART1 interfaces with a PC for terminal (PuTTY) display and control.

The buttons on the PCB are sampled and then used for control of the device. (Volume up/down, pause/play etc.)

The program was written in phases with testing at every point of development. An outline of the phases and issues/points of note are shown below:

1. Configuring UART1 for connection to PC. printf() was redefined to output over UART for easier use.
1. Implementation of hardware I/O layer for FatFs. Much of this information and code was taken from examples provided in the "Mastering STM32" book by Carmine Noviello. DMA was configured. SD card read tests passed.
1. Implementation of UART2 for communication with the BC127. Also set up as a pass-through for UART1 so the BC127 could be configured from a PC directly. Battery charging enabled using the BC127 because the BC127 will not function without a battery.
1. Debouncing and Oneshot was added and tested for user inputs.
1. Audio streaming was established. Audio data was read in with DMA but audio streaming was set up with I2S interrupts. I did not fully understand the HAL_DMA drivers so in the interest of progress, interrupts were used. The audio in the wav file is little endian, but the transmission is big endian so the bytes had to be switched. Volume control was implemented by shifting the bits of the audio samples to the right. At this point, I only had one Bluetooth Speaker to test with. The audio on this would crackle when playing off of this, but when connected to another device (e.g. a phone), the audio would be clear. I figured this was a timing issue with the audio transmission or a clock jitter issue but had no way to verify yet. Several months passed before work restarted.
1. Work restarted once I acquired an oscilloscope. I was able to see that there was a periodic dropout in the transmission. I optimized the code and reconfigured the interrupt priorities. The transmission dropout disappeared but the audio was still crackling. Around this time I also recieved a different Bluetooth speaker. When tested with this, I got good quality audio except for a drop that happens every 10 seconds. I provide more information about this in the "Current Issues" section.
1. DMA was successfully configured for I2S transmission. It was not entirely clear how to implement it with a circular buffer from the HAL manual, but trial and error along with significant googling provided an answer.
1. Major refactoring of almost all of the code. To this point, this had been more of a test platform with poor coding practices. I rewrote most of it with good practices that were learned from reading through example code and high-profile open source projects to see how code is arranged in them.
1. Implementation of a state machine to handle user inputs, BC127 configuration, pairing of new devices, and file navigation.
1. Implement error handling and the ability to accept multiple WAV configurations (e.g. mono or stereo). [Current Step]
1. Implement LED controller to visually show important information about the device (battery status, connection status etc).
1. Perform a final optimization of the code and decrease the clock speed as much as possible to increase battery life.

## Current Issues
1. Every 10 seconds, the Bluetooth stream goes silent for a moment and then comes back. After observing the I2S signal, there is no issue with that, so there must be something going on with the actual Bluetooth connection between the BC127 and the speaker. My current theory is that the BC127 will only connect with the SBC codec when it is acting as a source. When I connect my phone to the speakers, it uses the AAC codec. I believe that the periodic drop is due to the speaker not being able to handle the SBC bitrate. This issue occurs with both mono and stereo files I have contacted SierraWireless support regarding this but they have not been able to explain. Another issue I have is that when I configure the BC127 to use 44.1 KHz audio, it still connects to the speaker at 48 KHz. I have found posts from others online with this same issue but so far no one has had any answers. If I configure the BC127 to use 44.1 KHz audio, it will still connect at 48 KHz, but the audio will not cut out every 10 seconds. This is not a viable option though because the audio will be slowed down and there are frequent pops and crackles.
1. The BC127 datasheet shows the voltage range as 3.3 - 4.2 VDC. However, when the battery voltage drops below ~3.8 VDC, the device will initially have difficulty connecting or maintaining a Bluetooth link, and as the voltage drops further, the device may enter a perpetual reboot cycle. Initially I thought that for some reason there was a voltage drop across the power on circuit and while the battery may be 3.8 VDC, the actual voltage in was less. I observed the voltage with the oscilloscope and saw that this was not the case. My next step in debugging this will be to bypass the external charging transistor (BCX51T1). This was implemented based off of what was at the time, the current application note for the BC127. In more recent versions of the application note, this transistor is not present. It is made clear that this transistor is for external charging, and to not include it if you are only using the internal charging circuit. 

## Lessons Learned
1. The BC127 module has proven difficult to work with. I believe this can be attributed to the small user base and sometimes unclear documentation.
1. A development board is a valuable investment and would save significant time in future projects.
1. The HAL drivers are useful, but do not get too hung up on using them. Direct register access can be quicker and easier.
1. If a new section of code is written and tested, clean it up before moving on (or write it properly in the first place, but when trying new things this can take too much time), or else the code ends up being messy later.
1. In general I have gained a much better understanding of the operation of the microcontroller and organization and design of code.

## Future Work Ideas
1. Redo the PCB layout with a 4-layer PCB. This should allow for size reduction and proper signal routing.
1. Implement a discrete MP3 codec to decode MP3's. By allowing the use of MP3s, the file size would be significantly smaller. MP3s are also more common for music libraries.
1. Allow the user to access the SD card via the micro USB port, which is currently only used for charging.
