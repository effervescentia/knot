open Kore;

module Namespace = {
  open Reference.Namespace;

  let foo = Internal("foo");
  let bar = External("bar");
  let fizz = Internal("fizz");
  let buzz = External("buzz");
};

module Graph = {
  module N = Namespace;

  let two_node = () => Graph.create([N.bar, N.foo], [(N.foo, N.bar)]);

  let three_node = () =>
    Graph.create(
      [N.fizz, N.bar, N.foo],
      [(N.foo, N.bar), (N.bar, N.fizz)],
    );
};

module Program = {
  module U = Util.ResultUtil;

  let const_int =
    AST.[
      (
        "ABC" |> of_public |> U.as_untyped |> of_named_export,
        123L |> of_int |> of_num |> of_prim |> U.as_int |> of_const |> U.as_int,
      )
      |> of_decl
      |> U.as_untyped,
    ];
};
