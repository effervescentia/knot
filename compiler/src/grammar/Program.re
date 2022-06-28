open Kore;

type input_t = LazyStream.t(Input.t);
type output_t = option((AST.program_t, input_t));
type t = (ParseContext.t, input_t) => output_t;

let _program = x => x << (eof() |> M.lexeme);

let imports: t =
  ctx =>
    choice([ctx |> ParseContext.create_module |> Import.parser, any >> none])
    |> many;

let main: t =
  ctx => {
    let module_ctx = ParseContext.create_module(ctx);

    choice([Import.parser(module_ctx), Declaration.parser(module_ctx)])
    |> many
    |> _program;
  };

let definition = (ctx: ParseContext.t) =>
  Typing.module_parser(ctx) |> many |> _program;
