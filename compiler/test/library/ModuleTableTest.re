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

let _create_table = items => items |> List.to_seq |> Hashtbl.of_seq;

let suite =
  "Compile.ModuleTable"
  >::: [
    "add()"
    >: (
      () => {
        __table
        |> ModuleTable.(
             add(
               __id,
               Valid(
                 "foo",
                 {
                   ast: __program,
                   exports: _create_table(__types),
                   scopes: __scope_tree,
                 },
               ),
             )
           );

        Assert.module_table(
          _create_table([
            (
              __id,
              ModuleTable.Valid(
                "foo",
                {
                  exports:
                    _create_table([
                      (Export.Named("bar"), Type.Valid(`Element)),
                    ]),
                  ast: __program,
                  scopes: __scope_tree,
                },
              ),
            ),
          ]),
          __table,
        );
      }
    ),
  ];
