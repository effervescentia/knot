open Kore;

module Import = Grammar.Import;
module Export = Reference.Export;
module U = Util.CommonUtil;

module Assert =
  Assert.Make({
    type t = A.module_statement_t;

    let parser = ((_, ctx)) =>
      Import.parser(ctx) |> Assert.parse_completely |> Parser.parse;

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

let __context_with_named_exports =
  NamespaceContext.create(
    ~modules=
      [
        (
          "bar" |> A.of_internal,
          ModuleTable.Valid(
            "foo",
            {
              ast: [],
              exports:
                [
                  (Export.Main, Type.Valid(`Nil)),
                  (
                    Export.Named("bar" |> A.of_public),
                    Type.Valid(`Boolean),
                  ),
                  (Export.Named("foo" |> A.of_public), Type.Valid(`String)),
                ]
                |> List.to_seq
                |> Hashtbl.of_seq,
              scopes: __scope_tree,
            },
          ),
        ),
      ]
      |> List.to_seq
      |> Hashtbl.of_seq,
    Internal("mock"),
  );

let __context_with_main_export =
  NamespaceContext.create(
    ~modules=
      [
        (
          "bar" |> A.of_internal,
          ModuleTable.Valid(
            "foo",
            {
              ast: [],
              exports:
                [(Export.Main, Type.Valid(`Nil))]
                |> List.to_seq
                |> Hashtbl.of_seq,
              scopes: __scope_tree,
            },
          ),
        ),
      ]
      |> List.to_seq
      |> Hashtbl.of_seq,
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
          ~ns_context=__context_with_named_exports,
          (
            "bar" |> A.of_internal,
            [
              "foo"
              |> A.of_public
              |> U.as_raw_node
              |> A.of_main_import
              |> U.as_raw_node,
            ],
          )
          |> A.of_import
          |> U.as_raw_node,
          "import foo from \"@/bar\"",
        )
    ),
    "parse - empty named imports"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__context_with_named_exports,
          ("bar" |> A.of_internal, []) |> A.of_import |> U.as_raw_node,
          "import {} from \"@/bar\"",
        )
    ),
    "parse - named import"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__context_with_named_exports,
          (
            "bar" |> A.of_internal,
            [
              ("foo" |> A.of_public |> U.as_raw_node, None)
              |> A.of_named_import
              |> U.as_raw_node,
            ],
          )
          |> A.of_import
          |> U.as_raw_node,
          "import { foo } from \"@/bar\"",
        )
    ),
    "parse - named import with alias"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__context_with_named_exports,
          (
            "bar" |> A.of_internal,
            [
              (
                "foo" |> A.of_public |> U.as_raw_node,
                Some("bar" |> A.of_public |> U.as_raw_node),
              )
              |> A.of_named_import
              |> U.as_raw_node,
            ],
          )
          |> A.of_import
          |> U.as_raw_node,
          "import { foo as bar } from \"@/bar\"",
        )
    ),
    "parse - main and named imports"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__context_with_named_exports,
          (
            "bar" |> A.of_internal,
            [
              "fizz"
              |> A.of_public
              |> U.as_raw_node
              |> A.of_main_import
              |> U.as_raw_node,
              ("foo" |> A.of_public |> U.as_raw_node, None)
              |> A.of_named_import
              |> U.as_raw_node,
              (
                "bar" |> A.of_public |> U.as_raw_node,
                Some("Bar" |> A.of_public |> U.as_raw_node),
              )
              |> A.of_named_import
              |> U.as_raw_node,
            ],
          )
          |> A.of_import
          |> U.as_raw_node,
          "import fizz, { foo, bar as Bar } from \"@/bar\"",
        )
    ),
    "parse - trailing comma in named import list"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__context_with_named_exports,
          (
            "bar" |> A.of_internal,
            [
              ("foo" |> A.of_public |> U.as_raw_node, None)
              |> A.of_named_import
              |> U.as_raw_node,
              ("bar" |> A.of_public |> U.as_raw_node, None)
              |> A.of_named_import
              |> U.as_raw_node,
            ],
          )
          |> A.of_import
          |> U.as_raw_node,
          "import { foo, bar, } from \"@/bar\"",
        )
    ),
    "parse terminated"
    >: (
      () =>
        Assert.parse_all(
          ~ns_context=__context_with_main_export,
          (
            "bar" |> A.of_internal,
            [
              "foo"
              |> A.of_public
              |> U.as_raw_node
              |> A.of_main_import
              |> U.as_raw_node,
            ],
          )
          |> A.of_import
          |> U.as_raw_node,
          [
            "import foo from \"@/bar\";",
            "  import  foo  from   \"@/bar\"  ;  ",
          ],
        )
    ),
  ];
