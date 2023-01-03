open Kore;

module U = Util.ResultUtil;
module A = AST.Result;
module T = AST.Type;
module ExportKind = AST.ModuleStatement.ExportKind;

let _fixture = Filename.concat("./test/compile/.fixtures");

let valid_program_dir = _fixture("valid_program");
let invalid_program_dir = _fixture("invalid_program");
let cyclic_imports_dir = _fixture("cyclic_imports");

module Namespace = {
  open Reference.Namespace;

  let entry = Internal("entry");
  let other = Internal("other");
  let foo = Internal("foo");
  let bar = Internal("bar");
};

module Compiler = {
  module Compiler = Compile.Compiler;

  let config =
    Compiler.{
      name: "foo",
      root_dir: valid_program_dir,
      source_dir: ".",
      fail_fast: true,
      log_imports: false,
      stdlib: "stdlib.kd",
      ambient: "ambient.kd",
    };

  let cyclic_config = {...config, root_dir: cyclic_imports_dir};
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
      ExportKind.Named,
      U.as_untyped(~range=Range.create((1, 7), (1, 9)), "ABC"),
      123L
      |> A.of_int
      |> A.of_prim
      |> U.as_typed(
           ~range=Range.create((1, 13), (1, 15)),
           T.Valid(Integer),
         )
      |> A.of_const
      |> U.as_typed(
           ~range=Range.create((1, 13), (1, 15)),
           T.Valid(Integer),
         ),
    )
    |> A.of_export
    |> U.as_untyped(~range=Range.create((1, 1), (1, 15))),
  ];

  let import_and_const = [
    (
      N.entry,
      None,
      [
        (
          "ABC" |> U.as_untyped(~range=Range.create((1, 10), (1, 12))),
          None,
        )
        |> U.as_untyped(~range=Range.create((1, 10), (1, 12))),
      ],
    )
    |> A.of_import
    |> U.as_untyped(~range=Range.create((1, 1), (1, 29))),
    (
      ExportKind.Named,
      U.as_untyped(~range=Range.create((3, 7), (3, 9)), "BAR"),
      "bar"
      |> A.of_string
      |> A.of_prim
      |> U.as_typed(
           ~range=Range.create((3, 13), (3, 17)),
           T.Valid(String),
         )
      |> A.of_const
      |> U.as_typed(
           ~range=Range.create((3, 13), (3, 17)),
           T.Valid(String),
         ),
    )
    |> A.of_export
    |> U.as_untyped(~range=Range.create((3, 1), (3, 17))),
  ];

  let single_import = [
    (
      N.bar,
      "foo"
      |> U.as_untyped(~range=Range.create((2, 10), (2, 12)))
      |> Option.some,
      [],
    )
    |> A.of_import
    |> U.as_untyped(~range=Range.create((2, 3), (2, 25))),
    (
      ExportKind.Named,
      U.as_untyped(~range=Range.create((4, 9), (4, 11)), "ABC"),
      123L
      |> A.of_int
      |> A.of_prim
      |> U.as_typed(
           ~range=Range.create((4, 15), (4, 17)),
           T.Valid(Integer),
         )
      |> A.of_const
      |> U.as_typed(
           ~range=Range.create((4, 15), (4, 17)),
           T.Valid(Integer),
         ),
    )
    |> A.of_export
    |> U.as_untyped(~range=Range.create((4, 3), (4, 17))),
  ];

  let invalid_foo = [
    (
      N.bar,
      None,
      [
        (
          "BAR" |> U.as_untyped(~range=Range.create((1, 10), (1, 12))),
          None,
        )
        |> U.as_untyped(~range=Range.create((1, 10), (1, 12))),
      ],
    )
    |> A.of_import
    |> U.as_untyped(~range=Range.create((1, 1), (1, 27))),
    (
      ExportKind.Named,
      U.as_untyped(~range=Range.create((3, 7), (3, 11)), "const"),
      "foo"
      |> A.of_string
      |> A.of_prim
      |> U.as_typed(
           ~range=Range.create((3, 15), (3, 19)),
           T.Valid(String),
         )
      |> A.of_const
      |> U.as_typed(
           ~range=Range.create((3, 15), (3, 19)),
           T.Valid(String),
         ),
    )
    |> A.of_export
    |> U.as_untyped(~range=Range.create((3, 1), (3, 19))),
  ];
};

module Output = {
  let const_int = "var $knot = require(\"@knot/runtime\");
var ABC = 123;
exports.ABC = ABC;
";

  let import_and_const = "var $knot = require(\"@knot/runtime\");
var $import$_$entry = require(\"./entry\");
var ABC = $import$_$entry.ABC;
$import$_$entry = null;
var BAR = \"bar\";
exports.BAR = BAR;
";

  let single_import = "var $knot = require(\"@knot/runtime\");
var $import$_$bar = require(\"./bar\");
var foo = $import$_$bar.main;
$import$_$bar = null;
var ABC = 123;
exports.ABC = ABC;
";
};
