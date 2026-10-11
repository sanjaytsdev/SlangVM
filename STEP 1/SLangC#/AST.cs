using System;
using System.Collections.Generic;
using System.Text;

namespace SLangC_
{
    public class AST
    {
        public enum OPERATOR
        {
            PLUS,
            MINUS,
            MUL,
            DIV
        }
        public abstract record Exp;

        public record Constant(double value) : Exp;

        public record UnaryExp(Exp expr, OPERATOR op) : Exp;

        public record BinaryExp(Exp expr1, OPERATOR op, Exp expr2) : Exp;
    }
}
