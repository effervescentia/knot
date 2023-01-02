open Kore;

include Test.Assert;

module A = AST.Result;
module AR = AST.Raw;
module ParseContext = AST.ParseContext;
module Type = AST.Type;

module Compare = {
  include Compare;

  let type_ = Alcotest.testable(Type.pp, (==));
};

let _mock_context = report => ParseContext.create(~report, Internal("mock"));

let symbol_assoc_list =
  Alcotest.(
    check(list(pair(string, Compare.type_)), "symbol assoc list matches")
  );

let parse_completely = x => Parse.Onyx.(x << (eof() |> Parse.Matchers.lexeme));

module type AssertParams = {
  include Test.Assert.Target;

  let parser: (ParseContext.t, Language.Program.input_t) => option(t);
};

module Make = (Params: AssertParams) => {
  let parse =
      (
        ~report=AST.Error.throw,
        ~context=_mock_context(report),
        ~cursor=false,
        expected,
        source,
      ) =>
    InputStream.of_string(~cursor, source)
    |> LazyStream.of_stream
    |> Params.parser(context)
    |> (
      fun
      | Some(actual) => Params.test(expected, actual)
      | None =>
        source |> Alcotest.failf("failed to parse input: '%s'") |> ignore
    );

  let parse_all =
      (
        ~report=AST.Error.throw,
        ~context=_mock_context(report),
        ~cursor=false,
        o,
      ) =>
    List.iter(parse(~context, ~report, ~cursor, o));

  let no_parse =
      (
        ~report=AST.Error.throw,
        ~context=_mock_context(report),
        ~cursor=false,
        source,
      ) =>
    InputStream.of_string(~cursor, source)
    |> LazyStream.of_stream
    |> Params.parser(context)
    |> Option.iter(_ =>
         source |> Alcotest.failf("parsed input: '%s'") |> ignore
       );

  let parse_none =
      (
        ~report=AST.Error.throw,
        ~context=_mock_context(report),
        ~cursor=false,
      ) =>
    List.iter(no_parse(~report, ~context, ~cursor));

  let parse_throws =
      (
        ~report=AST.Error.throw,
        ~context=_mock_context(report),
        ~cursor=false,
        err,
        message,
        source,
      ) =>
    throws(err, message, () =>
      InputStream.of_string(~cursor, source)
      |> LazyStream.of_stream
      |> Params.parser(context)
    );
};

module type TypedParserParams = {
  type value_t;
  type type_t;

  let parser: ParseContext.t => Parse.Parser.t(N.t(value_t, type_t));

  let pp_value: Fmt.t(value_t);
  let pp_type: Fmt.t(type_t);
};

module MakeTyped = (Params: TypedParserParams) =>
  Make({
    type t = N.t(Params.value_t, Params.type_t);

    let parser = ctx =>
      Params.parser(ctx) |> parse_completely |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              Dump.node_to_xml(
                ~dump_type=Params.pp_type |> (~@),
                ~dump_value=Params.pp_value |> (~@),
                "Parsed",
              )
              % Fmt.xml_string(ppf),
            (==),
          ),
          "parsed result matches",
        )
      );
  });

module type PrimitiveParserParams = {
  let parser: Parse.Parser.t(N.t(AR.primitive_t, unit));
};

module MakePrimitive = (Params: PrimitiveParserParams) =>
  MakeTyped({
    type value_t = AR.primitive_t;
    type type_t = unit;

    let parser = _ => Params.parser;

    let pp_value = ppf =>
      KPrimitive.Plugin.to_xml((_ => raise(NotImplemented), _ => ""))
      % Fmt.xml_string(ppf);
    let pp_type = (_, ()) => ();
  });

module Declaration =
  Make({
    type t = AST.Module.module_statement_t;

    let parser = KDeclaration.Plugin.parse % parse_completely % Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              Language.Debug.module_statement_to_xml % Fmt.xml_string(ppf),
            (==),
          ),
          "parsed result matches",
        )
      );
  });
