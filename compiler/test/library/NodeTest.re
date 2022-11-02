open Kore;

let __value = "foo";
let __range = Range.create((1, 8), (3, 4));
let __type = AST.Type.Valid(`Nil);
let __raw_node = (__value, __range);
let __node = Node.typed(__value, __type, __range);

let suite = [
  "Library.Node"
  >::: [
    "create()"
    >: (
      () =>
        Assert.typed_node(
          Fmt.string,
          AST.Type.pp,
          (__value, (__type, __range)),
          Node.typed(__value, __type, __range),
        )
    ),
    "get_type()" >: (() => Assert.type_(__type, Node.get_type(__node))),
    "get_range()" >: (() => Assert.range(__range, Node.get_range(__node))),
    "pp()"
    >: (
      () =>
        Assert.string(
          "foo (nil) @ 1.8-3.4",
          __node |> ~@Node.pp(Fmt.string, AST.Type.pp),
        )
    ),
  ],
];
