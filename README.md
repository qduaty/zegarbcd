# zegarbcd
Binary clock for Windows tray.
- displays time in BCD format (screenshot shows 21:37)
- shows date in a tooltip
- installs itself automatically on the first launch and has a menu option for uninstall
- listens to Windows session events to display correct time when the system gets unlocked

![obraz](https://github.com/user-attachments/assets/61d69e4b-72e7-4e72-aab6-9102bcbdcec5)
## Modes
- 24-hour: hhmm
- 12-hour: hmm
- 5 min precision: 1/3 day, hour, quarter, 5-min
## Known bugs
- leaves settings in registry after uninstall
# Prerequisities
- boost (set the location in CmakeLists.txt)
- Qt 6
