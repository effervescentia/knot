open Knot.Kore;
open Common;

module Parser = Parse.Parser;

type debug_node_t('value, 'typ) = Node.t('value, 'typ) => Fmt.xml_t(string);

module Interface = {
  type parse_t('value) = Parser.t(raw_t('value));

  type contextual_parse_t('ast, 'value) =
    ParseContext.t('ast) => parse_t('value);

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

  module Make = (Params: Params) => {
    include Params;

    type parse_t('ast, 'expr) =
      parse_arg_t('ast, 'expr) => Interface.parse_t(value_t('expr, unit));
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
  module type MakeExtension = (Params: Params) => {let to_xml: string;};
};

module Primitive = {
  module type TypeParams = {type value_t;};

  module MakeTypes = (Params: TypeParams) => {
    include Params;

    type parse_t = Interface.parse_t(value_t);

    type format_t = Fmt.t(value_t);

    type debug_t = Interface.debug_t(value_t);
  };

  module type Params = {
    include (module type of
      MakeTypes({
        type value_t;
      }));

    let parse: parse_t;
    let format: format_t;
    let to_xml: debug_t;
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
      ((Operator.Binary.t, (raw_t('expr), raw_t('expr))) => 'expr) =>
      Parse.Parser.t(((raw_t('expr), raw_t('expr))) => raw_t('expr));

    let parse: parse_t('expr) =
      f =>
        Parse.Kore.(
          Parse.Util.binary_op(((lhs, rhs)) =>
            (lhs, rhs) |> f(fst(Params.operator))
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
    // include DebugPlugin.Params;
  };

  module MakeTypes = (Params: TypeParams) => {
    include Params;
    include AnalyzePlugin.Make(Params);
    include FormatPlugin.Make(Params);
    include DebugPlugin.Make({
      include Params;
      type debug_arg_t('expr, 'typ) = (
        debug_node_t('expr, 'typ),
        'typ => string,
      );
    });

    type tokenize_t('expr, 'prim, 'typ) =
      (Node.t('expr, 'typ) => TokenTree2.t('prim), value_t('expr, 'typ)) =>
      TokenTree2.t('prim);
  };

  module type Params = {
    include TypeParams;
    include (module type of
      AnalyzePlugin.Make({
        type value_t('expr, 'typ);
        type analyze_arg_t('ast, 'raw_expr, 'result_expr);
      }));
    // include (module type of
    //   FormatPlugin.Make({
    //     type value_t('expr, 'typ);
    //     type format_arg_t('expr, 'typ);
    //   }));
    include (module type of
      DebugPlugin.Make({
        type value_t('expr, 'typ);
        type debug_arg_t('expr, 'typ);
      }));

    let analyze: analyze_t('ast, 'raw_expr, 'result_expr);

    let format:
      format_arg_t('expr, 'typ) =>
      Interface.format_t('expr, value_t('expr, 'typ));

    let to_xml: debug_t('expr, 'typ);
  };

  module Make = (Params: Params) => {
    include Params;
  };
};

module Expression = {
  module type TypeParams = {
    include NoParseExpression.TypeParams;
    include ParsePlugin.Params;
  };

  module MakeParsePlugin = (Params: ParsePlugin.Params) =>
    ParsePlugin.Make({
      include Params;
      type value_t('expr, 'typ) = 'expr;
    });

  module ParseExtension = (Params: ParsePlugin.Params) => {
    type parse_t('ast, 'expr) =
      (Params.value_t('expr, unit) => 'expr) =>
      MakeParsePlugin(Params).parse_t('ast, 'expr);
  };

  module MakeTypes = (Params: TypeParams) => {
    include Params;
    include NoParseExpression.MakeTypes(Params);
    include ParseExtension(Params);
  };

  module type Params = {
    include TypeParams;
    include NoParseExpression.Params;
    include (module type of
      ParseExtension({
        type value_t('expr, 'typ);
        type parse_arg_t('ast, 'expr);
      }));

    let parse: parse_t('ast, 'expr);
  };

  module Make = (Params: Params) => {
    include Params;
  };
};

module Statement = {
  module type TypeParams = {include BaseParams;};

  module MakeTypes = (Params: TypeParams) => {
    include Params;
    include AnalyzePlugin.Make({
      include Params;
      type analyze_arg_t('ast, 'raw_expr, 'result_expr) =
        Interface.analyze_t('ast, 'raw_expr, 'result_expr);
    });
    include DebugPlugin.Make({
      include Params;
      type debug_arg_t('expr, 'typ) =
        Node.t('expr, 'typ) => Fmt.xml_t(string);
    });

    module ParseParams = {
      include Params;
      type parse_arg_t('ast, 'expr) = (
        ParseContext.t('ast),
        Interface.contextual_parse_t('ast, 'expr),
      );
    };

    type parse_t('ast, 'expr) =
      ParsePlugin.Make(ParseParams).parse_t('ast, 'expr);

    type format_t('expr, 'typ) =
      Interface.format_t('expr, value_t('expr, 'typ));
  };

  module type Params = {
    include (module type of
      MakeTypes({
        type value_t('expr, 'typ);
        type analyze_arg_t('ast, 'raw_expr, 'result_expr);
      }));

    let parse: parse_t('ast, 'expr);
    let analyze: analyze_t('ast, 'raw_expr, 'result_expr);
    let format: format_t('expr, 'typ);
    let to_xml: debug_t('expr, 'typ);
  };

  module Make = (Params: Params) => {
    include Params;
  };
};

module Declaration = {
  module type TypeParams = {type value_t('typ);};

  module MakeTypes = (Params: TypeParams) => {
    include Params;

    type parse_t('ast) =
      bool =>
      Interface.contextual_parse_t(
        'ast,
        (identifier_t, Node.t(value_t(Type.t), Type.t)),
      );

    type format_t('typ) = Fmt.t((string, value_t('typ)));

    type debug_t('typ) =
      ('typ => string, value_t('typ)) => Fmt.xml_t(string);
  };

  module type Params = {
    include (module type of
      MakeTypes({
        type value_t('typ);
      }));

    let parse: parse_t('ast);

    let format: format_t('typ);

    let to_xml: debug_t('typ);
  };

  module Make = (Params: Params) => {
    include Params;
  };
};
