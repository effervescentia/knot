open Kore;

module Primitive = Grammar.Primitive;
module U = Util.RawUtil;

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
