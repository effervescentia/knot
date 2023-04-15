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
      ~primitive=bind(Primitive.format()),
      ~identifier=bind(Identifier.format()),
      ~ksx=bind(KSX.format(attribute_needs_wrapper)),
      ~group=bind(Group.format(_is_binary_op)),
      ~binary_op=bind(BinaryOperator.format()),
      ~unary_op=bind(UnaryOperator.format()),
      ~closure=bind(Closure.format()),
      ~dot_access=bind(DotAccess.format()),
      ~bind_style=bind(BindStyle.format(_get_style_rules)),
      ~function_call=bind(FunctionCall.format()),
      ~style=bind(Style.format()),
    );
  };
