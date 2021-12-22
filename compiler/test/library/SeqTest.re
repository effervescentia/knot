open Kore;

let suite =
  "Library.Seq"
  >::: [
    "append()"
    >: (
      () =>
        [
          (
            ["a", "b", "c", "d"],
            Seq.append(["a", "b"] |> List.to_seq, ["c", "d"] |> List.to_seq)
            |> List.of_seq,
          ),
        ]
        |> Assert.(test_many(string_list))
    ),
  ];
