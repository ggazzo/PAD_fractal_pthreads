#include <sys/time.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/timeb.h>
void color(char *  data, int red, int green, int blue);


int itermax = 1000;		/* how many iterations to do	*/
double magnify =1.0;		/* no magnification		*/
int hxres = 5000;		/* horizonal resolution		*/
int hyres = 5000;		/* vertical resolution		*/

int colorize = 0;		/* vertical resolution		*/

char outputfile[256] = "./teste.ppm";

#define NUM_THREADS_DEFAULT sysconf(_SC_NPROCESSORS_ONLN)

struct color {
	int r,g,b;
};

int num_threads;
struct _header {
	int x;
	int y;
	int slice;
	int hyres;
	int hxres;
	struct color *color;

	char * data;
};

void *inc_x(void *x_void_ptr)
{
int iteration,hx,hy;

double x,xx,y,cx,cy;

struct _header *header = (struct _header *)x_void_ptr;

char * data  = header->data;

for (int hy=header->y + 1;hy<=header->y +header->slice;hy++)  {

	for (int hx=header->x + 1; hx<= header->hxres ;hx++)  {

		cx = (((float)hx)/((float)header->hxres)-0.5)/magnify*3.0-0.7;
		cy = (((float)hy)/((float)header->hyres)-0.5)/magnify*3.0;
		x = 0.0; y = 0.0;
		for (iteration=1;iteration<itermax;iteration++)  {
			xx = x*x-y*y+cx;
			y = 2.0*x*y+cy;
			x = xx;
			if (x*x+y*y>100.0)  iteration = 999999;
		}
		if (iteration<99999){

		 color(&data[(hx-1)*3+ 3*(hy-1)*header->hxres],0,0,0);
		}
		else{
		 color(&data[(hx-1)*3+ 3*(hy-1)*header->hxres],header->color->r,header->color->g,header->color->b);
		}
		// if(--header->slice == 0){return NULL;}
	}
}/* the function must return something - NULL will do */
return NULL;

}

void print_values(){
	printf("cores:%d threads:%d\n\nsize square:%d\nmagnify:%f\nintermax:%d\noutput file:%s\npaint thread:%s\n", (int)NUM_THREADS_DEFAULT , num_threads, hxres,magnify ,itermax, outputfile,colorize?"true":"false");
}

void print_options(){
	printf("-intermax %%d\n-magnify %%f\n-size %%d\n-t %%d num of threads\n-c %%d [0,1] will paint de background\n-o %%s outputfile\n--help, -help, -h\n\n\n");
	print_values();
}
int main(int argc, char * argv[])
{
	num_threads =  NUM_THREADS_DEFAULT;
	struct timeval t1, t2;



	char * data;

	for(int i = 0; i < argc; ){

		if(strcmp(argv[i],"-help")==0||strcmp(argv[i],"--help")==0||strcmp(argv[i],"-h")==0){
			print_options();
		 return 0;
		}
		if(strcmp(argv[i],"-intermax")==0){
		 if(sscanf(argv[i+1],"%d", &itermax)!=1) {
	      	fprintf(stderr, "intermax invalid\n" );
					print_options();
					return -1;
	  	}
			i = i + 2;
			continue;
		}

		if(strcmp(argv[i],"-c")==0){
		 if(sscanf(argv[i+1],"%d", &colorize)!=1) {
	      	fprintf(stderr, "intermax invalid\n" );
					print_options();
					return -1;
	  	}
			i = i + 2;
			continue;
		}

		if(strcmp(argv[i],"-magnify")==0){
			printf("(opa)\n");
		 if(sscanf(argv[i+1],"%lf", &magnify)!=1) {
					fprintf(stderr, "magnify invalid\n\n" );
			 		print_options();
					return -1;
	  	}
			i = i + 2;
			continue;
		}


		if(strcmp(argv[i],"-size")==0){
		 if(sscanf(argv[i+1],"%d", &hxres)!=1) {
				fprintf(stderr, "size invalid\n\n");
			 	print_options();
				return -1;
			}
			hyres = hxres;
			i = i + 2;
			continue;
		}

		if(strcmp(argv[i],"-t")==0){
		 if(sscanf(argv[i+1],"%d", &num_threads)!=1) {
				fprintf(stderr, "size invalid\n\n" );
			 	print_options();
				return -1;
			}
			hyres = hxres;
			i = i + 2;
			continue;
		}

		if(strcmp(argv[i],"-o")==0){
		 if(sscanf(argv[i+1],"%[^n]{0,250}s", outputfile)!=1) {
					fprintf(stderr, "output invalid\n\n" );
			 		print_options();
					return -1;
			}
			hyres = hxres;
			i = i + 2;
			continue;
		}
		i = i + 1;
	}

	print_values();
	gettimeofday(&t1, NULL);

	data = (char *) malloc(sizeof(char)*3*hxres*hyres);

	struct _header * header =  (struct _header *) malloc(sizeof(struct _header)* num_threads);

	pthread_t  * inc_x_thread = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);

	int total = hxres; //rgb
	int slice = total /num_threads;

	// printf("----%d %d-----\n", slice, total);
	for(int i = 0; i < num_threads; i++){

		header[i].color = (struct color*)malloc(sizeof(struct color));
		header[i].color->r = colorize ? 200/num_threads * i + 50: 255;
		header[i].color->g = header[i].color->r;
		header[i].color->b = header[i].color->r;
		header[i].data = data;
		header[i].x = 0;
		header[i].y = i * (int)(hxres/ num_threads);
		header[i].hyres=hyres;
		header[i].hxres=hxres;

		header[i].slice = num_threads-1 == i ? total - slice*i:slice;

		// printf("%d %d %d\n", header[i].x, header[i].y, header[i].slice);


		if(pthread_create(&inc_x_thread[i], NULL, inc_x, &header[i])) {
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}

	}

	for(int i = 0; i < num_threads; i++){
		if(pthread_join(inc_x_thread[i], NULL)) {
			fprintf(stderr, "Error joining thread\n");
			return 2;
		}
	}



	/* ... */
	gettimeofday(&t2, NULL);
	int milliSeconds = (t2.tv_sec - t1.tv_sec) * 1000 + (t2.tv_usec - t1.tv_usec)/1000;
	printf("time  = %d (milliseconds)\n", milliSeconds);



	FILE * file =  fopen(outputfile,"w");
	/* header for PPM output */
	fprintf(file, "P6\n# CREATOR: Guilherme Gazzo program\n");
	fprintf(file,"%d %d\n255\n",hxres,hyres);
	fwrite(data, sizeof(char), sizeof(char)*3*hxres*hyres, file);
	printf("\n\ndone. please check %s file \n",outputfile );
	 return 0;
}

void color(char *  data, int red, int green, int blue)  {

	data[0] = red;
	data[1] = green;
	data[2] = blue;

}
