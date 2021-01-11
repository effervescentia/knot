open Kore;

include Test.Assert;

module type ParseTarget = {
  include Test.Assert.Target;

  let parser: Grammar.Program.input_t => option(t);
};

module Make = (T: ParseTarget) => {
  let parse = (~cursor=false, source, result) =>
    CharStream.of_string(~cursor, source)
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

  let parse_many = (~cursor=false) =>
    List.iter(((i, o)) => parse(~cursor, i, o));

  let parse_all = (~cursor=false, o) => List.iter(i => parse(~cursor, i, o));

  let no_parse = (~cursor=false) =>
    List.iter(source =>
      CharStream.of_string(~cursor, source)
      |> LazyStream.of_stream
      |> T.parser
      |> (
        fun
        | Some(r) =>
          source |> Print.fmt("parsed input: '%s'") |> Alcotest.fail |> ignore
        | None => ()
      )
    );
};
