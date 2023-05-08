open Kore;

let __table = [("foo", "bar")] |> List.to_seq |> Hashtbl.of_seq;

let suite =
  "Library.Hashtbl"
  >::: [
    "compare()"
    >: (
      () => {
        Assert.true_(Hashtbl.compare(__table, Hashtbl.copy(__table)));

        let big_table = Hashtbl.create(10);
        Hashtbl.add(big_table, "foo", "bar");

        Assert.true_(Hashtbl.compare(__table, big_table));

        let different_table =
          [("fizz", "buzz")] |> List.to_seq |> Hashtbl.of_seq;

        Assert.false_(Hashtbl.compare(__table, different_table));
      }
    ),
    "pp()"
    >: (
      () =>
        Assert.string(
          "Hashtbl {
  foo: bar
}",
          __table |> ~@Fmt.(root(Hashtbl.pp(string, string))),
        )
    ),
  ];
