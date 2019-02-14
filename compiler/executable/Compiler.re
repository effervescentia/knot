module FileStream = Knot.FileStream;
module TokenStream = KnotLex.TokenStream;
module Parser = KnotParse.Parser;
module Analyzer = KnotAnalyze.Analyzer;
module Generator = KnotGen.Generator;

exception InvalidProgram;

let in_path =
  switch (Sys.argv) {
  | [|_, path|] => path
  | _ => raise(Arg.Bad("must provide the path to a source file"))
  };

/* let execute = prog => {
     let env: Hashtbl.t(string, Globals.env_value) = Hashtbl.create(16);
     Evaluator.eval(prog, env);

     Debug.print_env(env);
   }; */

let () =
  open_in(in_path)
  |> FileStream.of_channel
  |> TokenStream.of_file_stream(~filter=TokenStream.filter_comments)
  |> Parser.parse(Parser.prog)
  |> Analyzer.analyze
  |> (
    fun
    | Some(ast) =>
      /* KnotParse.Debug.print_ast(ast) |> prerr_endline; */
      Generator.generate(print_string, ast)
    | None => raise(InvalidProgram)
  );
