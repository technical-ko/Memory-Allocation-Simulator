#include "mmu.h"

Mmu::Mmu(int memory_size)
{
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu()
{
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process();
    proc->pid = _next_pid;
    _processes.push_back(proc);

    _next_pid++;
    return proc->pid;
}


Process* Mmu::getProcess(uint32_t pid)
{
    Process* result = NULL;
    for (int i=0; i < _processes.size(); i++)
    {
        if(_processes[i]->pid == pid)
        {
            result = _processes[i];
        }
    }
    return result;
}

//returns a vector of variables pointers for the free space entries of the given process,
//ordered by increasing virtual_address.
//If none are found, returns an empty vector.
std::vector<Variable*> Mmu::getVarByName(uint32_t pid, std::string name)
{
    std::vector<Variable*> result;
    std::vector<Variable*> vars = getProcess(pid)->variables;
    Variable* curr_var;
    for(int i = 0; i < vars.size(); i++)
    {
        curr_var = vars[i];
        if(curr_var->name.compare(name) == 0)
        {
            result.push_back(curr_var);
        }
    }
    return result;
}





void Mmu::print()
{
    int i, j;
    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++)
    {
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            // TODO: print all variables (excluding <FREE_SPACE> entries)
            Variable* var = _processes[i]->variables[j];
            if(var->name.compare(std::string("<FREE_SPACE>")) != 0)
            {
                std::string name = _processes[i]->variables[j]->name;
                printf("%d  | %9s %5s 0x%08x %3s %d \n", _processes[i]->pid, name.c_str(), "|", var->virtual_address, "|", var->size);
            }
        }
    }
}

void Mmu::printProcesses()
{
    for(int i = 0; i < _processes.size(); i++)
    {
        std::cout << _processes[i]->pid << std::endl;
    }
}