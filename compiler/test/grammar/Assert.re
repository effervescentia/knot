open Kore;

include Test.Assert;

type parser_context_t = (NamespaceContext.t, ModuleContext.t);

let _mock_ns_context = report =>
  NamespaceContext.create(~report, Internal("mock"));
let _mock_module_context = x => ModuleContext.create(x);

module type AssertParams = {
  include Test.Assert.Target;

  let parser: (parser_context_t, Grammar.Program.input_t) => option(t);
};

module Make = (Params: AssertParams) => {
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
    |> Params.parser((ns_context, mod_context(ns_context)))
    |> (
      fun
      | Some(actual) => Params.test(expected, actual)
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
    |> Params.parser((ns_context, mod_context(ns_context)))
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

  let parse_throws =
      (
        ~report=throw,
        ~ns_context=_mock_ns_context(report),
        ~mod_context=_mock_module_context,
        ~cursor=false,
        err,
        message,
        source,
      ) =>
    throws(err, message, () =>
      InputStream.of_string(~cursor, source)
      |> LazyStream.of_stream
      |> Params.parser((ns_context, mod_context(ns_context)))
    );
};

module type TypedParserParams = {
  type value_t;
  type type_t;

  let parser:
    parser_context_t => Grammar.Kore.parser_t(N.t(value_t, type_t));

  let pp_value: Fmt.t(value_t);
  let pp_type: Fmt.t(type_t);
};

module MakeTyped = (Params: TypedParserParams) =>
  Make({
    type t = N.t(Params.value_t, Params.type_t);

    let parser = ctx => Parser.parse(Params.parser(ctx));

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, node) =>
              A.Dump.(
                node
                |> node_to_entity(
                     Params.pp_type,
                     "Parsed",
                     ~attributes=[
                       ("value", node |> N.get_value |> ~@Params.pp_value),
                     ],
                   )
                |> Entity.pp(ppf)
              ),
            (==),
          ),
          "parsed result matches",
        )
      );
  });

module type PrimitiveParserParams = {
  let parser: Grammar.Kore.parser_t(N.t(AR.primitive_t, TR.t));
};

module MakePrimitive = (Params: PrimitiveParserParams) =>
  MakeTyped({
    type value_t = AR.primitive_t;
    type type_t = TR.t;

    let parser = _ => Params.parser;

    let pp_value = ppf => AR.Dump.prim_to_string % Fmt.string(ppf);
    let pp_type = TR.pp;
  });
