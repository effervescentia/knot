open Kore;

module Formatter = Language.Formatter;
module U = Util.ResultUtil;

let __int_const = ("ABC", 123 |> U.int_prim |> A.of_const);

let __int_const_stmt =
  (
    "ABC" |> U.as_untyped |> A.of_named_export,
    123 |> U.int_prim |> A.of_const |> U.as_int,
  )
  |> A.of_decl;

let __import_stmt =
  (
    "bar" |> A.of_external,
    ["Foo" |> U.as_untyped |> A.of_main_import |> U.as_untyped],
  )
  |> A.of_import;

let suite =
  "Grammar.Formatter | Module"
  >::: [
    "empty"
    >: (
      () =>
        Assert.string(
          "\n",
          [] |> List.map(U.as_untyped) |> ~@Formatter.format,
        )
    ),
    "import only"
    >: (
      () =>
        Assert.string(
          "import Foo from \"bar\";\n",
          [__import_stmt] |> List.map(U.as_untyped) |> ~@Formatter.format,
        )
    ),
    "declaration only"
    >: (
      () =>
        Assert.string(
          "const ABC = 123;\n",
          [__int_const_stmt] |> List.map(U.as_untyped) |> ~@Formatter.format,
        )
    ),
    "import and declaration"
    >: (
      () =>
        Assert.string(
          "import Foo from \"bar\";

const ABC = 123;\n",
          [__import_stmt, __int_const_stmt]
          |> List.map(U.as_untyped)
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
              |> U.as_untyped
              |> A.of_main_import
              |> U.as_untyped,
            ],
          )
          |> A.of_import;

        Assert.string(
          "import Bar from \"bar\";
import Foo from \"foo\";

import Buzz from \"@/buzz\";
import Fizz from \"@/fizz\";\n",
          [
            _main_import("buzz", A.of_internal),
            _main_import("bar", A.of_external),
            _main_import("fizz", A.of_internal),
            _main_import("foo", A.of_external),
          ]
          |> List.map(U.as_untyped)
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
              "foo" |> A.of_external,
              [
                (U.as_untyped("d"), None) |> A.of_named_import |> U.as_untyped,
                (U.as_untyped("c"), None)
                |> A.of_named_import
                |> U.as_untyped,
                (U.as_untyped("b"), None)
                |> A.of_named_import
                |> U.as_untyped,
                (U.as_untyped("a"), None)
                |> A.of_named_import
                |> U.as_untyped,
              ],
            )
            |> A.of_import,
          ]
          |> List.map(U.as_untyped)
          |> ~@Formatter.format,
        )
    ),
  ];
