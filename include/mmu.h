#ifndef __MMU_H_
#define __MMU_H_

#include <iostream>
#include <string>
#include <vector>

typedef struct Variable {
    std::string name;
    int virtual_address;
    int size;
    int data_type;
    bool isFloating;
} Variable;

typedef struct Process {
    uint32_t pid;
    std::vector<Variable*> variables;
} Process;

class Mmu {
private:
    uint32_t _next_pid;
    int _max_size;
    std::vector<Process*> _processes;

public:
    Mmu(int memory_size);
    ~Mmu();

    uint32_t createProcess();
    Process* getProcess(uint32_t pid);
    std::vector<Variable*> getVarByName(uint32_t pid, std::string name);
    void print();
    void printProcesses();
};

#endif // __MMU_H_
