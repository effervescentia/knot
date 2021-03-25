open Kore;

type input_t = LazyStream.t(Input.t);
type output_t = option((AST.program_t, input_t));
type t = (~scope: Scope.t=?, input_t) => output_t;

let _program = x => x << (eof() |> M.lexeme);

let imports: t =
  (~scope=Scope.create()) => choice([Import.parser, any >> none]) |> many;

let main: t =
  (~scope=Scope.create()) =>
    choice([Import.parser, Declaration.parser(scope)]) |> many |> _program;
