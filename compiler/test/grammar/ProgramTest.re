open Kore;

module Program = Grammar.Program;

module Target = {
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
};

module AssertImports =
  Assert.Make({
    include Target;

    let parser = Parser.parse(Program.imports);
  });
module Assert = Assert.Make(Target);

let suite =
  "Program"
  >::: [
    "no parse" >: (() => Assert.no_parse("import")),
    "parse"
    >: (
      () => Assert.parse("import foo from \"bar\"", [Import("bar", "foo")])
    ),
    "parse multiple"
    >: (
      () =>
        Assert.parse(
          "import foo from \"bar\"; import fizz from \"buzz\"",
          [Import("bar", "foo"), Import("buzz", "fizz")],
        )
    ),
    "parse imports only"
    >: (
      () =>
        AssertImports.parse(
          "import foo from \"bar\"; gibberish",
          [Import("bar", "foo")],
        )
    ),
  ];
