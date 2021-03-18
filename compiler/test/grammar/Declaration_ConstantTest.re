open Kore;
open AST;
open Util;

module Declaration = Grammar.Declaration;

module Assert =
  Assert.Make({
    type t = (identifier_t, declaration_t);

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
          ("foo" |> of_public |> as_lexeme, Util.nil_prim |> of_const),
        )
    ),
  ];
