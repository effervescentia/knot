open Kore;
open Pretty.Formatters;

let suite =
  "Pretty.Formatters"
  >::: [
    "uchar() - u{1f}"
    >: (() => Assert.string("\\u{1f}", Uchar.of_int(31) |> ~@uchar)),
    "uchar() - u{7f}"
    >: (() => Assert.string("\\u{7f}", Uchar.of_int(127) |> ~@uchar)),
    "uchar() - space"
    >: (() => Assert.string(" ", Uchar.of_int(32) |> ~@uchar)),
    "uchar() - tilde"
    >: (() => Assert.string("~", Uchar.of_int(126) |> ~@uchar)),
    "uchar() - tab"
    >: (() => Assert.string("\\t", Uchar.of_int(9) |> ~@uchar)),
    "uchar() - newline"
    >: (() => Assert.string("\\n", Uchar.of_int(10) |> ~@uchar)),
    "uchar() - carriage return"
    >: (() => Assert.string("\\r", Uchar.of_int(13) |> ~@uchar)),
  ];
