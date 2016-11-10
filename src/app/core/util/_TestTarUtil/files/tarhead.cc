/**
 *
 * Reads 512 bytes from stdin and outputs a formatted string containing these
 * characters.
 *
 * Usage: tarhead < <file>
 *
 * To compile:
 * $ g++ -o tarhead tarhead.cc
 */
#include <stdio.h>

int main(int argc, char *argv[])
{
    char buffer[512];

    int read = fread(buffer, 1, 512, stdin);
    if (read != 512)
    {
	printf("Incorrect tar header\n");
	return 1;
    }


    int sum = 0;

    for (int i=0; i< 20; ++i)
    {
	for (int j=i*25; j< i*25 + 25; ++j)
	{
	    if (buffer[j] != '\0')
		printf("'%c',", buffer[j]);
	    else
		printf("'\\0',");

	    if (j < 148 || j > 155)
		sum += buffer[j];
	    
	    //	    printf("%d ==> %d\n", j, sum);
	}
	putc('\n', stdout);
    }
    
    for (int i=20 * 25; i< 512; ++i)
    {
	if (buffer[i] != '\0')
	    printf("'%c',", buffer[i]);
	else
	    printf("'\\0',");

	sum += buffer[i];
    }
    putc('\n', stdout);

    sum += 8 * ' ';
    
    printf("Checksum %d (decimal)\n", sum);
    
    return 0;
}
