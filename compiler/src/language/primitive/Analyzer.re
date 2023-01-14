open Knot.Kore;

let analyze: Interface.Plugin.analyze_t('ast, 'expr, 'result_expr) =
  (_, _, (primitive, _)) => {
    let bind = (type_, _) => AST.Type.Valid(type_);

    (
      primitive,
      primitive
      |> Interface.fold(
           ~nil=bind(Nil),
           ~boolean=bind(Boolean),
           ~integer=bind(Integer),
           ~float=bind(Float),
           ~string=bind(String),
         ),
    );
  };
