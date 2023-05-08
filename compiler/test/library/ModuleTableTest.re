open Kore;
open Reference;
open ModuleAliases;

module U = Util.RawUtil;
module ModuleTable = AST.ModuleTable;
module SymbolTable = AST.SymbolTable;
module ModuleStatement = KModuleStatement.Interface;
module Type = AST.Type;

let __id = Namespace.Internal("foo");
let __types: list((Export.t, Type.t)) = [(Named("bar"), Valid(Element))];
let __program = [
  (__id, "bar" |> U.as_untyped |> Option.some, [])
  |> ModuleStatement.of_import
  |> U.as_untyped,
];
let __table = ModuleTable.create(1);
let __scope_tree = BinaryTree.create((Range.zero, None));

let _create_table = items =>
  ModuleTable.{
    modules: items |> List.to_seq |> Hashtbl.of_seq,
    plugins: [],
    globals: [],
  };

let _create_module =
    (exports: list((Export.t, Type.t)))
    : ModuleTable.module_t(Language.Interface.program_t(Type.t)) => {
  ast: __program,
  scopes: __scope_tree,
  symbols: SymbolTable.of_export_list(exports),
};

let suite =
  "Compile.ModuleTable"
  >::: [
    "add()"
    >: (
      () => {
        __table
        |> ModuleTable.(add(__id, Valid("foo", _create_module(__types))));

        Assert.module_table(
          _create_table([
            (
              __id,
              ModuleTable.Valid(
                "foo",
                _create_module([
                  (Export.Named("bar"), Type.Valid(Element)),
                ]),
              ),
            ),
          ]),
          __table,
        );
      }
    ),
  ];
