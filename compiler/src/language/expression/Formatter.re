open Kore;

let _is_binary_op =
  Interface.(
    fun
    | BinaryOp(_) => true
    | _ => false
  );

let _get_style_rules =
  Interface.(
    fun
    | Style(rules) => Some(rules)
    | _ => None
  );

let attribute_needs_wrapper =
  Interface.(
    fun
    | Primitive(_)
    | Identifier(_)
    | Group(_)
    | Closure(_)
    /* show tags or fragments with no children */
    | KSX(Tag(_, _, _, _, []) | Fragment([])) => false
    | _ => true
  );

let rec format: Fmt.t(Interface.t('typ)) =
  ppf => {
    let bind = formatter => formatter(format, ppf);

    Interface.fold(
      ~primitive=bind(KPrimitive.format()),
      ~identifier=bind(KIdentifier.format()),
      ~ksx=bind(KSX.format(attribute_needs_wrapper)),
      ~group=bind(KGroup.format(_is_binary_op)),
      ~binary_op=bind(KBinaryOperator.format()),
      ~unary_op=bind(KUnaryOperator.format()),
      ~closure=bind(KClosure.format()),
      ~dot_access=bind(KDotAccess.format()),
      ~bind_style=bind(KBindStyle.format(_get_style_rules)),
      ~function_call=bind(KFunctionCall.format()),
      ~style=bind(KStyle.format()),
    );
  };
