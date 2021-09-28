open Kore;

let suite =
  "Library.Hashtbl"
  >::: [
    "compare()"
    >: (
      () => {
        let table_a = [("foo", "bar")] |> List.to_seq |> Hashtbl.of_seq;
        Hashtbl.compare(table_a, Hashtbl.copy(table_a)) |> Assert.true_;

        let table_b = Hashtbl.create(10);
        Hashtbl.add(table_b, "foo", "bar");
        Hashtbl.compare(table_a, table_b) |> Assert.true_;

        let table_c = [("fizz", "buzz")] |> List.to_seq |> Hashtbl.of_seq;
        Hashtbl.compare(table_a, table_c) |> Assert.false_;
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
            [("foo", "bar")]
            |> ~@Fmt.struct_("Hashtbl", Fmt.string, Fmt.string),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
