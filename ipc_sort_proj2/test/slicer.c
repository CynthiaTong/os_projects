#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define SIZEofBUFF 68

int main (int argc, char** argv) {
   FILE *fpb;
   long lSize;
   int numOfrecords, i;
   char buffer[SIZEofBUFF];

   if (argc!= 4) {
      printf("Correct syntax is: %s input_file, output_file num_records \n", argv[0]);
	    exit(1);
   	}

   fpb = fopen (argv[1],"rb");
   if (fpb==NULL) {
      printf("Cannot open binary file\n");
	    exit(1);
   	}

   // check number of records
   numOfrecords = atoi(argv[3]);

    FILE *out = fopen(argv[2], "wb");
    if (out == NULL) {
      printf("Cannot open binary file\n");
	    exit(1);
   	}

   for (i=0; i < numOfrecords; i++) {
      	fread(buffer, SIZEofBUFF, 1, fpb);
      	fwrite(buffer, SIZEofBUFF, 1, out);
   	}
   fclose (fpb);
   fclose(out);
}
