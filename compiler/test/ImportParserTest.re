open Core;

let main_import_stmt = (name, module_name) =>
  [Keyword(Import), Identifier(name), Keyword(From), String(module_name)]
  |> Util.drift;

let test_parse_import = Util.test_parse_stmt(KnotParse.Import.stmt);

let __main_import = "MyImport";
let __module = "MyModule";

let tests =
  "KnotParse.Import"
  >::: [
    "parse main import"
    >:: (
      _ => {
        let stmt = main_import_stmt(__main_import, __module);
        let expected = Import(__module, [MainExport(__main_import)]);

        Util.test_many(
          test_parse_import,
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
      }
    ),
  ];
