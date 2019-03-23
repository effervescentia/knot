open Core;

let test_parse_import = Util.test_parse_stmt(KnotParse.Import.stmt);

let __import_name = "MyImport";
let __module = "MyModule";

let tests =
  "KnotParse.Import"
  >::: [
    "parse main import"
    >:: (
      _ => {
        let stmt =
          [
            Keyword(Import),
            Identifier(__import_name),
            Keyword(From),
            String(__module),
          ]
          |> Util.drift;
        let expected =
          Import(__module, [no_ctx(MainExport(__import_name))]);

        Util.test_many(
          test_parse_import,
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
      }
    ),
    "parse module import"
    >:: (
      _ => {
        let stmt =
          [
            Keyword(Import),
            Asterisk,
            Keyword(As),
            Identifier(__import_name),
            Keyword(From),
            String(__module),
          ]
          |> Util.drift;
        let expected =
          Import(__module, [no_ctx(ModuleExport(__import_name))]);

        Util.test_many(
          test_parse_import,
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
      }
    ),
    "parse named import"
    >:: (
      _ => {
        let stmt =
          [
            Keyword(Import),
            LeftBrace,
            Identifier(__import_name),
            RightBrace,
            Keyword(From),
            String(__module),
          ]
          |> Util.drift;
        let expected =
          Import(__module, [no_ctx(NamedExport(__import_name, None))]);

        Util.test_many(
          test_parse_import,
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
      }
    ),
    "parse aliased import"
    >:: (
      _ => {
        let alias_name = "MyAlias";
        let stmt =
          [
            Keyword(Import),
            LeftBrace,
            Identifier(__import_name),
            Keyword(As),
            Identifier(alias_name),
            RightBrace,
            Keyword(From),
            String(__module),
          ]
          |> Util.drift;
        let expected =
          Import(
            __module,
            [no_ctx(NamedExport(__import_name, Some(alias_name)))],
          );

        Util.test_many(
          test_parse_import,
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
      }
    ),
  ];
