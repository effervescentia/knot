open Kore;

let suite =
  "Library - Functional"
  >::: [
    "identity()"
    >: (
      () => [("a", Functional.identity("a"))] |> Assert.(test_many(string))
    ),
  ];
