open Globals;

let tests =
  "KnotGenerate.Generator"
  >::: [
    "generate module"
    >:: (
      _ => {
        let generated = ref("");
        let scope = KnotAnalyze.Scope.create();

        Util.load_resource(Config.sample_snippet)
        |> FileStream.of_channel
        |> TokenStream.of_file_stream(Lexer.next_token)
        |> Parser.parse(Parser.prog)
        |> Analyzer.analyze(~scope)
        |> (
          fun
          | Some(ast) =>
            switch (KnotResolve.Globals.opt_type_ref(ast)) {
            | _ =>
              KnotGenerate.Generator.generate(
                s => generated := generated^ ++ s,
                {
                  to_module_name: s => s,

                  to_import_statement:
                    (module_name, module_import, named_imports) =>
                    Knot.Print.optional(
                      name =>
                        Printf.sprintf(
                          "IMPORT MODULE %s from %s;",
                          name,
                          module_name,
                        ),
                      module_import,
                    )
                    ++ (
                      switch (named_imports) {
                      | [] => ""
                      | _ =>
                        Printf.sprintf(
                          "IMPORT %s FROM %s;",
                          Knot.Print.sequential(
                            ((name, alias)) =>
                              Knot.Print.optional(
                                ~default=name,
                                Printf.sprintf("%s AS %s", name),
                                alias,
                              ),
                            named_imports,
                          ),
                          module_name,
                        )
                      }
                    ),

                  to_export_statement: (name, alias) =>
                    Printf.sprintf(
                      "EXPORT %s%s;",
                      name,
                      Knot.Print.optional(Printf.sprintf(" as %s"), alias),
                    ),
                },
                fst(ast),
              )
            }
          | None => assert_failure("no AST found")
        );

        Assert.assert_string_eql(generated^, Fixtures.full_generated);
      }
    ),
  ];
