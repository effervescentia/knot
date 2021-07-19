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
              % AST.Raw.Debug.print_prim
              % Format.pp_print_string(pp),
            (==),
          ),
          "program matches",
        )
      );
  });

let _raw_bool = RawUtil.bool % as_bool;

let suite =
  "Grammar.Primitive (Boolean)"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse true"
    >: (() => ["true", " true "] |> Assert.parse_all(_raw_bool(true))),
    "parse false"
    >: (() => ["false", " false "] |> Assert.parse_all(_raw_bool(false))),
  ];
