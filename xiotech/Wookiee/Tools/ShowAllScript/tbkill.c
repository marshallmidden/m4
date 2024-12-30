#include <stdio.h>

int tbindex(char *s,char *t)   /* My simple indexer...find position of t in s */
{
  int i,j,k;
  for(i=0;s[i]!='\0';i++) {
    for(j=i,k=0;t[k]!='\0' && tolower(s[j])==tolower(t[k]);j++,k++);
    if(t[k]=='\0') return(i);
  }
  return(-1);
}

main(int argc, char *argv[])
{
  char tmp[200];
  unsigned long v1,v2,v3;
  FILE *ifp;
  printf("argc=%d\n",argc);
  if(argc!=2) exit(0);
  sprintf(tmp,"ps ax >/tmp/psit.out");
  system(tmp);
  ifp=fopen("/tmp/psit.out","r");
  if(ifp!=NULL) {
     while(fgets(tmp,199,ifp) != NULL) {
        if(tbindex(tmp,argv[1])>0) {
           if(tbindex(tmp,"tbkill")<0) {
             sscanf(tmp,"%ld",&v3);
             sprintf(tmp,"kill -HUP %ld",v3);
             system(tmp);
             printf("%s\n",tmp);
           }
        }
     }
  }
}
