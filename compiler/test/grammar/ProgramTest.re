open Kore;

module Program = Grammar.Program;

let _parse = Parser.parse(Program.main);

let _check_string = Alcotest.(check(string, "string matches"));

let _check_parse = (source, result) =>
  LazyStream.of_string(source)
  |> _parse
  |> (
    fun
    | Some(r) => _check_string(r, result)
    | None =>
      source
      |> Printf.sprintf("failed to parse input: '%s'")
      |> Alcotest.fail
      |> ignore
  );

let suite = "Program" >::: ["parse" >: (() => _check_parse("foo", "bar"))];
