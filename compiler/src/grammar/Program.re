open Kore;

type input_t = LazyStream.t(Input.t);
type output_t = option((AST.program_t, input_t));
type t = (ParseContext.t, input_t) => output_t;

let _program = x => x << (eof() |> M.lexeme);

let imports: t = ctx => choice([Import.parser(ctx), any >> none]) |> many;

let main: t =
  ctx =>
    choice([Import.parser(ctx), Declaration.parser(ctx)])
    |> many
    |> _program;

let definition = (ctx: ParseContext.t) =>
  Typing.root_parser(ctx) |> many |> _program;
