open Kore;

module Import = Grammar.Import;

module Assert =
  Assert.Make({
    type t = AST.module_statement_t;

    let parser = Parser.parse(Import.parser);

    let test =
      Alcotest.(
        check(
          testable(pp => fmt_mod_stmt % Format.pp_print_string(pp), (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Import"
  >::: [
    "no parse"
    >: (
      () =>
        ["gibberish", "import", "import foo", "import foo from"]
        |> Assert.no_parse
    ),
    "parse"
    >: (
      () =>
        Assert.parse(
          "import foo from \"@/bar\"",
          AST.(of_import(("bar" |> of_internal, "foo"))),
        )
    ),
    "parse terminated"
    >: (
      () =>
        Assert.parse(
          "import foo from \"@/bar\";",
          AST.(of_import(("bar" |> of_internal, "foo"))),
        )
    ),
  ];
