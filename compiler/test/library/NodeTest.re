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
        Assert.node(
          Fmt.string,
          (__value, __type, __range),
          Node.create(__value, __type, __range),
        )
    ),
    "get_value()" >: (() => Assert.string(__value, Node.get_value(__node))),
    "get_type()" >: (() => Assert.type_(__type, Node.get_type(__node))),
    "get_range()" >: (() => Assert.range(__range, Node.get_range(__node))),
    "pp()"
    >: (
      () =>
        Assert.string(
          "foo (nil) @ 1.8-3.4",
          __node |> ~@Node.pp(Fmt.string, Type.pp),
        )
    ),
  ],
  "Library.Node.Raw"
  >::: [
    "create()"
    >: (
      () =>
        Assert.raw_node(
          Fmt.string,
          (__value, __range),
          Node.Raw.create(__value, __range),
        )
    ),
    "get_value()"
    >: (() => Assert.string(__value, Node.Raw.get_value(__raw_node))),
    "get_range()"
    >: (() => Assert.range(__range, Node.Raw.get_range(__raw_node))),
    "pp()"
    >: (
      () =>
        Assert.string(
          "foo @ 1.8-3.4",
          __raw_node |> ~@Node.Raw.pp(Fmt.string),
        )
    ),
  ],
];
