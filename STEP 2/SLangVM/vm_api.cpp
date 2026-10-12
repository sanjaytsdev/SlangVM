/*
    where /r "C:\Program Files\Microsoft Visual Studio" VsDevCmd.bat
                                    or
    where /r "C:\Program Files (x86)\Microsoft Visual Studio" VsDevCmd.bat

    call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64

    cl /EHsc /std:c++17 /LD vm_api.cpp /Fe:vm.dll

    dumpbin /headers vm.dll | findstr machine - Confirm that the DLL is 64-bit
*/

#include "vm.hpp"

#include <cstddef>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#define VM_API __declspec(dllexport)
#else
#define VM_API __attribute__((visibility("default")))
#endif

struct VmInstruction {
    int op;
    double operand;
};

extern "C" VM_API int vm_execute(const VmInstruction* instructions, std::size_t count, double* result, char* error, std::size_t errorCapacity) {
    if (result == nullptr) {
        return 1;
    }

    if (instructions == nullptr || count == 0) {
        if (error != nullptr && errorCapacity > 0) {
            std::snprintf(error, errorCapacity, "Instruction array is empty or null");
        }
        return 1;
    }

    try {
        std::vector<Instruction> code;
        code.reserve(count);

        for (std::size_t i = 0; i < count; ++i) {
            if (instructions[i].op < 0 || instructions[i].op > 6) {
                throw std::runtime_error("Invalid opcode");
            }

            code.push_back({
                static_cast<OpCode>(instructions[i].op),
                instructions[i].operand
            });
        }

        VirtualMachine vm;
        *result = vm.execute(code);

        if (error != nullptr && errorCapacity > 0) {
            error[0] = '\0';
        }

        return 0;
    }
    catch (const std::exception& e) {
        if (error != nullptr && errorCapacity > 0) {
            std::snprintf(error, errorCapacity, "%s", e.what());
        }
        return 1;
    }
    catch (...) {
        if (error != nullptr && errorCapacity > 0) {
            std::snprintf(error, errorCapacity, "Unknown native VM error");
        }
        return 1;
    }
}