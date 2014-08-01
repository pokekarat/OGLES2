/******************************************************************************

 @File         OGLES2PVRScopeExample.cpp

 @Title        Iridescence

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to use our example PVRScope graph code.

******************************************************************************/
//#include "PVRShell.h"
//#include "OGLES2Tools.h"
#include <GLES2/gl2.h>
#include "PVRScopeGraph.h"
#include <pthread.h>
/*********************************************************************************************
*
* This example outputs the values of the hardware counters found in Group 0
* to Android Logcat once a second for 60 seconds, and consists of five steps:
*
* 1. Define a function to initialise PVRScopeStats
* 2. Initialise PVRScopeStats
* 3. Set the active group to 0
* 4. Read and output the counter information for group 0 to Logcat
* 5. Shutdown PVRScopeStats
*
*********************************************************************************************/

//#define NUMBER_OF_LOOPS 140

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <jni.h>
//
#include <android/log.h>
#include <android/bitmap.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO , "PVRScope", __VA_ARGS__)
#include "PVRScopeStats.h"

// Step 1. Define a function to initialise PVRScopeStats
bool PSInit(SPVRScopeImplData **ppsPVRScopeData, SPVRScopeCounterDef **ppsCounters, SPVRScopeCounterReading* const psReading, unsigned int* const pnCount)
{
	//Initialise PVRScope
	const EPVRScopeInitCode eInitCode = PVRScopeInitialise(ppsPVRScopeData);
	if(ePVRScopeInitCodeOk == eInitCode)
	{
		printf("Initialised services connection.\n");
	}
	else
	{
		printf("Error: failed to initialise services connection.\n");
		*ppsPVRScopeData = NULL;
		return false;
	}
	
	//Initialse the counter data structures.
	if (PVRScopeGetCounters(*ppsPVRScopeData, pnCount, ppsCounters, psReading))
	{
		printf("Total counters enabled: %d.", *pnCount);
	}
	return true;
}

double prev_total[4] = {0,0,0,0};
double prev_idle[4] = {0,0,0,0};

double csit[4][3] = {{0,0,0},{0,0,0},{0,0,0}, {0,0,0}};
double csiu[4][3] = {{0,0,0},{0,0,0},{0,0,0}, {0,0,0}};


double prev_tx = 0;
double prev_rx = 0;


double parseMem(char totalBuf[], char freeBuf[], char bufferBuf[])
{
	
	double ret = 0;
	double total = 0;
	double free = 0;
	double buffer = 0;
	
	char *tok = NULL;	
	tok = strtok(totalBuf, " ");
	tok = strtok(NULL, " ");
	total = atof(tok);
	printf("%s\n", tok);
	
	tok = strtok(freeBuf, " ");
	tok = strtok(NULL, " ");
	free = atof(tok);
	printf("%f\n", free);
	
	tok = strtok(bufferBuf, " ");
	tok = strtok(NULL, " ");
	buffer = atof(tok);
	printf("%f\n", buffer);
	
	ret = (total - (free+buffer))/total;
	ret = ret * 100;
	
	return ret;

}

double parseCPU(char cpuLine[],int core)
{
	//printf("prev total = %f\n",prev_total);
	
	double total = 0;
	double idle = 0;
	char *tok = NULL;	
	tok = strtok(cpuLine, " ");
	
	int count = 0;

	while(tok)
	{
		//printf("Token: %s\n", tok);
		if(count > 0){
			total += atof(tok);
		}
		
		if(count == 4){
			idle = atof(tok);
		}
		
		tok = strtok(NULL, " ");
		++count;
	}

	double diff_idle = idle - prev_idle[core];
	double diff_total = total - prev_total[core];
	double diff_util = (1000 * (diff_total - diff_idle) / diff_total) / 10;

	prev_total[core] = total;
	prev_idle[core] = idle;
	free(tok);
	//printf("parse CPU util = %f\n",diff_util);
	
	
	return diff_util;
}

void *callEvent(void *ptr){
	char aut2[1024];
	sprintf(aut2,"sh /data/local/tmp/firefox_yt_events.sh");
	system(aut2);
}

//void *method(void *ptr)
void method(int numTest, int numTime)
{		
		  // set the target framebuffer to read
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		GLubyte pixel[1*1*4];
		glReadPixels (400, 300, 1, 1, GL_RGBA,GL_UNSIGNED_BYTE,(void *)pixel);
		printf("%d %d %d\n",pixel[200],pixel[151],pixel[2]);
		
		char buffer[4096];
		char header[4096];
		char aut[1024];
		
		int numCol = 2048;
		
		char sample[numTime * 100][numCol];
		/*
		char **sample = (char **)malloc(numCol * sizeof(char *));		
		for(int x=0; x < 19999; x++)
		{
			sample[x] = (char *)malloc(numCol * sizeof(char));
		}
		*/
		
		printf("test passed2\n");
		
		SPVRScopeImplData *psData;
		
		//Counter information (set at init time)
		SPVRScopeCounterDef    *psCounters;
		unsigned int      uCounterNum;
		
		//Counter reading data
		unsigned int      uActiveGroup;
		unsigned int      uActiveGroupSelect;
		bool        bActiveGroupChanged;
		SPVRScopeCounterReading  sReading;
		
		// Step 2. Initialise PVRScopeStat
		if (PSInit(&psData, &psCounters, &sReading, &uCounterNum)){
			//LOGI("PVRScope up and running.");
			printf("PVRScore up and running...\n");
			
			/*sprintf(aut,"%s\n","su");
			system(aut);*/
			
		}else{
			//LOGE("Error initializing PVRScope.");
			printf("Error initializing PVRScope...\n");
		}
		
		//Print each and every counter (and its group)		
		//LOGI("Find below the list of counters:");
		
		/*for(int i = 0; i < uCounterNum; ++i)
		{
			printf(" Group %d %s\n", psCounters[i].nGroup, psCounters[i].pszName);
		}*/
		
		// Step 3. Set the active group to 0
		bActiveGroupChanged = true;
		uActiveGroupSelect = 0;
		unsigned int sampleRate = 100;
		unsigned int index = 0;
		unsigned int j = 0;
		
		FILE *fp;
		
		while (j < numTime)
		{
	
				// Ask for the active group 0 only on the first run. Then set it to 0xffffffff
				if(bActiveGroupChanged)
				{
						uActiveGroup = uActiveGroupSelect;
				}
				else 
				{
						uActiveGroup = 0xffffffff;
				}
				
				++index;
				
				if (index < sampleRate) 
				{
					// Sample the counters every 10ms. Don't read or output it.
					PVRScopeReadCountersThenSetGroup(psData, NULL, 0, uActiveGroup);
				} 
				else 
				{

					index = 0;
					struct timeval tv;
					gettimeofday(&tv, NULL);
					unsigned long time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
					
					// Step 4. Read and output the counter information for group 0 to Logcat
					if(PVRScopeReadCountersThenSetGroup(psData, &sReading, time_in_mill * 1000, uActiveGroup))
					{
					
						//printf("Start uCounterNum = %d %lu \n",uCounterNum, time_in_mill);
						if(j==1){
							sprintf(aut,"echo %s > /sys/class/backlight/panel/brightness","0");
							system(aut);
						}
						else if(j==5){
							sprintf(aut,"echo %s > /sys/class/backlight/panel/brightness","255");
							system(aut);
						}
						else if(j==10){
			
							pthread_t thread1;
							
							const char *message1 = "Thread 1";
							int iret1;
							
							iret1 = pthread_create(&thread1, NULL, callEvent, (void*) message1);
						}
							
						//Read CPU util every 1 second
						if((fp = fopen("/proc/stat","r")) != NULL) 
						{	
									
							fgets(buffer,sizeof buffer, fp);
							
							//CPU0
							fgets(buffer,sizeof buffer, fp);
							sprintf(header,"\nloop_%d\n_CPU_\n%s",j,"util0=");
							//calculate cpu util
							double cpu_util = parseCPU(buffer,0);
							char output[50];
							snprintf(output,50,"%.2f",cpu_util);
							strcat(header,output);
							strcat(sample[j],header);
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							memset(&output[0], 0, sizeof(output));
							
							//CPU1
							fgets(buffer,sizeof buffer, fp);
							sprintf(header,"\n%s","util1=");
							//calculate cpu util
							cpu_util = parseCPU(buffer,1);
							//char output[50];
							snprintf(output,50,"%.2f",cpu_util);
							strcat(header,output);
							strcat(sample[j],header);
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							memset(&output[0], 0, sizeof(output));
							
							//CPU2
							fgets(buffer,sizeof buffer, fp);
							sprintf(header,"\n%s","util2=");
							//calculate cpu util
							cpu_util = parseCPU(buffer,2);
							//char output[50];
							snprintf(output,50,"%.2f",cpu_util);
							strcat(header,output);
							strcat(sample[j],header);
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							memset(&output[0], 0, sizeof(output));
							
							//CPU3
							fgets(buffer,sizeof buffer, fp);
							sprintf(header,"\n%s","util3=");
							//calculate cpu util
							cpu_util = parseCPU(buffer,3);
							//char output[50];
							snprintf(output,50,"%.2f",cpu_util);
							strcat(header,output);
							strcat(sample[j],header);
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							memset(&output[0], 0, sizeof(output));
						}
						
						//Read bigLittle_status
						if((fp = fopen("/dev/bL_status","r")) != NULL) {	
							
							fgets(buffer,sizeof buffer, fp);
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","A15=");
							strcat(header,buffer);
							strcat(sample[j],header);
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf(" %s \n",sample[j]);
							
							fgets(buffer,sizeof buffer, fp);
							sprintf(header,"%s","A7=");
							strcat(header,buffer);
							strcat(sample[j],header);
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
						}
						
						//Read freq of CPU0-4
						if((fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","r")) != NULL) {	
							
							fgets(buffer,sizeof buffer, fp);
							sprintf(header,"%s","freq0=");
							strcat(header,buffer);
							strcat(sample[j],header);
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf(" %s \n",sample[j]);
						}
						
						//Read freq of CPU0-4
						if((fp = fopen("/sys/devices/system/cpu/cpu1/cpufreq/scaling_cur_freq","r")) != NULL) {	
							
							fgets(buffer,sizeof buffer, fp);
							sprintf(header,"%s","freq1=");
							strcat(header,buffer);
							strcat(sample[j],header);
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf(" %s \n",sample[j]);
						}
						//Read freq of CPU0-4
						if((fp = fopen("/sys/devices/system/cpu/cpu2/cpufreq/scaling_cur_freq","r")) != NULL) {	
							
							fgets(buffer,sizeof buffer, fp);
							sprintf(header,"%s","freq2=");
							strcat(header,buffer);
							strcat(sample[j],header);
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf(" %s \n",sample[j]);
						}
						//Read freq of CPU0-4
						if((fp = fopen("/sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq","r")) != NULL) {	
							
							fgets(buffer,sizeof buffer, fp);
							sprintf(header,"%s","freq3=");
							strcat(header,buffer);
							strcat(sample[j],header);
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf(" %s \n",sample[j]);
						}
						
						//Read CPU idle time C0S0IT //////////////////////////////////////////////////////////////////////////
						if((fp = fopen("/sys/devices/system/cpu/cpu0/cpuidle/state0/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"%s","c0s0it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[0][0];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[0][0] = cur;
						}
						
						//Read CPU idle time C0S1IT
						if((fp = fopen("/sys/devices/system/cpu/cpu0/cpuidle/state1/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c0s1it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[0][1];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[0][1] = cur;
						}
						
						//Read CPU idle time C0S2IT
						if((fp = fopen("/sys/devices/system/cpu/cpu0/cpuidle/state2/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c0s2it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[0][2];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[0][2] = cur;
						}
						
						
						//Read CPU idle time C1S0IT //////////////////////////////////////////////////////////////////////////
						if((fp = fopen("/sys/devices/system/cpu/cpu1/cpuidle/state0/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c1s0it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[1][0];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[1][0] = cur;
						}
						
						//Read CPU idle time C1S1IT
						if((fp = fopen("/sys/devices/system/cpu/cpu1/cpuidle/state1/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c1s1it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[1][1];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[1][1] = cur;
						}
						
						//Read CPU idle time C1S2IT
						if((fp = fopen("/sys/devices/system/cpu/cpu1/cpuidle/state2/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c1s2it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[1][2];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[1][2] = cur;
						}
						
						//Read CPU idle time C2S0IT //////////////////////////////////////////////////////////////////////////
						if((fp = fopen("/sys/devices/system/cpu/cpu2/cpuidle/state0/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c2s0it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[2][0];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[2][0] = cur;
						}
						
						//Read CPU idle time C2S1IT
						if((fp = fopen("/sys/devices/system/cpu/cpu2/cpuidle/state1/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c2s1it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[2][1];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[2][1] = cur;
						}
						
						//Read CPU idle time C2S2IT
						if((fp = fopen("/sys/devices/system/cpu/cpu2/cpuidle/state2/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c2s2it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[2][2];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[2][2] = cur;
						}
						
						//Read CPU idle time C3S0IT //////////////////////////////////////////////////////////////////////////
						if((fp = fopen("/sys/devices/system/cpu/cpu3/cpuidle/state0/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c3s0it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[3][0];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[3][0] = cur;
						}
						
						//Read CPU idle time C3S1IT
						if((fp = fopen("/sys/devices/system/cpu/cpu3/cpuidle/state1/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c3s1it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[3][1];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[3][1] = cur;
						}
						
						//Read CPU idle time C3S2IT
						if((fp = fopen("/sys/devices/system/cpu/cpu3/cpuidle/state2/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c3s2it=");
							double cur = atof(buffer);
							double diff_time = cur - csit[3][2];
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csit[3][2] = cur;
						}
						
						//Read CPU idle usage C0S0 ////////////////////////////////////////////////////////////
						if((fp = fopen("/sys/devices/system/cpu/cpu0/cpuidle/state0/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c0s0iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[0][0];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[0][0] = cur;
						}
						
						if((fp = fopen("/sys/devices/system/cpu/cpu0/cpuidle/state1/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c0s1iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[0][1];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[0][1] = cur;
						}
						if((fp = fopen("/sys/devices/system/cpu/cpu0/cpuidle/state2/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c0s2iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[0][2];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[0][2] = cur;
						}
						
						
						//Read CPU idle usage C1S0 ////////////////////////////////////////////////////////////
						if((fp = fopen("/sys/devices/system/cpu/cpu1/cpuidle/state0/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c1s0iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[1][0];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[1][0] = cur;
						}
						
						if((fp = fopen("/sys/devices/system/cpu/cpu1/cpuidle/state1/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c1s1iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[1][1];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[1][1] = cur;
						}
						if((fp = fopen("/sys/devices/system/cpu/cpu2/cpuidle/state2/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c1s2iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[1][2];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[1][2] = cur;
						}
						
						//Read CPU idle usage C2S0 ////////////////////////////////////////////////////////////
						if((fp = fopen("/sys/devices/system/cpu/cpu2/cpuidle/state0/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c2s0iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[2][0];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[2][0] = cur;
						}
						
						if((fp = fopen("/sys/devices/system/cpu/cpu2/cpuidle/state1/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c2s1iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[2][1];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[2][1] = cur;
						}
						if((fp = fopen("/sys/devices/system/cpu/cpu2/cpuidle/state2/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c2s2iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[2][2];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[2][2] = cur;
						}
						
						//Read CPU idle usage C3S0 ////////////////////////////////////////////////////////////
						if((fp = fopen("/sys/devices/system/cpu/cpu3/cpuidle/state0/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c3s0iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[3][0];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[3][0] = cur;
						}
						
						if((fp = fopen("/sys/devices/system/cpu/cpu3/cpuidle/state1/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c3s1iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[3][1];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[3][1] = cur;
						}
						
						if((fp = fopen("/sys/devices/system/cpu/cpu3/cpuidle/state2/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","c3s2iu=");
							double cur = atof(buffer);
							double diff_entry = cur - csiu[3][2];
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							csiu[3][2] = cur;
						}
					
						///////////////////////// Read memory usage /////////////////////////////////////////
						
						//if((fp = fopen("/data/local/tmp/busybox free -m", "r")) != NULL)
						if((fp = fopen("/proc/meminfo","r")) != NULL)
						{
							
							fgets(buffer, sizeof buffer, fp);
							//printf("buffer = %s\n",buffer);
							char buffer2[1024];
							char buffer3[1024];
							
							fgets(buffer2, sizeof buffer, fp);
							//printf("buffer2 = %s\n",buffer2);
							
							fgets(buffer3, sizeof buffer, fp);
							//printf("buffer3 = %s\n",buffer3);
							
							//fgets(buffer, sizeof buffer, fp);
							//fprintf(stdout, "-%s=\n",buffer);
							
							sprintf(header,"\n%s","mem_util=");
							double mem_util = parseMem(buffer, buffer2, buffer3);
							char output[50];
							snprintf(output,50,"%.2f",mem_util);
							strcat(header,output);
							strcat(sample[j],header);
					
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&buffer2[0], 0, sizeof(buffer2));
							memset(&buffer3[0], 0, sizeof(buffer3));
							memset(&header[0], 0, sizeof(header));
						}
						else
						{
							printf("mem null");
						}
						
						//Read bright
						if((fp = fopen("/sys/class/backlight/panel/brightness","r")) != NULL) {	
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n_DISPLAY_\n%s","bright=");
							strcat(header,buffer);				
							strcat(sample[j],header);
						
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
						}
						
						//wifi				
						if((fp = fopen("/sys/class/net/wlan0/statistics/tx_packets","r")) != NULL) {
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"_WiFi_\n%s","tx=");
							
							double cur = atof(buffer);
							double diff_tx = cur - prev_tx;
							char output_tx[50];
							snprintf(output_tx,50,"%.2f",diff_tx);
							
							strcat(header,output_tx);				
							strcat(sample[j],header);
						
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							prev_tx = cur;
						}
						
						if((fp = fopen("/sys/class/net/wlan0/statistics/rx_packets","r")) != NULL) {
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","rx=");
							
							double cur = atof(buffer);
							double diff_rx = cur - prev_rx;
							char output_rx[50];
							snprintf(output_rx,50,"%.2f",diff_rx);
							
							strcat(header,output_rx);				
							strcat(sample[j],header);
						
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							prev_rx = cur;
							
						}
						
						if((fp = fopen("/sys/class/net/wlan0/operstate","r")) != NULL) {
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","state=");
							strcat(header,buffer);				
							strcat(sample[j],header);
						
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
						}
						
						// Check for all the counters in the system if the counter has a value on the given active group and ouptut it.
						bool isFirst = true;
						
						for(int i = 0; i < uCounterNum; ++i)
						{					
							
							if(i < sReading.nValueCnt)
							{										
							
							    //printf("GPU >> %d : %s : %f\n", j,  psCounters[i].pszName, sReading.pfValueBuf[i]);
								
								/*if ((strcmp(psCounters[i].pszName, "Frame time") == 0) || 
								(strcmp(psCounters[i].pszName, "Frames per second (FPS)") == 0) ||
								(strcmp(psCounters[i].pszName, "GPU task load: 3D core") == 0) || 
								(strcmp(psCounters[i].pszName, "GPU task load: TA core") == 0) ||
								(strcmp(psCounters[i].pszName, "GPU task time: 3D core") == 0) || 
								(strcmp(psCounters[i].pszName, "GPU task time: TA core") == 0) || 
								(strcmp(psCounters[i].pszName, "TA load") == 0) || 
								(strcmp(psCounters[i].pszName, "Texture unit(s) load") == 0) || 
								(strcmp(psCounters[i].pszName, "USSE clock cycles per pixel") == 0) ||
								(strcmp(psCounters[i].pszName, "USSE clock cycles per vertex") == 0) || 
								(strcmp(psCounters[i].pszName, "USSE load: Pixel") == 0) || 
								(strcmp(psCounters[i].pszName, "USSE load: Vertex") == 0) ||
								(strcmp(psCounters[i].pszName, "Vertices per frame") == 0) ||
								(strcmp(psCounters[i].pszName, "Texture unit(s) load") == 0)||
								(strcmp(psCounters[i].pszName, "USSE load: Pixel") == 0) ||
								(strcmp(psCounters[i].pszName, "USSE load: Stall") == 0)||
								(strcmp(psCounters[i].pszName, "Vertices per second: on-screen") == 0))
								{
									
									if(isFirst)
									{
										isFirst = false;
										sprintf(header,"_GPU_\n%s=",psCounters[i].pszName);
									}
									else
										sprintf(header,"%s=",psCounters[i].pszName);
									
									sprintf(buffer, "%f\n",sReading.pfValueBuf[i]);
									strcat(header,buffer);								
									strcat(sample[j],header);

									memset(&buffer[0], 0, sizeof(buffer));
									memset(&header[0], 0, sizeof(header));
									//strcat(sample[j]," \n");
									//printf("%d : %s : %f\n", j,  psCounters[i].pszName, sReading.pfValueBuf[i]);
									//printf("%s \n",sample[j]);
								}	*/	

								if(isFirst)
								{
									isFirst = false;
									sprintf(header,"_GPU_\n%s=",psCounters[i].pszName);
								}
								else
								{
									sprintf(header,"%s=",psCounters[i].pszName);
								}
								
								sprintf(buffer, "%f\n",sReading.pfValueBuf[i]);
								strcat(header,buffer);								
								strcat(sample[j],header);

								memset(&buffer[0], 0, sizeof(buffer));
								memset(&header[0], 0, sizeof(header));	

								//printf("%s \n",sample[j]);								
							}
						}
					
						isFirst = true;
						//printf("End\n");
					}
					
					//printf("Data >> %s \n",sample[j]);
					++j;
				}
				
				//Poll for the counters once a second
				usleep(10 * 1000);
				//usleep(1000000);
		}
		
		//sprintf(aut,"sh /data/local/tmp/stopEvents.sh");
		//system(aut);
		
		//fclose(fp);
			
		// Step 5. Shutdown PVRScopeStats
		PVRScopeDeInitialise(&psData, &psCounters, &sReading);
		
		printf("save file\n");
		sprintf(aut,"/data/local/tmp/stat/sample%d.txt",numTest);
		fp = fopen(aut,"w+");
		for(int i = 0; i <= numTime; i++)
	    {
			fprintf(fp, "%s", sample[i]);
		}
		fclose(fp);
		
		printf("end file\n");
		//Clear array memory
		memset(&sample[0], 0, sizeof(sample));
		//memset(&buffer[0], 0, sizeof(buffer));
		//memset(&header[0], 0, sizeof(header));
		memset(&aut[0], 0, sizeof(aut));
		
		memset(prev_total, 0, sizeof(prev_total));
		memset(prev_idle, 0, sizeof(prev_idle));
		memset(csit, 0, sizeof(csit));
		memset(csiu, 0, sizeof(csiu));
	
		prev_tx = 0;
		prev_rx = 0;
		
		printf("clear file\n");
		exit(0);
}

int main(int argc, char **argv)
{
		printf("Start GPU testing...\n");
		//pthread_t thread1;
		//const char *message1 = "Thread 1";
		//int iret1;
		int nTest  = atoi(argv[1]);    
		int nTime = atoi(argv[2]);
		//iret1 = pthread_create(&thread1, NULL, method, (void*) message1);
		method(nTest, nTime);
		
		return 0;
}

