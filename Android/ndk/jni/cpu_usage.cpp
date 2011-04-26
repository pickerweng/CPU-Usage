/*
 * Copyright (c) 2011, Picker Weng
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "cpu_usage.h"

static int total_cpu;

cpu_t* get_cpu_stat(cpu_t* _ct)
{
	char buf[BUFSIZE];
	static FILE* fp = NULL;
	int number;

	if (!fp) {
		fp = fopen("/proc/stat", "r");
		if (!fp) {
			fprintf(stderr, "Open /proc/stat %d Failed.", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	rewind(fp);
	fflush(fp);

	if (!fgets(buf, sizeof(buf), fp)) {
		fprintf(stderr, "failed to read /proc/stat");
		exit(EXIT_FAILURE);
	}
	
	number = sscanf(buf, "cpu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
		&_ct->usr, 
		&_ct->nice, 
		&_ct->sys, 
		&_ct->idle, 
		&_ct->iowait, 
		&_ct->irq, 
		&_ct->softirq, 
		&_ct->stead, 
		&_ct->guest);
	
	if (number < 4) {
        fprintf(stderr, "failed to /proc/stat");
		exit(EXIT_FAILURE);
	}
	
	_ct->id = 0;

	return _ct;
}

float get_cpu_usage(long time)
{
	TIME_t tz;
	cpu_t preCpuStat, nextCpuStat;

	get_cpu_stat(&preCpuStat);
	sleep(time);
	get_cpu_stat(&nextCpuStat);

	cpu_t* cpu = &preCpuStat;
	cpu_t* next_cpu = &nextCpuStat;

	TIME_t diff_usr = next_cpu->usr - cpu->usr;
	TIME_t diff_nice = next_cpu->nice - cpu->nice;
	TIME_t diff_sys = next_cpu->sys - cpu->sys;
	TIME_t diff_idle = TRIMz(next_cpu->idle - cpu->idle);
	TIME_t diff_iowait = next_cpu->iowait - cpu->iowait;
	TIME_t diff_irq = next_cpu->irq - cpu->irq;
	TIME_t diff_sirq = next_cpu->softirq - cpu->softirq;
	TIME_t diff_stead = next_cpu->stead - cpu->stead;
	TIME_t diff_guest = next_cpu->guest - cpu->guest;

	int total_diff = 
				diff_usr +
				diff_nice +
				diff_sys +
				diff_idle + 
				diff_iowait +
				diff_irq +
				diff_sirq +
				diff_stead +
				diff_guest;

	//float usage = (100.0f * (next_cpu->idle - cpu->idle)) / total_diff;
	float usage = (100.0f 
					* ((next_cpu->usr + next_cpu->sys + next_cpu->nice) 
					- (cpu->usr + cpu->sys + cpu->nice))) 
					/ total_diff;

	return usage;
}


/**
 * JNI Related functions
 */
#include <jni.h>
cpu_t* cpuStat = NULL;

struct jniCpuStat
{
    jfieldID usr;
    jfieldID nice;
    jfieldID sys;
    jfieldID idle;
    jfieldID iowait;
    jfieldID irq;
    jfieldID softirq;
    jfieldID stead;
    jfieldID guest;
} gjniCpuUsage;


extern "C" {

JNIEXPORT void JNICALL
Java_com_android_cpuusage_CpuUsage_nativeClassInit(JNIEnv *_env, jclass _this)
{
    jclass statClass = _env->FindClass("com/android/cpuusage/CpuUsage$CpuStat");
    gjniCpuUsage.usr		= _env->GetFieldID(statClass, "usr",		"I");
    gjniCpuUsage.nice		= _env->GetFieldID(statClass, "nice",		"I");
    gjniCpuUsage.sys		= _env->GetFieldID(statClass, "sys",		"I");
    gjniCpuUsage.idle		= _env->GetFieldID(statClass, "idle",		"I");
    gjniCpuUsage.iowait		= _env->GetFieldID(statClass, "iowait",		"I");
    gjniCpuUsage.irq		= _env->GetFieldID(statClass, "irq",		"I");
    gjniCpuUsage.softirq	= _env->GetFieldID(statClass, "softirq",	"I");
    gjniCpuUsage.stead		= _env->GetFieldID(statClass, "stead",		"I");
    gjniCpuUsage.guest		= _env->GetFieldID(statClass, "guest",		"I");

    cpuStat = (cpu_t*) malloc(sizeof(cpu_t));
    if (cpuStat == NULL) {
        fprintf(stderr, "failed to allocate memory for cpu_t");
        exit(EXIT_FAILURE);
    }  
}

JNIEXPORT void JNICALL
Java_com_android_cpuusage_CpuUsage_nativeClassExit(JNIEnv *_env, jclass _this)
{
	free(cpuStat);
	cpuStat = NULL;
}

JNIEXPORT void JNICALL
Java_com_android_cpuusage_CpuUsage_jni_getCpuStat(JNIEnv* _env, jobject _this, jobject _stat)
{
	cpuStat = get_cpu_stat(cpuStat);

    _env->SetIntField(_stat, gjniCpuUsage.usr,			cpuStat->usr);
    _env->SetIntField(_stat, gjniCpuUsage.nice,			cpuStat->nice);
    _env->SetIntField(_stat, gjniCpuUsage.sys,			cpuStat->sys);
    _env->SetIntField(_stat, gjniCpuUsage.idle,			cpuStat->idle);
    _env->SetIntField(_stat, gjniCpuUsage.iowait,		cpuStat->iowait);
    _env->SetIntField(_stat, gjniCpuUsage.irq,			cpuStat->irq);
    _env->SetIntField(_stat, gjniCpuUsage.softirq,		cpuStat->softirq);
    _env->SetIntField(_stat, gjniCpuUsage.stead,		cpuStat->stead);
    _env->SetIntField(_stat, gjniCpuUsage.guest,		cpuStat->guest);
}

JNIEXPORT jfloat JNICALL
Java_com_android_cpuusage_CpuUsage_jni_getCpuUsage(JNIEnv* _env, jobject _this, jlong _time)
{
	return get_cpu_usage((long) _time);
}

}	/// extern "C"
