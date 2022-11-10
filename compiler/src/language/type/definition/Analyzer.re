open Knot.Kore;
open AST;

let analyze_decorator =
    (ctx: ParseContext.t, target: Type.DecoratorTarget.t, decorator) => {
  let decorator' =
    decorator
    |> Node.map(
         Tuple.map_fst2(
           Node.map_type(type_ => type_ |?: Type.Invalid(NotInferrable)),
         ),
       );

  let is_valid =
    decorator
    |> (
      (((id, args), _) as node) => {
        let args_types = args |> List.map(Node.get_type);

        switch (Node.get_type(id)) {
        | Some(Type.Valid(`Decorator(_, expected_target)))
            when expected_target != target =>
          ctx
          |> ParseContext.report(
               TypeError(DecoratorTargetMismatch(expected_target, target)),
               Node.get_range(id),
             );

          false;

        | Some(Type.Valid(`Decorator(expected_args, _)))
            when
              List.length(expected_args) == List.length(args_types)
              && List.for_all2((==), expected_args, args_types) =>
          true

        /* forward invalid types */
        | Some(Type.Invalid(_)) => false

        | Some(type_) =>
          ctx
          |> ParseContext.report(
               TypeError(InvalidDecoratorInvocation(type_, args_types)),
               Node.get_range(node),
             );
          false;

        | None =>
          ctx
          |> ParseContext.report(
               TypeError(NotFound(fst(id))),
               Node.get_range(id),
             );
          false;
        };
      }
    );

  (decorator', is_valid);
};
