/* 
 * File:   FSIO_Supplement.h
 * Author: Jonathan
 *
 * Created on June 20, 2012, 12:23 PM
 */

#ifndef FSIO_SUPPLEMENT_H
#define	FSIO_SUPPLEMENT_H

#include "MDD File System/FSIO.h"

void allocate_size(DWORD size,FSFILE *stream,BOOL all);
BYTE FILEallocate_multiple_clusters(FSFILE *fo, DWORD num_clusters);
void do_stuff(FSFILE *stream);
DWORD get_First_Sector(FSFILE *stream);
BYTE FAT_print_cluster_chain (DWORD cluster, DISK * dsk);

#endif	/* FSIO_SUPPLEMENT_H */

