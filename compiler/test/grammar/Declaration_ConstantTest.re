open Kore;

module Declaration = Grammar.Declaration;

module Assert =
  Assert.Make({
    type t = (string, AST.declaration_t);

    let parser = Parser.parse(Declaration.constant);

    let test =
      Alcotest.(
        check(
          testable(
            (x, y) => AST.print_decl(y) |> Format.print_string,
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Declaration - Constant"
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
