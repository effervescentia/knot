open Kore;

let rec format: Fmt.t(AST.Result.raw_expression_t) =
  ppf => {
    let bind = formatter => formatter(format, ppf);

    Util.fold(
      ~primitive=bind(KPrimitive.format),
      ~identifier=bind(KIdentifier.format),
      ~ksx=bind(KSX.format),
      ~group=bind(KGroup.format),
      ~binary_op=bind(KBinaryOperator.format),
      ~unary_op=bind(KUnaryOperator.format),
      ~closure=bind(KClosure.format),
      ~dot_access=bind(KDotAccess.format),
      ~bind_style=bind(KBindStyle.format),
      ~function_call=bind(KFunctionCall.format),
      ~style=bind(KStyle.format),
    );
  };
