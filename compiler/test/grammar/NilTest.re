open Kore;
open Util;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    type t = AST.primitive_t;

    let parser = _ => Parser.parse(Primitive.nil);

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
  "Grammar.Primitive (Nil)"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse"
    >: (() => ["nil", " nil "] |> Assert.parse_all(AST.nil |> as_nil)),
  ];
