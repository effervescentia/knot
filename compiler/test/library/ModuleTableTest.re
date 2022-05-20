open Kore;
open Reference;
open ModuleAliases;

module U = Util.RawUtil;

let __id = Namespace.Internal("foo");
let __types: list((Export.t, Type.t)) = [
  (Named(A.of_public("bar")), Valid(`Element)),
];
let __program = [
  (
    "foo" |> A.of_internal,
    [
      "bar" |> A.of_public |> U.as_raw_node |> A.of_main_import |> U.as_raw_node,
    ],
  )
  |> A.of_import
  |> U.as_raw_node,
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
        |> ModuleTable.add(__id, __program, __types, __scope_tree, "foo");

        Assert.module_table(
          _create_table([
            (
              __id,
              ModuleTable.Valid({
                exports:
                  _create_table([
                    (
                      Export.Named(A.of_public("bar")),
                      Type.Valid(`Element),
                    ),
                  ]),
                ast: __program,
                scopes: __scope_tree,
                raw: "foo",
              }),
            ),
          ]),
          __table,
        );
      }
    ),
    "add_type() - add type to existing module"
    >: (
      () => {
        __table |> ModuleTable.add(__id, __program, [], __scope_tree, "foo");
        __table
        |> ModuleTable.add_type(
             (__id, Export.Named(A.of_public("new_type"))),
             Valid(`Float),
           );

        Assert.module_table(
          _create_table([
            (
              __id,
              ModuleTable.Valid({
                exports:
                  _create_table([
                    (
                      Export.Named(A.of_public("new_type")),
                      Type.Valid(`Float),
                    ),
                  ]),
                ast: __program,
                scopes: __scope_tree,
                raw: "foo",
              }),
            ),
          ]),
          __table,
        );
      }
    ),
    "add_type() - add type to an unknown module"
    >: (
      () => {
        let original_table = Hashtbl.copy(__table);
        __table
        |> ModuleTable.add_type(
             (__id, Named(A.of_public("new_type"))),
             Valid(`Float),
           );

        Assert.module_table(original_table, __table);
      }
    ),
  ];
