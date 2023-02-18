open Knot.Kore;
open Parse.Kore;
open AST;

type input_t = LazyStream.t(Input.t);
type output_t = option((Interface.program_t(Type.t), input_t));
type t = (ParseContext.t(Interface.program_t(Type.t)), input_t) => output_t;

let _program = x => x << (eof() |> Matchers.lexeme);

let imports: t =
  ctx =>
    choice([
      KImport.Plugin.parse(
        (
          KModuleStatement.Interface.of_import,
          KModuleStatement.Interface.of_stdlib_import,
        ),
        ctx,
      ),
      any >> none,
    ])
    |> many;

let main = ctx =>
  KModuleStatement.Plugin.parse(KDeclaration.Plugin.parse, ctx)
  |> many
  |> _program;

let definition = (ctx: ParseContext.t(Interface.program_t(Type.t))) =>
  KTypeDefinition.Plugin.parse(ctx) |> many |> _program;
