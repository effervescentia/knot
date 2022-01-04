open Kore;

include Test.Assert;

let _mock_ns_context = report =>
  NamespaceContext.create(~report, Internal("mock"));
let _mock_module_context = x => ModuleContext.create(x);

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
        ~ns_context=_mock_ns_context(report),
        ~mod_context=_mock_module_context,
        ~cursor=false,
        expected,
        source,
      ) =>
    InputStream.of_string(~cursor, source)
    |> LazyStream.of_stream
    |> T.parser((ns_context, mod_context(ns_context)))
    |> (
      fun
      | Some(actual) => T.test(expected, actual)
      | None =>
        source
        |> Fmt.str("failed to parse input: '%s'")
        |> Alcotest.fail
        |> ignore
    );

  let parse_all =
      (
        ~report=throw,
        ~ns_context=_mock_ns_context(report),
        ~mod_context=_mock_module_context,
        ~cursor=false,
        o,
      ) =>
    List.iter(parse(~ns_context, ~mod_context, ~report, ~cursor, o));

  let no_parse =
      (
        ~report=throw,
        ~ns_context=_mock_ns_context(report),
        ~mod_context=_mock_module_context,
        ~cursor=false,
        source,
      ) =>
    InputStream.of_string(~cursor, source)
    |> LazyStream.of_stream
    |> T.parser((ns_context, mod_context(ns_context)))
    |> (
      fun
      | Some(r) =>
        source |> Fmt.str("parsed input: '%s'") |> Alcotest.fail |> ignore
      | None => ()
    );

  let parse_none =
      (
        ~report=throw,
        ~ns_context=_mock_ns_context(report),
        ~mod_context=_mock_module_context,
        ~cursor=false,
      ) =>
    List.iter(no_parse(~report, ~ns_context, ~mod_context, ~cursor));
};
