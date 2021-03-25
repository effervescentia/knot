open Kore;
open Util;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    type t = AST.primitive_t;

    let parser = _ => Parser.parse(Primitive.boolean);

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Tuple.fst3
              % Debug.print_prim
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
        ["true", " true "] |> Assert.parse_all(AST.of_bool(true) |> as_bool)
    ),
    "parse false"
    >: (
      () =>
        ["false", " false "]
        |> Assert.parse_all(AST.of_bool(false) |> as_bool)
    ),
  ];
