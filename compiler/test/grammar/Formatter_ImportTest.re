open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Formatter = Grammar.Formatter;

let _main_import = (name, f) => (
  name |> f,
  Some(name |> String.capitalize_ascii |> of_public),
  [],
);

let _assert_import = (expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(pp_import));

let suite =
  "Grammar.Formatter | Import"
  >::: [
    "pp_import() - main import"
    >: (
      () =>
        _assert_import(
          "import Fizz from \"buzz\";",
          ("buzz" |> of_external, Some("Fizz" |> of_public), []),
        )
    ),
    "pp_import() - named imports"
    >: (
      () =>
        _assert_import(
          "import { Foo as foo, Bar } from \"buzz\";",
          (
            "buzz" |> of_external,
            None,
            [
              ("Foo" |> of_public, Some("foo" |> of_public |> as_raw_node)),
              ("Bar" |> of_public, None),
            ],
          ),
        )
    ),
    "pp_import() - main and named imports"
    >: (
      () =>
        _assert_import(
          "import Fizz, { Foo as foo, Bar } from \"buzz\";",
          (
            "buzz" |> of_external,
            Some("Fizz" |> of_public),
            [
              ("Foo" |> of_public, Some("foo" |> of_public |> as_raw_node)),
              ("Bar" |> of_public, None),
            ],
          ),
        )
    ),
    "pp_import() - many named imports"
    >: (
      () =>
        _assert_import(
          "import {
  Sit,
  et,
  dolore,
  est,
  aliqua,
  in,
  fugiat,
  duis,
  officia,
  pariatur,
  fugiat,
  mollit,
  eiusmod,
  pariatur,
  magna,
} from \"buzz\";",
          (
            "buzz" |> of_external,
            None,
            [
              "Sit",
              "et",
              "dolore",
              "est",
              "aliqua",
              "in",
              "fugiat",
              "duis",
              "officia",
              "pariatur",
              "fugiat",
              "mollit",
              "eiusmod",
              "pariatur",
              "magna",
            ]
            |> List.map(word => (word |> of_public, None)),
          ),
        )
    ),
    "pp_import() - main and many named imports"
    >: (
      () =>
        _assert_import(
          "import Foo, {
  Sit,
  et,
  dolore,
  est,
  aliqua,
  in,
  fugiat,
  duis,
  officia,
  pariatur,
  fugiat,
  mollit,
  eiusmod,
  pariatur,
  magna,
} from \"buzz\";",
          (
            "buzz" |> of_external,
            Some("Foo" |> of_public),
            [
              "Sit",
              "et",
              "dolore",
              "est",
              "aliqua",
              "in",
              "fugiat",
              "duis",
              "officia",
              "pariatur",
              "fugiat",
              "mollit",
              "eiusmod",
              "pariatur",
              "magna",
            ]
            |> List.map(word => (word |> of_public, None)),
          ),
        )
    ),
    "pp_all_imports() - external only"
    >: (
      () =>
        Assert.string(
          "import Foo from \"foo\";
import Bar from \"bar\";",
          (
            [],
            [
              _main_import("foo", of_external),
              _main_import("bar", of_external),
            ],
          )
          |> ~@Fmt.root(pp_all_imports),
        )
    ),
    "pp_all_imports() - internal only"
    >: (
      () =>
        Assert.string(
          "import Foo from \"@/foo\";
import Bar from \"@/bar\";",
          (
            [
              _main_import("foo", of_internal),
              _main_import("bar", of_internal),
            ],
            [],
          )
          |> ~@Fmt.root(pp_all_imports),
        )
    ),
    "pp_all_imports() - external and internal"
    >: (
      () =>
        Assert.string(
          "import Bar from \"bar\";

import Foo from \"@/foo\";",
          (
            [_main_import("foo", of_internal)],
            [_main_import("bar", of_external)],
          )
          |> ~@Fmt.root(pp_all_imports),
        )
    ),
  ];
