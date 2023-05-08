open Kore;

module Formatter = Language.Formatter;
module U = Util.ResultUtil;

let __int_const = ("ABC", 123 |> U.int_prim |> Declaration.of_constant);

let __int_const_stmt =
  (
    ExportKind.Named,
    "ABC" |> U.as_untyped,
    123 |> U.int_prim |> Declaration.of_constant |> U.as_int,
  )
  |> ModuleStatement.of_export;

let __import_stmt =
  (Namespace.External("bar"), "Foo" |> U.as_untyped |> Option.some, [])
  |> ModuleStatement.of_import;

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
            name |> String.capitalize_ascii |> U.as_untyped |> Option.some,
            [],
          )
          |> ModuleStatement.of_import;

        Assert.string(
          "import Bar from \"bar\";
import Foo from \"foo\";

import Buzz from \"@/buzz\";
import Fizz from \"@/fizz\";\n",
          [
            _main_import("buzz", Namespace.of_internal),
            _main_import("bar", Namespace.of_external),
            _main_import("fizz", Namespace.of_internal),
            _main_import("foo", Namespace.of_external),
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
              Namespace.External("foo"),
              None,
              [
                (U.as_untyped("d"), None) |> U.as_untyped,
                (U.as_untyped("c"), None) |> U.as_untyped,
                (U.as_untyped("b"), None) |> U.as_untyped,
                (U.as_untyped("a"), None) |> U.as_untyped,
              ],
            )
            |> ModuleStatement.of_import,
          ]
          |> List.map(U.as_untyped)
          |> ~@Formatter.format,
        )
    ),
  ];
