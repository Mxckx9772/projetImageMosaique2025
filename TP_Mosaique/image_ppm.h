// ----------------------------------------------------------------------------
// Filename        : image_ppm.c
// Description     :
// Created On      : Tue Mar 31 13:26:36 2005
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define allocation_tableau(nom, type, nombre) \
if( (nom = (type*) calloc (nombre, sizeof(type) ) ) == NULL ) \
{\
 printf("\n Allocation dynamique impossible pour un pointeur-tableau \n");\
 exit(1);\
}

typedef unsigned char OCTET;

/*===========================================================================*/
void ignorer_commentaires(FILE * f)
{
  unsigned char c;
  while((c=fgetc(f)) == '#')
    while((c=fgetc(f)) != '\n');
  fseek(f, -sizeof(unsigned char), SEEK_CUR);
}
/*===========================================================================*/


/*===========================================================================*/
void ecrire_image_ppm(char  nom_image[], OCTET *pt_image, int nb_lignes, int nb_colonnes)
{
   FILE *f_image;
   int taille_image = 3*nb_colonnes * nb_lignes;

   if( (f_image = fopen(nom_image, "wb")) == NULL)
      {
	 printf("\nPas d'acces en ecriture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	 fprintf(f_image,"P6\r") ;                               /*ecriture entete*/
	 fprintf(f_image,"%d %d\r255\r", nb_colonnes, nb_lignes) ;

	 if( (fwrite((OCTET*)pt_image, sizeof(OCTET), taille_image, f_image))
	     != (size_t)(taille_image))
	    {
	       printf("\nErreur d'ecriture de l'image %s \n", nom_image);
	       exit(EXIT_FAILURE);
	    }
	 fclose(f_image);
      }
}
/*===========================================================================*/

/*===========================================================================*/		
void lire_nb_lignes_colonnes_image_ppm(char nom_image[], int *nb_lignes, int *nb_colonnes)
{
   FILE *f_image;
   int max_grey_val;

   /* cf : l'entete d'une image .ppm : P6                   */
   /*				       nb_colonnes nb_lignes */
   /*    			       max_grey_val          */


   if( (f_image = fopen(nom_image, "rb")) == NULL)
      {
	 printf("\nPas d'acces en lecture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	 fscanf(f_image, "P6 ");
	 ignorer_commentaires(f_image);
	 fscanf(f_image, "%d %d %d%*c", nb_colonnes, nb_lignes, &max_grey_val);
	 fclose(f_image);
      }
}
/*===========================================================================*/
/*===========================================================================*/
void lire_image_ppm(char  nom_image[], OCTET *pt_image, int taille_image)
{
   FILE *f_image;
   int  nb_colonnes, nb_lignes, max_grey_val;
   taille_image=3*taille_image;

   if( (f_image = fopen(nom_image, "rb")) == NULL)
      {
	 printf("\nPas d'acces en lecture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	fscanf(f_image, "P6 ");
	ignorer_commentaires(f_image);
	fscanf(f_image, "%d %d %d%*c",
	       &nb_colonnes, &nb_lignes, &max_grey_val); /*lecture entete*/

	 if( (fread((OCTET*)pt_image, sizeof(OCTET), taille_image, f_image))
	     !=  (size_t)(taille_image))
	    {
	       printf("\nErreur de lecture de l'image %s \n", nom_image);
	       exit(EXIT_FAILURE);
	    }
	 fclose(f_image);
      }
}

/*===========================================================================*/
/*===========================================================================*/

void planR(OCTET *pt_image, OCTET *src, int taille_image){
   int i;
   for (i=0; i<taille_image; i++){
      pt_image[i]=src[3*i];
      }
   }
   
/*===========================================================================*/
/*===========================================================================*/

void planV(OCTET *pt_image, OCTET *src, int taille_image){
   int i;
   for (i=0; i<taille_image; i++){
      pt_image[i]=src[3*i+1];
      }
   }   

/*===========================================================================*/
/*===========================================================================*/

void planB(OCTET *pt_image, OCTET *src, int taille_image){
   int i;
   for (i=0; i<taille_image; i++){
      pt_image[i]=src[3*i+2];
      }
   }
   
/*===========================================================================*/   
/*===========================================================================*/

void ecrire_image_pgm(char  nom_image[], OCTET *pt_image, int nb_lignes, int nb_colonnes)
{
   FILE *f_image;
   int taille_image = nb_colonnes * nb_lignes;

   if( (f_image = fopen(nom_image, "wb")) == NULL)
      {
	 printf("\nPas d'acces en ecriture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	 fprintf(f_image,"P5\r") ;                               /*ecriture entete*/
	 fprintf(f_image,"%d %d\r255\r", nb_colonnes, nb_lignes) ;

	 if( (fwrite((OCTET*)pt_image, sizeof(OCTET), taille_image, f_image))
	     != (size_t) taille_image)
	    {
	       printf("\nErreur de lecture de l'image %s \n", nom_image);
	       exit(EXIT_FAILURE);
	    }
	 fclose(f_image);
      }
}
/*===========================================================================*/

void lire_nb_lignes_colonnes_image_pgm(char nom_image[], int *nb_lignes, int *nb_colonnes)
{
   FILE *f_image;
   int max_grey_val;

   /* cf : l'entete d'une image .pgm : P5                    */
   /*				       nb_colonnes nb_lignes */
   /*    			       max_grey_val          */


   if( (f_image = fopen(nom_image, "rb")) == NULL)
      {
	 printf("\nPas d'acces en lecture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	 fscanf(f_image, "P5 ");
	 ignorer_commentaires(f_image);
	 fscanf(f_image, "%d %d %d%*c", nb_colonnes, nb_lignes, &max_grey_val);
	 fclose(f_image);
      }
}
/*===========================================================================*/
/*===========================================================================*/
void lire_image_pgm(char  nom_image[], OCTET *pt_image, int taille_image)
{
   FILE *f_image;
   int  nb_colonnes, nb_lignes, max_grey_val;

   if( (f_image = fopen(nom_image, "rb")) == NULL)
      {
	 printf("\nPas d'acces en lecture sur l'image %s \n", nom_image);
	 exit(EXIT_FAILURE);
      }
   else
      {
	fscanf(f_image, "P5 ");
	ignorer_commentaires(f_image);
	fscanf(f_image, "%d %d %d%*c",
	       &nb_colonnes, &nb_lignes, &max_grey_val); /*lecture entete*/

	 if( (fread((OCTET*)pt_image, sizeof(OCTET), taille_image, f_image))
	     !=  (size_t) taille_image)
	    {
	       printf("\nErreur de lecture de l'image %s \n", nom_image);
	       exit(EXIT_FAILURE);
	    }
	 fclose(f_image);
      }
}
/*===========================================================================*/

void erosion_seuillee_pgm(OCTET*ImgIn, OCTET*ImgOut, int nH,int nW){
    for (int i=0; i < nH; i++){
      for (int j=0; j < nW; j++)
         {
         if ((j>0 && ImgIn[i*nW+j-1]==255)){
               ImgOut[i*nW+j]=255;
         }
         else if((j<nW-1 && ImgIn[i*nW+j+1]==255)){
               ImgOut[i*nW+j]=255;
         }
         else if((i>0 && ImgIn[(i-1)*nW+j]==255)){
               ImgOut[i*nW+j]=255;
         }
         else if((i<nH-1 && ImgIn[(i+1)*nW+j]==255)){
               ImgOut[i*nW+j]=255;
         }
         else if((j>0 && i>0 && ImgIn[(i-1)*nW+j-1]==255)){
               ImgOut[i*nW+j]=255;
         }
         else if((j<nW-1 && i>0 && ImgIn[(i-1)*nW+j+1]==255)){
               ImgOut[i*nW+j]=255;
         }
         else if((j>0 && i<nH-1 && ImgIn[(i+1)*nW+j-1]==255)){
               ImgOut[i*nW+j]=255;
         }
         else if( (j<nW-1 && i<nH-1 && ImgIn[(i+1)*nW+j+1]==255)){
               ImgOut[i*nW+j]=255;
         }
         else if(ImgIn[i*nW+j]==255){
               ImgOut[i*nW+j]=255;
         }
         else ImgOut[i*nW+j]=0;  
         }
   }
}

/*===========================================================================*/

void dilatation_seuillee_pgm(OCTET*ImgIn, OCTET*ImgOut, int nH,int nW){
    for (int i=0; i < nH; i++){
      for (int j=0; j < nW; j++){
         if ((j>0 && ImgIn[i*nW+j-1]==0)){
               ImgOut[i*nW+j]=0;
         }
         else if((j<nW-1 && ImgIn[i*nW+j+1]==0)){
               ImgOut[i*nW+j]=0;
         }
         else if((i>0 && ImgIn[(i-1)*nW+j]==0)){
               ImgOut[i*nW+j]=0;
         }
         else if((i<nH-1 && ImgIn[(i+1)*nW+j]==0)){
               ImgOut[i*nW+j]=0;
         }
         else if((j>0 && i>0 && ImgIn[(i-1)*nW+j-1]==0)){
               ImgOut[i*nW+j]=0;
         }
         else if((j<nW-1 && i>0 && ImgIn[(i-1)*nW+j+1]==0)){
               ImgOut[i*nW+j]=0;
         }
         else if((j>0 && i<nH-1 && ImgIn[(i+1)*nW+j-1]==0)){
               ImgOut[i*nW+j]=0;
         }
         else if( (j<nW-1 && i<nH-1 && ImgIn[(i+1)*nW+j+1]==0)){
               ImgOut[i*nW+j]=0;
         }
         else if(ImgIn[i*nW+j]==0){
               ImgOut[i*nW+j]=0;
         }
         else ImgOut[i*nW+j]=255;  
      }
   }
}

/*===========================================================================*/

void fermeture_seuillee_pgm(OCTET*ImgIn,OCTET*ImgOut,OCTET*ImgTmp,int nH,int nW){
   dilatation_seuillee_pgm(ImgIn,ImgTmp,nH,nW);
   erosion_seuillee_pgm(ImgTmp,ImgOut,nH,nW);
}

/*===========================================================================*/

void ouverture_seuillee_pgm(OCTET*ImgIn,OCTET*ImgOut,OCTET*ImgTmp,int nH,int nW){
   erosion_seuillee_pgm(ImgIn,ImgTmp,nH,nW);
   dilatation_seuillee_pgm(ImgTmp,ImgOut,nH,nW);
}
/*===========================================================================*/

void erosion_pgm(OCTET*ImgIn, OCTET*ImgOut, int nH,int nW){
   int min = 256;
    for (int i=0; i < nH; i++){
      for (int j=0; j < nW; j++)
         {
         if (j>0){
               if(min>ImgIn[i*nW+j-1]) min =ImgIn[i*nW+j-1];
         }
         else if(j<nW-1 ){
               if(min>ImgIn[i*nW+j+1]) min =ImgIn[i*nW+j+1];
         }
         else if(i>0 ){
               if(min>ImgIn[(i-1)*nW+j]) min =ImgIn[(i-1)*nW+j];
         }
         else if(i<nH-1){
               if(min>ImgIn[(i+1)*nW+j]) min =ImgIn[(i+1)*nW+j];
         }
         else if(j>0 && i>0){
               if(min>ImgIn[(i-1)*nW+j-1]) min =ImgIn[(i-1)*nW+j-1];
         }
         else if(j<nW-1 && i>0){
               if(min>ImgIn[(i-1)*nW+j+1]) min =ImgIn[(i-1)*nW+j+1];
         }
         else if(j>0 && i<nH-1){
               if(min>ImgIn[(i+1)*nW+j-1]) min =ImgIn[(i+1)*nW+j-1];
         }
         else if(j<nW-1 && i<nH-1){
               if(min>ImgIn[(i+1)*nW+j+1]) min =ImgIn[(i+1)*nW+j+1];
         }
         else if(min>ImgIn[(i)*nW+j]) min =ImgIn[(i)*nW+j];
         ImgOut[i*nW+j]=min;
         min=256;  
      }
   }
}

/*===========================================================================*/

void dilatation_pgm(OCTET*ImgIn, OCTET*ImgOut, int nH,int nW){
      int max = 0;
    for (int i=0; i < nH; i++){
      for (int j=0; j < nW; j++){
         if (j>0){
               if(max<ImgIn[i*nW+j-1]) max =ImgIn[i*nW+j-1];
         }
         else if(j<nW-1){
               if(max<ImgIn[i*nW+j+1]) max =ImgIn[i*nW+j+1];
         }
         else if(i>0){
               if(max<ImgIn[(i-1)*nW+j]) max =ImgIn[(i-1)*nW+j];
         }
         else if(i<nH-1){
               if(max<ImgIn[(i+1)*nW+j]) max =ImgIn[(i+1)*nW+j];
         }
         else if(j>0 && i>0 ){
               if(max<ImgIn[(i-1)*nW+j-1]) max =ImgIn[(i-1)*nW+j-1];
         }
         else if(j<nW-1 && i>0){
               if(max<ImgIn[(i-1)*nW+j+1]) max =ImgIn[(i-1)*nW+j+1];
         }
         else if(j>0 && i<nH-1){
               if(max<ImgIn[(i+1)*nW+j-1]) max =ImgIn[(i+1)*nW+j-1];
         }
         else if(j<nW-1 && i<nH-1){
               if(max<ImgIn[(i+1)*nW+j+1]) max =ImgIn[(i+1)*nW+j+1];
         }
         else if(max<ImgIn[(i)*nW+j]) max =ImgIn[(i)*nW+j];
         ImgOut[i*nW+j]=max;
         max=0;  
      }
   }
}

/*===========================================================================*/

void fermeture_pgm(OCTET*ImgIn,OCTET*ImgOut,OCTET*ImgTmp,int nH,int nW){
   dilatation_pgm(ImgIn,ImgTmp,nH,nW);
   erosion_pgm(ImgTmp,ImgOut,nH,nW);
}

/*===========================================================================*/

void ouverture_pgm(OCTET*ImgIn,OCTET*ImgOut,OCTET*ImgTmp,int nH,int nW){
   erosion_pgm(ImgIn,ImgTmp,nH,nW);
   dilatation_pgm(ImgTmp,ImgOut,nH,nW);
}