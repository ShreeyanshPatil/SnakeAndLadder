#include "../inc/SnakeNLadderDataTypes.h"
#include "../inc/FileManager.h"
int** ReadColorMap()
{
    FILE *fp = NULL;
    char readBuffer[1024];
	int**colormap = (int**)malloc(sizeof(int*)*MAX_LINE);
	int i = 0;
	for (i = 0; i<MAX_LINE; i++)
	{
		colormap[i] = (int*)malloc(sizeof(int)*COLOR_COMP_PER_LINE);
	}
	
    memset(readBuffer, 0, sizeof(readBuffer));
    if (fopen_s(&fp, "config/config.txt", "r") != 0)
    {
        fprintf(gpFile, "Failed to open config.txt file\n");
    }
    else
    {
        fprintf(gpFile, "Color Map File opened successfully\n");
        int lineno = 0;
        int index = 0;
        while (fgets(readBuffer, sizeof(readBuffer), fp))
        {
            char *p = strtok(readBuffer, ",");
            while (p != NULL)
            {
                colormap[lineno][index] = atoi(p);
                index++;
                p = strtok(NULL, ",");
            }
            fprintf(gpFile, "LineNo = %d\n", lineno++);
            index = 0;
            memset(readBuffer, 0, sizeof(readBuffer));
        }
        fclose(fp);
    }
	return colormap;
}