open Core;

let tests =
  "KnotGenerate.Generator"
  >::: [
    "generate module"
    >:: (
      _ => {
        let generated = ref("");
        let scope = KnotAnalyze.Scope.create();

        Util.load_resource(Config.sample_snippet)
        |> UnicodeFileStream.of_channel
        |> TokenStream.of_file_stream(~filter=TokenStream.filter_comments)
        |> Parser.parse(Parser.prog)
        |> Analyzer.analyze(~scope)
        |> (
          fun
          | Some(ast) =>
            switch (KnotResolve.Core.opt_type_ref(ast)) {
            | _ =>
              KnotGenerate.Generator.generate(
                s => generated := generated^ ++ s,
                s => s,
                fst(ast),
              )
            }
          | None => assert_failure("no AST found")
        );

        Assert.assert_string_eql(generated^, Fixtures.full_generated);
      }
    ),
  ];
