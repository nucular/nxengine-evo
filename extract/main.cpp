#include <stdio.h>
#include <stdlib.h>
#include "extractpxt.h"
#include "extractstages.h"
#include "extractfiles.h"

static const char *filename = "Doukutsu.exe";

int main(int argc, char *argv[])
{
    FILE *fp;

    fp = fopen(filename, "rb");
    if (!fp)
    {
        printf("could not open %s\n", filename);
        return 1;
    }

    if (extract_pxt(fp)) {
        printf("could not extract pxt\n");
        fclose(fp);
        return 1;
    }
    if (extract_files(fp)) {
        printf("could not extract files\n");
        fclose(fp);
        return 1;
    }
    if (extract_stages(fp)) {
        printf("could not extract stages\n");
        fclose(fp);
        return 1;
    }

    printf("done\n");
    fclose(fp);
    return 0;
}


