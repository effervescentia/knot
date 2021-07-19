open Kore;
open AST.Final.Util;
open Reference;
open Util;

let __id = Namespace.Internal("foo");
let __types = AST.[(Export.Named("bar" |> to_public), Type.K_Weak(0))];
let __program =
  AST.Final.[
    Import(
      "foo" |> to_internal,
      ["bar" |> to_public |> as_lexeme |> to_main_import],
    ),
  ];
let __table = ModuleTable.create(1);

let __scope_tree = BinaryTree.create((Cursor.zero |> Cursor.expand, None));

let _create_table = items => List.to_seq(items) |> Hashtbl.of_seq;

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
                        (Export.Named("bar" |> to_public), Type.K_Weak(0)),
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
             (__id, Export.Named("new_type" |> to_public)),
             Type.K_Strong(K_Float),
           );

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
                          Export.Named("new_type" |> to_public),
                          Type.K_Strong(K_Float),
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
    "add_type() - add type to an unknown module"
    >: (
      () => {
        let original_table = Hashtbl.copy(__table);
        __table
        |> ModuleTable.add_type(
             (__id, Export.Named("new_type" |> to_public)),
             Type.K_Strong(K_Float),
           );

        [(original_table, __table)] |> Assert.(test_many(module_table));
      }
    ),
  ];
