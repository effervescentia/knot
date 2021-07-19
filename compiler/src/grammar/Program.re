open Kore;

type input_t = LazyStream.t(Input.t);
type output_t = option((AST.Final.program_t, input_t));
type t = (Context.t, input_t) => output_t;

let _program = x => x << (eof() |> M.lexeme);

let imports: t = ctx => choice([Import.parser(ctx), any >> none]) |> many;

let main: t =
  ctx =>
    choice([Import.parser(ctx), Declaration.parser(ctx)])
    |> many
    |> _program;
