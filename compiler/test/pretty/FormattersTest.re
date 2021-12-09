open Kore;
open Pretty.Formatters;

let suite =
  "Pretty.Formatters"
  >::: [
    "uchar()"
    >: (
      () =>
        [
          ("\\u{1f}", Uchar.of_int(31) |> ~@uchar),
          (" ", Uchar.of_int(32) |> ~@uchar),
          ("~", Uchar.of_int(126) |> ~@uchar),
          ("\\u{7f}", Uchar.of_int(127) |> ~@uchar),
          ("\\t", Uchar.of_int(9) |> ~@uchar),
          ("\\n", Uchar.of_int(10) |> ~@uchar),
          ("\\r", Uchar.of_int(13) |> ~@uchar),
        ]
        |> Assert.(test_many(string))
    ),
  ];
