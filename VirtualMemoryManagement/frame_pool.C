/*
    File: page_table.C

    Author: Shalini Priya Ashok Kumar
			Masters of Science in Computer Science
            Department of Computer Science
            Texas A&M University
    Date  : 05/15/2015

    Description: Management of the Free-Frame Pool.

*/
/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define KERNEL_START_FRAME  512
#define PROCESS_START_FRAME 1024
#define KERNEL_START_ADDRESS 0x200000
#define PROCESS_START_ADDRESS 0x400000

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "frame_pool.H"
#include "console.H"

   /* Initializes the data structures needed for the management of this
      frame pool. This function must be called before the paging system
      is initialized.
      _base_frame_no is the frame number at the start of the physical memory
      region that this frame pool manages.
      _nframes is the number of frames in the physical memory region that this
      frame pool manages.
      e.g. If _base_frame_no is 16 and _nframes is 4, this frame pool manages
      physical frames numbered 16, 17, 18 and 19
      _info_frame_no is the frame number (within the directly mapped region) of
      the frame that should be used to store the management information of the
      frame pool. However, if _info_frame_no is 0, the frame pool is free to
      choose any frame from the pool to store management information.
      */
FramePool::FramePool(unsigned long _base_frame_no,
             unsigned long _nframes,
             unsigned long _info_frame_no) {

	int i;

	base_frame_no = _base_frame_no;
	nframes = _nframes;
	info_frame_no = _info_frame_no;
	
	//bitmap for kernel frame pool
	if(info_frame_no==0) {
		bitmap = (unsigned long *)(KERNEL_START_ADDRESS);

		bitmap[0]=1;
		for (i=1;i<nframes;i++) {
			bitmap[i]=0;
		}	
	}
	//bitmap for process frame pool
	else {
		bitmap = (unsigned long *)((info_frame_no)*FRAME_SIZE);
		
		for (i=1;i<nframes;i++) {
			bitmap[i]=0;
		}	
	}
}


   /* Allocates a frame from the frame pool. If successful, returns the frame
    * number of the frame. If fails, returns 0. */
unsigned long FramePool::get_frame() {

	int i,j;
	for (i=0;i<nframes;i++) {
		if (bitmap[i] == 0) {
			bitmap[i] = 1;	
			return i+base_frame_no;
		}
	}
	return 0;
}

   /* Mark the area of physical memory as inaccessible. The arguments have the
    * same semanticas as in the constructor.
    */
void FramePool::mark_inaccessible(unsigned long _base_frame_no,
														 unsigned long _nframes) {

	int i,j;
	for (i=_base_frame_no,j=0;j<_nframes;i++,j++) {
		bitmap[i]=-1;
	}
}

   /* Releases frame back to the given frame pool.
      The frame is identified by the frame number. */
void FramePool::release_frame(unsigned long _frame_no) {

	if(bitmap[_frame_no]==1)
		bitmap[_frame_no]=0;	
}
