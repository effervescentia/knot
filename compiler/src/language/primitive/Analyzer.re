open Knot.Kore;

let analyze_primitive =
    ((primitive, _): AST.Common.raw_t(Interface.t))
    : (Interface.t, AST.Type.t) => {
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

let analyze: Interface.Plugin.analyze_t('ast, 'expr, 'result_expr) =
  ((), _) => analyze_primitive;
