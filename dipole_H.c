#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

double *box;

double dist (int j, int n1, int i, int n2, double **x, double **y, double **z){
            
            double dx,dy,dz,r;

            dx=x[j][n1]-x[i][n2];
            dy=y[j][n1]-y[i][n2];
            dz=z[j][n1]-z[i][n2];

            dx = dx - box[0]*( (int)(2.0*dx/box[0]) - (int)(dx/box[0]) );
            dy = dy - box[1]*( (int)(2.0*dy/box[1]) - (int)(dy/box[1]) );
            dz = dz - box[2]*( (int)(2.0*dz/box[2]) - (int)(dz/box[2]) );

            r=sqrt(dx*dx+dy*dy+dz*dz);
            return r;
}



int main(int nvar,char **cvar){


  /*declaring variables and asking for inputs */
//---------------------------------------------------------------------------------

    FILE *rptr,*wptr1,*wptr2,*wptr3,*wptr4,*wptr5;
    int natom,nwater,ntotal,nmol,nframes,nskip,tstep,wn=0;
    int i,j,k,l,m,r,a,*bufftime,nregion;                        /* count1 is region1 and so on */
    int *count1,*count2,*count3,*count4;
    double **x, **y, **z, xx;
    double dpx1,dpy1,dpz1,dpx2,dpy2,dpz2,tdiff,cutoff1=2.6,cutoff2=5;          /* dpx1 is O-H for frame j  and dpx2 is for frame i */
    double *dp1,*dp2,*dp3,*dp4,*dplg1,*dplg2,*dplg3,*dplg4;
    char buf1[1000],buf2[1000], buf3[1000];
    int **region, *regionprev,*ifound,bufflimit,nmono=40;
    double *regiontot;

/*   give the name of the trajectory file after ./a.out    */
//----------------------------------------------------------------------------------

    if(nvar!=5){
      printf("  Usage: %s trajectoryfile %s abcboxsize \n",cvar[0],cvar[1]);
      exit(1);
    }
    rptr=fopen(cvar[1],"r");

    if(rptr==NULL){
        printf("  ERROR: could not open trajectory file %s\n",cvar[1]);
        exit(1);
    }
    box = (double *)malloc((3)*sizeof(double)); 
    sscanf(cvar[2], "%lf", &box[0]);
    sscanf(cvar[3], "%lf", &box[1]);
    sscanf(cvar[4], "%lf", &box[2]);
    printf(" Box size is %lf %lf %lf \n ",box[0],box[1],box[2]);

    printf("Enter the number of atoms in molecule, number of molecules, total number of water molecules present and number of regions \n");
    scanf("%d %d %d %d",&natom,&nmol,&nwater,&nregion);
   
    printf("Enter the total number of frames and skip value for reference frame and time betweem two frames \n");
    scanf("%d %d %lf",&nframes,&nskip,&tdiff);
    ntotal=natom*nmol+3*nwater;
    tstep=(int)20.0/tdiff;
    bufflimit=(int)1.0/tdiff;
    printf("Total number of atoms present in the system is %d bufflimit is %d \n",ntotal,bufflimit);

    wptr1=fopen("rotOside.dat","w");
    wptr2=fopen("rotC.dat","w");
    wptr3=fopen("rotB.dat","w");
    wptr4=fopen("regiontot.dat","w");
    wptr5=fopen("rotOmiddle.dat","w");



 /* allocate memory */
//----------------------------------------------------------------------------------
    x = (double **)malloc(nframes*sizeof(double *));
    for(i=0;i<nframes;i++) x[i] = (double *)malloc(ntotal*sizeof(double));
  
    y = (double **)malloc(nframes*sizeof(double *));
    for(i=0;i<nframes;i++) y[i] = (double *)malloc(ntotal*sizeof(double));
  
    z = (double **)malloc(nframes*sizeof(double *));
    for(i=0;i<nframes;i++) z[i] = (double *)malloc(ntotal*sizeof(double));


/*
  ount = (int **)malloc(nframes*sizeof(int *));
 for(i=0;i<nframes;i++) count[i] = (int *)malloc(nregion*sizeof(int));


  dp = (double **)malloc(nframes*sizeof(double *));
 for(i=0;i<nframes;i++) x[i] = (double *)malloc(nregion*sizeof(double));


  dplg = (double **)malloc(nframes*sizeof(double *));
 for(i=0;i<nframes;i++) x[i] = (double *)malloc(nregion*sizeof(double));

*/


    dplg1 = (double *)malloc((nframes)*sizeof(double));
    for(i=0;i<nframes;i++) dplg1[i]=0;

    dplg2 = (double *)malloc((nframes)*sizeof(double));
    for(i=0;i<nframes;i++) dplg2[i]=0;

    dplg3 = (double *)malloc((nframes)*sizeof(double));
    for(i=0;i<nframes;i++) dplg3[i]=0;

    dplg4 = (double *)malloc((nframes)*sizeof(double));
    for(i=0;i<nframes;i++) dplg4[i]=0;

    dp1 = (double *)malloc((nframes)*sizeof(double));
    for(i=0;i<nframes;i++) dp1[i]=0;

    dp2 = (double *)malloc((nframes)*sizeof(double));
    for(i=0;i<nframes;i++) dp2[i]=0;

    dp3 = (double *)malloc((nframes)*sizeof(double));
    for(i=0;i<nframes;i++) dp3[i]=0;
    
    dp4 = (double *)malloc((nframes)*sizeof(double));
    for(i=0;i<nframes;i++) dp4[i]=0;
    
    count1 = (int *)malloc((nframes)*sizeof(int));
    for(i=0;i<nframes;i++) count1[i]=0;

    count2 = (int *)malloc((nframes)*sizeof(int));
    for(i=0;i<nframes;i++) count2[i]=0;

    count3 = (int *)malloc((nframes)*sizeof(int));
    for(i=0;i<nframes;i++) count3[i]=0;

    count4 = (int *)malloc((nframes)*sizeof(int));
    for(i=0;i<nframes;i++) count4[i]=0;


    region = (int **)malloc(nframes*sizeof(int *));
    for(i=0;i<nframes;i++) region[i] = (int *)malloc(ntotal*sizeof(int));

    regionprev = (int *)malloc((ntotal)*sizeof(int));
    for(i=0;i<ntotal;i++) regionprev[i]=-1;

    ifound = (int *)malloc((ntotal)*sizeof(int));
    for(i=0;i<ntotal;i++) ifound[i]=-1;

    regiontot = (double *)malloc((nregion)*sizeof(double));
    for(i=0;i<nregion;i++) regiontot[i]=0;


    bufftime = (int *)malloc((ntotal)*sizeof(int));
    for(i=0;i<ntotal;i++) bufftime[i]=0;


/*
for(i=0;i<nframes;i++){for(j=0;j<nregion;j++)	count[i][j]=0;}
for(i=0;i<nframes;i++){for(j=0;j<nregion;j++)	dp[i][j]=0;}
for(i=0;i<nframes;i++){for(j=0;j<nregion;j++)	dplg[i][j]=0;}
*/

printf("MEMORY ALLOCATION  FINISHED");


 /* start reading frames */
 //-----------------------------------------------------------------------------------
    for(j=0;j<nframes;j++){
       fscanf(rptr,"%d",&ntotal);

//first fgets is to read the end line which is left at the end of first line by scanf

       fgets(buf1,1000,rptr);
       fgets(buf1,1000,rptr);


       for(i=0;i<ntotal;i++){
          fscanf(rptr,"%s %lf %lf %lf",buf2,&x[j][i],&y[j][i],&z[j][i]);
       }

    }

printf("FRAME READING  FINISHED");


/* Deciding the region for each water molecule */
//---------------------------------------------------------------------------------------

 for(j=0;j<nframes;j++){
    for(i=0;i<ntotal;i++) ifound[i]=-1;
       for(k=natom*nmol;k<ntotal;k=k+3){
          for(l=k+1;l<k+3;l++){

             for(a=0;a<nmol*natom;a=a+natom){
                if(dist(j,l,j,a+1,x,y,z)<cutoff1){ region[j][l]=3; ifound[l]=1; break; }
             }
             if(ifound[l]!=1){
               for(a=0;a<nmol*natom;a=a+natom){
                  if(dist(j,l,j,a+10,x,y,z)<cutoff1 || dist(j,l,j,a+13,x,y,z)<cutoff1){   region[j][l]=0; ifound[l]=1; break; }  
               }
             } 
             if(ifound[l]!=1){
                for(a=0;a<nmol*natom;a=a+natom){
                   if(dist(j,k,j,a+0,x,y,z)<cutoff2 || dist(j,k,j,a+4,x,y,z)<cutoff2 || dist(j,k,j,a+5,x,y,z)<cutoff2 || dist(j,k,j,a+6,x,y,z)<cutoff2 || dist(j,k,j,a+15,x,y,z)<cutoff2 || dist(j,k,j,a+16,x,y,z)<cutoff2){   region[j][l]=1; ifound[l]=1; break; }  
                }
             } 
             if(ifound[l]!=1) region[j][l]=2;

/* Applying bufflimit to ignore small  fluctuations     */ 

             if(j>0){
               if(region[j][l]!= region[j-1][l]){
                 if(bufftime[l]<bufflimit && region[j][l]==regionprev[l]){
                   for(i=0;i<bufftime[l];i++){region[j-i][l]=regionprev[l];}
                 }
                 else{
                     bufftime[l]=0;
                     regionprev[l]=region[j-1][l];
                 }

               }
             }
             bufftime[l]++;
          }
       }
 }

printf("REGION CALCULATION FINISHED");


/* Calculating dipoles */
//---------------------------------------------------------------------------------------
 for(j=0;j<nframes;j=j+nskip){
    for(k=natom*nmol;k<ntotal;k=k+3){
       for(l=k+1;l<k+3;l++){ 

          dpx1=x[j][k]-x[j][l];
          dpy1=y[j][k]-y[j][l];
          dpz1=z[j][k]-z[j][l];

          dpx1 = dpx1 - box[0]*( (int)(2.0*dpx1/box[0]) - (int)(dpx1/box[0]) );
          dpy1 = dpy1 - box[1]*( (int)(2.0*dpy1/box[1]) - (int)(dpy1/box[1]) );
          dpz1 = dpz1 - box[2]*( (int)(2.0*dpz1/box[2]) - (int)(dpz1/box[2]) );

          for(i=j;i<nframes && i<j+tstep;i++){

             dpx2=x[i][k]-x[i][l];
             dpy2=y[i][k]-y[i][l];
             dpz2=z[i][k]-z[i][l];
          
             dpx2 = dpx2 - box[0]*( (int)(2.0*dpx2/box[0]) - (int)(dpx2/box[0]) );
             dpy2 = dpy2 - box[1]*( (int)(2.0*dpy2/box[1]) - (int)(dpy2/box[1]) );
             dpz2 = dpz2 - box[2]*( (int)(2.0*dpz2/box[2]) - (int)(dpz2/box[2]) );


             xx = (dpx1*dpx2+dpy1*dpy2+dpz1*dpz2)/(sqrt(dpx1*dpx1+dpy1*dpy1+dpz1*dpz1)*sqrt(dpx2*dpx2+dpy2*dpy2+dpz2*dpz2));
    	     r=region[j][l];
   
   	     if(r==0){	
               dp1[i-j] += xx;
               dplg1[i-j] += 0.5*(3.0 * xx*xx - 1.0);
                count1[i-j]++;
   	     }
   	     if(r==1){	
               dp2[i-j] += xx;
               dplg2[i-j] += 0.5*(3.0 * xx*xx - 1.0);
               count2[i-j]++;
   	     }
   	     if(r==2){	
               dp3[i-j] += xx;
               dplg3[i-j] += 0.5*(3.0 * xx*xx - 1.0);
               count3[i-j]++;
   	     }
   	     if(r==3){	
               dp4[i-j] += xx;
               dplg4[i-j] += 0.5*(3.0 * xx*xx - 1.0);
               count4[i-j]++;
   	     }
          }
       }
    }
 }



printf("DIPOLE CALCULATION FINISHED \n");


/* printing the result */
//-------------------------------------------------------------------------------------------
for(i=0;i<tstep;i++)   fprintf(wptr1,"%lf \t %lf \t %lf \n",i*tdiff,(dp1[i]/count1[i]),(dplg1[i]/count1[i]));
for(i=0;i<tstep;i++)   fprintf(wptr2,"%lf \t %lf \t %lf \n",i*tdiff,(dp2[i]/count2[i]),(dplg2[i]/count2[i]));
for(i=0;i<tstep;i++)   fprintf(wptr3,"%lf \t %lf \t %lf \n",i*tdiff,(dp3[i]/count3[i]),(dplg3[i]/count3[i]));
for(i=0;i<tstep;i++)   fprintf(wptr5,"%lf \t %lf \t %lf \n",i*tdiff,(dp4[i]/count4[i]),(dplg4[i]/count4[i]));


   for(i=0;i<nframes;i++){
      for(j=natom*nmol;j<ntotal;j=j+3){
         regiontot[region[i][j]]++;
      }
   }

fprintf(wptr4,"regionOxygenmiddle is %lf regionOxygenside is %lf regionCarbon is %lf regionBulk is %lf \n ",regiontot[3]/(nframes),regiontot[0]/(nframes),regiontot[1]/(nframes),regiontot[2]/(nframes));




fclose(rptr);
fclose(wptr1);
fclose(wptr2);
fclose(wptr3);
fclose(wptr4);
fclose(wptr5);
return 0;

}




