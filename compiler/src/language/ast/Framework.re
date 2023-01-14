open Knot.Kore;
open Common;

module Parser = Parse.Parser;

// TODO: the 'typ variables here can be replaced with unit
type expression_parser_t('expr, 'typ) = Parser.t(Node.t('expr, 'typ));
type contextual_expression_parser_t('ast, 'expr, 'typ) =
  ParseContext.t('ast) => expression_parser_t('expr, 'typ);

type binary_op_parser_t('expr) =
  Parser.t(((raw_t('expr), raw_t('expr))) => raw_t('expr));

type debug_node_t('value, 'typ) = Node.t('value, 'typ) => Fmt.xml_t(string);

// type statement_parser_t = Parser.t(Raw.statement_t);

// type declaration_parser_t =
//   Parser.t(raw_t((Common.identifier_t, Result.declaration_t)));

module Interface = {
  type analyze_t('ast, 'raw, 'result) =
    (Scope.t('ast), raw_t('raw)) => ('result, Type.t);

  type format_t('inner, 'outer) = Fmt.t('inner) => Fmt.t('outer);

  type debug_t('value) = 'value => Fmt.xml_t(string);
};

module type BaseParams = {type value_t('expr, 'typ);};

module ParsePlugin = {
  module type Params = {
    include BaseParams;

    type parse_arg_t('ast, 'expr);
  };
};

module AnalyzePlugin = {
  module type Params = {
    include BaseParams;

    type analyze_arg_t('ast, 'raw_expr, 'result_expr);
  };

  module Make = (Params: Params) => {
    include Params;

    type analyze_t('ast, 'raw, 'result) =
      analyze_arg_t('ast, 'raw, 'result) =>
      Interface.analyze_t(
        'ast,
        value_t('raw, unit),
        value_t('result, Type.t),
      );
  };
};

module FormatPlugin = {
  module type Params = {
    include BaseParams;

    type format_arg_t('expr, 'typ);
  };

  module Make = (Params: Params) => {
    include Params;

    type format_t('expr, 'typ) =
      format_arg_t('expr, 'typ) =>
      Interface.format_t('expr, value_t('expr, 'typ));
  };
};

module DebugPlugin = {
  module type Params = {
    include BaseParams;

    type debug_arg_t('expr, 'typ);
  };

  module Make = (Params: Params) => {
    include Params;

    type debug_t('expr, 'typ) =
      debug_arg_t('expr, 'typ) => Interface.debug_t(value_t('expr, 'typ));
  };
};

module Primitive = {
  module type Params = {
    type value_t;

    let parse: Parser.t(raw_t(value_t));

    let format: Fmt.t(value_t);

    let to_xml: value_t => Fmt.xml_t(string);
  };

  module Make = (Params: Params) => {
    include Params;
  };
};

module UnaryOperator = {
  module type Params = {let operator: (Operator.Unary.t, char);};

  module Make = (Params: Params) => {
    type parse_t('expr) =
      (((Operator.Unary.t, raw_t('expr))) => 'expr) =>
      Parse.Parser.t(raw_t('expr) => raw_t('expr));

    let parse: parse_t('expr) =
      f =>
        Parse.Kore.(
          Parse.Util.unary_op(expression =>
            (fst(Params.operator), expression) |> f
          )
          <$ (Params.operator |> snd |> Matchers.symbol >|= Node.map(_ => ()))
        );

    let format: Fmt.t(unit) =
      (ppf, ()) => Params.operator |> snd |> Fmt.char(ppf);
  };
};

module BinaryOperator = {
  type t =
    | Glyph(string)
    | Character(char);

  module type Params = {let operator: (Operator.Binary.t, t);};

  module Make = (Params: Params) => {
    type parse_t('expr) =
      (((Operator.Binary.t, raw_t('expr), raw_t('expr))) => 'expr) =>
      Parse.Parser.t(((raw_t('expr), raw_t('expr))) => raw_t('expr));

    let parse: parse_t('expr) =
      f =>
        Parse.Kore.(
          Parse.Util.binary_op(((lhs, rhs)) =>
            (fst(Params.operator), lhs, rhs) |> f
          )
          <$ (
            switch (snd(Params.operator)) {
            | Glyph(glyph) => Matchers.glyph(glyph)
            | Character(char) => Matchers.symbol(char) >|= Node.map(_ => ())
            }
          )
        );

    let format: Fmt.t(unit) =
      (ppf, ()) =>
        switch (snd(Params.operator)) {
        | Character(char) => Fmt.char(ppf, char)
        | Glyph(glyph) => Fmt.string(ppf, glyph)
        };
  };
};

module NoParseExpression = {
  module type TypeParams = {
    include AnalyzePlugin.Params;
    include FormatPlugin.Params;
    include DebugPlugin.Params;
  };

  module MakeTypes = (Params: TypeParams) => {
    include Params;
    include AnalyzePlugin.Make(Params);
    include FormatPlugin.Make(Params);
    include DebugPlugin.Make(Params);
  };

  module type Params = {
    include TypeParams;

    let analyze:
      analyze_arg_t('ast, 'raw_expr, 'result_expr) =>
      Interface.analyze_t(
        'ast,
        value_t('raw_expr, unit),
        value_t('result_expr, Type.t),
      );

    let format:
      format_arg_t('expr, 'typ) =>
      Interface.format_t('expr, value_t('expr, 'typ));

    let to_xml:
      debug_arg_t('expr, 'typ) => Interface.debug_t(value_t('expr, 'typ));
  };

  module Make = (Params: Params) => {
    include Params;
  };
};

module Expression = {
  include NoParseExpression;

  type parse_t('arg, 'value, 'expr) =
    ('value => 'expr, 'arg) => Parser.t(raw_t('expr));
  type _parse_t('arg, 'value, 'expr) = parse_t('arg, 'value, 'expr);

  module type TypeParams = {
    include NoParseExpression.TypeParams;

    type parse_arg_t('ast, 'expr);
  };

  module MakeTypes = (Params: TypeParams) => {
    include Params;
    include NoParseExpression.MakeTypes(Params);

    type parse_t('ast, 'expr) =
      _parse_t(parse_arg_t('ast, 'expr), value_t('expr, unit), 'expr);
  };

  module type Params = {
    include TypeParams;
    include NoParseExpression.Params;

    let parse:
      parse_t(parse_arg_t('ast, 'expr), value_t('expr, unit), 'expr);
    // let analyze:
    //   analyze_t(
    //     'ast,
    //     'raw_expr,
    //     'result_expr,
    //     value_t('raw_expr, unit),
    //     value_t('result_expr, Type.t),
    //   );
  };

  module Make = (Params: Params) => {
    include Params;
    include NoParseExpression.Make(Params);
  };
};

module Statement = {
  type _debug_arg_t('expr, 'typ) = Node.t('expr, 'typ) => Fmt.xml_t(string);

  type _parse_t('ast, 'expr, 'stmt) =
    (
      (
        ParseContext.t('ast),
        contextual_expression_parser_t('ast, 'expr, unit),
      )
    ) =>
    Parser.t(Common.raw_t('stmt));

  module type TypeParams = {include AnalyzePlugin.Params;};

  module MakeTypes = (Params: TypeParams) => {
    include Params;
    include AnalyzePlugin.Make(Params);
    include DebugPlugin.Make({
      type value_t('expr, 'typ) = Params.value_t('expr, 'typ);
      type debug_arg_t('expr, 'typ) = _debug_arg_t('expr, 'typ);
    });

    type parse_t('ast, 'expr) = _parse_t('ast, 'expr, value_t('expr, unit));

    type format_t('expr, 'typ) =
      Interface.format_t('expr, value_t('expr, 'typ));
  };

  module type Params = {
    include TypeParams;

    let parse: _parse_t('ast, 'expr, value_t('expr, unit));

    let analyze:
      analyze_arg_t('ast, 'raw_expr, 'result_expr) =>
      Interface.analyze_t(
        'ast,
        value_t('raw_expr, unit),
        value_t('result_expr, Type.t),
      );

    let format: Interface.format_t('expr, value_t('expr, 'typ));

    let to_xml:
      _debug_arg_t('expr, 'typ) => Interface.debug_t(value_t('expr, 'typ));
  };

  module Make = (Params: Params) => {
    include Params;
  };
};
