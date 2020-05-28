#include <stdio.h>
#include <stdlib.h>
#include "dc_image.h"
#include <time.h>


// #define CANNY_THRESH 45
// #define CANNY_BLUR   10

#define MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#define MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#define ABS(x)    ( (x) <= 0 ? 0-(x) : (x) )

int main()
{
	int i,j;
	int y,x;
	int temp;
	int rows, cols, chan;
	int x1,x2,y1,y2;
	int choice;
	int count;

	srand(time(0));
	//-----------------
	// Read the image    [y][x][c]   y number rows   x cols  c 3
	//-----------------
	byte ***img;
	int CANNY_THRESH;
	int CANNY_BLUR;
	for (choice=0;choice<4;choice++)
	{
		if (choice==0)
		{
			CANNY_BLUR=10;
			CANNY_THRESH=45;

			img = LoadRgb("pentagon.png", &rows, &cols, &chan);	

		printf("img %p rows %d cols %d chan %d\n", img, rows, cols, chan);
	}
	else if (choice==1)
	{
		CANNY_BLUR=2;
		CANNY_THRESH=148;

		img = LoadRgb("sidewalk.png", &rows, &cols, &chan);	
		printf("img %p rows %d cols %d chan %d\n", img, rows, cols, chan);	
	}
	else if (choice==2)
	{
		CANNY_BLUR=6;
		CANNY_THRESH=90;

		img = LoadRgb("puppy.png", &rows, &cols, &chan);	
		printf("img %p rows %d cols %d chan %d\n", img, rows, cols, chan);
	}

	else if (choice==3)
	{
		CANNY_BLUR=3;
		CANNY_THRESH=167;
		img = LoadRgb("building.png", &rows, &cols, &chan);	
		printf("img %p rows %d cols %d chan %d\n", img, rows, cols, chan);
	}


	/* byte ***img = LoadRgb("pentagon.png", &rows, &cols, &chan);	

	printf("img %p rows %d cols %d chan %d\n", img, rows, cols, chan);
 */
	
	//SaveRgbPng(img, "out/1_img.png", rows, cols);
	
	
	
	//-----------------
	// Convert to Grayscale
	//-----------------
	byte **gray = malloc2d(rows, cols);
	for (y=0; y<rows; y++){
		for (x=0; x<cols; x++) {
			int r = img[y][x][0];
			int g = img[y][x][1];
			int b = img[y][x][2];
			gray[y][x] =  (r+g+b) / 3;
		}
	}
	
	//SaveGrayPng(gray, "out/2_gray.png", rows, cols);
	
	

	//-----------------
	// Box Blur   ToDo: Gaussian Blur is better
	//-----------------
	
	// Box blur is separable, so separately blur x and y
	int k_x=CANNY_BLUR, k_y=CANNY_BLUR;
	
	// blur in the x dimension
	byte **blurx = (byte**)malloc2d(rows, cols);
	for (y=0; y<rows; y++) {
		for (x=0; x<cols; x++) {
			
			// Start and end to blur
			int minx = x-k_x/2;      // k_x/2 left of pixel
			int maxx = minx + k_x;   // k_x/2 right of pixel
			minx = MAX(minx, 0);     // keep in bounds
			maxx = MIN(maxx, cols);
			
			// average blur it
			int x2;
			int total = 0;
			int count = 0;
			for (x2=minx; x2<maxx; x2++) {
				total += gray[y][x2];    // use "gray" as input
				count++; 
			}
			blurx[y][x] = total / count; // blurx is output
		}
	}
	
	//SaveGrayPng(blurx, "out/3_blur_just_x.png", rows, cols);
	
	// blur in the y dimension
	byte **blur = (byte**)malloc2d(rows, cols);
	for (y=0; y<rows; y++) {
		for (x=0; x<cols; x++) {
			
			// Start and end to blur
			int miny = y-k_y/2;      // k_x/2 left of pixel
			int maxy = miny + k_y;   // k_x/2 right of pixel
			miny = MAX(miny, 0);     // keep in bounds
			maxy = MIN(maxy, rows);
			
			// average blur it
			int y2;
			int total = 0;
			int count = 0;
			for (y2=miny; y2<maxy; y2++) {
				total += blurx[y2][x];    // use blurx as input
				count++;
			}
			blur[y][x] = total / count;   // blur is output
		}
	}
	
	//SaveGrayPng(blur, "out/3_blur.png", rows, cols);
	
	
	//-----------------
	// Take the "Sobel" (magnitude of derivative)
	//  (Actually we'll make up something similar)
	//-----------------
	
	byte **sobel = (byte**)malloc2d(rows, cols);
	
	for (y=0; y<rows; y++) {
		for (x=0; x<cols; x++) {
			int mag=0;
			
			if (y>0)      mag += ABS(blur[y-1][x] - blur[y][x]);
			if (x>0)      mag += ABS(blur[y][x-1] - blur[y][x]);
			if (y<rows-1) mag += ABS(blur[y+1][x] - blur[y][x]);
			if (x<cols-1) mag += ABS(blur[y][x+1] - blur[y][x]);
			
			sobel[y][x] = 3*mag;
		}
	}
	
	//SaveGrayPng(sobel, "out/4_sobel.png", rows, cols);
	
	//-----------------
	// Non-max suppression
	//-----------------
	byte **nonmax = malloc2d(rows, cols);    // note: *this* initializes to zero!
	if (choice==0)
	{
for (y=1; y<rows-1; y++)
	{
		for (x=1; x<cols-1; x++)
		{
			// Is it a local maximum
			int is_y_max = (sobel[y][x] > sobel[y-1][x] && sobel[y][x]>=sobel[y+1][x]);
			int is_x_max = (sobel[y][x] > sobel[y][x-1] && sobel[y][x]>=sobel[y][x+1]);
			if (is_y_max || is_x_max)
				nonmax[y][x] = 255;
			else
				nonmax[y][x] = 0;
		}
	}
	}
	else
	{
		nonmax=sobel;
	}
	
	
	
	//SaveGrayPng(nonmax, "out/5_nonmax.png", rows, cols);
	
	//-----------------
	// Final Threshold
	//-----------------
	byte **edges = malloc2d(rows, cols);    // note: *this* initializes to zero!
	
	for (y=0; y<rows; y++) {
		for (x=0; x<cols; x++) {
			if (nonmax[y][x] > CANNY_THRESH)
				edges[y][x] = 255;
			else
				edges[y][x] = 0;
		}
	}
	
	//SaveGrayPng(edges, "out/6_edges.png", rows, cols);
	
	//generating a line
	//byte **line = malloc2d(rows, cols);
byte ***edges1 = malloc3d(rows, cols, chan);
for (y=0; y<rows; y++){
		for (x=0; x<cols; x++) {
			
			if (edges[y][x]==255) {
				edges1[y][x][0] = 255;
				edges1[y][x][1] = 255;
				edges1[y][x][2] = 255;
				
			}
		}
}
int iter = 10000;
int count_array [iter];
float m_array[iter];
int count_array_max[5];
int count_array_copy[iter];
int element_index[iter];
float c_array[iter];
float m,c;
for (int i=0;i<iter;i++)
{
	
	count=0;
	/*int count=0;
	int count_array [10000];
	float m_array[10000];
	int count_array_max[5];
	int count_array_copy[10000];
	int element_index[5];
	float c_array[10000];*/
	x1 = rand() % cols;
    y1 = rand() % rows;
    x2 = rand() % cols;
    y2 = rand() % rows;
	if (y2==y1)
	{
		/* code */
		y2 += 1;
	}
	 
	//m=(y2-y1)/(x2-x1);
	m=(x2-x1)/(y2-y1);
	// printf("%d\n", m);
	//c=(y1-(m*x1));
	c=(x1-(m*y1));	
	for (y=0; y<rows; y++)
	{
		for (x=0; x<cols; x++)
		 {
			 int x1 = m* y +c;
			if ((x ==  x1) && (edges[y][x] == 255))
			{
				count +=1;
		
			//edges[y][x] = 128;
		//line[y][x]=255;
		}	
		
		}
	} 
	//printf("\nupdated count: %d", count);
	count_array[i]=count;
	count_array_copy[i]=count;
	m_array[i]=m;
	c_array[i]=c;
}

// sort the array 
for (int i = 0; i < iter; i++)
	{
		
		for (int j = i + 1; j < iter; j++)
		{
			if(count_array[i] < count_array[j])
			{
				temp = count_array[i];
				count_array[i] = count_array[j];
				count_array[j] = temp;
			}
			
		}
	}

// for (i=0;i<5;i++)
// {
	
// 	// count_array_max[i]=count_array[i];
// 	printf(" \nmax array %d\n",count_array[element_index[i]]);
// }


for (i = 0; i < iter; i++)
  {		
	  for (j=0;j<iter;j++)
	  {	
		if (count_array[i]==count_array_copy[j]) {
				element_index[i] = j;
		count_array_copy[j]= -1;
		break;  
		  }
	  }
  }
		
  
float sorted_c_array[iter];
float sorted_m_array[iter];  
for (i=0;i<iter;i++)
  {
	  
	  sorted_m_array[i]=m_array[element_index[i]];
	  sorted_c_array[i]=c_array[element_index[i]];
	  
  }
  

/* float Status_Check[5][2];
int status_flag=1;
 for (j=0;j<5;i++)
{ 
 for (i=0;i<5;i++)
{
	Status_Check[0][i]=m_array[i];
	Status_Check[i][0]=c_array[i];
	printf("m:%f c:%f",Status_Check[0][i],Status_Check[i][0]);
} */
float m_array_new[5];
int cal=1;
float c_array_new[5];
//  for (i=0;i<100;i++)
// {
// 	printf(" %d  %f %f \n",i, sorted_c_array[i],sorted_m_array[i]);
// }
 for (i=0;i<iter;i++)
 {
 
	 int range=100;
	 int add_slope = 1;

	 for (int j = 0; j<cal; j++)
	 {
		if ((sorted_m_array[i]==0.0) && (sorted_c_array[i]==0.0)) {
			add_slope = 0;
		break;
	}
		//printf("slope %d %d \n ", sorted_m_array[i], m_array_new[j]);
	 if (( sorted_m_array[i]==m_array_new[j]) && (abs(abs(sorted_c_array[i])-abs(c_array_new[j]))<=range))
	 {
		
		 add_slope = 0;
		 break;
		//printf("Cal: %d M: %f C: %f \n",cal, m_array_new[cal],c_array_new[cal]);

	 }
	 }
	 if (add_slope == 1) {
		 m_array_new[cal-1]=sorted_m_array[i];
		 c_array_new[cal-1]=sorted_c_array[i];
		 cal=cal+1;
	 }
	
	 if (cal==6)
	 {
		 break;
	 }
 }
 /* for (i=0;i<5;i++)
{
	printf("Slope: %f Intercept: %f Count_Array:%d \n",m_array_new[i],c_array_new[i], count_array[i]);
} */

	int width_line= 5;
  for (i=0;i<=5;i++)
  {
	  
	for (y=0; y<rows; y++){
		for (x=0; x<cols; x++) {
			for (int j= -width_line; j<=width_line; j++) {
			c = c_array_new[i]+j;
			if ((x==(m_array_new[i]*y)+ c))
			{
				 if (edges[y][x]==255)
				 {
				edges1[y][x][0] = 0;
				edges1[y][x][1] = 255;
				edges1[y][x][2] = 0;
				 }
  }
  }}}
  
  }

  if (choice==0)
  {
	SaveRgbPng(edges1, "out/out_pentagon.png", rows, cols);  
  }
  else if (choice==1)
  {
	  SaveRgbPng(edges1, "out/out_sidewalk.png", rows, cols);
  }
  else if (choice==2)
  {
	  SaveRgbPng(edges1, "out/out_puppy.png", rows, cols);
  }
  else if (choice==3)
  {
	  SaveRgbPng(edges1, "out/out_building.png", rows, cols);
  }
  
  
	//SaveGrayPng(edges, "out/test.png", rows, cols);
	//SaveRgbPng(edges1, "out/test.png", rows, cols);
	printf("Done!\n");
	}
	return 0;

}
/*

	printf("load image\n");
	byte *data = stbi_load("puppy.jpg", &cols, &rows, &chan, 4);

	printf("data = %p\n", data);
	int rt=stbi_write_png("output.png", cols, rows, 4, data, cols*4);
*/