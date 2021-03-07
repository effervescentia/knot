open Kore;

module Declaration = Grammar.Declaration;

module Assert =
  Assert.Make({
    type t = (string, AST.declaration_t);

    let parser = Parser.parse(Declaration.constant);

    let test =
      Alcotest.(
        check(
          testable(pp => fmt_decl % Format.pp_print_string(pp), (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Declaration (Constant)"
  >::: [
    "no parse"
    >: (
      () =>
        ["gibberish", "const", "const foo", "const foo ="] |> Assert.no_parse
    ),
    "parse"
    >: (
      () =>
        Assert.parse(
          "const foo = nil",
          ("foo", Util.nil_prim |> AST.of_const),
        )
    ),
  ];
