open Kore;

type input_t = LazyStream.t(Input.t);
type output_t = option((AST.program_t, input_t));
type t = (NamespaceContext.t, input_t) => output_t;

let _program = x => x << (eof() |> M.lexeme);

let imports: t =
  ctx =>
    choice([ctx |> ModuleContext.create |> Import.parser, any >> none])
    |> many;

let main: t =
  ctx => {
    let module_ctx = ModuleContext.create(ctx);

    ctx
    |> NamespaceContext.define_module(
         Root,
         module_ctx.declarations,
         Range.zero,
       );

    choice([Import.parser(module_ctx), Declaration.parser(module_ctx)])
    |> many
    |> _program;
  };
