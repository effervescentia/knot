/**
 Parsers to extract information from modules.
 */
open Kore;
open Parse.Parser;

module Program = Grammar.Program;

/**
 parses document head to extract only the import statements

 anything that cannot be parsed as an import statement will be ignored
 */
let imports = (namespace: Reference.Namespace.t, input: Program.input_t) =>
  parse(
    namespace |> NamespaceContext.create(~report=ignore) |> Program.imports,
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
             | AST.Import(namespace, _) => Some(namespace)
             | _ => None
           ),
         )
    | None => []
  );

/**
 parses entire document to extract imports, declarations and type information
 */
let ast = (ctx: NamespaceContext.t, input: Program.input_t) =>
  input
  |> parse(Program.main(ctx))
  |> (
    fun
    | Some(stmts) => Ok(stmts)
    | None => Error(InvalidModule(ctx.namespace))
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
