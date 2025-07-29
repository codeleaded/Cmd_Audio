#ifndef TIME_H
#define TIME_H

#include <stdio.h>

#ifdef __linux__
#include <time.h>
#endif

#ifdef _WIN64
#include <windows.h>
#endif

#define TIME_NANOTOSEC  1000000000ULL
#define TIME_FNANOTOSEC 1.0E9

typedef struct Time_t {
    unsigned short Nano;
    unsigned short Micro;
    unsigned short Mill;
    unsigned short Sec;
    unsigned short Min;
    unsigned short Hour;
    unsigned short Day;
    unsigned short Month;
    unsigned short Year;
} Time_t;

typedef unsigned long long Timepoint;
typedef unsigned long long Duration;
typedef double FDuration;

double Time_DNowSec(){
    #ifdef __linux__
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC,&time);
    long seconds = time.tv_sec;
    long nanoseconds = time.tv_nsec;
    return (double)seconds + ((double)nanoseconds) / TIME_FNANOTOSEC;
    #endif
    #ifdef _WIN64
    LARGE_INTEGER freq,time;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&time);
    return (double)time.QuadPart / (double)freq.QuadPart;
    #endif
}

float Time_FNowSec(){
	#ifdef __linux__
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC,&time);
    long seconds = time.tv_sec;
    long nanoseconds = time.tv_nsec;
    return (float)seconds + ((float)nanoseconds) / TIME_FNANOTOSEC;
    #endif
    #ifdef _WIN64
    LARGE_INTEGER freq,time;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&time);
    return (float)time.QuadPart / (float)freq.QuadPart;
    #endif
}

Timepoint Time_Nano(){
	return (unsigned long long)(Time_DNowSec() * 1.0E9);
}

Timepoint Time_Micro(){
	return (unsigned long long)(Time_DNowSec() * 1.0E6);
}

Timepoint Time_Mill(){
	return (unsigned long long)(Time_DNowSec() * 1.0E3);
}

Timepoint Time_Sec(){
	return (unsigned long long)(Time_DNowSec());
}

Timepoint Time_Min(){
	return (unsigned long long)(Time_DNowSec() / 60.0);
}

Timepoint Time_Hour(){
	return (unsigned long long)(Time_DNowSec() / (60.0 * 60.0));
}

Timepoint Time_Day(){
	return (unsigned long long)(Time_DNowSec() / (60.0 * 60.0 * 24.0));
}

Timepoint Time_Month(){
	return (unsigned long long)(Time_DNowSec() / (60.0 * 60.0 * 24.0 * 30.0));
}

Timepoint Time_Year(){
	return (unsigned long long)(Time_DNowSec() / (60.0 * 60.0 * 24.0 * 365.25));
}

FDuration Time_Elapsed(Timepoint Start,Timepoint End){
    return (FDuration)(End-Start) / TIME_FNANOTOSEC;
}

Timepoint Time_SecToNano(double Secs){
	return (Timepoint)(Secs * (double)1.0E9);
}
double Time_NanoToSec(Timepoint Nanos){
	return (double)Nanos / (double)1.0E9;
}

Time_t Time_Get(Timepoint Nano){
    Time_t t;
    t.Nano = Nano - (Nano / 1000) * 1000;
    Nano /= 1000;
    t.Micro = Nano - (Nano / 1000) * 1000;
    Nano /= 1000;
    t.Mill = Nano - (Nano / 1000) * 1000;
    Nano /= 1000;
    t.Sec = Nano - (Nano / 1000) * 1000;
    Nano /= 60;
    t.Min = Nano - (Nano / 60) * 60;
    Nano /= 60;
    t.Hour = Nano - (Nano / 60) * 60;
    Nano /= 24;
    t.Day = Nano - (Nano / 24) * 24;
    return t;
}

void Time_Str(char* Buffer,unsigned long long Nano){
    Time_t t = Time_Get(Nano);
    sprintf(Buffer,"%d years: %d days [%d:%d:%d]",t.Year,t.Day,t.Hour,t.Min,t.Sec);
}

void Time_Sleep(double Duration){
    Timepoint Start = Time_Nano();
    while(Time_Elapsed(Start,Time_Nano())<Duration){}
}

#endif