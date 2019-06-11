/*
 * Copyright 2019 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Tom St Denis <tom.stdenis@amd.com>
 *
 */
#include "umrapp.h"
#include <inttypes.h>

void umr_ib_read_file(struct umr_asic *asic, char *filename, int pm)
{
	struct umr_ring_decoder decoder;
	int follow_ib;
	FILE *infile;
	char buf[128];
	uint32_t  *data, x;

	memset(&decoder, 0, sizeof decoder);

	data = calloc(sizeof(*data), 1024);
	if (!data) {
		fprintf(stderr, "[ERROR]: Out of memory\n");
		return;
	}

	infile = fopen(filename, "r");
	if (!infile) {
		free(data);
		perror("Cannot open IB file");
		return;
	}

	x = 0;
	while (fgets(buf, sizeof(buf)-1, infile) != NULL) {
		// skip any line that isn't just a hex value
		if (sscanf(buf, "%"SCNx32, &data[x]) == 1) {
			++x;
			if (!(x & 1023)) {
				void *tmp = realloc(data, sizeof(*data) * (x + 1024));
				if (tmp) {
					data = tmp;
				} else {
					fprintf(stderr, "[ERROR]: Out of memory\n");
					free(data);
					fclose(infile);
					return;
				}
			}
		}
	}
	fclose(infile);

	decoder.next_ib_info.vmid = UMR_PROCESS_HUB;
	decoder.next_ib_info.ib_addr = (uintptr_t)data;
	decoder.next_ib_info.size = x * sizeof(*data);
	decoder.pm = pm;
	follow_ib = asic->options.follow_ib;
	asic->options.follow_ib = 0;
	umr_dump_ib(asic, &decoder);
	asic->options.follow_ib = follow_ib;
	free(data);
}
