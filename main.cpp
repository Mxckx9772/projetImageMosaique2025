#include <stdio.h>
#include <stdlib.h>


#include "Image.h"
#include "Vernam.h"


int main(int argc, char* argv[]){
  char imgName[250];
  int width, height, size;
  
  if (argc != 2){
    printf("Usage:\n(1) Image.ppm\n"); 
    exit(1);
  }
   
  sscanf (argv[1],"%s", imgName);

  Image Img = Image(imgName);

  return 0;
}
