#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "driver/gps.h"

#ifdef __arm__
	#include <at32f421.h>
#endif

/*
 * NMEA 0183 receiver (RMC + GGA only).
 *
 *  - GPS_PushByte() runs in the USART2 interrupt: it only assembles one line
 *    and hands it over. Nothing is parsed there.
 *  - GPS_Task() runs in the main loop and parses the line, so the position
 *    globals are only ever written from the same context that reads them
 *    when building an APRS packet.
 *  - Every sentence must pass the checksum, every field is validated, and the
 *    position/altitude globals are only updated from a fully valid sentence.
 */

#define GPS_LINE_MAX	100		// NMEA max is 82 chars; margin for odd receivers
#define GPS_MAX_FIELDS	16

char gTime[16];
char gLatY[16];
char gLatS[ 2];
char gLonX[16];
char gLonS[ 2];
char gAlti[ 8];
bool gGPS_Fix;

int32_t gAltitudeM;
bool    gAltitudeValid;

// ISR -> main loop hand-over
static char rxLine[GPS_LINE_MAX];			// being assembled by the ISR
static uint8_t rxLen;
static char gpsLine[GPS_LINE_MAX];			// complete line waiting for the task
static volatile bool gpsLineReady;

static const char gEmpty[] = "";

#define GPS_BARRIER()	__asm__ volatile ("" ::: "memory")

static inline uint32_t IrqSave(void)
{
#ifdef __arm__
	uint32_t Primask = __get_PRIMASK();
	__disable_irq();
	return Primask;
#else
	return 0;
#endif
}

static inline void IrqRestore(uint32_t Primask)
{
#ifdef __arm__
	__set_PRIMASK(Primask);
#else
	(void)Primask;
#endif
}

/* ============================================================ interrupt side */

void GPS_PushByte(uint8_t Data)
{
	if (Data == '$') {				// start of sentence, always resync
		rxLen = 0;
		rxLine[rxLen++] = '$';
		return;
	}
	if (rxLen == 0) {				// still waiting for a '$'
		return;
	}
	if (Data == '\r' || Data == '\n') {		// end of sentence
		if (!gpsLineReady) {			// otherwise drop it: the task is behind
			rxLine[rxLen] = 0;
			memcpy(gpsLine, rxLine, rxLen + 1U);
			GPS_BARRIER();
			gpsLineReady = true;
		}
		rxLen = 0;
		return;
	}
	if (rxLen < GPS_LINE_MAX - 1U) {
		rxLine[rxLen++] = (char)Data;
	} else {
		rxLen = 0;				// too long to be NMEA: discard
	}
}

/* ================================================================== helpers */

static int HexVal(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}

// Verifies "$....*hh" and cuts the string at the '*'.
static bool CheckChecksum(char *Line)
{
	uint8_t Sum = 0;
	char *p = Line + 1;
	int Hi, Lo;

	while (*p && *p != '*') {
		Sum ^= (uint8_t)*p++;
	}
	if (*p != '*') {
		return !GPS_REQUIRE_CHECKSUM;
	}
	Hi = HexVal(p[1]);
	if (Hi < 0) return false;
	Lo = HexVal(p[2]);
	if (Lo < 0 || p[3] != 0) return false;
	if (Sum != (uint8_t)((Hi << 4) | Lo)) return false;
	*p = 0;
	return true;
}

static uint8_t SplitFields(char *s, const char **f, uint8_t Max)
{
	uint8_t n = 0;

	f[n++] = s;
	while (*s) {
		if (*s == ',') {
			*s = 0;
			if (n < Max) {
				f[n++] = s + 1;
			}
		}
		s++;
	}
	return n;
}

static const char *Field(const char **f, uint8_t n, uint8_t Index)
{
	return (Index < n) ? f[Index] : gEmpty;
}

static bool IsDigit(char c)
{
	return c >= '0' && c <= '9';
}

// "ddmm.mmmm" (DegDigits = 2) or "dddmm.mmmm" (DegDigits = 3).
// Needs at least two decimals because the APRS packet uses "ddmm.hh".
static bool IsCoordinate(const char *s, uint8_t DegDigits, uint16_t MaxDeg, uint8_t BufSize)
{
	uint8_t i;
	uint16_t Deg = 0;
	uint8_t Min;
	uint8_t Frac = 0;

	for (i = 0; i < DegDigits; i++) {
		if (!IsDigit(s[i])) return false;
		Deg = Deg * 10U + (uint16_t)(s[i] - '0');
	}
	if (!IsDigit(s[i]) || !IsDigit(s[i + 1])) return false;
	Min = (uint8_t)((s[i] - '0') * 10 + (s[i + 1] - '0'));
	i += 2;
	if (s[i++] != '.') return false;
	while (IsDigit(s[i])) {
		i++;
		Frac++;
	}
	if (s[i] != 0) return false;
	if (Frac < 2 || i >= BufSize) return false;
	return Deg <= MaxDeg && Min < 60;
}

static bool IsHemisphere(const char *s, char A, char B)
{
	return (s[0] == A || s[0] == B) && s[1] == 0;
}

// Decimal text -> tenths (truncated), e.g. "604.7" -> 6047, "-12.34" -> -123.
// At most 5 integer digits, so the result always fits easily.
static bool ParseTenths(const char *s, int32_t *Out)
{
	bool Neg = false;
	int32_t v = 0;
	uint8_t Digits = 0;

	if (*s == '-') {
		Neg = true;
		s++;
	} else if (*s == '+') {
		s++;
	}
	while (IsDigit(*s)) {
		if (++Digits > 5) return false;
		v = v * 10 + (*s - '0');
		s++;
	}
	if (Digits == 0) return false;
	v *= 10;
	if (*s == '.') {
		s++;
		if (IsDigit(*s)) {
			v += *s - '0';
			s++;
		}
		while (IsDigit(*s)) s++;		// ignore further decimals
	}
	if (*s != 0) return false;
	*Out = Neg ? -v : v;
	return true;
}

static void CopyField(char *Dst, uint8_t Size, const char *Src)
{
	uint8_t i = 0;

	while (i < Size - 1U && Src[i]) {
		Dst[i] = Src[i];
		i++;
	}
	while (i < Size) {
		Dst[i++] = 0;
	}
}

static bool IsTime(const char *s)
{
	uint8_t i;

	for (i = 0; i < 6; i++) {
		if (!IsDigit(s[i])) return false;
	}
	return true;
}

// Validates the position fields. Nothing is stored here.
static bool PositionOk(const char *Lat, const char *NS, const char *Lon, const char *EW)
{
	return IsCoordinate(Lat, 2, 90, sizeof(gLatY)) && IsHemisphere(NS, 'N', 'S')
	    && IsCoordinate(Lon, 3, 180, sizeof(gLonX)) && IsHemisphere(EW, 'E', 'W');
}

// Stores an already validated position. Interrupts are masked for the few
// microseconds of the copy so the shell (USART1 interrupt) never reads a mix
// of old and new data.
static void CommitPosition(const char *Time, const char *Lat, const char *NS, const char *Lon, const char *EW)
{
	uint32_t Primask = IrqSave();

	CopyField(gTime, sizeof(gTime), IsTime(Time) ? Time : gEmpty);
	CopyField(gLatY, sizeof(gLatY), Lat);
	CopyField(gLatS, sizeof(gLatS), NS);
	CopyField(gLonX, sizeof(gLonX), Lon);
	CopyField(gLonS, sizeof(gLonS), EW);
	IrqRestore(Primask);
}

/* ================================================================ sentences */

static void ParseRMC(const char **f, uint8_t n)
{
	// $xxRMC,time,status,lat,NS,lon,EW,spd,cog,date,mv,mvEW[,mode[,navStatus]]
	const char *Status = Field(f, n, 2);
	const char *Mode   = Field(f, n, 12);

	// 'A' = valid. Mode (NMEA 2.3+) is optional; when present it must not be
	// N (no fix), E (estimated), M (manual) or S (simulator).
	if (Status[0] != 'A' || Status[1] != 0 || (Mode[0] != 0 && (Mode[1] != 0 || strchr("ADPRF", Mode[0]) == NULL))) {
		gGPS_Fix = false;
		return;
	}
	if (!PositionOk(Field(f, n, 3), Field(f, n, 4), Field(f, n, 5), Field(f, n, 6))) {
		gGPS_Fix = false;
		return;
	}
	CommitPosition(Field(f, n, 1), Field(f, n, 3), Field(f, n, 4), Field(f, n, 5), Field(f, n, 6));
	gGPS_Fix = true;
}

static void ParseGGA(const char **f, uint8_t n)
{
	// $xxGGA,time,lat,NS,lon,EW,quality,numSV,HDOP,alt,altUnit,sep,sepUnit,...
	const char *Quality = Field(f, n, 6);
	const char *Unit    = Field(f, n, 10);
	int32_t Tenths;

	// 1 = GPS, 2 = DGPS, 3 = PPS, 4 = RTK, 5 = float RTK. 0 = no fix.
	if (Quality[0] < '1' || Quality[0] > '5' || Quality[1] != 0) {
		gGPS_Fix = false;
		gAltitudeValid = false;
		return;
	}
	if (!PositionOk(Field(f, n, 2), Field(f, n, 3), Field(f, n, 4), Field(f, n, 5))) {
		gGPS_Fix = false;
		gAltitudeValid = false;
		return;
	}
	CommitPosition(Field(f, n, 1), Field(f, n, 2), Field(f, n, 3), Field(f, n, 4), Field(f, n, 5));

	if ((Unit[0] == 0 || (Unit[0] == 'M' && Unit[1] == 0)) && ParseTenths(Field(f, n, 9), &Tenths)) {
		uint32_t Primask = IrqSave();

		CopyField(gAlti, sizeof(gAlti), Field(f, n, 9));
		gAltitudeM = (Tenths >= 0 ? Tenths + 5 : Tenths - 5) / 10;	// round to nearest metre
		gAltitudeValid = true;
		IrqRestore(Primask);
	} else {
		gAltitudeValid = false;
	}
	gGPS_Fix = true;
}

void GPSProcess(char *buffer)
{
	const char *f[GPS_MAX_FIELDS];
	uint8_t n;

	if (buffer[0] != '$' || strlen(buffer) < 6) {
		return;
	}
	if (!CheckChecksum(buffer)) {
		return;
	}
	n = SplitFields(buffer + 1, f, GPS_MAX_FIELDS);	// f[0] = "GNRMC", "GPGGA", ...

	if (strlen(f[0]) != 5) {
		return;
	}
	if (strcmp(f[0] + 2, "RMC") == 0) {
		ParseRMC(f, n);
	} else if (strcmp(f[0] + 2, "GGA") == 0) {
		ParseGGA(f, n);
	}
}

void GPS_Task(void)
{
	if (!gpsLineReady) {
		return;
	}
	GPS_BARRIER();
	GPSProcess(gpsLine);
	GPS_BARRIER();
	gpsLineReady = false;
}
