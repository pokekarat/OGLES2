#include "PVRScopeGraph.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <jni.h>
//
#include <android/log.h>
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

double prev_total = 0;
double prev_idle = 0;
double prev_idle_time = 0;
double prev_idle_entry = 0;
double prev_tx = 0;
double prev_rx = 0;
char **sample;
FILE *fp_save;
char saveFile[2048];

double parseMem(char memLine[])
{

	double ret = 0;
	double total = 0;
	double free = 0;
	double buffer = 0;
	
	char *tok = NULL;	
	tok = strtok(memLine, " ");
	
	int count = 0;

	while(tok)
	{
		if(count == 1){
			total = atof(tok);
			//printf("total = %f\n",total);
		}
		else if(count == 3){
			free = atof(tok);
			//printf("free = %f\n",free);
		}
		else if(count == 5){
			buffer = atof(tok);
			//printf("buffer = %f\n",buffer);
		}
		
		//printf("token = %f \n",atof(tok));
		
		
		tok = strtok(NULL, " ");
		++count;
	}
	
	ret = (total - (free+buffer))/total;
	ret = ret * 100;
		
	//printf("count = %d\n", count);
	//free(tok);
	
	return ret;

}

double parseCPU(char cpuLine[])
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

	double diff_idle = idle - prev_idle;
	double diff_total = total - prev_total;
	double diff_util = (1000 * (diff_total - diff_idle) / diff_total) / 10;

	prev_total = total;
	prev_idle = idle;
	free(tok);
	//printf("parse CPU util = %f\n",diff_util);
	
	
	return diff_util;
}

//void *method(void *ptr)
void method(int numTest, int numTime)
{		
			
		char buffer[2048];
		char header[1024];
		char aut[1024];
		
		int numCol = 1024;
		
		sample = (char **)malloc(numTime * sizeof(char *));
		
		int c=0;
		for(c=0; c<numTime; c++)
			sample[c] = (char *) malloc(numCol * sizeof(char));
		
			
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
		}else{
			//LOGE("Error initializing PVRScope.");
			printf("Error initializing PVRScope...\n");
		}
		
		//Print each and every counter (and its group)		
		//LOGI("Find below the list of counters:");
		
		for(int i = 0; i < uCounterNum; ++i)
		{
			printf(" Group %d %s\n", psCounters[i].nGroup, psCounters[i].pszName);
		}
		
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
						/*
						if(j==3){
							sprintf(aut,"echo %s > /sys/class/backlight/s5p_bl/brightness","0");
							system(aut);
						}
						else if(j==5){
							sprintf(aut,"echo %s > /sys/class/backlight/s5p_bl/brightness","255");
							system(aut);
						}
						*/
						
						//Read CPU util every 1 second
						if((fp = fopen("/proc/stat","r")) != NULL) {	
									
							fgets(buffer,sizeof buffer, fp);
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\nloop_%d\n_CPU_\n%s",j,"util=");
							
							//calculate cpu util
							double cpu_util = parseCPU(buffer);
							char output[50];
							snprintf(output,50,"%.2f",cpu_util);
							strcat(header,output);
							strcat(sample[j],header);
					
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							
							//printf("util %s \n", sample[j]);
							fclose(fp);
						}
						
						//Read CPU Freq
						if((fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","r")) != NULL) {	
							
							fgets(buffer,sizeof buffer, fp);
							sprintf(header,"\n%s","freq=");
							strcat(header,buffer);
							strcat(sample[j],header);
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf(" %s \n",sample[j]);
							
							fclose(fp);
						}
						
						//Read CPU idle time
						if((fp = fopen("/sys/devices/system/cpu/cpu0/cpuidle/state0/time","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"%s","idle_time=");
							double cur = atof(buffer);
							double diff_time = cur - prev_idle_time;
							char output_idle[50];
							snprintf(output_idle,50,"%.2f",diff_time/1000);
							strcat(header,output_idle);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							prev_idle_time = cur;
							fclose(fp);
						}
						
						//Read CPU idle usage
						if((fp = fopen("/sys/devices/system/cpu/cpu0/cpuidle/state0/usage","r")) != NULL) {	
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","idle_usage=");
							double cur = atof(buffer);
							double diff_entry = cur - prev_idle_entry;
							char output_entry[50];
							snprintf(output_entry,50,"%.2f",diff_entry);
							strcat(header,output_entry);				
							strcat(sample[j],header);
							
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							
							prev_idle_entry = cur;
							fclose(fp);
						}
						
						//Read memory usage
						if((fp = popen("free -m", "r")) != NULL){
						
							fgets(buffer, sizeof buffer, fp);
							fgets(buffer, sizeof buffer, fp);
							//fprintf(stdout, "-%s=\n",buffer);
							
							sprintf(header,"\n%s","mem_util=");
							double mem_util = parseMem(buffer);
							char output[50];
							snprintf(output,50,"%.2f",mem_util);
							strcat(header,output);
							strcat(sample[j],header);
					
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							fclose(fp);
						}
						
						
						//LCD bright
						if((fp = fopen("/sys/class/backlight/s5p_bl/brightness","r")) != NULL) 
						{	
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n_DISPLAY_\n%s","bright=");
							strcat(header,buffer);				
							strcat(sample[j],header);
						
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							//printf("%s \n",sample[j]);
							fclose(fp);
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
							fclose(fp);
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
							fclose(fp);
						}
						
						if((fp = fopen("/sys/class/net/wlan0/operstate","r")) != NULL) {
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"\n%s","state=");
							strcat(header,buffer);				
							strcat(sample[j],header);
						
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							printf("wlan\n");
							fclose(fp);
						}
						
						// Battery //
						sprintf(header,"%s\n","_BATT_");
						strcat(sample[j],header);
						if((fp = fopen("/sys/class/power_supply/battery/capacity","r")) != NULL) {
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"%s","capacity=");
							strcat(header,buffer);				
							strcat(sample[j],header);
						
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							
							fclose(fp);
						}
						
						if((fp = fopen("/sys/class/power_supply/battery/voltage_now","r")) != NULL) {
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"%s","volt=");
							strcat(header,buffer);				
							strcat(sample[j],header);
						
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							
							fclose(fp);
						}
						
						if((fp = fopen("/sys/class/power_supply/battery/temp","r")) != NULL) {
						
							fgets(buffer,sizeof buffer, fp);
							
							sprintf(header,"%s","temp=");
							strcat(header,buffer);				
							strcat(sample[j],header);
						
							memset(&buffer[0], 0, sizeof(buffer));
							memset(&header[0], 0, sizeof(header));
							
							fclose(fp);
						}
						
						sprintf(header,"%s\n","_GPU_");
						strcat(sample[j],header);
						
						for(int p = 0; p < uCounterNum; ++p)
						{					
							if(p < sReading.nValueCnt)
							{																
								/*sprintf(buffer, "%f\n",sReading.pfValueBuf[p]);
								strcat(header,buffer);								
								strcat(sample[j],header);
								memset(&buffer[0], 0, sizeof(buffer));
								memset(&header[0], 0, sizeof(header));*/
								//memset(buffer, 0, sizeof(buffer));							
									
								sprintf(header,"%s=",psCounters[p].pszName);
								sprintf(buffer, "%f\n",sReading.pfValueBuf[p]);
								strcat(header,buffer);								
								strcat(sample[j],header);

								memset(&buffer[0], 0, sizeof(buffer));
								memset(&header[0], 0, sizeof(header));		
								
							}
						}
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
		
		/*printf("save file\n");
		sprintf(aut,"/data/local/tmp/stat/sample%d.txt",numTest);
		fp = fopen(aut,"w+");
		for(int i = 0; i <= numTime; i++)
	    {
			fprintf(fp, "%s", sample[i]);
		}
		fclose(fp);
		
		printf("end file\n");*/
		
		printf("save file\n");
		sprintf(saveFile,"/data/local/tmp/stat/sample%d.txt", numTest);
		fp_save = fopen(saveFile,"w+");
		
		for(int i = 0; i < numTime; i++)
	    {
			//printf("%s",sample[i]);
			fprintf(fp_save, "%s", sample[i]);
		}
		
		printf("close all file\n");
		fclose(fp_save);
		
		printf("end file\n");
		
		//Clear array memory
		memset(&sample[0], 0, sizeof(sample));
		//memset(&buffer[0], 0, sizeof(buffer));
		//memset(&header[0], 0, sizeof(header));
		memset(&aut[0], 0, sizeof(aut));
		
		prev_total = 0;
		prev_idle = 0;
		prev_idle_time = 0;
		prev_idle_entry = 0;
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

