#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>

#include "mmu.h"
#include "pagetable.h"

void printStartMessage(int page_size);
std::vector<std::string> splitString(std::string text, char d);
int dataTypeToSize(std::string data_type);
void printElement(uint8_t* memory, int physical_address, int data_type, bool isFloating);


int main(int argc, char **argv)
{
    // Ensure user specified page size as a command line parameter
    if (argc < 2)
    {
        fprintf(stderr, "Error: you must specify the page size\n");
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory'
    uint8_t *memory = new uint8_t[67108864]; // 64 MB (64 * 1024 * 1024)
    //Create mmu
    Mmu *mmu = new Mmu(67108864);
    int current_size = 0;
    //Create page table
    PageTable *pagetable = new PageTable(page_size);

    // Prompt loop
    std::string command;
    std::cout << "> ";
    std::getline (std::cin, command);
    std::vector<std::string> args;
    while (command != "exit") {
        // Handle command
        // TODO: implement this!
        args = splitString(command, ' ');
        if (args[0] == "create")//arg1, arg2 = text_size, globals_size
        {
            //error check args1 and 2
            int argc = args.size();
            if(argc != 3)
            {
                std::cout << "Error: 'create' expects two arguments" << std::endl;
            }
            else
            {   
                //TODO: Check that args1 and 2 are integers
                uint32_t text_size = std::stoi(args[1]);
                uint32_t globals_size = std::stoi(args[2]);
                if(text_size < 2048 || text_size > 16384 || globals_size < 0 || globals_size > 1024)
                {
                    std::cout << "Error: create <text_size> must be 2048-16384 bytes, <data_size> must be 0-1024 bytes" << std::endl;
                }
                else
                {
                    //ERROR CHECK that text_size + globals_size + 65536 + current_size < max_size
                    if(text_size + globals_size + 65536 + current_size > 67108864)
                    {
                        std::cout << "Error: Memory allocation exceeds available system memory" << std::endl;
                    }
                    else
                    {
                        //init new process
                        uint32_t pid = mmu->createProcess();
                        //add new page entries to the page table
                        //get virtual addr of the last variable: do math to figure out how many pages there are

                        Process* proc = mmu->getProcess(pid);
                        //allocate text, globals, stack
                        Variable *text_var = new Variable();
                        text_var->name = "<TEXT>";
                        text_var->virtual_address = 0;
                        text_var->size = text_size;
                        text_var->data_type = 1; //denotes "bytes" datatype
                        proc->variables.push_back(text_var);

                        Variable *globals_var = new Variable();
                        globals_var->name = "<GLOBALS>";
                        globals_var->virtual_address = text_size;
                        globals_var->size = globals_size;
                        globals_var->data_type = 1;
                        proc->variables.push_back(globals_var);

                        Variable *stack_var = new Variable();
                        stack_var->name = "<STACK>";
                        stack_var->virtual_address = text_size + globals_size;
                        stack_var->size = 65536; //always 65536
                        stack_var->data_type = 1;
                        proc->variables.push_back(stack_var);
                       
                        //update the page table
                        int num_pages =  (text_size + globals_size + 65536) / page_size;
                        if((text_size + globals_size + 65536) % page_size != 0)
                        {
                            //mark leftover space in last page as <FREE_SPACE>
                            Variable* free_space = new Variable();
                            free_space->name = "<FREE_SPACE>";
                            free_space->virtual_address = text_size + globals_size + 65536;

                            free_space->size = page_size - ((text_size + globals_size + 65536) % page_size);
                            free_space->data_type = 1;

                            proc->variables.push_back(free_space);
                            num_pages++;
                        }
                        for(int i = 0; i < num_pages; i++)
                        {
                            pagetable->addEntry(pid, i);
                        }

                        //update current_size
                        current_size = current_size + text_size + globals_size + 65536;
                        //print the pid
                        std::cout << pid << std::endl;
                    }   
                }
            }
        }//end create
        if(args[0] == "print")
        {

            //TODO: Error check args
            if(args[1] == "mmu")
            {
                mmu->print();
            }
            else if(args[1] == "page")
            {
                pagetable->print();
            }
            else if(args[1] == "processes")
            {
                mmu->printProcesses();
            }
            else
            {
                int pid = std::stoi(splitString(args[1], ':')[0].c_str());
                std::string var_name = splitString(args[1], ':')[1];
                std::vector<Variable*> found_vars = mmu->getVarByName(pid, var_name);

                int total_elem = 0;
                int physical_address;
                int offset = 0;
                int var_idx = 0;
                int space_found = found_vars[0]->size;
                for(int i = 0; i < found_vars.size(); i++)
                {
                    total_elem += found_vars[i]->size;
                }
                if(total_elem > 4)
                {
                    for(int i = 0; i < 4; i++)
                    {
                        if(i == space_found)
                        {
                            offset = 0;
                            var_idx++;
                            space_found += found_vars[var_idx]->size;
                        }
                        physical_address = pagetable->getPhysicalAddress(pid, found_vars[var_idx]->virtual_address) + offset*found_vars[var_idx]->data_type;
                        printElement(memory, physical_address, found_vars[var_idx]->data_type, found_vars[var_idx]->isFloating);
                        offset++;
                    }
                    std::cout << "... [" << total_elem << " items]" << std::endl;
                }
                else
                {
                    for(int i = 0; i < total_elem; i++)
                    {
                        if(i == space_found)
                        {
                            offset = 0;
                            var_idx++;
                            space_found += found_vars[var_idx]->size;
                        }
                        physical_address = pagetable->getPhysicalAddress(pid, found_vars[var_idx]->virtual_address) + offset*found_vars[var_idx]->data_type;
                        printElement(memory, physical_address, found_vars[var_idx]->data_type, found_vars[var_idx]->isFloating);
                        offset++;                        
                    }
                    std::cout << std::endl;
                }
            }

        }//end print
        if(args[0] == "allocate") //<PID> <var_name> <data_type> <number_of_elements>
        {
            int argc = args.size();
            if(argc != 5)
            {
                std::cout << "Error: 'allocate' expects four arguments" << std::endl;
            }
            else {
                uint32_t pid = stoi(args[1]);
                std::string var_name = args[2];
                int data_t = dataTypeToSize(args[3]);
                int num_elem = stoi(args[4]);
                bool isFloating = false;
                
                if(data_t*num_elem + current_size > 67108864)
                {
                    std::cout << "Not enough space to allocate" << std::endl;
                }
                else
                {
                    current_size += data_t*num_elem;
                    if(args[3].compare(std::string("float")) == 0 || args[3].compare(std::string("double")) == 0)
                    {
                        isFloating = true;
                    }

                    //Use first-fit to allocate within already existing pages
                    std::vector<Variable*> vars = mmu->getProcess(pid)->variables;
                    Variable* curr_var;
                    int remaining = num_elem;
                    int first_vaddr = -1;
                    std::vector<Variable*>::iterator iter = vars.begin();
                    while(iter != vars.end() && remaining > 0)//Filling multiple FREE SPACE gaps still untested
                    {
                        curr_var = *iter;
                        if(curr_var->name.compare(std::string("<FREE_SPACE>")) == 0)
                        {
                            //integer division to determine number of elements that can fit in <FREE_SPACE>
                            int num_fit = curr_var->size/data_t;

                            if(num_fit > 0)
                            {
                                //allocate as many elements as will fit
                                Variable* new_var = new Variable();
                                new_var->name = var_name;
                                new_var->data_type = data_t;
                                new_var->size = std::min(num_fit, remaining);
                                new_var->isFloating = isFloating;
                                new_var->virtual_address = curr_var->virtual_address;
                                remaining = remaining - num_fit;
                                if(first_vaddr == -1)
                                {
                                    first_vaddr = curr_var->virtual_address;
                                }

                                //place variable in variables
                                curr_var->size -= (new_var->size*data_t);
                                if(curr_var->size == 0)
                                {//perfect fit, replace <FREE_SPACE> var w/new_var
                                    *curr_var = *new_var;
                                }
                                else
                                {//insert new_var before <FREE_SPACE>
                                    iter = vars.insert(iter, new_var);
                                    //update <FREE_SPACE> virtual address
                                    curr_var->virtual_address = new_var->virtual_address + (new_var->size*data_t);
                                }
                            }
                        }
                        ++iter;
                    }
                    
                    if(remaining > 0)
                    {
                        //create new page to allocate remaining elements
                        //get highest virtual address
                        int last_addr = vars.back()->virtual_address + (vars.back()->size*vars.back()->data_type);
                        int page_num = (last_addr/page_size);
                        int pages_needed = (remaining*data_t)/page_size;

                        Variable* new_var = new Variable();
                        new_var->name = var_name;
                        new_var->data_type = data_t;
                        new_var->size = remaining*data_t;
                        new_var->virtual_address = last_addr;
                        new_var->isFloating = isFloating;

                        if(first_vaddr == -1)
                        {
                            first_vaddr = new_var->virtual_address;
                        }

                        vars.push_back(new_var);

                        if((remaining*data_t) % page_size != 0)
                        {
                            pages_needed++;

                            //mark leftover space in last page as <FREE_SPACE>
                            Variable* free_space = new Variable();
                            free_space->name = "<FREE_SPACE>";
                            free_space->virtual_address = new_var->virtual_address + (new_var->size*new_var->data_type);
                            free_space->size = page_size - ((remaining*data_t) % page_size);
                            free_space->data_type = 1;
                            vars.push_back(free_space);                    
                        }

                        //add neccesary pages
                        for(int i = 0; i < pages_needed; i++)
                        {
                            pagetable->addEntry(pid, page_num + i);
                        }
                    }


                    //update mmu
                    mmu->getProcess(pid)->variables = vars;
                    std::cout << first_vaddr <<std::endl;
                }
            }
        }//end allocate
        if(args[0] == "set") //<PID> <var_name> <offset> <value_0> <value_1>... <value_N>
        {
            //TODO: error checks
            //Check that enough space exists in found_vars for all values
            int argc = args.size();
            if(argc < 5)
            {
                std::cout << "Error: 'set' expects at least four arguments" << std::endl;
            }
            else {

            
                uint32_t pid = stoi(args[1]);
                std::string var_name = std::string(args[2]);
                int offset = stoi(args[3]);
                int argc = args.size();
                
                std::vector<Variable*> found_vars = mmu->getVarByName(pid, var_name);
                if(found_vars.empty())
                {
                    std::cout << "Error: No space allocated for variable " << var_name << " in process " << pid << std::endl;
                }
                else
                {

                    //find var that holds offset
                    int count = 0;
                    std::vector<Variable*>::iterator iter = found_vars.begin();
                    while(iter != found_vars.end() && count < offset)
                    {
                        count += found_vars.front()->size;
                        if(count >= offset)
                        {
                            iter = found_vars.end();
                        }
                        else
                        {
                            offset -= found_vars.front()->size;
                            iter = found_vars.erase(iter);
                        }
                    }

                    int data_t = found_vars[0]->data_type;
                    bool isFloating = found_vars[0]->isFloating;

                    if(data_t == 1)//chars
                    {
                        std::vector<char> data_v;
                        char* data = new char[argc-4];
                        for (int i = 4; i < argc; i++)
                        {
                            //TODO: error check
                            strcpy(data, args[i].c_str());
                            data_v.push_back(*data);
                        }

                        int virtual_address;
                        int vars_idx = 0;
                        int remaining = data_v.size();
                        int physical_address;

                        Variable* curr_var;
                        int i = 0;
                        while(i < data_v.size())
                        {
                            curr_var = found_vars[vars_idx];
                            for(int j = 0; j + offset < curr_var->size; j++)
                            {
                                if(remaining > 0)
                                {
                                    virtual_address = curr_var->virtual_address;
                                    virtual_address += (offset + j)*curr_var->data_type;
                                    physical_address = pagetable->getPhysicalAddress(pid, virtual_address);
                                    memory[physical_address] = data_v[i];
                                }
                                i++;
                                remaining--;
                            }
                            offset = 0;
                            vars_idx++;
                        }
                        

                    }
                    if(data_t == 2)//shorts
                    {
                        std::vector<short> data_v;
                        short data;
                        short* mem_ptr;
                        for (int i = 4; i < argc; i++)
                        {
                            //TODO: error check
                            data = (short) stoi(args[i]);
                            data_v.push_back(data);
                        }

                        int virtual_address;
                        int vars_idx = 0;
                        int remaining = data_v.size();
                        int physical_address;

                        Variable* curr_var;
                        int i = 0;
                        while(i < data_v.size())
                        {
                            curr_var = found_vars[vars_idx];
                            for(int j = 0; j + offset < curr_var->size; j++)
                            {
                                if(remaining > 0)
                                {
                                    virtual_address = curr_var->virtual_address;
                                    virtual_address += (offset + j)*curr_var->data_type;
                                    physical_address = pagetable->getPhysicalAddress(pid, virtual_address);
                                    mem_ptr = (short*) &memory[physical_address];
                                    *mem_ptr = data_v[i];
                                }
                                i++;
                                remaining--;
                            }
                            offset = 0;
                            vars_idx++;
                        }
                    }
                    if(data_t == 4 && isFloating)//floats
                    {
                        std::vector<float> data_v;
                        float data;
                        float* mem_ptr;
                        for (int i = 4; i < argc; i++)
                        {
                            //TODO: error check
                            data =  std::stof(args[i]);
                            data_v.push_back(data);
                        }

                        int virtual_address;
                        int vars_idx = 0;
                        int remaining = data_v.size();
                        int physical_address;

                        Variable* curr_var;
                        int i = 0;
                        while(i < data_v.size())
                        {
                            curr_var = found_vars[vars_idx];
                            for(int j = 0; j + offset < curr_var->size; j++)
                            {
                                if(remaining > 0)
                                {
                                    virtual_address = curr_var->virtual_address;
                                    virtual_address += (offset + j)*curr_var->data_type;
                                    physical_address = pagetable->getPhysicalAddress(pid, virtual_address);
                                    mem_ptr = (float*) &memory[physical_address];
                                    *mem_ptr = data_v[i];
                                }
                                i++;
                                remaining--;
                            }
                            offset = 0;
                            vars_idx++;
                        }
                    }
                    if(data_t == 4 && !isFloating) //ints
                    {
                        std::vector<int> data_v;
                        int data;
                        int* mem_ptr;
                        for (int i = 4; i < argc; i++)
                        {
                            //TODO: error check
                            data = stoi(args[i]);
                            data_v.push_back(data);
                        }

                        int virtual_address;
                        int vars_idx = 0;
                        int remaining = data_v.size();
                        int physical_address;

                        Variable* curr_var;
                        int i = 0;
                        while(i < data_v.size())
                        {
                            curr_var = found_vars[vars_idx];
                            for(int j = 0; j + offset < curr_var->size; j++)
                            {
                                if(remaining > 0)
                                {
                                    virtual_address = curr_var->virtual_address;
                                    virtual_address += (offset + j)*curr_var->data_type;
                                    physical_address = pagetable->getPhysicalAddress(pid, virtual_address);
                                    mem_ptr = (int*) &memory[physical_address];
                                    *mem_ptr = data_v[i];
                                }
                                i++;
                                remaining--;
                            }
                            offset = 0;
                            vars_idx++;
                        }
                    }
                    if(data_t == 8 && isFloating)//doubles
                    {
                        std::vector<double> data_v;
                        double data;
                        double* mem_ptr;
                        for (int i = 4; i < argc; i++)
                        {
                            //TODO: error check
                            data = stod(args[i]);
                            data_v.push_back(data);
                        }

                        int virtual_address;
                        int vars_idx = 0;
                        int remaining = data_v.size();
                        int physical_address;

                        Variable* curr_var;
                        int i = 0;
                        while(i < data_v.size())
                        {
                            curr_var = found_vars[vars_idx];
                            for(int j = 0; j + offset < curr_var->size; j++)
                            {
                                if(remaining > 0)
                                {
                                    virtual_address = curr_var->virtual_address;
                                    virtual_address += (offset + j)*curr_var->data_type;
                                    physical_address = pagetable->getPhysicalAddress(pid, virtual_address);
                                    mem_ptr = (double*) &memory[physical_address];
                                    *mem_ptr = data_v[i];
                                }
                                i++;
                                remaining--;
                            }
                            offset = 0;
                            vars_idx++;
                        }
                    }
                    if(data_t == 8 && !isFloating)//longs
                    {
                        std::vector<long> data_v;
                        long data;
                        long* mem_ptr;
                        for (int i = 4; i < argc; i++)
                        {
                            //TODO: error check
                            data = (long) stol(args[i]);
                            data_v.push_back(data);
                        }

                        int virtual_address;
                        int vars_idx = 0;
                        int remaining = data_v.size();
                        int physical_address;
                        Variable* curr_var;
                        int i = 0;
                        while(i < data_v.size())
                        {
                            curr_var = found_vars[vars_idx];
                            for(int j = 0; j + offset < curr_var->size; j++)
                            {
                                if(remaining > 0)
                                {
                                    virtual_address = curr_var->virtual_address;
                                    virtual_address += (offset + j)*curr_var->data_type;
                                    physical_address = pagetable->getPhysicalAddress(pid, virtual_address);
                                    mem_ptr = (long*) &memory[physical_address];
                                    *mem_ptr = data_v[i];
                                }
                                i++;
                                remaining--;
                            }
                            offset = 0;
                            vars_idx++;
                        }
                    }
                }
            }
        }//end set
        if(args[0] == "free") //<PID> <var_name>
        {
            int argc = args.size();
            if(argc != 3)
            {
                std::cout << "Error: 'free' expects two arguments" << std::endl;
            }
            else {
                uint32_t pid = stoi(args[1]);
                std::string var_name = std::string(args[2]);
                std::vector<Variable*> vars = mmu->getProcess(pid)->variables;

                //find vars w/name match and mark as <FREE_SPACE>
                for(int i = 0; i < vars.size(); i++)
                {
                    if(vars[i]->name.compare(var_name) == 0)
                    {
                        vars[i]->name = std::string("<FREE_SPACE>");
                        vars[i]->size = vars[i]->size*vars[i]->data_type;
                        vars[i]->data_type = 1;
                        current_size -= vars[i]->size*vars[i]->data_type;
                    }
                }

                std::vector<Variable*>::iterator iter = vars.begin();
                Variable* curr_var;
                Variable* next_var;
                while(iter != vars.end())
                {
                    curr_var = *iter;
                    if(curr_var->name.compare(std::string("<FREE_SPACE>")) == 0)
                    {
                        while(std::next(iter) != vars.end())
                        {
                            next_var = *std::next(iter);
                            if(next_var->name.compare(std::string("<FREE_SPACE>")) == 0)
                            {
                                curr_var->size = (curr_var->size*curr_var->data_type) + (next_var->size*next_var->data_type);
                                iter = std::prev(vars.erase(std::next(iter)));//iter should remain pointing at the same element throughout this loop
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                ++iter;
                }

                //free pages that are all free space
                iter = vars.begin();
                while(iter != vars.end())
                {
                    curr_var = *iter;

                    if(curr_var->name.compare(std::string("<FREE_SPACE>")) == 0)
                    {
                        int end_addr = curr_var->virtual_address + curr_var->size;
                        int pages_empty;

                        //free_space starts on page line
                        if(curr_var->virtual_address % page_size == 0)
                        {
                            pages_empty = curr_var->size/page_size;
                            if(end_addr % page_size == 0)
                            {
                                //case 1: free_space starts and ends on page borders:
                                //remove empty page(s)
                                int start_page = curr_var->virtual_address/page_size;
                                for(int i = 0; i < curr_var->size/page_size; i++)
                                {
                                    pagetable->removeEntry(pid, (curr_var->virtual_address/page_size)+i);
                                }
                                //remove entire variable
                                iter = std::prev(vars.erase(iter));
                            }
                            else
                            {
                                //case 2: free_space starts on page border, ends off border:
                                pages_empty = curr_var->size/page_size;
                                if(pages_empty > 0)
                                {
                                    //remove empty pages
                                    for(int i = 0; i < pages_empty; i++)
                                    {
                                        pagetable->removeEntry(pid, (curr_var->virtual_address/page_size)+i);

                                    }
                                    //shift free_space to start at beginning of last page it overlaps
                                    curr_var->virtual_address = end_addr - (end_addr%page_size);
                                    curr_var->size = end_addr % page_size;
                                }
                            }                        
                        }
                        else
                        {//free_space starts off page border

                            if(end_addr % page_size == 0)
                            {
                                //case 3: free_space starts off page border, ends on page border
                                pages_empty = curr_var->size/page_size;
                                if(pages_empty > 0)
                                {
                                    //shift size so that free_space ends at end of its first page
                                    curr_var->size = page_size - (curr_var->virtual_address % page_size);
                                    for(int i = 0; i < pages_empty; i++)
                                    {
                                        pagetable->removeEntry(pid, (curr_var->virtual_address/page_size)+1+i);
                                    }
                                }
                            }
                            else
                            {
                                //case 4: free_space starts off page border, ends off border
                                if(curr_var->size > page_size)
                                {
                                    //discount extra from low end
                                    pages_empty = curr_var->size - (page_size -(curr_var->virtual_address % page_size));
                                    //discount extra from high end
                                    pages_empty = pages_empty - (end_addr % page_size);
                                    pages_empty = pages_empty/page_size;

                                    if(pages_empty > 0)
                                    {
                                        //create a new free space var for the high end extra
                                        Variable* new_var = new Variable();
                                        new_var->name = std::string("<FREE_SPACE>");
                                        new_var->virtual_address = end_addr - (end_addr % page_size);//beginning of last page
                                        new_var->size = end_addr % page_size;
                                        new_var->data_type = 1;
                                        iter = vars.insert(std::next(iter), new_var);
                                        //curr_var size now extends to end of page containing virtual_addr
                                        curr_var->size = (page_size -(curr_var->virtual_address%page_size));

                                        //remove page(s) between these two vars
                                        for(int i = 0; i < pages_empty; i++)
                                        {
                                            pagetable->removeEntry(pid, (curr_var->virtual_address/page_size)+1+i);
                                        }   
                                    }
                                }
                            }   
                        }
                    }
                    iter++;
                }
                //update mmu
                mmu->getProcess(pid)->variables = vars;
            }
        }//end free
         if(args[0] == "terminate") //<PID>
        {
            int argc = args.size();
            if(argc != 2)
            {
                std::cout << "Error: 'terminate' expects one argument" << std::endl;
            }
            else { //kill all processes of the pid
            uint32_t pid = stoi(args[1]);
            //kill(pid, SIGTERM);
            
            //free the processes, delete them from mmu
            //std::string var_name = "delete";
            std::vector<Variable*> vars = mmu->getProcess(pid)->variables;

            //mark all variables with the pid as <FREE_SPACE>
            for(int i = 0; i < vars.size(); i++)
            {
                vars[i]->name = std::string("<FREE_SPACE>");
                vars[i]->size = vars[i]->size*vars[i]->data_type;
                vars[i]->data_type = 1;
            }
            
            std::vector<Variable*>::iterator iter = vars.begin();
            Variable* curr_var;
            Variable* next_var;
            while(iter != vars.end())
            {
                curr_var = *iter;
                 if(curr_var->name.compare(std::string("<FREE_SPACE>")) == 0)
                {
                    while(std::next(iter) != vars.end())
                    {
                        next_var = *std::next(iter);
                        if(next_var->name.compare(std::string("<FREE_SPACE>")) == 0)
                        {
                            curr_var->size = (curr_var->size*curr_var->data_type) + (next_var->size*next_var->data_type);
                            iter = std::prev(vars.erase(std::next(iter)));//iter should remain pointing at the same element throughout this loop
                        }
                        else
                        {
                            break;
                        }
                    }
                }
               ++iter;
            }
    
            //delete all entries in page table with pid
            pagetable->deleteEntry(pid);
            //update mmu
            mmu->getProcess(pid)->variables = vars;
            
            }
            
        }//end terminate




        // Get next command
        std::cout << "> ";
        std::getline (std::cin, command);
    }
    //free table memory
    pagetable->deleteTable();
    return 0;
}




void printStartMessage(int page_size)
{
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std:: endl;
    std::cout << "Commands:" << std:: endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std:: endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std:: endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std:: endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std:: endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std:: endl;
    std::cout << "  * print <object> (prints data)" << std:: endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std:: endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std:: endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std:: endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std:: endl;
    std::cout << std::endl;
}


// Returns vector of strings created by splitting `text` on every occurance of `d`
std::vector<std::string> splitString(std::string text, char d)
{   
    std::vector<std::string> result;
    char text_cstr[text.length()];
    char * c_ptr;
    strcpy(text_cstr, text.c_str());
    c_ptr = strtok(text_cstr, &d);

    while(c_ptr != NULL)
    {
        result.push_back(std::string(c_ptr));
        c_ptr = strtok(NULL, &d);
    }
    return result;
}

int dataTypeToSize(std::string data_type)
{
    int result = 0;
    if(data_type.compare(std::string("char")) == 0)
    {
        result = 1;
    }
    if(data_type.compare(std::string("short")) == 0)
    {
        result = 2;
    }
    if(data_type.compare(std::string("int")) == 0 || data_type.compare(std::string("float")) == 0)
    {
        result = 4;
    }
    if(data_type.compare(std::string("long")) == 0 || data_type.compare(std::string("double")) == 0)
    {
        result = 8;
    }

    return result;
}

void printElement(uint8_t* memory, int physical_address, int data_type, bool isFloating)
{
    if(data_type == 1)
    {
        char* elem_ptr;
        elem_ptr = (char*) &memory[physical_address];
        std::cout << *elem_ptr << ", ";
    }
    if(data_type == 2)
    {
        short* elem_ptr;
        elem_ptr = (short*) &memory[physical_address];
        std::cout << *elem_ptr << ", ";
    }
    if(data_type == 4)
    {
        if(isFloating)
        {
            float* elem_ptr;
            elem_ptr = (float*) &memory[physical_address];
            std::cout << *elem_ptr << ", ";
        }
        else
        {
            int* elem_ptr;
            elem_ptr = (int*) &memory[physical_address];
            std::cout << *elem_ptr << ", ";
        }
    }
    if(data_type == 8)
    {
        if(isFloating)
        {
            double* elem_ptr;
            elem_ptr = (double*) &memory[physical_address];
            std::cout << *elem_ptr << ", ";
        }
        else
        {
            long* elem_ptr;
            elem_ptr = (long*) &memory[physical_address];
            std::cout << *elem_ptr << ", ";
        }
        
    }
}