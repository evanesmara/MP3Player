#include "wave.h"
#include "pff2/src/pff.h"
//#include "startup/printf_P.h"
#include "../startup/lpc2xxx.h"
#include "../startup/general.h"
#include "../startup/config.h"
#include "timer.h"
#include "led.h"
#include "lcd.h"

#define CRYSTAL_FREQUENCY FOSC
#define PLL_FACTOR        PLL_MUL
#define VPBDIV_FACTOR     PBSD

//#define NULL 0


static void timer0ISR(void) __attribute__ ((interrupt));
static uint8 wave_stream_buf[512 * 1];//1 sectors
static void irqIni(tU16 czestotliwosc, void(*pCallback)());

static WORD bytesReaded;
static FRESULT rc;

static WAVEHEADER wh;
static FmtChunk ws1;
static DataChunk ws2;

static tBool koniecPetli;
static tBool warunekKoncowy;

static void (*pDelayCallback)(void) = NULL;
static void callback1(void);

static void zatrzymajIrq() {
	koniecPetli = TRUE;
	T0TCR = 0x00000002; //disable and reset Timer0
	VICIntEnable &= ~0x10; //disable timer0 interrupt
}

static void timer0ISR(void) {
	//call delay callback, if registered
	//if (pDelayCallback != NULL)
	if (koniecPetli == FALSE)
		(*pDelayCallback)();

	T0IR = 0xff; //reset all IRQ flags
	VICVectAddr = 0x00; //dummy write to VIC to signal end of interrupt
}

static void irqIni(tU16 czestotliwosc, void(*pCallback)()) {
	//register callback and reset expired flag
	pDelayCallback = pCallback;

	//initialize VIC for Timer0 interrupts
	VICIntSelect &= ~0x10; //Timer0 interrupt is assigned to IRQ (not FIQ)
	VICVectAddr4 = (tU32) timer0ISR; //register ISR address
	VICVectCntl4 = 0x24; //enable vector interrupt for timer0
	VICIntEnable = 0x10; //enable timer0 interrupt

	//initialize and start Timer #0
	T0TCR = 0x00000002; //disable and reset Timer0
	T0PC = 0x00000000; //no prescale of clock
	T0MR0 = (CRYSTAL_FREQUENCY * PLL_FACTOR) / (czestotliwosc * VPBDIV_FACTOR);//calculate no of timer ticks
	T0IR = 0x000000ff; //reset all flags before enable IRQs
	T0MCR = 0x00000003; //reset counter and generate IRQ on MR0 match
	T0TCR = 0x00000001; //start Timer0
}

tBool playerInit(void) {
	bytesReaded = 0;

	uint32 i = 0;

	//	printf("read data\n");
	rc = pf_read(wave_stream_buf, sizeof(wave_stream_buf), &bytesReaded);

	if (rc || !bytesReaded)
		return FALSE;

	//	printf("fill structs\n");
	//wave header
	wh.chunkId = wave_stream_buf[i++] << 24;
	wh.chunkId += wave_stream_buf[i++] << 16;
	wh.chunkId += wave_stream_buf[i++] << 8;
	wh.chunkId += wave_stream_buf[i++];

	wh.chunkSize = wave_stream_buf[i++];
	wh.chunkSize += wave_stream_buf[i++] << 8;
	wh.chunkSize += wave_stream_buf[i++] << 16;
	wh.chunkSize += wave_stream_buf[i++] << 24;

	wh.format = wave_stream_buf[i++] << 24;
	wh.format += wave_stream_buf[i++] << 16;
	wh.format += wave_stream_buf[i++] << 8;
	wh.format += wave_stream_buf[i++];

	//	printf("id=%x size=%x format=%x\n",wh.chunkId,wh.chunkSize,wh.format);

	ws1.subChunkId = wave_stream_buf[i++] << 24;
	ws1.subChunkId += wave_stream_buf[i++] << 16;
	ws1.subChunkId += wave_stream_buf[i++] << 8;
	ws1.subChunkId += wave_stream_buf[i++];

	ws1.subChunkSize = wave_stream_buf[i++];
	ws1.subChunkSize += wave_stream_buf[i++] << 8;
	ws1.subChunkSize += wave_stream_buf[i++] << 16;
	ws1.subChunkSize += wave_stream_buf[i++] << 24;

	ws1.audioFormat = wave_stream_buf[i++];
	ws1.audioFormat += wave_stream_buf[i++] << 8;

	ws1.numChannels = wave_stream_buf[i++];
	ws1.numChannels += wave_stream_buf[i++] << 8;

	ws1.sampleRate = wave_stream_buf[i++];
	ws1.sampleRate += wave_stream_buf[i++] << 8;
	ws1.sampleRate += wave_stream_buf[i++] << 16;
	ws1.sampleRate += wave_stream_buf[i++] << 24;

	ws1.byteRate = wave_stream_buf[i++];
	ws1.byteRate += wave_stream_buf[i++] << 8;
	ws1.byteRate += wave_stream_buf[i++] << 16;
	ws1.byteRate += wave_stream_buf[i++] << 24;

	ws1.blockAlign = wave_stream_buf[i++];
	ws1.blockAlign += wave_stream_buf[i++] << 8;

	ws1.bitsPerSample = wave_stream_buf[i++];
	ws1.bitsPerSample += wave_stream_buf[i++] << 8;

	//printf("sid=%x ssize=%x audioformat=%x\n",ws1.subChunkId,ws1.subChunkSize,ws1.audioFormat);
	//printf("ch=%x sr=%x br=%x\n",ws1.numChannels,ws1.sampleRate,ws1.byteRate);
	//printf("block=%x bps=%x\n",ws1.blockAlign,ws1.bitsPerSample);

	ws2.subChunkId = wave_stream_buf[i++] << 24;
	ws2.subChunkId += wave_stream_buf[i++] << 16;
	ws2.subChunkId += wave_stream_buf[i++] << 8;
	ws2.subChunkId += wave_stream_buf[i++];

	ws2.subChunkSize = wave_stream_buf[i++];
	ws2.subChunkSize += wave_stream_buf[i++] << 8;
	ws2.subChunkSize += wave_stream_buf[i++] << 16;
	ws2.subChunkSize += wave_stream_buf[i++] << 24;

	ws2.byteToOmmit = i;

	//	printf("end init\n");
	//	printf("sid=%x ssize=%x i=%x\n",ws2.subChunkId,ws2.subChunkSize,ws2.byteToOmmit);

	return TRUE;
}

#define BUFFSIZE 512 * 5 //wielkoœæ bufora
#define BUFFCOUNT 2 //iloœæ buforów
uint8 buffer[BUFFCOUNT][BUFFSIZE]; //bufor na próbki

static uint8 bufferPlayed = 0; //numer bufora z którego s¹ aktualnie odczytywane dane
static uint8 bufferToDownload = 0; //numer bufora który nale¿y uzupe³niæ
static uint16 sample = 0; //dana próbka z bufora
static uint64 totalPlayed = 0; //iloœæ oftworzonych próbek
static tBool refil = FALSE; //czy bufor pusty

static PlayerStatus *status;
static TimerStatus *timer;

static uint32 uplynelo;
static uint32 czasCalkowity;
static uint32 pozostalo;

tBool playWave(PlayerStatus *status2, TimerStatus *timer2) {
	status = status2;
	timer = timer2;

	bufferPlayed = 0;
	bufferToDownload = 0;
	sample = 0;
	totalPlayed = 0;
	refil = FALSE;

	bytesReaded = 0;

	uint32 i = 0;
	pf_lseek(ws2.byteToOmmit);

	//wype³niamy na sam pocz¹tek czymœ
	do {
		rc = pf_read(buffer[i], sizeof(buffer[i]), &bytesReaded);
		i++;
	} while (i < BUFFCOUNT && !rc && bytesReaded);

	initLed(3);

	uplynelo = 0;
	czasCalkowity = ws2.subChunkSize / ws1.byteRate;
	//iloœæ próbek przez prêdkosæ odtwarzania
	pozostalo = 0;
	koniecPetli = FALSE;

	warunekKoncowy = TRUE;

	irqIni(ws1.byteRate, callback1);

	while (!koniecPetli)
		;
	//asm("NOP");

	return warunekKoncowy;
}
// funkcja ktora bedzie wywolywana przez timer z czestotliwoscia probkowania
static void callback1(void) {
	if (totalPlayed >= ws2.subChunkSize) {
		zatrzymajIrq();
	} else {
		if (*status == Player_Playing) {
			if (sample >= (BUFFSIZE)) {
				sample = 0;
				bufferToDownload = bufferPlayed++;
				refil = TRUE;
			}
			if (bufferPlayed >= BUFFCOUNT)
				bufferPlayed = 0;

			if (ws1.bitsPerSample == 8)
				DACR = buffer[bufferPlayed][sample++] << 5;
			else if (ws1.bitsPerSample == 16)
				DACR = ((buffer[bufferPlayed][sample++] << 8)
						+ (buffer[bufferPlayed][sample++])) << 5;
			//lcdGotoxy(0, 64);
			//lcdPuts();


			if (totalPlayed % ws1.byteRate == 0) {
				if (*timer == Timer_UP) {
					uplynelo = totalPlayed / ws1.byteRate;
					//printf("uplynelo: %d - ", uplynelo);
					//printf("%d\n", czasCalkowity );
					lcdGotoxy(0, 32);
					lcdPutsNumber(uplynelo);
					lcdPuts(" : ");
					lcdPutsNumber(czasCalkowity);
				} else {
					pozostalo = (ws2.subChunkSize - totalPlayed) / ws1.byteRate;

					lcdGotoxy(0, 32);
					if (pozostalo >= 100) {
						lcdPuts("-");
					} else if (pozostalo >= 10) {
						lcdPuts(" -");
					} else {
						lcdPuts("  -");
					}
					lcdPutsNumber(pozostalo);
				}

			}
		}

		//sprawdŸ przyciski
		updateStatus(status, timer, FALSE);

		//wyjdŸ z pêtli jeœli trzeba przerywaj¹c odtwarzanie
		if (*status != Player_Playing && *status != Player_Paused) {
			zatrzymajIrq();
			warunekKoncowy = FALSE;
		}

		if (warunekKoncowy == TRUE) {
			if (refil) { //uzupe³nij bufor nawet jak zapauzowane
				turnOnLed(3);
				rc = pf_read(buffer[bufferToDownload],
						sizeof(buffer[bufferToDownload]), &bytesReaded);
				refil = FALSE;
				turnOffLed(3);
			}
			/*
			 else if( refil == FALSE && *status == Player_Playing ){
			 if( totalPlayed % ws1.byteRate != 0 )
			 zegarek(1000000/(ws1.byteRate + 2500));
			 //trzeba przyœpieszyæ mimo wszystko o jakieœ 2500 próbek
			 //ekran bowiem daje za du¿e opóŸnienia :/ i tak to nie jest
			 //doskona³e
			 //gdy odczyt z karty, to on s³u¿y za opóxnienie, wiêc nie czekaj

			 }
			 */

			if (*status == Player_Playing) {
				totalPlayed++;
			}
		}
	}
}
