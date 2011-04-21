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

#include "cpu_usage.h"

static int total_cpu;

cpu_t* cpu_time_refresh(cpu_t* _ct)
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

int main(int argc, char** argv)
{
	cpu_t* cpu = (cpu_t*) malloc(sizeof(cpu_t));
	if (cpu == NULL) {
		fprintf(stderr, "failed to allocate memory for cpu_t");
		exit(EXIT_FAILURE);
	}

	cpu = cpu_time_refresh(cpu);

	printf("time: %Ld %Ld %Ld %Ld %Ld %Ld %Ld %Ld\n", 
		cpu->usr, 
		cpu->nice, 
		cpu->sys, 
		cpu->idle, 
		cpu->iowait, 
		cpu->irq, 
		cpu->softirq, 
		cpu->stead, 
		cpu->guest);

	free(cpu);
}

