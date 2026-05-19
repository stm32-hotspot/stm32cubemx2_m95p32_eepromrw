# __Example: *m95p32_eepromrw*__

How to use the M95P32 part API.

It illustrates it by performing read and write operations on the EEPROM memory and displaying them on a terminal.


## __1. Detailed scenario__

__Initialization phase__: At the beginning of the `main()` function, the `mx_system_init()` function is called to initialize the peripherals.

The application executes the following __example steps__:
- __Step 1__: Initializes the M95P32 EEPROM
- __Step 2__: Validates read and write operations with single byte, test data, and test page opearations. The user needs to select the menu options provided in a interactive way by providing the required number of the desired option
- __Step 3__: Deinitializes M95P32 EEPROM

__End of example__: Runs in a loop at Step 2 based on user input.

You can follow these execution steps in the terminal logs:
```
[INFO] Step 1: M95P32 EEPROM init completed

**********************************
Main Menu
**********************************
Command Set            : 1
User Scenario          : 2
Specific Behaviour     : 3
Run Test Cases         : 4

**********************************

Enter Command Number :
```

- __Option 1 : Command Set (User can select different AT commands from docklight utility)__
```
Enter Command set:

[TX] - 05

[RX] - STATUS_REG value: 0x00
Enter Command set: 

[TX] - 06

[RX] - WRITE_ENABLE 
Enter Command set: 

[TX] - 05

[RX] - STATUS_REG value: 0x02
Enter Command set: 

[TX] - 04

[RX] - WRITE_DISABLE 
Enter Command set: 

[TX] - 05

[RX] - STATUS_REG value: 0x00
Enter Command set: 
```

- __Option 2 : User Scenario__
```
***************************************************
User Scenario
***************************************************
Program with safety register check          : 1
Write with safety register check            : 2
Buffer Mode                                 : 3
Enter any other option to return to Main Menu!
**************************************************
```
- __Program with safety register check__
```
Program with Safety Register Check:
Power UP Done.
Power UP OK.
Page Erase Done.
Page Erase Done without issues.
Page Erased Correctly.
NO ECC detected during Erase Check.
Page Program Done without issues.
Page Programmed Correctly.
NO ECC detected during Page Program Check.
```

- __Write with safety register check__
```
Write with Safety Register Check:
Power UP Done.
Power UP OK.
Page Write Done without issues.
Page Written Correctly.
NO ECC detected during Page Write Check.
Page Write Done without issues.
Page Written Correctly.
NO ECC detected during Page Write Check.
```

- __Buffer Mode__
```
Buffer Mode:
Power UP Done.
Power UP OK.
Safety Register status OK.
Data is in erased state. OK.
Data is in erased state. OK.
NO ECC correction detected. OK.
Last Page program is managed outside of an array.
Safety Register status OK.
BUFFEN bit is 0. OK.
Data programmed correctly.
Data programmed correctly.
NO ECC correction detected. OK.
```

- __Option 3 : Specific Behaviour__
```
***************************************************
Specific Behaviour
***************************************************
16 word align             : 1
Write in protected area   : 2
Enter any other option to return to Main Menu!
***************************************************

Enter Command Number :
```
- __16 word align__
```
16 word align:
Power UP Done.
Power UP OK.
Page Erased from address 0x00. Safety Register Value is: 0x0
PAMAF and ERF bit not set. OK.
Erased page data is in erased state <0xFF>. OK.
Safety Register Value is: 0x0
No ECC flag set. OK.

Programmed 1 byte from address 0x00 with value 0x55. Safety Register Value is: 0x0
No flag set. OK.
Read 1 byte from address 0x00. Safety Register Value is: 0x0
No ECC flag set. OK.

Programmed 1 byte from address 0x100 with value 0xA5. Safety Register Value is: 0x0
No Flag set. OK.
Data at address 0x100 is: 0xA5
Page Program successful
Read 1 byte from address 0x100. Safety Register Value is: 0x0
No ECC flag set. OK.

Programmed 1 byte from address 0x101 with value 0x55. Safety Register Value is: 0x10
PRF Flag set as expected!
Data at address 0x101 is: 0xFF
Page Program NOT successful
Safety Register Value is: 0x0
No ECC flag set. OK
```
- __Write in protected area__
```
Write in protected area
Power UP Done.
Power UP OK.
Status Register value is: 0x1C
Chip fully protected.
Data stored at address 0x600 is 0xFF
Attempt to write 0x87 at address 0x600. Safety Register Value is: 0xB0
PAMAF bit set, Modify operation to a protected memory area has been attempted.
Data stored at address 0x600 is 0xFF
NO ECC correction detected. OK.
Status Register value : 0x0
```
- __Option 4 : Run Test Cases__
```
[INFO] Test 1: READ STATUS REGISTER
[INFO] STATUS REGISTER : 0x0
[INFO] WRITE ENABLE
[INFO] STATUS REGISTER : 0x2
[INFO] WRITE DISABLE
[INFO] STATUS REGISTER : 0x0
        **************Test 1 Passed**************

                ______Test 2 Starts______
        Test case to verify Page Write and Read

 Write Page (100 Bytes) at Address 0x00


 Write Page                             : Passed


 Read data in Single SPI                : Passed
  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff

 Read data Fast in Single SPI           : Passed
  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  0f  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff
                                                                                                **************Test 2 Passed**************


                ______Test 3 Starts______

          Test Case to verify Page Erase

 *PAGE_ERASE* at Address 0x00
 Data after Page Erase
  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff


 Page Erase                             : Passed
        **************Test 3 Passed**************


                ______Test 4 Starts______
          Test Case to verify Sector Erase

 *Sector_ERASE* at Address 0x00
 Data after Sector Erase
  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff


 Sector Erase                           : Passed
        **************Test 4 Passed**************


                ______Test 5 Starts______

          Test Case to verify Block Erase
 *Block_ERASE* at Address 0x00
 Data after Block Erase
  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff

 Block Erase                            : Passed
        **************Test 5 Passed**************


                ______Test 6 Starts______

          Test Case to verify Page Program in Buffer Mode
Data is in erased state. OK.
Data is in erased state. OK.
NO ECC correction detected. OK.
Last Page program is managed outside of an array.
Safety Register status OK.
BUFFEN bit is 0. OK.
Data programmed correctly.
Data programmed correctly.
NO ECC correction detected. OK.

 Data after Programming 1024 bytes starting from address 0x00 in buffer mode
  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  09  0f  0f  0f  0f  0f  0f

 Buffer Mode                            : Passed
        **************Test 6 Passed**************
```


## __2. Example configuration__

This example demonstrates the following drivers:
- Part m95p32.c/.h
- Part m95p32/interfaces/spi/m95p32_io.c/.h

In this example the M95P32 component is configured through the SPI IO operations defined under interfaces/spi folder.
Once the SPI is initialized, the M95P32 can be initialized too through the call of m95p32_drv_init() API.
After this step, the EEPROM will be ready for read and write operations.


## __3. Hardware environment and setup__

### __3.1. Generic Setup__

This section describes the hardware setup principles that apply to any board.

### __3.2. Specific board setups__

<details>
<summary>On STM32C5 series.</summary>
  <summary>On board NUCLEO-C562RE.</summary>

  | Board connector | MCU pin | Signal name | ARDUINO <br> connector pin |
  | :-------------: | :-----: | :---------: | :------------------------: |
  |       CN5-6     |   PA5   |  SPI1_SCK   |          D13               |
  |       CN5-5     |   PA6   |  SPI1_MISO  |          D12               |
  |       CN5-4     |   PA7   |  SPI1_MOSI  |          D11               |
  |       CN5-3     |   PB6   |  WP         |          D10               |
  |       CN8-4     |   PB0   |  HOLD       |          A3                |
  |       CN5-1     |   PA9   |  CS         |          D8                |
  |       CN5-2     |   PC6   |  LED        |          D9                |
</details>

## __4. Software setup__

To create a functional project, complete the following steps:
- Select the appropriate IoC2 file based on the combination of NUCLEO and X-NUCLEO boards. For example, use c562re_pgeez1_m95p32_eepromrw.ioc2 for NUCLEO-C562RE and X-NUCLEO-PGEEZ1.
- Open the IoC2 file with STM32CubeMX2.
- Select the preferred toolchain and generate the source code.
- Copy the main.c, main.h, example.c, and example.h files into the project folder of the generated code.
- Open the Integrated Development Environment (IDE), add the example.c and example.h files to the project.
- Add the USE_TRACE=1 to the global variables of the project.
- Compile the project.

## __5. Troubleshooting__

No specific debug tips.


## __6. See Also__

More information about M95P32 part driver can be found in the [M95P32 Part Driver](https://dev.st.com/stm32cube-docs/part-drivers-m95p32/1.0.0/en/index.html)

More information about the STM32 ecosystem can be found in the [STM32 MCU Developer Zone](https://www.st.com/content/st_com/en/stm32-mcu-developer-zone.html).


## __7. License__

Copyright (c) 2026 STMicroelectronics.

This software is licensed under terms that can be found in the LICENSE file in the root directory
of this software component.
If no LICENSE file comes with this software, it is provided AS-IS.

