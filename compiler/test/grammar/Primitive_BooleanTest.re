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
            ppf => Raw.Dump.prim_to_entity % Dump.Entity.pp(ppf),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Primitive | Boolean"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse true"
    >: (
      () =>
        ["true", " true "]
        |> Assert.parse_all(AST.Raw.of_bool(true) |> as_raw_node)
    ),
    "parse false"
    >: (
      () =>
        ["false", " false "]
        |> Assert.parse_all(AST.Raw.of_bool(false) |> as_raw_node)
    ),
  ];
