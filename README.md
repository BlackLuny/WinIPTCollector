# WinIPTCollector
WinIPTCollector is a tool for collecting INTEL Processor Trace (IPT) package for any specific process and for specific virtual address ranges(ADDR_N) on Windows platform without Performace Monitor Interrupt(PMI).
Such features are:
1. Support CR3 filter, ADDR_N filter without PMI(that is no #BSOD#);
2. Realtime data collecting and saving as files without package data losing;
3. configurable(config.ini);


Please reference Intel manual for more about Intel Processor trace technology.

## Want to try?

### Step 1: Download Release

### Step 2:
Install and start the driver "WinIPTCollector.sys";

### Step 3:
```bat
   APPExample.exe [Process name] [output path director]
```
such as:
```bat
   APPExample.exe notepad.exe d:\
```
### Step 4:
   Waiting until you pressing 'Ctrl+C' to cancle collecting.
