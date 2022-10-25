open Kore;

module Export = Reference.Export;
module U = Util.CommonUtil;

module Assert =
  Assert.Make({
    type t = A.module_statement_t;

    let parser = KImport.Plugin.parse % Assert.parse_completely % Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf => A.Dump.(mod_stmt_to_entity % Entity.pp(ppf)),
            (==),
          ),
          "program matches",
        )
      );
  });

let __scope_tree = BinaryTree.create((Range.zero, None));

let _create_module =
    (exports: list((Export.t, Type.t))): ModuleTable.module_t => {
  ast: [],
  scopes: __scope_tree,
  symbols: SymbolTable.of_export_list(exports),
};

let _create_module_table = modules =>
  ModuleTable.{
    modules: modules |> List.to_seq |> Hashtbl.of_seq,
    plugins: [],
    globals: [],
  };

let __context_with_named_exports =
  ParseContext.create(
    ~modules=
      [
        (
          "bar" |> A.of_internal,
          ModuleTable.Valid(
            "foo",
            _create_module([
              (Export.Main, Type.Valid(`Nil)),
              (Export.Named("bar"), Type.Valid(`Boolean)),
              (Export.Named("foo"), Type.Valid(`String)),
            ]),
          ),
        ),
      ]
      |> _create_module_table,
    Internal("mock"),
  );

let __context_with_main_export =
  ParseContext.create(
    ~modules=
      [
        (
          "bar" |> A.of_internal,
          ModuleTable.Valid(
            "foo",
            _create_module([(Export.Main, Type.Valid(`Nil))]),
          ),
        ),
      ]
      |> _create_module_table,
    Internal("mock"),
  );

let suite =
  "Grammar.Import"
  >::: [
    "no parse"
    >: (
      () =>
        Assert.parse_none([
          "gibberish",
          "import",
          "import foo",
          "import foo from",
        ])
    ),
    "parse - main import"
    >: (
      () =>
        Assert.parse(
          ~context=__context_with_named_exports,
          (
            "bar" |> A.of_internal,
            ["foo" |> U.as_untyped |> A.of_main_import |> U.as_untyped],
          )
          |> A.of_import
          |> U.as_untyped,
          "import foo from \"@/bar\"",
        )
    ),
    "parse - empty named imports"
    >: (
      () =>
        Assert.parse(
          ~context=__context_with_named_exports,
          ("bar" |> A.of_internal, []) |> A.of_import |> U.as_untyped,
          "import {} from \"@/bar\"",
        )
    ),
    "parse - named import"
    >: (
      () =>
        Assert.parse(
          ~context=__context_with_named_exports,
          (
            "bar" |> A.of_internal,
            [
              (U.as_untyped("foo"), None) |> A.of_named_import |> U.as_untyped,
            ],
          )
          |> A.of_import
          |> U.as_untyped,
          "import { foo } from \"@/bar\"",
        )
    ),
    "parse - named import with alias"
    >: (
      () =>
        Assert.parse(
          ~context=__context_with_named_exports,
          (
            "bar" |> A.of_internal,
            [
              (U.as_untyped("foo"), Some(U.as_untyped("bar")))
              |> A.of_named_import
              |> U.as_untyped,
            ],
          )
          |> A.of_import
          |> U.as_untyped,
          "import { foo as bar } from \"@/bar\"",
        )
    ),
    "parse - main and named imports"
    >: (
      () =>
        Assert.parse(
          ~context=__context_with_named_exports,
          (
            "bar" |> A.of_internal,
            [
              "fizz" |> U.as_untyped |> A.of_main_import |> U.as_untyped,
              (U.as_untyped("foo"), None)
              |> A.of_named_import
              |> U.as_untyped,
              (U.as_untyped("bar"), Some(U.as_untyped("Bar")))
              |> A.of_named_import
              |> U.as_untyped,
            ],
          )
          |> A.of_import
          |> U.as_untyped,
          "import fizz, { foo, bar as Bar } from \"@/bar\"",
        )
    ),
    "parse - trailing comma in named import list"
    >: (
      () =>
        Assert.parse(
          ~context=__context_with_named_exports,
          (
            "bar" |> A.of_internal,
            [
              (U.as_untyped("foo"), None) |> A.of_named_import |> U.as_untyped,
              (U.as_untyped("bar"), None)
              |> A.of_named_import
              |> U.as_untyped,
            ],
          )
          |> A.of_import
          |> U.as_untyped,
          "import { foo, bar, } from \"@/bar\"",
        )
    ),
    "parse terminated"
    >: (
      () =>
        Assert.parse_all(
          ~context=__context_with_main_export,
          (
            "bar" |> A.of_internal,
            ["foo" |> U.as_untyped |> A.of_main_import |> U.as_untyped],
          )
          |> A.of_import
          |> U.as_untyped,
          [
            "import foo from \"@/bar\";",
            "  import  foo  from   \"@/bar\"  ;  ",
          ],
        )
    ),
    "parse - standard named import"
    >: (
      () =>
        Assert.parse(
          ~context=__context_with_named_exports,
          [(U.as_untyped("foo"), None) |> U.as_untyped]
          |> A.of_standard_import
          |> U.as_untyped,
          "import { foo }",
        )
    ),
    "parse - standard named import with alias"
    >: (
      () =>
        Assert.parse(
          ~context=__context_with_named_exports,
          [
            (U.as_untyped("foo"), Some(U.as_untyped("bar"))) |> U.as_untyped,
          ]
          |> A.of_standard_import
          |> U.as_untyped,
          "import { foo as bar }",
        )
    ),
  ];
