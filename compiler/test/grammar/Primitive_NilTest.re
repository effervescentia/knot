open Kore;

module Primitive = Grammar.PrimitiveV2;
module U = Util.RawUtilV2;

module Assert =
  Assert.MakePrimitive({
    let parser = Primitive.parser;
  });

let suite =
  "Grammar.Primitive | Nil"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse" >: (() => Assert.parse_all(U.as_nil(AR.nil), ["nil", " nil "])),
  ];
