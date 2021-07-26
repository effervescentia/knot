open Kore;
open Util.RawUtil;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    open AST;

    type t = Raw.primitive_t;

    let parser = _ => Parser.parse(Primitive.boolean);

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Node.Raw.value
              % Raw.Debug.print_prim
              % Pretty.to_string
              % Format.pp_print_string(pp),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Primitive (Boolean)"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse true"
    >: (
      () =>
        ["true", " true "]
        |> Assert.parse_all(AST.Raw.of_bool(true) |> as_lexeme)
    ),
    "parse false"
    >: (
      () =>
        ["false", " false "]
        |> Assert.parse_all(AST.Raw.of_bool(false) |> as_lexeme)
    ),
  ];
