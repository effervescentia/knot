/**
 Parsers to extract information from modules.
 */
open Knot.Kore;

module ParseContext = AST.ParseContext;
module Program = Language.Program;

let _parse = Parse.Parser.parse;

/**
 parses document head to extract only the import statements

 anything that cannot be parsed as an import statement will be ignored
 */
let imports = (namespace: Reference.Namespace.t, input: Program.input_t) =>
  _parse(
    namespace |> ParseContext.create(~report=ignore) |> Program.imports,
    input,
  )
  |> (
    fun
    | Some(stmts) =>
      stmts
      |> List.filter_map(
           fst
           % AST.ModuleStatement.(
               fun
               | Import(namespace, _, _) => Some(namespace)
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
  |> _parse(Program.main(ctx))
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
  |> _parse(Program.definition(ctx))
  |> (
    fun
    | Some(modules_) => Ok(modules_)
    | None => Ok([])
  );
