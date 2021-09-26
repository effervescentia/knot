open Kore;

let suite =
  "Library.Print"
  >::: [
    "opt()"
    >: (
      () =>
        [
          ("", Print.opt(_ => "a", None)),
          ("a", Print.opt(Fun.id, Some("a"))),
        ]
        |> Assert.(test_many(string))
    ),
    "many()"
    >: (
      () =>
        [
          ("abc", Print.many(Fun.id, ["a", "b", "c"])),
          ("a b c", Print.many(~separator=" ", Fun.id, ["a", "b", "c"])),
        ]
        |> Assert.(test_many(string))
    ),
  ];
