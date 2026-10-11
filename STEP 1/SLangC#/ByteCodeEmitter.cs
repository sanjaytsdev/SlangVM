using System.Globalization;

namespace SLangC_
{
    using static AST;
    public static class VmFileEmitter
    {
        public static void EmitToFile(Exp expr, string filePath)
        {
            var instructions = new List<string>();

            Emit(expr, instructions);

            instructions.Add("HALT");

            File.WriteAllLines(filePath, instructions);
        }

        private static void Emit(Exp expr, List<string> instructions)
        {
            switch (expr)
            {
                case Constant c:
                    instructions.Add("PUSH " + c.value.ToString("R", CultureInfo.InvariantCulture));
                    break;

                case UnaryExp u:
                    Emit(u.expr, instructions);

                    switch (u.op)
                    {
                        case OPERATOR.PLUS:
                            break;

                        case OPERATOR.MINUS:
                            instructions.Add("NEG");
                            break;

                        default:
                            throw new NotSupportedException($"Unsupported unary operator: {u.op}");
                    }
                    break;

                case BinaryExp b:
                    Emit(b.expr1, instructions);
                    Emit(b.expr2, instructions);

                    string op = b.op switch
                    {
                        OPERATOR.PLUS => "ADD",
                        OPERATOR.MINUS => "SUB",
                        OPERATOR.MUL => "MUL",
                        OPERATOR.DIV => "DIV",
                        _ => throw new NotSupportedException($"Unsupported operator: {b.op}")
                    };

                    instructions.Add(op);
                    break;

                default:
                    throw new NotSupportedException($"Unsupported expression: {expr.GetType().Name}");
            }
        }
    }
}