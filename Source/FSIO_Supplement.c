/* 
 * File:   FSIO_Supplement.c
 * Author: Jonathan
 *
 * Created on June 26, 2012, 4:31 PM
 */

#include "FSIO_Supplement.h"
//#include "MDD File System/FSIO.h"

/* *****************************************************************************
 * Placed by Jonathan Bruce. 
 * Taken from FSIO.c at lines 147-167
 * ****************************************************************************/
extern DWORD       gLastFATSectorRead;    // Global variable indicating which FAT sector was read last
extern BYTE        gNeedFATWrite;              // Global variable indicating that there is information that needs to be written to the FAT
extern FSFILE  *   gBufferOwner;                // Global variable indicating which file is using the data buffer
extern DWORD       gLastDataSectorRead;   // Global variable indicating which data sector was read last
extern BYTE        gNeedDataWrite;             // Global variable indicating that there is information that needs to be written to the data section
extern BYTE        nextClusterIsLast;          // Global variable indicating that the entries in a directory align with a cluster boundary

extern BYTE    gBufferZeroed;      // Global variable indicating that the data buffer contains all zeros

extern DWORD   FatRootDirClusterValue;     // Global variable containing the cluster number of the root dir (0 for FAT12/16)

extern BYTE    FSerrno;                    // Global error variable.  Set to one of many error codes after each function call.

extern DWORD   TempClusterCalc;            // Global variable used to store the calculated value of the cluster of a specified sector.
extern BYTE    dirCleared;                 // Global variable used by the "recursive" FSrmdir function to indicate that all subdirectories and files have been deleted from the target directory.
extern BYTE    recache;            // Global variable used by the "recursive" FSrmdir function to indicate that additional cache reads are needed.
extern FSFILE  tempCWDobj;                 // Global variable used to preserve the current working directory information.
extern FSFILE  gFileTemp;                  // Global variable used for file operations.

extern FSFILE   cwd;               // Global current working directory
extern FSFILE * cwdptr;     // Pointer to the current working directory
/* *****************************************************************************
 * Placed by Jonathan Bruce.
 * Taken from FSIO.c at lines 147-167
 * ****************************************************************************/

/* *****************************************************************************
 * Placed by Jonathan Bruce.
 * Taken from FSIO.c at lines 291
 * ****************************************************************************/
extern DWORD ReadFAT (DISK *, DWORD);
/* *****************************************************************************
 * Placed by Jonathan Bruce.
 * Taken from FSIO.c at lines 291
 * ****************************************************************************/

/* *****************************************************************************
 * Placed by Jonathan Bruce.
 * Taken from FSIO.c at lines 294
 * ****************************************************************************/
extern DWORD Cluster2Sector(DISK *, DWORD);
/* *****************************************************************************
 * Placed by Jonathan Bruce.
 * Taken from FSIO.c at lines 294
 * ****************************************************************************/

/* *****************************************************************************
 * Placed by Jonathan Bruce.
 * Taken from FSIO.c at lines 325
 * ****************************************************************************/
extern DWORD FATfindEmptyCluster(FSFILE *);
/* *****************************************************************************
 * Placed by Jonathan Bruce.
 * Taken from FSIO.c at lines 325
 * ****************************************************************************/

/* *****************************************************************************
 * Placed by Jonathan Bruce.
 * Taken from FSIO.c at lines 331
 * ****************************************************************************/
extern DWORD WriteFAT (DISK *, DWORD, DWORD, BYTE);
/* *****************************************************************************
 * Placed by Jonathan Bruce.
 * Taken from FSIO.c at lines 331
 * ****************************************************************************/

DWORD get_First_Sector(FSFILE *stream)
{
	DWORD Sector;
	Sector=Cluster2Sector(stream->dsk,stream->cluster)+stream->sec;
	//printf("first Cluster: %d\r\nCurrent Sector: %d",(int)Sector,(int)stream->sec);
	return Sector;
}



BYTE FAT_print_cluster_chain (DWORD cluster, DISK * dsk)
{
    DWORD     c,c2,ClusterFailValue;
    enum    _status {Good, Fail, Exit}status;

    status = Good;

    /* Settings based on FAT type */
            ClusterFailValue = CLUSTER_FAIL_FAT16;
            c2 =  LAST_CLUSTER_FAT16;
	printf("Starting to print cluster chain\r\n");
	printf("%lu\t",cluster);
    // Make sure there is actually a cluster assigned
    if(cluster == 0 || cluster == 1)  // Cluster assigned can't be "0" and "1"
    {
        status = Exit;
    }
    else
    {
        while(status == Good)
        {
            // Get the FAT entry
            if((c = ReadFAT( dsk, cluster)) == ClusterFailValue)
                status = Fail;
            else
            {
                if(c == 0 || c == 1)  // Cluster assigned can't be "0" and "1"
                {
                    status = Exit;
                }
                else
                {
                    // compare against max value of a cluster in FATxx
                    // look for the last cluster in the chain
                    if ( c >= c2)
                        status = Exit;

                    // Now erase this FAT entry
                    //if(WriteFAT(dsk, cluster, CLUSTER_EMPTY, FALSE) == ClusterFailValue)
                      //  status = Fail;
					printf("%lu\t",c);
					while(!U2STAbits.TRMT);
                    // now update what the current cluster is
                    cluster = c;
                }
            }
        }// while status
    }// cluster == 0
	printf("\r\n");
    //WriteFAT (dsk, 0, 0, TRUE);

    if(status == Exit)
        return(TRUE);
    else
        return(FALSE);
} // Erase cluster



BYTE FILEallocate_multiple_clusters( FSFILE *fo, DWORD num_sectors)
{

	//printf("hi starting the process\r\n");

	static int first=0;
    DISK *      dsk;
    DWORD c,curcls, num_clusters,temp;
    dsk = fo->dsk;

    //need the numbers of sectors per cluster as they change in larger cards
    curcls=dsk->SecPerClus;
    //printf("Sectors Per Cluster: %lu\r\n",curcls);

    //number of clusters needed as that is the smalles size possible to allocate
    num_clusters=num_sectors/curcls;
    //printf("num_clusters: %lu\r\n",num_clusters);
    //c = fo->ccls;
	//printf("Current Cluster: %lu\r\nFirstCluster: %lu\r\n",c,fo->cluster);
	#ifdef __DEBUG
		//FAT_print_cluster_chain(fo->cluster,dsk);
	#endif
    // find the next empty cluster

    //this finds the first free sector, eventually it needs to find the largest contiguos space

   // if (c == 0)      // "0" is just an indication as Disk full in the fn "FATfindEmptyCluster()"
       // return CE_DISK_FULL;
	//else
	//{


	    //last cluster is then overwritten with last cluster_tag

        if(first!=0)
        {
	        c=fo->ccls+1;
			#ifdef __DEBUG
				//printf("Sector Found: %lu\r\n",c);
			#endif

	    DWORD count_clust;
        //loops through the required number of clusters linking them together
        for(count_clust=c;count_clust<(c+num_clusters);count_clust++)
        {
	        //printf("Cluster Allocated: %lu\r\n",count_clust);
	          WriteFAT(dsk,count_clust,count_clust+1,FALSE);
	    }
	    //printf("count_clust: %lu\r\n",count_clust);
	    count_clust--;  //subtract one off to be at last cluster
	    WriteFAT( dsk, count_clust, LAST_CLUSTER_FAT16, FALSE);
        WriteFAT(dsk,0,0,TRUE);
	        WriteFAT( dsk, fo->ccls, c, FALSE);
	        WriteFAT(dsk,0,0,TRUE);
	        fo->ccls = count_clust;
	     	//last_clust=count_clust;
	     	#ifdef __DEBUG
	     		printf("After first time\r\n");
	     	#endif
	     }
		else
		{
		c = FATfindEmptyCluster(fo);
		DWORD count_clust;
        //loops through the required number of clusters linking them together
        for(count_clust=c;count_clust<(c+num_clusters);count_clust++)
        {
	       // printf("Cluster Allocated: %lu\r\n",count_clust);
	          WriteFAT(dsk,count_clust,count_clust+1,FALSE);
	    }
	    //printf("count_clust: %lu\r\n",count_clust);
	    count_clust--;
	            WriteFAT( dsk, count_clust, LAST_CLUSTER_FAT16, FALSE);
        WriteFAT(dsk,0,0,TRUE);
			FILEget_next_cluster(fo, 20);
			curcls=fo->ccls;
			WriteFAT(dsk,curcls,0,FALSE);
			//WriteFAT(dsk,0,0,TRUE);
			fo->cluster=c;
			fo->ccls=count_clust;
			first++;
			//curcls=c;
			//WriteFAT(dsk,curcls,c,FALSE);
			//temp=ReadFAT(dsk,curcls);
			//last_clust=count_clust;
			//printf("Current Cluster: %lu\r\nFirstCluster: %lu\r\nhmm: %lu\r\n",c,fo->cluster,temp);
			#ifdef __DEBUG
				//printf("in first time\r\n");
			#endif
		}
        //curcls=fo->cluster;
        //WriteFAT( dsk, curcls, c, FALSE);
         //printf("file size: %lu\r\n",fo->size);
        fo->size=fo->size+num_sectors*512;
        gNeedFATWrite=TRUE;
        fo->flags.write=TRUE;
        int err;
        //_LATA6=1;
        err=FSfclose(fo);

        //_LATA6=0;
        fo->flags.write=TRUE;
        //printf("FSfclose error: %d\r\n",err);
        WriteFAT(dsk,0,0,TRUE);
        //FAT_print_cluster_chain(fo->cluster,dsk);
       	//fo=FSfopen("WRITE.TXT","r");

        //FAT_print_cluster_chain(fo->cluster,dsk);
        //FAT_print_cluster_chain(c,dsk);
		//#ifdef __DEBUG
		//	printf("file size: %lu\r\n",fo->size);
		//#endif

    //}
    return CE_GOOD;
    // link current cluster to the new one
    curcls = fo->ccls;

    WriteFAT( dsk, curcls, c, FALSE);

    // update the FILE structure
    fo->ccls = c;
	//fo->size=size;
    // IF this is a dir, we need to erase the cluster
    // If it's a file, we can leave it- the file size
    // will limit the data we see to the data that's been
    // written
        return CE_GOOD;

} // allocate new cluster