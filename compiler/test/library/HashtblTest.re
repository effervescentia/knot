open Kore;

let __table = [("foo", "bar")] |> List.to_seq |> Hashtbl.of_seq;

let suite =
  "Library.Hashtbl"
  >::: [
    "compare()"
    >: (
      () => {
        Hashtbl.compare(__table, Hashtbl.copy(__table)) |> Assert.true_;

        let big_table = Hashtbl.create(10);
        Hashtbl.add(big_table, "foo", "bar");
        Hashtbl.compare(__table, big_table) |> Assert.true_;

        let different_table =
          [("fizz", "buzz")] |> List.to_seq |> Hashtbl.of_seq;
        Hashtbl.compare(__table, different_table) |> Assert.false_;
      }
    ),
    "pp()"
    >: (
      () =>
        [
          (
            "Hashtbl {
  foo: bar
}",
            __table |> ~@Fmt.(root(Hashtbl.pp(string, string))),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
