open Kore;
open Util;

module ModuleTable = Compile.ModuleTable;

let __id = Reference.Namespace.Internal("foo");
let __types = [("bar", Type.K_Weak(0))];
let __program =
  AST.[
    Import(
      "foo" |> of_internal,
      ["bar" |> of_public |> as_lexeme |> of_main],
    ),
  ];
let __table = ModuleTable.create(1);

let _create_table = items => List.to_seq(items) |> Hashtbl.of_seq;

let suite =
  "Compile.ModuleTable"
  >::: [
    "add()"
    >: (
      () => {
        __table |> ModuleTable.add(__id, __program, __types);

        [
          (
            _create_table([
              (
                __id,
                ModuleTable.{
                  types: _create_table([("bar", Type.K_Weak(0))]),
                  ast: __program,
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
        __table |> ModuleTable.add(__id, __program, []);
        __table
        |> ModuleTable.add_type((__id, "new_type"), Type.K_Strong(K_Float));

        [
          (
            _create_table([
              (
                __id,
                ModuleTable.{
                  types:
                    _create_table([("new_type", Type.K_Strong(K_Float))]),
                  ast: __program,
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
        |> ModuleTable.add_type((__id, "new_type"), Type.K_Strong(K_Float));

        [(original_table, __table)] |> Assert.(test_many(module_table));
      }
    ),
  ];
