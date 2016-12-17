#include <stdio.h>
#include <string.h>

int character_split(char *string, const char *delim)
{
	char *result = NULL;
	result = strtok(string, delim);
	while( result != NULL )
	{
		printf( "result is \"%s\"\n", result);
		result = strtok( NULL, delim);
	}

}

int main(void)
{
	char string[24] = {"2016/12/06"};
	const char *delim = "/";
	character_split(string, delim);
	return 0;
}
