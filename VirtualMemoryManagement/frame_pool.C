		
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
	/*	Console::putui((int)bitmap);
		Console::puts("Bitmap\n");*/

		bitmap[0]=1;
		for (i=1;i<nframes;i++) {
			bitmap[i]=0;
		}	
		/*for (i=0;i<2;i++) {
			Console::putui(bitmap[i]);
		}*/	
	}
	//bitmap for process frame pool
	else {
		//Console::puts("info frame no");
		//Console::putui(info_frame_no);
		//Console::puts("frame_size");
		//Console::putui(FRAME_SIZE);
		bitmap = (unsigned long *)((info_frame_no)*FRAME_SIZE);
		Console::putui((int)bitmap);
		Console::puts("Bitmap\n");
		
		for (i=1;i<nframes;i++) {
			bitmap[i]=0;
		}	
	}
}


unsigned long FramePool::get_frame() {

	int i,j;
/*	if(info_frame_no==0){	
	Console::puts("bitmap\n");
		for (i=0;i<8;i++) {
			Console::putui(bitmap[i]);
		}	
	}*/
	for (i=0;i<nframes;i++) {
		if (bitmap[i] == 0) {
			bitmap[i] = 1;	
			if(info_frame_no==0){	
				Console::puts("returning frame :");
				Console::putui(i);
				Console::puts("\n");	
			}
			return i+base_frame_no;
		}
	}
	Console::puts("returning frame 0\n");
	return 0;
}

void FramePool::mark_inaccessible(unsigned long _base_frame_no,
														 unsigned long _nframes) {

	int i,j;
	for (i=_base_frame_no,j=0;j<_nframes;i++,j++) {
		bitmap[i]=-1;
	}
}

void FramePool::release_frame(unsigned long _frame_no) {

	if(bitmap[_frame_no]==1)
		bitmap[_frame_no]=0;	
}
