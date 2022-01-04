open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Program = Grammar.Program;

module Target = {
  type t = program_t;

  let parser = ((ctx, _)) => ctx |> Program.main |> Parser.parse;

  let test =
    Alcotest.(
      check(
        list(
          testable(
            ppf => Dump.mod_stmt_to_entity % Dump.Entity.pp(ppf),
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
    "bar" |> of_internal,
    ["foo" |> of_public |> as_raw_node |> of_main_import |> as_raw_node],
  )
  |> of_import
  |> as_raw_node;
let __const_decl_ast =
  (
    "foo" |> of_public |> as_raw_node |> of_named_export,
    nil_prim |> of_const |> as_nil,
  )
  |> of_decl
  |> as_raw_node;

let __ns_context =
  NamespaceContext.create(
    ~modules=
      [
        (
          "bar" |> of_internal,
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
              "bar" |> of_public |> as_raw_node |> of_named_export,
              "foo"
              |> of_public
              |> as_nil
              |> of_id
              |> as_nil
              |> of_const
              |> as_nil,
            )
            |> of_decl
            |> as_raw_node,
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
                    "bar" |> of_internal,
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
              "bar" |> of_public |> as_raw_node |> of_named_export,
              "foo"
              |> of_public
              |> as_bool
              |> of_id
              |> as_bool
              |> of_const
              |> as_nil,
            )
            |> of_decl
            |> as_raw_node,
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
