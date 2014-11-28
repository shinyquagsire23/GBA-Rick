#include "frames.h"
#include "music.h"
#include <stdio.h>
#include <tonc.h>

int main()
{
    *(unsigned int*)0x04000000 = 0x0402;
    *(unsigned int*)0x0400000C = 0x1000;
    *(unsigned int*)0x04000020 = 0x40;
    *(unsigned int*)0x04000026 = 0x40;
    *(unsigned int*)0x04000028 = 0x600;
    *(unsigned int*)0x0400002C = 0x600;
    *(unsigned int*)0x040000B0 = 0x0300;
    *(unsigned int*)0x040000B4 = 0x0500;
    *(unsigned int*)0x040000BA = 0x8000;

// turn sound on
    REG_SNDSTAT= SSTAT_ENABLE;
    // snd1 on left/right ; both full volume
    REG_SNDDMGCNT = SDMG_BUILD_LR(SDMG_SQR1, 7);
    // DMG ratio to 100%
    REG_SNDDSCNT= SDS_DMG100;

    // no sweep
    REG_SND1SWEEP= SSW_OFF;
    // envelope: vol=12, decay, max step time (7) ; 50% duty
    REG_SND1CNT= SSQR_ENV_BUILD(12, 0, 7) | SSQR_DUTY1_2;
    REG_SND1FREQ= 0;

	int frame = 0;
	int wait = 0;
	int lol = 5;
	DmaPlaySound();

	while(1)
	{
		if(frame == 0)
			REG_DMA1CNT_H=0xb600;

		LZ77UnCompVram(framesTiles[frame], 0x06000040);
		memcpy(pal_bg_mem, framesPals[frame], 0x200);
		int i;
		for(i = 0; i < 16*16; i++)
		{
			((unsigned short*)0x06008000)[i] = map[(i*2)] + (map[(i*2)+1]<<8);
		}
		wait++;
		if(wait == 2)
		{
			frame++; 
			wait = 0;
		}

		if(frame > 6200)
		{
			REG_DMA1SAD=(unsigned long)rick_rolls;	//dma1 source
			REG_DMA1CNT_H=0x3600;
		}

		if(frame > 6378)
		{
			frame = 0;
		}
		vid_vsync();

    //REG_SND1FREQ = SFREQ_RESET | SND_RATE(0x2, 0x2);
	}
    while(1);

    return 0;
}

void DmaPlaySound (void){
	//Play a mono sound at 16khz in DMA mode Direct Sound
	//uses timer 0 as sampling rate source
	//uses timer 1 to count the samples played in order to stop the sound 
	REG_SOUNDCNT_L=0;
	REG_SOUNDCNT_H=0x0b0F;  //DS A&B + fifo reset + timer0 + max volume to L and R
	REG_SOUNDCNT_X=0x0080;  //turn sound chip on
	
	REG_DMA1SAD=(unsigned long)rick_rolls;	//dma1 source
	REG_DMA1DAD=0x040000a0; //write to FIFO A address
	REG_DMA1CNT_H=0xb600;	//dma control: DMA enabled+ start on FIFO+32bit+repeat
	
	REG_TM1CNT_L=0x7098;	//0xffff-the number of samples to play
	REG_TM1CNT_H=0xC4;		//enable timer1 + irq and cascade from timer 0

	//REG_IE=0x10;	  	//enable irq for timer 1 overflow
	REG_IME=1;				//enable interrupt
	
	//Formula for playback frequency is: 0xFFFF-round(cpuFreq/playbackFreq)
	REG_TM0CNT_L=0xFBE8;	//16khz playback freq
	REG_TM0CNT_H=0x0080; 	//enable timer at CPU freq 
				 
}

void TimerPlaySound(void){
	//Play a mono sound at 16khz in Interrupt mode Direct Sound
	//uses timer 0 as sampling rate source

	REG_SOUNDCNT_H=0x0B0F;  //DirectSound A + fifo reset + max volume to L and R
	REG_SOUNDCNT_X=0x0080;  //turn sound chip on

	REG_IE=0x8;		//enable timer 0 irq
	REG_IME=1;				//enable interrupts

	//set playback frequency
	//note: using anything else thank clock multipliers to serve as sample frequencies
	//		tends to generate distortion in the output. It has probably to do with timing and 
	//		FIFO reloading.

	REG_TM0CNT_L=0xffff; 
	REG_TM0CNT_H=0x00C3;	//enable timer at CPU freq/1024 +irq =16386Khz sample rate
	
}

