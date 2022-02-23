open Kore;
open Util.CommonUtil;
open Reference;

module Import = Grammar.Import;

module Assert =
  Assert.Make({
    type t = AST.module_statement_t;

    let parser = ((_, ctx)) =>
      Import.parser(ctx) |> Assert.parse_completely |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf => AST.Dump.mod_stmt_to_entity % AST.Dump.Entity.pp(ppf),
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
      AST.[
        (
          "bar" |> of_internal,
          ModuleTable.{
            ast: [],
            exports:
              [
                (Export.Main, Type.Valid(`Nil)),
                (Export.Named("bar" |> of_public), Type.Valid(`Boolean)),
                (Export.Named("foo" |> of_public), Type.Valid(`String)),
              ]
              |> List.to_seq
              |> Hashtbl.of_seq,
            scopes: __scope_tree,
            raw: "foo",
          },
        ),
      ]
      |> List.to_seq
      |> Hashtbl.of_seq,
    Internal("mock"),
  );

let __context_with_main_export =
  NamespaceContext.create(
    ~modules=
      AST.[
        (
          "bar" |> of_internal,
          ModuleTable.{
            ast: [],
            exports:
              [(Export.Main, Type.Valid(`Nil))]
              |> List.to_seq
              |> Hashtbl.of_seq,
            scopes: __scope_tree,
            raw: "foo",
          },
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
          AST.(
            (
              "bar" |> of_internal,
              [
                "foo"
                |> of_public
                |> as_raw_node
                |> of_main_import
                |> as_raw_node,
              ],
            )
            |> of_import
            |> as_raw_node
          ),
          "import foo from \"@/bar\"",
        )
    ),
    "parse - empty named imports"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__context_with_named_exports,
          AST.(("bar" |> of_internal, []) |> of_import |> as_raw_node),
          "import {} from \"@/bar\"",
        )
    ),
    "parse - named import"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__context_with_named_exports,
          AST.(
            (
              "bar" |> of_internal,
              [
                ("foo" |> of_public |> as_raw_node, None)
                |> of_named_import
                |> as_raw_node,
              ],
            )
            |> of_import
            |> as_raw_node
          ),
          "import { foo } from \"@/bar\"",
        )
    ),
    "parse - named import with alias"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__context_with_named_exports,
          AST.(
            (
              "bar" |> of_internal,
              [
                (
                  "foo" |> of_public |> as_raw_node,
                  Some("bar" |> of_public |> as_raw_node),
                )
                |> of_named_import
                |> as_raw_node,
              ],
            )
            |> of_import
            |> as_raw_node
          ),
          "import { foo as bar } from \"@/bar\"",
        )
    ),
    "parse - main and named imports"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__context_with_named_exports,
          AST.(
            (
              "bar" |> of_internal,
              [
                "fizz"
                |> of_public
                |> as_raw_node
                |> of_main_import
                |> as_raw_node,
                ("foo" |> of_public |> as_raw_node, None)
                |> of_named_import
                |> as_raw_node,
                (
                  "bar" |> of_public |> as_raw_node,
                  Some("Bar" |> of_public |> as_raw_node),
                )
                |> of_named_import
                |> as_raw_node,
              ],
            )
            |> of_import
            |> as_raw_node
          ),
          "import fizz, { foo, bar as Bar } from \"@/bar\"",
        )
    ),
    "parse - trailing comma in named import list"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__context_with_named_exports,
          AST.(
            (
              "bar" |> of_internal,
              [
                ("foo" |> of_public |> as_raw_node, None)
                |> of_named_import
                |> as_raw_node,
                ("bar" |> of_public |> as_raw_node, None)
                |> of_named_import
                |> as_raw_node,
              ],
            )
            |> of_import
            |> as_raw_node
          ),
          "import { foo, bar, } from \"@/bar\"",
        )
    ),
    "parse terminated"
    >: (
      () =>
        Assert.parse_all(
          ~ns_context=__context_with_main_export,
          AST.(
            (
              "bar" |> of_internal,
              [
                "foo"
                |> of_public
                |> as_raw_node
                |> of_main_import
                |> as_raw_node,
              ],
            )
            |> of_import
            |> as_raw_node
          ),
          [
            "import foo from \"@/bar\";",
            "  import  foo  from   \"@/bar\"  ;  ",
          ],
        )
    ),
  ];
