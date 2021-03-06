open Kore;
open Util;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    type t = Block.t(AST.primitive_t);

    let parser = Parser.parse(Primitive.nil);

    let test =
      Alcotest.(
        check(
          testable(
            pp => fmt_block(fmt_prim) % Format.pp_print_string(pp),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar - Nil"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse"
    >: (() => ["nil", " nil "] |> Assert.parse_all(AST.nil |> as_nil)),
  ];
