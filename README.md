# settime

HTTP-based time synchronization client for SymbOS on Amstrad CPC.

## Quick Start

```bash
./build-direct.sh
# Copy build/settime.com to your SymbOS system
# Run: settime.com
```

## What This Does

Fetches the current time from time.akamai.com via HTTP and sets the SymbOS system clock.

## Why HTTP and Not NTP?

**UDP_Receive is not implemented in the SymbOS Network Daemon.** NTP requires UDP, so we use HTTP instead.

## Project Files

### Source Code
- `src/timeclient-direct.c` - HTTP time client with direct syscall
- `src/timeset_direct.s` - Assembly wrapper for Device_TimeSet (#810F)
- `src/config.c` - Configuration file parser
- `include/config.h` - Configuration structure and function declarations

### Build and Configuration
- `build-direct.sh` - Build script
- `settime.cfg.example` - Example configuration file
- **Output:** `build/settime.com`

## Key Discoveries

1. **UDP_Receive not implemented** - Network daemon has stub functions
2. **Time_Set() C library is buggy** - Reads structure fields incorrectly
3. **Time2Obj() expects DOS format** - Not Unix timestamps
4. **Stack offset gotcha** - Char parameters promoted to 2-byte int

## Technical Details

### Time Source
- **Server:** time.akamai.com
- **Protocol:** HTTP GET on port 80
- **Response:** Plain text Unix timestamp (10 digits)
- **Reliability:** Akamai CDN - extremely reliable

### System Call
- **Function:** Device_TimeSet (#810F)
- **Registers:**
  - A = Second (0-59)
  - B = Minute (0-59)
  - C = Hour (0-23)
  - D = Day (1-31)
  - E = Month (1-12)
  - HL = Year (1900-2100) - **absolute**, not relative!
  - IXL = Timezone

### Why Direct Assembly?

The SCC C library's `Time_Set()` function has bugs. We bypass it and call the SymbOS system function directly in assembly (`src/timeset_direct.s`).

**Implementation Details:**
- Assembly function `Time_Set_Direct()` is called from C
- When calling from C, char parameters are promoted to 2-byte int on the stack
- Parameters are spaced every 2 bytes (critical for correct stack layout)
- Uses RST #0x20 instruction to invoke SymbOS system call #810F
- Properly handles register mapping (A, B, C, D, E, HL, IXL) for Device_TimeSet

## Build Requirements

- **SymbOS C Compiler (SCC)** - Required to build this project
  - Get it from: https://github.com/danielgaskell/scc
  - Default path expected: `~/Dev/LEISURE/scc`
  - Update `CC` path in `build-direct.sh` if installed elsewhere

- Target: SymbOS 4.x on Amstrad CPC
- Network: Net4CPC with W5100S chip or Duke's M4 or any other network device

## Usage

Basic usage (no config file):
```
settime.com
```

Expected output:
```
HTTP Time Client

Fetching time... OK
Setting: 2026-05-02 18:35:26 (UTC)
System clock updated!
```

With timezone config:
```
HTTP Time Client
Timezone: UTC+2

Fetching time... OK
Setting: 2026-05-02 20:35:26 (UTC+2)
System clock updated!
```

Then verify:
```
date
```

Should show the correct date and time.

### Running at Startup

This application can be added to the SymbOS Autostart menu to automatically synchronize the system clock on boot. Simply add `settime.com` to your autostart configuration, and the time will be set automatically each time SymbOS starts.


## Configuration File

Create a file named `settime.cfg` in the same directory as `settime.com` to customize settings:

```
# Time server (optional - defaults to time.akamai.com)
server=time.akamai.com

# Timezone offset from UTC (optional - defaults to 0/UTC)
# Examples: +1 (CET), +2 (CEST), -5 (EST), -8 (PST)
timezone=+2
```

See `settime.cfg.example` for a complete example.

**Configuration Notes:**
- Configuration parsing is handled by `src/config.c`
- Server can be specified with or without `http://` prefix
- Server must return plain text Unix timestamp (like time.akamai.com)
- Timezone uses standard UTC offset format (+/- hours from UTC)
- Timezone adjusts the displayed/set time by the specified offset
- If config file is not found, uses defaults (time.akamai.com, UTC)
- Both settings are optional - can specify one, both, or neither
- Supports comments (lines starting with # or ;)

## Troubleshooting

### "Error: Network daemon not running"
- Start the network daemon first
- Check with `wget` or another network tool

### "failed" during fetch
- Check network connection
- Try: `wget http://time.akamai.com/` from SymbOS

### Date still wrong after running
- Make sure you copied the latest `build/settime.com`
- Verify network connectivity with `wget http://time.akamai.com/`
- Check that the SymbOS system clock accepts the values

## Credits

- Network daemon by Prodatron (SymbiosiS)
- Net4CPC drivers by d_kef
- SymbOS by Prodatron
- Time client implementation vibe-coded by salvogendut

## References

- SymbOS Developer Docs: https://github.com/Prodatron/symdoc-developer
- Net4CPC: Hardware by d_kef
- W5100S: WIZnet Ethernet controller

---

**Status:** ✅ Working  
**Last Updated:** 2026-05-03  
**Version:** 1.0
