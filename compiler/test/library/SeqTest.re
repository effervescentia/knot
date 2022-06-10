open Kore;

let suite =
  "Library.Seq"
  >::: [
    "append()"
    >: (
      () =>
        Assert.string_list(
          ["a", "b", "c", "d"],
          Seq.append(["a", "b"] |> List.to_seq, ["c", "d"] |> List.to_seq)
          |> List.of_seq,
        )
    ),
  ];
