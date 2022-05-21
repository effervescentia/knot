open Kore;

module U = Util.ResultUtil;
module A = AST;
module T = Type;

module Namespace = {
  open Reference.Namespace;

  let entry = Internal("entry");
  let other = Internal("other");
  let foo = Internal("foo");
  let bar = Internal("bar");
};

module Raw = {
  let single_import = "
  import foo from \"@/bar\";

  const ABC = 123;
";

  let multiple_import = "
  import foo from \"bar\";
  import fizz from \"buzz\";

  const ABC = 123;
";
};

module Graph = {
  module N = Namespace;

  let two_node = Graph.create([N.foo, N.bar], [(N.foo, N.bar)]);
};

module Program = {
  module N = Namespace;

  let const_int = [
    (
      ("ABC" |> A.of_public, Range.create((1, 7), (1, 9)))
      |> A.of_named_export,
      123L
      |> A.of_int
      |> A.of_num
      |> A.of_prim
      |> U.as_node(
           ~range=Range.create((1, 13), (1, 15)),
           T.Valid(`Integer),
         )
      |> A.of_const
      |> U.as_node(
           ~range=Range.create((1, 13), (1, 15)),
           T.Valid(`Integer),
         ),
    )
    |> A.of_decl
    |> U.as_raw_node(~range=Range.create((1, 1), (1, 15))),
  ];

  let import_and_const = [
    (
      N.entry,
      [
        (
          "ABC"
          |> A.of_public
          |> U.as_raw_node(~range=Range.create((1, 10), (1, 12))),
          None,
        )
        |> A.of_named_import
        |> U.as_raw_node(~range=Range.create((1, 10), (1, 12))),
      ],
    )
    |> A.of_import
    |> U.as_raw_node(~range=Range.create((1, 1), (1, 29))),
    (
      ("BAR" |> A.of_public, Range.create((3, 7), (3, 9)))
      |> A.of_named_export,
      "bar"
      |> A.of_string
      |> A.of_prim
      |> U.as_node(
           ~range=Range.create((3, 13), (3, 17)),
           T.Valid(`String),
         )
      |> A.of_const
      |> U.as_node(
           ~range=Range.create((3, 13), (3, 17)),
           T.Valid(`String),
         ),
    )
    |> A.of_decl
    |> U.as_raw_node(~range=Range.create((3, 1), (3, 17))),
  ];

  let single_import = [
    (
      N.bar,
      [
        "foo"
        |> A.of_public
        |> U.as_raw_node(~range=Range.create((2, 10), (2, 12)))
        |> A.of_main_import
        |> U.as_raw_node(~range=Range.create((2, 10), (2, 12))),
      ],
    )
    |> A.of_import
    |> U.as_raw_node(~range=Range.create((2, 3), (2, 25))),
    (
      ("ABC" |> A.of_public, Range.create((4, 9), (4, 11)))
      |> A.of_named_export,
      123L
      |> A.of_int
      |> A.of_num
      |> A.of_prim
      |> U.as_node(
           ~range=Range.create((4, 15), (4, 17)),
           Type.Valid(`Integer),
         )
      |> A.of_const
      |> U.as_node(
           ~range=Range.create((4, 15), (4, 17)),
           Type.Valid(`Integer),
         ),
    )
    |> A.of_decl
    |> U.as_raw_node(~range=Range.create((4, 3), (4, 17))),
  ];
};
