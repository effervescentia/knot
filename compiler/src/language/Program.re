open Knot.Kore;
open Parse.Kore;
open AST;

type input_t = LazyStream.t(Input.t);
type output_t = option((Module.program_t, input_t));
type t = (ParseContext.t, input_t) => output_t;

let _program = x => x << (eof() |> Matchers.lexeme);

let imports: t =
  ctx => choice([KImport.Plugin.parse(ctx), any >> none]) |> many;

let main: t =
  ctx =>
    choice([KImport.Plugin.parse(ctx), KDeclaration.Plugin.parse(ctx)])
    |> many
    |> _program;

let definition = (ctx: ParseContext.t) =>
  KTypeDefinition.Plugin.parse(ctx) |> many |> _program;
