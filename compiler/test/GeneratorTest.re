open Core;

let tests =
  "KnotGenerate.Generator"  /* "generate module"
          >:: (
            _ => {
              let generated = ref("");

              Util.load_resource(Config.unix_module_file)
              |> FileStream.of_channel
              |> TokenStream.of_file_stream(~filter=TokenStream.filter_comments)
              |> Parser.parse(Parser.prog)
              |> Analyzer.analyze()
              |> (
                fun
                | Some({contents: Resolved(res, _)}) =>
                  KnotGenerate.Generator.generate(s => generated := generated^ ++ s, res)
                | _ => assert_failure("no AST found")
              );

              Assert.assert_string_eql(generated^, Fixtures.full_generated);
            }
          ), */
  >::: [];
