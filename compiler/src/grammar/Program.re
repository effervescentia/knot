open Kore;

type input_t = LazyStream.t(Char.t);
type output_t = option((list(AST.module_statement_t), input_t));
type t = input_t => output_t;

let _program = x => x << (eof() |> M.lexeme);

let imports: t = choice([Import.parser, any >> none]) |> many;

let main: t = choice([Import.parser, Declaration.parser]) |> many |> _program;
