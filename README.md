# zegarbcd
Binary clock for Windows tray.
- displays time in BCD format (screenshot shows 21:37)
- shows date in a tooltip
- installs itself automatically on the first launch and has a menu option for uninstall
- listens to Windows session events to display correct time when the system gets unlocked
- colors can be configured

![obraz](https://github.com/user-attachments/assets/61d69e4b-72e7-4e72-aab6-9102bcbdcec5)
## Modes
- 24-hour: hhmm, above
- 12-hour: hmm (shows 7:20)

![obraz](https://github.com/user-attachments/assets/51cf817f-1d2c-4709-983c-46763ed0170f)

- experimental "humane clock" with 5 min precision

![obraz](https://github.com/user-attachments/assets/5689735a-828f-4a89-8186-4c02f1e1f01d)

The top row means hours since 1/3 day (7), the 3 columns below mean: 1/3 of day (1, hence 1*8+7=15), quarter (2, hence 30) and 5-min (0). The approximate hour is 15:30.
- a similar 5-min mode with the hour as the second column (shows 19:15)

![obraz](https://github.com/user-attachments/assets/6c44a794-dbb2-451f-bd8d-5cd7948a0db9)

## Experimental date for fun
Date can be seen by single clicking on the icon and lasts for 5 seconds. The date on screenshot is 9/12, Thursday.

![obraz](https://github.com/user-attachments/assets/9a0eee28-a8a2-4e6d-9842-2e16f7f83502)

# Prerequisities
- boost (set the location in CmakeLists.txt)
- Qt 6
