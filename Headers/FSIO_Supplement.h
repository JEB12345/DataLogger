/* 
 * File:   FSIO_Supplement.h
 * Author: Jonathan
 *
 * Created on June 26, 2012, 4:11 PM
 */

#ifndef FSIO_SUPPLEMENT_H
#define	FSIO_SUPPLEMENT_H

#include "MDD File System/FSIO.h"

BYTE FILEallocate_multiple_clusters(FSFILE *fo, DWORD num_clusters);
DWORD get_First_Sector(FSFILE *stream);
BYTE FAT_print_cluster_chain (DWORD cluster, DISK * dsk);

#endif	/* FSIO_SUPPLEMENT_H */

