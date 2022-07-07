open Kore;
open Reference;
open ModuleAliases;

module U = Util.RawUtil;

let __id = Namespace.Internal("foo");
let __types: list((Export.t, Type.t)) = [
  (Named("bar"), Valid(`Element)),
];
let __program = [
  (
    "foo" |> A.of_internal,
    ["bar" |> U.as_untyped |> A.of_main_import |> U.as_untyped],
  )
  |> A.of_import
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
    (exports: list((Export.t, Type.t))): ModuleTable.module_t => {
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
                  (Export.Named("bar"), Type.Valid(`Element)),
                ]),
              ),
            ),
          ]),
          __table,
        );
      }
    ),
  ];
