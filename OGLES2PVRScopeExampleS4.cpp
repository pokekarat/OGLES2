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
	//printf("%s\n", tok);
	
	tok = strtok(freeBuf, " ");
	tok = strtok(NULL, " ");
	free = atof(tok);
	//printf("%f\n", free);
	
	tok = strtok(bufferBuf, " ");
	tok = strtok(NULL, " ");
	buffer = atof(tok);
	//printf("%f\n", buffer);
	
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
	/*char aut2[1024];
	sprintf(aut2,"sh /data/local/tmp/gpu_train_event.sh");
	system(aut2);*/
}

char buffer[4096];
char header[9999];
char saveFile[2048];

int numCol = 9999;

char **sample_cpu;

FILE *fp_cpu;
FILE *fp_cpu_chk;
FILE *fp;
FILE *fp_save;
FILE *fp_mem;

		
//void *method(void *ptr)
void method(int numTest, int numTime)
{		

		sample_cpu = (char **)malloc(numTime * sizeof(char *));
		
		int c2=0;
		for(c2=0; c2<numTime; c2++)
			sample_cpu[c2] = (char *) malloc(numCol * sizeof(char));
			
		
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
		unsigned int sampleRate = 1;
		unsigned int index = 0;
		unsigned int j = 0;
		
		
		
		while (j < numTime)
		{
	
				//printf("begin working...\n");
				
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
					printf("sample %d\n",j);

					index = 0;
					struct timeval tv;
					gettimeofday(&tv, NULL);
					unsigned long time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
					
					// Step 4. Read and output the counter information for group 0 to Logcat
					if(PVRScopeReadCountersThenSetGroup(psData, &sReading, time_in_mill * 1000, uActiveGroup))
					{
						printf("test test\n");
					
						//printf("Start uCounterNum = %d %lu \n",uCounterNum, time_in_mill);
						/*if(j==1){
							printf("set bright to dark\n");
							sprintf(aut,"echo %s > /sys/class/backlight/panel/brightness","0");
							system(aut);
						}
						else if(j==5){
							printf("set bright to bright\n");
							sprintf(aut,"echo %s > /sys/class/backlight/panel/brightness","255");
							system(aut);
						}
						else if(j==10){
			
							pthread_t thread1;
							
							const char *message1 = "Thread 1";
							int iret1;
							
							iret1 = pthread_create(&thread1, NULL, callEvent, (void*) message1);
						}*/
						
						strcat(header,"\n_LOOP_");
						char loop[50];
						snprintf(loop,50,"%d\n",j);
						strcat(header,loop);
						
						//Read bigLittle_status
						if((fp_cpu = fopen("/dev/bL_status","r")) != NULL) 
						{	
							//printf("bl\n");
							strcat(header,"_BL_\n");
							fgets(buffer,sizeof buffer, fp_cpu);
							fgets(buffer,sizeof buffer, fp_cpu);
							strcat(header,buffer);
							fgets(buffer,sizeof buffer, fp_cpu);
							strcat(header,buffer);
							memset(buffer, 0, sizeof(buffer));
							fclose(fp_cpu);
						}
						
						//Read CPU util every 1 second
						strcat(header,"_CPU_\n");
						if((fp_cpu = fopen("/proc/stat","r")) != NULL) 
						{		
							
							fgets(buffer,sizeof buffer, fp_cpu);
							
							//CPU0
							fgets(buffer,sizeof buffer, fp_cpu);
							double cpu_util = parseCPU(buffer,0);
							char output[50];
							snprintf(output,50,"util=%.2f",cpu_util);
							strcat(header,output);
							memset(buffer, 0, sizeof(buffer));
							memset(output, 0, sizeof(output));
							
							if((fp_cpu_chk = fopen("/sys/devices/system/cpu/cpu1/online","r")) != NULL){ 
								char test[50];
								fgets(test, sizeof test, fp_cpu_chk);
								
								if(atoi(test) == 1){
									fgets(buffer,sizeof buffer, fp_cpu);
									cpu_util = parseCPU(buffer,1);
									snprintf(output,50," %.2f",cpu_util);
									strcat(header,output);
									memset(buffer, 0, sizeof(buffer));
									memset(output, 0, sizeof(output));
									//fclose(fp_cpu);
								}
								else
								{
									
									strcat(header," x");
								}
								fclose(fp_cpu_chk);
							}
							
							if((fp_cpu_chk = fopen("/sys/devices/system/cpu/cpu2/online","r")) != NULL){ 
								char test[50];
								fgets(test, sizeof test, fp_cpu_chk);
								if(atoi(test) == 1){
									fgets(buffer,sizeof buffer, fp_cpu);
									cpu_util = parseCPU(buffer,2);
									snprintf(output,50," %.2f",cpu_util);
									strcat(header,output);
									memset(buffer, 0, sizeof(buffer));
									memset(output, 0, sizeof(output));
									//fclose(fp_cpu);
								}
								else
								{
									strcat(header," x");
									//printf(" x");
								}
								fclose(fp_cpu_chk);
							}
							
							if((fp_cpu_chk = fopen("/sys/devices/system/cpu/cpu3/online","r")) != NULL){ 
								char test[50];
								fgets(test, sizeof test, fp_cpu_chk);
								if(atoi(test) == 1)
								{
									fgets(buffer,sizeof buffer, fp_cpu);
									cpu_util = parseCPU(buffer,3);
									snprintf(output,50," %.2f",cpu_util);
									strcat(header,output);
									memset(buffer, 0, sizeof(buffer));
									memset(output, 0, sizeof(output));
									//fclose(fp_cpu);
								}
								else
								{
									strcat(header," x");
									//printf("last x\n");
								}
								
								fclose(fp_cpu_chk);
								
							}
							
							fclose(fp_cpu);		
							
						}
						
						printf(">> %s\n <<",header);
										
						strcat(header,"\n_FREQ_");
						
						//Read freq0
						if((fp_cpu = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","r")) != NULL)
						{	
							
							fgets(buffer,sizeof buffer, fp_cpu);
							strcat(header,"\nfreq0=");
							strcat(header,buffer);
							printf("%s\n",header);
							memset(buffer, 0, sizeof(buffer));
						}
						
						fclose(fp_cpu);
						
						//Read freq1
						if((fp_cpu = fopen("/sys/devices/system/cpu/cpu1/cpufreq/scaling_cur_freq","r")) != NULL)
						{	
							printf("freq1\n");
							fgets(buffer,sizeof buffer, fp_cpu);
							strcat(header,"freq1=");
							strcat(header,buffer);
							memset(buffer, 0, sizeof(buffer));
							fclose(fp_cpu);
						}
						else
						{
							strcat(header,"freq1=x");
							printf("freq1=x");
						}					

						//Read freq2
						if((fp_cpu = fopen("/sys/devices/system/cpu/cpu2/cpufreq/scaling_cur_freq","r")) != NULL) {	
							
							printf("freq2\n");
							fgets(buffer,sizeof buffer, fp_cpu);
							strcat(header,"freq2=");
							strcat(header,buffer);
							memset(buffer, 0, sizeof(buffer));
							fclose(fp_cpu);	
						}
						else
						{
							strcat(header,"\nfreq2=x");
							printf("\nfreq2=x");
						}
						
								
						
						//Read freq3
						if((fp_cpu = fopen("/sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq","r")) != NULL) {	
							printf("freq3\n");
							fgets(buffer,sizeof buffer, fp_cpu);
							strcat(header,"freq3=");
							strcat(header,buffer);
							memset(buffer, 0, sizeof(buffer));
							fclose(fp_cpu);
														
						}
						else
						{
							strcat(header,"\nfreq3=x");
							printf("\nfreq3=x");
						}
						
						
						
						printf("%s\n",header);
						
						//Read CPU idle time C0S0IT //////////////////////////////////////////////////////////////////////////
						strcat(header,"\n_IDLE_TIME_\n");
						printf("idle time\n");
						for(int core=0; core<4; core++)
						{
							char title[100];
							snprintf(title,100,"idle_time_%d=",core);
							strcat(header,title);
						
							for(int state=0; state<3; state++)
							{
							
								char proc[100];
								sprintf(proc,"/sys/devices/system/cpu/cpu%d/cpuidle/state%d/time",core,state);
								
								if((fp_cpu = fopen(proc,"r")) != NULL) 
								{	
								
									fgets(buffer,sizeof buffer, fp_cpu);
									strcat(header," ");
									double cur = atof(buffer);
									double diff_time = cur - csit[core][state];
									char output_idle[50];
									snprintf(output_idle,50,"%.2f",diff_time/1000);
									strcat(header,output_idle);				
									memset(buffer, 0, sizeof(buffer));
									csit[core][state] = cur;
									fclose(fp_cpu);
								}
								else{
									strcat(header," x");
								}
							}
							
							strcat(header,"\n");
								
						}
						
						strcat(header,"_IDLE_USAGE_\n");
						for(int core=0; core<4; core++)
						{
							char title[100];
							snprintf(title,100,"idle_usage_%d=",core);
							strcat(header,title);
						
							for(int state=0; state<3; state++)
							{
							
								char proc[100];
								sprintf(proc,"/sys/devices/system/cpu/cpu%d/cpuidle/state%d/usage",core,state);
								
								if((fp_cpu = fopen(proc,"r")) != NULL) 
								{	
								
									fgets(buffer,sizeof buffer, fp_cpu);
									strcat(header," ");
									int cur = atoi(buffer);
									int diff_entry = cur - csiu[core][state];
									char output_entry[50];
									snprintf(output_entry,50,"%d",diff_entry);
									strcat(header,output_entry);				
									memset(buffer, 0, sizeof(buffer));
									csiu[core][state] = cur;
									fclose(fp_cpu);
								}
								else{
									strcat(header," x");
								}
							}
							
							strcat(header,"\n");
								
						}
						
						///////////////////////// Read memory usage /////////////////////////////////////////
						strcat(header,"_MEM_\n");
						//if((fp = fopen("/data/local/tmp/busybox free -m", "r")) != NULL)
						if((fp_mem = fopen("/proc/meminfo","r")) != NULL)
						{
							//printf("mem\n");
							fgets(buffer, sizeof buffer, fp_mem);
							//printf("buffer = %s\n",buffer);
							strcat(header,"mem=");
							char buffer2[1024];
							char buffer3[1024];
							fgets(buffer2, sizeof buffer, fp_mem);
							fgets(buffer3, sizeof buffer, fp_mem);
							double mem_util = parseMem(buffer, buffer2, buffer3);
							char output[50];
							snprintf(output,50,"%.2f",mem_util);		
							strcat(header,output);
														
							memset(buffer, 0, sizeof(buffer));
							memset(buffer2, 0, sizeof(buffer2));
							memset(buffer3, 0, sizeof(buffer3));		
							fclose(fp_mem);
							
						}
					
						strcat(header,"\n_DISPLAY_");
						//Read bright
						if((fp = fopen("/sys/class/backlight/panel/brightness","r")) != NULL) {	
						
							//printf("brightness\n");
							fgets(buffer,sizeof buffer, fp);
							//sprintf(header,"\n_DISPLAY_\n%s","bright=");
							strcat(header,"\nbright=");
							strcat(header,buffer);				
							memset(buffer, 0, sizeof(buffer));
							fclose(fp);
						}
					
						strcat(header,"_WIFI_\n");
						//wifi				
						if((fp = fopen("/sys/class/net/wlan0/statistics/tx_packets","r")) != NULL) {
							
							//printf("WiFi\n");
							fgets(buffer,sizeof buffer, fp);
							double cur = atof(buffer);
							double diff_tx = cur - prev_tx;
							char output_tx[50];
							snprintf(output_tx,50,"%.2f",diff_tx);
							strcat(header,"tx=");
							strcat(header,output_tx);
							memset(buffer, 0, sizeof(buffer));
							
							prev_tx = cur;
							fclose(fp);
						}
						
						if((fp = fopen("/sys/class/net/wlan0/statistics/rx_packets","r")) != NULL) {
						
							fgets(buffer,sizeof buffer, fp);
							double cur = atof(buffer);
							double diff_rx = cur - prev_rx;
							char output_rx[50];
							snprintf(output_rx,50,"%.2f",diff_rx);
							strcat(header,"\nrx=");
							strcat(header,output_rx);
							memset(buffer, 0, sizeof(buffer));
							
							prev_rx = cur;
							fclose(fp);
							
						}
						
						if((fp = fopen("/sys/class/net/wlan0/operstate","r")) != NULL) {
						
							fgets(buffer,sizeof buffer, fp);
							strcat(header,"\noperstate=");				
							strcat(header,buffer);				
							memset(buffer, 0, sizeof(buffer));
							fclose(fp);
						}
						
						// Check for all the counters in the system if the counter has a value on the given active group and ouptut it.
						
						strcat(header,"_GPU_\n");
						
						for(int p = 0; p < uCounterNum; ++p)
						{					
							
							if(p < sReading.nValueCnt)
							{										
								
								strcat(header,psCounters[p].pszName);
								strcat(header,"=");								
								char params[50];
								snprintf(params,50,"%.2f\n",sReading.pfValueBuf[p]);
								strcat(header, params);				
								//memset(buffer, 0, sizeof(buffer));
								
							}
						}
					
					}
					
					strcpy(sample_cpu[j],header);
					printf("%s\n",sample_cpu[j]);
					memset(header, 0, sizeof(header));
					printf("End_loop %d\n",j);
					++j;
						
				}
				
				//Poll for the counters once a second
				usleep(1000000);
				//usleep(1000000);
								
		}
					
		// Step 5. Shutdown PVRScopeStats
		PVRScopeDeInitialise(&psData, &psCounters, &sReading);
		
		printf("save file\n");
		sprintf(saveFile,"/data/local/tmp/stat/sample%d.txt",numTest);
		fp_save = fopen(saveFile,"w+");
		
		for(int i = 0; i < numTime; i++)
	    {
			printf("%s",sample_cpu[i]);
			fprintf(fp_save, "%s", sample_cpu[i]);
		}
		
		printf("close all file\n");
		fclose(fp_save);
		
		printf("end file\n");
		
		//Clear array memory
		//sample is a heap data structure
		free(sample_cpu);
		
		memset(saveFile, 0, sizeof(saveFile));
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

