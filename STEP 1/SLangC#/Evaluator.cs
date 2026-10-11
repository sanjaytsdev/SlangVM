using System;
using System.Collections.Generic;
using System.Text;

namespace SLangC_
{
    using static AST;

    public static class Evaluator
    {
        public static double Evaluate(Exp expr)
        {
            VmInstruction[] instructions = BytecodeInterpreter.Compile(expr);

            return NativeVm.Execute(instructions);
        }

        public static void Emit(Exp expr, string filePath)
        {
            VmFileEmitter.EmitToFile(expr, filePath);
        }
    }
}
