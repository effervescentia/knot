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
          testable(
            ppf => Raw.Dump.prim_to_entity % Dump.Entity.pp(ppf),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Primitive | Nil"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse"
    >: (() => Assert.parse_all(AST.Raw.nil |> as_raw_node, ["nil", " nil "])),
  ];
