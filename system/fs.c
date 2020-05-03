#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#ifdef FS

#include <fs.h>

static struct fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
struct filetable oft[NUM_FD]; // opens the file table
int next_open_fd = 0;


#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock);

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEBLOCKS - 2) {
    printf("No indirect block support\n");
    return SYSERR;
  }

  diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

  return diskblock;
}

/* read in an inode and fill in the pointer */
int fs_get_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;
  int inode_off;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_get_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(struct inode);

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  printf("inn*sizeof(struct inode): %d\n", inode_off);
  */

  bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(in, &block_cache[inode_off], sizeof(struct inode));

  return OK;

}

/* write inode indicated by pointer to device */
int fs_put_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_put_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  */

  bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
  memcpy(&block_cache[(inn*sizeof(struct inode))], in, sizeof(struct inode));
  bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}
     
/* create file system on device; write file system block and block bitmask to
 * device */
int fs_mkfs(int dev, int num_inodes) {
  int i;
  
  if (dev == 0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  }
  else {
    printf("Unsupported device\n");
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  }
  else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) {i++;}
  fsd.freemaskbytes = i / 8; 
  
  if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *)SYSERR) {
    printf("fs_mkfs memget failed.\n");
    return SYSERR;
  }
  
  /* zero the free mask */
  for(i=0;i<fsd.freemaskbytes;i++) {
    fsd.freemask[i] = '\0';
  }
  
  fsd.inodes_used = 0;
  
  /* write the fsystem block to SB_BLK, mark block used */
  fs_setmaskbit(SB_BLK);
  bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));
  
  /* write the free block bitmask in BM_BLK, mark block used */
  fs_setmaskbit(BM_BLK);
  bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  return 1;
}

/* print information related to inodes*/
void fs_print_fsd(void) {

  printf("fsd.ninodes: %d\n", fsd.ninodes);
  printf("sizeof(struct inode): %d\n", sizeof(struct inode));
  printf("INODES_PER_BLOCK: %d\n", INODES_PER_BLOCK);
  printf("NUM_INODE_BLOCKS: %d\n", NUM_INODE_BLOCKS);
}

/* specify the block number to be set in the mask */
int fs_setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

/* specify the block number to be read in the mask */
int fs_getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
  return OK;

}

/* specify the block number to be unset in the mask */
int fs_clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
   block is indicated in the high-order bit.  Shift the byte by j
   positions to make the match in bit7 (the 8th bit) and then shift
   that value 7 times to the low-order bit to print.  Yes, it could be
   the other way...  */
void fs_printfreemask(void) { // print block bitmask
  int i,j;

  for (i=0; i < fsd.freemaskbytes; i++) {
    for (j=0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}


int fs_open(char *filename, int flags)
{
    struct filetable *f_desc; 
    struct directory *r_dir; 
    struct dirent *ent; 
    struct inode *i;
    int j;

    if (filename == NULL)
    {
        fprintf(stderr, "file name is null\n");
        return SYSERR;
    }
    if (strlen(filename) == 0)
    {
        fprintf(stderr, "file name is correct\n");
        return SYSERR;
    }
    r_dir = &(fsd.root_dir);


    for (int i = 0; i < FILENAMELEN; i++)
    {
        // printf("checkpoint 3\n");
        f_desc = &(oft[i]);
        //printf("\nchecking state: %d and name: %s\n", f_desc->state, f_desc->de->name);
        // printf("\nchecking the fileName arg :%s\n\n", filename);
        if (strncmp(ent->name, f_desc->de->name, FILENAMELEN + 1) == 0)
        {
            // printf("fileName found in OFT\n\n");
            if (f_desc->state == 1)
            {
                printf("\n\nFile already open \n");
                return SYSERR;
            }
        }
    }

    for (j = 0; j < r_dir->numentries; j++) 
    {
        ent = &(r_dir->entry[j]); 
        if (!strcmp(ent->name, filename))
        {
            i = (struct inode *)getmem(sizeof(struct inode));
            if (fs_get_inode_by_num(0, ent->inode_num, i) == SYSERR)
            {
                fprintf(stderr, "could not get inode for %s\n", filename);
                return SYSERR;
            }
            f_desc = &(oft[next_open_fd]); 
            next_open_fd++; 
            f_desc->in = *i; 
            f_desc->de = ent; 
            f_desc->fileptr = 0; 
            f_desc->state = flags; 
            break;
        }
    }
    if (ent == NULL) 
    {
        return SYSERR;
    }
    return (next_open_fd - 1);
}

int fs_close(int fd)
{
    struct filetable *f_desc; 
    struct inode *i;
    if (fd < 0 || fd >= next_open_fd)
    {
        fprintf(stderr, "Bad file descriptor \n");
        return SYSERR;
    }
    f_desc = &(oft[fd]); 
    i = &(f_desc->in); 
    fs_put_inode_by_num(0, i->id, i);
    f_desc->fileptr = 0; 
    freemem(&(f_desc->in), sizeof(struct inode)); 
    f_desc->state = FSTATE_CLOSED; 
    return OK;
}

int fs_create(char *filename, int mode)
{
    struct filetable *f_desc; 
    struct dirent *ent; 
    struct inode *i;
    struct directory *root = &(fsd.root_dir); 
    int j, fd, nIn;
    // check if file already present
    fd = fs_open(filename, O_RDWR);
    if (fd != SYSERR)
    {
        fprintf(stderr, "file will open in read/write mode \n");
        return fd;
    }

    nIn = fsd.inodes_used;
    fsd.inodes_used++;
    i = (struct inode *)getmem(sizeof(struct inode));
    fs_get_inode_by_num(0, nIn, i);
    i->id = nIn;
    i->type = INODE_TYPE_FILE;
    i->device = 0;
    i->size = 0;

    fs_put_inode_by_num(0, nIn, i);
    ent = &(root->entry[root->numentries]); 
    ent->inode_num = nIn; 
    strcpy(ent->name, filename); 
    (root->numentries)++;

    if (next_open_fd >= NUM_FD)
    {
        fprintf(stderr, " max number of files have been opened\n");
        return SYSERR;
    }
    f_desc = &(oft[next_open_fd]); 
    f_desc->fileptr = 0; 
    f_desc->state = mode; 
    f_desc->de = &ent;  
    f_desc->in = *i; 
    next_open_fd++;

    return (next_open_fd - 1);
}

int fs_seek(int fd, int offset)
{
    struct inode *i;
    struct filetable *f_desc; 

    if (fd >= next_open_fd)
    {
        fprintf(stderr, "Bad file descriptor \n");
        return SYSERR;
    }

    f_desc = &(oft[fd]);
    i = &(f_desc->in); 

    if (offset > i->size)
    {
        fprintf(stderr, "error on seeking beyond End of file\n");
        return SYSERR;
    }
    f_desc->fileptr += offset;
    return OK;
}

int fs_read(int fd, void *buf, int nbytes)
{
    int bk = 0; 
    int bk_offset = 0; 
    int b_read = 0; 
    int remaining; 
    struct inode *i;
    struct filetable *f_desc; 
    char *curr; 

    if (fd < 0 || fd >= next_open_fd)
    {
        fprintf(stderr, "Bad file descriptor \n");
        return SYSERR;
    }

    f_desc = &(oft[fd]);
    i = &(f_desc->in);
    curr = buf;
    bk = f_desc->fileptr / MDEV_BLOCK_SIZE; 
    bk_offset = f_desc->fileptr % MDEV_BLOCK_SIZE; 
    if (bk_offset > 0) 
    { // extra space has to be copied
        b_read = (MDEV_BLOCK_SIZE - bk_offset + 1); 
        bs_bread(0, i->blocks[bk], bk_offset, block_cache, remaining);
        memcpy(curr, block_cache, remaining);
    }

    while (b_read < nbytes)
    {
        bk = f_desc->fileptr / MDEV_BLOCK_SIZE;
        if ((nbytes - b_read) >= MDEV_BLOCK_SIZE)
        { // read remaining
            remaining = MDEV_BLOCK_SIZE;
        }
        else
        {
            remaining = (nbytes - b_read);
        }
        bs_bread(0, i->blocks[bk], 0, block_cache, remaining);
        memcpy(curr, block_cache, remaining);
        curr += remaining;

        b_read += remaining;
        f_desc->fileptr += remaining;
    }
    return b_read;
}

int fs_write(int fd, void *buf, int nbytes)
{
    struct filetable *f_desc; 
    struct inode *i;
    char *curr; 
    int j, ndbs, data_block;
    int bk; 
    int bk_offset; 
    int nWritten = 0, remaining = 0;

    if (fd < 0 || fd >= next_open_fd)
    {
        fprintf(stderr, "Bad file descriptor \n");
        return SYSERR;
    }

    f_desc = &oft[fd];
    if (f_desc->state == O_RDONLY)
    {
        fprintf(stderr, "cannot open file in read only mode\n");
        return SYSERR;
    }

    curr = buf;
    i = &(f_desc->in);
    bk = f_desc->fileptr / MDEV_BLOCK_SIZE;
    bk_offset = f_desc->fileptr % MDEV_BLOCK_SIZE;
    if (bk_offset > 0)
    {
        memcpy(block_cache, curr, (MDEV_BLOCK_SIZE - bk_offset + 1));
        bs_bwrite(0, i->blocks[bk], 0, (void *)block_cache, (MDEV_BLOCK_SIZE - bk_offset + 1));
        nWritten += (MDEV_BLOCK_SIZE - bk_offset + 1);
        i->size += nWritten;
        f_desc->fileptr += nWritten;
        curr += nWritten;
    }
    while (nWritten < nbytes)
    {
        bk = f_desc->fileptr / MDEV_BLOCK_SIZE;
        data_block = -1;
        for (j = 15; j <= 512; j++)
        {
            if (!fs_getmaskbit(j))
            {
                data_block = j;
                break;
            }
        }
        //data_block = get_next_free_db();
        fs_setmaskbit(data_block);
        i->blocks[bk] = data_block;
        if ((nbytes - nWritten) > MDEV_BLOCK_SIZE)
        {
            remaining = MDEV_BLOCK_SIZE;
        }
        else
        {
            remaining = (nbytes - nWritten);
        }

        memcpy(block_cache, curr, remaining);
        bs_bwrite(0, i->blocks[bk], 0, (void *)block_cache, remaining);
        f_desc->fileptr += remaining;
        nWritten += remaining;
        i->size += remaining;
        curr += remaining;
    }
    return nWritten;
}

int fs_link(char *src_filename, char* dst_filename) {
  int i;
  struct inode in; 
	struct filetable *f_desc;

      if (src_filename == NULL)
    {
        fprintf(stderr, "Incorrect file name\n");
        return SYSERR;
    }
    if (strlen(src_filename) == 0)
    {
        fprintf(stderr, "Incorrect file name\n");
        return SYSERR;
    }

  for(i = 0; i < fsd.root_dir.numentries; i++) {
	   if (strncmp(src_filename, fsd.root_dir.entry[i].name, FILENAMELEN+1) == 0) {
         f_desc->de = &(fsd.root_dir.entry[fsd.root_dir.numentries++]);
         (f_desc->de)->inode_num = fsd.root_dir.entry[i].inode_num;         
				strcpy((f_desc->de)->name, dst_filename);
        f_desc->state = FSTATE_CLOSED;
        f_desc->flag = O_RDWR;
         fs_get_inode_by_num(0, fsd.root_dir.entry[i].inode_num, &in);  
	   	  in.nlink = in.nlink+1;
         fs_put_inode_by_num(dev0, fsd.root_dir.entry[i].inode_num, &in);
         return OK;
	   }
       }
  return SYSERR;
}

int fs_unlink(char *filename) {
  int i,k;
  struct inode in;

   // printf("\n\n ckecking fs unlink  \n");
    if (filename == NULL)
    {
        fprintf(stderr, "Incorrect file name: File Name Null\n");
        return SYSERR;
    }
    if (strlen(filename) == 0)
    {
        fprintf(stderr, "Incorrect file name\n");
        return SYSERR;
    }

  for(i = 0; i < fsd.root_dir.numentries; i++) {
    if (strncmp(filename, fsd.root_dir.entry[i].name, FILENAMELEN+1) == 0) {
      fs_get_inode_by_num(0, fsd.root_dir.entry[i].inode_num, &in);
       strcpy(fsd.root_dir.entry[i].name, "");
      fsd.root_dir.numentries = fsd.root_dir.numentries-1;
      if(in.nlink > 1) {
        in.nlink = --in.nlink;
        fs_put_inode_by_num(dev0, fsd.root_dir.entry[i].inode_num, &in);
        return OK;
      }
      else if (in.nlink == 1) {
        //If the nlinks of the inode is just 1, then delete the respective data blocks as well.
        
        for(  k = 0;k<INODEBLOCKS;k++){
            fs_clearmaskbit(k);
        }
        fs_put_inode_by_num(dev0, fsd.root_dir.entry[i].inode_num, &in);
        return OK;
      }
    
    }
  }
  
  return SYSERR;
}

#endif /* FS */