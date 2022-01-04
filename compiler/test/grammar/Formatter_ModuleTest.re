open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Formatter = Grammar.Formatter;

let __int_const = ("ABC" |> of_public, 123 |> int_prim |> of_const);

let __int_const_stmt =
  (
    "ABC" |> of_public |> as_raw_node |> of_named_export,
    123 |> int_prim |> of_const |> as_int,
  )
  |> of_decl;

let __import_stmt =
  (
    "bar" |> of_external,
    ["Foo" |> of_public |> as_raw_node |> of_main_import |> as_raw_node],
  )
  |> of_import;

let suite =
  "Grammar.Formatter | Module"
  >::: [
    "empty"
    >: (
      () =>
        Assert.string(
          "\n",
          [] |> List.map(as_raw_node) |> ~@Formatter.format,
        )
    ),
    "import only"
    >: (
      () =>
        Assert.string(
          "import Foo from \"bar\";\n",
          [__import_stmt] |> List.map(as_raw_node) |> ~@Formatter.format,
        )
    ),
    "declaration only"
    >: (
      () =>
        Assert.string(
          "const ABC = 123;\n",
          [__int_const_stmt] |> List.map(as_raw_node) |> ~@Formatter.format,
        )
    ),
    "import and declaration"
    >: (
      () =>
        Assert.string(
          "import Foo from \"bar\";

const ABC = 123;\n",
          [__import_stmt, __int_const_stmt]
          |> List.map(as_raw_node)
          |> ~@Formatter.format,
        )
    ),
    "sort import statements"
    >: (
      () => {
        let _main_import = (name, f) =>
          (
            name |> f,
            [
              name
              |> String.capitalize_ascii
              |> of_public
              |> as_raw_node
              |> of_main_import
              |> as_raw_node,
            ],
          )
          |> of_import;

        Assert.string(
          "import Bar from \"bar\";
import Foo from \"foo\";

import Buzz from \"@/buzz\";
import Fizz from \"@/fizz\";\n",
          [
            _main_import("buzz", of_internal),
            _main_import("bar", of_external),
            _main_import("fizz", of_internal),
            _main_import("foo", of_external),
          ]
          |> List.map(as_raw_node)
          |> ~@Formatter.format,
        );
      }
    ),
    "sort named imports"
    >: (
      () =>
        Assert.string(
          "import { a, b, c, d } from \"foo\";\n",
          [
            (
              "foo" |> of_external,
              [
                ("d" |> of_public |> as_raw_node, None)
                |> of_named_import
                |> as_raw_node,
                ("c" |> of_public |> as_raw_node, None)
                |> of_named_import
                |> as_raw_node,
                ("b" |> of_public |> as_raw_node, None)
                |> of_named_import
                |> as_raw_node,
                ("a" |> of_public |> as_raw_node, None)
                |> of_named_import
                |> as_raw_node,
              ],
            )
            |> of_import,
          ]
          |> List.map(as_raw_node)
          |> ~@Formatter.format,
        )
    ),
  ];
