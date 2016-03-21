/*HW02 MUHAMMET TAYYIP CANKAYA 131044054*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#define MAX 1024	//path icin max uzunluk
#define OUTPUTFILE "gfd.log"

int grepfromFile(FILE *iptr,FILE *optr,char* argv2);
pid_t  grepfromDir(FILE *optr,char *argv1,char *argv2,char*temp);

int main(int argc, char *argv[])
{
	char mycwd[PATH_MAX];
	/*Eger 3 parametre girilmediyse hata verip cikiyor. */
	if (argc != 3) {
		fprintf(stderr,"Kullanım: %s Directory adi String \n", argv[0]);
		exit(1);}
	
	
	FILE *optr=fopen(OUTPUTFILE,"w");
	if (getcwd(mycwd, PATH_MAX) == NULL) {
		perror("Failed to get current working directory");
		return 1;}

	/*Burada calistigimiz directory de MTC isimli bir dosya olusturuyorum.Bu dosya daha sonra
	olustaracagim gecici log dosyalarini tutmama yarayacak.*/	
	strcat(mycwd,"/MTC/");	
    mkdir(mycwd,0755);

    /*asil fonksiyonumu cagiriyorumç*/
    grepfromDir(optr,argv[1],argv[2],mycwd);
		
	fclose(optr);
	return 0; 
}

/*gonderilen directorydeki file ve directory lere bakip iclerinde kactane istenen stringten
bulundugunu yazan fonksiyon.*/
pid_t grepfromDir(FILE *optr,char *argv1,char *argv2,char*temp)
{
    char mycwd[MAX];
    struct dirent *direntp;
    struct stat status;    
    DIR *directory;
    int result;
	pid_t childpid=getpid();
	pid_t res;


	/*directory nin NULL olma durumuna bakiyorum*/
    if ((directory = opendir(argv1))== NULL) {
        fprintf(stderr,"Directory acilamadi veya bulunamadi.\n");
        exit(-1);}


    /*Null degilse directory icini okumaya basliyorum*/    
    while ((direntp = readdir(directory)) != NULL) 
    {
        /*recursion i ilerletmek icin mycwd yi daha sonra argv1 parametresi yerine gonderecegim. */
        sprintf(mycwd,"%s/%s",argv1,direntp->d_name);
        
        /*directory olarak . ve .. dosyalari degilse islemleri yapiyorum.*/
        if (strcmp(direntp->d_name, ".")!= 0 && strcmp(direntp->d_name, "..")!= 0) 
        {
            /*file durumuna bakiyorum.*/
            if (stat(mycwd, &status) == -1){
                fprintf(stderr,"Failed to get file status.\n");
                break;}

            /*directory olma durumunda recursion olarak tekrar bu fonksiyonu cagiriyorum.*/
            if (direntp->d_type==DT_DIR){
                res=grepfromDir(optr,mycwd,argv2,temp);

                if(res==childpid)
                	return childpid;}

            /*eger directory degilde file sa fork() yapiyorum.*/    
           	else if(direntp->d_type==DT_REG)
           	{	
				FILE*iptr=fopen(mycwd,"r");
				char path[MAX]="";
				char logFilePath[MAX]="";
				strncpy(path,direntp->d_name,strlen(direntp->d_name)-4);
				strcat(path,".log");
				strcat(logFilePath,temp);
				strcat(logFilePath,path);

				FILE*dosya=fopen(logFilePath,"w");
				childpid=fork();
				
				/*forkla olusan cocuksa grepfromfile dosyasini cagiriyorum.*/
				if(childpid==0){			
					printf("\nFile Name==>%s\n", direntp->d_name);
					fprintf(dosya,"\nFile Name==>%s\n", direntp->d_name); 
					grepfromFile(iptr,dosya,argv2);
					return childpid;
				}

				/*anne process cocugun yazdigi log dosyalarini gfd.log a yazacak.ve cocugun yazdiklarini silecek.*/
				else if(childpid>0){				
				while(wait(NULL)>0);}

				/*fork hatali ise*/
				else{
					printf("Fork failed\n");
					exit(0);}		  	
			}
        }   
    }/*directory i kapatma islemi.*/
    while ((closedir(directory) == -1) && (errno == EINTR));

}




/*-------------------------------------------------------------------------------------------*/
int grepfromFile(FILE *iptr,FILE *optr,char* argv2)
{
	int kacTaneVar=0,temp,sbt=0,dosyadakiHarfSayisi=0,i=0;
	char *metin,karakter;
	int kelimeBoyutu=strlen(argv2);
	int satir=1,sutun=1,count=1,index,sayac;	

	/*Burada okuyacagim dosyada kac tane karakter oldugunu buluyorum.*/
	while(!feof(iptr)){
		fscanf(iptr,"%c",&karakter);
		dosyadakiHarfSayisi++;}
	
	/*Dosyada kac tane karakter oldugunu yukarida bulmustum.malloc la o boyutta yer aliyorum.*/
	metin=(char*)malloc(sizeof(char)*dosyadakiHarfSayisi);


	/*Bu ozel fonksiyon dosyanin konum gostergecini dosya basina aliyor.*/
	rewind(iptr);
	/*Metin isimli char*'a okuyorum.*/
	while(!feof(iptr)){
		fscanf(iptr,"%c",&metin[i]);
		i++;}
	

	/*for icerisinde arana ilk karakterle dosyadaki karakterlere bakip ayniysa aranan kelimenin boyutu
	kadar karsilastirma yapip bulundu deyip count'u bir artiriyorum.*/
	
	for(index=0;index<dosyadakiHarfSayisi;++index)
	{
		temp=index;

		if(metin[index]=='\n'){
			satir++;		
			sutun=0;}
		
		if(metin[index]==argv2[0]){
			for(sayac=1;sayac<kelimeBoyutu;sayac++){
				if(argv2[sayac]==metin[temp+1])
					sbt++;
			
				temp++;	

				if(sbt==kelimeBoyutu-1){
					kacTaneVar++;
					printf("%d-->%d.satir %d.sutunda var\n",count,satir,sutun);
					fprintf(optr,"%d-->%d.satir %d.sutunda var\n",count,satir,sutun);
					count++;}}}
		sbt=0;
		sutun++;	
	}
	
	/*Malloc la aldigim yeri geri veriyorum.*/
	free(metin);
	/*Toplamda kac tane bulundugunu return ediyorum.*/
	return kacTaneVar;
}