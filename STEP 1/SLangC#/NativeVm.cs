using System.Runtime.InteropServices;
using System.Text;

namespace SLangC_
{
    public enum VmOpCode
    {
        Push = 0,
        Add = 1,
        Sub = 2,
        Mul = 3,
        Div = 4,
        Neg = 5,
        Halt = 6
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VmInstruction
    {
        public int Op;
        public double Operand;

        public VmInstruction(VmOpCode op, double operand = 0)
        {
            Op = (int)op;
            Operand = operand;
        }
    }

    public static class NativeVm
    {
        [DllImport("vm.dll", EntryPoint = "vm_execute", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ExecuteNative([In] VmInstruction[] instructions, nuint count, out double result, StringBuilder error, nuint errorCapacity);
        public static double Execute(VmInstruction[] instructions)
        {
            var error = new StringBuilder(512);

            int status = ExecuteNative(instructions, (nuint)instructions.Length, out double result, error, (nuint)error.Capacity);

            if (status != 0)
            {
                throw new InvalidOperationException("Native VM error: " + error);
            }

            return result;
        }
    }
}
