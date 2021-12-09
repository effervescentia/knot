open Kore;
open Util.RawUtil;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    open AST;

    type t = Raw.primitive_t;

    let parser = _ => Parser.parse(Primitive.nil);

    let test =
      Alcotest.(
        check(
          testable(pp => Raw.Dump.prim_to_entity % Dump.Entity.pp(pp), (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Primitive (Nil)"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse"
    >: (
      () => ["nil", " nil "] |> Assert.parse_all(AST.Raw.nil |> as_raw_node)
    ),
  ];
