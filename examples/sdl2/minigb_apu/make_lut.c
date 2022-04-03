#include <stdio.h>

#define AUDIO_SAMPLE_RATE	48000.0

int main(void)
{
	for(unsigned f = 0; f < 1536; f += 16)
	{
		double val;
		double freq;

		val = 4194304.0 / (double)((2048 - f) << 5);
		freq = (float)val/AUDIO_SAMPLE_RATE;
		printf("%f, ", freq);

		if(f % 15  == 14)
			putchar('\n');
	}

	putchar('\n');
	putchar('\n');
	for(unsigned f = 1536; f < 2048; f += 2)
	{
		double val;
		double freq;

		val = 4194304.0 / (double)((2048 - f) << 5);
		freq = (float)val/AUDIO_SAMPLE_RATE;
		printf("%f, ", freq);

		if(f % 15  == 14)
			putchar('\n');
	}

	putchar('\n');
	return 0;
}
