/*
    File: page_table.C

    Author: Shalini Priya Ashok Kumar
			Masters of Science in Computer Science
            Department of Computer Science
            Texas A&M University
    Date  : 05/15/2015

    Description: Basic Paging.

*/
		
/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "page_table.H"
#include "console.H"

PageTable     *PageTable::current_page_table; /* pointer to currently loaded page table object */
unsigned int   PageTable::paging_enabled;     /* is paging turned on (i.e. are addresses logical)? */
FramePool     *PageTable::kernel_mem_pool;    /* Frame pool for the kernel memory */
FramePool     *PageTable::process_mem_pool;   /* Frame pool for the process memory */
unsigned long  PageTable::shared_size;

/* Set the global parameters for the paging subsystem. */
void PageTable::init_paging(FramePool * _kernel_mem_pool,
                          FramePool * _process_mem_pool,
                          const unsigned long _shared_size) {

	kernel_mem_pool = _kernel_mem_pool;
	process_mem_pool = _process_mem_pool;
	shared_size = _shared_size;
}

  /* Initializes a page table with a given location for the directory and the
     page table proper.
     NOTE: The PageTable object still needs to be stored somewhere! Probably it is best
           to have it on the stack, as there is no memory manager yet...
     NOTE2: It may also be simpler to create the first page table *before* paging
           has been enabled.
  */
PageTable::PageTable() {

	unsigned long *page_table;
	unsigned long address=0;
	int i;
	page_directory = (unsigned long*)(kernel_mem_pool->get_frame()*PAGE_SIZE);
	page_table = (unsigned long*)(kernel_mem_pool->get_frame()*PAGE_SIZE); 
	//set up page directory
	page_directory[0] = (unsigned long)page_table|3;
	
	for (i=1;i<ENTRIES_PER_PAGE;i++) {
		page_directory[i]=0|2;
	}
	
	//map first 4MB, first page table for kernel space
	for (int i=0;i<ENTRIES_PER_PAGE;i++) {
		page_table[i]=address|3;
		address = address+PAGE_SIZE;
	}
}

  /* Makes the given page table the current table. This must be done once during
     system startup and whenever the address space is switched (e.g. during
     process switching). */
void PageTable::load() {
	current_page_table = this;
	write_cr3((unsigned long)this->page_directory);
}
	
  /* Enable paging on the CPU. Typically, a CPU start with paging disabled, and
     memory is accessed by addressing physical memory directly. After paging is
     enabled, memory is addressed logically. */
void PageTable::enable_paging() {

	write_cr0(read_cr0() | 0x80000000);
}

  /* The page fault handler. */
void PageTable::handle_fault(REGS * _r) {

	unsigned long fault_address = read_cr2();
	unsigned long *page_directory_address = (unsigned long *)read_cr3();

	unsigned long *page_table_address, frame, address;
	int page_directory_entry, page_table_entry;
	
	page_directory_entry = fault_address >> 22; 
	page_table_entry = fault_address >> 12 & 0x03FF;

	//if page_table is present, get the page
	if (page_directory_address[page_directory_entry] & 1) { 
		page_table_address = (unsigned long *)(page_directory_address[page_directory_entry]&0xFFFFF000);
		frame = (unsigned long)(process_mem_pool->get_frame());
		address = (unsigned long)(frame*PAGE_SIZE);
		page_table_address[page_table_entry] = (address|7);
	}
	// page table not present, load page table
	else {
		frame = (unsigned long)(kernel_mem_pool->get_frame());
		if(frame==0)
			 Console::puts("PANIC\n");
		address = (unsigned long)(frame*PAGE_SIZE);
		//fill page directory entry
		page_directory_address[page_directory_entry] = (address)|3;
		page_table_address = (unsigned long *)(page_directory_address[page_directory_entry]&0xFFFFF000);

		//fill page table	
		for(int i=0;i<ENTRIES_PER_PAGE;i++) {
			frame = (unsigned long)(process_mem_pool->get_frame());
			page_table_address[i] = (unsigned long)(frame*PAGE_SIZE);
			page_table_address[i] = page_table_address[i]|3;
		}
	}
}
