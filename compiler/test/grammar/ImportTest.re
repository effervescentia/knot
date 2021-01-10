open Kore;

module Import = Grammar.Import;

module Assert =
  Assert.Make({
    type t = AST.module_statement_t;

    let parser = Parser.parse(Import.parser);

    let test =
      Alcotest.(
        check(
          testable(_ => fmt_mod_stmt % Format.print_string, (==)),
          "program matches",
        )
      );
  });

let suite =
  "Import"
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
          "import foo from \"bar\"",
          AST.of_import(("bar", "foo")),
        )
    ),
    "parse terminated"
    >: (
      () =>
        Assert.parse(
          "import foo from \"bar\";",
          AST.of_import(("bar", "foo")),
        )
    ),
  ];
