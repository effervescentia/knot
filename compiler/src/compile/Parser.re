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
let imports = (namespace, input) =>
  parse(Program.imports(Context.create(~report=ignore, namespace)), input)
  |> (
    fun
    | Some(stmts) =>
      stmts
      |> List.filter_map(
           fun
           | AST.Import(namespace, _) => Some(namespace)
           | _ => None,
         )
    | None => []
  );

/**
 parses entire document to extract imports, declarations and type information
 */
let ast = (ctx, input) =>
  parse(Program.main(ctx), input)
  |> (
    fun
    | Some(stmts) => stmts
    | None => {
        ctx.report(InvalidModule(ctx.namespace));
        [];
      }
  );
