open Kore;

module Primitive = Grammar.Primitive;
module U = Util.RawUtilV2;

module Assert =
  Assert.MakePrimitive({
    let parser = Primitive.nil;
  });

let suite =
  "Grammar.Primitive | Nil"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse" >: (() => Assert.parse_all(AR.nil |> U.as_nil, ["nil", " nil "])),
  ];
