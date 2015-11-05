//·dtbuf
#include"lzfx.h"
#include"lzfx.c"
#include<stdio.h>
#include<windows.h>
#include<process.h>
//#define tnum 4
//#define roundsize 1024
//#define tsize (roundsize*1024)/(tnum*2)
int tnum;
int tsize;
char a[20],b[20];
class chunk
{
public:
  chunk()
  {

      chunkmode=0;
      size=tsize*2;
      inputsize=0;
      workdown=0;
  }
  ~chunk()
  {
      free(buf);
      free(inputbuf);
  }
  void clean()
  {
      start=0;
      end=0;
 /*     free(buf);
      buf=(unsigned char*) malloc (sizeof(unsigned char)*tsize*2);*/
        inputsize=0;
      size=tsize*2;
  }
    int cid;
  long start;
  long end;
  unsigned char * buf;
  unsigned char *inputbuf;
  unsigned int inputsize;
  unsigned int size;
  int chunkmode;
  char workdown;
};
void worker(int*);
void fuckyouFREAD(int *);
long lSize;
unsigned char * buffer;
unsigned char *debuf;
unsigned char *tbuf;
unsigned char *lastchunk;
long    deSize;
DWORD tid;

int chunksize[100000];
int csizecnt;
HANDLE S[16];   //start
HANDLE E[16];   //end
chunk C[16];
HANDLE T[16];
HANDLE boss;
HANDLE bosswait;
int id[16];
int cccc;
int wc;
int lastchunknum=0;

    FILE *test;
    FILE * pFile;
    FILE * decodefile;
    FILE *out;
int main()
{

	LARGE_INTEGER ticksPerSecond;
LARGE_INTEGER start_tick;
LARGE_INTEGER end_tick;
QueryPerformanceFrequency(&ticksPerSecond);
QueryPerformanceCounter(&start_tick);

    printf("inputfile name:");
    scanf("%s",a);

    printf("outputfile name:");
    scanf("%s",b);

    printf("worker number:");
    scanf("%d",&tnum);

    printf("chunk size:");
    scanf("%d",&tsize);
    tsize=tsize*1024;
    out=fopen("fuckyou.haha","wb");

    pFile = fopen ( a , "rb" );

    size_t result;

    if (pFile==NULL) {fputs ("File error",stderr); }

    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);
    if(lSize==0)
    {
        fclose(pFile);
        fclose(out);
    }
    // allocate memory to contain the whole file:
    buffer = (unsigned char*) malloc (sizeof(unsigned char)*lSize);
    if (buffer == NULL) {fputs ("Memory error",stderr);}

    // copy the file into the buffer:
    result = fread (buffer,1,lSize,pFile);
    if (result != lSize) {fputs ("Reading error",stderr);}
      //  for(int i=0;i<lSize;i++)printf("%c",buffer[i]);


    //init
    //tbuf=(unsigned char*) malloc (sizeof(unsigned char)*tsize*4);
    for(int i=0;i<tnum;i++)
    {
        id[i]=i;
        C[i].buf=(unsigned char*) malloc (sizeof(unsigned char)*tsize*3);
        C[i].inputbuf=(unsigned char*) malloc (sizeof(unsigned char)*tsize*3);
        S[i]=CreateSemaphore( NULL, 0, 1, NULL );
        E[i]=CreateSemaphore( NULL, 0, 1, NULL );
        T[i] = CreateThread( NULL, 0,(LPTHREAD_START_ROUTINE)worker,id+i, 0, &tid );

    }
    int nouse=0;
    bosswait = CreateSemaphore( NULL, 0, 1, NULL );
    boss = CreateThread( NULL, 0,(LPTHREAD_START_ROUTINE)fuckyouFREAD,&nouse, 0, &tid );
    WaitForSingleObject( bosswait, INFINITE );
	QueryPerformanceCounter(&end_tick);
	double elapsed = ((double)(end_tick.QuadPart - start_tick.QuadPart) / ticksPerSecond.QuadPart);
	printf("time=%f\n",elapsed);
}
void fuckyouFREAD(int *nouse)
{

    long fileptr=0;
    csizecnt=0;
    cccc=0;
    wc=0;

    //start work
    int count=0;
    int incount=0;
    while(fileptr<lSize)
    {

        int last=-1;
        for(int i=0;i<tnum;i++)
        {
            C[i].clean();
            C[i].start=fileptr;
            if(fileptr==lSize){C[i].workdown=1;}
            else if((fileptr+tsize) < lSize)
            {
                ++incount;
                fileptr=fileptr+tsize;
                C[i].end=fileptr;
                ReleaseSemaphore(S[i], 1, NULL);
            }
            else
            {

                ++incount;
                fileptr =lSize;
                C[i].end=fileptr;
                ReleaseSemaphore(S[i], 1, NULL);
            }

        }

        for(int j=0;j<tnum;j++)
        {
            if(C[j].workdown==0)
            {

                WaitForSingleObject( E[j], INFINITE );

                chunksize[csizecnt]=C[j].size;
                csizecnt++;

                wc+=fwrite (C[j].buf , 1 , C[j].size, out );
                //dealing with last chunk


                //printf("%d write %d\n",j,a);
            }
        }

       // printf("round %d fileptr:%d lSize:%d \n",++count,fileptr,lSize);
    }
   // printf("over! incount=%d %d\n",incount ,csizecnt);
   // system("pause");
    free(buffer);

    /////////////////////////////////////////////////////////////////////////
    //decode init

    fclose(pFile);
    fclose(out);

    decodefile=fopen("fuckyou.haha","rb");
    test=fopen(b,"wb");

    fseek (decodefile , 0 , SEEK_END);
    deSize = ftell (decodefile);
    rewind (decodefile);
    //printf(" lsize=%d cccc=%d wc=%d desize=%d\n",lSize,cccc,wc,deSize);
    //system("pause");
  //  debuf=(unsigned char*) malloc (sizeof(unsigned char)*tsize*2);
    //buffer=(unsigned char*) malloc (sizeof(unsigned char)*lSize);


    int ccount=0;
    int roundcount=0;
     for(int i=0;i<tnum;i++)
     {
         C[i].chunkmode=1;//decodemode
         C[i].workdown=0;
     }

        while(ccount<csizecnt)
        {
            //printf("rund %d\n",roundcount);
            roundcount++;
            for(int i=0;i<tnum;i++)
            {
                C[i].clean();
                C[i].size=tsize*2;
                if(ccount<csizecnt)
                {

                     //printf("%d:(3) ccount=%d\n",i,ccount);
                    C[i].cid=ccount;

                    int s=fread(C[i].inputbuf,1,chunksize[ccount],decodefile);
                    if(s!=chunksize[ccount])
                    {
                        printf("%d only read %d\n",chunksize[ccount],s);
                        //system("pause");
                    }
                    C[i].inputsize=chunksize[ccount];
                    ccount++;
                    ReleaseSemaphore(S[i], 1, NULL);
                    //printf("%d:(1) ccount=%d\n",i,ccount);
                }

                else
                {
                  C[i].workdown=1;
                  ccount++;
                }
            }
            for(int j=0;j<tnum;j++)
            {
                if(C[j].workdown==0)
                {
                     //printf("%d wait start!\n",j);
                    WaitForSingleObject( E[j], INFINITE );
                    //printf("%d write %d words!\n",j,C[j].size);
                    fwrite (C[j].buf , 1 , C[j].size,test);

                }
            }

            //if(t!=chunksize[i])printf("catch!! no. %d chunk only read %d/%d\n",i,t,chunksize[i]);
        }
        ReleaseSemaphore(bosswait, 1, NULL);
        ExitThread( 0 );

}







void worker(int *x )
{

    while(1)
    {
        //printf("%d wait  ",*x);
        WaitForSingleObject( S[*x], INFINITE );
        if(C[*x].chunkmode==0)
        {
            int k=lzfx_compress(
            &buffer[C[*x].start], 		//input buffer
            C[*x].end-C[*x].start,   	//input buffer size
            C[*x].buf, 			//output buffer
            &C[*x].size);

            cccc+=C[*x].size;
            if(k!=0)printf("%d start:%d  end:%d  success? %d size:  %d cccc:%d\n",*x,C[*x].start,C[*x].end,k,C[*x].size,cccc);
        }
        else
        {
            int k=lzfx_decompress(
            C[*x].inputbuf, 		//input buffer
            C[*x].inputsize,   	//input buffer size
            C[*x].buf, 			//output buffer
            &C[*x].size);
           // printf("%d :success? %d  id=%d\n",*x,k,C[*x].cid);
            //printf("%s",C[*x].buf);
            //system("pause");
        }
        ReleaseSemaphore(E[*x], 1, NULL);
    }
}
