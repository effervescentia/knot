open Kore;

let __value = "foo";
let __range = Range.create((1, 8), (3, 4));
let __type = Type.Valid(`Nil);
let __raw_node = (__value, __range);
let __node = (__value, __type, __range);

let suite = [
  "Library.Node"
  >::: [
    "create()"
    >: (
      () =>
        [
          (
            (__value, __type, __range),
            Node.create(__value, __type, __range),
          ),
        ]
        |> Assert.(test_many(node(Functional.identity)))
    ),
    "get_value()"
    >: (
      () =>
        [(__value, Node.get_value(__node))] |> Assert.(test_many(string))
    ),
    "get_type()"
    >: (
      () => [(__type, Node.get_type(__node))] |> Assert.(test_many(type_))
    ),
    "get_range()"
    >: (
      () =>
        [(__range, Node.get_range(__node))] |> Assert.(test_many(range))
    ),
  ],
  "Library.Node.Raw"
  >::: [
    "create()"
    >: (
      () =>
        [((__value, __range), Node.Raw.create(__value, __range))]
        |> Assert.(test_many(raw_node(Functional.identity)))
    ),
    "get_value()"
    >: (
      () =>
        [(__value, Node.Raw.get_value(__raw_node))]
        |> Assert.(test_many(string))
    ),
    "get_range()"
    >: (
      () =>
        [(__range, Node.Raw.get_range(__raw_node))]
        |> Assert.(test_many(range))
    ),
  ],
];
