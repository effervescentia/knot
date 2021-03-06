open Kore;

let suite =
  "Library - Print"
  >::: [
    "opt()"
    >: (
      () =>
        [
          ("", Print.opt(_ => "a", None)),
          ("a", Print.opt(Functional.identity, Some("a"))),
        ]
        |> Assert.(test_many(string))
    ),
    "many()"
    >: (
      () =>
        [
          ("abc", Print.many(Functional.identity, ["a", "b", "c"])),
          (
            "a b c",
            Print.many(~separator=" ", Functional.identity, ["a", "b", "c"]),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
