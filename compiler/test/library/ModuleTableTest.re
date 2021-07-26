open Kore;
open Util.RawUtil;
open Reference;

let __id = Namespace.Internal("foo");
let __types: list((Export.t, Type.t)) = [
  (Named(AST.of_public("bar")), Valid(`Abstract(Unknown))),
];
let __program =
  AST.[
    Import(
      "foo" |> of_internal,
      ["bar" |> of_public |> as_lexeme |> of_main_import |> as_lexeme],
    )
    |> as_lexeme,
  ];
let __table = ModuleTable.create(1);
let __scope_tree = BinaryTree.create((Cursor.(expand(zero)), None));

let _create_table = items => items |> List.to_seq |> Hashtbl.of_seq;

let suite =
  "Compile.ModuleTable"
  >::: [
    "add()"
    >: (
      () => {
        __table
        |> ModuleTable.add(__id, __program, __types, __scope_tree, "foo");

        [
          (
            _create_table([
              (
                __id,
                ModuleTable.{
                  types:
                    _create_table(
                      AST.[
                        (
                          Export.Named("bar" |> of_public),
                          Type.Valid(`Abstract(Unknown)),
                        ),
                      ],
                    ),
                  ast: __program,
                  scopes: __scope_tree,
                  raw: "foo",
                },
              ),
            ]),
            __table,
          ),
        ]
        |> Assert.(test_many(module_table));
      }
    ),
    "add_type() - add type to existing module"
    >: (
      () => {
        __table |> ModuleTable.add(__id, __program, [], __scope_tree, "foo");
        __table
        |> ModuleTable.add_type(
             (__id, Export.Named("new_type" |> AST.of_public)),
             Valid(`Float),
           );

        [
          (
            _create_table([
              (
                __id,
                ModuleTable.{
                  types:
                    _create_table([
                      (
                        Export.Named("new_type" |> AST.of_public),
                        Type.Valid(`Float),
                      ),
                    ]),
                  ast: __program,
                  scopes: __scope_tree,
                  raw: "foo",
                },
              ),
            ]),
            __table,
          ),
        ]
        |> Assert.(test_many(module_table));
      }
    ),
    "add_type() - add type to an unknown module"
    >: (
      () => {
        let original_table = Hashtbl.copy(__table);
        __table
        |> ModuleTable.add_type(
             (__id, Named(AST.of_public("new_type"))),
             Valid(`Float),
           );

        [(original_table, __table)] |> Assert.(test_many(module_table));
      }
    ),
  ];
