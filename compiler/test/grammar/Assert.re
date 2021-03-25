open Kore;

include Test.Assert;

module type ParseTarget = {
  include Test.Assert.Target;

  let parser: (Scope.t, Grammar.Program.input_t) => option(t);
};

module Make = (T: ParseTarget) => {
  let parse = (~scope=Scope.create(), ~cursor=false, source, expected) =>
    InputStream.of_string(~cursor, source)
    |> LazyStream.of_stream
    |> T.parser(scope)
    |> (
      fun
      | Some(actual) => T.test(expected, actual)
      | None =>
        source
        |> Print.fmt("failed to parse input: '%s'")
        |> Alcotest.fail
        |> ignore
    );

  let parse_many = (~scope=Scope.create(), ~cursor=false) =>
    List.iter(((i, o)) => parse(~scope, ~cursor, i, o));

  let parse_all = (~scope=Scope.create(), ~cursor=false, o) =>
    List.iter(i => parse(~scope, ~cursor, i, o));

  let no_parse = (~scope=Scope.create(), ~cursor=false) =>
    List.iter(source =>
      InputStream.of_string(~cursor, source)
      |> LazyStream.of_stream
      |> T.parser(scope)
      |> (
        fun
        | Some(r) =>
          source |> Print.fmt("parsed input: '%s'") |> Alcotest.fail |> ignore
        | None => ()
      )
    );
};
