#ifndef DRIVER_GPS_H
#define DRIVER_GPS_H

#include <stdbool.h>
#include <stdint.h>

// Require a valid NMEA "*hh" checksum on every sentence (recommended).
// Set to 0 only for a receiver that is known not to send checksums.
#ifndef GPS_REQUIRE_CHECKSUM
#define GPS_REQUIRE_CHECKSUM	1
#endif

extern char gTime[16];		// "hhmmss.sss" (UTC), zero filled when unknown
extern char gLatY[16];		// "ddmm.mmmm"  - only ever holds a validated value
extern char gLatS[ 2];		// "N" / "S"
extern char gLonX[16];		// "dddmm.mmmm" - only ever holds a validated value
extern char gLonS[ 2];		// "E" / "W"
extern char gAlti[8];		// altitude as text (for the shell), may be truncated
extern bool gGPS_Fix;

extern int32_t gAltitudeM;	// altitude above mean sea level, rounded to metres
extern bool    gAltitudeValid;	// false until a GGA sentence with a usable altitude

// Called from the USART2 interrupt for every received byte. Only assembles
// a line and flags it; it does no parsing.
void GPS_PushByte(uint8_t Data);

// Called from the main loop. Parses a pending line, if there is one.
void GPS_Task(void);

// Parses one complete, NUL terminated NMEA sentence starting at '$' (no CR/LF).
// NOTE: the buffer is modified in place (it is split into fields).
void GPSProcess(char *buffer);

#endif
