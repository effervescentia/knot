open Knot.Kore;
open AST;

let rec analyze: (SymbolTable.t, Interface.t) => Type.t =
  defs =>
    Interface.fold(
      ~nil=() => Type.Valid(Nil),
      ~boolean=() => Type.Valid(Boolean),
      ~integer=() => Type.Valid(Integer),
      ~float=() => Type.Valid(Float),
      ~string=() => Type.Valid(String),
      ~element=() => Type.Valid(Element),
      ~style=() => Type.Valid(Style),
      ~identifier=
        ((id, _)) =>
          defs |> SymbolTable.resolve_type(id) |?: Invalid(NotInferrable),
      ~group=((x, _)) => analyze(defs, x),
      ~list=((x, _)) => Valid(List(analyze(defs, x))),
      ~object_=xs => Valid(Object(analyze_object_properties(defs, xs))),
      ~function_=
        ((args, (res, _))) =>
          Valid(
            Function(
              args |> List.map(fst % analyze(defs)),
              analyze(defs, res),
            ),
          ),
      ~dot_access=
        (((root, _), (prop, _))) =>
          switch (root |> analyze(defs)) {
          | Valid(Module(entries)) =>
            entries
            |> List.find_map(
                 fun
                 | (Type.ModuleEntryKind.Type, id, type_) when id == prop =>
                   Some(type_)
                 | _ => None,
               )
            |?: Invalid(NotInferrable)
          | _ => Invalid(NotInferrable)
          },
      ~view=
        (((props, _), (res, _))) =>
          switch (props |> analyze(defs), res |> analyze(defs)) {
          | (
              Valid(Object(props')),
              Valid(Nil | Boolean | Integer | Float | String | Element) as res',
            ) =>
            Valid(View(props', res'))

          | _ => Invalid(NotInferrable)
          },
    )

and analyze_object_properties = (defs, properties) =>
  properties
  |> List.fold_left(
       acc =>
         fst
         % Interface.ObjectEntry.(
             fold(
               ~required=
                 (((key, _), (value, _))) =>
                   [(key, (value |> analyze(defs), true))]
                   |> List.merge_assoc(acc),
               ~optional=
                 (((key, _), (value, _))) =>
                   [(key, (value |> analyze(defs), false))]
                   |> List.merge_assoc(acc),
               ~spread=
                 ((value, _)) => {
                   let type_ = value |> analyze(defs);

                   switch (type_) {
                   | Invalid(_) => acc

                   | Valid(Object(xs)) => xs |> List.merge_assoc(acc)

                   // TODO: need to add error handling here to report spreading a non-struct type
                   | Valid(_) => raise(NotImplemented)
                   };
                 },
             )
           ),
       [],
     );
