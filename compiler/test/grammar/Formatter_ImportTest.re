open Kore;

module A = AST.Result;
module Formatter = Language.Formatter;
module Namespace = Reference.Namespace;
module U = Util.ResultUtil;

let _main_import = (name, f) => (
  name |> f,
  Some(String.capitalize_ascii(name)),
  [],
);

let _assert_import = (expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(KImport.Plugin.format));

let suite =
  "Grammar.Formatter | Import"
  >::: [
    "pp_import() - main import"
    >: (
      () =>
        _assert_import(
          "import Fizz from \"buzz\";",
          (Namespace.External("buzz"), Some("Fizz"), []),
        )
    ),
    "pp_import() - named imports"
    >: (
      () =>
        _assert_import(
          "import { Foo as foo, Bar } from \"buzz\";",
          (
            Namespace.External("buzz"),
            None,
            [("Foo", Some("foo")), ("Bar", None)],
          ),
        )
    ),
    "pp_import() - main and named imports"
    >: (
      () =>
        _assert_import(
          "import Fizz, { Foo as foo, Bar } from \"buzz\";",
          (
            Namespace.External("buzz"),
            Some("Fizz"),
            [("Foo", Some("foo")), ("Bar", None)],
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
            Namespace.External("buzz"),
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
            |> List.map(word => (word, None)),
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
            Namespace.External("buzz"),
            Some("Foo"),
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
            |> List.map(word => (word, None)),
          ),
        )
    ),
    "pp_all_imports() - standard only"
    >: (
      () =>
        Assert.string(
          "import {
  JSX,
};",
          ([("JSX", None)], [], [])
          |> ~@Fmt.root(Language.Formatter.format_all_imports),
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
            [],
            [
              _main_import("foo", Namespace.of_external),
              _main_import("bar", Namespace.of_external),
            ],
          )
          |> ~@Fmt.root(Language.Formatter.format_all_imports),
        )
    ),
    "pp_all_imports() - internal only"
    >: (
      () =>
        Assert.string(
          "import Foo from \"@/foo\";
import Bar from \"@/bar\";",
          (
            [],
            [
              _main_import("foo", Namespace.of_internal),
              _main_import("bar", Namespace.of_internal),
            ],
            [],
          )
          |> ~@Fmt.root(Language.Formatter.format_all_imports),
        )
    ),
    "pp_all_imports() - external and internal"
    >: (
      () =>
        Assert.string(
          "import Bar from \"bar\";

import Foo from \"@/foo\";",
          (
            [],
            [_main_import("bar", Namespace.of_external)],
            [_main_import("foo", Namespace.of_internal)],
          )
          |> ~@Fmt.root(Language.Formatter.format_all_imports),
        )
    ),
    "pp_all_imports() - all types"
    >: (
      () =>
        Assert.string(
          "import {
  JSX, JSX as Other,
};

import Bar from \"bar\";

import Foo from \"@/foo\";",
          (
            [("JSX", None), ("JSX", Some("Other"))],
            [_main_import("bar", Namespace.of_external)],
            [_main_import("foo", Namespace.of_internal)],
          )
          |> ~@Fmt.root(Language.Formatter.format_all_imports),
        )
    ),
  ];
