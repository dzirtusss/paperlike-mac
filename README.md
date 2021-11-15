# paperlike-mac

Alternative MacOS implementation of software monitor controls for **Dasung Paperlike HD**
great eInk monitor.

To compile:
```
git clone https://github.com/dzirtusss/paperlike-mac
cd paperlike-mac
make
```

To use:
```
paperlike clean
```

## Motivation
- Dasung's native Paperlike HD software looks poorly implemented
- it sits in background and eats about 10-20% of CPU w/o any "reasonable" reasons
- there is no possibilitites to use it from scripts
- only Intel binary, no Apple Silicon, and I guess will not be

## Differences vs native Dasung software
- cli interface vs UI
- does not run any background process
- written in pure C, no any other languages/frameworks except Apple APIs
- should work on Apple Silicon (will test once I'll have it)
- open source
- can be easily extended with other commands and functionality

## Currently implemented
- finding Paperlike monitor in multi-monitor setup
- correctly sending DDCCI commands via HDMI connection
- correctly build Dasung's internal messages format
- sending 'clean' command

Actually atm, I personally don't need other commands except clean, but as information about
all other commands is available, it is rather trivial to add those. Just email me if interested.

## Good resources found:
- https://glenwing.github.io/docs/VESA-DDCCI-1.1.pdf
- https://github.com/leoluk/paperlike-go
- https://github.com/dzirtusss/Paperlike-Pro2019
- https://github.com/phlntn/ddc
- https://github.com/jontaylor/DDC-CI-Tools-for-OS-X
- https://github.com/kfix/ddcctl
