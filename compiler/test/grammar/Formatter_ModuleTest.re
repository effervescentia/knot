open Kore;

module Formatter = Grammar.Formatter;
module U = Util.ResultUtil;

let __int_const = ("ABC" |> A.of_public, 123 |> U.int_prim |> A.of_const);

let __int_const_stmt =
  (
    "ABC" |> A.of_public |> U.as_untyped |> A.of_named_export,
    123 |> U.int_prim |> A.of_const |> U.as_int,
  )
  |> A.of_decl;

let __import_stmt =
  (
    "bar" |> A.of_external,
    [
      "Foo" |> A.of_public |> U.as_untyped |> A.of_main_import |> U.as_untyped,
    ],
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
              |> A.of_public
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
                ("d" |> A.of_public |> U.as_untyped, None)
                |> A.of_named_import
                |> U.as_untyped,
                ("c" |> A.of_public |> U.as_untyped, None)
                |> A.of_named_import
                |> U.as_untyped,
                ("b" |> A.of_public |> U.as_untyped, None)
                |> A.of_named_import
                |> U.as_untyped,
                ("a" |> A.of_public |> U.as_untyped, None)
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
