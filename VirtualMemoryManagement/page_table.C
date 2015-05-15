
		
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

PageTable::PageTable() {

	unsigned long *page_table;
	unsigned long address=0;
	int i;
	page_directory = (unsigned long*)(kernel_mem_pool->get_frame()*PAGE_SIZE);
	page_table = (unsigned long*)(kernel_mem_pool->get_frame()*PAGE_SIZE); //try with process mem pool
	page_directory[0] = (unsigned long)page_table|3;
	
	for (i=1;i<ENTRIES_PER_PAGE;i++) {
		page_directory[i]=0|2;
	}
	
	//map first 4MB
	for (int i=0;i<ENTRIES_PER_PAGE;i++) {
		page_table[i]=address|3;
		address = address+PAGE_SIZE;
	}
}

void PageTable::load() {
	current_page_table = this;
	write_cr3((unsigned long)this->page_directory);
}
	
void PageTable::enable_paging() {

	write_cr0(read_cr0() | 0x80000000);
}

void PageTable::handle_fault(REGS * _r) {

	unsigned long fault_address = read_cr2();
	unsigned long *page_directory_address = (unsigned long *)read_cr3();

	unsigned long *page_table_address, physical_address;
	int page_directory_entry, page_table_entry;
	
	page_directory_entry = fault_address >> 22; 
	page_table_entry = fault_address >> 12 & 0x03FF;
	Console::putui(fault_address);
	Console::puts("page fault occurred\n");

	//if page_table is present, get the page, otherwise get the page_table
	if (page_directory_address[page_directory_entry] & 1) { // present page table
		Console::puts("page table present, page not present\n");
		page_table_address = (unsigned long *)(page_directory_address[page_directory_entry]&0xFFFFF000);
		unsigned long frame = (unsigned long)(process_mem_pool->get_frame());
		unsigned long address = (unsigned long)(frame*4096);
		page_table_address[page_table_entry] = (address|7);
	}
	else {// page table not present, load page table
		Console::puts("page table not present\n");
		unsigned long frame = (unsigned long)(kernel_mem_pool->get_frame());
		if(frame==0) Console::puts("PANIC\n");
		unsigned long address = (unsigned long)(frame*4096);
/*		Console::putui(frame);
		Console::puts("frame\n");
		Console::putui(address);
		Console::puts("address\n");*/
		page_directory_address[page_directory_entry] = (address)|3;
		page_table_address = (unsigned long *)(page_directory_address[page_directory_entry]&0xFFFFF000);
		Console::putui(page_directory_entry);
		Console::puts("page directory entry\n");
		Console::putui((int)page_table_address);
		Console::puts("page table Address\n");
		for(int i=0;i<ENTRIES_PER_PAGE;i++) {
			frame = (unsigned long)(process_mem_pool->get_frame());
			page_table_address[i] = (unsigned long)(frame*PAGE_SIZE);
			page_table_address[i] = page_table_address[i]|3;
		}
		//frame = (unsigned long)(process_mem_pool->get_frame());
		//page_table_address[page_table_entry] = (frame*PAGE_SIZE)|7;
	}
}
