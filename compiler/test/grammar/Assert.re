open Kore;

include Test.Assert;

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
        ~report=throw,
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
      (~report=throw, ~context=_mock_context(report), ~cursor=false, o) =>
    List.iter(parse(~context, ~report, ~cursor, o));

  let no_parse =
      (~report=throw, ~context=_mock_context(report), ~cursor=false, source) =>
    InputStream.of_string(~cursor, source)
    |> LazyStream.of_stream
    |> Params.parser(context)
    |> Option.iter(_ =>
         source |> Alcotest.failf("parsed input: '%s'") |> ignore
       );

  let parse_none =
      (~report=throw, ~context=_mock_context(report), ~cursor=false) =>
    List.iter(no_parse(~report, ~context, ~cursor));

  let parse_throws =
      (
        ~report=throw,
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

  let parser: ParseContext.t => Language.Kore.parser_t(N.t(value_t, type_t));

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
            (ppf, node) =>
              A.Dump.(
                node
                |> node_to_entity(
                     Params.pp_type,
                     "Parsed",
                     ~attributes=[
                       ("value", node |> fst |> ~@Params.pp_value),
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
  let parser: Language.Kore.parser_t(N.t(AR.primitive_t, unit));
};

module MakePrimitive = (Params: PrimitiveParserParams) =>
  MakeTyped({
    type value_t = AR.primitive_t;
    type type_t = unit;

    let parser = _ => Params.parser;

    let pp_value = ppf => AR.Dump.prim_to_string % Fmt.string(ppf);
    let pp_type = (_, ()) => ();
  });
