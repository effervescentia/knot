open Kore;

module U = Util.RawUtil;

module Assert =
  Assert.MakePrimitive({
    let parser = Primitive.Parser.parse_primitive;
  });

let suite =
  "Grammar.Primitive | Nil"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse"
    >: (() => Assert.parse_all(U.as_node(Primitive.nil), ["nil", " nil "])),
  ];
