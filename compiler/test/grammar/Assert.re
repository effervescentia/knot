open Kore;

include Test.Assert;

module type ParseTarget = {
  include Test.Assert.Target;

  let parser: (Context.t, Grammar.Program.input_t) => option(t);
};

module Make = (T: ParseTarget) => {
  let parse =
      (~scope=Scope.create(), ~report=throw, ~cursor=false, source, expected) =>
    InputStream.of_string(~cursor, source)
    |> LazyStream.of_stream
    |> T.parser(Context.create(~scope, ~report, Internal("mock")))
    |> (
      fun
      | Some(actual) => T.test(expected, actual)
      | None =>
        source
        |> Print.fmt("failed to parse input: '%s'")
        |> Alcotest.fail
        |> ignore
    );

  let parse_many = (~scope=Scope.create(), ~report=throw, ~cursor=false) =>
    List.iter(((i, o)) => parse(~scope, ~report, ~cursor, i, o));

  let parse_all = (~scope=Scope.create(), ~report=throw, ~cursor=false, o) =>
    List.iter(i => parse(~scope, ~report, ~cursor, i, o));

  let no_parse = (~scope=Scope.create(), ~report=throw, ~cursor=false) =>
    List.iter(source =>
      InputStream.of_string(~cursor, source)
      |> LazyStream.of_stream
      |> T.parser(Context.create(~scope, ~report, Internal("mock")))
      |> (
        fun
        | Some(r) =>
          source |> Print.fmt("parsed input: '%s'") |> Alcotest.fail |> ignore
        | None => ()
      )
    );
};
