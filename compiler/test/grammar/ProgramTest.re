open Kore;

module Program = Grammar.Program;

module Target = {
  type t = AST.program_t;

  let parser = Parser.parse(Program.main);

  let test =
    Alcotest.(
      check(
        list(testable(_ => fmt_mod_stmt % Format.print_string, (==))),
        "program matches",
      )
    );
};

module AssertImports =
  Assert.Make({
    include Target;

    let parser = Parser.parse(Program.imports);
  });
module Assert = Assert.Make(Target);

let __main_import = "import foo from \"bar\"";
let __const_decl = "const foo = nil";

let __main_import_ast = ("bar", "foo") |> AST.of_import;
let __const_decl_ast =
  ("foo", AST.nil |> AST.of_prim |> AST.of_const) |> AST.of_decl;

let suite =
  "Program"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse import"
    >: (() => Assert.parse(__main_import, [__main_import_ast])),
    "parse declaration"
    >: (() => Assert.parse(__const_decl, [__const_decl_ast])),
    "parse multiple"
    >: (
      () =>
        Assert.parse(
          Print.fmt("%s; %s", __main_import, __const_decl),
          [__main_import_ast, __const_decl_ast],
        )
    ),
    "parse imports only"
    >: (
      () =>
        AssertImports.parse(
          __main_import |> Print.fmt("%s; gibberish"),
          [__main_import_ast],
        )
    ),
  ];
