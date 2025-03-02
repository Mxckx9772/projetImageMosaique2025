#include <stdio.h>
#include <stdlib.h>

#include "src/ImageGrey.h"


int main(int argc, char* argv[]){
  char imgName[250];
  
  if (argc != 2){
    printf("Usage:\n(1) Image.pgm\n"); 
    exit(1);
  }
   
  sscanf (argv[1],"%s", imgName);

  ImageGrey Img = ImageGrey(imgName);

  printf("Ecriture de l'image...\n");
  Img.writeImage("Name.pgm");

  return 0;
}
