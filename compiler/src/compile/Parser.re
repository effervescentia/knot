/**
 Parsers to extract information from modules.
 */
open Kore;
open Parse.Parser;

module ParseContext = AST.ParseContext;
module Program = Language.Program;

/**
 parses document head to extract only the import statements

 anything that cannot be parsed as an import statement will be ignored
 */
let imports = (namespace: Reference.Namespace.t, input: Program.input_t) =>
  parse(
    namespace |> ParseContext.create(~report=ignore) |> Program.imports,
    input,
  )
  |> (
    fun
    | Some(stmts) =>
      stmts
      |> List.filter_map(
           fst
           % (
             fun
             | AST.Result.Import(namespace, _) => Some(namespace)
             | _ => None
           ),
         )
    | None => []
  );

/**
 parses entire document to extract imports, declarations and type information
 */
let ast = (ctx: ParseContext.t, input: Program.input_t) =>
  input
  |> parse(Program.main(ctx))
  |> (
    fun
    | Some(stmts) => Ok(stmts)
    | None => Error(AST.Error.InvalidModule(ctx.namespace))
  );

/**
 parses entire document to extract type modules and definitions
 */
let definition = (ctx: ParseContext.t, input: Program.input_t) =>
  input
  |> parse(Program.definition(ctx))
  |> (
    fun
    | Some(modules_) => Ok(modules_)
    | None => Ok([])
  );
