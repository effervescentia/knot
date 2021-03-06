/**
 * Parsers to extract information from modules.
 */
open Kore;
open Parse.Parser;

module Program = Grammar.Program;

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
               | Import(m_id, _) => Some(to_m_id(m_id))
               | _ => None,
             )
        | None => raise(ParseFailed)
      )
  );

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
