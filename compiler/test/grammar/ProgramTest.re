open Kore;

module Program = Grammar.Program;

module Assert =
  Assert.Make({
    type t = AST.program_t;

    let parser = Parser.parse(Program.main);

    let test =
      Alcotest.(
        check(
          list(
            testable(
              (x, y) => AST.print_mod_stmt(y) |> Format.print_string,
              (==),
            ),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Program"
  >::: [
    "no parse" >: (() => Assert.no_parse("import")),
    "parse"
    >: (
      () => Assert.parse("import foo from \"bar\"", [Import("bar", "foo")])
    ),
  ];
