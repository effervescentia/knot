open Kore;
open Reference;

module ModuleTable = AST.ModuleTable;
module Program = Language.Program;
module U = Util.ResultUtil;

let dump_program = ppf =>
  Language.Debug.program_to_xml(~@Type.pp) % Pretty.XML.xml(Fmt.string, ppf);

module Target = {
  type t = Language.Interface.program_t(Type.t);

  let parser = Program.main % Parser.parse;

  let test =
    Alcotest.(check(testable(dump_program, (==)), "program matches"));
};

module AssertImports =
  Assert.Make({
    include Target;

    let parser = Program.imports % Parser.parse;
  });
module Assert = Assert.Make(Target);

let __main_import = "import foo from \"@/bar\"";
let __const_decl = "const foo = nil";
let __scope_tree = BinaryTree.create((Range.zero, None));

let __main_import_ast =
  (Namespace.Internal("bar"), "foo" |> U.as_untyped |> Option.some, [])
  |> ModuleStatement.of_import
  |> U.as_untyped;
let __const_decl_ast =
  (
    ExportKind.Named,
    U.as_untyped("foo"),
    U.nil_prim |> Declaration.of_constant |> U.as_nil,
  )
  |> ModuleStatement.of_export
  |> U.as_untyped;

let _create_module =
    (exports: list((Export.t, Type.t)))
    : ModuleTable.module_t(Language.Interface.program_t(Type.t)) => {
  ast: [],
  scopes: __scope_tree,
  symbols: AST.SymbolTable.of_export_list(exports),
};

let _create_module_table = modules =>
  ModuleTable.{
    modules: modules |> List.to_seq |> Hashtbl.of_seq,
    plugins: [],
    globals: [],
  };

let __context =
  ParseContext.create(
    ~modules=
      [
        (
          Namespace.Internal("bar"),
          ModuleTable.Valid(
            "foo",
            _create_module([(Export.Main, Type.Valid(String))]),
          ),
        ),
      ]
      |> _create_module_table,
    Internal("mock"),
  );

let suite =
  "Grammar.Program"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse import"
    >: (
      () =>
        Assert.parse(~context=__context, [__main_import_ast], __main_import)
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
              ExportKind.Named,
              "bar" |> U.as_untyped,
              "foo"
              |> Expression.of_identifier
              |> U.as_nil
              |> Declaration.of_constant
              |> U.as_nil,
            )
            |> ModuleStatement.of_export
            |> U.as_untyped,
          ],
          __const_decl ++ "; const bar = foo",
        )
    ),
    "parse import and declaration"
    >: (
      () =>
        Assert.parse(
          ~context=__context,
          [__main_import_ast, __const_decl_ast],
          Fmt.str("%s; %s", __main_import, __const_decl),
        )
    ),
    "parse import with dependent declaration"
    >: (
      () =>
        Assert.parse(
          ~context=
            ParseContext.create(
              ~modules=
                [
                  (
                    Namespace.Internal("bar"),
                    ModuleTable.Valid(
                      "foo",
                      _create_module([(Export.Main, Type.Valid(Boolean))]),
                    ),
                  ),
                ]
                |> _create_module_table,
              Internal("mock"),
            ),
          [
            __main_import_ast,
            (
              ExportKind.Named,
              U.as_untyped("bar"),
              "foo"
              |> Expression.of_identifier
              |> U.as_bool
              |> Declaration.of_constant
              |> U.as_bool,
            )
            |> ModuleStatement.of_export
            |> U.as_untyped,
          ],
          __main_import ++ "; const bar = foo",
        )
    ),
    "parse imports only"
    >: (
      () =>
        AssertImports.parse(
          ~context=__context,
          [__main_import_ast],
          __main_import |> Fmt.str("%s; gibberish"),
        )
    ),
  ];
