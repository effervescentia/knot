open Kore;

include Test.Assert;

module type Target = {
  type t;

  let test: (t, t) => unit;

  let parser: Grammar.Program.input_t => option(t);
};

module Make = (T: Target) => {
  let parse = (source, result) =>
    CharStream.of_string(source)
    |> LazyStream.of_stream
    |> T.parser
    |> (
      fun
      | Some(r) => T.test(result, r)
      | None =>
        source
        |> Print.fmt("failed to parse input: '%s'")
        |> Alcotest.fail
        |> ignore
    );

  let no_parse = source =>
    CharStream.of_string(source)
    |> LazyStream.of_stream
    |> T.parser
    |> (
      fun
      | Some(r) =>
        source |> Print.fmt("parsed input: '%s'") |> Alcotest.fail |> ignore
      | None => ()
    );
};
