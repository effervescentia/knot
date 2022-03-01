open Kore;
open Reference;

module Program = Grammar.Program;
module U = Util.ResultUtil;

module Target = {
  type t = A.program_t;

  let parser = ((ctx, _)) => ctx |> Program.main |> Parser.parse;

  let test =
    Alcotest.(
      check(
        list(
          testable(
            ppf => A.Dump.(mod_stmt_to_entity % Entity.pp(ppf)),
            (==),
          ),
        ),
        "program matches",
      )
    );
};

module AssertImports =
  Assert.Make({
    include Target;

    let parser = ((ctx, _)) => ctx |> Program.imports |> Parser.parse;
  });
module Assert = Assert.Make(Target);

let __main_import = "import foo from \"@/bar\"";
let __const_decl = "const foo = nil";
let __scope_tree = BinaryTree.create((Range.zero, None));

let __main_import_ast =
  (
    "bar" |> A.of_internal,
    [
      "foo" |> A.of_public |> U.as_raw_node |> A.of_main_import |> U.as_raw_node,
    ],
  )
  |> A.of_import
  |> U.as_raw_node;
let __const_decl_ast =
  (
    "foo" |> A.of_public |> U.as_raw_node |> A.of_named_export,
    U.nil_prim |> A.of_const |> U.as_nil,
  )
  |> A.of_decl
  |> U.as_raw_node;

let __ns_context =
  NamespaceContext.create(
    ~modules=
      [
        (
          "bar" |> A.of_internal,
          ModuleTable.{
            ast: [],
            exports:
              [(Export.Main, Type.Valid(`String))]
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
  "Grammar.Program"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse import"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__ns_context,
          [__main_import_ast],
          __main_import,
        )
    ),
    "parse single declaration"
    >: (() => Assert.parse([__const_decl_ast], __const_decl)),
    "parse multiple declarations"
    >: (
      () =>
        Assert.parse(
          [
            __const_decl_ast,
            (
              "bar" |> A.of_public |> U.as_raw_node |> A.of_named_export,
              "foo"
              |> A.of_public
              |> A.of_id
              |> U.as_nil
              |> A.of_const
              |> U.as_nil,
            )
            |> A.of_decl
            |> U.as_raw_node,
          ],
          __const_decl ++ "; const bar = foo",
        )
    ),
    "parse import and declaration"
    >: (
      () =>
        Assert.parse(
          ~ns_context=__ns_context,
          [__main_import_ast, __const_decl_ast],
          Fmt.str("%s; %s", __main_import, __const_decl),
        )
    ),
    "parse import with dependent declaration"
    >: (
      () =>
        Assert.parse(
          ~ns_context=
            NamespaceContext.create(
              ~modules=
                [
                  (
                    "bar" |> A.of_internal,
                    ModuleTable.{
                      ast: [],
                      exports:
                        [(Export.Main, Type.Valid(`Boolean))]
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
            ),
          [
            __main_import_ast,
            (
              "bar" |> A.of_public |> U.as_raw_node |> A.of_named_export,
              "foo"
              |> A.of_public
              |> A.of_id
              |> U.as_bool
              |> A.of_const
              |> U.as_nil,
            )
            |> A.of_decl
            |> U.as_raw_node,
          ],
          __main_import ++ "; const bar = foo",
        )
    ),
    "parse imports only"
    >: (
      () =>
        AssertImports.parse(
          ~ns_context=__ns_context,
          [__main_import_ast],
          __main_import |> Fmt.str("%s; gibberish"),
        )
    ),
  ];
