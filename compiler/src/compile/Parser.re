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
let imports =
  AST.(
    input =>
      parse(Program.imports, input)
      |> (
        fun
        | Some(stmts) =>
          stmts
          |> List.filter_map(
               fun
               | Import(name, _) => Some(to_m_id(name))
               | _ => None,
             )
        | None => raise(ParseFailed)
      )
  );

/**
 parses entire document to extract imports, declarations and type information
 */
let ast =
  AST.(
    input =>
      parse(Program.main, input)
      |> (
        fun
        | Some(stmts) => stmts
        | None => raise(ParseFailed)
      )
  );
