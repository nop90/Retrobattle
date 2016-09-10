
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
//#include "modplug.h"
#include "SDL_3ds.h"
#include "WavLib.h"

bool soundEnabled;

typedef struct chStatus {
	u64 playlen;	
	u64 playstart;		
} chStatus;

chStatus soundchannels[8];

FSOUND_SAMPLE SFX[NUMSFX];

int SFXMasterVolume = 64;
int MasterVolume = 64;
int frequency=0;
int channel=0;

const char* stdMixErr = "SDL MIX ERROR";

int getFreeChannel()
{
	int startchannel = channel; 
	do
	{
		channel = (channel+1)%7;
		if (svcGetSystemTick()> soundchannels[channel].playlen + soundchannels[channel].playstart) return channel;
	} while(channel != startchannel);
	return -1;
}

void soundInit()
{
	int i;
	for(i=0;i<NUMSFX;i++)
	{
		SFX[i].used=false;
	}

	for(i=0;i<8;i++)
	{
		soundchannels[i].playlen=0;
		soundchannels[i].playstart=0;
	}


	if(csndInit()==0)soundEnabled=true;
	else soundEnabled=false;
}

void soundClose()
{
	int i;
	for(i=0;i<NUMSFX;i++)
	{
		if(SFX[i].used)
		{
			if(SFX[i].data)
			{
				linearFree(SFX[i].data);
				SFX[i].data=NULL;
			}
			SFX[i].used=false;
		}
	}
	if(soundEnabled)csndExit();
}

FILE* openFile(const char* fn, const char* mode)
{
	if(!fn || !mode)return NULL;
	return fopen(fn, mode);
}

void* bufferizeFile(const char* filename, u32* size, bool binary, bool linear)
{
	FILE* file;
	
	if(!binary)file = openFile(filename, "r");
	else file = openFile(filename, "rb");
	
	if(!file) return NULL;
	
	u8* buffer;
	long lsize;
	fseek (file, 0 , SEEK_END);
	lsize = ftell (file);
	rewind (file);
	if(linear)buffer=(u8*)linearMemAlign(lsize, 0x80);
	else buffer=(u8*)malloc(lsize);
	if(size)*size=lsize;
	
	if(!buffer)
	{
		fclose(file);
		return NULL;
	}
		
	fread(buffer, 1, lsize, file);
	fclose(file);
	return buffer;
}
void* parsewavbuffer(unsigned char* wav, u32* size,  bool linear, int * freq, int * bps)
{
	u8 *WAVData, *buffer;
	long      WAVElements;
	WAVInfo_t WaveFile;
	WaveFile=WAV_Parse(wav);
	if (!WaveFile) 
		return NULL;
	WAVData= (u8*) WAV_GetData(WaveFile,&WAVElements);
	
	if(!WAVData)
	{
		WAV_Free (WaveFile);
		return NULL;
	}
	// gets only first channel
	u8 step = WAV_Channels (WaveFile);
	int samplesize=WAV_BitsPerSample(WaveFile);
	* bps = samplesize;
	* freq = WAV_SampleFreq(WaveFile);
 
	if (samplesize==8)
	{
		if(linear)buffer=(u8*)linearMemAlign(WAVElements, 0x80);
		else buffer=(u8*)malloc(WAVElements);
		if(size)*size=WAVElements;
	
		if(!buffer)
		{
			free(WAVData);
			WAV_Free (WaveFile);
			return NULL;
		}
		long count;
		for(count=0; count < WAVElements; count++) 
			buffer[count]=WAVData[count*step];
		free(WAVData);
		WAV_Free (WaveFile);
		return buffer;
	} else if (samplesize==16) {
		if(linear)buffer=(u8*)linearMemAlign(WAVElements*2, 0x80);
		else buffer=(u8*)malloc(WAVElements*2);
		if(size)*size=WAVElements;
	
		if(!buffer)
		{
			free(WAVData);
			WAV_Free (WaveFile);
			return NULL;
		}
		long count;
		for(count=0; count < WAVElements; count++)
			((u16*)buffer)[count]=((u16*)WAVData)[count*step];
		free(WAVData);
		WAV_Free (WaveFile);
		return buffer;
	} else {
		free(WAVData);
		WAV_Free (WaveFile);
		return NULL;
	}

}


int FSOUND_Init(u32 freq, u32 bps, u32 unkn)
{

	frequency = freq;
	
	return 0;//soundEnabled;
}

void initSFX(FMUSIC_MODULE* s)
{
	if(!s)return;

	s->data=NULL;
	s->size=0;
	s->used=true;
	s->loop=false;
}

void load_SFX(FMUSIC_MODULE* s, const char* filename, u32 format)
{
	if(!s)return;

	initSFX(s);

	s->data=(u8*) bufferizeFile(filename, &s->size, true, true);
	s->format=format;
	s->freq=frequency;
}

int FSOUND_GetSFXMasterVolume()
{
	return SFXMasterVolume;
}

int FMUSIC_GetMasterVolume(FMUSIC_MODULE* s)
{
	return MasterVolume;
}

void FMUSIC_SetMasterVolume(FMUSIC_MODULE* s, u8 volume)
{
	MasterVolume = volume;
}

void FSOUND_SetSFXMasterVolume(u8 volson)
{
	SFXMasterVolume = volson;
}

void FSOUND_PlaySound(int ch,FSOUND_SAMPLE* s)
{
	if(!s || !s->used || !s->data || !soundEnabled || SFXMasterVolume == 0)return;

	int freech = getFreeChannel();
	if (freech>=0)
	{
		soundchannels[freech].playlen= ((u64)s->size * 1000 * TICKS_PER_MSEC) / s->freq;
		soundchannels[freech].playstart=svcGetSystemTick();
		csndPlaySound(freech+8, s->format, s->freq, 1.0, 0.0, (u32*)s->data, (u32*)s->data, s->size);
	}
}

void FMUSIC_StopSong(FMUSIC_MODULE* s)
{
	CSND_SetPlayState(15, 0);//Stop music audio playback.
	csndExecCmds(0);
}

void FMUSIC_PlaySong(FMUSIC_MODULE* s)
{
	int flag;
	if(!s || !s->used || !s->data || !soundEnabled || MasterVolume == 0)return;
	flag = s->format;
	if(s->loop) flag |= SOUND_REPEAT;
	soundchannels[7].playlen= ((u64)s->size * 1000 * TICKS_PER_MSEC) / s->freq;
	soundchannels[7].playstart=svcGetSystemTick();
	csndPlaySound(15, flag,s->freq, 1.0, 0.0, (u32*)s->data, (u32*)s->data, s->size);
}


FSOUND_SAMPLE* FSOUND_Sample_Load(int flag, const char * f,int a, int b, int c)
{
	int i;
	for(i=0;i<NUMSFX;i++)
	{
		if(!SFX[i].used)
		{
			load_SFX(&SFX[i], f, SOUND_FORMAT_16BIT);

			if(!SFX[i].data)return NULL;
			SFX[i].used = true;
			SFX[i].loop=false;
			return &SFX[i];
		}
	}
	return NULL;
}

FMUSIC_MODULE* FMUSIC_LoadSong(const char * f)
{
//	int size;
	int i;
	for(i=0;i<NUMSFX;i++)
	{
		if(!SFX[i].used)
		{
			load_SFX(&SFX[i], f, SOUND_FORMAT_16BIT);
			
			if(!SFX[i].data) return NULL;
			SFX[i].used = true;
			SFX[i].loop=false;
			return &SFX[i];
		}
	}
	return NULL;
}

void FSOUND_Close(){
	soundClose();
}


void FMUSIC_SetLooping(FMUSIC_MODULE* s, bool flag)
{
	if (s)
		s->loop=flag;
}

void FSOUND_Sample_Free(FSOUND_SAMPLE* s)
{
	if(s) {
		if (s->data)
			linearFree(s->data);
		s->size=0;
		s->used=false;
		s->loop=false;
	}
}


void FMUSIC_FreeSong(FMUSIC_MODULE* s)
{
	if(s) {
		if (s->data)
			linearFree(s->data);
		s->size=0;
		s->used=false;
		s->loop=false;
	}
}

int Mix_PlayChannel(int ch, Mix_Chunk *s, int loops)
{
	if(!s || !s->used || !s->data || !soundEnabled || SFXMasterVolume == 0) return -1;

	if(loops) s->format |= SOUND_REPEAT;
	else s->format &= ~SOUND_REPEAT;
	int freech = getFreeChannel();
	if (freech>=0)
	{
		soundchannels[freech].playlen= ((u64)s->size * 1000 * TICKS_PER_MSEC) / s->freq;
		soundchannels[freech].playstart=svcGetSystemTick();
		csndPlaySound(freech+8, s->format, s->freq, 1.0, 0.0, (u32*)s->data, (u32*)s->data, s->size);
	}
	return freech;
}

int Mix_PlayMusic(Mix_Music * s , int loops )
{
	int flag;
	if(!s || !s->used || !s->data || !soundEnabled || MasterVolume == 0) return -1;
	flag = s->format;
	if (loops) flag |= SOUND_REPEAT;
	soundchannels[7].playlen= ((u64)s->size * 1000 * TICKS_PER_MSEC) / s->freq;
	soundchannels[7].playstart=svcGetSystemTick();
	csndPlaySound(15, flag, s->freq, 1.0, 0.0, (u32*)s->data, (u32*)s->data, s->size);
	return 0;
}

int Mix_PlayingMusic()
{
	return (svcGetSystemTick()< soundchannels[7].playlen + soundchannels[7].playstart)?1:0;
}

int Mix_OpenAudio( int audio_rate, u16 audio_format, int audio_channels, int bufsize ){
	return FSOUND_Init(audio_rate, 0, 0);
}

void Mix_VolumeMusic( int vol ){}
void Mix_Volume(  int c, int vol ){}

Mix_Chunk * Mix_LoadWAV(const char * f) {
	u32 size;
	u8 * buf = (u8*) bufferizeFile(f, &size, true, false);
	SDL_RWops* rw = (SDL_RWops*) malloc (sizeof(SDL_RWops));
	if(!rw) return NULL;
	rw->data=buf;
	rw->size=size;
	Mix_Chunk * mus =  Mix_LoadWAV_RW(rw, false);
	free(rw);
	free(buf);
	return mus;  
}

Mix_Music * Mix_LoadMUS(const char *f){
	u32 size;
	u8 * buf = (u8*) bufferizeFile(f, &size, true, false);
	SDL_RWops* rw = (SDL_RWops*) malloc (sizeof(SDL_RWops));
	if(!rw) return NULL;
	rw->data=buf;
	rw->size=size;
	Mix_Chunk * mus =  Mix_LoadWAV_RW(rw, true);
	free(rw);
	free(buf);
	return mus;  
}

void Mix_AllocateChannels( int c){}

const char * Mix_GetError(){
	return stdMixErr;
}

void Mix_CloseAudio(){
	soundClose();
}

void Mix_FadeInMusic(Mix_Music* s, int a, int b){
	Mix_PlayMusic(s , 1);
}

void Mix_FadeOutMusic(int a){
	Mix_HaltMusic();
}

Mix_Chunk * Mix_LoadWAV_RW(SDL_RWops* buffer, int loops)
{
	int i;
	for(i=0;i<NUMSFX;i++)
	{
		if(!SFX[i].used)
		{
			initSFX(&SFX[i]);
			int freq,bps;
			SFX[i].data=(u8*) parsewavbuffer(buffer->data, &SFX[i].size,  true, &freq, &bps);
			if(!SFX[i].data)return NULL;
			SFX[i].freq=freq;
			SFX[i].format=(bps==8)?SOUND_FORMAT_8BIT:SOUND_FORMAT_16BIT;
			SFX[i].used = true;
			SFX[i].loop=loops;
			return &SFX[i];
		}
	}
	return NULL;
}

void Mix_FreeMusic(Mix_Music* s)
{
	if(s) {
		if (s->data)
			linearFree(s->data);
		s->size=0;
		s->used=false;
		s->loop=false;
	}
}

void Mix_FreeChunk(Mix_Chunk* s)
{
	if(s) {
		if (s->data)
			linearFree(s->data);
		s->size=0;
		s->used=false;
		s->loop=false;
	}
}

void Mix_HookMusicFinished(void* p){}

void Mix_HaltMusic(void)
{
	CSND_SetPlayState(15, 0);//Stop music audio playback.
	csndExecCmds(0);
	soundchannels[7].playlen= 0;
	soundchannels[7].playstart=0;
}


void Mix_HaltChannel(int ch)
{
	if (ch<-1 || ch>7) return;
	else if (ch == -1)
	{	
		int chcount;
		for (chcount=0; chcount<8;chcount++)
		{
			CSND_SetPlayState(chcount+8, 0);//Stop music audio playback.
			csndExecCmds(0);
			soundchannels[chcount].playlen= 0;
			soundchannels[chcount].playstart=0;
		}
		return;
	}
	CSND_SetPlayState(ch+8, 0);//Stop music audio playback.
	csndExecCmds(0);
	soundchannels[ch].playlen= 0;
	soundchannels[ch].playstart=0;
};