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
    AST.Result.[
      (
        AST.ModuleStatement.ExportKind.Named,
        "ABC" |> U.as_untyped,
        123 |> U.int_prim |> of_const |> U.as_int,
      )
      |> of_export
      |> U.as_untyped,
    ];
};
