using static SLangC_.AST;

namespace SLangC_
{
    class Program
    {
        static void Main(string[] args)
        {
            Exp expr = new BinaryExp(new BinaryExp(new Constant(10), OPERATOR.PLUS, new Constant(2)), OPERATOR.MUL, new UnaryExp(new Constant(3), OPERATOR.PLUS));

            double result = Evaluator.Evaluate(expr);

            Console.WriteLine($"Result: {result}");

            string outputFolder = "output";
            Directory.CreateDirectory(outputFolder);

            string outputPath = Path.Combine(outputFolder, "program.vm");

            Evaluator.Emit(expr, outputPath);

            Console.WriteLine($"VM bytecode generated: {outputPath}");
        }
    }
}