#include "pagetable.h"
#include "cstring"
#include <cmath>

PageTable::PageTable(int page_size)
{
    _page_size = page_size;
    //_frames[i] holds whether frame i is full or not.
    //if _frames[i] == -1, frame i is empty.
    //else, frame i is full. 
    _frames = new int[67108864/_page_size];
    //initialize all frames to empty
    for(int i=0; i < 67108864/_page_size; i++)
    {
        _frames[i] = -1;
    }

}

PageTable::~PageTable()
{
}

void PageTable::addEntry(uint32_t pid, int page_number)
{
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    
    // Find free frame
    int frame = -1;
    int i = 0;
    while(frame == -1)
    {
        if(_frames[i] == -1)
        {
            frame = i;
        }
        i++;
    }

    _frames[frame] = 1;
    _table[entry] = frame;
}

int PageTable::getPhysicalAddress(uint32_t pid, int virtual_address)
{
    // Convert virtual address to page_number and page_offset
    // TODO: implement this!
    int page_number = virtual_address / _page_size;
    int page_offset = virtual_address % _page_size;


    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    
    // If entry exists, look up frame number and convert virtual to physical address
    int address = -1;
    if (_table.count(entry) > 0)
    {
        address = _table[entry];
        address = address << int(log2(_page_size));
        address += page_offset;
    }

    return address;
}

void PageTable::print()
{
    std::map<std::string, int>::iterator it;

    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    for (it = _table.begin(); it != _table.end(); it++)
    {
        // TODO: print all pages
        std::string pid = it->first;
        printf("  %6s %12s%d \n", it->first.c_str(), "|", it->second);
    }
}

void PageTable::removeEntry(uint32_t pid, int page_number)
{
    std::string key = std::to_string(pid) + "|" + std::to_string(page_number);
    int frame = _table[key];
    _frames[frame] = -1;
    _table.erase(key);
}

void PageTable::deleteEntry(uint32_t pid)
{
     // Combination of pid and page number act as the key to look up frame number
    //std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    
    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        std::string locate = it->first;
        int test = stoi(locate);
        if(test == (int)pid)
        {
           _table.erase(locate); 
        }
    }
    
}



void PageTable::deleteTable()
{
    free(_frames);
}