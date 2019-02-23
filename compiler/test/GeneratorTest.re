open Core;

let tests =
  "KnotGen.Generator"  /* "generate module"
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
                | Some(res) =>
                  KnotGen.Generator.generate(s => generated := generated^ ++ s, res)
                | None => assert_failure("no AST found")
              );

              Assert.assert_string_eql(generated^, Fixtures.full_generated);
            }
          ), */
  >::: [];
