open Kore;

include Test.Assert;

module type ParseTarget = {
  include Test.Assert.Target;

  let parser:
    ((NamespaceContext.t, ModuleContext.t), Grammar.Program.input_t) =>
    option(t);
};

module Make = (T: ParseTarget) => {
  let parse =
      (
        ~report=throw,
        ~ns_context=NamespaceContext.create(~report, Internal("mock")),
        ~mod_context=x => ModuleContext.create(x),
        ~cursor=false,
        source,
        expected,
      ) =>
    InputStream.of_string(~cursor, source)
    |> LazyStream.of_stream
    |> T.parser((ns_context, mod_context(ns_context)))
    |> (
      fun
      | Some(actual) => T.test(expected, actual)
      | None =>
        source
        |> Print.fmt("failed to parse input: '%s'")
        |> Alcotest.fail
        |> ignore
    );

  let parse_many =
      (
        ~report=throw,
        ~ns_context=NamespaceContext.create(~report, Internal("mock")),
        ~mod_context=x => ModuleContext.create(x),
        ~cursor=false,
      ) =>
    List.iter(((i, o)) =>
      parse(~ns_context, ~mod_context, ~report, ~cursor, i, o)
    );

  let parse_all =
      (
        ~report=throw,
        ~ns_context=NamespaceContext.create(~report, Internal("mock")),
        ~mod_context=x => ModuleContext.create(x),
        ~cursor=false,
        o,
      ) =>
    List.iter(i => parse(~ns_context, ~mod_context, ~report, ~cursor, i, o));

  let no_parse =
      (
        ~report=throw,
        ~ns_context=NamespaceContext.create(~report, Internal("mock")),
        ~mod_context=x => ModuleContext.create(x),
        ~cursor=false,
      ) =>
    List.iter(source =>
      InputStream.of_string(~cursor, source)
      |> LazyStream.of_stream
      |> T.parser((ns_context, mod_context(ns_context)))
      |> (
        fun
        | Some(r) =>
          source |> Print.fmt("parsed input: '%s'") |> Alcotest.fail |> ignore
        | None => ()
      )
    );
};
