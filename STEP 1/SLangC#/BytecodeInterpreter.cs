using System;
using System.Collections.Generic;
using System.Text;

namespace SLangC_
{
    using static AST;
    public static class BytecodeInterpreter
    {
        public static VmInstruction[] Compile(Exp expr)
        {
            var code = new List<VmInstruction>();

            Emit(expr, code);

            code.Add(new VmInstruction(VmOpCode.Halt));

            return code.ToArray();
        }

        private static void Emit(Exp expr, List<VmInstruction> code)
        {
            switch (expr)
            {
                case Constant c:
                    code.Add(new VmInstruction(VmOpCode.Push, c.value));
                    break;

                case UnaryExp u:
                    Emit(u.expr, code);

                    switch (u.op)
                    {
                        case OPERATOR.PLUS:
                            break;

                        case OPERATOR.MINUS:
                            code.Add(new VmInstruction(VmOpCode.Neg));
                            break;

                        default:
                            throw new NotSupportedException($"Unsupported unary operator: {u.op}");
                    }
                    break;

                case BinaryExp b:
                    Emit(b.expr1, code);
                    Emit(b.expr2, code);

                    VmOpCode op = b.op switch
                    {
                        OPERATOR.PLUS => VmOpCode.Add,
                        OPERATOR.MINUS => VmOpCode.Sub,
                        OPERATOR.MUL => VmOpCode.Mul,
                        OPERATOR.DIV => VmOpCode.Div,
                        _ => throw new NotSupportedException($"Unsupported operator: {b.op}")
                    };

                    code.Add(new VmInstruction(op));
                    break;

                default:
                    throw new NotSupportedException($"Unsupported expression: {expr.GetType().Name}");
            }
        }
    }
}
