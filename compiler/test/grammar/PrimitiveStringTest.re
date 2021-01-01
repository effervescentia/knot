open Kore;

module Primitive = Grammar.Primitive;

module Assert =
  Assert.Make({
    type t = (string, Cursor.t);

    let parser = Parser.parse(Primitive.string);

    let test =
      Alcotest.(
        check(
          testable(
            (x, y) =>
              Print.fmt("%s %s", fst(y), snd(y) |> Cursor.to_string)
              |> Format.print_string,
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "String"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse string"
    >: (
      () =>
        Assert.parse(
          "\"foo\"",
          ("foo", Cursor.range(Cursor.point(1, 1), Cursor.point(1, 5))),
        )
    ),
    "with escape characters"
    >: (
      () =>
        Assert.parse(
          "\"foo\\\"bar\"",
          (
            "foo\\\"bar",
            Cursor.range(Cursor.point(1, 1), Cursor.point(1, 10)),
          ),
        )
    ),
  ];
