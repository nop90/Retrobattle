/* WavLib.h
 * COMS30123
 * Michael McTernan, mm7323@bris.ac.uk
 * 17/02/00
 */

#ifndef WAVLIB_HEADER
#define WAVLIB_HEADER

/* RIFF Definition */
/* Taken from Assignment handout
 */
typedef struct WAVHeader{     
        char riff[4];         /* "RIFF" 4 bytes */ 
        long TotLen;          /* Total length 4 bytes */ 
        char wavefmt[8];      /* "WAVEfmt " 8 bytes */ 
        long Len;             /* Remaining length 4 bytes */ 
        short format_tag;     /* Tag (1 = PCM) 2 bytes */ 
        short channels;       /* Mono=1 Stereo=2 2 bytes */ 
        long SamplesPerSec;   /* No samples/sec 4 bytes */ 
        long AvgBytesPerSec;  /* Average bytes/sec 4 bytes */ 
        short BlockAlign;     /* Block align 2 bytes */ 
        short wBitsPerSample; /* 8 or 16 bit 2 bytes */ 
        char data[4];         /* "data" 4 bytes */ 
        long datalen;         /* Raw data length 4 bytes */ 

} WAVHeader_t;

/* The structure for a cuepoint */
/* Found on the web
 */
typedef struct cuepoint {
  long dwName;
  long dwPosition;
  char fccChunk[4];
  long dwChunkStart;
  long dwBlockStart;
  long dwSampleOffset;
  struct cuepoint *Next;  /* This doesn't get saved to the file! */
} wavcuepoint_t;

/* The type for the ADT */
typedef struct WAVInfo {
  WAVHeader_t WAVHead;
  long        CueCount,Elements;
  wavcuepoint_t *CueList,*CueEnd,*CueIterator;
  void       *RawData;
} WAVInfo;

typedef WAVInfo * WAVInfo_t;

typedef struct {
  long Name;
  long Position;
} cuepoint_t;

WAVInfo_t  WAV_New          (int Bits,int Channels,int SampleRate);
WAVInfo_t  WAV_Parse		(unsigned char *buffer);
WAVInfo_t  WAV_Open         (const char *File);
void      *WAV_GetData      (WAVInfo_t W,long *Elements);
void       WAV_AddCuePoint  (WAVInfo_t W,long Position);
int        WAV_GetCueCount  (WAVInfo_t W);
cuepoint_t WAV_CueIterate   (WAVInfo_t W);
void       WAV_ResetIterator(WAVInfo_t W);
void       WAV_Write        (WAVInfo_t W,char *Filename);
int        WAV_Channels     (WAVInfo_t W);
short      WAV_BitsPerSample(WAVInfo_t W);
long       WAV_SampleFreq   (WAVInfo_t W);
void       WAV_SetData      (WAVInfo_t W,void *Data,long Elements);
void       WAV_Free         (WAVInfo_t W);

#endif
